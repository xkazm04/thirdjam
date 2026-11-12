# P4 figure slice: a generated hockey player that plays (lane A28, H10.4-H12.4)

Status: experiment notes, 2026-11-12 16:57-18:57 CET. R-0025 point 3: P4 is a masterclass in graphic design from commercial APIs and free assets; the
toolkit-fit slice first: the recurring animated character, in motion, at the game camera. Scripts and job lists in `tools/p4/`, imports in `Game/Content/P4/`,
stills in `devlog/shots/20261112-17*-A-a28-*`, eyes verdicts in `docs/experiments/eyes/a28-*.json`, every generation in `tools/p4/ledger/{meshy,rodin,fal}.jsonl`.
Reads with `docs/experiments/MESHY.md` (E1), `ANIMATION-SOURCES.md` (E2), `BLENDER.md` (A22), `status/B-skater-s3.md` on `origin/dev-b` (B's Rodin route).

Marks: **[run]** = done on this machine today, numbers from the logs; **[vendor]** = vendor docs only; **[eyes]** = lane A27's Gemini preset `figure` (advice, not a ruling).

## 0. The answer in six lines

1. **All four routes produce a figure that walks a Manny clip and ragdolls over knocked_down within 25 minutes of API and editor time each** [run]: Meshy (text-to-3D, its
   `ue5_mannequin` rig), Rodin (mesh, Meshy rig: B's route), Tripo through fal (mesh, Meshy rig; fal hosts no Tripo rig endpoint), and Hyper3D's own rigging is **dashboard-only**
   (no endpoint in the API docs), so route four stops there.
2. **Recommendation: Tripo mesh + Meshy rig for the look, Rodin mesh + Meshy rig as the cheap variant factory; Meshy's own mesh last.** Tripo was the only one that drew the brief
   (toy proportions, blue jersey with white trim, a 7 front and back, gloves, helmet with cage) [run, stills]; Rodin is a chunkier toy with a visor and gloves for 0.5 credit;
   Meshy drew a slender adult [eyes: toy 2/5 against 4/5 for the other two].
3. **The rig is the same 28-bone root-less Meshy skeleton on all three** (auto characterization in the IK Rig succeeds on its UE5 bone names; chains Spine, Neck, Head, legs, feet,
   clavicles, arms) and **the one defect that blocks it is the 100x `Armature` node in Meshy's FBX**: an IK-retargeted clip puts the pelvis at 90 m. `tools/p4/normalize_fbx.py`
   (Blender, 10 s per file) bakes it out; after that the walk plays with the pelvis at 87-96 cm and the physics asset gets 16-17 bodies instead of 2.
4. **Bone table totality** (`tools/p4/bonemap-*.json`, `bonemap_check.py`): 23 rows, 23/26 core bones mapped; unmapped on every vendor: `spine_04`, `spine_05`, `neck_02`
   (Meshy has three spine bones and one neck), plus all twist, IK and finger bones. The retarget copes; the shortfall shows as a stiff upper back and mitten hands.
5. **Cost per figure** (mesh + rig + 5 library falls): Meshy 40 cr (5 + 10 + 5 + 15 + 5 for a wasted rig of the preview), Rodin 0.5 cr + 20 Meshy cr, Tripo (fal: priced on the
   dashboard, not in the response) + 20 Meshy cr. **Per team of six** on one mesh: one figure plus material instances; six distinct figures: Rodin 3 cr + 120 Meshy cr, about 15 minutes.
6. **Three blockers, in order:** (1) a stick in the hand (every eyes verdict: "the absence of a stick destroys the silhouette"): the socket exists, the attached plain stick needs its
   grip rotation set by eye in the editor; (2) the `puck_socket` and root motion: the vendor skeleton has no `root` of its own, the normalizer makes one from the armature node, so the
   puck socket rides it, but the Meshy library clips carry pelvis travel that must be stripped (MESHY.md section 4); (3) the retarget onto a big-headed toy clips arms into the body in
   the walk (Rodin, Tripo) and needs the retarget pose fixed by hand, or shorter arm chains, before it goes past 24 m.

## 1. Vendor table [run]

Same prompt family on all three: *stylized hockey player, toy-like proportions, helmet with visor, blue jersey with white trim, number 7, hockey gloves, no stick, A-pose with
arms and legs clearly separated from the body, game-ready low poly, clean topology, flat solid colours, no text, no logos*. Rig: Meshy `ue5_mannequin`, `height_meters` 1.8 on all.

| | Meshy | Rodin (Hyper3D) | Tripo (through fal) |
|---|---|---|---|
| mesh call | text-to-3d preview `meshy-6-lite` a-pose 20k + refine 2K | `Gen-2.5-Low`, `mesh_mode=Quad`, `quality_override=18000`, `TAPose=true`, GLB | `tripo3d/tripo/v2.5/text-to-3d`, texture standard, pbr false, face_limit 20000, auto_size |
| task ids | 01a1d07d-5f79-75b8-aac0-057e4de60564, 01a1d082-cfba-7780-8437-0af0b6b44130 | ad9c0508-72b2-4a2d-8efc-e4aeba8c7a7b | fal 01a1d082-11a3-7e70-b431-41bb40729ac1, tripo 12678a9a-55d1-422b-b16b-2a9bb75a6db5 |
| wall time, mesh | 150 s + 69 s | 68 s (6 jobs) | 53 s (50 s inference) |
| credits, mesh | 15 | 0.5 | not in the response; fal lists "$0 per compute second", quad +$0.05 |
| triangles | 20,832 | 35,920 (18k quads) | 19,986 |
| textures | 1 x 2048 base colour | 3 x 2048 (diffuse, normal, metallic-roughness) | 1 x 2048 colour |
| delivered size | GLB 2.8 MB; rig FBX 1.6 MB | GLB 8.0 MB, 1.71 m | GLB 1.5 MB, **0.25 m tall** (`auto_size` did not give metres) |
| rig (Meshy) | 01a1d08d-903b-71a2-8f1c-1093bba6b669, 22 s, 5 cr | 01a1d095-0270-76cc-887d-cbedf924221d, 25 s, 5 cr | 01a1d095-8b3f-7521-9ca2-cad3c8af3053, 16 s, 5 cr |
| rig preset, bones | `ue5_mannequin`, 28 (+ `root` from the armature node at import = 29) | same | same |
| mapping totality | 23/26 core; unmapped spine_04, spine_05, neck_02 | same | same |
| IK Rig auto characterization | True (11 chains) | True | True |
| physics asset bodies (auto) | 16 | 17 | auto |
| size check vs Manny (pelvis / head / hand span / foot span, ratio) | 1.04 / 0.96 / 0.79 / 1.13 | 0.93 / 0.84 / 1.05 / 1.87 | 0.93 / 0.84 / 0.91 / 1.44 |
| the three hardest things wrong | adult proportions (head 0.14 of height); bare hands in the preview, gloves only painted; no cage, a visor-less open face | mitten hands with no fingers, arms thick as thighs; grey, dull texture under the normal map; the visor is a slab | T-pose with arms straight out (the rig still took it); hands and head blend into the torso (one colour); a 7 on the front and back but the white trim smears |
| eyes [eyes] hockey player full / small, toy full / small | 2 / 1, 2 / 1 | 3 / 2, 4 / 3 | 2 / 2, 4 / 3 |
| stills | `20261112-172933-A-a28-meshy-walk-{chase,far}.png` | `20261112-173143-A-a28-rodin-walk-{chase,far}.png` | `20261112-173209-A-a28-tripo-walk-{chase,far}.png` |

Route four, Hyper3D's own rigging (Director's addition): `docs.hyper3d.ai` (index, features, quick-start, API spec, check-status, download-results, read 17:04) documents
`POST /api/v2/rodin`, `/status`, `/download` and nothing else; the changelog entry says only "Added rigging support for 3D models" and the quick-start calls rigging an account
permission ("Basic access permits rigging"). No endpoint: dashboard-only, stopped as instructed. The Rodin mesh was still generated Quad + TAPose so the operator can rig it there.

## 2. The clip test [run]

`tools/p4/run_vendor.sh <Vendor>`: delete `/Game/P4/<Vendor>`, `ue_setup.py` (legacy FbxFactory through an `AssetImportTask` with an explicit factory, which keeps an
animation-only import on the same skeleton; A22's commandlet route sent it to Interchange), measure, IK Rig on both skeletons, IK Retargeter Manny -> vendor (FUZZY chain map,
auto-align), batch retarget of `MF_Unarmed_Walk_Fwd`, `MF_Unarmed_Jog_Fwd`, `MM_Idle` (the 5.9 names of the brief's `MM_Walk_Fwd` / `MM_Jog_Fwd`; the Fab Hockey Player pack is
descoped by the operator), physics asset (`PhysicsAssetToolset.CreateFromMesh`), lab level `/Game/P4/Maps/P4_Lab` with Manny beside the figure, chase (4.2 m) and far (24 m)
SceneCapture2D, fixed 60 Hz, then under Simulate: 2 s of the clip sampled every 8 game frames, 20 frames of ragdoll (`set_simulate_physics`, blend weight 1, an impulse on
`spine_02`), the sequence again. Frames are counted by game time at 60 Hz, never wall clock. One figure takes 25-35 s of editor time.

| figure | retargeted walk, pelvis z over 120 frames | foot_r z | ragdoll 20 frames, pelvis z | reads as |
|---|---|---|---|---|
| Meshy | 99.6 rest -> 87-96 | 16-29 | 90 -> 41 (sits back, arms out) | a walking adult in a blue jersey; the stride and arm swing are Manny's |
| Rodin | 89.1 rest -> 80-86 | 13-19 | 78 -> 36 (face down) | a chunky toy marching; the big gloves swing past the hips |
| Tripo | 89.4 rest -> 81-86 | 10-18 | 62 -> 58 | a toy in a cage helmet marching; dark legs vanish against the floor at 24 m |

The vendor's own walk clip (`AS_P4_<Vendor>_vendor_walk`, 32 frames @30) plays without a retarget and was the first proof (`20261112-172816-A-a28-meshy-vendorwalk-*`).
What the retarget leaves: the toy figures' arms cross the torso in the walk (no twist bones, hands a third wider than Manny's), the upper back is stiff (one spine chain of three).

## 3. Collision as a cosmetic layer [run]

A physics asset per figure (`PA_P4_<Vendor>`, auto bodies on pelvis, spine, head, clavicles, upper and lower arms, thighs, calves, feet). The capture drives the ragdoll only from
the knocked_down window: physics on for 20 frames, then off, the component's relative transform reset (the pawn owns the transform; the ragdoll never moves it) and the sequence
resumed. Nothing reads back into the sim: the sim's `SetKnockedDown` is the only trigger, as C-0001 v3 demands. Not yet proven in a bot match through A26's skeletal visual:
`status/A26.md` at 17:02 had the component in progress; this is the next step once A26 reports the visual works.

## 4. Free assets on Fab (research only, nothing downloaded)

Fab renders prices and licence tiers in the browser; the fetched pages show `Select a License` and a structured-data price only. Found for a hockey rink, stands, benches, lights:
- `fab.com/listings/b0bf34d1-3f2d-41dd-8b46-07ae8dbce393` "International Arena, Ice Hockey Environment" (AnnaStakhiv): UE 5.5+, crowd tool, LED toolkit; **5,492.90 CZK, not free**.
- `fab.com/listings/55719c00-752b-415d-b490-3f5961602197` "Hockey stadium" (holydeerz): modular stands, changeable materials; **571.06 CZK, not free**.
- No free hockey rink or arena listing surfaced in two searches; the free stand-ins under the Fab Standard licence are the Quixel Megascans surfaces (ice, concrete, plastic seats)
  and Epic's own sample content, which the operator must claim signed in. The arena pieces stay with A22's props and B's dressing (BLENDER.md section 3).

## 5. Costs and the per-team estimate

Meshy balance 786 -> about 700 (this lane: 5 + 10 + 5 + 5 + 5 + 5 + 15 + 15 + 15 = 80 cr; one rig of the preview wasted); Rodin 0.5 cr; fal unpriced in the response.
One figure with a rig and five falls: Rodin + Meshy 20.5 cr, 4 minutes; Tripo + Meshy 20 cr + fal; Meshy only 35 cr. Six distinct figures through Rodin: about 123 cr and
15 minutes of API time plus 3 minutes of editor time; one mesh with team material instances (the P2 way) is the cheaper path and keeps both teams identical.

## 6. Pipeline notes (what cost time)

1. The Meshy rig of the preview task has no texture and different UVs from the refine: rig the refine id (5 cr lost).
2. Meshy FBX: bone translations in metres under an `Armature` node scaled 100. Vendor clips play; everything retargeted or physics-based is 100x off. Four Blender export variants
   imported lying along Y; the fifth (bake transforms, centimetre scene as A22's `skater.py`, pre-rotate -90 deg about X, `axis_up` Z) imports upright. `normalize_fbx.py` does it.
3. Tripo's GLB is 0.25 m tall; Meshy's `height_meters` rescales it at the rig step, so nothing else cared.
4. `LevelEditorSubsystem.load_level` refuses a dirty open level: save first, or skip when it is already open.
5. Git Bash rewrites `/Game/...` arguments into `C:/Program Files/Git/Game/...`: `MSYS_NO_PATHCONV=1` on every script that passes asset paths.
6. The editor must be closed whenever no import or retarget runs (R-0009; it holds `UnrealEditor-JamCore.dll` on the shared tree).

## 7. Open questions for the Director

1. Which figure goes forward for the A29 night-rig stills: Tripo (look) or Rodin (cost)? Both share the rig and the retargets; the swap is one config file.
2. `Game/Content/Characters/Mannequins/` already held the Manny set when this lane started (another lane's copy); this lane added `Meshes/SK_Mannequin`, `SKM_Manny_Simple`,
   `Rigs/PA_Mannequin`, `Anims/Unarmed/MM_Idle`, `Walk/MF_Unarmed_Walk_Fwd`, `Jog/MF_Unarmed_Jog_Fwd` (identical engine files). Who owns that folder, and is it ledgered?
3. The Fab Standard licence question needs the operator signed in; nothing was downloaded.
