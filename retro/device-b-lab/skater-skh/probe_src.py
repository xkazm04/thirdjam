import bpy, sys, math
from mathutils import Vector
f = sys.argv[-1]
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=f, automatic_bone_orientation=False)
arm = next(o for o in bpy.data.objects if o.type == "ARMATURE")
print("SRC objs", [(o.name, o.type, len(o.data.vertices) if o.type == "MESH" else "", [g.name for g in o.vertex_groups][:3] if o.type=="MESH" else "") for o in bpy.data.objects])
print("SRC arm scale", [round(x,4) for x in arm.matrix_world.to_scale()], "nacts", len(bpy.data.actions), sorted(a.name.split("|")[-1] for a in bpy.data.actions)[:4])
mw = arm.matrix_world
for n in ("pelvis", "thigh_l", "calf_l", "foot_l", "stick_socket", "hand_r", "puck_socket"):
    b = arm.data.bones[n]; M = mw @ b.matrix_local
    q = M.to_3x3().normalized()
    print("SRC", n, [round(x,4) for x in M.translation], "Y", [round(x,3) for x in q.col[1]], "Z", [round(x,3) for x in q.col[2]])
