//! Fresh engine-backed shop metadata queried on item/consumable type values.
//! Optional tags are decoded explicitly; private ABI records retain fields omitted from public values.
use crate::{query, sys, ConsumableType, Error, ItemType, Result};
/// Checked item-passive tag; item metadata uses `None` for no passive.
#[repr(u32)]
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub enum ItemPassive {
    /// The engine Monster item passive.
    Monster = 1,
    /// The engine Durability item passive.
    Durability = 2,
    /// The engine Treasure item passive.
    Treasure = 3,
    /// The engine Jungle item passive.
    Jungle = 4,
    /// The engine Home item passive.
    Home = 5,
}
impl TryFrom<u32> for ItemPassive {
    type Error = Error;
    fn try_from(v: u32) -> Result<Self> {
        match v {
            1 => Ok(Self::Monster),
            2 => Ok(Self::Durability),
            3 => Ok(Self::Treasure),
            4 => Ok(Self::Jungle),
            5 => Ok(Self::Home),
            _ => Err(Error::invalid_value("item passive", v as i64)),
        }
    }
}
/// Owned, engine-queried item metadata with checked optional tags.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct ItemInfo {
    /// Gold cost to purchase one unit.
    pub cost: u32,
    /// Number of identical ingredients required for the combination.
    pub combine_count: u32,
    /// Combination result, or `None` when this item cannot combine.
    pub result: Option<ItemType>,
    /// Item health bonus.
    pub health_points: i32,
    /// Strength value reported by this record.
    pub strength_points: i32,
    /// Focus value reported by this record.
    pub focus_points: i32,
    /// Item passive, or `None` when no passive applies.
    pub passive: Option<ItemPassive>,
    /// Magnitude interpreted according to the item passive.
    pub passive_magnitude: f32,
    /// Whether the item can be bought directly; emblems must be combined.
    pub purchasable: bool,
}
/// Owned consumable cost and carry limit; matches the engine C record.
#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub struct ConsumableInfo {
    /// Gold cost to purchase one unit.
    pub cost: u32,
    /// Maximum number of this consumable that may be carried.
    pub carry_limit: u32,
}
const _: () = {
    assert!(core::mem::size_of::<ConsumableInfo>() == 8);
    assert!(core::mem::align_of::<ConsumableInfo>() == 4);
    assert!(core::mem::offset_of!(ConsumableInfo, carry_limit) == 4);
};
impl ItemType {
    /// Get the full value in gold: the purchase price, refunded in full when sold.
    pub fn cost(self) -> Result<u32> {
        Ok(self.info()?.cost)
    }
    /// Get the ingredient count needed to create the result item.
    pub fn combine_count(self) -> Result<u32> {
        Ok(self.info()?.combine_count)
    }
    /// Get the combination result, if this item can be combined.
    pub fn result(self) -> Result<Option<ItemType>> {
        Ok(self.info()?.result)
    }
    /// Get the health bonus.
    pub fn health_points(self) -> Result<i32> {
        Ok(self.info()?.health_points)
    }
    /// Get the strength bonus.
    pub fn strength_points(self) -> Result<i32> {
        Ok(self.info()?.strength_points)
    }
    /// Get the focus bonus.
    pub fn focus_points(self) -> Result<i32> {
        Ok(self.info()?.focus_points)
    }
    /// Get the optional passive effect.
    pub fn passive(self) -> Result<Option<ItemPassive>> {
        Ok(self.info()?.passive)
    }
    /// Get the passive magnitude.
    pub fn passive_magnitude(self) -> Result<f32> {
        Ok(self.info()?.passive_magnitude)
    }
    /// Whether the item can be bought directly; emblems must be combined.
    pub fn is_purchasable(self) -> Result<bool> {
        Ok(self.info()?.purchasable)
    }
    /// Query a fresh owned item record, checking passive/result tags.
    /// Engine errors and unknown runtime enum values remain explicit SDK errors.
    pub fn info(self) -> Result<ItemInfo> {
        let raw = query(|p| unsafe { sys::item_info(self as u32, p) })?;
        Ok(ItemInfo {
            cost: raw.cost,
            combine_count: raw.combine_count,
            result: if raw.result == 0xff {
                None
            } else {
                Some(ItemType::try_from(raw.result)?)
            },
            health_points: raw.health_points,
            strength_points: raw.strength_points,
            focus_points: raw.focus_points,
            passive: if raw.passive == 0 {
                None
            } else {
                Some(ItemPassive::try_from(raw.passive)?)
            },
            passive_magnitude: raw.passive_magnitude,
            purchasable: raw.purchasable != 0,
        })
    }
}
impl ConsumableType {
    /// Get the purchase cost in gold.
    pub fn cost(self) -> Result<u32> {
        Ok(self.info()?.cost)
    }
    /// Get the maximum carried inventory count.
    pub fn carry_limit(self) -> Result<u32> {
        Ok(self.info()?.carry_limit)
    }
    /// Query a fresh owned consumable cost/carry-limit record, preserving engine errors.
    pub fn info(self) -> Result<ConsumableInfo> {
        query(|p| unsafe { sys::consumable_info(self.raw(), p) })
    }
}
