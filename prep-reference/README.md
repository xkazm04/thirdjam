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

To regenerate one image: `node tools/gen.mjs <name>` (skips names whose PNG already exists).

Nothing in this folder is a game asset.
