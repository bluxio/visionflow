//! Engine settings queried at runtime, plus parameters without Game API getters.
use crate::{sys, Result};

/// Immutable engine rule parameters, loaded by `cambc::init`; matches `Game_Settings`.
#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct Constants {
    /// Engine capture duration.
    pub capture_time: i32,
    /// Camp sleep duration.
    pub camp_sleep_time: i32,
    /// Champion respawn duration.
    pub respawn_time: i32,
    /// Recall channel duration.
    pub recall_time: i32,
    /// Per-gem submission-duration multiplier.
    pub submit_gem_duration_multiplier: f64,
    /// Base gem-submission duration.
    pub submit_gem_base_duration: i32,
    /// Minimum gem-submission duration.
    pub submit_gem_minimum_duration: i32,
    /// Allowed range for submitting gems to an altar.
    pub submit_gems_range: i32,
    /// Maximum match length in rounds.
    pub max_rounds: u32,
    /// Submitted-gem count required to win.
    pub winning_gems: u32,
    /// Gold reward for a champion kill.
    pub kill_gold: u32,
    /// Gold reward for a kill assist.
    pub assist_gold: u32,
    /// Passive gold income parameter.
    pub passive_gold: u32,
    /// Healing parameter for champions in their spawn region.
    pub spawn_healing: f32,
    /// Maximum active ordinary wards per champion.
    pub active_wards: u32,
    /// ABI-only field, not part of the bot-facing rules.
    pub(crate) ward_charges: u32,
    /// Team shared-memory capacity in bytes.
    pub shared_memory_bytes: u32,
    /// Draft-memory capacity in bytes.
    pub draft_memory_bytes: u32,
    /// Number of rounds revealed tiles remain known.
    pub tile_reveal_rounds: u32,
    /// Ordinary-ward resource cooldown.
    pub ward_cooldown: i32,
    /// Ordinary-ward lifetime.
    pub ward_lifetime: i32,
    /// Ordinary-ward vision radius.
    pub ward_vision_radius: f32,
    /// Ordinary-ward placement radius.
    pub ward_placement_radius: f32,
    /// ABI-only engine setting, not exposed by the bot-facing API.
    pub(crate) control_ward_vision_radius: f32,
    /// ABI-only engine setting, not exposed by the bot-facing API.
    pub(crate) control_ward_placement_radius: f32,
    /// Hits required to break an altar lock.
    pub altar_lock_hits: u32,
    /// Retired teleport-scroll channel duration; unused.
    pub(crate) tp_scroll_time: i32,
    /// Teleport-crystal channel duration in rounds.
    pub tp_crystal_time: i32,
    pub(crate) reserved: u32,
}

const _: () = {
    assert!(core::mem::size_of::<Constants>() == 120);
    assert!(core::mem::align_of::<Constants>() == 8);
    assert!(core::mem::offset_of!(Constants, submit_gem_duration_multiplier) == 16);
    assert!(core::mem::offset_of!(Constants, tp_scroll_time) == 108);
    assert!(core::mem::offset_of!(Constants, tp_crystal_time) == 112);
    assert!(core::mem::offset_of!(Constants, reserved) == 116);
};

/// Read the exact engine settings record at runtime, preserving host failures.
pub(crate) fn read_constants() -> Result<Constants> {
    // SAFETY: the host assigns the whole settings record on success.
    unsafe { crate::query_init(|p| sys::game_settings(p)) }
}

/// Passive-ability constants mirrored from SDK balance values.
impl Constants {
    /// Prism focus gained per carried gem; no Game API getter exists.
    pub const PRISM_FOCUS_PER_GEM: f32 = 0.2;
    /// Varg passive splash radius; no Game API getter exists.
    pub const VARG_SPLASH_RADIUS: f32 = 1.0;
    /// Vesper Vale passive bonus-gold percentage; no Game API getter exists.
    pub const VESPER_VALE_BONUS_GOLD_PERCENT: u32 = 20;
    /// Ryth passive health/strength ratio; no Game API getter exists.
    pub const RYTH_HEALTH_STRENGTH_RATIO: f32 = 0.01;
    /// Ren retaliation focus ratio; no Game API getter exists.
    pub const REN_RETALIATION_FOCUS_RATIO: f32 = 0.2;
}
