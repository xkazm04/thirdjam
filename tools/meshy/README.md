# tools/meshy: Meshy API client and Blender evaluation scripts

Zero-dependency Node client (`meshy.mjs`, Node 18+) for the Meshy REST API at `https://api.meshy.ai`, plus two Blender headless scripts and a
PIL strip composer used to evaluate what comes back. Findings, costs and the recommended jam role: `docs/experiments/MESHY.md`.

**Everything Meshy generates is an experiment (contest rule 7(h)).** Download into a scratch folder outside the repo, never into the repo,
never into an Unreal project without the Director's promotion of track T-B2, and declare it in the dev log.

## Key

Read at run time from `MESHI_API_KEY` (the operator's spelling) or `MESHY_API_KEY` in the environment, else from `<repo>/.env`
(`MESHY_ENV_FILE` overrides the path). The key is sent only in the `Authorization: Bearer` header; it is never printed, logged or put in a URL.

## Commands

```
node tools/meshy/meshy.mjs balance
node tools/meshy/meshy.mjs text-to-3d --prompt "..." --ai-model meshy-6-lite --pose a-pose --polycount 20000 --wait     # preview, 5 cr
node tools/meshy/meshy.mjs text-to-3d --preview-task <id> --texture-prompt "..." --wait                                   # refine, 10 cr (2K)
node tools/meshy/meshy.mjs image-to-3d --image sketch.png --ai-model meshy-6-lite --wait                                  # 15 cr textured
node tools/meshy/meshy.mjs rig --input-task <refine id> --template ue5_mannequin --height 1.7 --wait                     # 5 cr
node tools/meshy/meshy.mjs motion --prompt "..." --duration 4 --mode prime --wait                                         # text to motion, 10 cr (swift 3)
node tools/meshy/meshy.mjs animate --rig-task <rig id> --motion-task <motion id> --wait                                   # 3 cr
node tools/meshy/meshy.mjs animate --rig-task <rig id> --actions 509,187 --wait                                          # 3 cr per action, one merged file
node tools/meshy/meshy.mjs library --search run                                                                          # free; 678 presets on 2026-11-12
node tools/meshy/meshy.mjs status <task> [--kind rigging] [--wait] [--raw]
node tools/meshy/meshy.mjs download <task> <dir> [--only glb,fbx,png]
```

Common options: `--wait` (poll until a final state), `--interval-s 10`, `--timeout-s 1200` (a timeout leaves the task running server-side;
resume with `status <id> --wait`), `--dry-run` (print the request body, send nothing), `--ledger file.jsonl` or `MESHY_LEDGER` (append every
result line, which carries `credits`).

Output: progress on stderr; the **last stdout line** is one JSON object
`{"tool":"thirdjam.meshy/1","cmd","ok","kind","task_id","status","elapsed_ms","server_ms","queue_ms","credits","urls"|"files","error"}`.
`error.kind` is one of `usage`, `auth` (401, no key), `quota` (402 credits or plan, 429 limits), `validation` (400, 403 moderation),
`not_found`, `timeout`, `task_failed`, `network`, `api`. Exit 0 ok, 1 failed task or API error, 2 could not run (usage, auth, network).

Behaviour worth knowing:
- A POST is never retried (a retry could spend credits twice). GETs retry twice on network errors and 5xx; a 429 "Rate limit exceeded" waits `Retry-After`.
- `status`/`download` without `--kind` find the task kind themselves. `GET /openapi/v2/text-to-3d/<id>` answers 200 for **any** task id
  (with the wrong shape), so the client reads `type` and refetches from the right endpoint.
- Rig, animation and text-to-motion download URLs expire after 3 days (`expires_at`); text-to-3d and image-to-3d URLs did not expire in practice.

## Evaluation scripts (Blender 4.2, headless)

```
B="C:/Program Files/Blender Foundation/Blender 4.2/blender.exe"
"$B" -b --factory-startup --python tools/meshy/inspect.py -- <file.fbx|glb> [...]      # bones, triangles, textures, actions, pelvis travel
"$B" -b --factory-startup --python tools/meshy/render.py -- <file> <frames_dir> motion <action_substring|-> 8 192
"$B" -b --factory-startup --python tools/meshy/render.py -- <file> <frames_dir> turntable - 8 192
python -I tools/meshy/strip.py <frames_dir> <out.png> 8 280 "label"                     # PIL strip, shrunk to <= 280 KB
```
`render.py` uses Workbench (flat lighting, texture colour, object outline): a cheap stand-in for the P2 cel look, not the game's shader.
