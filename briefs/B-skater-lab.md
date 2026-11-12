# Skater lab: parallel experiments on how the skater looks and moves (device B, from H5)

Operator decision, 2026-11-12 about 10:55: device B keeps **two editors** and adds builders that need **no editor**, to try several skater approaches
quickly against the plan's baseline, so that by tonight there is a ranked, tested list instead of one bet.

## Baseline (from `docs/HOCKEY.md` 5; nothing here changes it)

| Track | What | State |
|---|---|---|
| T-A toy figure | engine shapes posed by C++; ships in M1, the fallback forever | device A, lane A5 |
| T-B2 generated (Meshy), figure 1 | text-to-3D chunky toy skater, `ue5_mannequin` rig, text-to-motion stride and shot, library fall | lane B5, importing now on editor port 8002 |
| T-B1 Fab clips retargeted | the pack "Hockey Player Animated"; fallback from H24; needs an operator purchase | not started |

Promotion rule for any humanoid: swapped in through `game.skater.visual`, the player bot still wins, the frame gate holds. The game reads from 6 m behind
and 3.5 m above the player's skater; the other five skaters are 10 to 100 m away, so **silhouette, team colour and readable motion at distance** matter more
than surface detail. The look is P2 painted cel at night: flat colours, ink outline, toon ramp.

## Lab lanes (no Unreal editor; each writes to its own folder under `C:\Users\kazda\jam\skater-lab\`)

| Lane | Question it answers | Tools | Budget |
|---|---|---|---|
| S1 figures | Which *figure style* reads best: chunkier toy, big-head mascot, lean stylised athlete in full gear? | Meshy text-to-3D + rig | 150 Meshy credits |
| S2 motions | Which *clips* are usable: stride variants, glide, stop, crossover, shot, check, celebrate; text-to-motion against library presets; do they loop? | Meshy motion + animate on lane B5's rig | 150 Meshy credits |
| S3 Rodin route | Does "mesh in Hyper3D Rodin, rig and clips in Meshy" work end to end, and how does a Rodin mesh compare? (the Hyper3D / NVIDIA pool) | `C:\Users\kazda\jam\rodin-tools\rodin.mjs`, Meshy rig | 10 Rodin credits, 40 Meshy |
| S4 scripted original | Can a Blender Python script build an *original* articulated toy skater (rigid parts on a small skeleton) with scripted, perfectly looping clips, exported as FBX? | Blender 4.2 headless | none |

Lane B5 stays the only lane that imports into Unreal. Lab lanes stop at "inspected files plus contact sheets"; the lead queues the best one or two for
import when an editor is free.

## Shared rules

- Read `briefs/B-machine-notes.md` section "Always" first. You never start Unreal, never run git, never write inside a repository except your report
  `C:\Users\kazda\jam\thirdjam\status\B-skater-<lane>.md` and contact sheets filed with `node devlog/shot.mjs B lab-<lane>-<label> <file> "<caption>"`
  (run from `C:\Users\kazda\jam\thirdjam`).
- Meshy: `node tools/meshy/meshy.mjs` from `C:\Users\kazda\jam\thirdjam` (the key is read from its `.env`; never print or copy it). Set `MESHY_LEDGER` to
  a jsonl in your lab folder. A POST is never retried by the client; on a `quota` or rate-limit error wait and try once more, then report. Three other lanes use
  the same account: stay inside your budget and check `balance` before the first and after the last call. Download at once (rig and animation URLs expire).
- Blender: `"C:/Program Files/Blender Foundation/Blender 4.2/blender.exe" -b --factory-startup --python <script> -- <args>` (x64 under emulation: slower, and it
  prints a harmless `TBBmalloc` warning). `tools/meshy/inspect.py` reports bones, triangles, textures, actions and pelvis travel; `tools/meshy/render.py` renders
  turntables and motion frames with Workbench (flat colour plus outline: a cheap stand-in for the game's cel look). Python has no PIL: build contact sheets
  with ffmpeg instead (`ffmpeg -y -i frames/%03d.png -vf "tile=8x1" sheet.png`, adjusting the pattern to the real file names), under 400 KB each.
- No text, letters, numbers, logos or brand shapes in any prompt result you keep; no real teams or people. Prompts ask for "painted cel style, flat solid
  colours, no text, no logos". Deep blue is the reference jersey; the orange team is a material tint later.
- Provenance per file: generated from our prompt = `ai-generated` (tool, task id); a service preset clip = `sourced` (preset id, the service's licence);
  made by our script = `original`.

## The rubric: every candidate gets one row (1 = poor, 5 = excellent, with one line of evidence each)

| Axis | What to look at |
|---|---|
| Look fit | flat colours, clean silhouette, reads as a hockey skater at a glance; a second render at about 40 px tall (the size of a skater 40 m away at 1080p) |
| Technical | triangles (30k cap), textures (one set, 2K cap), skeleton (bone count, `ue5_mannequin` names, root bone or not), pose (A or T), limbs separate from the body |
| Motion | stride loop seam (pose difference first to last frame), pelvis drift in cm over the clip, foot slide, is the shot's wind-up readable, does the fall end flat |
| Cost | credits, wall time, retries |
| Provenance | origin class, licence, which prize pool it can count for, whether it is scored as ours |
| Import risk | what is still unknown for the Unreal import (lane B5's report answers most of it) |

End your report with: the rows; the contact sheets' file names; a verdict per candidate (**queue for import** / **keep as fallback** / **drop**) with the
reason; what you could not verify; what the lead or the operator must decide.
