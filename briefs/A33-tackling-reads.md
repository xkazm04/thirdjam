# Lane A33: tackling that reads (device A, from H11.5, Fable 5.2)

You are lane A33 of the Third Jam (team Hokej). R-0025 point 1 and R-0028 point 3: P2 ships playable with tackling that reads. The sim already has body checks, pokes, lifts, fouls
(R-0013), knock-downs and the strip on a check; what is missing is that a player *sees and feels* the hit. Read first: `docs/experiments/FEEL.md` (the numbers), `contracts/skater-visual.md`
(C-0001 v3: `PlayCheck`, `SetKnockedDown`, `PlayFoulReaction`, `PlayPoke(bClean)`), `contracts/events.md` (C-0003: `check_landed`, `check_received`, `foul`, `poke`, `lift`, `board`),
`Game/Source/JamCore/Private/JamSim.cpp` (the check resolution: `TryCheck`, the knock-down window, `CollideSkaterStatic` for the boards), `Game/Source/JamGame/Private/{JamPawns,
SkeletalSkaterVisual,EffectsBinder,HokejCamera*}.cpp`, device B's `status/SK2-report.md` (the four figures' takes: knocked_down, hit reactions), `docs/experiments/P4-MOTION.md`
(the falls group on the Manny, pelvis heights), `tools/eyes/README.md`, R-0021 to R-0028.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a33` (detached at the merge commit; build it yourself, unity off; editor on MCP port 8020 only while proving; at most
three editors on the machine). Run no git command; never touch `Game/Plugins/JamAssets`; the sim's state hash never changes for cosmetic work (Jam.Core.Determinism, Jam.Bot.VisualSwap);
sim changes that do change play are calibrations (R-0021 point 1) and are recorded in FEEL.md with the Determinism baseline re-recorded.

You own: `JamPawns.cpp` (the reaction calls), `SkeletalSkaterVisual.*` (reaction selection by direction), `JamSim.cpp` (only: the check's direction and strength in the event
payload, a `board` event on a hard board collision, a stumble on a mistimed poke if it does not exist), `EffectsBinder.cpp` (the new Variant values), the camera's punch
(`HokejCamera*`), `docs/experiments/TACKLING.md`, `status/A33.md`.

## Job
1. **Direction and strength in the hit**: `check_landed` / `check_received` carry `Kind` = `front | side_l | side_r | behind` and `Strength` 0..1 (closing speed over the max); the
   visual picks knocked_down_front / side / behind (B's takes and A26's sequences both have them) and a hit reaction without a fall below the knock-down threshold; the fouler's
   `PlayFoulReaction` on a foul. The checked skater's puck leaves with a visible pop (the sim already strips it; the puck gets a 0.3 s lift in the kit, cosmetic).
2. **Boards**: a `board` event when a skater hits the boards above 4 m/s (sim, read-out only), `NS_board` / `MS_board` through the binder with `Strength`, the figure's
   `hit_in_back_skating` or a lean-into-boards pose for 12 frames, a glass wobble if B's glass material has a parameter for it (ask by status, do not edit the material).
3. **Camera punch**: a 6-frame camera kick on a check the human lands or takes (2 cm translation, 0.6 deg roll, decays), none on the AI's checks out of frame; a short
   `SetTimeDilation`-free hit stop is forbidden (fixed step); instead a 3-frame freeze of the *visual* only if it reads better (judge).
4. **Stumble**: a mistimed poke stumbles the poker for 20 frames (R-0013 says it does): make it visible (the figure's stumble take; the stick drops low).
5. **Proof**: a scripted scene (two bots told to collide at centre ice at three closing speeds, both directions) captured as 8-frame strips and a 10-second clip; eyes
   `bug-hunt` and `readability` at 24 m; FEEL.md gets the numbers (thresholds, frames) and a before/after pair. Determinism and VisualSwap green.
6. `docs/experiments/TACKLING.md`: the event payloads, the reaction table (direction x strength -> take / effect / sound / camera), what B's figures need (if anything: a handoff
   draft text for the Director).

Report: the reaction table, the strip and clip paths, the eyes verdicts, the file list. Time box 2 hours; interim at 45 minutes with a check landing on camera.
