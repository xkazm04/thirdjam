# SKI2 checkpoint: P2C done (22:28 machine clock)

Lane SKI2, device B, main tree, editor port 8000. No build, no git write.

**P2C is in the engine behind `BP_SkaterVisual_TB2`** (class name unchanged); the previous Blueprint is kept as `BP_SkaterVisual_TB2_old` (self-contained: its PlayCheck now looks for its own class). Verified in PIE with `jam.Cut` = scratch cartridge `generated_b2` + `l2_night`: log `JAMVISUAL generated_b2 tier=l2_night class=/JamAssets/Skaters/BP_SkaterVisual_TB2.BP_SkaterVisual_TB2_C`, six P2C figures, numbers 7 13 42 / 9 21 88.

| Step | Result | Numbers |
|---|---|---|
| 1 Import | PASS | `SK_SkaterGen_P2C` + `SKEL_SkaterGen_P2C` (32 bones), 72 `AS_SkaterGen_P2C_*` from the loopcut file (Stride wrap loopcut 1.03 cm blade / 0.56 hand against a mean step of 1.40: kept; main file 0.00 but a repeated frame), no physics asset, sockets `stick_socket`, `head_socket`, `puck_socket` (bone rest identical to R's, so R's socket values apply), `T_SkaterGen_P2C_BaseColor` 1K in game, BC1, RDO Medium |
| 2 Material | PASS | `MI_SkaterGen_P2C` (instance of `M_SkaterTB2`): KeyGain 10, KeyBias 2.3 (keys the deep blue, leaves navy pants and mittens, face, cage, skates, white trim); number box Z 88, Y -6, W 40, H 24, NumberWidth 0.45, NumberHeight 0.6; crest Z 97, Y -7, size 10; jersey Emissive 1.25 (boost 1.75) at night |
| 3 AnimBP + Blueprint | PASS | `ABP_SkaterGen_P2C` (copy of `ABP_SkaterTB2`, Idle state plays `Idle`), `BS_SkaterGen_P2C_Skate` 14/0 empty, `BS_SkaterGen_P2C_Aim` 24/0 empty. Blueprint = TB1's SK5 graph (stumble cut, bus strength, trip get-up, 0.12 s reactions, guard) with TB2's material path. Aim: Glide worst 0.0 cm, blade bottom +0.5 cm; Stride worst 0.9 cm, bottom -0.4..+1.7 cm; correction used max 3.4 deg / 4.5 deg / 5.2 cm. Hand to shaft 6 to 16 cm on two-handed holds (mitten radius 12.2). Carry 12 frames with a 13 deg turn: 4.9..6.9 cm puck to blade centre |
| 4 Match | PASS (Knockdown_Left on test figure only) | Match log: Knockdown_Back x3, Knockdown_Right x2, Fall x2, each with its get-up; strength rates 1.06..1.15; human check `Check_Deliver_Left` -> victim `Fall`. No error or warning naming P2C assets during play |

Stills: `devlog/shots/20261112-2218*-B-ski2-p2c-*`, `20261112-2223*-B-ski2-p2c-*`.
Next: P3C, then P4D. Full report at the end in `status/SKI2-report.md`.
