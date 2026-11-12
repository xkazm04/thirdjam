# tools/p4/ue_capture.py (lane A28): run under Simulate (StartPIE bSimulate) through MCP execute_python as a top-level-await script.
# The MCP runner substitutes the placeholders: __VENDOR__, __CLIP__ (asset path), __OUT__ (folder), __TAG__, __RAGDOLL__ (0/1).
# Plays the clip on the P4_<vendor> actor, exports one chase and one far frame every 8 frames for 2 s (16 frames of each), then,
# with __RAGDOLL__, switches the mesh to physics for 20 frames and back to the sequence, exporting a frame every 4 frames meanwhile.
# Frames are counted by game time at the fixed 60 Hz (never wall clock). Prints "A28 <vendor> cap <key> <value>".
import unreal, asyncio, json, os
V, CLIP, OUT, TAG, RAG = "__VENDOR__", "__CLIP__", "__OUT__", "__TAG__", __RAGDOLL__
os.makedirs(OUT, exist_ok=True)
rep = {"tool": "thirdjam.p4-capture/1", "vendor": V, "clip": CLIP, "tag": TAG, "frames": []}
def say(k, v):
    print("A28 %s cap %s %s" % (V, k, v)); rep[k] = v
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
if not world:
    raise RuntimeError("no game world: start Simulate first")
def by_label(cls, label):
    for a in unreal.GameplayStatics.get_all_actors_of_class(world, cls):
        if a.get_actor_label() == label:
            return a
    return None
fig = by_label(unreal.SkeletalMeshActor, "P4_" + V)
cams = {"chase": by_label(unreal.SceneCapture2D, "P4_Cam_Chase"), "far": by_label(unreal.SceneCapture2D, "P4_Cam_Far")}
rts = {k: unreal.RenderingLibrary.create_render_target2d(world, 960, 720, unreal.TextureRenderTargetFormat.RTF_RGBA8) for k in cams}
for k, c in cams.items():
    c.capture_component2d.set_editor_property("texture_target", rts[k])
comp = fig.skeletal_mesh_component
seq = unreal.load_asset(CLIP) or unreal.EditorAssetLibrary.load_asset(CLIP)
if not seq:
    say("clip_missing", [CLIP, unreal.EditorAssetLibrary.does_asset_exist(CLIP), [str(a.asset_name) for a in unreal.AssetRegistryHelpers.get_asset_registry().get_assets_by_path(CLIP.rsplit("/", 1)[0])]])
    raise RuntimeError("clip not found " + CLIP)
comp.set_animation_mode(unreal.AnimationMode.ANIMATION_SINGLE_NODE)
comp.set_animation(seq)
comp.set_play_rate(1.0)
comp.play(True)
say("clip_len_s", round(seq.get_play_length(), 3))
t0 = unreal.GameplayStatics.get_time_seconds(world)
frame_of = lambda: int(round((unreal.GameplayStatics.get_time_seconds(world) - t0) * 60.0))
async def until(frame):
    while frame_of() < frame:
        await asyncio.sleep(0.004)
def grab(name):
    for k, c in cams.items():
        c.capture_component2d.capture_scene()
        unreal.RenderingLibrary.export_render_target(world, rts[k], OUT, "%s_%s_%s.png" % (TAG, name, k))
    pos = comp.get_position()
    pel = comp.get_socket_location("pelvis"); fr = comp.get_socket_location("foot_r")
    rep["frames"].append({"name": name, "game_frame": frame_of(), "anim_pos_s": round(pos, 3), "pelvis_z": round(pel.z, 1), "foot_r_z": round(fr.z, 1)})
for i in range(16):
    await until(i * 8)
    grab("f%02d" % i)
say("clip_frames", len(rep["frames"]))
if RAG:
    pa = comp.get_skeletal_mesh_asset().get_editor_property("physics_asset")
    say("physics_asset", pa.get_path_name() if pa else None)
    f0 = frame_of()
    comp.set_collision_profile_name("Ragdoll")
    comp.set_all_bodies_physics_blend_weight(1.0)
    comp.set_simulate_physics(True)
    comp.add_impulse(unreal.Vector(-600, 0, 100), "spine_02", True)
    for j in range(6):
        await until(f0 + 4 * j)
        grab("rag%02d" % j)
    await until(f0 + 20)
    comp.set_simulate_physics(False)
    comp.set_all_bodies_physics_blend_weight(0.0)
    comp.set_relative_location_and_rotation(unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0), False, False)  # the pawn owns the transform: the ragdoll never moves it
    comp.set_animation_mode(unreal.AnimationMode.ANIMATION_SINGLE_NODE)
    comp.set_animation(seq)
    comp.play(True)
    for j in range(4):
        await until(f0 + 20 + 8 * j)
        grab("back%02d" % j)
    say("ragdoll_frames", 20)
say("last_game_frame", frame_of())
open(os.path.join(OUT, TAG + ".capture.json"), "w").write(json.dumps(rep, indent=1))
print(json.dumps(rep))
