# Lane A150, track Q3 (performance at five a side), evolution 5 (device A, Opus)

You are lane A150 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then the track's docs: `docs/experiments/PROFILING-5.md` (the warm scene behind the menu, the arena preload, the budget table), `status/A137.md`, the tools under
`tools/perf/` (`bot.ps1`, `ab.ps1`, `firstsec.mjs`, `five.mjs`, `syncrows.mjs`, `loadrow.mjs`), and what changed under the match start tonight:
`docs/experiments/MENU.md` (a new first screen: PLAY NOW starts `hockey.regular`, which now plays on the **clay** tier, not night) and
`docs/experiments/ARENA.md` sections 38 to 48 (four new arena meshes, a flash light on a board hit, a wave of lights on a goal).

Your worktree and the commit it stands on are named in the Director's message that starts you: the track's worktree, moved to the newest main. It was built
before: an incremental build is enough; then one Development package (start it first: it is the long pole; wait out an AutomationTool already running by
polling inside one command: the Director's release chain packages tonight too). Your status file: `status/A150.md`. Port 4706 (the track's). Family as the
track's: `EffectsBinder.*`, the match-start path in `HokejGameMode.*` / `RinkActor.*` / `MenuFlow.*` (start-up order only), `DefaultEngine.ini` rendering and
loading settings, `tools/perf/**`, `docs/experiments/PROFILING-6.md`.

## Evolution 5: the operator's first match starts clean
The operator plays at 10:00 from a package cut by 09:00. His first press is PLAY NOW; his first match is the regular cut on clay, five a side.
1. **The warm scene follows the new first match**: the menu's default tier guess must be the tier PLAY NOW will play (clay now), and the warm scene must hold
   what that match draws in its first second: the rig, the rink's materials, **a figure of each team, the puck, the HUD's first widgets, the arena's meshes
   and its lights** (your next-1). Bar: the first rendered frame under 33 ms in 6 of 6 runs a tier (today one frame of 40 to 58 ms remains in 10 of 12), the
   precache misses under 10, on a package, through PLAY NOW's own path (`jam.menuscreen play` or the Return key posted to your own window: never the system
   cursor).
2. **The load frame under 100 ms in 6 of 6 runs a tier** with everything of tonight in (11 of 12 with the warm scene; the arena's new meshes are in the
   preload list as of the arena lane's landing: confirm 0 synchronous loads at match start with `syncrows.mjs` after fixing its off-by-60 skip at line 16,
   which hides the match-start frame; `T_UI_ScorePlate` and `/Water/Materials/MPC/MPC_Water` still load synchronously there: find what asks for them).
3. **The board-hit flash and the goal's wave in a package**: their GPU rows in a 5 v 5 match (bars: the flash 0.1 ms or under; no frame over 33 ms on a goal
   or a hard hit), on both tiers.
4. **A Shipping player's first launch**: the engine saves its user pipeline cache in Shipping only; say what a first launch of the Shipping zip does on a
   clean profile (the first match's worst frame) against a second launch, if the release chain's Shipping package exists when you get there (its folder is
   named on the last `package` line of `status/A.md` in the main checkout: read it, never write there).

Six runs a tier per arm, interleaved, one session; say the machine's load. The gate: the full `Jam` filter green, the hash unchanged; each change behind a
console variable that gives back the old start. Time box 110 minutes; interim in the status file.
