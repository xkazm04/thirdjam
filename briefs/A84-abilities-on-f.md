# Lane A84: special abilities on F, granted by a Director's mode or a boost; the pass moves to Q (device A, from H18, R-0041 point 3, Opus)

You are lane A84 of the Third Jam (team Hokej). The operator (R-0041): "Director's mode then can produce via boosts fun aspects like one-shot shotgun, binding special
abilities to 'F' key." Your job: the ability system, cartridge-defined, one ability held at a time, used with F; four abilities that play; the pass on Q; the HUD slot and
the bark; proven headless and in a package.

Read first: R-0041 (points 3 and 4: the teen rating: rough, not cruel), `docs/experiments/MODES.md`, `MODE-KEYS.md`, `MODES-2.md` if lane A81 has written it (its asks name the
abilities the modes want), `Game/Content/Cartridges/hockey.boost.cartridge.json` (pickups: the sim's pickup rule and `JamPickupActor`), `Game/Source/JamCore/Private/JamSim.cpp`
(pickups, `TryShoot`, `TryCheck`, the knockdown, the lifted puck `Z`, the sprint and stamina, the carrier rule), `JamSim.h` (`FJamInput`: the pass is `bPass` + `bPassHeld`
since A77), `Game/Source/JamGame/Private/InputMapping.cpp` (F = pass, Q = deke's second key, the wheel = deke), `HokejGameMode.cpp` (`ReadHumanInput`), `HUDWidget.cpp`
(UI.md section 8: the table of every HUD element), `CommentarySystem.cpp` + `tools/commentary/` (a bark per ability use), `Game/Source/JamNet/Public/JamNetPackets.h` (the
next flag bit after PassHeld 1024 is 2048; the protocol bumps to 7 with the checklist there), `schema/`, R-0034, R-0037, R-0040.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a84` (the Director creates it at the commit that lands A77 and A82: the saucer, the lift shield, the carrier
speed; build it yourself, unity off; the Development game package with `-WaitMutex`; one editor on MCP port 8042 only for a HUD still, closed after). Run no git command;
never touch `Game/Plugins/JamAssets` (a burst or sound for an ability is B's: ask in the report with the event name); never print `.env`; never kill a process you did not
start. The eyes tool is out of credits. A sim change is a calibration with the hash before and after; every existing test stays green; the abilities are off in every
cartridge that does not name them, so the default game and the curve do not move.

You own: `game.abilities[]` in `schema/` + `JamCartridge.*` (`{{ id, trigger: mode|pickup, cooldown_frames, params... }}`), the ability rule in `JamSim.cpp/.h` (held ability,
`bAbility` input, cooldown, the four effects), `InputMapping.*` (F = ability, Q = pass, the wheel keeps the deke; gamepad: X = pass stays, the ability on the left trigger's
button or D-pad down: say which), `HokejGameMode.cpp` (`ReadHumanInput`), `JamNetPackets.*` (`Ability` = 2048, protocol 7, `InputCoverage`), `HUDWidget.cpp` (an ability slot
beside the stamina bar: icon or name, the cooldown ring), `CommentarySystem` bank (one bark per ability, teen-rated), two mode cartridges that grant an ability
(`modes/hockey.mode.shotgun.cartridge.json`, `hockey.mode.arsenal.cartridge.json`: boosts grant a random ability), `Game/Source/JamCore/Private/Tests/JamAbilityTests.cpp`,
`docs/experiments/ABILITIES.md`, `docs/experiments/img/a84/**`, `status/A84.md`.

## Job
1. The system: a cartridge grants an ability by mode (held from the drop, cooldown) or by pickup (a boost kind `ability` with the id; held until used); F uses it; the HUD slot
   shows it and the cooldown; the bark plays; `Jam.Core.Abilities` (grant, use, cooldown, pickup, none when unnamed).
2. Four abilities (each a rule in the sim, each under the teen rating): **shotgun** (the next shot leaves at 1.5x speed and knocks down every skater within 1 m of its path:
   the knockdown is the existing one, Strength 1), **hip check** (the next check lands from 0 m/s own speed with 1.5x knockdown frames and a 3 m push), **shield** (6 s during
   which checks and lifts on the carrier fail; the body still bumps), **slow puck** (the next pass or shot travels at half speed for 2 s: a trick play). Each named in a mode or
   a pickup; each proven headless (a scripted scene per ability) and once in the package through the real input path (F pressed with the puck; the `JAMCTRL ability` line).
3. The pass on Q: `InputMapping`, the controls card, `CONTROLS.md`'s table; the saucer hold moves with it (a held Q); the gamepad pass stays on X.
4. The two modes: Shotgun (the ability held from the drop, cooldown 20 s, pro) and Arsenal (boost pickups grant a random ability, rookie); `tools/playtest/modes.mjs` with a
   character number each (knockdowns per match; abilities used per match).
5. `ABILITIES.md`: the shape of `game.abilities[]`, the four rules, the tests, the proofs, the asks for B (a burst and a sound per ability), six more ability ideas.

Report: the tests, the proof lines, the hash, the file list. Interim at 45 minutes with steps 1-2 in the sim. Time box 120 minutes.
