# Lane A47: device B's end-to-end play, the A-side faults fixed in rank order; binder round 3 (device A, from H12.7)

You are lane A47 of the Third Jam (team Hokej). Device B played the joined game end to end (`status/QA1-report.md`, handoff `B-to-A-0030.md`) and ranked the faults on A's side by what
a player sees first; B also rebuilt the contact effects (`B-to-A-0029.md`) and the six newer events' sounds (`B-to-A-0031.md`) and asks for small binder changes. Read those three
handoffs first (on main after the merge of 22:35), then `docs/experiments/COMMENTARY.md` (A32's rules, its own open point 1), `docs/experiments/UI.md` (A18's cards, A42's pass),
`Game/Source/JamGame/Private/{EffectsBinder,HokejGameMode,HUDWidget,DirectorsCallWidget,CommentarySystem,CameraFollow,JamPawns}.cpp`, `Game/Source/JamCore/Effects/JamPickupActor.cpp`,
`Game/Content/Cartridges/hockey.boost.cartridge.json`, R-0021 point 3, R-0031, `tools/eyes/README.md`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a47` (the Director creates it at the merge commit; build it yourself, unity off; one editor on MCP port 8026 only
while proving; at most three editors on the machine). Run no git command; never touch `Game/Plugins/JamAssets`; presentation and binder work never changes the sim hash; the one sim
read-out you add (contact normal vectors) is a read-out.

You own: `EffectsBinder.*`, `HUDWidget.*` + `DirectorsCallWidget.*` (the card and the label), `CommentarySystem.*`, `HokejGameMode.cpp` (`BuildMatch` visual teardown, the match
card), `CameraFollow.*` (post-process settings only), `JamPawns.cpp` (own-goal colour), `JamPickupActor.cpp` (the shape), `JamSim.cpp` (read-out vectors only), the boost cartridge's
look tier, `MenuWidget.cpp` (the cut cards' rink size text), `docs/experiments/QA1-FIXES.md`, `status/A47.md`.

## Job, in B's rank order
1. Motion blur off from A's side too: if the follow camera carries post-process settings, set motion blur 0 there; confirm in PIE that no turn smears.
2. Hockey on the ice (R-0021 point 3): the faceoff card is a plain banner ("MATCH 1", "PERIOD 1 - FACEOFF") without the clapperboard art (use A18's plain panel), and the standing
   label becomes "CALL READY" (or the team's call name) instead of "DIRECTOR'S CALL READY"; the menu keeps the film words.
3. Commentary: a bark never cuts a line under 0.5 s old (it waits for the line or is dropped); must-play lines (goal calls, priority 5) cut a playing lower-priority line; the
   `when` conditions of the bank are applied (B says they are not: find out why on `board.bank.*` and fix); the caption clears when the line ends. Keep A32's tests green and add
   one for each rule.
4. `hockey.boost` runs on `l2_night` (the cartridge's look tier), unless the Designer library's `base_sha256` guard needs a regen: say so and run `gen-designer-data`.
5. Ghost figures: `BuildMatch` destroys every skater's visual before the skater on a rebuild (A31's `ResetVisual` path), proven by three `jam.livecut` rebuilds leaving no actor
   behind (count `BP_SkaterVisual_*` actors in the log).
6. Own goal bursts in the scoring team's colour; the goal-light box sits on the glass line (A25's placement); the menu cards print the cut's own rink size; `JamPickupActor` asks
   for `/Engine/BasicShapes/Cylinder`.
7. Pro bots' taps: A31 set `ShotChargeFrames` pro 10, yet QA1 saw charge 0.00 to 0.07; find why the hold does not reach the sim (the adapter, the playtester path or the aimed
   check) and fix; the slap wind-up must show in a default pro match.
8. Binder round 3 (0029, 0031): `User.Dir` on `check` and `knockdown` (the contact normal, world x/y, z 0; the sim exposes it as a read-out), `User.Normal` on `board` (into the rink),
   blade spray every 12 frames; `carry` silent; `period_end`, `clock`, `season_take` sound-only and 2D; `clock` Kind `sudden_death` -> Variant 1.
9. Proof: a 60-second bot match in PIE with stills of the plain banner, a check burst along the hit, a board burst off the board, a goal call that was not cut, three rebuilds with
   no ghosts, a pro bot's wind-up; the eyes `bug-hunt` on the stills; Jam.Commentary, Jam.Effects, Jam.Core, Jam.Bot green.

Report: the fault table with the fix per row, the still paths, the file list. Time box 90 minutes; interim at 45 minutes with rows 1-5.
