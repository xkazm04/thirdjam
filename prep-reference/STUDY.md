# Look studies: four art directions for the jam

Status: written 2026-11-11, before the jam. Reference studies only (see `README.md`: never imported, never a game asset).
Target for the feasibility notes: Unreal Engine 5.9 (stock promoted build, see PLAN.md section 5), one RTX 4090 on device B,
agents doing the work in about 20 hours of device-B time. Engine feature statements are from general UE 5.x knowledge and are
not measured on this machine or verified for 5.9; hours are estimates, not measurements.

## How the images were made

Every image is one Codex run (`tools/gen.mjs`, one image per run, no reference images attached). Each run received this wrapper
around the per-image prompt below (the wrapper is identical for all 11):

> Use the built-in image_gen tool (imagegen skill) to generate exactly ONE image, landscape. **[per-image prompt]**
>
> Reference study for art direction only. Landscape 16:9, no text, no letters, no logos, no watermark, no characters or people.
> Subject: a physics-driven toy arena seen from a three-quarter game camera, a few chunky interactive objects, a clear open play
> area in the middle, and a distinct background.
>
> Then copy the generated file to ./out/<name>.png and print the original path. Do not generate more than one image.

The per-image prompts are stored verbatim in `tools/jobs.json`. Results: 11 of 11 generated on the first attempt, about 55 to 75
seconds each, no failures.

A caution that applies to all four: a generated still shows a finished look and hides the cost of getting there. Every image has
a full painted backdrop and lots of small set dressing; in the engine that dressing is the expensive part, not the arena.

---

## D1 clean stylized toy diorama

Saturated flat-ish materials, soft global illumination, strong rim light, glossy accents.

| File | Prompt (per-image part) |
|---|---|
| `images/d1-toy-diorama-a.png` | Art direction: clean stylized toy diorama. Chunky stacked wooden-and-plastic blocks and a big rubber ball on a bright round platform. Saturated, flat-ish materials in primary colors, soft global illumination, strong rim light on every edge, glossy accents on plastic parts. Sunny pastel sky background with simple soft clouds. Daylight. |
| `images/d1-toy-diorama-b.png` | Art direction: clean stylized toy diorama. A square play mat with a ring of cylindrical pins, a few oversized cubes and a glossy sphere. Saturated flat-ish materials in teal, orange and yellow, soft global illumination, strong rim light, glossy accents. Warm sunset lighting with long soft shadows, distant stylized hills as background. |
| `images/d1-toy-diorama-c.png` | Art direction: clean stylized toy diorama. An indoor playroom floor as the arena: chunky toy ramps, tubes and spinning discs, soft saturated materials in candy colors, soft global illumination from a big window, strong rim light, glossy accents. Background is a blurred shelf wall with simple colorful shapes. Bright cheerful indoor light. |

What the images show: a and b are almost pure primitives (cylinder, cube, sphere, a rounded rim) with a gradient sky or simple hills;
c needs a lot of bespoke modelled props. Range is carried by palette and light, not by geometry.

**Feasibility (UE 5.9, RTX 4090, about 20 h)**
- Engine features that carry the look: a handful of master materials (flat color with a tint, fresnel rim, clearcoat gloss), one
  directional light plus sky light with Lumen GI, a rim from a second dim directional light or a fresnel term, light bloom, a
  mild color grade. Nanite is not needed for primitives. Niagara only for confetti and impact puffs.
- Asset sources: procedural (engine basic shapes, bevelled via modeling tools or a geometry script), a few generated meshes for hero
  props, optional Fab stylized packs if their licence allows (ledger them). The rounded-bevel chunky shape is the look; it can be made
  with scripts, so agents can do it without hand modelling.
- Cost: **low**. Materials are simple and parametric; the cartridge's theme skin tokens map directly onto material parameters
  (tint, gloss), which is also the best fit for the Designer layer.
- Frame-rate risk on a mid-range judge machine: **low**. Lumen at medium scalability or baked-looking static lighting, few draw calls.
  Software Lumen is enough; no ray tracing needed.
- Theme flexibility: **high**. Toy-like chunky shapes in any palette suit almost any theme; only a dark or gritty theme fights it.
- Risk: looks generic and "mobile" if it stays plain. It needs strong color design, a good sky and some care with soft shadows to
  avoid reading as a template.

| Score (1-5) | |
|---|---|
| Theme flexibility | 5 |
| Buildable in 20 h | 5 |
| Striking in a 60-second video | 3 |
| Performance | 5 |
| Readability of gameplay | 5 |
| **Total** | **23** |

---

## D2 painted cel-shaded

Hand-painted textures, visible ink-like outlines, limited palette.

| File | Prompt (per-image part) |
|---|---|
| `images/d2-cel-painted-a.png` | Art direction: painted cel-shaded. A stone courtyard arena with chunky barrels, crates and a large round boulder. Hand-painted textures with visible brush strokes, bold ink-like dark outlines around every object, flat cel-shaded lighting steps, limited palette of five or six muted earthy colors plus one red accent. Distinct painted forest-and-cliff backdrop. Late afternoon light. |
| `images/d2-cel-painted-b.png` | Art direction: painted cel-shaded. A snowy wooden platform arena with chunky barrels and round stones, hand-painted textures, visible ink-like outlines, flat cel-shaded shadows, very limited palette of cool blues, cream and one orange accent. Painted pine trees and mountains as background. Overcast soft light. |
| `images/d2-cel-painted-c.png` | Art direction: painted cel-shaded. A desert ruin arena with sandstone blocks, clay pots and a round stone wheel, hand-painted textures with rough pencil-like hatching, thick ink-like outlines, limited palette of sand, terracotta, teal and cream. Painted dunes and a huge pale sun in the background. Hard noon light with crisp flat shadows. |

What the images show: strong palette discipline (b is nearly monochrome with one accent), which makes the interactive objects stand
out. The backdrops are flat painted planes in effect; the arena objects carry the outlines.

**Feasibility (UE 5.9, RTX 4090, about 20 h)**
- Engine features: a post-process material for outlines (depth and normal edge detection, or a custom-stencil outline per object
  class), a toon-ramp lighting model in the master material (or a post-process quantisation of the scene color), stylized
  hand-painted-looking texture sets, Lumen can be turned down or off. Niagara for ink-splat style VFX.
- Asset sources: the outline and ramp are engine-side work (low risk). The painted textures are the cost: generated texture sets
  (made inside the jam) plus a texture-tiling master material, or Fab painted packs. A painted backdrop is cheap if it is a few
  layered cards, which also suits a fixed three-quarter camera.
- Cost: **medium**. Outline post process and toon ramp are about 3 to 4 hours; consistent painted textures across every prop is the
  long tail, and an agent can drift in style between assets.
- Frame-rate risk: **low to medium**. Edge-detect post process is cheap at 1080p; it gets noisier and costlier at high resolution or
  with many overlapping translucent materials.
- Theme flexibility: **medium to high**. Palette swaps rebuild the whole mood (b and c show it), but each theme needs painted
  textures redone, which Designer-time skinning cannot do live.
- Risk: outline artifacts on thin geometry and on particles; style drift between assets.

| Score (1-5) | |
|---|---|
| Theme flexibility | 4 |
| Buildable in 20 h | 3 |
| Striking in a 60-second video | 4 |
| Performance | 4 |
| Readability of gameplay | 4 |
| **Total** | **19** |

---

## D3 neon night, high contrast

Dark scene, emissive trim, wet reflective floor, glow and haze; the path-traced flavour.

| File | Prompt (per-image part) |
|---|---|
| `images/d3-neon-night-a.png` | Art direction: neon night, high contrast. A dark industrial arena with a wet reflective black floor, chunky metal crates and a glowing sphere, emissive cyan and magenta light strips along the edges of the play area, volumetric haze and glow, strong reflections of the lights in the wet floor, deep shadows. Distant dark city skyline with a few lit windows as background. Path-traced look. |
| `images/d3-neon-night-b.png` | Art direction: neon night, high contrast. A rainy rooftop arena at night, puddles reflecting colored light, chunky barrels and a heavy ball, emissive orange and violet trim on the barriers, light haze and bloom, glossy wet surfaces, very dark ambient with sharp bright highlights. Blurred glowing city lights far behind. Path-traced look. |
| `images/d3-neon-night-c.png` | Art direction: neon night, high contrast. A dark glass-floored space arena with mirror-like reflections, floating chunky geometric objects with glowing green edge trim, emissive grid lines on the floor, thick glowing fog near the ground, strong bloom, deep black surroundings with a faint nebula as background. Path-traced look. |

What the images show: the most striking stills of the set. c is the simplest to build (dark floor, grid lines, emissive edges,
fog, a star-field sky) and reads as a clean abstract arena; b is the richest and also the most expensive (rain, puddle ripples,
dozens of small props, a skyline).

**Feasibility (UE 5.9, RTX 4090, about 20 h)**
- Engine features: emissive materials plus Lumen GI (emissive surfaces light the scene), Lumen reflections on a glossy floor
  (hardware ray-traced reflections look right; screen-space and software reflections break at this camera angle), exponential
  height fog and volumetric fog, bloom, a strong color grade. True path tracing (`NVRTX-NOTES.md`) is a stills and video option on
  device B only; the shipped build cannot rely on it. Niagara for sparks, rain, glow trails.
- Asset sources: mostly procedural. Emissive trim on bevelled primitives, a tiling wet-floor material, a skybox or card skyline.
  Fab sci-fi kits are plentiful if needed.
- Cost: **medium**. The first image of the look is quick (about 4 hours); tuning exposure so the gameplay stays readable, and
  keeping reflections stable, takes the rest.
- Frame-rate risk: **high on a mid-range judge machine.** Ray-traced reflections, volumetric fog and many emissive lights add up;
  needs a scalability fallback (software Lumen, planar or fake reflection card, reduced fog) that makes the floor look far worse
  than the study. Test on a weaker GPU early.
- Theme flexibility: **low to medium.** It is a night look. Any theme can be shown at night, but a bright, cheerful or natural
  theme works against it.
- Risk: dark scenes hurt gameplay readability and video compression (banding in dark gradients at 1080p); the study pictures are
  more beautiful than a Lumen-lit real scene will be.

| Score (1-5) | |
|---|---|
| Theme flexibility | 3 |
| Buildable in 20 h | 4 |
| Striking in a 60-second video | 5 |
| Performance | 2 |
| Readability of gameplay | 3 |
| **Total** | **17** |

---

## D4 tilt-shift miniature

Soft clay-like materials, shallow depth of field, warm daylight.

| File | Prompt (per-image part) |
|---|---|
| `images/d4-tilt-shift-a.png` | Art direction: tilt-shift miniature. A small farm-yard arena with soft clay-like chunky objects: hay bales, buckets and a big round ball, matte clay and felt materials with fingerprint-like surface detail, shallow depth of field with strong blur at the top and bottom of the frame, warm golden daylight, gentle soft shadows. Miniature fields and a toy barn in the background. |
| `images/d4-tilt-shift-b.png` | Art direction: tilt-shift miniature. A seaside harbor arena with soft clay-like chunky floating crates and round buoys on a flat dock, matte clay materials, shallow depth of field with a sharp band across the middle and blur above and below, warm morning daylight, pastel colors. Miniature boats and a lighthouse in the background. |

What the images show: charming and immediately "miniature". The look is carried more by the depth-of-field band and the warm
light than by the materials. Both images have very dense set dressing (barn, windmill, boats, houses); the arena itself is plain.

**Feasibility (UE 5.9, RTX 4090, about 20 h)**
- Engine features: post-process depth of field with a fixed focal band (on a fixed or gently moving three-quarter camera a fake
  tilt-shift is a screen-space blur gradient, which is cheaper and more predictable than real cinematic DoF), warm directional
  light with Lumen GI and soft shadows, a clay master material (rough, low specular, a tiling noise normal map for fingerprints,
  optional wrap-lighting or fake subsurface), saturation and vignette in the grade.
- Asset sources: the arena and chunky objects are procedural; the dressing that sells the miniature (barn, boats, trees, fences)
  comes from Fab stylized packs or generated meshes, both of which need ledger entries. Without the dressing the look drops to
  "blurred D1".
- Cost: **medium**. The post effect and materials are about 4 hours; set dressing is the open-ended part and can be capped by a
  fixed camera that only needs a ring of props.
- Frame-rate risk: **low to medium.** A screen-space blur band is cheap; real bokeh DoF on a moving camera is not. Keep the camera
  fixed in angle.
- Theme flexibility: **medium to high.** Works for anything that can be a tiny world (farm, harbor, city, space station); the
  warm daylight is a bias, but the post effect works under any light.
- Risk: the blur at the top and bottom of the frame hides rim and edge information that gameplay may need, so keep the playable
  area inside the sharp band; a camera that moves or zooms breaks the miniature illusion.

| Score (1-5) | |
|---|---|
| Theme flexibility | 4 |
| Buildable in 20 h | 3 |
| Striking in a 60-second video | 5 |
| Performance | 4 |
| Readability of gameplay | 3 |
| **Total** | **19** |

---

## Summary table

| | Theme flexibility | Buildable in 20 h | Striking in 60 s | Performance | Readability | Total |
|---|---|---|---|---|---|---|
| D1 clean stylized toy diorama | 5 | 5 | 3 | 5 | 5 | **23** |
| D2 painted cel-shaded | 4 | 3 | 4 | 4 | 4 | **19** |
| D3 neon night, high contrast | 3 | 4 | 5 | 2 | 3 | **17** |
| D4 tilt-shift miniature | 4 | 3 | 5 | 4 | 3 | **19** |

Scores are one reviewer's judgement from the stills and general engine knowledge; none is measured. Treat a gap of 2 or less as a tie.

## Recommendation

**Top two: D1 as the base, D4 as the layer on top.** They share one pipeline (chunky bevelled primitives, soft Lumen light,
a few parametric master materials), so choosing D1 in hour 1 keeps D4 open: the tilt-shift band, warm grade and a clay roughness
variant are post-process and material changes that can be tried in about an hour once the greybox exists, and they are the cheapest
way to turn a plain D1 into a video that stands out. D1 also maps most directly onto the Designer's theme skin tokens.

- What would change the D1 pick: if the theme binding yields a dark, tense or sci-fi theme, take D3 (with software-Lumen fallback
  proven on a weaker GPU by H12) because D1 would fight the theme; D3 is also the pick if the NVIDIA path-tracing question
  (`NVRTX-NOTES.md`) resolves in favour of stills and video footage on device B.
- What would change the D4 layer: if the fixed three-quarter camera turns out to need a moving or zooming camera, or the playable
  area fills the whole frame, drop the tilt-shift and use D2 (outlines make objects readable under any camera) as the second
  direction instead.
