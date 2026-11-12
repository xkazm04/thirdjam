"""tools/p4/ue_pa_plan.py (lane A28, pass 3): a hand-made capsule plan for one figure, from its rest pose, printed as PhysicsAssetToolset calls.
Run: A28_PA = {"label": "Rodin2", "mesh": "/Game/P4/Rodin2/SK_P4_Rodin2", "alias": {}}; exec(open(r".../ue_pa_plan.py").read())
Bodies: pelvis, spine_02, head, upperarm_l/r, lowerarm_l/r, thigh_l/r, calf_l/r (capsules along the bone toward its child), hand_l/r (spheres: the gloves).
A capsule's long axis is its local Z: rotation = make_rot_from_z(child direction in bone space); centre = half way; radius = 0.22 x length (clamped 5-14 cm);
length = bone length - 2 x radius. Constraints: each body to its nearest body ancestor. Prints one line "A28 pa plan <json>" with the call list.
"""
import unreal, json
P = A28_PA; al = P.get("alias") or {}
bn = lambda n: al.get(n, n)
eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
lib = unreal.EditorAssetLibrary
a = None
for x in eas.get_all_level_actors():
    if x.get_actor_label() == "AB_" + P["label"]:
        a = x; break
if a is None:
    a = eas.spawn_actor_from_class(unreal.SkeletalMeshActor, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0)); a.set_actor_label("AB_" + P["label"])
    a.skeletal_mesh_component.set_skeletal_mesh_asset(lib.load_asset(P["mesh"]))
c = a.skeletal_mesh_component
BODIES = [("pelvis", "spine_01", None), ("spine_02", "spine_03", "pelvis"), ("head", None, "spine_02"),
          ("upperarm_l", "lowerarm_l", "spine_02"), ("lowerarm_l", "hand_l", "upperarm_l"), ("upperarm_r", "lowerarm_r", "spine_02"), ("lowerarm_r", "hand_r", "upperarm_r"),
          ("thigh_l", "calf_l", "pelvis"), ("calf_l", "foot_l", "thigh_l"), ("thigh_r", "calf_r", "pelvis"), ("calf_r", "foot_r", "thigh_r"),
          ("hand_l", None, "lowerarm_l"), ("hand_r", None, "lowerarm_r")]
b = c.get_skeletal_mesh_asset().get_bounds(); height = b.origin.z + b.box_extent.z
calls = []
for bone, child, parent in BODIES:
    bb = bn(bone)
    if not c.does_socket_exist(bb): continue
    t = c.get_socket_transform(bb, unreal.RelativeTransformSpace.RTS_WORLD)
    calls.append({"op": "AddBody", "bone": bb})
    if bone == "head":
        r = round(height * 0.11, 1)   # a toy head: about a fifth of the height across
        calls.append({"op": "SetSphere", "bone": bb, "name": "head", "center": [r * 0.6, 0, 0], "radius": r})
    elif bone in ("hand_l", "hand_r"):
        r = round(height * 0.055, 1)
        calls.append({"op": "SetSphere", "bone": bb, "name": "glove", "center": [r * 0.5, 0, 0], "radius": r})
    elif bone == "pelvis":
        r = round(height * 0.09, 1)
        calls.append({"op": "SetSphere", "bone": bb, "name": "pelvis", "center": [0, 0, 0], "radius": r})
    else:
        cc = bn(child)
        if not c.does_socket_exist(cc): continue
        p1 = c.get_socket_location(cc)
        loc = unreal.MathLibrary.inverse_transform_location(t, p1)
        length = loc.length()
        d = unreal.Vector(loc.x, loc.y, loc.z); d.normalize()
        rot = unreal.MathLibrary.make_rot_from_z(d)
        r = max(5.0, min(14.0, 0.22 * length))
        calls.append({"op": "SetCapsule", "bone": bb, "name": "cap", "center": [round(loc.x / 2, 1), round(loc.y / 2, 1), round(loc.z / 2, 1)],
                      "rotation": [round(rot.pitch, 1), round(rot.yaw, 1), round(rot.roll, 1)], "radius": round(r, 1), "length": round(max(0.0, length - 2 * r), 1)})
    if parent:
        calls.append({"op": "AddConstraint", "child": bb, "parent": bn(parent)})
print("A28 pa plan %s" % json.dumps({"label": P["label"], "height_cm": round(height, 1), "mass_kg": round(c.get_mass(), 1), "calls": calls}))
