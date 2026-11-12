# Lane A22: Blender pipeline for the skater figure, stick and props (device A, from H9.3)

You are lane A22 of the Third Jam (team Hokej). The operator at H9: the machine is idle most of the day and "we are not close to P2"; use Blender for the figures and props that
code and Rodin cannot give us cleanly. Blender 4.2 is installed (`C:\Program Files\Blender Foundation\Blender 4.2\blender.exe`, run it headless with `-b -P script.py`).
Read first: `docs/HOCKEY.md` (art ladder, P2 toy figures, P3 clay, P4 broadcast), `contracts/skater-visual.md` (C-0001 v3: the calls a visual implements, `puck_socket`),
`contracts/assets-naming.md` (C-0004), `Game/Source/JamGame/Private/ToySkaterVisual.cpp` (the code-built toy figure: proportions, stick, glove blobs), `docs/experiments/ANIMATION-SOURCES.md`,
`handoff/A-to-B-0007.md` (the operator's stick and hands comment), `devlog/shots/20261112-144641-A-a16-*.png` (how the figure reads today), R-0021 and R-0022 in `contracts/RULINGS.md`.

You own: `tools/blender/**` (scripts, job list, README), `Game/Content/Figures/**` (imports: `SK_`/`SM_` + textures, FBX sources kept under `tools/blender/out/`), rows in `ASSETS.md`,
`status/A22.md`, `docs/experiments/BLENDER.md`. Run no git command; no Unreal editor unless you ask the Director for a slot (imports can go through the MCP editor of lane A23 by
handoff, or `UnrealEditor-Cmd -run=ImportAssets` headless). Blender renders on the GPU: stop a render when a message from the Director asks for the GPU.

## Job
1. **A parametric toy skater in bpy** (`tools/blender/skater.py`): body as the toy figure today (capsule torso, ball head, stub legs on a skate plate), two mitten hands on a
   **hockey stick with a shaft and a lying blade** (shaft 1.3 m, blade 0.3 m, heel angle), team-colour material slots (base, trim, number), `puck_socket` empty on the blade, a
   `stick_root` bone so the stick can aim, and a small armature (root, pelvis, torso, head, two arms, stick) with the poses from `contracts/skater-visual.md` v3 baked as actions:
   idle, skate (cycle), carry, aim left/right extremes, poke, lift, shoot wind-up and release, check, knocked down, faceoff ready, draw, goalie stance (left, centre, right).
   Export FBX for Unreal (Z up, cm, with animations). Parameters at the top of the script: proportions, stick lengths, colours, `variant` in {toy, clay, broadcast}: `clay` adds
   fingerprint-dimple displacement and a felt-stitch seam; `broadcast` lengthens the limbs toward a humanoid 1.8 m figure with a helmet (no face). One script, three figures.
2. **Props the arena needs without film props** (`tools/blender/props.py`): a net with a tubular frame and a mesh netting (alpha card), boards segment with a kick plate and a
   glass panel, a bench, a puck (disc, chamfer), a goal light housing, two kinds of pickup pedestal. Low poly, one material slot each, 1K textures at most.
3. **Renders for the operator**: a 3 x 3 contact sheet (toy, clay, broadcast x idle, carry, poke) at 1024 px, PNG, in `docs/experiments/BLENDER.md`, plus a 2-second turntable
   MP4 per figure via ffmpeg (under 2 MB each). The Director turns the sheet into a review page.
4. **Import** the toy variant into `Game/Content/Figures/SK_SkaterToy` with its animations (headless import or through A23's editor by message), ledger rows (`hand-made`, tool
   `Blender 4.2`, script path in `url`, `size_mb`), and write how `ToySkaterVisual` or a Blueprint visual would drive it (which call maps to which action). Do not replace the
   code-built figure; the swap is the Director's call after the review.
5. Status in `status/A22.md` with real `date` timestamps: script runtimes, triangle counts, file sizes, what failed.

Report: the contact sheet path, the FBX list with sizes and triangle counts, the import result, and three risks. Time box 2 hours; interim report at 45 minutes with the toy figure render.
