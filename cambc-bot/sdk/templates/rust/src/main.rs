// Dispatch draft and gameplay. Rust propagates SDK errors explicitly.
mod draft;
mod bot;
use std::cell::Cell;

thread_local! {
    static RANDOM: Cell<Option<u64>> = const { Cell::new(None) };
}

// WASI randomness is always zero in matches, so seed from our spawn and this champion's start.
fn random_seed() -> u64 {
    let spawn = cambc::map::spawn_region(cambc::Team::Ally).ok().and_then(|region| region.positions().ok()?.first().copied());
    let start = cambc::self_champion().ok().flatten().and_then(|me| me.position().ok());
    [spawn, start].into_iter().flatten()
        .fold(0u64, |seed, p| seed.wrapping_mul(1_000_003).wrapping_add((p.x as u64) << 16 | p.y as u64))
}

fn random_number(limit: u32) -> u32 {
    RANDOM.with(|random| {
        let state = random.get().unwrap_or_else(random_seed)
            .wrapping_mul(6364136223846793005).wrapping_add(1442695040888963407);
        random.set(Some(state));
        ((state >> 33) % u64::from(limit)) as u32
    })
}

fn main() -> cambc::Result<()> {
    let mut bot = bot::Bot::default();
    loop {
        let result = if cambc::is_draft() {
            draft::play()
        } else {
            bot.play_turn()
        };
        if let Err(error) = result {
            println!("bot error: {error}");
        }
        cambc::end_turn();
    }
}
