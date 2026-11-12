"""tools/p4/ue_lab.py (lane A28): the P4 lab level: one vendor figure, Manny beside it for scale, a chase SceneCapture2D, fixed 60 Hz.
Run in the editor's Python: A28_LAB = {"vendor": "Meshy", "mesh": "/Game/P4/Meshy/SK_P4_Meshy"}; exec(open(r".../ue_lab.py").read())
Prints "A28 <vendor> lab <key> <value>" lines and a JSON summary; sizes are measured on registered components (socket locations are valid here).
"""
import unreal, json
P = A28_LAB
V = P["vendor"]
lib = unreal.EditorAssetLibrary
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
out = {"tool": "thirdjam.p4-lab/1", "vendor": V}
def say(k, v):
    print("A28 %s lab %s %s" % (V, k, v)); out[k] = v

LEVEL = "/Game/P4/Maps/P4_Lab"
cur = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
cur_path = cur.get_path_name().split(".")[0] if cur else ""
if cur_path == LEVEL:
    say("level_already_open", les.save_current_level())
elif not lib.does_asset_exist(LEVEL):
    say("new_level", les.new_level_from_template(LEVEL, "/Engine/Maps/Templates/Template_Default"))
else:
    les.save_current_level()
    say("load_level", les.load_level(LEVEL))

# fixed 60 Hz so a background editor ticks animation and physics by frames (tools/mcp/PHYSICS.md)
eng = unreal.find_object(None, "/Engine/Transient.UnrealEdEngine_0")
if eng:
    eng.set_editor_property("bUseFixedFrameRate", True); eng.set_editor_property("FixedFrameRate", 60.0)
    say("fixed_frame_rate", 60)

def find(label):
    for a in eas.get_all_level_actors():
        if a.get_actor_label() == label:
            return a
    return None
def spawn_mesh(label, mesh_path, loc):
    a = find(label)
    if a: eas.destroy_actor(a)
    a = eas.spawn_actor_from_class(unreal.SkeletalMeshActor, loc, unreal.Rotator(0, 0, 0))
    a.set_actor_label(label)
    c = a.skeletal_mesh_component
    c.set_skeletal_mesh_asset(lib.load_asset(mesh_path))
    c.set_editor_property("visibility_based_anim_tick_option", unreal.VisibilityBasedAnimTickOption.ALWAYS_TICK_POSE_AND_REFRESH_BONES)
    return a
fig = spawn_mesh("P4_" + V, P["mesh"], unreal.Vector(0, 0, 0))
man = spawn_mesh("P4_Manny", "/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple", unreal.Vector(0, 200, 0))

def measure(c):
    def loc(n): return c.get_socket_location(n) if c.does_socket_exist(n) else None
    def dist(a, b): return None if (a is None or b is None) else round((a - b).length(), 1)
    pel, head, hl, hr = loc("pelvis"), loc("head"), loc("hand_l"), loc("hand_r")
    he = None
    for n in ("RightHand_End", "middle_01_r", "index_01_r"):
        if c.does_socket_exist(n): he = loc(n); break
    fl, fr = loc("foot_l"), loc("foot_r")
    base = c.get_owner().get_actor_location()
    b = c.get_skeletal_mesh_asset().get_bounds()
    return {"pelvis_z_cm": round(pel.z - base.z, 1) if pel else None, "head_z_cm": round(head.z - base.z, 1) if head else None,
            "hand_span_cm": dist(hl, hr), "hand_len_cm": dist(hr, he), "foot_span_cm": dist(fl, fr),
            "height_cm": round(b.origin.z + b.box_extent.z, 1), "width_cm": round(2 * b.box_extent.y, 1), "depth_cm": round(2 * b.box_extent.x, 1)}
if P.get("stick"):
    st = find("P4_Stick_" + V)
    if st: eas.destroy_actor(st)
    st = eas.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))
    st.set_actor_label("P4_Stick_" + V)
    st.static_mesh_component.set_static_mesh(lib.load_asset(P["stick"]))
    st.static_mesh_component.set_mobility(unreal.ComponentMobility.MOVABLE)
    ok = st.attach_to_component(fig.skeletal_mesh_component, "stick_socket", unreal.AttachmentRule.SNAP_TO_TARGET, unreal.AttachmentRule.SNAP_TO_TARGET, unreal.AttachmentRule.KEEP_WORLD, False)
    r = P.get("stick_rot", [0, 0, 0]); st.static_mesh_component.set_relative_rotation(unreal.Rotator(r[0], r[1], r[2]), False, False)
    say("stick_attached", [ok, fig.skeletal_mesh_component.does_socket_exist("stick_socket")])
mv, mm = measure(fig.skeletal_mesh_component), measure(man.skeletal_mesh_component)
say("size_vendor", mv); say("size_manny", mm)
say("size_ratio", {k: (round(mv[k] / mm[k], 2) if mv.get(k) and mm.get(k) else None) for k in mv})

# chase camera: 5 m front-left three-quarter, 1.3 m up, looking at the chest; a second far camera at 24 m
def cam(label, loc, target):
    a = find(label)
    if a: eas.destroy_actor(a)
    a = eas.spawn_actor_from_class(unreal.SceneCapture2D, loc, unreal.Rotator(0, 0, 0))
    a.set_actor_label(label)
    rot = unreal.MathLibrary.find_look_at_rotation(loc, target)
    a.set_actor_rotation(rot, False)
    cc = a.capture_component2d
    cc.set_editor_property("fov_angle", 40.0)
    cc.set_editor_property("capture_source", unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR)
    cc.set_editor_property("capture_every_frame", False)
    cc.set_editor_property("capture_on_movement", False)
    return a
cam("P4_Cam_Chase", unreal.Vector(-420, -300, 130), unreal.Vector(0, 0, 95))
cam("P4_Cam_Far", unreal.Vector(-2200, -1000, 700), unreal.Vector(0, 100, 90))
say("saved", les.save_current_level())
print(json.dumps(out))
