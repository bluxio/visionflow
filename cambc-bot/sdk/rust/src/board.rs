//! Aligned owning spatial buffers. Layout is queried and validated for each direct constructor.
//! Host primitives validate coordinates and buffer sizes; mutable operations require exclusive access.
use crate::{check, query, sys, Error, Result, Symmetry, Vec2};
/// Query and validate current map/storage metadata without populating any global cache.
pub(crate) fn layout() -> Result<sys::Metadata> {
    // SAFETY: the host writes all seven metadata fields on success.
    let value = unsafe { crate::query_init(|p| sys::bc_metadata(p)) }?;
    validate_metadata(&value)?;
    Ok(value)
}
/// Reject inconsistent dimensions, chunk counts, strides and storage sizes before allocation.
pub(crate) fn validate_metadata(m: &sys::Metadata) -> Result<()> {
    if m.width <= 0 || m.height <= 0 {
        return Err(Error::invalid_argument("invalid host dimensions"));
    }
    let w = (m.width as u32).div_ceil(8);
    let h = (m.height as u32).div_ceil(8);
    let stride = (w + 1) & !1;
    let board = stride.checked_mul(h).and_then(|v| v.checked_mul(8));
    let field = w.checked_mul(h).and_then(|v| v.checked_mul(128));
    if m.chunked_width != w
        || m.chunked_height != h
        || m.chunk_stride != stride
        || board != Some(m.board_bytes)
        || field != Some(m.field_bytes)
    {
        return Err(Error::invalid_argument("inconsistent host metadata"));
    }
    Ok(())
}
#[repr(C, align(16))]
#[derive(Clone, Debug)]
struct Chunk([u64; 2]);
/// Aligned, owning engine-chunked tile mask. Clone copies all storage.
#[derive(Debug)]
pub struct Bitboard {
    storage: Vec<Chunk>,
    width: i32,
    height: i32,
}
impl PartialEq for Bitboard {
    fn eq(&self, b: &Self) -> bool {
        self.is_equal(b).unwrap_or(false)
    }
}
impl Eq for Bitboard {}
impl Bitboard {
    /// Allocate an empty aligned owning mask from a freshly queried, validated layout.
    /// No global initialisation or cached layout is required.
    pub fn new() -> Result<Self> {
        Self::from_layout(&layout()?)
    }
    /// Allocate owned aligned storage from already validated engine metadata, cleared by the host.
    pub(crate) fn from_layout(m: &sys::Metadata) -> Result<Self> {
        // SAFETY: Bitboard_Zero writes every chunk of a board-sized buffer.
        unsafe {
            Self::from_host(m.board_bytes as usize / 16, m.width, m.height, |p| {
                sys::Bitboard_Zero(p)
            })
        }
    }
    /// Allocate uninitialised storage and expose it only after `fill` succeeds.
    ///
    /// # Safety
    /// On success, `fill` must initialise all `len` chunks.
    pub(crate) unsafe fn from_host(
        len: usize,
        width: i32,
        height: i32,
        fill: impl FnOnce(*mut u64) -> i32,
    ) -> Result<Self> {
        let mut storage = Vec::<Chunk>::with_capacity(len);
        check(fill(storage.as_mut_ptr().cast()))?;
        // SAFETY: capacity is len and the caller guarantees fill initialized every chunk.
        unsafe { storage.set_len(len) };
        Ok(Self {
            storage,
            width,
            height,
        })
    }
    /// Return the stored map width in tiles.
    #[must_use]
    pub fn width(&self) -> i32 {
        self.width
    }
    /// Return the stored map height in tiles.
    #[must_use]
    pub fn height(&self) -> i32 {
        self.height
    }
    /// Return the required chunked storage size in bytes, including padding.
    #[must_use]
    pub fn raw_buffer_size(&self) -> usize {
        self.storage.len() * 16
    }
    /// Return the required row-major storage size in bytes, excluding chunk padding.
    #[must_use]
    pub fn linear_buffer_size(&self) -> usize {
        (self.width as usize * self.height as usize).div_ceil(8)
    }
    /// Borrow the aligned raw storage pointer for a synchronous host operation.
    pub(crate) fn ptr(&self) -> *const u64 {
        self.storage.as_ptr().cast()
    }
    /// Borrow the aligned mutable storage pointer for a synchronous host operation.
    pub(crate) fn ptr_mut(&mut self) -> *mut u64 {
        self.storage.as_mut_ptr().cast()
    }
    /// Clear all bits locally, including chunk padding.
    pub fn clear(&mut self) {
        if unsafe { sys::Bitboard_Zero(self.ptr_mut()) } != 0 {
            self.storage.fill(Chunk([0; 2]));
        }
    }
    /// Set every valid map tile using the host primitive.
    pub fn fill(&mut self) -> Result<()> {
        check(unsafe { sys::Bitboard_One(self.ptr_mut()) })
    }
    /// Query whether the bit at `p` is set; invalid coordinates propagate an engine error.
    ///
    /// # Arguments
    /// * `p` - Tile to query.
    pub fn is_set(&self, p: Vec2) -> Result<bool> {
        Ok(query(|out| unsafe { sys::Bitboard_Test(self.ptr(), p.x, p.y, out) })? != 0)
    }
    /// Set or clear the bit at `p`; invalid coordinates propagate an engine error.
    ///
    /// # Arguments
    /// * `p` - Tile to update.
    /// * `value` - `true` to set the bit, `false` to clear it.
    pub fn set(&mut self, p: Vec2, value: bool) -> Result<()> {
        check(unsafe { sys::Bitboard_Set(self.ptr_mut(), p.x, p.y, i32::from(value)) })
    }
    /// Toggle the bit at `p`; invalid coordinates propagate an engine error.
    ///
    /// # Arguments
    /// * `p` - Tile to flip.
    pub fn toggle(&mut self, p: Vec2) -> Result<()> {
        check(unsafe { sys::Bitboard_Toggle(self.ptr_mut(), p.x, p.y) })
    }
    /// Query whether the mask has no set map tiles.
    pub fn is_empty(&self) -> Result<bool> {
        Ok(query(|p| unsafe { sys::Bitboard_IsZero(self.ptr(), p) })? != 0)
    }
    /// Query the number of set map tiles.
    pub fn count(&self) -> Result<usize> {
        Ok(query(|p| unsafe { sys::Bitboard_NumSetBits(self.ptr(), p) })? as usize)
    }
    /// Return an owning list of all set tile coordinates; oversized host lists are rejected.
    pub fn positions(&self) -> Result<Vec<Vec2>> {
        let count = self.count()?;
        let mut values = vec![Vec2::default(); count];
        let written = query(|out| unsafe {
            sys::Bitboard_GetSetBits(self.ptr(), values.as_mut_ptr().cast(), values.len(), out)
        })?;
        let written = usize::try_from(written)
            .map_err(|_| Error::invalid_argument("host returned invalid position count"))?;
        if written > count {
            return Err(Error::invalid_argument("host returned oversized positions"));
        }
        values.truncate(written);
        Ok(values)
    }
    /// Print the stored board/field through the host debug primitive.
    pub fn print(&self) -> Result<()> {
        check(unsafe { sys::Bitboard_Print(self.ptr()) })
    }
    /// Initialise output storage, call the host, and return it only after checking success.
    pub(crate) fn query(f: impl FnOnce(*mut u64) -> i32) -> Result<Self> {
        let mut b = Self::new()?;
        check(f(b.ptr_mut()))?;
        Ok(b)
    }
    /// Allocate zeroed output with this mask's layout, without another metadata query.
    fn empty_like(&self) -> Result<Self> {
        // SAFETY: Bitboard_Zero writes every chunk of a board-sized buffer.
        unsafe {
            Self::from_host(self.storage.len(), self.width, self.height, |p| {
                sys::Bitboard_Zero(p)
            })
        }
    }
    /// Return independent same-layout output only after the host operation succeeds.
    fn query_like(&self, f: impl FnOnce(*mut u64) -> i32) -> Result<Self> {
        let mut b = self.empty_like()?;
        check(f(b.ptr_mut()))?;
        Ok(b)
    }
    /// Return an owning mask with valid map tiles complemented.
    pub fn complement(&self) -> Result<Self> {
        self.query_like(|p| unsafe { sys::Bitboard_Not(self.ptr(), p) })
    }
    /// Query whether two masks have equal map bits.
    ///
    /// # Arguments
    /// * `b` - Mask to compare against.
    pub fn is_equal(&self, b: &Self) -> Result<bool> {
        if core::ptr::eq(self, b) {
            return Ok(true);
        }
        Ok(query(|p| unsafe { sys::Bitboard_IsEqual(self.ptr(), b.ptr(), p) })? != 0)
    }
    /// Return an owning mask transformed by the requested map symmetry.
    ///
    /// # Arguments
    /// * `s` - Map symmetry to apply.
    pub fn reflect(&self, s: Symmetry) -> Result<Self> {
        self.query_like(|p| unsafe { sys::Bitboard_Reflect(self.ptr(), p, s as i32) })
    }
    /// Return an owning horizontally reflected mask using the host map geometry.
    pub fn reflect_horizontal(&self) -> Result<Self> {
        self.reflect(Symmetry::Horizontal)
    }
    /// Return an owning vertically reflected mask using the host map geometry.
    pub fn reflect_vertical(&self) -> Result<Self> {
        self.reflect(Symmetry::Vertical)
    }
    /// Return an owning mask rotated 180 degrees using the host map geometry.
    pub fn rotate_180(&self) -> Result<Self> {
        self.reflect(Symmetry::Rotational180)
    }
    /// Compute reachability from `origin` up to `distance`, reusing caller-provided scratch storage.
    ///
    /// # Arguments
    /// * `origin` - Start tile.
    /// * `distance` - Maximum number of 8-directional steps from `origin`.
    /// * `frontier` - Working buffer; its contents are overwritten.
    /// * `scratch` - Working buffer; its contents are overwritten.
    pub fn reachable_tiles_with_scratch(
        &self,
        origin: Vec2,
        distance: u16,
        frontier: &mut Self,
        scratch: &mut Self,
    ) -> Result<Self> {
        self.query_like(|p| unsafe {
            sys::Bitboard_ReachableMaskWithScratch(
                self.ptr(),
                origin.x,
                origin.y,
                p,
                distance,
                frontier.ptr_mut(),
                scratch.ptr_mut(),
            )
        })
    }
    /// Return an owning union of this mask and `b`.
    ///
    /// # Arguments
    /// * `b` - Mask to combine with; may alias `self`.
    pub fn union(&self, b: &Self) -> Result<Self> {
        if core::ptr::eq(self, b) {
            return Ok(self.clone());
        }
        self.query_like(|p| unsafe { sys::Bitboard_Union(self.ptr(), b.ptr(), p) })
    }
    /// Replace this mask with its union with `b`.
    ///
    /// # Arguments
    /// * `b` - Mask to combine with; may alias `self`.
    pub fn union_assign(&mut self, b: &Self) -> Result<()> {
        check(unsafe { sys::Bitboard_Union(self.ptr(), b.ptr(), self.ptr_mut()) })
    }
    /// Return an owning intersection of this mask and `b`.
    ///
    /// # Arguments
    /// * `b` - Mask to combine with; may alias `self`.
    pub fn intersect(&self, b: &Self) -> Result<Self> {
        if core::ptr::eq(self, b) {
            return Ok(self.clone());
        }
        self.query_like(|p| unsafe { sys::Bitboard_Intersect(self.ptr(), b.ptr(), p) })
    }
    /// Replace this mask with its intersection with `b`.
    ///
    /// # Arguments
    /// * `b` - Mask to combine with; may alias `self`.
    pub fn intersect_assign(&mut self, b: &Self) -> Result<()> {
        check(unsafe { sys::Bitboard_Intersect(self.ptr(), b.ptr(), self.ptr_mut()) })
    }
    /// Return an owning symmetric difference of this mask and `b`.
    ///
    /// # Arguments
    /// * `b` - Mask to combine with; may alias `self`.
    pub fn xor(&self, b: &Self) -> Result<Self> {
        if core::ptr::eq(self, b) {
            return self.empty_like();
        }
        self.query_like(|p| unsafe { sys::Bitboard_Xor(self.ptr(), b.ptr(), p) })
    }
    /// Replace this mask with its symmetric difference with `b`.
    ///
    /// # Arguments
    /// * `b` - Mask to combine with; may alias `self`.
    pub fn xor_assign(&mut self, b: &Self) -> Result<()> {
        check(unsafe { sys::Bitboard_Xor(self.ptr(), b.ptr(), self.ptr_mut()) })
    }
    /// Return an owning mask shifted horizontally by the signed column count.
    ///
    /// # Arguments
    /// * `count` - Columns to shift; negative values shift left.
    pub fn shift_right(&self, count: i32) -> Result<Self> {
        self.query_like(|p| unsafe { sys::Bitboard_ShiftRight(self.ptr(), p, count) })
    }
    /// Return an owning mask shifted vertically by the signed row count.
    ///
    /// # Arguments
    /// * `count` - Rows to shift; negative values shift down.
    pub fn shift_up(&self, count: i32) -> Result<Self> {
        self.query_like(|p| unsafe { sys::Bitboard_ShiftUp(self.ptr(), p, count) })
    }
    /// Add tiles within a Euclidean circle centred at `center`; host validates the radius.
    ///
    /// # Arguments
    /// * `center` - Centre tile.
    /// * `radius` - Euclidean radius in tiles.
    pub fn draw_circle(&mut self, center: Vec2, radius: f32) -> Result<()> {
        check(unsafe { sys::Bitboard_DrawCircle(self.ptr_mut(), center.x, center.y, radius) })
    }
    /// Add tiles within Chebyshev `radius` of `center`; host validates the radius.
    ///
    /// # Arguments
    /// * `center` - Centre tile.
    /// * `radius` - Chebyshev radius in tiles.
    pub fn draw_chebyshev(&mut self, center: Vec2, radius: f32) -> Result<()> {
        check(unsafe { sys::Bitboard_DrawChebyshev(self.ptr_mut(), center.x, center.y, radius) })
    }
    /// Compute an owning reachability mask from `origin` up to `distance`.
    ///
    /// # Arguments
    /// * `origin` - Start tile.
    /// * `distance` - Maximum number of 8-directional steps from `origin`.
    pub fn reachable_tiles(&self, origin: Vec2, distance: u16) -> Result<Self> {
        self.query_like(|p| unsafe {
            sys::Bitboard_ReachableMask(self.ptr(), origin.x, origin.y, p, distance)
        })
    }
    /// Compute an owning shadow-cast mask from `origin` up to `distance`.
    ///
    /// # Arguments
    /// * `origin` - Viewer tile.
    /// * `distance` - Maximum sight radius in tiles.
    pub fn shadow_cast(&self, origin: Vec2, distance: u16) -> Result<Self> {
        self.query_like(|p| unsafe {
            sys::Bitboard_ShadowCast(self.ptr(), origin.x, origin.y, p, distance)
        })
    }
    /// Like [`Self::shift_right`], but writes into `out`.
    ///
    /// # Arguments
    /// * `count` - Columns to shift; negative values shift left.
    /// * `out` - Destination mask; its previous contents are overwritten.
    pub fn shift_right_into(&self, count: i32, out: &mut Self) -> Result<()> {
        check(unsafe { sys::Bitboard_ShiftRight(self.ptr(), out.ptr_mut(), count) })
    }
    /// Like [`Self::shift_up`], but writes into `out`.
    ///
    /// # Arguments
    /// * `count` - Rows to shift; negative values shift down.
    /// * `out` - Destination mask; its previous contents are overwritten.
    pub fn shift_up_into(&self, count: i32, out: &mut Self) -> Result<()> {
        check(unsafe { sys::Bitboard_ShiftUp(self.ptr(), out.ptr_mut(), count) })
    }
    /// Like [`Self::reflect`], but writes into `out`.
    ///
    /// # Arguments
    /// * `s` - Map symmetry to apply.
    /// * `out` - Destination mask; its previous contents are overwritten.
    pub fn reflect_into(&self, s: Symmetry, out: &mut Self) -> Result<()> {
        check(unsafe { sys::Bitboard_Reflect(self.ptr(), out.ptr_mut(), s as i32) })
    }
    /// Like [`Self::reflect_horizontal`], but writes into `out`.
    ///
    /// # Arguments
    /// * `out` - Destination mask; its previous contents are overwritten.
    pub fn reflect_horizontal_into(&self, out: &mut Self) -> Result<()> {
        self.reflect_into(Symmetry::Horizontal, out)
    }
    /// Like [`Self::reflect_vertical`], but writes into `out`.
    ///
    /// # Arguments
    /// * `out` - Destination mask; its previous contents are overwritten.
    pub fn reflect_vertical_into(&self, out: &mut Self) -> Result<()> {
        self.reflect_into(Symmetry::Vertical, out)
    }
    /// Like [`Self::rotate_180`], but writes into `out`.
    ///
    /// # Arguments
    /// * `out` - Destination mask; its previous contents are overwritten.
    pub fn rotate_180_into(&self, out: &mut Self) -> Result<()> {
        self.reflect_into(Symmetry::Rotational180, out)
    }
    /// Like [`Self::reachable_tiles`], but writes into `out`.
    ///
    /// # Arguments
    /// * `origin` - Start tile.
    /// * `distance` - Maximum number of 8-directional steps from `origin`.
    /// * `out` - Destination mask; its previous contents are overwritten.
    pub fn reachable_tiles_into(&self, origin: Vec2, distance: u16, out: &mut Self) -> Result<()> {
        check(unsafe {
            sys::Bitboard_ReachableMask(self.ptr(), origin.x, origin.y, out.ptr_mut(), distance)
        })
    }
    /// Like [`Self::reachable_tiles_with_scratch`], but writes into `out`.
    ///
    /// # Arguments
    /// * `origin` - Start tile.
    /// * `distance` - Maximum number of 8-directional steps from `origin`.
    /// * `frontier` - Working buffer; its contents are overwritten.
    /// * `scratch` - Working buffer; its contents are overwritten.
    /// * `out` - Destination mask; its previous contents are overwritten.
    pub fn reachable_tiles_with_scratch_into(
        &self,
        origin: Vec2,
        distance: u16,
        frontier: &mut Self,
        scratch: &mut Self,
        out: &mut Self,
    ) -> Result<()> {
        check(unsafe {
            sys::Bitboard_ReachableMaskWithScratch(
                self.ptr(),
                origin.x,
                origin.y,
                out.ptr_mut(),
                distance,
                frontier.ptr_mut(),
                scratch.ptr_mut(),
            )
        })
    }
    /// Like [`Self::shadow_cast`], but writes into `out`.
    ///
    /// # Arguments
    /// * `origin` - Viewer tile.
    /// * `distance` - Maximum sight radius in tiles.
    /// * `out` - Destination mask; its previous contents are overwritten.
    pub fn shadow_cast_into(&self, origin: Vec2, distance: u16, out: &mut Self) -> Result<()> {
        check(unsafe {
            sys::Bitboard_ShadowCast(self.ptr(), origin.x, origin.y, out.ptr_mut(), distance)
        })
    }
    /// Import exact-size chunked bytes; wrong size or invalid layout propagates an error.
    ///
    /// # Arguments
    /// * `bytes` - Engine-chunked bytes, exactly `raw_buffer_size` long.
    pub fn import_raw_buffer(&mut self, bytes: &[u8]) -> Result<()> {
        check(unsafe { sys::Bitboard_ImportRawBuffer(self.ptr_mut(), bytes.as_ptr(), bytes.len()) })
    }
    /// Export chunked bytes into caller storage; its size must match `raw_buffer_size`.
    ///
    /// # Arguments
    /// * `bytes` - Destination, exactly `raw_buffer_size` long.
    pub fn export_raw_buffer_into(&self, bytes: &mut [u8]) -> Result<()> {
        check(unsafe { sys::Bitboard_ExportRawBuffer(self.ptr(), bytes.as_mut_ptr(), bytes.len()) })
    }
    /// Return an owning copy of the chunked storage bytes.
    pub fn export_raw_buffer(&self) -> Result<Vec<u8>> {
        let mut bytes = vec![0; self.raw_buffer_size()];
        self.export_raw_buffer_into(&mut bytes)?;
        Ok(bytes)
    }
    /// Import row-major tile values; host validates the exact buffer size.
    ///
    /// # Arguments
    /// * `bytes` - One value per tile in row-major order.
    pub fn import_linear_buffer(&mut self, bytes: &[u8]) -> Result<()> {
        check(unsafe {
            sys::Bitboard_ImportLinearBuffer(self.ptr_mut(), bytes.as_ptr(), bytes.len())
        })
    }
    /// Export row-major tile values into an exact-size caller buffer.
    ///
    /// # Arguments
    /// * `bytes` - Destination with one value per tile in row-major order.
    pub fn export_linear_buffer_into(&self, bytes: &mut [u8]) -> Result<()> {
        check(unsafe {
            sys::Bitboard_ExportLinearBuffer(self.ptr(), bytes.as_mut_ptr(), bytes.len())
        })
    }
    /// Return an owning row-major buffer without chunk padding.
    pub fn export_linear_buffer(&self) -> Result<Vec<u8>> {
        let mut bytes = vec![0; self.linear_buffer_size()];
        self.export_linear_buffer_into(&mut bytes)?;
        Ok(bytes)
    }
}
/// Owning engine-chunked u16 tile distances.
#[repr(C, align(16))]
#[derive(Clone, Debug)]
struct FieldChunk([u16; 8]);
/// Owning, 16-byte-aligned map distances; cloning copies its chunked storage.
#[derive(Debug)]
pub struct DistanceField {
    /// Empty until written; reads before then see zeros.
    storage: Vec<FieldChunk>,
    chunks: usize,
    width: i32,
    height: i32,
}
impl PartialEq for DistanceField {
    fn eq(&self, b: &Self) -> bool {
        core::ptr::eq(self, b)
            || matches!((self.export_linear_buffer(), b.export_linear_buffer()), (Ok(x), Ok(y)) if x == y)
    }
}
impl Eq for DistanceField {}
const DIRECTIONS: [Vec2; 8] = [
    Vec2::NORTH,
    Vec2::NORTH_EAST,
    Vec2::EAST,
    Vec2::SOUTH_EAST,
    Vec2::SOUTH,
    Vec2::SOUTH_WEST,
    Vec2::WEST,
    Vec2::NORTH_WEST,
];

impl DistanceField {
    /// Allocate aligned owning distance storage from a freshly queried, validated layout.
    /// No global initialisation or cached layout is required; compute or set values before use.
    pub fn new() -> Result<Self> {
        Ok(Self::from_layout(&layout()?))
    }

    /// Distance sentinel for an unreachable tile.
    pub const UNREACHABLE: u16 = u16::MAX;
    /// Allocate owned aligned storage from already validated engine metadata.
    pub(crate) fn from_layout(m: &sys::Metadata) -> Self {
        let chunks = m.field_bytes as usize / 16;
        Self {
            storage: Vec::with_capacity(chunks),
            chunks,
            width: m.width,
            height: m.height,
        }
    }
    /// Return the stored map width in tiles.
    #[must_use]
    pub fn width(&self) -> i32 {
        self.width
    }
    /// Return the stored map height in tiles.
    #[must_use]
    pub fn height(&self) -> i32 {
        self.height
    }
    /// Return the required chunked storage size in bytes, including padding.
    #[must_use]
    pub fn raw_buffer_size(&self) -> usize {
        self.chunks * 16
    }
    /// Return the required row-major storage size in bytes, excluding chunk padding.
    #[must_use]
    pub fn linear_buffer_size(&self) -> usize {
        self.width as usize * self.height as usize * 2
    }
    /// Borrow the aligned raw storage pointer for a synchronous host operation.
    pub(crate) fn ptr(&self) -> *const u16 {
        if self.storage.len() == self.chunks {
            self.storage.as_ptr().cast()
        } else {
            zero_field(self.chunks)
        }
    }
    /// Borrow the aligned mutable storage pointer for a synchronous host operation.
    fn ptr_mut(&mut self) -> *mut u16 {
        if self.storage.len() != self.chunks {
            self.storage.clear();
            self.storage.resize(self.chunks, FieldChunk([0; 8]));
        }
        self.storage.as_mut_ptr().cast()
    }
    /// Run a host compute that writes every chunk on success, skipping the zero fill.
    fn compute_into(&mut self, f: impl FnOnce(*mut u16, *mut u16) -> i32) -> Result<u16> {
        self.storage
            .reserve_exact(self.chunks.saturating_sub(self.storage.len()));
        let field = self.storage.as_mut_ptr().cast();
        let count = query(|out| f(field, out))?;
        // SAFETY: capacity covers chunks and the successful compute initialized all of them.
        unsafe { self.storage.set_len(self.chunks) };
        Ok(count)
    }
    /// Query the distance at `p`; invalid coordinates propagate an engine error.
    ///
    /// # Arguments
    /// * `p` - Tile to query.
    pub fn get(&self, p: Vec2) -> Result<u16> {
        query(|out| unsafe { sys::DistanceField_Get(self.ptr(), p.x, p.y, out) })
    }
    /// First optimal step from `from` towards a root, as a direction delta in compass order from north.
    /// Returns `None` at a root or unreached tile; an out-of-map `from` propagates an engine error.
    ///
    /// # Arguments
    /// * `from` - Tile to step from.
    pub fn next_move_from(&self, from: Vec2) -> Result<Option<Vec2>> {
        let (best, distances) = self.neighbour_distances(from)?;
        Ok(DIRECTIONS
            .into_iter()
            .zip(distances)
            .find(|&(_, d)| best != Self::UNREACHABLE && d == best)
            .map(|(m, _)| m))
    }
    /// Every optimal step from `from` towards a root, as direction deltas in compass order from north.
    ///
    /// # Arguments
    /// * `from` - Tile to step from.
    pub fn next_moves_from(&self, from: Vec2) -> Result<Vec<Vec2>> {
        let (best, distances) = self.neighbour_distances(from)?;
        Ok(DIRECTIONS
            .into_iter()
            .zip(distances)
            .filter(|&(_, d)| best != Self::UNREACHABLE && d == best)
            .map(|(m, _)| m)
            .collect())
    }
    /// Neighbour distances and the smallest one if it improves on `from`, else `UNREACHABLE`.
    fn neighbour_distances(&self, from: Vec2) -> Result<(u16, [u16; 8])> {
        let here = self.get(from)?;
        let mut out = [Self::UNREACHABLE; 8];
        for (d, slot) in DIRECTIONS.into_iter().zip(&mut out) {
            let p = from + d;
            if p.x >= 0 && p.y >= 0 && p.x < self.width && p.y < self.height {
                *slot = self.get(p)?;
            }
        }
        let best = out.into_iter().min().unwrap_or(Self::UNREACHABLE);
        Ok((
            if here != Self::UNREACHABLE && best < here {
                best
            } else {
                Self::UNREACHABLE
            },
            out,
        ))
    }
    /// Set the distance at `p`; invalid coordinates propagate an engine error.
    ///
    /// # Arguments
    /// * `p` - Tile to update.
    /// * `value` - Distance to store.
    pub fn set(&mut self, p: Vec2, value: u16) -> Result<()> {
        check(unsafe { sys::DistanceField_Set(self.ptr_mut(), p.x, p.y, value) })
    }
    /// Print the stored board/field through the host debug primitive.
    pub fn print(&self) -> Result<()> {
        check(unsafe { sys::DistanceField_Print(self.ptr()) })
    }
    /// Import exact-size chunked bytes; wrong size or invalid layout propagates an error.
    ///
    /// # Arguments
    /// * `bytes` - Engine-chunked bytes, exactly `raw_buffer_size` long.
    pub fn import_raw_buffer(&mut self, bytes: &[u8]) -> Result<()> {
        check(unsafe {
            sys::DistanceField_ImportRawBuffer(self.ptr_mut(), bytes.as_ptr(), bytes.len())
        })
    }
    /// Export chunked bytes into caller storage; its size must match `raw_buffer_size`.
    ///
    /// # Arguments
    /// * `bytes` - Destination, exactly `raw_buffer_size` long.
    pub fn export_raw_buffer_into(&self, bytes: &mut [u8]) -> Result<()> {
        check(unsafe {
            sys::DistanceField_ExportRawBuffer(self.ptr(), bytes.as_mut_ptr(), bytes.len())
        })
    }
    /// Return an owning copy of the chunked storage bytes.
    pub fn export_raw_buffer(&self) -> Result<Vec<u8>> {
        let mut b = vec![0; self.raw_buffer_size()];
        self.export_raw_buffer_into(&mut b)?;
        Ok(b)
    }
    /// Import row-major tile values; host validates the exact buffer size.
    ///
    /// # Arguments
    /// * `values` - One distance per tile in row-major order.
    pub fn import_linear_buffer(&mut self, values: &[u16]) -> Result<()> {
        check(unsafe {
            sys::DistanceField_ImportLinearBuffer(
                self.ptr_mut(),
                values.as_ptr().cast(),
                core::mem::size_of_val(values),
            )
        })
    }
    /// Export row-major tile values into an exact-size caller buffer.
    ///
    /// # Arguments
    /// * `values` - Destination with one distance per tile in row-major order.
    pub fn export_linear_buffer_into(&self, values: &mut [u16]) -> Result<()> {
        check(unsafe {
            sys::DistanceField_ExportLinearBuffer(
                self.ptr(),
                values.as_mut_ptr().cast(),
                core::mem::size_of_val(values),
            )
        })
    }
    /// Return an owning row-major buffer without chunk padding.
    pub fn export_linear_buffer(&self) -> Result<Vec<u16>> {
        let mut v = vec![0; self.linear_buffer_size() / 2];
        self.export_linear_buffer_into(&mut v)?;
        Ok(v)
    }
    /// Compute distances from `roots` around `walls`, writing both this field and `visited`.
    /// Expansion stops at `max_distance`; buffers must have compatible map layouts.
    ///
    /// # Arguments
    /// * `walls` - Impassable tiles.
    /// * `roots` - Tiles at distance zero.
    /// * `visited` - Receives the tiles the search reached.
    /// * `max_distance` - Tiles farther than this are left unreached.
    pub fn compute(
        &mut self,
        walls: &Bitboard,
        roots: &Bitboard,
        visited: &mut Bitboard,
        max_distance: u16,
    ) -> Result<()> {
        self.compute_with_expansion_count(walls, roots, visited, max_distance)
            .map(|_| ())
    }
    /// Compute distances and return the host expansion count, writing this field and `visited`.
    ///
    /// # Arguments
    /// * `walls` - Impassable tiles.
    /// * `roots` - Tiles at distance zero.
    /// * `visited` - Receives the tiles the search reached.
    /// * `max_distance` - Tiles farther than this are left unreached.
    pub fn compute_with_expansion_count(
        &mut self,
        walls: &Bitboard,
        roots: &Bitboard,
        visited: &mut Bitboard,
        max_distance: u16,
    ) -> Result<u16> {
        self.compute_into(|field, out| unsafe {
            sys::DistanceField_ComputeWithExpansionCount(
                walls.ptr(),
                roots.ptr(),
                visited.ptr_mut(),
                field,
                max_distance,
                out,
            )
        })
    }
    /// Compute distances towards `destinations`, writing this field and `visited`.
    /// The host stops when destinations or the distance limit end the search.
    ///
    /// # Arguments
    /// * `walls` - Impassable tiles.
    /// * `roots` - Tiles at distance zero.
    /// * `destinations` - The search stops once these tiles are reached.
    /// * `visited` - Receives the tiles the search reached.
    /// * `max_distance` - Tiles farther than this are left unreached.
    pub fn compute_to(
        &mut self,
        walls: &Bitboard,
        roots: &Bitboard,
        destinations: &Bitboard,
        visited: &mut Bitboard,
        max_distance: u16,
    ) -> Result<()> {
        self.compute_to_with_expansion_count(walls, roots, destinations, visited, max_distance)
            .map(|_| ())
    }
    /// Compute towards `destinations` and return the host expansion count.
    ///
    /// # Arguments
    /// * `walls` - Impassable tiles.
    /// * `roots` - Tiles at distance zero.
    /// * `destinations` - The search stops once these tiles are reached.
    /// * `visited` - Receives the tiles the search reached.
    /// * `max_distance` - Tiles farther than this are left unreached.
    pub fn compute_to_with_expansion_count(
        &mut self,
        walls: &Bitboard,
        roots: &Bitboard,
        destinations: &Bitboard,
        visited: &mut Bitboard,
        max_distance: u16,
    ) -> Result<u16> {
        self.compute_into(|field, out| unsafe {
            sys::DistanceField_ComputeToWithExpansionCount(
                walls.ptr(),
                roots.ptr(),
                destinations.ptr(),
                visited.ptr_mut(),
                field,
                max_distance,
                out,
            )
        })
    }
}
/// Shared zeroed storage for reading distance fields that were never written.
fn zero_field(chunks: usize) -> *const u16 {
    static ZEROS: std::sync::Mutex<&'static [FieldChunk]> = std::sync::Mutex::new(&[]);
    let mut zeros = ZEROS.lock().unwrap_or_else(|e| e.into_inner());
    if zeros.len() < chunks {
        *zeros = Vec::leak(vec![FieldChunk([0; 8]); chunks]);
    }
    zeros.as_ptr().cast()
}
impl Clone for Bitboard {
    fn clone(&self) -> Self {
        // SAFETY: a successful raw import writes every chunk.
        unsafe {
            Self::from_host(self.storage.len(), self.width, self.height, |p| {
                sys::Bitboard_ImportRawBuffer(p, self.ptr().cast(), self.raw_buffer_size())
            })
        }
        .unwrap_or_else(|_| Self {
            storage: self.storage.clone(),
            width: self.width,
            height: self.height,
        })
    }
}
impl Clone for DistanceField {
    fn clone(&self) -> Self {
        let mut copy = Self {
            storage: Vec::with_capacity(self.chunks),
            chunks: self.chunks,
            width: self.width,
            height: self.height,
        };
        if self.storage.len() == self.chunks {
            let field = copy.storage.as_mut_ptr().cast();
            if unsafe {
                sys::DistanceField_ImportRawBuffer(field, self.ptr().cast(), self.raw_buffer_size())
            } == 0
            {
                // SAFETY: capacity covers chunks and a successful raw import writes all of them.
                unsafe { copy.storage.set_len(self.chunks) };
            } else {
                copy.storage.clone_from(&self.storage);
            }
        }
        copy
    }
}
