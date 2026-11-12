# SK4 checkpoint (after item 2): falls and collisions on the scripted figure (device B)

Lane SK4, 2026-11-12, started 18:57, checkpoint about 19:45. Main tree, editor on port 8000, no git, no build. `/Game/Maps/Rink` open in memory with
test actors `SK4_*` (never saved). Another editor (port 8002) shares the GPU: no frame numbers. Scratch:
`C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/sk4` (`$SK4`). Final report follows in
`status/SK4-report.md`. Verified in PIE, the real game (`jam.Cut` = `$SK4/b1.cartridge.json`, `visual: humanoid_b1`, score_to_win 9).

## Item 1: import (done on all four figures)

- T-B1: all 70 takes from `skf/export4/p2_cel/SK_Skater_p2_cel_loopcut.fbx`, animation-only onto `SKEL_SkaterTB1v2` (55 replaced in place, 15 new).
- R, P3A, P4B: per take as SKH's sixth pass says: `Knockdown_Left/_Right`, `Fall`, `Fall_Forward`, `Board_Pin` from `skh/export6/<c>/`, all the rest from
  `skh/export5/<c>/` (loopcut). Not imported (not good enough, not wired): `Knockdown_Back`, `Trip` on all three; `Board_Hit`, `GetUp_Side_Left` on P3A.
- P4B `Stride`: measured in the engine: loopcut wrap hand / blade 2.48 / 7.78 cm (mean step 2.27 / 4.17), main file 0.00 / 0.00 (its last key repeats
  the first). The main file's take is the smaller wrap and is now `AS_SkaterTB2_P4_Stride`.
- **Cook error fixed:** `BS_SkaterTB2_P4_Skate` had its two `Stride` samples (speed 5 and 8, turn 0) with no animation; both now point at
  `AS_SkaterTB2_P4_Stride`. The other three skate blend spaces and the four aim blend spaces list every sample with its take (14 and 24 samples, none empty).

## Item 2: `BP_SkaterVisual_TB1`: PASS

**Direction convention (verified in code and in PIE):** `(DirX, DirZ)` is the direction the victim is pushed, i.e. where it falls to. In
`UJamSim::CollideSkaters` it is the contact normal from the checker to the victim; a check "from behind" is `Dot(Dir, VictimForward) > 0.5`. The take
names say where the hit came from: `Knockdown_Left` = hit from the left = falls to its right (engine FK: head ends 45 cm to the figure's right).
Test figure facing +X: `SetKnockedDown(true, 0, 1)` (push to its right) played `Knockdown_Left` and the figure lies to the right; `(0, -1)` played
`Knockdown_Right`, lies to the left; `(1, 0)` `Knockdown_Back` (face down ahead); `(-1, 0)` `Fall` (on its back); `(0.6, 0.8)` `Knockdown_Left`.

**Picking:** f = push · facing, r = push · right. f ≥ 0.707 → `Knockdown_Back`; f ≤ -0.707 → `Fall`; else r > 0 → `Knockdown_Left`, r < 0 →
`Knockdown_Right`. The mesh turns over 6 frames so the fall runs exactly along the push (side falls: yaw = push - 90° / + 90°).

**Hold and get-up:** the knockdown plays as a dynamic montage (blend in 0.06 s, no auto blend-out) and is paused 0.08 s before its end, so the lying pose
holds until the get-up. `SetKnockedDown(false)` plays the get-up matching the knockdown (`GetUp_Side_Left/_Right`, `GetUp_Front` after `Knockdown_Back`
and `Trip`, `GetUp` after `Fall`). The visual learns the down time (game seconds between the two calls) on a skater's first knockdown; from then on it
starts the get-up early so the figure is up when the sim lets it skate, and it time-scales the knockdown if the down time is under 0.55 s.

**Trip:** nothing in the sim or the pawn identifies a trip (`KnockDown` has one cause, `check`). Reachable as Blueprint event
`SetTripped(bool bDown, float DirX, float DirZ)` → `Trip` → `GetUp_Front`.

**Timing in the match:** the sim keeps a skater down exactly `knockdown_frames` sim frames (default 90 = 1.5 s); the visual measured 1.35 to 1.5 s game
time between the calls. Knockdown 0.5 s + get-up 0.9 s = 1.4 s fits inside 1.5 s once the down time is learned.

**Hit reactions and collisions (Blueprint events, tested on test figures):** `PlayHitReaction(DirX, DirZ)` (`Check_Hit_Left/_Right/_Back`, same
direction convention), `PlayBoardHit()` (`Board_Hit`), `SetBoardPin(bool)` (`Board_Pin` looping, stops on false), `PlayStickHit(bool bLifted)`
(`Lift_Hit` / `Poke_Hit`). Priority: knocked down 9 (blocks all), get-up and reactions 6, foul 5, shot / check 4, poke / lift / pass 3, draw 2.
`PlayCheck()` plays `Check_Deliver_Right` or `_Left` by the side of the nearest opponent visual (verified: opponent on the right → `_Right`).
