# B-images-n1: generated 2D art, batch 1 (lane N1, device B)

Everything lives in `C:/Users/kazda/jam/image-lab/` (outside any repository). Nothing was imported into the project; only this file is in the repo.
Triage page: `C:/Users/kazda/jam/image-lab/pick.html` (open it in a browser; radio per group, click again to clear, "Copy my picks").

## Generator

`cd C:/Users/kazda/jam/image-lab && node mkjobs.mjs && node gen.mjs --par=4` (`gen.mjs` is a copy of `prep-reference/tools/gen.mjs`, original untouched). It runs `node ...@openai/codex/bin/codex.js exec --skip-git-repo-check -s workspace-write -o last_message.txt -C <work dir> -- "<prompt>"`, one image per `codex` run, with `stdio: ['ignore','pipe','pipe']`. Jobs in `jobs.json`, log in `gen.log`.

What happened first (my bug, not a Codex failure): my first version spawned `codex` with an open stdin pipe, so it printed "Reading additional input from stdin..." and waited. It was killed after 10 minutes (14:06 to 14:16 local, `gen-attempt1-stdin-hang.log`), no image produced. Fix: stdin ignored; then 47 s per image, 4 in parallel. Total: 28 runs (1 hung, 27 images), well under the cap of 45; no other failure. The original `gen.mjs` path `C:/nvm4w/...` does not exist on this machine; the codex package is under `C:/Users/kazda/AppData/Roaming/npm/node_modules/@openai/codex/bin/codex.js` (version 0.166.0).

Tool size limits: backdrops and number sheets came out 2172 x 724 (3:1, the widest the tool gave); square images 1254 x 1254. Neither reaches 2K, so the engine will not downscale them.

## Every image

All files are in `C:/Users/kazda/jam/image-lab/images/`. The prompt is in `jobs.json` (name = file name without `.png`); the shared style suffix is the `style` field. I looked at every image (the glyphs and zones through two contact sheets).

| File | Purpose | Pixels | Bytes | Verdict |
|---|---|---|---|---|
| backdrop-a-hills.png | backdrop | 2172 x 724 | 1566284 | kept: clean, dark, centre empty; amber cloud-bushes only at far edges |
| backdrop-b-city.png | backdrop | 2172 x 724 | 1464278 | kept with caution: centre empty and dark; the tall stepped-spire towers at both edges are close to a real skyline icon, generic enough but not recommended |
| backdrop-c-lake.png | backdrop | 2172 x 724 | 1370978 | kept: dark, symmetrical, centre empty; amber glow only at the edges |
| crest-blue-comet.png | crest-blue | 1254 x 1254 | 1363825 | kept: clear, 4 flat colours, reads small |
| crest-blue-snowflake.png | crest-blue | 1254 x 1254 | 1424628 | kept: reads, but plain and like a weather icon; white outer ring, so keying needs care |
| crest-blue-owl.png | crest-blue | 1254 x 1254 | 1367565 | kept: strong silhouette; generic owl |
| crest-orange-flame.png | crest-orange | 1254 x 1254 | 1325818 | kept: reads; generic flame icon |
| crest-orange-fox.png | crest-orange | 1254 x 1254 | 1394733 | kept: strong, reads small; generic fox head |
| crest-orange-sun.png | crest-orange | 1254 x 1254 | 1345703 | REJECTED: navy disc, 12 triangular rays and a solid centre disc echo a real national emblem too closely |
| numbers-a-block.png | numbers | 2172 x 724 | 1287010 | kept: legible; inner counters of 2 and 3 have stray dark blobs |
| numbers-b-varsity.png | numbers | 2172 x 724 | 1316629 | kept: clean, ten digits in order, even; slanted varsity |
| glyph-speed-a.png | glyph-speed | 1254 x 1254 | 1263930 | kept: reel with speed lines, reads small |
| glyph-speed-b.png | glyph-speed | 1254 x 1254 | 1404029 | kept: busier at 64 px |
| glyph-precision-a.png | glyph-precision | 1254 x 1254 | 1306981 | kept: lens with crosshair, as asked |
| glyph-precision-b.png | glyph-precision | 1254 x 1254 | 1256442 | kept: crosshair only, no lens |
| glyph-heavy-a.png | glyph-heavy | 1254 x 1254 | 1514242 | kept: canister, reads as a tin rather than heavy |
| glyph-heavy-b.png | glyph-heavy | 1254 x 1254 | 1371901 | kept: canisters over a weight; a tiny film-strip pictogram on the lid, not text |
| glyph-big-a.png | glyph-big | 1254 x 1254 | 1265924 | kept: spotlight with beam |
| glyph-big-b.png | glyph-big | 1254 x 1254 | 1404775 | kept: front-on spotlight with rays |
| glyph-reverse-a.png | glyph-reverse | 1254 x 1254 | 1190969 | kept: blank clapperboard reads like a screen, "flipped" unclear |
| glyph-reverse-b.png | glyph-reverse | 1254 x 1254 | 1330268 | kept: clapperboard with U-turn arrows, reads as reverse |
| zone-slow-a.png | zone-slow | 1254 x 1254 | 2806095 | kept: rough frosted ice; orange flecks look like embers; seam when tiled |
| zone-slow-b.png | zone-slow | 1254 x 1254 | 2078696 | kept: matte frosted slush, mild seam |
| zone-fast-a.png | zone-fast | 1254 x 1254 | 1363280 | kept: vertical polished streaks; strong left-right seam |
| zone-fast-b.png | zone-fast | 1254 x 1254 | 1395717 | kept: horizontal streaks, tiles cleanly in both axes |
| zone-bumper-a.png | zone-bumper | 1254 x 1254 | 1668770 | kept: clean chevrons; strong top-bottom seam, crop to a whole chevron period |
| zone-bumper-b.png | zone-bumper | 1254 x 1254 | 1654920 | kept: busier ridges; strong top-bottom seam |

No image contains text, letters or numbers where none was asked (the number sheets contain exactly the digits 0 to 9). Seam numbers: mean abs difference between opposite edges vs. between neighbouring pixels (slow-a 15.1 vs 8.9; slow-b 7.2 vs 3.8; fast-a 28.3 vs 5.4; fast-b 2.0 vs 0.6 and 5.6 vs 5.8; bumper-a top/bottom 77.8 vs 2.9). "Tileable-looking" was asked for; only fast-b is actually seamless.

## Picks

- **Backdrop: a (hills)**, or c (lake) as the second. Both have a dark, empty centre; a also has the stars from the stills the style was chosen on. b has the skyline-icon caution.
- **Blue crest: comet** (reads best at 64 px, most distinctive; owl is the second).
- **Orange crest: fox** (strongest silhouette); flame second. Sun rejected.
- **Number sheet: b (varsity)**: cleaner than a.
- **Glyphs:** speed a, precision a, heavy b, big a, reverse b (the others fail the "reads at 64 px" or "reads as the idea" test, see table).
- **Zones:** slow b (matte, only mild seam), fast b (tiles), bumper a (needs the crop). If the zone tiles are used as stamped patches rather than tiled, slow a is the more striking rough ice.

## Use in the engine (rules of A-to-B-0004)

| Image | Size after rules | Alpha | Takes it |
|---|---|---|---|
| backdrop-a-hills (or c) | 2048 x 683 is not allowed (power of two preferred): resample to 2048 x 1024 with a 3:1 UV crop, or import as is at `MaxTextureSize` 2048; BC1 + RDO Medium, no alpha | no | the existing backdrop flat materials (end-wall `MI_*` flats), emissive/base colour scaled by the tested `Brightness` 4.5 |
| crest-blue-comet, crest-orange-fox | 512 (props: 1024 cap; 512 is enough for a jersey chest and a UI badge); need alpha: key the magenta out and import BC7 + RDO Medium (or BC1 if a round-mask in the material is used) | yes | skater jersey material (UV0 chest area) and device A's HUD/menu widgets (A's UI) |
| numbers-b-varsity | 1024 wide atlas slice (cut into ten cells); key the grey to alpha, BC7 + RDO Medium, or keep opaque and use luminance as mask in BC1 | yes or mask | skater back-number decal (UV0) and the scoreboard digits, if A wants them; UI is device A's |
| glyph-speed-a, precision-a, heavy-b, big-a, reverse-b | 256 (they read at 64 px; no bigger needed); key the dark grey-green background to alpha, BC7 + RDO Medium | yes | pickup billboards in the rink (a translucent/masked unlit material) and the HUD effect icons (A's UI) |
| zone-slow-b, zone-fast-b, zone-bumper-a (cropped) | 1024, BC1 + RDO Medium, `LODBias 0`, sRGB on | no | the zone-strip material over the ice (A15 renders zones; B supplies the material and tile) |

Texture count: 12 files, about 0.3 to 0.6 MB each after BC1+RDO at 1K, under 6 MB all told; nothing here threatens the 350 MB art line. `bFinalUsesRDO` is still needed in device A's config (A-to-B-0004).

## Ledger rows (for the recommended picks; paste, then fix `size_mb` after the import and cook)

Prompt file for all rows: `C:/Users/kazda/jam/image-lab/jobs.json` (copy it to `briefs/` or `tools/images/` on import, per R-0008). Format follows C-0004: kind, name, source type, tool, url.

| kind | name | source | tool | url (prompt file) |
|---|---|---|---|---|
| texture | T_Backdrop_Hills | ai-generated | Codex image generation | image-lab/jobs.json#backdrop-a-hills |
| texture | T_Crest_Blue_Comet | ai-generated | Codex image generation | image-lab/jobs.json#crest-blue-comet |
| texture | T_Crest_Orange_Fox | ai-generated | Codex image generation | image-lab/jobs.json#crest-orange-fox |
| texture | T_Numbers_Varsity | ai-generated | Codex image generation | image-lab/jobs.json#numbers-b-varsity |
| texture | T_Glyph_Speed | ai-generated | Codex image generation | image-lab/jobs.json#glyph-speed-a |
| texture | T_Glyph_Precision | ai-generated | Codex image generation | image-lab/jobs.json#glyph-precision-a |
| texture | T_Glyph_HeavyPuck | ai-generated | Codex image generation | image-lab/jobs.json#glyph-heavy-b |
| texture | T_Glyph_BigPuck | ai-generated | Codex image generation | image-lab/jobs.json#glyph-big-a |
| texture | T_Glyph_Reverse | ai-generated | Codex image generation | image-lab/jobs.json#glyph-reverse-b |
| texture | T_Zone_SlowIce | ai-generated | Codex image generation | image-lab/jobs.json#zone-slow-b |
| texture | T_Zone_FastIce | ai-generated | Codex image generation | image-lab/jobs.json#zone-fast-b |
| texture | T_Zone_Bumper | ai-generated | Codex image generation | image-lab/jobs.json#zone-bumper-a |

(The `#name` suffix is a locator inside the job file; if the gate wants a plain path, drop it.) Use the C-0004 column order of the actual `ASSETS.md`; I did not open it (not mine to touch).

## Triage page: tests run

- Every one of the 26 `src` paths resolves to a file (checked by script).
- The page's script, extracted, passes `node --check`. The `\n` in "Copy my picks" is a real backslash-n in the file (`lines.join('\n')`), no newline inside a string.
- Single self-contained file, relative `src`, no external resources, light and dark by `prefers-color-scheme`. Rebuild with `node build-pick.mjs` after editing `verdicts.json` (rejected images are not shown: the rejected sun crest is on disk only).

## What I could not verify

- The page in a real browser: radio clear-on-second-click, `localStorage` persistence and the clipboard button are written but not clicked through (no browser run).
- That any image looks right in the engine (no Unreal here): backdrop brightness, key-out of the magenta and grey-green backgrounds (flat but slightly noisy, edges antialiased, so expect a fringe; crests may need a 1 to 2 px erode), the crest at real jersey size, the 64 px legibility (judged by eye on contact sheets only).
- Tileability by eye: only the numeric seam check above; the bumper tile needs its crop chosen by hand.
- Whether the city skyline or the sun crest would really be read as a landmark or emblem; I rejected the sun by caution, kept the city flagged.
- Codex image generation terms and file size after the Shipping cook (`size_mb` is a guess until a cook).
