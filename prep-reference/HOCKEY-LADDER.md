# Hockey ladder: one arena, three lighting tiers (reference images only)

Status: written 2026-11-11, before the jam. **Reference studies only** (contest rule 7(h), see `README.md`): never imported into any
Unreal project, never used as textures, backdrops or tracing sources, never shipped. Whatever is chosen is rebuilt from scratch in
the engine during the jam. Feasibility notes target Unreal Engine 5.9 (stock build) on one RTX 4090; engine feature statements are
from general UE 5.x knowledge, not measured here and not verified for 5.9. Hours and frame times are estimates, not measurements.

## The idea

Ice hockey 3 vs 3 in a toy-like arena (rink 60 by 30 m, goals 4 m wide at each end, rounded corners, boards all round), seen from a
high three-quarter camera that keeps both goals in frame. One geometry and one material set; three stacking lighting-and-post tiers,
each tied to a difficulty tier of the game: L1 daylight toy rink (M1 regular hockey), L2 night arena (M2 raw play, tackles),
L3 full neon (M3 boost packages). Each tier has three environment variants: (a) open-air rink, (b) indoor rink, (c) tabletop-sized
rink. The operator picks a variant by looking at the climb, then one is built.

## How the images were made

Same method as the first look studies: Codex image generation, one image per run, no reference images attached, via
`tools/gen-hockey.mjs` (a copy of `tools/gen.mjs` that reads `tools/hockey-jobs.json`; `tools/mkjobs.mjs` writes that file).
Command: `node tools/gen-hockey.mjs` (skips names whose PNG already exists; `node tools/gen-hockey.mjs hockey-l2-b` for one).
Each run received this wrapper around the per-image prompt below:

> Use the built-in image_gen tool (imagegen skill) to generate exactly ONE image, landscape. **[per-image prompt]**
>
> Reference study for art direction only. Landscape 16:9, same high three-quarter camera angle in every study. No text, no letters, no logos, no watermark, no readable numbers or letters on jerseys or boards, no real people, no real teams or leagues.
>
> Then copy the generated file to ./out/<name>.png and print the original path. Do not generate more than one image.

Per-image prompt structure: shared subject sentence, then tier look, then variant. The subject sentence and the camera wording
are identical in all nine, so the ladder reads as one place. The prompts below are the exact text from `tools/hockey-jobs.json`.
Results: 9 of 9 generated on the first attempt, 47 to 65 seconds each, no failures.

The caution from `STUDY.md` applies again: a generated still shows a finished look and hides the cost. The stands, trees, bookshelves
and skylines in these images are dressing that a stock-engine build would not have; the arena is the part that must be rebuilt.

---

## L1 Daylight toy rink (game tier M1, regular hockey)

**`images/hockey-l1-a.png`** (open-air, clear sky, simple stands)

> An ice hockey rink for a toy-like 3 versus 3 game, a 60 by 30 metre rink with rounded corners, boards all around and a four metre wide goal at each end, seen from a high three-quarter camera that keeps BOTH goals in frame. Six stylized chunky toy figures (capsule bodies, round heads, simple sticks, three in deep blue jerseys and three in warm orange jerseys) and one small near-black puck on the ice. Tier look, daylight toy rink: clean stylized toy diorama, saturated flat-ish materials, pale blue-white ice with simple painted markings, bright boards, soft global illumination, strong rim light, glossy plastic accents, cheerful. Variant: an open-air rink under a clear sunny sky with simple stands around it.

**`images/hockey-l1-b.png`** (indoor community rink, big windows, warm light)

> An ice hockey rink for a toy-like 3 versus 3 game, a 60 by 30 metre rink with rounded corners, boards all around and a four metre wide goal at each end, seen from a high three-quarter camera that keeps BOTH goals in frame. Six stylized chunky toy figures (capsule bodies, round heads, simple sticks, three in deep blue jerseys and three in warm orange jerseys) and one small near-black puck on the ice. Tier look, daylight toy rink: clean stylized toy diorama, saturated flat-ish materials, pale blue-white ice with simple painted markings, bright boards, soft global illumination, strong rim light, glossy plastic accents, cheerful. Variant: an indoor community rink with big windows and warm light.

**`images/hockey-l1-c.png`** (tabletop rink on a wooden table, window light)

> An ice hockey rink for a toy-like 3 versus 3 game, a 60 by 30 metre rink with rounded corners, boards all around and a four metre wide goal at each end, seen from a high three-quarter camera that keeps BOTH goals in frame. Six stylized chunky toy figures (capsule bodies, round heads, simple sticks, three in deep blue jerseys and three in warm orange jerseys) and one small near-black puck on the ice. Tier look, daylight toy rink: clean stylized toy diorama, saturated flat-ish materials, pale blue-white ice with simple painted markings, bright boards, soft global illumination, strong rim light, glossy plastic accents, cheerful. Variant: a tabletop-sized rink standing on a wooden table, toy-box feel, soft window light.

### Feasibility, UE 5.9 on one RTX 4090

- **What carries the look**: one directional light (warm sun) plus a sky light, both with soft shadows; Lumen global illumination for
  the soft bounce (or baked lightmaps on the static rink, cheaper and just as cheerful for a flat-ish toy look). The strong rim light
  is a second, shadowless directional light from the far side of the camera, or a Fresnel term in the master material. Materials:
  saturated base colours, low-roughness plastic on the figures, boards and goal frames, ice as a pale blue-white with a soft
  clear-coat and a tiling scratch normal. Post process volume: slight bloom (about 0.2), locked auto-exposure range, light vignette,
  gentle colour grade; no ray tracing.
- **Costs the most**: soft shadows from the sun over six characters plus the Lumen bounce on the white boards. In variant (b) the
  window light needs rect lights or emissive-lit planes, and big windows through Lumen add noise. Variant (c) wants a cheap, stable
  depth-of-field tilt.
- **Frame-rate risk on a mid-range judge GPU**: low. Lumen at medium quality plus virtual shadow maps with a fixed light count
  should hold 60 fps on a GTX 1660 / RTX 3060 class card at 1080p; the safe fallback is baked lighting plus one cascaded shadow map.
- **Readability**: best of the three. A near-black puck on pale ice is the strongest contrast in the game; deep blue against warm
  orange separates at any distance and is the colour-blind-safe pairing. The puck is only a few pixels tall from this camera at
  1080p, so give it a slightly oversized visual radius and a soft contact shadow.

---

## L2 Night arena (game tier M2, raw play, tackles)

**`images/hockey-l2-a.png`** (open-air at night, floodlights, dark sky)

> An ice hockey rink for a toy-like 3 versus 3 game, a 60 by 30 metre rink with rounded corners, boards all around and a four metre wide goal at each end, seen from a high three-quarter camera that keeps BOTH goals in frame. Six stylized chunky toy figures (capsule bodies, round heads, simple sticks, three in deep blue jerseys and three in warm orange jerseys) and one small near-black puck on the ice. Tier look, night arena, the same rink after dark: dark ambient light, emissive trim lines along the boards and goal frames, a few strong spotlights, mild bloom, cool shadows, ice still readable and dry (not wet, not reflective), a bit grittier with scuffed ice, one or two figures tipped over. Variant: the open-air rink at night under floodlights with a dark sky.

**`images/hockey-l2-b.png`** (indoor, house lights down, spotlights on)

> An ice hockey rink for a toy-like 3 versus 3 game, a 60 by 30 metre rink with rounded corners, boards all around and a four metre wide goal at each end, seen from a high three-quarter camera that keeps BOTH goals in frame. Six stylized chunky toy figures (capsule bodies, round heads, simple sticks, three in deep blue jerseys and three in warm orange jerseys) and one small near-black puck on the ice. Tier look, night arena, the same rink after dark: dark ambient light, emissive trim lines along the boards and goal frames, a few strong spotlights, mild bloom, cool shadows, ice still readable and dry (not wet, not reflective), a bit grittier with scuffed ice, one or two figures tipped over. Variant: the indoor rink with the house lights down and spotlights on.

**`images/hockey-l2-c.png`** (tabletop rink in a dark room, lit by its own trim)

> An ice hockey rink for a toy-like 3 versus 3 game, a 60 by 30 metre rink with rounded corners, boards all around and a four metre wide goal at each end, seen from a high three-quarter camera that keeps BOTH goals in frame. Six stylized chunky toy figures (capsule bodies, round heads, simple sticks, three in deep blue jerseys and three in warm orange jerseys) and one small near-black puck on the ice. Tier look, night arena, the same rink after dark: dark ambient light, emissive trim lines along the boards and goal frames, a few strong spotlights, mild bloom, cool shadows, ice still readable and dry (not wet, not reflective), a bit grittier with scuffed ice, one or two figures tipped over. Variant: the tabletop rink on a table in a dark room, lit by the toy's own glowing trim.

### Feasibility, UE 5.9 on one RTX 4090

- **What carries the look**: the L1 sun is replaced by a dim cool sky light (a low-intensity moonlight directional is optional);
  three to five spot or rect lights (floodlights or spotlights) in cool white, with a visible light shaft (localised volumetric
  fog, or a cheap additive cone mesh). The emissive trim is the L1 trim material with its emissive parameter raised (blue for one
  side, orange for the other, which also helps readability). Post process: bloom up to about 0.8 to 1.0 (this makes the trim glow),
  exposure bias down, cool shadow tint in the grade, a little more contrast. Scuffed ice is a second normal/roughness layer blended
  by a mask (vertex colour or a runtime scuff texture), not new geometry. Tipped-over figures are a pose or physics state, not an asset.
- **Costs the most**: the number of shadow-casting spot lights (each costs virtual shadow map time); keep them to four and fake the
  rest with additive cones and emissive spots. Lumen in a dark scene shows slow, noisy GI, so lock indirect intensity low.
- **Frame-rate risk on a mid-range judge GPU**: low to medium. Four shadowed spots plus mild bloom is close to L1 in cost; the real
  risk is volumetric fog, so make it optional or low resolution.
- **Readability**: the puck is the weak point. A near-black puck on mid-dark ice loses contrast, so L2 needs a puck rim (a thin
  emissive ring or a light-grey edge band) that is also on in L1, so nothing is rebuilt. Teams stay readable through jersey colour
  plus emissive trim of the same hue on their own goal. Dry, non-reflective ice keeps the surface calm so the figures stand out.

---

## L3 Full Neon (game tier M3, boost packages)

**`images/hockey-l3-a.png`** (open-air, neon strips, reflections, dark city or hills behind)

> An ice hockey rink for a toy-like 3 versus 3 game, a 60 by 30 metre rink with rounded corners, boards all around and a four metre wide goal at each end, seen from a high three-quarter camera that keeps BOTH goals in frame. Six stylized chunky toy figures (capsule bodies, round heads, simple sticks, three in deep blue jerseys and three in warm orange jerseys) and one small near-black puck on the ice. Tier look, full neon: the same rink at full neon, wet mirror-like reflective ice, cyan and magenta neon strips along the boards and goal frames, low ground fog, strong bloom and glow, small glowing boost pickups (bright capsules and rings) lying on the ice, path-traced look, high contrast. Variant: open-air rink, neon strips along boards and goals, their reflections visible in the ice, dark city skyline or hills behind.

**`images/hockey-l3-b.png`** (indoor rink, neon trim, haze, reflections)

> An ice hockey rink for a toy-like 3 versus 3 game, a 60 by 30 metre rink with rounded corners, boards all around and a four metre wide goal at each end, seen from a high three-quarter camera that keeps BOTH goals in frame. Six stylized chunky toy figures (capsule bodies, round heads, simple sticks, three in deep blue jerseys and three in warm orange jerseys) and one small near-black puck on the ice. Tier look, full neon: the same rink at full neon, wet mirror-like reflective ice, cyan and magenta neon strips along the boards and goal frames, low ground fog, strong bloom and glow, small glowing boost pickups (bright capsules and rings) lying on the ice, path-traced look, high contrast. Variant: indoor rink with neon trim, haze in the air and reflections in the ice.

**`images/hockey-l3-c.png`** (tabletop rink in a dark room, the whole toy glowing, glossy table)

> An ice hockey rink for a toy-like 3 versus 3 game, a 60 by 30 metre rink with rounded corners, boards all around and a four metre wide goal at each end, seen from a high three-quarter camera that keeps BOTH goals in frame. Six stylized chunky toy figures (capsule bodies, round heads, simple sticks, three in deep blue jerseys and three in warm orange jerseys) and one small near-black puck on the ice. Tier look, full neon: the same rink at full neon, wet mirror-like reflective ice, cyan and magenta neon strips along the boards and goal frames, low ground fog, strong bloom and glow, small glowing boost pickups (bright capsules and rings) lying on the ice, path-traced look, high contrast. Variant: tabletop rink in a dark room, the whole toy glowing, reflections on a glossy table.

### Feasibility, UE 5.9 on one RTX 4090

- **What carries the look**: the L2 rig with emissive intensity pushed much higher and a cyan/magenta palette on the trim strips;
  ground fog (exponential height fog at low density plus a few ground-level fog cards or a small volumetric fog); strong bloom
  (1.5 or more); the ice material goes to low roughness (about 0.05 to 0.15) with a wet-sheen layer, which turns every emissive
  strip into a reflection streak. Boost pickups are emissive capsules and rings (a mesh plus an emissive material, optionally a
  small unshadowed point light). The "path-traced look" is a high-contrast, saturated grade, not the real path tracer.
- **Needs hardware ray tracing**: the mirror-like reflection of the neon strips, goal frames and figures in the ice (Lumen with
  hardware ray-traced reflections, or the path tracer). The RTX 4090 on device B does this comfortably; a judge GPU without RT
  hardware does not.
- **Fallback look (no ray tracing)**: Lumen software reflections or screen-space reflections on the ice plane, plus a fake mirror
  layer (the strips and goal frames duplicated below the ice surface with a fresnel fade). Off-screen things such as the skyline
  drop out of the reflection and figures smear, but the rink still reads as wet and glowing because bloom and the emissive strips
  carry most of the effect. The last resort is a fixed reflection capture plus a gloss map: flatter, still clearly "neon".
- **Costs the most**: ray-traced reflections on a full-screen glossy ice plane (nearly every pixel pays), then bloom and fog together.
  The real path tracer is for stills, not play: do not plan to ship it.
- **Frame-rate risk on a mid-range judge GPU**: high if reflections are ray traced (a GTX 1660 has no RT hardware; an RTX 3060 would
  likely fall well under 60 fps at 1080p with hardware RT on a full-screen glossy plane). Ship L3 with the reflection method as a
  quality switch (RT, then screen-space, then fake mirror) defaulted by detected hardware, so the judge's machine falls back
  gracefully instead of dropping frames. This is the one tier that can embarrass the project on weak hardware.
- **Readability**: this is where the L3 images show a problem. Neon, reflections and bloom compete with the puck and the boost
  pickups, and a near-black puck on dark wet ice nearly vanishes. The puck needs its own glow (emissive core, thin halo or short
  trail) in L3 via the same emissive parameter. Keep the painted ice lines dim so the glowing strips, pickups and puck are the
  brightest things. Blue and orange jerseys still read against cyan and magenta trim because the figures are lit and saturated, but
  a blue figure next to a cyan strip is the case to watch; keeping the trim palette distinct from the jersey palette (as these
  images do) is the safer choice.

---

## What stays constant (nothing rebuilt between tiers)

- **Geometry**: the rink (60 by 30 m, rounded corners), boards and glass or fence panels, two goals (4 m wide), face-off circles and
  lines (decals or a texture layer on the ice), the figures (capsule bodies, round heads, sticks), the puck, the pickup meshes
  (present from the start, hidden until L3), and the surrounding dressing of the chosen variant (stands, windows or table).
- **Materials**: one master ice material, one master plastic/boards material, one emissive-trim material. The emissive and roughness
  parameters exist from L1 (trim emissive at zero or low, ice roughness at its dry value) and are only driven differently per tier
  through a material parameter collection. Jersey colours (deep blue, warm orange) never change.
- **Camera**: one high three-quarter camera that keeps both goals in frame, same field of view, framing and height in all three
  tiers, so a player or judge sees the same place.
- **Gameplay-facing values**: collision, physics, ice friction, the puck rim (see readability), the size of everything.

## What changes per tier

| | L1 Daylight | L2 Night | L3 Full Neon |
|---|---|---|---|
| Key light | warm sun directional + sky light | dim cool sky light, three to five spot or rect lights | the L2 lights, colourised, plus neon strips as lights |
| Ambient / GI | bright, soft bounce (Lumen or baked) | low, cool | low, saturated, bounce from emissives |
| Post process | bloom about 0.2, light grade, vignette | bloom about 0.8 to 1.0, exposure down, cool shadows | bloom 1.5 or more, high contrast, saturated grade |
| Emissive trim | off or weak | on, moderate (side colours) | on, very strong, cyan and magenta |
| Fog | none | none, or a faint volumetric shaft | ground fog, haze |
| Ice | pale, soft gloss, clean | scuffed, dry, readable | wet, mirror-like, low roughness |
| Reflection method | reflection capture / Lumen default | same as L1 | hardware RT reflections, falling back to screen-space or fake mirror |
| Extras | none | tipped-over figure poses, scuff decals | glowing boost pickups, spray and trail particles |
| Cost class | cheap | cheap to medium | expensive (reflections) |

Reading across: L1 to L2 is mostly a lighting swap plus post-process once the master materials exist; L2 to L3 is the emissive and
reflection step and carries nearly all of the risk. Build L1 and L2 first, and treat L3 as an add-on with its own fallback switch.

## One-line read of each tier (does it read as the same arena?)

- **L1**: yes. a is the cleanest rink and the best anchor. b adds a lot of dressing (windows, plants, benches) that a stock build
  would not carry. c reads as a cheerful tabletop toy with the camera noticeably lower and closer than a and b.
- **L2**: yes, same rink with boards and goals lit from within, scuffed ice and tipped figures. a and b stay close to L1-a and L1-b;
  c is the same toy at night with blue and orange trim. The puck is dark against grey ice in all three.
- **L3**: yes, same geometry; reflections, fog and pickups sell the climb in all three. The puck is hard to see in b and c, and the
  neon bloom is heavy enough to hide the ice markings. The tabletop L3-c is the closest to a sealed, self-lit toy. L3-a shows a
  skyline with a tower that resembles a real landmark; it was not asked for, and it is only a reference image.
- **Across variants**: the camera is similar but not identical (c is closer, a shows more surroundings), and the model invented
  different goal and net details between images, which a real build would not do.
