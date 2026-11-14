# SK26 filming script for the picture lane

Every move of the three new sets, fired from the console on the human's figure, in about ten minutes. Nothing here has been seen as a picture: it was all run headless and measured by bone positions.

## Before you start

- The branch `lane/b-fig` must be merged into the tree your editor runs. Both figure Blueprints, both animation Blueprints, 56 new takes and the props under `Skaters/Props/` carry the change.
- Any cartridge. `jam.ScriptHuman 0` is best: a standing human is the cleanest subject. With `jam.ScriptHuman 1` the moves still fire, over whatever the scripted human is doing.
- For stills at the window's resolution: `r.Editor.Viewport.OverridePIEScreenPercentage 0`, and back to `1` afterwards.

## The trigger

One console command fires a move on **the human's figure and on every summoned test figure**. Bots ignore it. Nothing calls it in a normal match.

```
ke * SK26Test <move>
```

From Python: `unreal.SystemLibrary.execute_console_command(game_world, 'ke * SK26Test 4', player_controller)`.

| Move | What it fires | What plays |
|---|---|---|
| 1 | `PlayDiveBlock`, shooter a little to the right | `Dive_Block_Left`, lying, the side get-up; 101 frames in all |
| 2 | `PlayDiveBlock`, shooter a little to the left | `Dive_Block_Right` |
| 22 | `PlayDiveBlockFront` | `Dive_Block_Front`, face down, the front get-up; 101 frames |
| 3 | `PlayDiveBlockHit` | the 12-frame jolt; only while lying after move 1 or 2 |
| 4 | `PlaySlideTackle`, side +1 | `Slide_Tackle_Left`, lying on the back, the back get-up; 85 frames |
| 21 | `PlaySlideTackle`, side -1 | `Slide_Tackle_Right` |
| 5, 6, 7 | `SetWeapon` 1, 2, 3 | the giant stick, the mallet, the cannon in the hands, with its carry pose from the waist up |
| 8 | `SetWeapon 0` | the stick back |
| 9 | `PlayWeapon`, aimed ahead, hit | giant stick: `Giant_Poke`. Mallet: `Mallet_Swing`. Cannon: `Cannon_Kick` (fires, sits down, rolls onto the right side, gets up) |
| 10 | `PlayWeapon`, aimed to the side, miss | giant stick: `Giant_Sweep`. Mallet: `Mallet_Miss`. Cannon: `Cannon_Fire` |
| 11 | the mallet's victim | `Fall_Bonk`, lying on the back, the back get-up; 110 frames |
| 12 | `PlayDash(96)` | `Dash_Start`, four cycles of `Dash_Loop`, `Dash_End`, then the stride |
| 13, 14 | `EndDash`, braked, turning left / right | `Dash_Skid_Left` / `Dash_Skid` |
| 15, 20 | `EndDash`, bumped through, victim on the left / right | `Dash_Bump_Left` / `Dash_Bump_Right`, then the loop goes on |
| 16 | `EndDash`, lost | `Dash_Wipeout`, face down, the front get-up; 86 frames |
| 17 | `PlayBoostPickup` | `Boost_Take` from the waist up, 12 frames |
| 18, 19 | the boost as a level, on / off | the loop for as long as it is on, then `Dash_End` |

## Ten minutes

1. **The dive, 2 minutes.** Move 1, wait 110 frames. Move 2, and 45 frames later move 3 (the jolt lands while it lies). Move 22. Capture each from the game camera and from in front of the figure (the shooter's view). Frames to keep: 0, 5, 10 (full cover), 15, 18 (the second reach and the shuffle), 27, 35 (lying). Move 3's jolt peaks on its frame 2.
2. **The slide, 1 minute.** Move 4, wait 90 frames, move 21. Frames 0, 5, 10 (contact), 15, 19 (lying).
3. **The giant stick, 2 minutes.** Move 5, then 9 (poke, contact frame 9, angled 15 degrees right), then 10 (sweep, contact 10 to 14), then 8. **It is a stand-in:** the lab's giant stick file is empty, so the figure's own stick mesh is shown 1.7 times as large.
4. **The mallet, 2 minutes.** Move 6, then 9 (swing: up beside the helmet by frame 10, contact 17), then 10 (miss, 50 frames). For the pair: summon a second figure 1.08 m ahead of the first and 0.72 m to its left, facing it, and fire move 11 on it 16 frames after move 9 on the first (two `ke` lines by object name instead of `*`: `ke BP_SkaterVisual_TB1_P3_C_0 SK26Test 9`). **Also a stand-in:** the mallet's file is empty too; a short stick stands in, with no drum.
5. **The cannon, 1 minute.** Move 7, then 10 (fire: the puck leaves at frame 8; there is no puck from me), then 9 (the kick), wait 95 frames, then 8. This one is the real prop. `Cannon_Fire` is 25 frames; the keg jumps at 10 and settles by 17.
6. **The dash, 2 minutes.** The human must be skating (hold forward, or `jam.ScriptHuman 1`). Move 12 and let it run out: look at the feet where `Dash_End` hands over to the stride, 12 frames after it starts. Then 12 and 13 after 40 frames; 12 and 15; 12 and 16; then 17 on its own while skating.

## What to look for, because I could not

- Whether the stick is really hidden while a weapon is shown. I hide the stick's mesh section; the engine reports it hidden, but nothing was drawn.
- The cannon in the hands: its mouth measures 3.8 cm ahead of and 3.5 cm below the lab's point at the firing frame, sideways exact.
- The first frame after `SetWeapon`: the carry pose and the prop appear together, with no blend.
- One frame of the prop at the start of a hit reaction or a stumble before the stick comes back.
- `Boost_Take` a second time: it may show only its last frame (see the report).
- The dive's act (frames 12 to 20) from the shooter's side.

## Test figures, if you want them beside the human

```python
import unreal
gw = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
pc = unreal.GameplayStatics.get_player_controller(gw, 0)
unreal.SystemLibrary.execute_console_command(gw, 'summon /JamAssets/Skaters/BP_SkaterVisual_TB1_P3.BP_SkaterVisual_TB1_P3_C', pc)
```

Move it with `set_actor_location_and_rotation`. **Do not set any property on its mesh component with `set_editor_property`** unless you pass `unreal.PropertyAccessChangeNotifyMode.NEVER`: a plain set rebuilds the figure and wipes every variable it set at begin-play (see the report, job 1).
