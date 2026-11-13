# Lane A73: tackling as the key feature: the hit felt and seen in the shipped build (device A, from H15, R-0036 / B-to-A-0036, Fable)

You are lane A73 of the Third Jam (team Hokej). The operator told device B at 09:43: "the ability to tackle other players to the ground, collide and animate fall, getting
up, that would be key for the game." C-0001 v4 is on main since dfb808f: B's figures play `PlayHitReaction`, `PlayBoardHit`, `SetBoardPin`, `PlayStickHit` and the
timed `SetKnockedDown` from the pawn's read-outs. B's ask to A (`handoff/B-to-A-0036.md` point 4): the hit felt, not only seen: on a knockdown that involves the human, a
hit-stop of 3 to 4 frames and a short camera kick along the hit direction; and the whole chain (press, hit, fall, slide, lying, get-up) proven in a packaged build.

Read first: `handoff/B-to-A-0036.md`, `contracts/skater-visual.md` (v4), `docs/experiments/TACKLING.md` (A16, A45: the check rule, `check_speed_min`, the strip, the
knockdown read-outs `JAMVIS hit|board|pin|stickhit`), `CONTROLS.md` (A49: RMB tackle, MMB cross-check, the human's check from 2 m/s; the SendInput harness in
`tools/qa/ctrl/`), `STICK.md` (A51: the spin checked mid-turn falls 1.5x), `FEEL.md`, `CAMERA.md` (A33's punch, A40's clamp, A49's bounds), `Game/Source/JamGame/Private/
{CameraFollow,JamPawns,HokejGameMode,EffectsBinder}.cpp`, `Game/Source/JamCore/Private/JamSim.cpp` (`TryCheck`, the knockdown, `knockdown_frames` 90: B's takes are fall
26, lying about 20, get-up 44), `Game/Source/JamCore/AI/JamPlaytester.cpp` (the classes check), `docs/experiments/PROFILING.md` (the remote console drives a packaged game
when a foreign dialog holds the foreground).

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a73` (created at the current main; build it yourself, unity off; one editor on MCP port 8035 only while
proving; at most three editors on the machine). Run no git command; never touch `Game/Plugins/JamAssets` (what the figure lacks goes to B in your report with the call
you would add); never print `.env`; never kill a process you did not start. The hit-stop is presentation (the sim never pauses; the pawns and camera hold their frame
for N frames while the sim runs: say how, so lockstep is untouched); the sim hash never changes in this lane.

You own: `CameraFollow.*` (the kick along the hit direction, `Strength`-scaled, 8 frames, under A49's bounds), `JamPawns.cpp` (the hit-stop: the visuals hold 3 frames on a
knockdown that involves the human; the carried puck and the HUD too), `HokejGameMode.cpp` (the read-out routing only), `EffectsBinder.cpp` (a stronger `check`/`knockdown`
variant by `Strength` if B says so), `tools/qa/ctrl/a73-*` (the proof driver), `docs/experiments/TACKLING-2.md`, `docs/experiments/img/a73/**`, `status/A73.md`.

## Job
1. **The chain seen**: in a Development package of your worktree, a 1-on-1 drill where the human (SendInput or the remote console; A49's harness) checks a carrying bot
   ten times from the side at 3 to 6 m/s: count from the log the `JAMVIS hit`, the knockdowns, the strips, the get-ups; a still every 4 frames through one fall, lying,
   get-up (12 stills) and the same for a board hit and a stick hit; eyes `bug-hunt` on three.
2. **The hit felt**: the hit-stop (3 frames, both figures and the puck held, the sim running) and the camera kick (along the hit direction, 8 frames, Strength-scaled,
   clamped inside A49's bounds); a `jam.hitstop` cvar 0..6 so the operator can taste it; proven by a 60-frame still strip of a human check and one of a human knocked down.
3. **The human knocked down**: a bot's check on the human at pro: the fall, the camera staying on the lying human, the get-up, the YOU ring through it; ten of ten in the
   package with the counts.
4. **What B needs**: anything the figure lacks (a fall direction that reads wrong, a get-up that pops, a pin that never shows) with the call or parameter you would add;
   the `Strength` scale you used for the effect variant.
5. `TACKLING-2.md`: the counts, the stills, the cvar, the asks for B, and the three things the operator will feel first.

Report: the counts (hits, knockdowns, strips, get-ups), the still paths, what B needs, the file list. Interim at 40 minutes with step 1. Time box 100 minutes.
