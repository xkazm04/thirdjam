# Eyes A27: Gemini as a second pair of eyes on the proof stills (device A, H10.9-H11.3, 2026-11-12)

Ruling R-0024 point 2. Tool: `tools/eyes/eyes.mjs` (README there). Every call below was made at 17:01-17:11 with the key from `.env`, sent only in the
`x-goog-api-key` header; the answers are in `docs/experiments/eyes/*.json` (validated JSON, the markdown block, token counts). I looked at every still
myself after reading the answers; "wrong" below means wrong against the pixels or against the lab's measurements in `LOOK-LAB.md`, not against taste.

## Models

`--models` lists 36 `generateContent` models for this key; after dropping image generators, TTS, transcription, lite, omni and custom-tools variants, 12 critics.
Picks: **`gemini-3.1-pro-preview`** for judgement (p2-bar, ab-pair, figure) and **`gemini-3.8-flash`** for batch (readability, hud, bug-hunt). `gemini-4-carbon` and
`gemini-4-argon` are newer (and named outside the pro/flash scheme), priced like the pro pick until 2026-12-01; one probe with carbon gave the most pixel-grounded
A/B reading of all (hex colours, coordinates, two real artefacts nobody had listed) at 52 s. A lane that wants the strongest read passes `--model gemini-4-carbon`.

## Ledger (prices read from aistudio.google.com/docs/pricing at 17:12; thinking tokens billed as output; prompt tokens include the images, about 1.3k per 2560 px still)

| File (docs/experiments/eyes/) | Preset | Model | Stills | Calls | Prompt tok | Output tok | Thinking tok | Cost (est.) | Time |
|---|---|---|---|---|---|---|---|---|---|
| ab-faceoff-24m.json | ab-pair | gemini-3.1-pro-preview | 2 | 1 | 3219 | 369 | 1107 | $0.0242 | 20.8 s |
| ab-play-24m.json | ab-pair | gemini-3.1-pro-preview | 2 | 1 | 3215 | 391 | 1320 | $0.0270 | 18.8 s |
| ab-faceoff-40m-swapped.json | ab-pair | gemini-3.1-pro-preview | 2 | 1 | 3219 | 380 | 3349 | $0.0512 | 44.0 s |
| ab-faceoff-24m-g4carbon.json | ab-pair | gemini-4-carbon | 2 | 1 | 3315 | 559 | 2804 | $0.0403 | 52.1 s |
| p2bar-24m.json (proof) | p2-bar | gemini-3.1-pro-preview | 1 | 1 | 3135 | 360 | 1946 | $0.0339 | 21.7 s |
| p2bar-24m.json (B's look) | p2-bar | gemini-3.1-pro-preview | 1 | 1 | 3135 | 367 | 2037 | $0.0351 | 26.2 s |
| figure-blender-sheet.json | figure | gemini-3.1-pro-preview | 1 (+128 px thumb) | 1 | 3008 | 513 | 1261 | $0.0273 | 24.1 s |
| readability-play.json | readability | gemini-3.8-flash | 4 | 1 | 5322 | 1427 | 2136 | $0.0174 | 46.6 s |
| readability-play-nothink.json | readability | gemini-3.8-flash | 4 | 1 | 5322 | 1485 | 0 | $0.0096 | 18.3 s |
| hud-a18.json | hud | gemini-3.8-flash | 5 | 2 (1 retry) | 14609 | 5589 | 4514 | $0.0488 | 88.5 s |
| bughunt.json | bug-hunt | gemini-3.8-flash | 4 | 1 | 5191 | 371 | 3385 | $0.0180 | 46.1 s |
| **Total** | | | 28 | 12 | 52690 | 11811 | 23859 | **$0.33** | 7.0 min |

Per call: **a pro A/B pair $0.024-0.051** (thinking varies 1.1k-3.3k tokens with the pair), **a pro single still $0.034**, **a figure sheet $0.027**,
**flash on four stills $0.010 (no thinking) to $0.018**, five HUD shots $0.024 per call. The HUD retry was my validator's fault (an optional `text` field sent as `""`
was rejected; fixed, so `string?` accepts empty), not the model's. The JSON files carry a `cost_usd_est` computed with my first price table (flash at 0.5 / 3.0);
the table above is recomputed at the page's prices. A lane spending one p2-bar and one A/B pair per report pays about six cents.

## What was judged

- Look-lab pairs: `base2-b-24m` vs `proof-faceoff-24m` (A = B's look, B = lab proof), `play-b-24m` vs `proof-play-24m`, and `proof-faceoff-40m` vs `base2-b-40m` with the
  **order swapped** as a position-bias check; p2-bar on `proof-faceoff-24m` and `base2-b-24m` with `prep-reference/images/path-p2-t2.png` as the reference.
- Readability on `proof-play-10/24/40m` and `play-b-24m` (with and without thinking).
- The figure sheet `blender-sheet.png` (3072 px, three rows toy / clay / broadcast) with its 128 px thumbnail made by the script.
- A18's HUD shots from 15:33: `hud-play-1080p`, `hud-faceoff-720p`, `hud-controls-720p`, `call-card-1080p`, `hud-effects-bark-1080p`.
- Bug hunt on `proof-play-10m`, `base2-b-10m`, the QA play shot `tools/qa/out/play-2/shots/hud-16-carry-f1564.png` (1275 x 700, the only H7/H9 package-era
  screenshots that exist; there is no `tools/qa/out/h7` or `h9` folder) and `a18-hud-play-1080p`.

## Results in one paragraph each

**A/B pairs.** All three pro verdicts pick the lab proof over B's current look (confidence 0.8, 0.95, 0.9), and the swapped pair still picks the proof, so there is no
position bias at this confidence. Element by element it agreed with the lab on the sky (dome over void), the ice (brush streaks over cloudy noise), the pools
(painted band edges) and the construction (kick plate and rail), called the figures a tie (same meshes), and was inconsistent on the outline (below). Its "what the
loser keeps": B's quieter ice lets the puck glow pop; B's constant outline is a clean graphic line at 40 m. Carbon added two real artefacts in the proof: a cyan
rectangle on the ice at about x 1750, y 400 (it is in the still) and black dashed "comb" ticks along the far-corner boards at x 640-760 and 1760-1910: those are the lab's
glass-rail stanchions (5 cm every outline segment) reading as a comb at 24 m, which the lab did not report.

**p2-bar.** Proof 3/10, B's look 2/10. Gap 1 in both: the figures are capsules, not toys (big hands, visible blade, round head); the fix it names is the one R-0024 point 1
already took (A22's Blender toy goes on the ice). Gap 2: the pools are "harsh concentric rings", soften the penumbra: this is the cel ramp doing its job on the ice and
is a taste call for the Director (the lab chose it; Gemini calls it noise every single time, in every preset). Gap 3 was wrong for the proof (see below). "Works": the
jersey colours separate, no crowd or goalies, the far net is red and placed. The lead sentence both times: "this still looks like a greybox blockout."

**Readability (flash).** Puck 4 / 3 / 2 at 10 / 24 / 40 m on the proof, 3 on B's 24 m; carrier 4 / 3 / 3; team colours 5 everywhere; far net 4 / 4 / 3; zone strips 5. It
gives the puck's pixel position and width each time (16, 8, 4 px) and names the two foreground blue figures as what steals the eye at 24 m. Noise, every still: the
stepped pool rings, the far-net beacon column, and the film-set dressing (crane, lamps, clapperboard) behind the boards. The no-thinking run gave the same scores
within one point and the same noise list at half the cost and a third of the time: **use `--no-think` for readability and bug-hunt.**

**Figure.** Hockey player 4 at full size, 3 at 128 px; toy 4 / 4; best row the toy row "because the legless cylindrical body reads as a peg toy, the broadcast row is
spindly and mannequin-like": the same choice A22 and the Director made. Parts seen: stick, blade, hands, skates, helmet all true. Numbers: head 0.25 of height
(BLENDER.md's toy is 49 cm on 160 cm = 0.31; Gemini under-reads it), shoulders 0.40, stick 0.9 of height; changes: head to 0.35, a grey blade line under the black
skate blocks, a longer thinner stick blade ("a mallet" now), a cuff or thumb on the sphere hands. Worst three: skates read as boots or stands, blade too short, hands
featureless. All four are visible in the sheet and none is in BLENDER.md's open list: this is the most useful single answer of the session.

**HUD (flash).** It transcribed every element right at 1080p and 720p (score plates, "PERIOD 1: FIRST TO 3", net markers with distances, the bark line, the countdown
digit, the controls strip, the Director's Call card with its five sliders and values, "ESC / START DIRECTOR'S CALL READY", the radar). Collisions it found: the own-net
marker flush against the right screen edge; the countdown reticle drawn over both centres; the bark shown twice (bubble and bottom strip, same text); the call card
hiding the own-net marker. Missing for a first-time player, in all five stills: nothing says which skater is yours, and nothing says how to win the faceoff.
Smallest text 9 px at 720p, 11-12 px at 1080p.

**Bug hunt (flash).** Both look-lab 10 m stills clean. QA play shot: the far-net beacon renders as a black-and-white dithered column (true, it is the translucent beacon
under TSR at 1275 px), a blue skater "clipping into the board" (it is leaning at the board edge, plausible but not provable from one frame), and "two blurred blue blocks
floating top-left without shadows, orphaned LODs" (they are the two blue teammates under motion blur; wrong label, right that it looks odd). A18 play shot: the emissive
trim has a step where the straight board meets the corner curve at about x 1300-1340, y 370 (true, visible, severity 2); "orange #13 submerged a third into the board"
(the figure is in front of the board; its lean makes the silhouette cross the trim line, a false positive at confidence 0.95); sticks of the two checking figures
intersect (true).

## Five most useful findings

1. **The figure sheet's four fixes in numbers** (head 0.25 to 0.35 of height, a blade line under the skates, a longer thinner stick blade, cuffs on the hands): concrete, visible in
   the sheet, not in BLENDER.md's list, and the row choice matches the Director's. Hand to A26 / A22.
2. **The glass-rail stanchions read as a black comb at 24 m** (carbon, A/B pair) and **the emissive trim steps at the corner seam** (flash, bug-hunt, A18 play shot): two
   construction artefacts the lab and A18 did not list; both are in the pixels. Hand to A23's handoff to A (RinkActor bands) and to B's rig.
3. **The far-net beacon column is noise in every still and a dithered artefact at 720p-class resolutions**: four presets flagged it independently. HOCKEY.md already says
   "brighter frame useless past 60 m" and the screen marker exists; the beacon is a candidate to drop or to render opaque.
4. **The HUD misses two things for a first-time player** in all five shots: which skater is yours, and how a faceoff is won; plus the bark is drawn twice (bubble and strip) and
   the own-net marker sits on the screen edge. Hand to A18.
5. **The A/B form is reliable, the single-still form is not**: three pairs plus a swapped pair gave the same winner with the same element reasons, and carbon's reading
   added pixel positions. Lanes should report an A/B pair (before / after) rather than a lone still whenever they have both frames.

## Two where Gemini was wrong

1. **The outline.** At 24 m it said B's look "largely lacks" an ink outline and the proof has one; at 40 m (order swapped) it said B's outline is "thick, ink-like, properly
   scaling down with distance" and the proof's is "hairline". The lab measured both: B is a constant 1.5 px with a distance fade, the proof is 3 px to 1.5 px by distance; at
   40 m they are the same width. Neither claim is in the pixels; "scales with distance" is the bar text's own phrase quoted back as evidence. One frame cannot show scaling.
   Treat every outline sentence from a single frame as a guess.
2. **The sky on the proof (p2-bar).** Gap 3 for `proof-faceoff-24m`: "the background is a black void ... add a deep-blue unlit sky dome." The proof has the dome (horizon
   0.05, 0.09, 0.22; the still shows a dark navy with cloud texture), and the same model, given the pair, correctly called it "a textured deep-blue dome" against B's
   "flat black void". Alone, a dark navy reads as black to it; next to black, it reads as blue. The reference image did not stop this (the reference's sky is far brighter).
   Also false: "three harsh pools instead of four spots" (four spots, three in frame) and the figures "without heads" (they have a helmet cap; the point about toy
   proportions stands).

Also noted: it treats B2's film-set dressing (crane, lamps, clapperboard) as props to delete, in every preset; by R-0021 the film theme is menu-only on the ice, so the
Director may agree, but Gemini does not know that and should not be the reason.

## Where it helps and where it does not (my reading)

- Helps: transcription and inventory (HUD text, elements, positions), counting and measuring in pixels (puck width per distance), comparisons of two frames of the same
  scene, figure-sheet proportions, and spotting small artefacts a tired eye skips (seams, combs, dither). The flash model is enough for all of that; thinking adds cost, not findings.
- Does not: judging anything temporal (outline scaling, flicker, motion blur: it invents a story), absolute mood from one dark still (void vs dome), taste the team already
  decided (stepped pools), and intent (dressing, beacon). Scores move by a point between calls; read the sentences.
- Process: run it on an A/B pair, paste the block, and let the Director read the element lines, not the verdict digit. Six cents per report.
