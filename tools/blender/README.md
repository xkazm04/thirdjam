# tools/blender: the Blender pipeline for figures and props (lane A22)

Blender 4.2 headless only (`-b -P`), never an interactive window. Renders use the GPU (EEVEE Next): stop a run when the Director asks for the GPU.
Everything is parametric Python; no `.blend` file is kept. Outputs go to `tools/blender/out/` (FBX sources, JSON stats, logs, render frames) and the
review images to `docs/experiments/img/`. The experiment notes are `docs/experiments/BLENDER.md`.

## Job list

| Job | Command (from the repository root, Git Bash) | Makes | Time on device A |
|---|---|---|---|
| all | `bash tools/blender/run_all.sh` | everything below | about 60 s |
| figures | `bash tools/blender/run_all.sh figures` | `out/SK_SkaterToy.fbx`, `SK_SkaterClay.fbx`, `SK_SkaterBroadcast.fbx` (+ `.json`), the 3 x 3 contact sheet `docs/experiments/img/blender-sheet.png` (cells 1024 px; `-1536` preview), three 2 s turntables `blender-turntable-<variant>.mp4` | 13-18 s per variant |
| one figure | `"$BLENDER" -b -P tools/blender/skater.py -- --variant toy --out tools/blender/out [--render] [--turntable] [--no-export]` | one FBX, its cells, its turntable frames | 2 s without renders |
| props | `bash tools/blender/run_all.sh props` or `"$BLENDER" -b -P tools/blender/props.py -- --out tools/blender/out --render` | `out/SM_Net.fbx`, `SM_Board_Segment`, `SM_Bench`, `SM_Puck`, `SM_GoalLight`, `SM_Pedestal_Round`, `SM_Pedestal_Hex`, `out/T_Props_Atlas.png` (1K RGBA), `props.json`, `docs/experiments/img/blender-props.png` | 6 s |
| check | `"$BLENDER" -b -P tools/blender/check_fbx.py -- tools/blender/out/SK_SkaterToy.fbx` | re-imports the FBX into a clean Blender and prints the forearm-tip-to-mitten gap per action (PASS under 2 cm) | 5 s |
| sheet | `bash tools/blender/run_all.sh sheet` | re-tiles the sheet and re-encodes the MP4s from the frames already in `out/render/` (ffmpeg only) | 2 s |
| import (toy) | `"C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" Game/ThirdJam.uproject -run=ImportAssets -importsettings=tools/blender/import_toy.json -unattended -nopause -nosplash -NullRHI -log` | `Game/Content/Figures/SK_SkaterToy` + skeleton + 19 `AS_` sequences (legacy FbxFactory, materials and textures not imported) | 1-3 min; only when no other `UnrealEditor-Cmd` or package runs |

`BLENDER` defaults to `C:/Program Files/Blender Foundation/Blender 4.2/blender.exe`; `run_all.sh` writes `out/timings.txt` and `out/log_<job>.txt`.

## Conventions baked into the scripts

- Scene unit scale 0.01 (1 Blender unit = 1 cm); parameters at the top of each script are metres. FBX export: `apply_unit_scale`, `FBX_SCALE_NONE`,
  forward -Z / up Y (Blender's default), so Unreal gets centimetres and an armature at scale 1.0 (lane B4 saw a root at scale 100 with the metre recipe).
- The figure faces Blender +X, stick on Blender -Y. Unreal's import flips Y: in Unreal the figure faces +X with the stick on +Y, the toy figure's frame.
- Sockets are leaf bones (`stick_socket`, `puck_socket`, `head_socket`); Unreal treats bone names as sockets (`DoesSocketExist` / `GetSocketTransform`).
  Static-mesh sockets are empties named `SOCKET_<name>` (`SOCKET_PickupTop` on the pedestals).
- Every action is keyed in Unreal's pose vocabulary (body pitch, sway, roll, bob, stick yaw/pitch/forward, tip) and converted once in `apply_channels`;
  timings are the ones in `ToySkaterVisual.cpp` (shot keys 0/18/25/34/40/51, check 8 in 12 out, knockdown 14 + bounce, get-up 20).
- Material slots on the skater (plain names, first-use order): `base` (TeamColour), `number` (flat UV 0..1 square on the back for NumberTens/NumberOnes), `head`,
  `trim`, `steel` (skate blade strips, cage line), `stick`. Arms: two bones each, IK-posed and baked to plain keys (`bake_arms`); `check_fbx.py` proves the bake by re-import. Props: one slot `M_PropsAtlas` on `T_Props_Atlas.png` (net pattern top half with alpha; colour bands bottom half).
