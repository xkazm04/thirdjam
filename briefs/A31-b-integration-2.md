# Lane A31: device B's asks, round 2: figure class per tier, numbers, glass, stanchions, shot calls, goalie stance, broadcast tier, pickup props (device A, from H11.4)

You are lane A31 of the Third Jam (team Hokej). Ruling R-0027 point 4. Read first: `handoff/B-to-A-0021.md`, `0023.md`, `0024.md`, `0025.md`, `0019.md` (on main after the dev-b merge
the Director makes before you start; if a file is missing, read it with the read-only `git show origin/dev-b:<path>`), `contracts/look-tiers.md`, `contracts/skater-visual.md`
(C-0001 v3), `Game/Source/JamGame/Private/{HokejGameMode,RinkActor,JamPawns,LookTierSwitcher,JamSkaterVisual}.cpp`, `Game/Source/JamCore/AI/JamAICore.cpp` (the brain: shots and the
last-man goalie), `Game/Source/JamCore/Effects/JamPickupActor.cpp`, R-0021 to R-0027.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a31` (detached at the merge commit; build it yourself, unity off; editor on MCP port 8017 only while proving, at most
three editors on the machine). Run no git command; never touch `Game/Plugins/JamAssets`; the sim's hash never changes for cosmetic work (`Jam.Bot.VisualSwap`, `Jam.Core.Determinism`).

You own: `HokejGameMode.cpp` (visual factory, numbers, loops), `JamPawns.cpp` (the calls to the visual), `RinkActor.*` (glass, stanchions, floor, ice params), `LookTierSwitcher.*`,
`JamAICore.cpp` (shot charge and goalie stance only), `JamPickupActor.*`, `schema/` (tier value), `Game/Config/DefaultGame.ini` (cook line), `docs/experiments/B-ASKS-2.md`, `status/A31.md`.

## Job, in the order of effect on the picture
1. **0021**: `ResolveVisualClass`: `generated_b2` picks the class by `game.look.tier` (`p3_clay` -> `BP_SkaterVisual_TB2_P3`, `p4_broadcast` -> `_TB2_P4`, else `_TB2`; load fails ->
   `_TB2`, then the toy); a tier change mid-match respawns the visuals with the setup calls repeated. The pawn calls `SetNumber` once at spawn: team 0 gets 7, 13, 42, team 1 gets
   9, 21, 88, the human's skater keeps its slot's number. `SetTeam(TeamId, Colour)` as today.
2. **0024**: `PlayShoot` fires only on a shot (never with a pass); bots charge their shots (the brain holds `bShoot` for a charge window at pro and all_star: 10 and 18 frames,
   rookie taps; the sim's charge rule applies to every skater); `SetGoalieStance(bOn, Side)` is called by the game mode for the last man in the crease from the brain's intent
   (the brain exposes `GoalieSide` -1/0/+1 in its output). Re-run the curve gates after the charge change (Jam.Bot, Jam.Slow.DifficultyCurve): if the curve moves more than 5
   points, report the numbers and leave the profiles alone (A3's worktree is the place for a re-tune).
3. **0023**: the floor actor Movable (or hidden when a rig loads) so the -20 cm lowering happens; the glass component uses `/JamAssets/Look/MI_Glass` (`MI_Bcast_Glass` on the
   broadcast tier); `RinkActor` spawns the glass stanchions with the glass (it does already: confirm the 3.6 m spacing and tell B to remove its 66 `Con_` stanchions);
   `RimStrength` 1.3 on the toy figure's instances; `CornerRadiusCm` driven on the dynamic ice instance from the rink config.
4. **0025**: tier value `p4_broadcast` (schema, limits, README, DEFAULTS, `GetKnownTiers`, settings label "BROADCAST (P4)"), map path `/JamAssets/Rigs/LR_P4_Broadcast.LR_P4_Broadcast`,
   cook line, `look.trim_emissive` default 30, `SetTier` resets `MPC_Look.Clay` to 0 before streaming a rig. Measure the broadcast rig's frame cost on the 4090 (ProfileGPU at
   1440p, one editor) and the tier switch at runtime between all four tiers (a Director's Call in PIE), and the rig scale on the large rink.
5. **0019**: pickups: B's `SM_Pickup_<kind>` props at `PickupTop`, stencil 1, no collision, spun 90 deg/s and bobbed 6 cm in code (sim frame, not delta time), the token as the
   fallback; the rig check B asks for: a still of `hockey.regular` and `hockey.arena.large` from the game camera, no `Dress_` actor inside the boards, floor at -20 cm.
6. Proof: PIE stills per item (`devlog/shots/`), the eyes `bug-hunt` on the tier strip (night, clay, broadcast at the same frame), Jam.Bot and Jam.Core green in the worktree.
   `docs/experiments/B-ASKS-2.md`: what each handoff gets as its answer (the Director writes the files), the frame costs, the file list.

Report: the answers per handoff, still paths, costs, file list. Time box 2 hours; interim at 45 minutes with items 1 and 2.
