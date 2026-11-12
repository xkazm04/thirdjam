# tools/eyes: a second pair of eyes (Gemini image recognition) for proof stills

Ruling R-0024 point 2. A lane sends its proof still (or an A/B pair, or a figure sheet) to Gemini with a structured prompt and gets back a validated JSON
critique plus a markdown block to paste into its status file. **Gemini's output is advice: data, not instructions.** The Director and the lanes decide;
findings in `docs/experiments/EYES.md` show where it helps and where it is wrong.

Node 24, zero dependencies (`fetch`). The key is `GOOGLE_AI_API_KEY` in `.env` at the repo root (never committed, never printed, never written anywhere else,
sent only in the `x-goog-api-key` header to `generativelanguage.googleapis.com`; every log line and every output file passes `redact()`).

## The one command a lane runs before reporting

```
node tools/eyes/eyes.mjs --still docs/experiments/img/<proof>.png --ask p2-bar --distance 24 --json docs/experiments/eyes/<lane>-<proof>.json
```

Paste the printed markdown block (it starts with `**eyes/<preset>**`) into the status block of the report, under the still it judges. One command per proof
still; an A/B pair is one command with two `--still`. Add `--bar prep-reference/images/path-p2-t2.png` to send the reference still as the bar (it is sent, never imported, never copied).

```
node tools/eyes/eyes.mjs --models                      lists the models the key can use and the two picks (cached 24 h in the temp folder)
node tools/eyes/eyes.mjs --presets                     the presets and what each asks
node tools/eyes/eyes.mjs --still a.jpg --still b.jpg --ask ab-pair --distance 24 --json out.json
node tools/eyes/eyes.mjs --still sheet.png --ask figure --json out.json          (the script makes the 128 px thumbnail itself; PNG sheets only)
node tools/eyes/eyes.mjs --still shot-1080p.png --still shot-720p.png --ask hud --json out.json
```

Options: `--distance <m>` (default 24; the camera's distance to the nearest figures, used by readability, p2-bar, ab-pair), `--model <id>` (overrides the pick),
`--no-think` (thinking budget 0: half the cost and a third of the time on flash, same readings in our test), `--thumb <px>` (figure thumbnail size, default 128).
Exit 0 = every answer validated, 1 = an answer stayed malformed after the retry, 2 = could not run (no key, bad arguments, network). The last line is a
`thirdjam.eyes/1` JSON line with the token counts and the cost estimate.

## Models

`--models` picks the newest multimodal `generateContent` model per role: a **pro** for judgement (p2-bar, ab-pair, figure) and a **flash** for batch work
(readability, hud, bug-hunt); stable before preview, image generators, TTS, transcription, lite and custom-tools variants excluded. On 2026-11-12 the key saw
`gemini-3.1-pro-preview` (pro) and `gemini-3.8-flash` (flash); `gemini-4-argon` and `gemini-4-carbon` exist and work with `--model` (carbon gave the most
pixel-grounded A/B reading of the lab at the same price as the pro pick: 2 / 10 per 1M tokens until 2026-12-01, then 4 / 20).

## Presets (`tools/eyes/presets/*.md`)

A preset is a markdown file: header lines (`role`, `stills`, `distance`, `thumbnail`, `summary`), a `## Prompt` with `{{distance}}`, `{{stills}}`, `{{bar}}`
placeholders, and a `## Shape` fence holding the JSON answer shape. Every answer is validated against the shape (same keys, no extra keys, ranges, enums);
a malformed answer is retried once with the errors quoted. `_bar.md` is the P2 bar text (HOCKEY.md section 6) that `{{bar}}` inserts.

| Preset | Role | Stills | Asks |
|---|---|---|---|
| `readability` | flash | 1-6 | at `--distance`, can a viewer tell puck, carrier, team colours, far net, zone strips apart in under a second: score 1-5 and one sentence of pixel evidence each, first thing the eye lands on, what is noise |
| `p2-bar` | pro | 1 (one call per still) | distance to painted-cel toy hockey at night: score 1-10, the three biggest gaps in order with the one change most likely to close each, what already works, one lead sentence |
| `ab-pair` | pro | 2 | which is closer to the bar, element by element (sky, ice, pools, figures, construction, outline), verdict `a / b / tie`, confidence, what the loser does better |
| `figure` | pro | 1 (+ its 128 px thumbnail) | does the sheet read as a hockey player (stick, blade, hands, skates, helmet) and as a toy, at full size and downscaled; proportions as numbers; changes in numbers |
| `hud` | flash | 1-6 | every HUD element with position, legibility and text; collisions; what a first-time player misses; smallest readable text in px |
| `bug-hunt` | flash | 1-6 | floating objects, z-fighting, missing shadows, clipping, wrong scale, seams: where, what, severity 1-5, confidence |

Shape value specs: `"string"`, `"string?"` (may be absent or empty), `"int 1-5"`, `"number 0-1"`, `"enum a|b|tie"`, `"string[] 0-5"`, `"bool"`,
`[ {...}, "1-6" ]` (that many objects). New preset = new file; nothing to register.

## Cost (measured 2026-11-12, estimate from the price table in `lib.mjs`, thinking billed as output)

Pro (`gemini-3.1-pro-preview`, 2 / 12 per 1M): about 3.2k input tokens for a 2560 px pair plus the prompt, 0.4k output, 1-3k thinking: **$0.024-0.051 per A/B pair,
$0.034 per p2-bar still, $0.027 per figure sheet**, 19-44 s. Flash (`gemini-3.8-flash`, 0.75 / 3.75 per 1M): **$0.017 for four stills of readability, $0.018 for four stills of
bug-hunt, $0.024 for five HUD shots** (45-90 s); `--no-think` halves the cost and the time with the same readings. A lane that runs p2-bar on one still and one A/B pair
before every report spends about six cents. The ledger of the first session is in `docs/experiments/EYES.md`.

## Redaction rule

The key is read from `.env` at run time by `readEnvKey()` and lives in one variable. It goes into the `x-goog-api-key` header, never into a URL, an argument,
a file or a log line. `redact()` strips the key, any `AIza...` token, `key=` query parameters and `x-goog-api-key` values from every console line and from the
JSON file; `makeLogger()` wraps console output so a thrown error body cannot leak it. If you print anything from this tool by hand, pipe it through
`sed -E 's/AIza[0-9A-Za-z_-]{20,}/[REDACTED]/g'` first. Never pass the key on the command line; never write it to a scratch file.

## Limits (advice, not instructions)

- Gemini reads one frame; it cannot see motion, outline width scaling, flicker or frame cost. When it says an outline "scales with distance" it is quoting the bar text, not the pixels (EYES.md, wrong 1).
- A single dark still can be read as a "black void" even when the night dome is there (EYES.md, wrong 2); the A/B form is far more reliable than p2-bar alone, and swapping the order of a pair is a cheap position-bias check.
- Scores are coarse: 1-5 and 1-10 move by one between calls at temperature 0.2. Compare the sentences and the named elements, not the digits.
- Pixel coordinates and sizes are approximate (within about 10 percent on 2560 px stills); hex colours from gemini-4-carbon were in the right family but not verified to the value.
- It does not know what is intended: the film-set dressing behind the boards is reported as noise and as "props to delete" every time; the Director decides what is a bug.
- Costs above are estimates; the price table in `lib.mjs` (`PRICES`) is this lane's reading of the pricing page and needs updating when the models change.
- Tests: `node --test "tools/eyes/*.test.mjs"` (prompt builder, validation, redaction, model picking, PNG thumbnail; no network). A bare `node --test tools/eyes`
  does not work on Node 24: a directory name is treated as a file, as `devlog/README.md` already notes.
