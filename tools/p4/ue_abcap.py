# tools/p4/ue_abcap.py (lane A28, pass 3): under Simulate, reveal one AB_<label> figure at a time, play its clip to the same game frame, grab the chase
# (4.2 m) and far (24 m) cameras, hide it again. Placeholders: __FIGURES__ (JSON list of {label, clip, ragdoll, alias}), __OUT__, __FRAME__ (game frames
# after the clip starts; the frozen frame), __TAG__. The ragdoll block (20 frames, mass-scaled shove on spine_02, then the sequence again) runs for the
# figures with ragdoll true, sampled every 4 frames, pelvis height recorded. Prints "A28 ab cap <key> <value>".
import unreal, asyncio, json, os
FIGS, OUT, FRAME, TAG = __FIGURES__, "__OUT__", __FRAME__, "__TAG__"
os.makedirs(OUT, exist_ok=True)
rep = {"tool": "thirdjam.p4-abcap/1", "tag": TAG, "frame": FRAME, "figures": {}}
def say(k, v):
    print("A28 ab cap %s %s" % (k, v)); rep[k] = v
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
if not world:
    raise RuntimeError("no game world: start Simulate first")
def by_label(cls, label):
    for a in unreal.GameplayStatics.get_all_actors_of_class(world, cls):
        if a.get_actor_label() == label:
            return a
    return None
cams = {"chase": by_label(unreal.SceneCapture2D, "P4_Cam_Chase"), "far": by_label(unreal.SceneCapture2D, "P4_Cam_Far")}
rts = {k: unreal.RenderingLibrary.create_render_target2d(world, 960, 720, unreal.TextureRenderTargetFormat.RTF_RGBA8) for k in cams}
for k, c in cams.items():
    c.capture_component2d.set_editor_property("texture_target", rts[k])
t0 = unreal.GameplayStatics.get_time_seconds(world)
frame_of = lambda: int(round((unreal.GameplayStatics.get_time_seconds(world) - t0) * 60.0))
async def until(frame):
    while frame_of() < frame:
        await asyncio.sleep(0.004)
def grab(name):
    for k, c in cams.items():
        c.capture_component2d.capture_scene()
        unreal.RenderingLibrary.export_render_target(world, rts[k], OUT, "%s_%s_%s.png" % (TAG, name, k))
for f in FIGS:
    a = by_label(unreal.SkeletalMeshActor, "AB_" + f["label"])
    if not a:
        say("missing_actor", f["label"]); continue
    al = f.get("alias") or {}
    bn = lambda n: al.get(n, n)
    comp = a.skeletal_mesh_component
    a.set_actor_hidden_in_game(False)
    seq = unreal.load_asset(f["clip"]) if f.get("clip") else None
    if seq:
        comp.set_animation_mode(unreal.AnimationMode.ANIMATION_SINGLE_NODE); comp.set_animation(seq); comp.set_play_rate(1.0); comp.play(True)
    fstart = frame_of()
    await until(fstart + FRAME)
    grab(f["label"] + "_f%02d" % FRAME)
    pel = comp.get_socket_location(bn("pelvis"))
    rec = {"clip": f.get("clip"), "pelvis_z_at_frame": round(pel.z, 1), "clip_len_s": round(seq.get_play_length(), 3) if seq else None, "mass_kg": round(comp.get_mass(), 1)}
    if f.get("ragdoll"):
        r0 = frame_of()
        comp.set_collision_profile_name("Ragdoll"); comp.set_all_bodies_physics_blend_weight(1.0); comp.set_simulate_physics(True)
        mass = comp.get_mass(); comp.add_impulse(unreal.Vector(-150 * mass, 0, 0), bn("spine_02"), False)
        rag = []
        for j in range(6):
            await until(r0 + 4 * j)
            grab(f["label"] + "_rag%02d" % j)
            rag.append(round(comp.get_socket_location(bn("pelvis")).z, 1))
        await until(r0 + 20)
        comp.set_simulate_physics(False); comp.set_all_bodies_physics_blend_weight(0.0)
        comp.set_relative_location_and_rotation(unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0), False, False)
        if seq:
            comp.set_animation_mode(unreal.AnimationMode.ANIMATION_SINGLE_NODE); comp.set_animation(seq); comp.play(True)
        await until(r0 + 28)
        grab(f["label"] + "_back00")
        rec["ragdoll_pelvis_z"] = rag; rec["back_pelvis_z"] = round(comp.get_socket_location(bn("pelvis")).z, 1)
        rec["ragdoll_max_pelvis_z"] = max(rag)
    rep["figures"][f["label"]] = rec
    say(f["label"], rec)
    a.set_actor_hidden_in_game(True)
    await until(frame_of() + 2)
open(os.path.join(OUT, TAG + ".abcap.json"), "w").write(json.dumps(rep, indent=1))
print(json.dumps(rep))
