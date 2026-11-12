# Where skater animation can come from, tested (experiment lane E2)

Status: experiment notes, 2026-11-12 01:00-01:20 local, before the jam starts (rule 7(h)): nothing made here is kept as content. The repo keeps
these notes, the recipe `tools/skills/animation-sources.md` and three evaluation images under `docs/experiments/img/anim-*.png`. The throwaway
project (`scratchpad/anim-src`, content-only, MCP on port 8012, 250 MB) was deleted afterwards; C: had 535 GB free at the end.
Reads with `docs/HOCKEY.md` section 5, `docs/SKATERS-RESEARCH.md`, `tools/mcp/ANIMATION.md`.

Evidence tags as in SKATERS-RESEARCH: **[run]** = done on this machine tonight; **[local]** = read on disk; **[vendor]** = vendor page;
**[3rd]** = review or forum; **[unverified]** = could not open the page. Web pages were seen 2026-11-12.

## 0. The answer

**Is it worth making skater animation from real videos? No, not for this jam.**
1. Nobody publishes a single-camera video-to-motion result on ice. The services' standard foot clean-up exists to remove foot sliding, and
   sliding is what a skating glide is (section 4).
2. Somebody would have to film a real skater on ice: full body in frame, wide enough to keep a long glide in shot. Found footage (NHL
   broadcasts, YouTube) is someone else's copyrighted video of recognisable people.
3. Our figure is a chunky toy that the arena camera sees from 52 m. At that distance stride detail is invisible even on our own test figure
   (section 1).

What does read from the arena camera is **lean, tip-over, stick swing and body pitch**, and code makes all four in minutes.

## 1. Ranked comparison

Hours are for one builder in the jam, including import or retarget and wiring behind `IJamSkaterVisual`. "Agent alone" means no human in an
editor and no human account step during the jam.

| # | Source | Hours | Skating quality | Rights | Agent alone | Fit to P2 toy | Verdict |
|---|---|---|---|---|---|---|---|
| 1 | **Procedural toy motion by code** (T-A), with the tuned numbers below | 4-6 (recipe); the throwaway took about 15 min | Reads at 52 m: lean, check, shot, knockdown. The stride does not read; nothing would at that distance | ours | **yes** [run] | native: it *is* the toy | **Ships at M1** |
| 2 | **Fab "Hockey Player Animated"** (14 skate clips + shot, Epic skeleton), retargeted UE4 to UE5 onto a figure with a UE5 skeleton | 6-10 (T-B1); about 2 h of that is retarget and montages | the only real skating strides found; hand-keyed | Fab Standard, sourced, unscored, ledger row | almost: the Fab sign-in is a human step once; the AnimGraph needs the C++ helper from `ANIMATION.md` | poor on its own mesh (realistic proportions); fair once retargeted onto a toy skeleton (arms clip the big head) | T-B1, and the clip source for T-B2 |
| 3 | **Engine Mannequin clips** (`MM_Death_*` x6, `MM_HitReact_*` x8, attacks, dash) | about 2 for falls and hits on a UE5-skeleton figure | no skating; falls are a "shot" crumple, not a slip on ice [run, image] | Epic content inside a UE product, sourced | **yes** [run: copied, sampled, captured] | needs a skeleton, so useless for T-A; on a toy, falls read, hits are too small to see at 52 m | T-B only, for `Fall` and `Check` |
| 4 | **Meshy text to motion and its library** (partner) | 1-2 on top of a T-B2 Meshy figure | library IDs 0-587 contain **no skate, skating, hockey or tackle**; it has `falling_down` (366), `Fall1`-`Fall4` (502-505), `Push_Forward_and_Stop` (260) [vendor] | paid plan: commercial; Free: CC BY 4.0 (SKATERS-RESEARCH) | yes, REST API | good if the figure is made in Meshy | T-B2 extra: falls, plus a text-to-motion skate stride to *try* |
| 5 | **Other text to motion** (Uthana, DeepMotion SayMotion, Rokoko Create T2M) | 2-4 plus retarget | no skating evidence anywhere | Uthana: "Paid usage includes commercial use of generated outputs", $0.10/s [vendor]; others unclear | Uthana has an API; the rest are web UI | depends on retarget | dropped |
| 6 | **Video to motion** (Rokoko Vision, Move One, DeepMotion, Plask, Autodesk Flow Studio) | 4-8 including filming, and the clean-up is open-ended | unproven on ice; foot clean-up fights the glide | own footage only; free tiers mostly non-commercial or unstated | **no** (filming, web UI) | poor: retarget onto a toy plus clean-up | **dropped** (section 4) |
| 7 | **Kimodo** (Epic's 5.9 plugin, NVIDIA model) | blocked; at least 3 h for downloads and approval before the first clip | untested; text-to-motion trained on 700 h of mocap; no skating claim | output: NVIDIA claims no ownership; Llama 3 attribution (section 3) | **no**: gated Llama approval and HF token, Sequencer UI, no MCP tool | SOMA skeleton retargeted onto the mannequin | **dropped** for the jam (section 3) |

## 2. Candidate 1: procedural toy motion. PASS [run]

**What was built.** Two figures made of basic shapes (cylinder body, sphere head and helmet, cylinder stick, cube blade, cube skates), posed
every sim frame by the pose function from `tools/skills/skater-toy-figure.md` step 5. The function was ported line for line to editor Python
and stepped at 60 Hz by frame count, not by wall clock.
- **Red** uses the recipe's numbers: `k_lean 0.6`, cap 25 deg, static skates, no gloves.
- **Blue** uses tuned numbers: `k_lean 2.5`, cap 35 deg, skates pushing out in alternation (18 cm sideways, 12 cm back at full speed), two
  glove spheres on the stick.

The input script runs 450 frames:
1. speed 0 to 8 m/s over 60 frames;
2. a right turn at 2 rad/s from frame 60 to 150, then a left turn from 150 to 240;
3. a check at frame 205;
4. a shot at frame 250, using the Godot keys;
5. a knockdown at frame 310, getting back up at frame 390.

A SceneCapture2D exported a PNG every 4 or 6 frames.

| Step | Time |
|---|---|
| editor start to MCP ready, content-only project, port 8012 | 19 s |
| level from `Template_Default`, `M_Toy` + 6 instances, ice plane (one Python call) | 6.6 s |
| 450 sim frames, 113 captures at 640x360 | 5.6 s |
| 450 sim frames, 75 captures at 1920x1080 | 19.1 s |
| authoring the pose port and the capture harness | about 15 min |

**Images.**
- `img/anim-procedural-chase.png`: chase camera, 9 m away.
- `img/anim-procedural-arena52m.png`: camera 33 m back and 40 m up (52 m away), FOV 60. The panels are 300x200 crops of a 1080p frame,
  shown at 1.5x.

**Judgement: yes, it reads as hockey players moving, provided the lean is tuned.**

| Element | Chase cam | Arena cam (52 m) | Change |
|---|---|---|---|
| lean in turns, recipe `k 0.6` (about 10 deg at 8 m/s and 2 rad/s) | barely visible | **invisible** | raise to `k_lean = 2.5`, cap 35 (blue): clearly visible from both cameras |
| crouch pitch and bob 4 cm | visible | invisible (bob); the crouch reads as "leaning forward" | keep; the bob costs nothing |
| skates striding | visible, a little mechanical | **invisible** | keep it small; it is for the replay or chase camera only |
| check lunge (20 deg, 15 cm) | reads | reads as a forward tilt | raise to 30 deg so it differs from the crouch |
| shot (stick yaw keys) | reads | the stick is a thin line, but the wind-up reads | make the stick 6 cm thick and the blade white or bright |
| knockdown tip 88 deg + bounce | reads strongly | **reads strongly** | keep |
| team colour | reads | reads; the head blends into the white ice | add a dark helmet band or a darker helmet (recipe step 2 already hints at this) |

**What would make it read better, in order of value at 52 m:**
1. lean 2.5 / 35 (done in the test);
2. a white blade-spray burst (Niagara, proven by R2) on hard turns and stops, when `|turn_rate| * speed > 8`, and on knockdown;
3. a short ice trail or a darker shadow under each skater for contrast on white ice;
4. a thicker stick;
5. a helmet band.

A faster bob or stride does not help from 52 m.

**Deviations from the brief.** The figure was posed from editor Python stepping sim frames, not from a Blueprint or C++ tick in PIE. Frames
were exported from a SceneCapture, not from a movie capture. R2 already proved the PIE path for a lean component (`ANIMATION.md` section 6),
and the jam builds the C++ component the recipe describes. Blade spray was not built tonight.

## 3. Candidate 2: Kimodo. NOT RUN: it needs about 21 GB of downloads and a gated licence approval

**Read on disk** [local], `C:\Program Files\Epic Games\UE_5.9\Engine\Plugins\Experimental\Animation\Kimodo` (8.7 MB):
- **Plugin.** `Kimodo.uplugin`: "Kimodo plugin for the AnimGenOffline framework", `IsExperimentalVersion: true`, an Editor module only. It
  depends on `AnimGenOffline` and `PythonMLPackages`.
- **Installs on enable.** It declares `PythonRequirements` (transformers 5.1.0, accelerate, peft, huggingface-hub, safetensors, tokenizers,
  ...), which are pip-installed when the editor first loads the plugin. `PythonMLPackages` ("Auto-install Pytorch and related ML packages")
  adds `torch==2.5.1+cu124`, torchaudio and torchvision on Win64. Enabling the plugin therefore **installs packages from the internet** (torch
  with CUDA is about 2.5 GB). That breaks the lane's "install nothing", so **the plugin was not enabled**.
- **Model weights** are not shipped. The `.tps` records say "Downloaded at runtime from Hugging Face, not redistributed":
  - `nvidia/Kimodo-SOMA-RP-v1.1`: about 1.1 GB (`_KIMODO_CHECKPOINT_EXPECTED_BYTES` in `kimodo_generate.py`), NVIDIA Open Model License.
  - `meta-llama/Meta-Llama-3-8B-Instruct` (**gated**) plus the `McGill-NLP/LLM2Vec-Meta-Llama-3-8B-Instruct-mntp(-supervised)` adapters:
    about 17.5 GB (`_TEXT_ENCODER_EXPECTED_BYTES`), Llama 3 Community License and MIT.
  - Neither repo is in the local Hugging Face cache (`~/.cache/huggingface/hub`).
  - **Total first-run download: about 21 GB.** It needs a Hugging Face read token with access to the gated Llama repo; the setting
    `KimodoSettings.HuggingFaceToken` reads "Needs access to meta-llama/Meta-Llama-3-8B-Instruct".
- **Hardware.** The README says Kimodo "requires ~17GB of VRAM to generate locally, primarily due to the text embedding model". The 5.9
  plugin defaults the text encoder to 4-bit NF4 ("About 6 GB") placed on the CPU, plus "the ~1 GB diffusion model" on the GPU.
- **How it runs.** It is a Sequencer generator section (`UAnimGenOfflineGeneratorSection`) with prompt keys. The model runs "at a fixed 30
  fps", and a section longer than 300 frames (10 s) warns "Animation quality will degrade heavily". It generates on NVIDIA's SOMA 77-joint
  skeleton, which AnimGenOffline retargets onto the shipped UE4, UE5 and UEFN mannequins.
- **No agent path.** There is no MCP tool (`TOOLS.md` has no Kimodo or AnimGen entry). The plugin's only `BlueprintCallable` is
  `ShowErrorDialog`.

**Licence terms.**
- **Code: Apache-2.0** (`Content/Python/kimodo/LICENSE`). The README adds: "Note that model checkpoints and data are licensed separately as
  indicated on the HuggingFace download pages."
- **NVIDIA Open Model License** (weights) [vendor; the fetch tool cut quotes at about 125 characters, so check the official text before
  quoting it in the dev log]:
  - "Models are commercially usable."
  - "NVIDIA claims no ownership rights in outputs. You are responsible for outputs and their subsequent uses."
  - Attribution applies only "If you distribute the Model" (a copy of the agreement and the notice "Licensed by NVIDIA Corporation under the
    NVIDIA Open Model License").
  - Rights terminate if you circumvent the model's guardrails (paraphrased).
- **Meta Llama 3 Community License** (text encoder) [vendor, raw.githubusercontent.com/meta-llama/llama3/main/LICENSE]:
  - Section 1.b.i: "If you distribute or make available the Llama Materials (or any derivative works thereof), or a product or service that
    uses any of them, including another AI model, you shall ... prominently display "Built with Meta Llama 3" on a related website, user
    interface, blogpost, about page, or product documentation."
  - Section 1.b.v: "You will not use the Llama Materials or any output or results of the Llama Materials to improve any other large
    language model".
  - Section 2: the licence is void above "700 million monthly active users" (irrelevant to us).
  - There is no clause on owning outputs.
  - The Hugging Face repo is gated: "You need to agree to share your Hugging Face information to access this model". Meta approves requests,
    and forum reports range **from minutes to weeks** [3rd].

**Is the output usable in a jam entry?**
- Yes on the licence terms: NVIDIA claims no ownership of outputs, and the game ships animation clips, not the model.
- To be safe, add "Built with Meta Llama 3" and "Kimodo, NVIDIA Open Model License" to the dev log and credits. Arguably the game "uses" no
  Llama Materials, but the line is cheap.
- A generated clip is `ai-generated` in `ASSETS.md`.

**Verdict.** Kimodo is the only text-to-motion source that runs inside Unreal and does not need a vendor account. It is still **dropped for
the jam**:
1. about 21 GB to download plus a pip install of torch;
2. a Meta approval with an unknown wait;
3. no agent path (Sequencer UI only);
4. the 52 m camera hides what it adds over procedural motion;
5. device B's look hours are better spent on T-B2.

If the operator wants it for the NVIDIA prize, the only cheap route is to **request Llama 3 access on Hugging Face now** and treat Kimodo as a
device-B side try after H48 if the approval has arrived.

## 4. Candidate 3: video to motion. Research only; verdict: no

| Service | Input | Output and skeleton | Free tier / cheapest paid / output rights | Time per clip | Fast lower body, skating | Path to UE5 | Agent? |
|---|---|---|---|---|---|---|---|
| **Rokoko Vision 3.0** (jam perk: Rokoko Basic) | one camera or a video file; "relies on a complete view of the performer"; dual-camera retired after 3.0 [vendor] | FBX on Free; **Basic adds BVH and "Import & retarget custom characters"**; FAQ: "compatible with all major skeletons including UE5 Manny" [vendor] | Free 30 s/month; **Basic 600 s/month, single camera, $10-12/mo**; terms: "we do not claim ownership over any User Content", no explicit statement on commercial use of Vision output [vendor] | "30-40 seconds to process 10 seconds" (2023) [3rd] | no foot-lock or skating claim; no skating examples | Manny export directly | web UI, no Vision API |
| **Move One** (single camera) / Move multi-cam | phone video, 30 s on Free, 60 s paid [vendor]; multi-cam needs Move Pro (about £995/month trial) [3rd] | FBX, USD; ships an `IK_MoveOne` asset for UE [vendor] | 30 credits once; Starter $18/mo = 60 credits, Gen 2 costs 2 credits per second [vendor]; commercial use not stated | not stated | single camera: "depth ambiguity [that] introduces foot-slide… no filter fully removes" [3rd] | IK Retargeter asset | API by request |
| **DeepMotion Animate 3D** | one video, 20 s per clip on Free and Starter [3rd] | FBX, BVH, GLB, MP4 [vendor] | 60 s/month; **"Freemium accounts are available for personal, non-commercial use"** [vendor] | not stated | the **only skating-aware control found**: "Grounding: Turns foot locking off while still enforcing character grounding… Recommended when foot gliding is desired… Available for Professional or higher plans" [vendor] | FBX + IK Retargeter [unverified] | web UI; API through sales |
| **Plask Motion** | one video; Free 15 s/day [vendor] | FBX, GLB, BVH [vendor] | Standard $18/mo; commercial use not stated | not stated | "twitching and sliding" [3rd] | IK Retargeter [unverified] | web UI |
| **Autodesk Flow Studio** (formerly Wonder Studio) | video of an actor | FBX body and hand mocap [3rd] | Free 300 credits, about 30 s, watermarked, "essentially a trial" [3rd]; Lite $10/mo | not stated | no data | IK Retargeter [unverified] | web UI |
| Meshy | **no video-to-motion feature** in its docs [vendor] | | | | | | |

**In plain words.** To use video you first need a video: one person filmed skating on our own ice time, or footage you own. NHL clips and
YouTube are a rights problem and add a recognisable person. Then:
1. The cheapest tool (Rokoko Basic, which we have) gives 10 minutes a month and exports straight to Manny. That part is easy.
2. No vendor shows a single skating result.
3. Every default clean-up locks planted feet; DeepMotion's paid Grounding mode is the only documented exception. A glide therefore comes back
   either as foot sliding the tool tried to remove, or as steps.
4. Single-camera depth errors are worst on fast leg motion.
5. The result then needs a retarget onto a toy that the game shows at 52 m, where our own test shows stride detail is invisible.

Hours go in and nothing visible comes out. **Use Rokoko Basic, if at all, for a slapshot or a body-check filmed off-ice in shoes**, as a T-B
try. Never for the skating stride.

## 5. Candidate 4: text-to-motion services. Research only

| Service | Input | Output | Free tier / price / rights | Skating | API |
|---|---|---|---|---|---|
| **Meshy Text to Motion** | prompt up to 400 characters, 2-10 s | `prime` FBX (10 credits), `swift` BVH (3 credits); not attached to a character, rig it through the Rigging API [vendor] | Free outputs CC BY 4.0, paid commercial (SKATERS-RESEARCH) | library has no skating (IDs 0-587); text-to-motion skating untested | REST, agent-doable |
| DeepMotion SayMotion | prompt | FBX, GLB, BVH | Free 5 credits / 1 download a month, Pro $39/mo with a commercial licence [unverified] | none | API through sales |
| Rokoko Create T2M (April 2026) | prompt, browser | FBX | Free: "up to five text-to-motion clips per month"; pitched for blocking, not final work [3rd] | none | web only |
| Uthana | prompt, 4-10 s | FBX, GLB, BVH | $0.10 per second; "Paid usage includes commercial use of generated outputs" [vendor] | none | GraphQL API |

Tripo animation presets (`fall`, `hit_to_side`, `shoot`) are covered in SKATERS-RESEARCH section 3.

## 6. Candidate 5: sourced clips. Mannequin sampled [run]; Fab pack not downloaded

- **Engine Mannequin clips** [run]. Copied from `Templates/TemplateResources/High/Characters/Content` (126 MB, no download) and sampled at 0,
  50, 75 and 100% under Simulate (`img/anim-mannequin-stock.png`):
  - `MM_Death_Back_01` lasts 1.10 s and `MM_Death_Front_02` 1.13 s, both without root motion. They are a "shot" stagger-and-crumple. They read
    as a fall, not as a slip on ice; at 100% the figure is still on its way down, so `AM_Fall` must hold the last frame or add a ground pose.
  - `MM_HitReact_Front_Hvy_01` (0.80 s) is a small flinch, too small to see at 52 m.
  - `MM_Attack_02` (1.0 s, root motion) is the closest thing to a body check: a lunging punch.
  - There is no skating; Jog and Dash are running.

  **A 2-hour retarget gives** `Fall` (x6 directions), `Check` (Attack_02 or a heavy hit react) and nothing for the stride, on any figure
  with a UE5 skeleton. It is useless for T-A, which has no skeleton.
- **Fab "Hockey Player Animated"** (Code This Lab; not bought or downloaded by this lane). By the listing it has 14 hand-keyed clips: idle,
  skate fwd/bwd/left/right as start, loop and stop, and shooting, on the Epic (UE4) skeleton.

  **A 2-hour retarget gives** skate loops, starts, stops and turns plus a shot, through the UE4 to UE5 script R2 proved in 4 s
  (`ANIMATION.md` section 4). Onto Manny this is low risk; onto a toy with a UE5 skeleton, expect arm-into-head clipping. With Manny falls
  and hits added, it is a complete T-B1 set except the check.

## 7. Recommended plan

| When | What | Who |
|---|---|---|
| **M1 ships** | **T-A procedural toy** per `skater-toy-figure.md`, with these number changes from this test: `k_lean` **2.5** (not 0.6), lean cap **35** (not 25), check pitch **30**, stick thickness 6 cm, a dark helmet band. Add a blade-spray Niagara burst on hard turns, stops and knockdowns (the condition and steps are in `tools/skills/animation-sources.md`). | device A |
| T-B1 (H12-H36) | Fab Hockey Player pack if the operator bought it: retarget the skate set and shot UE4 to UE5, plus Manny `MM_Death_*` for `Fall` and `MM_Attack_02` / `MM_HitReact_Front_Hvy_01` for `Check`. Promote only by the existing gate. | device B |
| T-B2 (H24-H48) | Meshy toy figure rigged `ue5_mannequin`, playing the **same retargeted Hockey Player clips** (one AnimBP for both tracks), Meshy library falls 366 / 502-505 as alternatives. Optional: one Meshy text-to-motion "ice skating stride, hockey crouch" clip, judged next to the Fab loop. | device B |
| optional, after H48 | Rokoko Basic: a slapshot and a shoulder check filmed off-ice in shoes, if T-B2 is ahead of schedule | device B with a human |
| **dropped** | video-to-motion for skating (all vendors); Kimodo (downloads, gated approval, no agent path); Control Rig on a toy skeleton; Mixamo; physics tip-over | |

The operator acts before H0 only if they want Kimodo kept open: request Meta Llama 3 access on Hugging Face. It costs nothing and the wait can
be weeks.

## 8. Errors and surprises during the run

1. **Assets already exist.** A second run of the setup script hit `NewLevelFromTemplate ... An asset already exists at this location` and
   `CanCreateAsset cannot ask the user as the application is running unattended`. Setup scripts must check `does_asset_exist` first.
2. **The editor world does not tick skeletal animation.** `override_animation_data` and `set_animation_mode(SINGLE_NODE)` +
   `set_position` left the head bone at z 160.2 for every sampled time. **Under Simulate** (`StartPIE {"options":{"bSimulate":true}}`), the same
   calls on the game world's actors plus `await asyncio.sleep(0.15)` sampled correctly: head z went 160, 148, 115, 118, 122.
3. **Copying content into a running editor's `Content/`** logged one `OpenFile failed ... being used by another process` while the asset
   registry scanned. `scan_paths_synchronous(['/Game/Characters'], True)` afterwards loaded everything.
4. **Pose updates.** A SceneCapture `capture_scene()` right after `set_actor_transform` captured the new pose when the script yielded
   `await asyncio.sleep(0.03)` first. No stale frames were seen.
5. **Bypass rule.** Basic-shape materials need `M_Toy` with a `TeamColour` parameter. It was made by `MaterialEditingLibrary` in one call
   (allowed in a throwaway; in the jam it is the T-A material the recipe names).
