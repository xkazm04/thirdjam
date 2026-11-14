# B43 filming script for the GPU session

Eleven new systems were built without a renderer in lane B43. **None of them has been seen, and none has been simulated**: Niagara compiles and saves under the null RHI but does not spawn or tick there. Everything below is for one GPU session of about fifteen minutes.

## Before you start
- The tree must hold B43's `VFX/` assets (branch `lane/b-aud`) and the figure lane's takes and `SK26Test` trigger.
- Cartridge: clay (`jam.Cut` on a `p3_clay` cut), `jam.ScriptHuman 0` so the human's figure stands where it is put; `r.Editor.Viewport.OverridePIEScreenPercentage 0` for stills, back to `1` after; `Slate.bAllowThrottling 0` if the world's time stays at 0.
- Start the play session, wait two seconds, run the block once. It was dry-run headless in B43: the moves fire, the schedule runs, and every spawn returns nothing there, as expected.
- Each strip: `unreal._b43film['run']('<name>')`, then poll `unreal._b43film['state']()` until it says `True`. Stills land in `devlog/b43/<name>-f<frame>.png` (the game's own screenshot, un-paused). Run each strip twice: once as it is (the game camera), once after `unreal._b43film['closecam']()`; `unreal._b43film['gamecam']()` goes back.
- Frame share: measure as in B42 and FILM1 (the same frozen frame with and without the effect, pixels that differ by more than 12 of 255 after a 1 px blur). The "should hit" numbers are predictions from one calibration (FILM1's fan: 34,200 cm2 of sprites measured 2.99 % of the frame at the human's distance); expect them within a half either way.

## The strips
| Strip | What it fires (frame: move) | Effect spawned by the block (frame: system at where) | Frames to capture | It should hit |
|---|---|---|---|---|
| `dash` | 0: move 12 (`PlayDash(96)`) | 10: `NS_dash_streak` attached at `pelvis`; 12, 36, 60, 84: `NS_dash_push` at `foot_l` on the ice, Variant 0; 24, 48, 72, 96: at `foot_r`, Variant 1 | 10, 16, 22, 28, 34, 46, 58, 70 | each push: ten small pieces thrown back and out from the pushing skate, under 0.2 % of the frame, gone 10 frames later, nothing above the knee. The streak: one pale line behind the hips 2.5 to 3.5 m long, 0.2 to 0.5 % of the frame, **0 % of the figure's pixels**. **The streak only spawns above 6 m/s of the figure's own travel: on a standing test figure it shows nothing; film it with the human skating (`jam.ScriptHuman 1`) or move the figure** |
| `skid` | 0: move 12; 40: move 14 (`Dash_Skid`) | 42: `NS_dash_skid` attached at the root (skid frame 2) | 42, 44, 46, 48, 52, 56, 62 | a pale fan ahead of the skates along the travel, **2 to 3 % of the frame at frame 50** (three bursts: 42, 46, 50), gone by frame 68, under 5 % of the figure covered |
| `wipeout` | 0: move 12; 40: move 16 (`Dash_Wipeout`) | 53: `NS_dash_wipeout` at the root (wipeout frame 13) | 50, 53, 55, 57, 59, 63, 73 | a low pale cloud ahead of the chest and at the hips, **about 2 % at frame 57**, nothing higher than 70 cm, gone by frame 78 |
| `dive` | 0: move 1 (`Dive_Block_Left`); 45: move 3 (the jolt) | 9: `NS_dive` attached at the root; 47: `NS_dive_hit` at `spine_03` | 5, 9, 11, 13, 15, 19, 29, 47, 49, 51 | a line of white streaks across the body's front and a pale cloud, **about 2 % at frame 13**, gone by frame 30. The trail only shows if the figure is moved (the take has no travel). The hit: a small cream star for 3 frames and a dozen pieces, under 0.3 % |
| `slide` | 0: move 4 (`Slide_Tackle_Left`) | 2: `NS_slide_tackle` attached at the root (its contact burst fires 8 frames later, on take frame 10) | 2, 5, 8, 10, 12, 14, 16, 20 | three small sprays ahead of the leading skate at frames 2, 5, 8 (under 0.3 % each), then **about 1 % at frame 12** |
| `mallet` | 0: move 6 (the mallet); 20: move 9 (`Mallet_Swing`) | 37: `NS_mallet_bonk` at the root moved 105 cm ahead, 45 cm left, 72 cm up (the drum's end face at contact frame 17) | 30, 37, 39, 41, 43, 47, 57 | a round cream puff that opens into a ring, a cream star for 3 frames, **1.5 to 2 % at frame 41**; the hip under it is seen again from frame 40; gone by frame 63. For the pair, summon the victim as `status/SK26-film.md` says |
| `cannon` | 0: move 7 (the cannon); 20: move 10 (`Cannon_Fire`) | 28: `NS_cannon_fire` at the root moved 79 cm ahead, 35 cm right, 89 cm up (the muzzle at frame 8) | 26, 28, 30, 32, 34, 38, 48 | a cone of pale chalk and eight white shards along the facing, **1.5 to 2 % at frame 32**, no light on anything, no yellow, orange or red in the effect, gone by frame 56 |
| `sweep` | 0: move 5 (the giant stick); 20: move 10 (`Giant_Sweep`) | 30: `NS_giant_sweep` at the root on the ice (sweep frame 10) | 28, 30, 32, 34, 36, 40, 50 | five small sprays 2 m ahead, one a frame, from the figure's right to its left, **about 1 % at frame 36**. If the blade runs the other way, pass Variant 1 |

`NS_pin_roll` has no test move (the rolling pin is not in the figure Blueprint yet). Spawn it alone on the standing figure: `unreal._b43film['fx']('NS_pin_roll', unreal._b43film['at']())` and capture the next 12 frames: a line of white streaks 1 m ahead across about 1 m and a cream dust, about 0.8 % of the frame.

Also check, from FILM1's leftovers (changed by numbers in B43, not seen): the shotgun's cloud is blue-grey (`fx('NS_ability', at(), 0.0)`), the shield's ring is still there 45 frames after the spawn (`fx('NS_ability', at(), 2.0)`), the goal's dust still stands at +100, and the cross-check's snow kick is streaks (`fx('NS_lift', at(), 0.0)`).

## The block
```python
# B43 filming block: run once per play session through execute_python (GPU editor). Fires the figure lane's test moves on the human's figure and spawns
# the B43 systems at the right bone on the right take frame until the figure lane wires them into the Blueprints.
import unreal, math
gw = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
pc = unreal.GameplayStatics.get_player_controller(gw, 0)
SHOTS = 'C:/Users/kazda/jam/thirdjam/devlog/b43'          # screenshots land here as <tag>-f<frame>.png; make the folder first, or point this at one that exists
def fig():
    for v in pc.get_controlled_pawn().get_attached_actors():
        if 'Visual' in v.get_class().get_name(): return v
def mesh(): return fig().get_components_by_class(unreal.SkeletalMeshComponent)[0]
def facing():
    f = pc.get_controlled_pawn().get_actor_forward_vector(); n = math.hypot(f.x, f.y) or 1.0; return unreal.Vector(f.x / n, f.y / n, 0)
def at(bone=None, fwd=0.0, left=0.0, up=0.0, on_ice=True):
    # a world point: a bone or socket of the figure (dropped to the ice unless on_ice is False), or the root, moved fwd / left / up cm in the figure's frame
    d = facing(); r = fig().get_actor_location(); b = mesh().get_socket_location(bone) if bone else unreal.Vector(r.x, r.y, 0)
    z = 0.0 if on_ice else b.z
    return unreal.Vector(b.x + fwd * d.x - left * d.y, b.y + fwd * d.y + left * d.x, z + up)
def fx(name, where, variant=0.0, strength=1.0, attach=False):
    s = unreal.load_asset('/JamAssets/VFX/' + name)
    if attach: c = unreal.NiagaraFunctionLibrary.spawn_system_attached(s, fig().root_component, 'None', where, unreal.Rotator(0, 0, 0), unreal.AttachLocation.KEEP_WORLD_POSITION, True)
    else: c = unreal.NiagaraFunctionLibrary.spawn_system_at_location(gw, s, where, unreal.Rotator(0, 0, 0), unreal.Vector(1, 1, 1), True, True, unreal.NCPoolMethod.NONE, False)
    if c is None: return None        # no renderer: Niagara does not spawn
    c.set_variable_vec3('User.Dir', facing()); c.set_variable_float('User.Strength', strength); c.set_variable_float('User.Variant', variant)
    return c
def cam(x, y, z, pitch, yaw, fov=70.0):
    # a fixed camera (world cm, degrees); gamecam() goes back to the game's own
    before = set(a.get_name() for a in unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.CameraActor))
    unreal.SystemLibrary.execute_console_command(gw, 'summon CameraActor', pc)
    c = [a for a in unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.CameraActor) if a.get_name() not in before][0]
    r = unreal.Rotator(); r.pitch = pitch; r.yaw = yaw
    c.set_actor_location_and_rotation(unreal.Vector(x, y, z), r, False, True)
    c.camera_component.set_field_of_view(fov); c.camera_component.set_editor_property('constrain_aspect_ratio', False)
    pc.set_view_target_with_blend(c, 0.0); return c
def closecam(dist=700.0, up=180.0):
    # beside the human's figure, at right angles to its facing, looking at it
    d = facing(); r = fig().get_actor_location(); yaw = math.degrees(math.atan2(d.y, d.x)) + 90.0
    return cam(r.x + d.y * dist + d.x * 80.0, r.y - d.x * dist + d.y * 80.0, up, -8.0, yaw, 55.0)
def gamecam(): pc.set_view_target_with_blend(pc.get_controlled_pawn(), 0.0)
def ke(move): unreal.SystemLibrary.execute_console_command(gw, 'ke * SK26Test %d' % move, pc)
# strip name -> (moves at frames, effects at frames, frames to capture). Effect entries: (frame, system, where-lambda, variant, attach)
STRIPS = {
 'dash':     ({0: 12}, [(12 + 24 * k, 'NS_dash_push', (lambda: at('foot_l')), 0.0, False) for k in range(4)] + [(24 + 24 * k, 'NS_dash_push', (lambda: at('foot_r')), 1.0, False) for k in range(4)]
              + [(10, 'NS_dash_streak', (lambda: at('pelvis', on_ice=False)), 0.0, True)], [10, 16, 22, 28, 34, 46, 58, 70]),
 'skid':     ({0: 12, 40: 14}, [(42, 'NS_dash_skid', (lambda: at()), 0.0, True)], [42, 44, 46, 48, 52, 56, 62]),
 'wipeout':  ({0: 12, 40: 16}, [(53, 'NS_dash_wipeout', (lambda: at()), 0.0, False)], [50, 53, 55, 57, 59, 63, 73]),
 'dive':     ({0: 1, 45: 3}, [(9, 'NS_dive', (lambda: at()), 0.0, True), (47, 'NS_dive_hit', (lambda: at('spine_03', on_ice=False)), 0.0, False)], [5, 9, 11, 13, 15, 19, 29, 47, 49, 51]),
 'slide':    ({0: 4}, [(2, 'NS_slide_tackle', (lambda: at()), 0.0, True)], [2, 5, 8, 10, 12, 14, 16, 20]),
 'mallet':   ({0: 6, 20: 9}, [(37, 'NS_mallet_bonk', (lambda: at(None, 105.0, 45.0, 72.0)), 0.0, False)], [30, 37, 39, 41, 43, 47, 57]),
 'cannon':   ({0: 7, 20: 10}, [(28, 'NS_cannon_fire', (lambda: at(None, 79.0, -35.0, 89.0)), 0.0, False)], [26, 28, 30, 32, 34, 38, 48]),
 'sweep':    ({0: 5, 20: 10}, [(30, 'NS_giant_sweep', (lambda: at()), 0.0, False)], [28, 30, 32, 34, 36, 40, 50]),
}
def run(tag, cam=None):
    moves, effects, shots = STRIPS[tag]
    s = {'n': 0, 'done': False, 'spawned': 0, 'none': 0, 'log': [], 'last': max(shots) + 3}
    def cb(dt):
        try:
            if s['done']: return
            n = s['n']
            if n in moves: ke(moves[n])
            for (f, name, where, variant, attach) in effects:
                if f == n:
                    c = fx(name, where(), variant, 1.0, attach)
                    s['spawned' if c else 'none'] += 1; s['log'].append('%d %s' % (n, name))
            if n in shots: unreal.SystemLibrary.execute_console_command(gw, 'HighResShot 1 filename="%s/%s-f%03d.png"' % (SHOTS, tag, n), pc)
            s['n'] = n + 1
            if s['n'] > s['last']: s['done'] = True; unreal.unregister_slate_post_tick_callback(s['h'])
        except Exception as ex: s['err'] = str(ex)[:300]; s['done'] = True; unreal.unregister_slate_post_tick_callback(s['h'])
    s['h'] = unreal.register_slate_post_tick_callback(cb); unreal._b43 = s
    return 'filming ' + tag
def state(): s = unreal._b43; return '%s n=%d spawned=%d none=%d %s %s' % (s['done'], s['n'], s['spawned'], s['none'], s.get('err'), s['log'][:6])
unreal._b43film = {'run': run, 'state': state, 'at': at, 'fx': fx, 'ke': ke, 'fig': fig, 'mesh': mesh, 'cam': cam, 'closecam': closecam, 'gamecam': gamecam}
'B43 film block ready: unreal._b43film["run"]("dash")'
```
