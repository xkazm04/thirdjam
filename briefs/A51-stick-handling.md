# Lane A51: advanced stick and puck control with the mouse, and a spin move on E (device A, from H13.2, Fable 5.2)

You are lane A51 of the Third Jam (team Hokej). The operator, after H19 (controls that work, lane A49), asks: "use the resources for advanced stick/puck control and tests with
the mouse so player can use mouse and movement to get past defenders, special move like spin binding to keyboard shortcut like 'E' to extend variety of gameplay options."
The Director's decisions (R-0034): the spin goes on **E**; the pass moves to **F** (gamepad: spin on B, pass stays on X); the mouse sweep is a real mechanic in the sim,
not an animation; everything is proven in a packaged Development Game build through the real input path, as A49 did.

Read first: `docs/experiments/CONTROLS.md` (A49: the input path in a package, `CursorAimRink`, `SmoothAimDeg`, the SendInput harness `tools/qa/ctrl/*` and `a49-analyze.mjs`,
the drill), R-0013 (the mouse-aimed stick), R-0029 point 4 and `docs/experiments/PAYOFFS.md` (A39's deke: `bDeke`, `DekeFrames`, `DekeSide`, `DekeReadyFrame`, the
`game.skater.deke` key), `Game/Source/JamCore/Private/JamSim.cpp` (`Stick`, the carried puck's offset on the blade, `ResolvePokes`, `TryLift`, `TryCheck`, the strip rule,
`TryShoot`), `Game/Source/JamCore/AI/JamAICore.cpp` (how a defending bot aims its poke and its check at the carrier), `Game/Source/JamGame/Private/InputMapping.cpp`
(`IA_Deke`, `IA_Pass`, `IA_Sprint`; the contexts are built in code), `HokejGameMode.cpp` (`ReadHumanInput`, `GatherInputs`), `JamPawns.cpp` (`SetStickAim`, the puck's visual on
the blade), `Game/Source/JamNet/Public/JamNetPackets.h` (flags are 16 bits; Sprint = 256 is the last used bit; the next free is 512), `docs/experiments/FEEL.md`,
`docs/experiments/TACKLING.md`, `Game/Source/JamCore/AI/JamPlaytester.cpp` (the three player classes drive the real input path), `schema/game.schema.json`, `limits.json`,
`DEFAULTS.md`, `README.md`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a51` (the Director created it at main bb48ee7; build it yourself, unity off; one editor on MCP port 8029 only
while proving; at most three editors on the machine). Run no git command; never touch `Game/Plugins/JamAssets`; never print `.env`. Lane A50 works at the same time on
`JamSim.cpp` (the goal test for a lifted puck, `goal_height_m`), `JamAICore.cpp` (the goalie stance, the aim height), `JamPawns.cpp` and `HokejGameMode.cpp` (the goalie
stance for the human, `ResolveVisualClass`): keep your edits in their own functions and blocks, away from those, so the Director's three-way merge stays clean.

You own: the stick-handling part of `JamSim.cpp/.h` (the sweep, the spin, the puck's offset, the poke resolution against the offset), the defending bot's poke timing in
`JamAICore.cpp`, `InputMapping.*` (E spin, F pass, gamepad B spin), `ReadHumanInput` in `HokejGameMode.cpp`, `JamPawns.cpp` (the spin's figure call through the existing
interface: a yaw turn of the visual over the spin's frames, no new interface event), `JamNetPackets.*` (the spin flag bit 512, protocol 5, `InputCoverage`), `JamPlaytester.cpp`
(the regular and good classes use the sweep and the spin), `schema/` + `JamCartridge.*` for the new keys, `tools/qa/ctrl/a51-*`, the controls card text in the pause menu,
`docs/experiments/CONTROLS.md` (a new section), `docs/experiments/STICK.md`, `status/A51.md`.

## Job, each step proven in a packaged Development Game build with SendInput (A49's harness), with a test in the sim
1. **The puck lives where the blade is.** The carried puck's offset follows the mouse aim every frame (not only on a deke tap): the blade's reach is `game.stick.reach_m`
   (default 1.1, range 0.6..1.6), the puck sits at the blade's position on the ice, forehand to backhand across the body as the cursor sweeps. A fast sweep (the aim moving
   more than `game.stick.sweep.min_deg_per_frame`, default 6) counts as a **sweep**: for `sweep.shield_frames` (default 12) a poke from the side the puck left misses.
   A39's tap deke stays (Q, wheel) as the short-hand that does the same in one tap.
2. **Beating a defender is real.** A defending bot pokes at the puck's position `bot_react_frames` ago (`game.stick.sweep.bot_react_frames`: rookie 14, pro 8, all_star 5 by the
   difficulty profile), so a sweep timed inside the reach of its stick beats the poke, and a slow carrier is poked as before. A bot's check still works on the body (the
   sweep never defends a body check: that is the sprint's and the spin's job). Test `Jam.Core.SweepBeatsPoke`: a scripted carrier sweeping at frame N against a pro defender
   in poke range keeps the puck 10 of 10 seeds; a carrier that does not sweep loses it 10 of 10.
3. **The spin move on E** (gamepad B): `game.skater.spin { frames 24, cooldown_frames 90, speed_mult 0.7, stamina_cost 0.25 }`: a 360-degree turn over the frames, the puck
   carried on the far side of the body from the nearest opponent each frame, the carrier's speed scaled by `speed_mult`, the stamina drained by `stamina_cost` (the spin is
   refused with an empty bar or inside the cooldown). A body check that lands during the spin knocks the carrier down for 1.5x the normal frames (risk against reward).
   A poke during the spin never reaches the puck (the body is between). Tests `Jam.Core.SpinShieldsPuck`, `Jam.Core.SpinCooldown`, `Jam.Core.SpinCheckedFalls`.
4. **Bindings**: E spin, F pass (remove E from the pass), gamepad B spin; the pause menu's controls card and `CONTROLS.md`'s table updated; a `JAMCTRL spin` and `JAMCTRL sweep`
   log line per event in the packaged build.
5. **The wire**: `FJamNetQuantInput::EFlag::Spin = 512` (edge), `JamNet::Version = 5`, `InputCoverage` and `PacketRoundTrip` and `LockstepTwoPeers` green.
6. **The playtester** regular and good classes use the sweep when a defender is within 2.5 m in front and the spin when a defender closes from behind at speed; the curve
   (R-0029: novice 80/85 at rookie, regular 50/55 at pro, good 30/25 at all_star) is re-measured with `tools/playtest/run.mjs`; if it moves more than 10 points at any class,
   say so with the numbers and do not re-tune (A3 owns the curve).
7. **Proofs in the packaged Development build with SendInput**: a 1-on-1 drill (A49's drill or a new `jam.drill 1v1`) where the mouse sweeps past a pro defender ten times
   (count the kept pucks from the log), ten spins with the puck kept and the stamina bar visible, one spin checked mid-turn with the knock-down; a 10-second capture that
   the puck's visual sits on the blade through a sweep (stills at 4 frames apart; eyes `bug-hunt` on three of them). Sim hash changes are calibrations: re-record the
   determinism baseline and write the before/after in `STICK.md`.
8. `docs/experiments/STICK.md`: the model (reach, sweep, shield, bot reaction, spin), the keys with their defaults and ranges, the tests, the proofs, and what you would do
   next for variety (a toe drag, a between-the-legs, a saucer pass) with the frames each needs.

Report: the file list, each test with its result, the proof paths, the curve numbers before and after, any protocol or hash change. Interim at 40 minutes with steps 1-3 in
the sim; time box 100 minutes.
