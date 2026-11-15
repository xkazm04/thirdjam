# Lane A223, track L1 (the look), Sunday: three looks of A's that the trailer will show (device A, Fable: judgement by looking)

You are lane A223 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md`, then `briefs/NIGHT-COMMON.md`: their rules are yours in full.**
Then **R-0069 and R-0070** at the end of `contracts/RULINGS.md` (nothing technical is drawn on or in front of a skater; no lines on the ice),
device B's handoff `handoff/B-to-A-0046.md` (in your worktree: it is on device B's branch, merged into your tree by the Director), device B's
`status/B.md` from H58.5 on (lane LK1: the day rig, the shotgun blast, Glow Ice in light, the stands), `docs/experiments/BAR.md` (lane A219's
method and tools for filming the game off screen from the game camera: `tools/qa/eye2/`: yours to copy into `tools/qa/eye3/` and change).

Your worktree and what it stands on are named in the Director's message that starts you. Status file `status/A223.md`. Two heavy processes at
most. Off screen only (`-RenderOffScreen`, `-dpcvars=r.RDG.AsyncCompute=0`): no window, no cursor. Nothing of yours runs from the main checkout.

## Why
The operator records the trailer from about 18:00 today; Pond, Fog of War and Glow Ice are in it. Device B's look lane filled the day rig and
then looked at A's own parts from the game camera at 1920 x 1080 and names three that are A's code:

1. **Board trim on `l1_day` (Pond, Outnumbered):** still Emissive 30 (`LookTierSwitcher`, `GTierTrim`): in daylight a soft yellow glowing line.
   B suggests 8 to 12. Judge by looking: the trim should read as painted trim in daylight, not as a light.
2. **Glow Ice (`l3_neon`, the trailer's finale):** the penalty boxes and their clocks are lit cream: the brightest things outside the play.
   They should sit back: nothing off the ice outshines the skaters, the puck and the lines.
3. **Fog of War:** the fog itself is not visible at the game camera. The mode's line is "Late bots in the fog. Slip by, then hit.": a person
   must SEE fog (depth: far skaters and the far boards fade into it) while the puck, his own skater, the nearest opponents and the rings stay
   readable. No change to the sim: presentation only, the sim's hash equal.

## Do
- **Look first (20 minutes):** the three modes from the game camera at 1920 x 1080 and 1280 x 720 on your tree, stills of a faceoff, open
  play at both ends, a goal: write `SEEN` lines with the stills (`docs/experiments/looks/`), each fault named in your words. Also look for
  anything else in these three modes that would embarrass a trailer shot (a black void, a blown-out surface, a mark that R-0069 forbids, a
  flat untextured mesh in frame) and list it: cure what is A's and small; what is device B's goes under `FOR B` with the asset's name.
- **Cure** by the smallest sure change in A's files (find where each value lives: `LookTierSwitcher.*`, the arena's dressing code, the mode
  cartridges' look keys); every number behind a `jam.look.*` variable or a look key with the old value as its neutral. Never edit
  `Game/Plugins/JamAssets` or `contracts/`. Pictures before and after, side by side, the same frames.
- The other looks (clay PLAY NOW, the night look) must not change: one still each before and after to show it.

## Prove
Sheets before / after for the three modes at both sizes; the tests of the look (`Jam.Look.*`, `Jam.Bot.VisualSwap`: an audio-device swap on
this machine can false-fail it: rerun once before believing it) updated where they pin a number; the full `Jam` filter in a cleared
environment; Shipping compiles; the sim's hash equal on a 60-second scripted match per mode.

`SEEN` lines as soon as the first stills exist (the Director looks at them himself); `LOOKS READY` with the files by name, the sheets and
the gate **by 15:40 by the machine clock**. Keep the files closed after the line. If one of the three cannot be cured by then, land the
others and name it.
