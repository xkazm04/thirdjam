# Style paths: three art styles for the hockey game, third-person camera, double-size rink (reference images only)

Status: written 2026-11-12, before the jam. **Reference studies only** (contest rule 7(h), see `README.md`): never imported into any Unreal project, never used as
textures, backdrops or tracing sources, never shipped. Whatever is chosen is rebuilt from scratch in the engine during the jam. Feasibility notes target Unreal
Engine 5.9 (stock build) on one RTX 4090, agents on device B in about 20 hours; engine feature statements are general UE 5.x knowledge, not measured and not verified
for 5.9. Hours and frame times are estimates.

## Why this study exists

The first nine stills (`HOCKEY-LADDER.md`) showed one toy look in three lighting tiers, so there was no real choice of art path. This study keeps the tier idea
(T1 regular, T2 raw night, T3 boost climax) and changes **the style** three ways, plus two framing changes that apply to every path:

- **Third-person camera**: behind and above the player's skater, over the shoulder, looking down the ice toward the far net (the earlier set used a top-down three-quarter view).
- **Arena twice as large**: 120 by 60 metres. The far net is far away; boards, markings and dressing scale with it.

The baseline for comparison is **P1 toy diorama**, the look of `HOCKEY-LADDER.md` (and D1 in `STUDY.md`). It was not re-imaged here.

## How the images were made

Same method as `HOCKEY-LADDER.md`: Codex image generation, one image per run, no reference images attached, via `tools/gen-paths.mjs` (a copy of `tools/gen-hockey.mjs`
reading `tools/path-jobs.json`; `tools/mkpaths.mjs` writes that file). Command: `node tools/gen-paths.mjs` (skips names whose PNG exists; `node tools/gen-paths.mjs path-p3-t2`
for one). Log: `tools/gen-paths.log`. All nine came out on the first run, 52 to 72 seconds each. Web copies: `images/web/path-*.jpg`, 960 px wide, under 125 KB each.
Each run received the wrapper from `gen-hockey.mjs` around the per-image prompt below, followed by this style line from `path-jobs.json`:

> Reference study for art direction only. Landscape 16:9, same third-person camera in every study. No text, no letters, no logos, no watermark, no readable numbers or letters on jerseys or boards, no real people, no real teams or leagues.

The subject sentence is identical in all nine, so the paths compare on style alone:

> Ice hockey 3 vs 3 in a huge rink 120 by 60 metres with rounded corners and boards all round, seen from a third-person camera behind and slightly above one skater's shoulder, looking down the length of the ice toward the far net; the player's skater is large in the lower foreground with a stick and a small near-black puck ahead; the other five skaters are spread down the ice; two jersey colours, deep blue and warm orange.

Each full prompt below is subject + style + tier + "No text, no letters, no logos, no readable numbers."

## The nine prompts


### P2 Painted cel

**path-p2-t1** (T1)

> Ice hockey 3 vs 3 in a huge rink 120 by 60 metres with rounded corners and boards all round, seen from a third-person camera behind and slightly above one skater's shoulder, looking down the length of the ice toward the far net; the player's skater is large in the lower foreground with a stick and a small near-black puck ahead; the other five skaters are spread down the ice; two jersey colours, deep blue and warm orange. Style, painted cel: hand-painted textures, visible ink-like outlines, flat cel lighting steps, a limited palette of five or six colours plus one accent, painted backdrop cards. Figures are chunky toy-proportioned skaters, inked and flat-shaded. Tier, regular: bright, full house lights or daylight, cheerful. No text, no letters, no logos, no readable numbers.

**path-p2-t2** (T2)

> Ice hockey 3 vs 3 in a huge rink 120 by 60 metres with rounded corners and boards all round, seen from a third-person camera behind and slightly above one skater's shoulder, looking down the length of the ice toward the far net; the player's skater is large in the lower foreground with a stick and a small near-black puck ahead; the other five skaters are spread down the ice; two jersey colours, deep blue and warm orange. Style, painted cel: hand-painted textures, visible ink-like outlines, flat cel lighting steps, a limited palette of five or six colours plus one accent, painted backdrop cards. Figures are chunky toy-proportioned skaters, inked and flat-shaded. Tier, raw night: lights down, spot cones, emissive trim, scuffed ice, one tipped-over figure. No text, no letters, no logos, no readable numbers.

**path-p2-t3** (T3)

> Ice hockey 3 vs 3 in a huge rink 120 by 60 metres with rounded corners and boards all round, seen from a third-person camera behind and slightly above one skater's shoulder, looking down the length of the ice toward the far net; the player's skater is large in the lower foreground with a stick and a small near-black puck ahead; the other five skaters are spread down the ice; two jersey colours, deep blue and warm orange. Style, painted cel: hand-painted textures, visible ink-like outlines, flat cel lighting steps, a limited palette of five or six colours plus one accent, painted backdrop cards. Figures are chunky toy-proportioned skaters, inked and flat-shaded. Tier, boost climax: a saturated poster-like glow with painted light rays and glowing boost pickups (bright capsules and rings) on the ice. No text, no letters, no logos, no readable numbers.


### P3 Clay miniature

**path-p3-t1** (T1)

> Ice hockey 3 vs 3 in a huge rink 120 by 60 metres with rounded corners and boards all round, seen from a third-person camera behind and slightly above one skater's shoulder, looking down the length of the ice toward the far net; the player's skater is large in the lower foreground with a stick and a small near-black puck ahead; the other five skaters are spread down the ice; two jersey colours, deep blue and warm orange. Style, clay miniature: matte clay and felt materials with fingerprint detail, soft rounded forms, warm bounce light, a hint of tilt-shift depth of field at the far end. Figures are clay toys with a stitched jersey look. Tier, regular: bright, full house lights or daylight, cheerful. No text, no letters, no logos, no readable numbers.

**path-p3-t2** (T2)

> Ice hockey 3 vs 3 in a huge rink 120 by 60 metres with rounded corners and boards all round, seen from a third-person camera behind and slightly above one skater's shoulder, looking down the length of the ice toward the far net; the player's skater is large in the lower foreground with a stick and a small near-black puck ahead; the other five skaters are spread down the ice; two jersey colours, deep blue and warm orange. Style, clay miniature: matte clay and felt materials with fingerprint detail, soft rounded forms, warm bounce light, a hint of tilt-shift depth of field at the far end. Figures are clay toys with a stitched jersey look. Tier, raw night: lights down, spot cones, emissive trim, scuffed ice, one tipped-over figure. No text, no letters, no logos, no readable numbers.

**path-p3-t3** (T3)

> Ice hockey 3 vs 3 in a huge rink 120 by 60 metres with rounded corners and boards all round, seen from a third-person camera behind and slightly above one skater's shoulder, looking down the length of the ice toward the far net; the player's skater is large in the lower foreground with a stick and a small near-black puck ahead; the other five skaters are spread down the ice; two jersey colours, deep blue and warm orange. Style, clay miniature: matte clay and felt materials with fingerprint detail, soft rounded forms, warm bounce light, a hint of tilt-shift depth of field at the far end. Figures are clay toys with a stitched jersey look. Tier, boost climax: strings of fairy lights and glowing clay boost pickups (capsules and rings) in a dark room. No text, no letters, no logos, no readable numbers.


### P4 Stylized broadcast

**path-p4-t1** (T1)

> Ice hockey 3 vs 3 in a huge rink 120 by 60 metres with rounded corners and boards all round, seen from a third-person camera behind and slightly above one skater's shoulder, looking down the length of the ice toward the far net; the player's skater is large in the lower foreground with a stick and a small near-black puck ahead; the other five skaters are spread down the ice; two jersey colours, deep blue and warm orange. Style, stylized broadcast: semi-realistic physically based materials, glossy boards, real ice with scrape marks, glass panels, arena rig lighting, lens flare and bloom, a sports-broadcast feel, stylized proportions. Figures are stylized athletes with helmets and visors, proportions between toy and real. Tier, regular: bright, full house lights or daylight, cheerful. No text, no letters, no logos, no readable numbers.

**path-p4-t2** (T2)

> Ice hockey 3 vs 3 in a huge rink 120 by 60 metres with rounded corners and boards all round, seen from a third-person camera behind and slightly above one skater's shoulder, looking down the length of the ice toward the far net; the player's skater is large in the lower foreground with a stick and a small near-black puck ahead; the other five skaters are spread down the ice; two jersey colours, deep blue and warm orange. Style, stylized broadcast: semi-realistic physically based materials, glossy boards, real ice with scrape marks, glass panels, arena rig lighting, lens flare and bloom, a sports-broadcast feel, stylized proportions. Figures are stylized athletes with helmets and visors, proportions between toy and real. Tier, raw night: lights down, spot cones, emissive trim, scuffed ice, one tipped-over figure. No text, no letters, no logos, no readable numbers.

**path-p4-t3** (T3)

> Ice hockey 3 vs 3 in a huge rink 120 by 60 metres with rounded corners and boards all round, seen from a third-person camera behind and slightly above one skater's shoulder, looking down the length of the ice toward the far net; the player's skater is large in the lower foreground with a stick and a small near-black puck ahead; the other five skaters are spread down the ice; two jersey colours, deep blue and warm orange. Style, stylized broadcast: semi-realistic physically based materials, glossy boards, real ice with scrape marks, glass panels, arena rig lighting, lens flare and bloom, a sports-broadcast feel, stylized proportions. Figures are stylized athletes with helmets and visors, proportions between toy and real. Tier, boost climax: full neon strips, wet reflective ice, haze, glowing boost pickups (capsules and rings) on the ice. No text, no letters, no logos, no readable numbers.


## What the stills show

What the generator did that a real build would not (reviewer notes with chips are on `STYLE-PATHS.html`):

- **Camera and net.** Every still keeps the far net in frame and the puck ahead of the skater, which is the point of the framing. On a 120 m rink the far net is small
  in every image; in P4 T1 it is a few pixels wide and is found only because the boards converge on it.
- **Skater count.** P3 T1 and T2 draw a goalie in the far net, which the brief did not ask for; the engine build decides the roster.
- **P2 skaters are not chunky toys.** The generator drew inked sports figures with athlete proportions, closer to P4 than to P1. To keep the baseline's toy proportions
  in the painted look, the figure mesh would have to be shorter and wider than these stills.
- **Brand-like marks.** P4 T3 (faintly P4 T1 and T2) shows small blue lettering-like marks on the stick shaft and glove cuff. Nothing legible, but it looks like a stick
  maker's mark. Do not show P4 stills publicly without cropping or regenerating. P4 T1 also has a dense crowd of tiny people (no identifiable person). No real team, landmark or
  logo was seen in the other images; the mountains, pines and trees are generic.
- **Jersey blue drifts.** P3 T3's jersey reads violet under the warm light; P4 T3's blue jerseys sit near blue neon and lose some separation.

## Feasibility per path (UE 5.9, RTX 4090, agents, about 20 h of device-B time)

All three share the 120 by 60 m rink: boards, glass and lines are tiled modular pieces and decals, so double size costs little geometry. It costs **dressing**
(stands, backdrops, props along 360 m of boards) and **camera craft**. A chase camera behind the skater needs a spring arm with lag, a field of view of about 70 to 80 degrees,
a **puck marker** and a **far-net beacon**. Build those in the shared kit regardless of style: in every still the puck and the far net are what the player must find.

### P2 Painted cel

- **What carries the look.** A toon ramp in the master materials (two to three lighting steps through a lookup texture or a stepped lighting function), a post-process
  outline (depth plus normal edge detect, one post-process material) and painted textures in a small palette. Painted backdrop cards at a distance are cheap and hide the size of the arena.
- **Third-person effects.** Outlines make figures pop against pale ice at any distance: the best figure and puck separation of the three. The puck needs a thicker outline or
  a coloured ring to survive the far distance. The far net reads as a red outlined box. A painted sky card removes the need for a real stadium.
- **Cost.** About 8 h of the 20: toon master and outline post 3 h, painted textures and decals 3 h (agents paint with procedural noise, not brushwork), backdrop cards and dressing 2 h.
- **Frame-rate risk.** Low. One full-screen post pass plus simple materials; should hold 60 fps on a GTX 1660 at 1080p. Drop Lumen to fixed ambient to be safer.
- **Bends to an unknown theme.** Good. Palette swap and backdrop cards re-theme everything; outlines unify any mesh agents add late.
- **Designer fit.** High. Palette, ramp steps, outline width and colour and the backdrop set are parameters in one material parameter collection. About 80 percent of the look is parameters.

### P3 Clay miniature

- **What carries the look.** Clay and felt are **texture** work: a fingerprint normal map, felt fuzz via a fresnel-driven fake fur (shell cards or a rim-light trick),
  stitched seams as decals or a baked normal, high roughness, warm bounce light, and depth of field. The tilt-shift is a cinematic depth of field whose focal distance tracks the player.
- **Third-person effects.** Depth of field softens the far end, which is exactly where the far net is. On a 120 m rink the net would blur by design. Set the focal distance
  between the player and the puck and cap the blur at the net, or the lens effect cannot stay on.
- **Cost.** About 12 h: fingerprint and felt materials 4 h, figure meshes with soft forms and stitch detail 4 h, fairy-light strings and pickups 2 h, depth of field and puck visibility 2 h.
  The hand-made feel needs asset quality that agents reach with difficulty.
- **Frame-rate risk.** Medium. Cinematic depth of field is a full-screen cost; shell fur on six figures is cheap; many small unshadowed emissive bulbs in T3 are cheap.
  A GTX 1660 should hold 60 fps with a simple (not bokeh) depth of field.
- **Bends to an unknown theme.** Medium to poor. Clay and felt suit a warm toy theme and fight a sci-fi or horror theme; a retheme needs new textures.
- **Designer fit.** Medium. Colour, roughness, depth-of-field strength and light colour are parameters; the clay character is in textures and meshes, which are not.

### P4 Stylized broadcast

- **What carries the look.** Stock UE PBR: a glossy ice material with scrape decals, glass panels, rig-light actors and spots, bloom and lens flare in a post volume,
  Lumen with hardware ray tracing for ice reflections. The heavy part is **dressing**: stands with crowds, a truss rig, scoreboard-like geometry (no text allowed),
  a broadcast colour grade. The stills show nearly photographic skaters, so the look is "real" more than "stylized"; holding it stylized is itself work.
- **Third-person effects.** Best match for a third-person sports feel: the low camera, flares and a reflective ice plane feel like a broadcast. The costs at 120 m: the stands
  must run 360 m, the far net is tiny in the frame, and reflections on a huge plane are a full-screen cost. It needs the strongest puck highlight and net beacon.
- **Cost.** About 14 h and the highest risk of not finishing: athlete figures with helmets, visors and animation 5 h (the weakest point for agents), stands and crowd
  instancing 3 h, rig lights and post 2 h, ice and glass materials 2 h, reflection fallback 2 h.
- **Frame-rate risk.** High. The reflection and haze issues of L3 in `HOCKEY-LADDER.md`, now present in T1 because glossy ice is part of the base look. A GTX 1660 has no RT hardware
  and an RTX 3060 likely falls under 60 fps at 1080p with hardware RT and haze. Needs a reflection quality switch from the start.
- **Bends to an unknown theme.** Medium. A broadcast look carries sports and arena themes; other themes need new dressing and new athletes.
- **Designer fit.** Medium to low. Light colours, bloom and grade are parameters, but the look is carried by assets (stands, athletes, glass), which are not.

## Scores (1 to 5, higher is better; performance 5 is cheap, readability 5 is clear)

| Score (1-5) | P1 toy diorama (baseline) | P2 Painted cel | P3 Clay miniature | P4 Stylized broadcast |
|---|---|---|---|---|
| Theme flexibility | 5 | 4 | 3 | 3 |
| Buildable in 20 h | 5 | 4 | 2 | 2 |
| Striking in a 60-second video | 3 | 4 | 5 | 5 |
| Performance | 5 | 5 | 3 | 2 |
| Readability (puck, teams, far net, third person) | 4 | 4 | 3 | 3 |
| **Total** | **22** | **21** | **16** | **15** |

P1 row: the scores of D1 in `STUDY.md` (23), with readability lowered one point because the baseline was never imaged from the third-person camera on the large rink.

## How each ladder stacks without rebuilding

- **P2:** one toon master and one outline post pass from T1; T2 swaps the key light and raises emissive trim; T3 adds ray cards, pickup glow and saturation as parameters. No rebuild.
- **P3:** one clay and felt material set from T1; T2 dims the room and enables spot cones and trim; T3 adds fairy-light strings and glowing pickups. No rebuild, but the depth-of-field focus rule must be in T1.
- **P4:** one PBR set and glossy ice from T1; T2 turns house lights down and spots up; T3 adds neon strips, haze and wet ice. It stacks, but T1 already pays the reflection cost, so the ladder saves less than in P2 or P3.

## Closing comparison, with the baseline as a row

| | Look carried by | Cost of 20 h | Frame-rate risk | Theme bend | Designer fit | Readability at 120 m, third person | Total |
|---|---|---|---|---|---|---|---|
| P1 toy diorama (baseline) | flat-ish materials, soft light, rim light | about 5 h of look work | low | high | high | clear in the earlier top-down set, not imaged here | 22 |
| P2 Painted cel | toon ramp, outline post, painted cards | about 8 h | low | high | high | best figure and puck separation | 21 |
| P3 Clay miniature | textures, depth of field, warm light | about 12 h | medium | low to medium | medium | far end blurred by design | 16 |
| P4 Stylized broadcast | PBR, reflections, rig lights, dressing | about 14 h | high | medium | medium to low | far net tiny, glare competes | 15 |

Reading it: P3 and P4 look best in the stills and cost most. P2 looks clearly different from the baseline at a similar cost and keeps puck and team readability.
P1 is the cheapest and nothing in these stills shows it breaking under the new framing. A costlier path is worth it only if the operator values the look over the finish risk.
Order of risk: P1, P2, P3, P4.

Nothing in this folder is a game asset.
