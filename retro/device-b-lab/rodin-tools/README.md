# rodin-tools: Hyper3D Rodin API client

Zero-dependency Node client (`rodin.mjs`, Node 18+, `node:https` only) for the Hyper3D Rodin REST API, base URL
`https://api.hyper3d.com/api/v2`. Same conventions as `tools/meshy/meshy.mjs`. Lives outside any repository.
Treat everything it generates as an experiment (contest rule 7(h)), like the Meshy output.

## Key

`RODIN_API_KEY` from the environment, else from `C:/Users/kazda/jam/thirdjam/.env` (`RODIN_ENV_FILE` overrides). Sent only in
`Authorization: Bearer`. Never printed, logged, written or put in a URL; output is scrubbed of the key as a last guard.
Download links are fetched without any credentials.

## Commands

```
node rodin.mjs balance [--usage 1]                       # free; the one real call used in testing
node rodin.mjs text-to-3d  --prompt "a wooden barrel, painted cel look" --wait --out ./scratch/barrel
node rodin.mjs image-to-3d --image front.png[,side.png,...] [--prompt "..."] --format fbx --wait --out ./scratch/x
node rodin.mjs status <task_uuid> --sub <subscription_key> [--wait]      # or --ledger file.jsonl holding the submit line
node rodin.mjs download <task_uuid> <dir> [--only glb,fbx]
```
Every command accepts `--dry-run` (prints the request, sends nothing, no key needed). Common: `--wait`, `--interval-s 10` (min 5),
`--timeout-s 1200`, `--ledger file.jsonl` / `RODIN_LEDGER`. Progress on stderr; the last stdout line is one JSON object
`{"tool":"thirdjam.rodin/1","cmd","ok",...}`. Exit 0 ok, 1 failed task or API error, 2 could not run (usage, auth, network).
`error.kind`: `usage`, `auth` (401, plan/subscription codes), `quota` (429, `API_INSUFFICIENT_FUNDS`, `API_PARALLELISM_LIMIT_REACHED`),
`validation`, `not_found` (`NO_SUCH_TASK`), `timeout`, `task_failed`, `network`, `api`.

Retries: the generation POST is never retried. `/status` and `/download` are read-only POSTs and retry like GETs
(network error, 5xx, 429 with `Retry-After`). A body-level `error` on HTTP 201 is treated as a failure (docs say 201 does not mean accepted).

**Two ids.** Submit returns `task_uuid` (use for `download`) and `subscription_key` (use for `status`); swapping them gives `NO_SUCH_TASK`.
Use `--ledger` on the submit so `status <task_uuid>` can find the key itself.

### Generation parameters and defaults

| Option | Form field | Default | Notes |
|---|---|---|---|
| `--prompt` | `prompt` | none | required for text-to-3d |
| `--image a,b` | `images` (repeated) | none | image-to-3d, 1 to 5 png/jpg/webp; first drives material |
| `--tier` | `tier` | `Gen-2.5-Low` | Basic API access covers only `Gen-2.5-Low` and `Gen-2.5-Extreme-Low`; Medium/High/Extreme-High need Full access |
| `--mesh-mode` | `mesh_mode` | `Raw` | `Raw` is documented (and a Fast-mode condition). `Quad` exists per third parties, unverified |
| `--polycount` | `quality_override` | `20000` | docs: Fast mode needs it <= 20000. Unit (faces vs triangles) not stated |
| `--format` | `geometry_file_format` | `glb` | glb, fbx, usdz documented. **One format per task**; run a second task for FBX, or see unverified |
| `--material` | `material` | omitted | unverified: PBR / Shaded (third party) |
| `--tapose true` | `TAPose` | omitted | unverified: third-party boolean |
| `--texture-mode` | `texture_mode` | tier default | extreme-low, low, medium, high, extreme-high |
| `--uhd true` / `--highpack` | `uhd_texture` / `addons=HighPack` | off | default 2K; either one raises to 4K. Array encoding of `addons` unverified |
| `--seed`, `--extra k=v,k=v` | any | omitted | pass-through for fields not modelled |

Defaults give: Low tier, triangle-style Raw mesh capped at 20k, one 2K texture set, GLB.

## Costs (as documented)

Gen-2.5 page: base 0.5 credits; +0.5 for `tier=Gen-2.5-Extreme-High`; +2.0 for `texture_mode=extreme-high`; parameters not listed add nothing.
So a textured 20k prop on the default or any non-Extreme tier costs 0.5 credit by the docs, and a character-suitable mesh at the same
settings costs the same 0.5 (no rigging or character surcharge is documented). The real `consumed` value is returned at submit and is authoritative;
`--dry-run` prints the estimate. The pricing page lists direct credits at $1.5 each and Business plan API access ($120/month, 120 or 240 RPM).
Account balance at 2026-11-12 test: 421 credits.

## Result links and retention

`/download` returns `{"list":[{"name","url"}]}`. Docs state links can expire and do not give a duration; download promptly.
Data Retention Policy: request payloads and outputs are kept 7 days then deleted. Downloads of an unfinished task may return partial results.

## Licence notes (not legal advice)

Terms of Service (hyper3d.ai/legal/terms): for Rodin "we will not limit your use of such Output", subject to the agreements and law; export for
private or commercial use "depending on your subscription plan"; no representation about copyrightability; no reselling Content as such; no using the
services to train competing models. The ChatAvatar-style per-output "License" purchase is a different product. No API-specific terms were found.
The pricing page says only that paid plans have broader export and usage rights. Confirm in writing with Hyper3D before shipping Rodin output in the contest build.

## Sources (all accessed 2026-11-12)

- https://docs.hyper3d.ai/ and https://docs.hyper3d.ai/en (overview, base URL)
- https://docs.hyper3d.ai/en/get-started/quick-start (auth, flow, access levels, polling)
- https://docs.hyper3d.ai/en/get-started/features (tiers, concurrency cap)
- https://docs.hyper3d.ai/en/api-specification/rodin-gen2-5 (submit fields, texture table, Fast mode, pricing)
- https://docs.hyper3d.ai/en/api-specification/rodin-gen2 (Gen-2: tier `Gen-2`, 0.5 credits)
- https://docs.hyper3d.ai/en/api-specification/check-status, .../download-results, .../check-balance, .../errors
- https://docs.hyper3d.ai/en/legal/data-retention-policy
- https://hyper3d.ai/pricing?lang=en and https://hyper3d.ai/legal/terms
- Third party, lower trust: https://docs.comfy.org/built-in-nodes/Rodin3D_Gen25_Text (material, TAPose, formats), Runware and Scenario Rodin pages (Quad, polygon presets)

## Unverified

- Allowed values and defaults for `mesh_mode` (only `Raw` official), `quality`, `material`, `TAPose`; whether `quality_override` counts faces or triangles.
- Whether `geometry_file_format` accepts obj/stl on Gen-2.5, and whether one task can return GLB and FBX together (assume one format per task).
- Link lifetime; numeric rate limits (429 with `Retry-After` is documented); image size limits.
- Per-tier credit costs beyond the additive rules; whether Basic access can use `Quad`.
- Rigging: the Gen-2.5 and Quick Start pages mention rigging for Basic access but no endpoint page was found (404 on guessed URLs). No animation documented.
- The `/status` and `/download` flows, `addons` encoding and the success path of generation were NOT exercised (no generation submitted by design).

## Verified 2026-11-12 (lane S3, three real generations, 1.5 credits)

- `text-to-3d`, `status`, `download` all worked unchanged. Only change to the client: `download` / `--out` output now also carries `links` (the raw `{name,url}` list) and `raw_keys`, so a URL can be handed to another service.
- Submit (HTTP ok): `uuid` (task), `jobs.subscription_key`, `jobs.uuids` (3 job uuids), `consumed`. **Cost: `consumed` = 0.5 credit** for each of: skater 20k Gen-2.5-Low, spotlight `quality_override=10000`, skater without TAPose. Balance 421 -> 419.5. No surcharge for the TAPose field.
- Wall time: 37 s (skater, incl. 5 s initial wait) and 26 s (spotlight, skater 2). `/status` jobs: `{uuid,status}` with states seen `Generating`, `Done` (3 jobs, one per stage; no `queue_length` seen).
- `/download` returns `{"list":[{"name":"base_basic_pbr.glb","url":...}]}`: **one entry only** for glb (a PBR GLB with diffuse, normal and orm maps, all 2048x2048). Link host `file.hyper3d.com` (signed, `X-Tos-Expires=604800`, i.e. **7 days**, matching the retention policy). The link needs no credentials.
- Meshy `rig --model-url <that Rodin link>` was accepted as is (no data URI needed) and rigged in 22 s.
- `quality_override` is a triangle budget in practice: 20000 gave 19,282 and 19,748 triangles; 10000 gave 9,438.
- `TAPose=true` was accepted (no error) but the effect is small: bounds width 1.19 m vs 1.31 m without it; both come out in a wide A-pose. The prompt text "A-pose, arms and legs clearly separated" already produces an A-pose. The no-flag result had better colours (see S3 report).
- Not exercised: Quad mesh mode, usdz/fbx, material, uhd, highpack, any rigging endpoint (none found in the docs, none tried).
