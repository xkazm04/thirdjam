# Lane A27: a second pair of eyes (Gemini image recognition) for every proof still (device A, from H10.3)

You are lane A27 of the Third Jam (team Hokej). Ruling R-0024 point 2 in `contracts/RULINGS.md`. The operator put a Google AI Studio key in `.env` as `GOOGLE_AI_API_KEY`.
Read first: R-0024, `docs/HOCKEY.md` (art ladder, the P2 bar), `docs/experiments/LOOK-LAB.md`, `docs/experiments/BLENDER.md`, `docs/experiments/FEEL.md` (what a still must show),
`tools/barks/generate.mjs` (how this repo reads `.env` and calls an external model without printing the key), `prep-reference/README*` if present (the reference stills: may be sent as
the bar, never imported, never copied into the repo output).

You own: `tools/eyes/**`, `docs/experiments/EYES.md`, `status/A27.md`. Node 24, zero dependencies (fetch). Run no git command; no editor; no build. **The key is never printed, never
written to any file but `.env`, never sent anywhere but `generativelanguage.googleapis.com`.** Redact it in every log line.

## Job
1. `tools/eyes/eyes.mjs`: first `--models` lists the models the key can use (`GET /v1beta/models`) and the script picks the newest multimodal model by default (prefer a `pro` for
   judgement, a `flash` for batch; both configurable with `--model`). Then: `node tools/eyes/eyes.mjs --still <png|jpg> [--still ...] --ask <preset> [--bar <reference.png>]
   [--distance 24] [--json out.json]`. Presets in `tools/eyes/presets/*.md` (prompt templates, one per question, each ending with a strict JSON answer shape):
   - `readability`: at the stated distance, can a viewer tell puck, carrier, team colours, the far net, the zone strips apart in under a second; what is the first thing the eye lands
     on; what is noise. Scores 1-5 per element plus one sentence each.
   - `p2-bar`: how far the still is from painted-cel toy hockey at night (the bar text from HOCKEY.md plus an optional reference still): the three biggest gaps in order, each with the
     one change most likely to close it, and what already works.
   - `ab-pair`: two stills, which is closer to the bar and why, element by element (sky, ice, pools, figures, construction, outline), with a verdict `a | b | tie` and a confidence.
   - `figure`: a figure sheet: does it read as a hockey player (stick, hands, skates, helmet), as a toy, at 1024 px and when downscaled to 128 px (the script makes the thumbnail);
     proportions to change, in numbers if possible.
   - `hud`: a gameplay still with HUD: what is legible at 1080p and 720p, what collides, what is missing for a first-time player.
   - `bug-hunt`: anything that looks wrong (floating objects, z-fighting, missing shadows, clipping, wrong scale).
   Every answer is validated against its JSON shape; a malformed answer is retried once with the error quoted. Output: the JSON file and a markdown block to paste into a status file.
2. Run it on what exists and write `docs/experiments/EYES.md` with the results and your own reading of where Gemini helps and where it is wrong: the look-lab pairs
   (`docs/experiments/img/look-lab/proof-* vs base2-b-* / play-b-*`), the figure sheet (`docs/experiments/img/blender-sheet.png`), A18's HUD shots (`devlog/shots/20261112-153300-A-a18-*`),
   the H7 and H9 package screenshots if any exist under `tools/qa/out/`. Keep the cost in the ledger: tokens per call, calls made, the model used (`docs/experiments/EYES.md` table).
3. `tools/eyes/README.md`: how a lane calls it before reporting (one command per proof still), what the presets ask, the redaction rule, and the limits (advice, not instructions).
4. Tests: `node --test tools/eyes` for the prompt builder, the JSON validation and the redaction (no network in tests).

Report: the model chosen, the preset list, the EYES.md findings (five most useful, two where it was wrong), cost per call, and the one-line command a lane uses. Time box 75 minutes.
