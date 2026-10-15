// Champion decisions. main.rs dispatches here after the draft.
use cambc::{
    Bitboard, Champion,
    DistanceField, ItemType as Item, Result, Team, Unit, Vec2,
};

fn me() -> Result<&'static Champion> {
    Ok(cambc::self_champion()?.expect("play starts after the draft"))
}

fn distance(a: Vec2, b: Vec2) -> i32 {
    a.chebyshev_distance(b) as i32
}

fn tiles_around(position: Vec2, radius: i32) -> Result<Bitboard> {
    let mut tiles = Bitboard::new()?;
    tiles.draw_chebyshev(position, radius as f32)?;
    Ok(tiles)
}

fn blocked_tiles() -> Result<Bitboard> {
    // Combine terrain and visible occupants using native bitboard operations.
    let mut blocked = cambc::map::walls()?.clone();
    for mask in [cambc::map::holes()?.clone(), cambc::map::altars()?.clone(),
        cambc::map::spawn_region(Team::Enemy)?.clone(), cambc::vision::champion_tiles(None)?,
        cambc::vision::camp_tiles()?, cambc::vision::teleport_tiles()?] {
        blocked.union_assign(&mask)?;
    }
    blocked.set(me()?.position()?, false)?;
    Ok(blocked)
}

fn move_towards_closest(mut goals: Bitboard) -> Result<bool> {
    let me = me()?;
    let origin = me.position()?;
    if !me.can_move()? || goals.is_set(origin)? { return Ok(false); }

    let blocked = blocked_tiles()?;
    goals.intersect_assign(&blocked.complement()?.union(&cambc::vision::camp_tiles()?)?)?;
    if goals.is_empty()? { return Ok(false); }
    if goals.count()? == 1 && !blocked.is_set(goals.positions()?[0])? {
        return me.move_towards(goals.positions()?[0]);
    }

    let mut field = DistanceField::new()?;
    field.compute_to(&blocked.intersect(&goals.complement()?)?, &goals, &tiles_around(origin, 0)?, &mut Bitboard::new()?, DistanceField::UNREACHABLE)?;

    let mut step = origin;
    for tile in blocked.reachable_tiles(origin, me.base_stats()?.base_move_speed as u16)?.positions()? {
        if field.get(tile)? < field.get(step)? { step = tile; }
    }
    if step == origin { Ok(false) } else { me.move_absolute(step) }
}

// Make one basic attack if the target is already in attack range.
fn attack_if_in_range(target: &Unit) -> Result<bool> {
    let me = me()?;
    if !me.can_attack()? || !target.is_visible()? || target.health()? <= 0.0
        || !me.attack_tiles()?.is_set(target.position()?)? {
        return Ok(false);
    }
    me.attack(target)?;
    Ok(true)
}

fn farm_camps() -> Result<()> {
    // Attack a camp already in range, otherwise move toward the nearest camp tiles.
    for camp in cambc::vision::camps()? {
        if attack_if_in_range(&camp)? { return Ok(()); }
    }

    let mut goals = Bitboard::new()?;
    let visible = cambc::vision::visible_tiles()?;
    for group in cambc::map::camp_groups()? {
        if !group.spawn_tiles()?.intersect(&visible)?.is_empty()?
            && matches!(group.state()?, cambc::CampGroupState::Respawning(_)) { continue; }
        goals.union_assign(&group.spawn_tiles()?)?;
    }

    if move_towards_closest(goals)? {
        for camp in cambc::vision::camps()? {
            if attack_if_in_range(&camp)? { return Ok(()); }
        }
    }
    Ok(())
}

// Per-champion state survives between turns. Pick one stat and keep buying it.
#[derive(Default)]
pub struct Bot {
    chosen_stat: Option<usize>,
}

impl Bot {
    fn buy_items(&self) -> Result<()> {
        // Spend only on power shards of the randomly chosen stat.
        let me = me()?;
        let item = [Item::PowerShardOfStrength, Item::PowerShardOfFocus, Item::PowerShardOfHealth][self.chosen_stat.unwrap()];
        let count = me.gold_count()? / item.cost()?;
        if count > 0 && me.can_buy(item, count)? { me.buy_count(item, count)?; }
        Ok(())
    }

    pub fn play_turn(&mut self) -> Result<()> {
        if self.chosen_stat.is_none() {
            self.chosen_stat = Some(crate::random_number(3) as usize);
            println!("draft: {}", String::from_utf8_lossy(&cambc::draft_memory().bytes()[..11]));
        }
        // Read before writing: little-endian u32 at shared-memory byte offset zero.
        let mut bytes = [0; 4];
        cambc::shared_memory().read(0, &mut bytes)?;
        println!("shared round: {}", u32::from_le_bytes(bytes));
        cambc::shared_memory().write(0, &cambc::round_number()?.to_le_bytes())?;

        // Priorities: shop and recall, attack enemies in range, bank gems, then farm.
        let me = me()?;
        if !me.is_alive()? || me.is_recalling()? || me.is_submitting_gems()? || me.is_teleporting()?
            || (!me.can_move()? && !me.can_attack()?) { return Ok(()); }

        let at_spawn = cambc::map::spawn_region(Team::Ally)?.is_set(me.position()?)?;
        if at_spawn { self.buy_items()?; }

        if !at_spawn && me.health()? < me.max_health()? * 0.35 {
            me.recall()?;
            return Ok(());
        }

        for enemy in cambc::vision::champions(Some(Team::Enemy))? {
            if attack_if_in_range(&enemy)? { return Ok(()); }
        }

        if me.gem_count()? > 0 {
            // Union all altar approach tiles: the field chooses the nearest reachable altar.
            let mut goals = Bitboard::new()?;
            for altar in cambc::map::altars()?.positions()? {
                if cambc::vision::has_altar_lock(altar)? { continue; }
                let range = cambc::constants().submit_gems_range;
                if distance(me.position()?, altar) <= range { me.submit_gems(altar)?; return Ok(()); }
                goals.union_assign(&tiles_around(altar, range)?)?;
            }
            move_towards_closest(goals)?;
            return Ok(());
        }

        farm_camps()?;

        cambc::self_champion()?.expect("gameplay champion").set_priority(1 + crate::random_number(100) as i32)
    }
}

