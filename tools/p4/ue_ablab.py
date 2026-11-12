"""tools/p4/ue_ablab.py (lane A28, pass 3): the A/B lab: several skeletal meshes at the origin of /Game/P4/Maps/P4_Lab, all hidden in game except
when the capture script reveals one; the two SceneCapture2D cameras (chase 4.2 m, far 24 m) from ue_lab.py are reused. Manny and the P4_* figure actors
of earlier passes are removed from the level first. Also measures, per figure, the bone lengths the capsule plan needs.
Run: A28_AB = {"figures": [{"label": "Rodin2", "mesh": "/Game/P4/Rodin2/SK_P4_Rodin2", "alias": {}}, ...]}; exec(open(r".../ue_ablab.py").read())
Prints "A28 ab lab <key> <value>" lines; the bone table as one JSON line "A28 ab bones <label> {...}".
"""
import unreal, json
P = A28_AB
lib = unreal.EditorAssetLibrary
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
out = {"tool": "thirdjam.p4-ablab/1"}
def say(k, v):
    print("A28 ab lab %s %s" % (k, v)); out[k] = v
LEVEL = "/Game/P4/Maps/P4_Lab"
cur = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
if (cur.get_path_name().split(".")[0] if cur else "") != LEVEL:
    say("load_level", les.load_level(LEVEL))
eng = unreal.find_object(None, "/Engine/Transient.UnrealEdEngine_0")
if eng:
    eng.set_editor_property("bUseFixedFrameRate", True); eng.set_editor_property("FixedFrameRate", 60.0)
# clear earlier figure actors (P4_<Vendor>, P4_Manny, P4_Stick_*), keep the cameras
for a in list(eas.get_all_level_actors()):
    lab = a.get_actor_label()
    if lab.startswith("P4_") and not lab.startswith("P4_Cam"):
        eas.destroy_actor(a)
cams = [a.get_actor_label() for a in eas.get_all_level_actors() if a.get_actor_label().startswith("P4_Cam")]
say("cameras", cams)
def bn(alias, n): return alias.get(n, n)
CHAIN = [("pelvis", "spine_01"), ("spine_02", "spine_03"), ("head", None), ("upperarm_l", "lowerarm_l"), ("upperarm_r", "lowerarm_r"), ("lowerarm_l", "hand_l"), ("lowerarm_r", "hand_r"),
         ("thigh_l", "calf_l"), ("thigh_r", "calf_r"), ("calf_l", "foot_l"), ("calf_r", "foot_r"), ("hand_l", None), ("hand_r", None)]
for f in P["figures"]:
    a = eas.spawn_actor_from_class(unreal.SkeletalMeshActor, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))
    a.set_actor_label("AB_" + f["label"])
    c = a.skeletal_mesh_component
    m = lib.load_asset(f["mesh"])
    if not m:
        say("missing_mesh", f["mesh"]); eas.destroy_actor(a); continue
    c.set_skeletal_mesh_asset(m)
    c.set_editor_property("visibility_based_anim_tick_option", unreal.VisibilityBasedAnimTickOption.ALWAYS_TICK_POSE_AND_REFRESH_BONES)
    a.set_actor_hidden_in_game(True)
    if f.get("yaw"):
        a.set_actor_rotation(unreal.Rotator(0, f["yaw"], 0), False)
    al = f.get("alias") or {}
    b = m.get_bounds()
    bones = {}
    for bone, child in CHAIN:
        bb, cc = bn(al, bone), bn(al, child) if child else None
        if not c.does_socket_exist(bb): continue
        p0 = c.get_socket_location(bb)
        rec = {"z": round(p0.z, 1)}
        if cc and c.does_socket_exist(cc):
            p1 = c.get_socket_location(cc); rec["len"] = round((p1 - p0).length(), 1)
            # the child's position in the bone's local frame tells which local axis runs down the bone
            t = c.get_socket_transform(bb, unreal.RelativeTransformSpace.RTS_WORLD)
            loc = t.inverse_transform_location(p1) if hasattr(t, "inverse_transform_location") else unreal.MathLibrary.inverse_transform_location(t, p1)
            rec["child_local"] = [round(loc.x, 1), round(loc.y, 1), round(loc.z, 1)]
        bones[bone] = rec
    print("A28 ab bones %s %s" % (f["label"], json.dumps({"height_cm": round(b.origin.z + b.box_extent.z, 1), "bones": bones, "mass_kg": round(c.get_mass(), 1) if hasattr(c, "get_mass") else None})))
    say("spawned", f["label"])
say("saved", les.save_current_level())
print(json.dumps(out))
