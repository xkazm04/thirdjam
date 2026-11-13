# Lane A92: a hit obeys inertia: impulse against stability, three outcomes, the carrier cannot hit (device A, from H22, R-0045 law 1, Fable)

You are lane A92 of the Third Jam (team Hokej). Read `docs/VISION.md` first: law 1 is yours, word for word. Today a check lands when the checker's own speed passes
`check_speed_min` inside a reach and a press window, and it always knocks the victim down for `knockdown_frames`; two skaters going the same way at the same speed hit as
hard as a head-on charge, and a standing skater falls like a sprinting one. Your job: contact by momentum.

Read first: `docs/VISION.md`, R-0045, `docs/PLAN-RECONCILED.md`, `docs/experiments/TACKLING.md`, `TACKLING-2.md`, `TACKLING-3.md` (A16, A45, A73, A80: the check rule, the
read-outs `bBumpThisFrame`, `JAMVIS hit|board|pin|stickhit`, the knockdown and its frames, the hit-stop), `contracts/skater-visual.md` (v4: `PlayHitReaction(DirX, DirZ)` is
the stumble's figure call; `SetKnockedDown(bDown, DirX, DirZ, KnockdownFrames)`), `contracts/events.md` (`check`, `knockdown`: Kind, Strength, Dir), `Game/Source/JamCore/
Private/JamSim.cpp` (`TryCheck`, the skater-skater collision and `SkaterSkaterBounce`, `TryLift`, `IsLiftShielded`, the foul-from-behind rule, `EndKnockdown`), `JamSim.h`
(`FJamSkaterState`), `Game/Source/JamCore/Private/Tests/JamTacklingTests.cpp`, `Game/Source/JamGame/Private/TacklingTests.cpp`, `docs/experiments/ACCEPT.md` and
`HokejAcceptDrill.cpp` (steps 7 and 8), `schema/`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a92` (created at main 8f6b157; build it yourself, unity off; the Development game package with `-WaitMutex`;
no interactive editor). Run no git command; never touch `Game/Plugins/JamAssets`; never print `.env`; never kill a process you did not start. Three other lanes edit the sim
and the brain at the same time, each fenced to its functions: **A93** the skater's movement step, the sprint, the carried puck's distance (`StepSkater`, the stamina, `StepStick`,
`CarrierReachM`); **A91** the shot's lift and charge and drills 9-11; **A94** the brain (`JamAICore.cpp`); **A86** the difficulty numbers. You own the contact functions only
(`TryCheck`, the collision resolve, `TryLift`'s hit part, a new `StabilityOf`, a new `ResolveContact`); you **read** `S.Vel`, the sprint flag and the carrier, you do not
write them.

You own: the contact model in `JamSim.cpp/.h`; `game.contact { lunge_mps 1.5, thrust_mps 3.0, bump_ratio 0.4, loose_ratio 0.7, down_ratio 1.0, stand_brace 1.5,
sprint_stability 0.7, unseen_stability 0.8, down_frames_min 60, down_frames_max 150, stagger_frames 16 }` in `schema/` + `JamCartridge.*` (names yours to refine; unknown keys
are errors; the three generators); the tackling tests; the acceptance drill's steps 7 and 8 (`HokejAcceptDrill.cpp` and the two rows in `tools/qa/accept/accept.mjs`: nothing
else there); `docs/experiments/CONTACT.md`; `status/A92.md`.

## Job
1. **The model** (VISION law 1): the impulse from the closing speed along the contact line times the attacker's mass, plus the lunge for a pressed check and the thrust for a
   cross-check; `StabilityOf(skater, hit direction, kind)`: the standing brace against a side body hit, the loss with speed, with the sprint, in a spin or a full wind-up,
   unseen from behind; a cross-check inside the victim's front arc ignores the standing brace; three outcomes by ratio: bump, stumble (half the speed, `stagger_frames` without
   control, `PlayHitReaction` through the existing bump read-out, the puck loose above `loose_ratio`), down (`down_frames_min..max` by ratio, the existing knockdown path, so the
   hit-stop and B's figure work unchanged); the attacker loses the speed it gave and stumbles itself when its own stability is the lower and the victim's brace holds.
2. **The carrier cannot hit**: a body check or a cross-check pressed while carrying is refused (a `check refused carrying` read-out for the HUD's existing miss text).
3. **Neutral values give back today's game**: with `game.contact` set to its neutral block (document it) a check behaves as on main; `Jam.Core.ContactNeutral` proves the
   hash of a bot match equals main's. The default cartridge uses the new model.
4. **Tests** (`Jam.Core.Contact.*`): same direction and speed: a bump at most; head-on at full speed: down at the maximum; a standing braced skater: a side body check from
   4 m/s stumbles it at most, a front cross-check from the same speed puts it down; a sprinting victim falls to a hit a walking one survives; the attacker bounces off a
   braced skater when it sprints into it; the carrier's press is refused; fouls from behind still call the foul.
5. **The acceptance run's steps 7 and 8** re-staged for the model (the check comes with closing speed; the cross-check from the front) and green on your Development package;
   the whole run exit 0 on your tree except rows another lane owns (say which).
6. **A bot match** at pro and at all_star, 3 minutes each, headless: bumps, stumbles, knockdowns per match and by whom; the numbers against main's (A80: 5 knockdowns, all
   of the human). Report, do not tune the levels (A86 and the final curve own them).
7. `CONTACT.md`: the model with its formulas, the keys, the neutral block, the tests, the match numbers, what B's figure should show for a stumble (the call exists).

Report: the tests, the hash before and after, the neutral proof, the acceptance rows, the match numbers, the file list. Write `status/A92.md` from the first minute. Interim
at 40 minutes with steps 1-3 in the sim. Time box 110 minutes.
