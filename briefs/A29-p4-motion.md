# Lane A29: the P4 motion set: skating and the stick verbs from text-to-motion, video and sourced clips (device A, from H10.4)

You are lane A29 of the Third Jam (team Hokej). Ruling R-0025 point 3. The expectation: P4 figures able to play hockey, with animations as the known risk. Read first: R-0025,
`docs/experiments/ANIMATION-SOURCES.md` (today's research: Meshy text-to-motion, Rokoko Vision and Create, the Fab Hockey Player pack, Manny clips; what was dropped and why),
`contracts/skater-visual.md` (C-0001 v3: the verb list a figure must show: idle, skate cycle, carry, aim left/right, poke, lift, pass, shoot wind-up and release, check, knocked down,
get up, faceoff ready, draw, goalie stances, foul reaction), `docs/experiments/BLENDER.md` section 5 (the toy figure's 17 actions: the same list, your clips must map to it),
`tools/mcp/ANIMATION.md` (the editor-side retarget helpers), `briefs/A28-p4-figure-slice.md` (the figures your clips will play on; coordinate by status file only).

Keys in `.env`: `MESHI_API_KEY` (Meshy text-to-motion and its animation library), `FAL_API_KEY`. Rokoko is a web account (the operator's); Rokoko Create text-to-motion gives
five free clips a month and Rokoko Vision needs the operator to film: write the exact filming instructions (camera placement, two views for contact moments, what to perform, in
which order, how long) into `docs/experiments/P4-MOTION.md` section "For the operator to film" and stop there for that route. Never print a key.

You own: `tools/p4motion/**`, `Game/Content/P4/Anim/**` (`AS_P4_*`, IK rigs and retargeters), `docs/experiments/P4-MOTION.md`, `status/A29.md`. Editor on MCP port 8016 only for
retargeting and capture, at most three editors on the machine, headless import preferred. Run no git command; never touch `Game/Plugins/JamAssets`.

## Registry rules that bind this lane (motion source by action class)
- Cyclic locomotion on a human skeleton (skate stride, backward skate, crossover, glide) comes from **text-to-motion first**; video only for a gait the model has not learned. A
  text-to-motion loop still owes a closure check.
- Actions along the camera axis or with a contact moment (check, poke, shot, faceoff draw) need **two views or a side camera**, or an explicit depth prior you can name.
- **One clip end to end before a batch.** A route proven on a shot has not been proven on a stride.
- A motion gate before any clip enters a build: loop closure (first and last pose within tolerance), foot contact (no float, no slide beyond the glide the sim gives), root motion
  lint (the sim owns displacement: every clip is in place, root motion stripped), a contact window per attack clip (the frame range in which check or poke "hits", written next to
  the clip so the visual can time the impact effect).

## Job
1. **Text-to-motion**: Meshy text-to-motion for the stride set (skate forward, backward, crossover left/right, glide, stop) and Rokoko Create (five clips: pick the stick verbs it
   is most likely to get: shoot wind-up and release, pass, faceoff ready, goalie stance). Prompts, task ids, credits, wall time in `tools/p4motion/jobs.json`; outputs as FBX on
   the vendor's skeleton; retarget to UE5 Manny with an IK Retargeter (bone table in `tools/p4motion/bonemap-<vendor>.json`, totality checked).
2. **Meshy animation library**: the falls, hit reactions and idles it already has for a Manny-rigged figure (B used 366 and 502-505): map them to knocked_down, get_up, check
   received, foul reaction.
3. **Manny clips** that ship with the engine for the rest (hit react, death as fall) as the fallback set, so every verb has a clip from day one.
4. **The gate**: `tools/p4motion/gate.mjs` reads an exported clip summary (per-frame root and foot bone positions, exported by a small editor Python or by the FBX via Blender
   headless: `blender -b -P`) and prints loop closure error, foot slide per frame, root displacement, with pass/fail against thresholds in `docs/experiments/P4-MOTION.md`.
5. **Proof**: every clip played on the Manny (or on A28's first figure if it is in by then) in a 2-second capture per verb under `devlog/shots/`, judged with lane A27's eyes tool
   (`figure` and `bug-hunt` presets) when it exists.
6. `docs/experiments/P4-MOTION.md`: the verb-to-clip table with source, gate result and contact windows; the filming plan for the operator; what each route cost; the
   recommendation per action class.

Report: the table, the gate results, the still paths, the filming plan, the blockers. Time box 2 hours; interim at 45 minutes with the first stride loop retargeted and playing.
