"""tools/p4/ue_grip.py (lane A28): the stick grip rotation at stick_socket, three numbers per skeleton, computed from the hand bone.
Run in the editor's Python: A28_GRIP = {"vendor": "Meshy", "mesh": "/Game/P4/Meshy/SK_P4_Meshy"}; exec(open(r".../ue_grip.py").read())
SM_P4_Stick v2 runs along its local +X from the grip. The figure faces +Y in the lab (right hand on -X). The shaft should leave the right hand toward
the ice ahead and slightly across the body: world direction d = normalize(0.35, 0.80, -0.70) in the figure's space; the socket's relative rotation is
inverse(hand_r world rotation) * make_rot_from_xz(d, up). Prints "A28 <vendor> grip <key> <value>" and writes the socket on the mesh asset.
"""
import unreal, json
_ALIAS = globals().get("A28_ALIAS") or {}
def bn(n):
    return _ALIAS.get(n, n)

P = A28_GRIP; V = P["vendor"]
_ALIAS = P.get("alias") or {}
lib = unreal.EditorAssetLibrary
eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
out = {"tool": "thirdjam.p4-grip/1", "vendor": V}
def say(k, v):
    print("A28 %s grip %s %s" % (V, k, v)); out[k] = v
mesh = lib.load_asset(P["mesh"])
# a registered component to read the hand's rest-pose world rotation
a = eas.spawn_actor_from_class(unreal.SkeletalMeshActor, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))
c = a.skeletal_mesh_component; c.set_skeletal_mesh_asset(mesh)
hand_rot = c.get_socket_rotation(bn("hand_r"))
hand_loc = c.get_socket_location(bn("hand_r"))
d = unreal.Vector(0.35, 0.80, -0.70); d.normalize()
want = unreal.MathLibrary.make_rot_from_xz(d, unreal.Vector(0, 0, 1))
rel = unreal.MathLibrary.normalized_delta_rotator(want, hand_rot) if False else unreal.MathLibrary.compose_rotators(want, unreal.MathLibrary.negate_rotator(hand_rot))
# compose_rotators(A, B) = A then B in UE's convention (B * A); we need rel such that hand_rot * rel = want  =>  rel = inverse(hand_rot) * want
q_hand = hand_rot.quaternion(); q_want = want.quaternion()
q_rel = q_hand.inverse() * q_want if hasattr(q_hand, "inverse") else None
if q_rel is not None:
    rel = q_rel.rotator()
say("hand_r_world_rot", [round(hand_rot.pitch, 1), round(hand_rot.yaw, 1), round(hand_rot.roll, 1)])
say("want_world_rot", [round(want.pitch, 1), round(want.yaw, 1), round(want.roll, 1)])
say("socket_relative_rot", [round(rel.pitch, 1), round(rel.yaw, 1), round(rel.roll, 1)])
sock = mesh.find_socket("stick_socket")
if sock:
    sock.set_editor_property("relative_rotation", rel)
    sock.set_editor_property("relative_location", unreal.Vector(0, 0, 0))
    say("socket_set", True)
    lib.save_asset(P["mesh"], only_if_is_dirty=False)
else:
    say("socket_set", "stick_socket missing")
# check: the socket's world X axis after the change should point along d
c.set_skeletal_mesh_asset(mesh)
sr = c.get_socket_rotation("stick_socket")
x = unreal.MathLibrary.get_forward_vector(sr)
say("stick_x_world", [round(x.x, 2), round(x.y, 2), round(x.z, 2)]); say("wanted", [round(d.x, 2), round(d.y, 2), round(d.z, 2)])
say("blade_end_z_cm", round(hand_loc.z + 130 * x.z, 1))
eas.destroy_actor(a)
print(json.dumps(out))
