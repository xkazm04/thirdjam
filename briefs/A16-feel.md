# Lane A16: seams first, then puck control, passing, charged shots, deflections (device A, from H7)

You are lane A16 of the Third Jam (team Hokej). Read first: `contracts/depth.md` (C-0005: you own the sim and land the two seams FIRST), `GAME.md`, `docs/HOCKEY.md` (2, 7), `docs/experiments/FEEL.md`
(A10's measurements: 0.67 s to top speed, 218 m glide, carrying impossible before drag 0.5), `docs/experiments/BALANCING.md` (A3's final profiles and the sweep; your changes must keep the curve
within 5 points), `Game/Source/JamCore/Public/JamSim.h`, `Private/JamSim.cpp`, `JamPawns.*`, `Game/Source/JamCore/AI/JamBrain.h` (A3: bots must learn to pass and to intercept passes; propose the
brain patch, do not edit AI/), `Game/Source/JamGame/Public/ToySkaterVisual.h`, `InputMapping.h` (A5; you add the pass action and the charge parameter), `contracts/skater-visual.md`, `tools/sweep/run.mjs`.

You own what C-0005 section 1 says. Run no git command; headless first (automation tests, the sweep), editor on MCP port **8001** only for the PIE proof; link windows at :00 and :30; keep the
tree compiling (build before every pause and report). Never kill a process you did not start.

## Order
1. **Seams (30 minutes, report when done):** `JamModifiers.h` and `JamRuleHooks.h` exactly as C-0005 sections 2 and 3; the sim resolves effective stats from a modifier source (none set = base
   values, bit-identical hashes to today: prove with the Determinism test), calls the rule hook at every trigger (none set = no-op), applies the action set (AddScore, SetScoreMultiplier, EndPeriod,
   EndMatch, ShrinkRink, GrowRink at the next faceoff, SetParam via the live-config path, SpawnPickup as a request the pickup system reads, SwapNets, Announce as a bus event, ApplyModifier as a
   request the modifier source reads), new events `announce`, `pass`, `effect_start/end`, `zone_enter/exit` registered on the bus and in `contracts/events.md` (append under C-0005). Build; tell the Director.
2. **Feel** per C-0005 section 5: pass, carry assist, charge shot with the visual parameter, deflections, strip-on-check; schema fields `shot.charge_frames_max`, `charge_speed_bonus`, `deflect`,
   `puck.carry_assist`, `puck.count`, `skater.pass_speed`, `pass_lead_frames` with defaults and limits; `PlayShoot(float Charge)` on `IJamSkaterVisual` (update C-0001 text, the toy figure and
   B's note in your report); input action `IA_Pass` (E / X).
3. **Tests:** `PassReachesTeammate` (a scripted pass arrives within `pass_lead_frames` tolerance in 9 of 10 seeds), `CarryTurn` (the carrier keeps the puck through a 90-degree turn at speed 8 with
   `carry_assist` 0.6, loses it on a 180 at full speed), `ChargeShot` (speed scales with hold frames), `DeflectionConserves` (70 %), `StripOnCheck`; Determinism and the full `Jam.Bot` suite still pass;
   run the A3 sweep on regular at the three levels and report the curve delta.
4. PIE proof with the scripted human: a pass, a carried turn, a charged goal; screenshots to `devlog/shots/`.

Report: seams done (time), then feel: numbers, the curve delta, the brain patch A3 needs (pass awareness, pass interception), every error. Time box 3 hours; report at 30 minutes (seams) and 90 minutes.
