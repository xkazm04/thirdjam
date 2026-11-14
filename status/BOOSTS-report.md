# BOOSTS report: lane B2, boosts you can see (14 Nov, 14:15 to 14:50)

Lab: `C:\Users\kazda\jam\sets-lab\boosts\`. Blender 4.2 (`C:\Program Files\Blender Foundation\Blender 4.2\blender.exe`, the weapons lab's), Cycles on the CPU,
16 samples, 6 threads. I ran one Blender at a time. No editor, no git. Nothing written under `Game\`.

## 1. Done and not done
- **Done: the six pickups** `SM_Boost_<kind>`, as FBX with stills. The six can be told apart at 45 m from a game-camera stand-in.
- **Done: the equipped set** for shotgun, hip_check, shield and slow_puck. There is one FBX per ability and style (p3_clay, p2_cel), with its bone and socket. Each was measured against the skinned body in all 149 takes of export35. Stills are on the clay figure.
- **Not done:**
  - Nothing has been imported into or seen in the engine.
  - There are no `.uasset` files.
  - There are no stills of the cel figure with the props (only its numbers).

## 2. Items

### Pickups (`export\SM_Boost_*.fbx`)
**What they are:** shape first, then colour. The main clay colour is the kind's colour from `contracts/events.md`.

| Kind | Shape | Colours |
|---|---|---|
| speed | a winged lightning bolt; its wings face the camera when the bolt is edge-on | cyan |
| precision | a bullseye target, ring by ring | magenta and cream |
| heavy_puck | a kettlebell on a dark puck | violet |
| big_puck | a huge puck tumbling on its rim | gold with a dark band |
| reverse_other | two fat curved arrows going round a circle | red |
| custom | a cream block with a dark question mark on four sides and the top | cream; no rivets and no yellow, so it is nobody's block |

Triangles: 400, 840, 692, 560, 608 and 1,124, so all are under 1,500. Heights are 64 to 83 cm. The origin is at the base centre. The slots are `Clay` and `Accent`.

**The proving number:** on a 720-px frame from a camera 35 deg down, the figure is 110 px tall at 25 m.
- At 25 m the props are 45 to 64 px.
- At 45 m the props are 25 to 35 px and the figure is 60 px.

**Still:** `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-142438-B-boosts-pickups.png` (with a `.txt` caption).

**What I looked at (Read):**
- That sheet: each prop alone from three turns, the 25 m and 45 m rows at native pixels, and the 45 m row enlarged.
- Two earlier field renders.

At 45 m all six tell apart by silhouette and colour. Speed is the smallest read, a cyan zigzag with pale wings. Custom has the weakest contrast on pale ice (cream on pale), and its glyph is a dark blot at 45 m.

Fixes after looking:
- speed: bolt 30 % wider and thicker, and wings in a pale cyan, because cream wings disappeared on the ice.
- reverse_other: arrow heads in dark red, because cream heads disappeared.
- custom: question marks 1.3 times larger.

### Equipped (`export\SM_Equip_<ability>_<style>.fbx`)
Each is one rigid mesh on one bone. The prop frame is the figure's rest frame (+X forward, +Z up) with its origin at a pivot.

| Ability | What it is | Bone |
|---|---|---|
| shotgun | a timber keg with dark hoops and a puck in the lid, on the back | spine_03 |
| hip_check | a padded yellow bumper on the left back of the hips | pelvis |
| shield | a round green shield with a cream rim and boss, on the back | spine_03 |
| slow_puck | a white snowflake crest of two crossed flakes, on a pale-blue stalk on the helmet | head |

The pivots, sockets and colours are in `C:\Users\kazda\jam\sets-lab\boosts\export\MANIFEST.md`.

**Two changes from the brief's examples, made because of what I looked at and measured:**
- **The shield is on the back, not the forearm.** In the ready stance the left forearm is in front of the trunk. From behind, the camera saw nothing of a forearm shield, and from the side only a green fleck.
- **The hip guard is one bumper on the left back, not pads on both sides.**
  - Pads on the hip sides sat inside the hanging arms in every take.
  - A bumper across the whole back met the right hand, which rests behind the right hip in Glide and Idle.
  - So I measured which angles round the hips the arms and hands use, in Glide, Idle, Stride, Stride_Sprint, Check and Shot_Slap. The arc from 126 to 172 deg (the left back) is the only free one, and the bumper fills it.

**The proving numbers, p3_clay:** all 149 takes, every 2nd frame, 2,702 frames. A prop vertex counts as inside when it is more than 1 cm behind the nearest face of the skinned body.

| Prop | Glide, Idle, Stride, Check, Fall | Stride_Sprint | Takes with any frame inside | Lowest point while lying |
|---|---|---|---|---|
| shotgun | 0 frames | 0 | 24 (hands and arms in hits and get-ups; worst 4.7 cm) | -24.3 cm, in 10 lying takes |
| hip_check | 0 frames | 4 frames, a hand 4.1 cm in | 38 (worst 12.2 cm, GetUp_Snap f6) | -17.1 cm, in 21 takes |
| shield | 0 frames, except Fall: 1 frame at 1.3 cm | 0 | 36 (worst 6.0 cm) | -11.9 cm, in 10 takes |
| slow_puck | 0 frames | 0 | 15 (hands over the head in knockdowns) | never under the ice |

**p2_cel, the same measurement:**
- shotgun, shield and slow_puck: 0 frames inside in Glide, Idle, Stride, Check and Fall.
- hip_check: Idle 4 of 61 frames, with the trunk 7.0 cm in. Fall 9 of 14 frames, at up to 1.1 cm.
- The cel back and hip faces are taken from every body vertex (`ENVELOPE` in the script), not only the spine's. On the cel figure the first rule put the keg and shield inside the trunk.

**Still:** `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-144421-B-boosts-equipped.png` (with a `.txt` caption).

**What I looked at (Read):**
- That sheet: four rows, each with Glide from behind and from the front, Stride frames 15 and 45, and Knockdown_Back frame 26, plus a 25 m game-camera row.
- Two earlier versions of it, which led to moving the shield and to the crossed snowflake. The single flake was a line from the side.

At 25 m: the keg, the green shield and the snowflake read clearly. The hip bumper reads as a yellow lump at the left hip.

## 3. Not verified
- Nothing has been checked in the engine: the import, the socket transforms (the Unreal-axes column is my Y-flip of the Blender numbers, as the weapons lane did, and has not been tested), the materials, or the spin.
- The game camera is a stand-in (35 deg down, vertical FOV 20.6 deg), not the game's real camera.
- The props were never tested on the pedestal of light or against `NS_pickup_idle`.
- The p2_cel props were never rendered.
- The clearance test uses vertices only. A thin body part could pass between prop vertices unseen.

## 4. For the engine lane / device A
**Pickups**
- Import each `SM_Boost_<kind>.fbx` to `/JamAssets/Props/SM_Boost_<kind>`, with `Clay` and `Accent` as two material instances in the clay material, in the colours listed in MANIFEST.md.
- A's A188 scales them to 0.8 m. The meshes are 64 to 83 cm tall already, so the scale is close to 1.
- Older `SM_Pickup_<kind>` meshes with `MI_PickupProp_*` and `T_Pickup_*` already exist in `Game/Plugins/JamAssets/Content/Props/`. A looks for `SM_Boost_*`, so decide which ones ship.

**Equipped**
- Import `SM_Equip_<ability>_p3_clay.fbx` (and `_p2_cel` for the cel figure) as static meshes.
- In `SetBoostEquipped(Kind, bOn)`, show one static-mesh component per kind, attached to its bone. One way: at the reference pose, set its relative transform to (pivot, no rotation, in component space) x inverse(the bone's reference-pose component transform). The bone-frame numbers are in MANIFEST.md.
- Pivots for p3_clay, in Blender component cm:

| Kind | Bone | Pivot (x, y, z) |
|---|---|---|
| shotgun | spine_03 | (-32.32, 0, 108.53) |
| hip_check | pelvis | (0, 0, 88.0) |
| shield | spine_03 | (-21.22, 0, 105.53) |
| slow_puck | head | (0.62, 0, 162.51) |

  Unreal Y = -Blender Y, which is 0 here.
- **Hide the equipped prop while the figure is down**, from the knockdown or fall to the end of the get-up. The back props go up to 24 cm under the ice while he lies on his back, and that cannot be fixed by placement.
- First test: shotgun on p3_clay in Glide. The keg should sit about 2 cm off the jersey, centred on the upper back.

## 5. Decisions for the operator
- **The shield is on the back, not the arm.** That is the only place the game camera sees it. Do you want it on the forearm anyway, as the brief's example had it?
- **The hip_check bumper is on the left hip only,** because the right hand rests on the right back hip in the ready stance. Is one side enough, or should it be a different mark (for example on the thighs, which would need two components)?
- **Which pickups ship:** the new `SM_Boost_*` or the existing `SM_Pickup_*`.

## 6. Next three steps
1. Import into a headless tree, put the pickups on the pedestal, and take GPU stills from the real game camera at 25 and 45 m. This is the engine lane's job, after the 15:00 merge.
2. Stills of the cel figure with its four props, and a second pass on the cel hip bumper (Idle 7 cm).
3. If the operator wants it, a small turn or pulse for the equipped props, which can only be judged in motion in the engine.

## 7. Ledger rows
None. This lane made no asset under `Game/Plugins/JamAssets/`; it made FBX files in the lab only. Draft rows for the importing lane, with `size_mb` to be replaced by the `.uasset` size after import, are in the hand-back message.
