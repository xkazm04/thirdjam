# prep-reference: look studies (reference images only)

**Declaration.** The images in this folder are reference studies for choosing an art direction. They are **not game assets**.
They are never imported into any Unreal project, never used as textures, backdrops, concept overlays or tracing sources, and
never shipped. Under rule 7(h) of the contest rules only tools, prompts and scripts may exist before the start; art assets are
made inside the contest period. This folder is committed before the start (before the `pre-jam` tag) and declared in the dev log
at hour 0. Whatever look device B picks is rebuilt from scratch in the engine during the jam.

- Tool: Codex image generation (Codex CLI 0.166.0, built-in image generation tool, one image per run).
- Generation date: 2026-11-11 (about 20:54 to 21:05 CET).
- Subject (generic, genre-neutral): a physics-driven toy arena seen from a three-quarter game camera, a few chunky interactive
  objects, a clear play area, a distinct background, no characters.
- Prompts contain no brand names, no living artists' names, no logos and no real franchises.

## Files

| File | What |
|---|---|
| `STUDY.md` | the four directions: exact prompts, file names, feasibility for UE 5.9 on one RTX 4090, scores, recommendation |
| `LOOK-STUDIES.html` | the same as one page with the stills, for the operator's art-direction decision (ratings stay in the browser; the page builds a checklist line) |
| `images/d1-toy-diorama-a.png` | D1 clean stylized toy diorama, blocks on a round platform, daylight |
| `images/d1-toy-diorama-b.png` | D1, play mat with pins and cubes, sunset |
| `images/d1-toy-diorama-c.png` | D1, playroom floor with ramps and discs, window light |
| `images/d2-cel-painted-a.png` | D2 painted cel-shaded, stone courtyard, late afternoon |
| `images/d2-cel-painted-b.png` | D2, snowy wooden platform, cool palette |
| `images/d2-cel-painted-c.png` | D2, desert ruin, hard noon light |
| `images/d3-neon-night-a.png` | D3 neon night, industrial wet floor, cyan and magenta |
| `images/d3-neon-night-b.png` | D3, rainy rooftop, orange and violet |
| `images/d3-neon-night-c.png` | D3, glass-floored space arena, green trim |
| `images/d4-tilt-shift-a.png` | D4 tilt-shift miniature, clay farm yard |
| `images/d4-tilt-shift-b.png` | D4, clay harbor dock, morning |
| `tools/gen.mjs`, `tools/jobs.json` | the generator script (a tool, not content) and the exact job list; `tools/gen.log` is its log |
| `HOCKEY-LADDER.md` | the hockey ladder study: one 3 vs 3 ice hockey arena in three lighting tiers (L1 daylight toy rink, L2 night arena, L3 full neon), three variants each: exact prompts, UE 5.9 feasibility per tier, what stays constant and what changes |
| `images/hockey-l1-a.png` | hockey L1 daylight toy rink, open-air under a sunny sky, simple stands |
| `images/hockey-l1-b.png` | hockey L1, indoor community rink, big windows, warm light |
| `images/hockey-l1-c.png` | hockey L1, tabletop rink on a wooden table, window light |
| `images/hockey-l2-a.png` | hockey L2 night arena, open-air with floodlights |
| `images/hockey-l2-b.png` | hockey L2, indoor rink, house lights down, spotlights on |
| `images/hockey-l2-c.png` | hockey L2, tabletop rink in a dark room, lit by its own trim |
| `images/hockey-l3-a.png` | hockey L3 full neon, open-air, neon strips reflected in wet ice, city and hills behind |
| `images/hockey-l3-b.png` | hockey L3, indoor rink, neon trim, haze, reflections |
| `images/hockey-l3-c.png` | hockey L3, tabletop rink in a dark room, whole toy glowing, glossy table |
| `tools/gen-hockey.mjs`, `tools/hockey-jobs.json`, `tools/mkjobs.mjs` | the hockey generator (a copy of `gen.mjs` reading its own job list), the exact nine prompts, and the script that writes them; `tools/gen-hockey.log` is its log |
| `HOCKEY-LADDER.html`, `images/web/hockey-*.jpg` | the hockey ladder review page (self-contained, nine 960 px JPEG copies embedded, reviewer notes, feasibility digest, pick form that writes the checklist line) and the nine web copies it was built from; originals in `images/` are untouched |
| `STYLE-PATHS.md` | the style paths study: three art styles (P2 painted cel, P3 clay miniature, P4 stylized broadcast) x three tiers, third-person camera on a 120 by 60 m rink: exact prompts, UE 5.9 feasibility per path, scores, ladder stacking, comparison with the P1 toy baseline |
| `images/path-p2-t1.png`, `-t2`, `-t3` | P2 painted cel: regular, raw night, boost climax |
| `images/path-p3-t1.png`, `-t2`, `-t3` | P3 clay miniature: regular, raw night, boost climax |
| `images/path-p4-t1.png`, `-t2`, `-t3` | P4 stylized broadcast: regular, raw night, boost climax (P4 stills show small stick and glove marks that look like a maker mark: do not show publicly) |
| `tools/gen-paths.mjs`, `tools/path-jobs.json`, `tools/mkpaths.mjs` | the style-paths generator (a copy of `gen-hockey.mjs` reading its own job list), the exact nine prompts, and the script that writes them; `tools/gen-paths.log` is its log |
| `STYLE-PATHS.html`, `images/web/path-*.jpg` | the style-paths review page (self-contained, nine 960 px JPEG copies embedded, reviewer notes, feasibility digest, comparison with the baseline, one-pick form that writes the checklist line) and the nine web copies it was built from; originals in `images/` are untouched |

To regenerate one image: `node tools/gen.mjs <name>` (skips names whose PNG already exists); for the hockey ladder, `node tools/gen-hockey.mjs <name>`.  For the style paths, `node tools/gen-paths.mjs <name>`.

Nothing in this folder is a game asset.
