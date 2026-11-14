# SK25 filming script for the picture lane

Three strips, about ten minutes on the GPU editor. Everything below was run headless in lane SK25 and measured by bone positions; nothing here has been seen as a picture.

## Before you start

- The branch `lane/b-fig` must be merged into the tree your editor runs. Both figure Blueprints and both animation Blueprints (`ABP_SkaterTB1`, `ABP_SkaterTB1_P3`) carry the change.
- Cartridge: `jam.Cut hockey.regular` (seed 1, clay). No scripted human is needed for the test figures.
- For stills at the window's resolution: `r.Editor.Viewport.OverridePIEScreenPercentage 0`, and back to `1` afterwards.
- Keep every test figure in view. A figure off screen does not advance its pose (`OnlyTickPoseWhenRendered`).
- Start the play session, wait two seconds, then run block 0 once.

## Block 0: the test figures (Python, once per play session)

```python
import unreal
gw = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
pc = unreal.GameplayStatics.get_player_controller(gw, 0)
BP = 'BP_SkaterVisual_TB1_P3'          # 'BP_SkaterVisual_TB1' for the night figure
def spawn(x, y, yaw, mode=0):
    before = set(a.get_name() for a in unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.Actor))
    unreal.SystemLibrary.execute_console_command(gw, 'summon /JamAssets/Skaters/%s.%s_C' % (BP, BP), pc)
    a = [a for a in unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.Actor) if a.get_name() not in before][0]
    r = unreal.Rotator(); r.yaw = yaw
    a.set_actor_location_and_rotation(unreal.Vector(x, y, 0), r, False, True)
    a.set_team(0, unreal.LinearColor(0.0122, 0.0762, 0.6867, 1.0)); a.set_number(13)
    if mode: a.call_method('SetTestMode', (mode,))
    return a
F = {'L1': spawn(600, -600, 0), 'L2': spawn(600, -900, 0, 8),      # two to lie down; L2 takes rule B
     'W1': spawn(-150, -600, 0), 'W2': spawn(150, -600, 0),         # the two whips
     'G':  spawn(-600, -900, 0)}                                    # the back get-up
def hold(dt):
    for a in F.values():
        r = unreal.Rotator(); a.update_motion(0.0, 0.0, r)
unreal._sk25film = {'F': F, 'h': unreal.register_slate_post_tick_callback(hold)}
```

`SetTestMode(8)` puts a figure on rule B whatever the bus count says. `SetTestMode(16)` switches the whip off for a figure (and also means rule B), if you want a twin without it.

When done: `unreal.unregister_slate_post_tick_callback(unreal._sk25film['h'])`, then stop the play session. Nothing is saved.

## Strip 1: two lying players moving differently

```python
F = unreal._sk25film['F']
for k in ('L1', 'L2'): F[k].set_knocked_down(True, 0.0, 1.0, 110)
# 110 frames later:
for k in ('L1', 'L2'): F[k].set_knocked_down(False, 0.0, 0.0, 0)
```

- Both fall on the left side (`Knockdown_Left`), lie on `Lie_Loop_Side_Left` from knockdown frame 16, and get up (`GetUp_Side_Left`) from frame 75 to 110.
- L1 (rule A) starts the loop on frame 93 and shows the first move: the top forearm drags in and the head lifts. L2 (rule B) starts on frame 39 and shows the second: the top knee draws up.
- Both moves fall between knockdown frames 46 and 70. **Capture frames 40, 46, 52, 58, 64, 70, 76** with both figures in one picture.
- Measured: L1's forearm travels 18.4 cm, L2's shin 16.9 cm; the two differ by up to 18.6 cm at the forearm while both lie.
- Camera: the game camera's height and angle is the honest one. For a readable strip, from above the pair: about (600, -750, 450) looking straight down, or the game camera cropped on them.

## Strip 2: a head whip at Strength 1.0 against 0.3

```python
F = unreal._sk25film['F']
F['W1'].call_method('WhipGoS', (1.0, 0.0, 1.0))
F['W2'].call_method('WhipGoS', (1.0, 0.0, 0.3))
```

- Both figures face +x and are thrown along +x. Nothing else plays: the whip is laid over the idle stance.
- The head peaks on frame 3 to 4 (13.9 cm on W1, 4.2 cm on W2), springs back through zero to about 2 cm the other way on frame 12, and is at zero on frame 20.
- **Capture frames 0, 2, 4, 6, 9, 12, 16, 20.** Camera from the side: about (0, -950, 120) looking along +y, so the throw runs left to right across the picture.
- With a take under it, as in play: `F['W1'].call_method('WhipStr', (1.0,)); F['W1'].call_method('PlayStumble', (1.0, 0.0, 16))` and the same on W2 with 0.3. The take is `Stumble_Back`; a stumble takes the Strength whole (13.8 cm at 1.0). `play_hit_reaction(1.0, 0.0)` in its place plays `Check_Hit_Back` at half the size.
- Other directions: `(-1, 0)` from the front, `(0, 1)` and `(0, -1)` from the sides.

## Strip 3: the back get-up with the stick beside the skate

```python
F = unreal._sk25film['F']
F['G'].set_knocked_down(True, -1.0, 0.0, 110)
# 110 frames later:
F['G'].set_knocked_down(False, 0.0, 0.0, 0)
```

- A hit from the front: `Fall` or its spin variant `Fall_B` (they alternate), then `Lie_Loop` on the back, then `GetUp` (export29's, the stick carried out through the sit) from knockdown frame 75 to 110.
- **Capture frames 75, 80, 85, 90, 95, 100, 105, 110**, once from the side and once from the game camera. The thing to look for is the shaft against the legs between frames 80 and 100.
- A knockdown of 80 frames plays `Fall_Light` and `GetUp_Quick`; under 70, `GetUp_Snap`. All three back get-ups changed in export29.

## The same things in a real match

Seeds 1 and 7920, `jam.ScriptHuman 1`, clay. The sim frames below are from this lane's headless runs; seed 1 repeated exactly between two runs in lane SK24, seed 7920 nearly.

| What | Seed | Sim frames | Who | What plays |
|---|---|---|---|---|
| Two players down together, moving differently | 1 | 10163 to 10225 | skater 7 (down at 10141 for 84 frames) and skater 1 (down at 10163 for 150) | 7: `Knockdown_Left_Light`, `Lie_Loop_Side_Left` on rule B (the knee). 1: `Fall_Heavy`, `Lie_Loop` on rule A (the forearm, then the knee) |
| A whip at Strength 1.0 | 1 | 10163 | skater 1, hit by skater 5 | knockdown, the head thrown about 7 cm (a knockdown takes half) |
| A whip at Strength 1.0 on the human | 7920 | 2317 | the human, hit by skater 5 | `Knockdown_Left_Heavy`, 150 frames |
| A weak whip | 1 | 1888 | the human, Strength 0.19 | a stumble, the head thrown under 3 cm |
| A stumble at Strength 0.61 | 1 | 1238 | the human, hit by skater 6 | a stumble, about 8.5 cm |
| The back get-up | 1 | 10277 to 10313 | skater 1 | `GetUp` after `Fall_Heavy` |

In a match the whip at full size is rare: a stumble takes the Strength whole, a knockdown and a board hit take half, a plain collision takes 0.3. The test figures of strip 2 are the place to see 1.0 against 0.3 side by side.

## What I could not check for you

- Whether any of it reads at the game camera. That is the point of the film.
- Whether the stick hand's lag pulls the lower hand off the shaft in a way the eye catches (the two hands' distance changes by up to 1.9 cm at Strength 1.0).
- Whether a whip that starts while another is still running shows a pop.
