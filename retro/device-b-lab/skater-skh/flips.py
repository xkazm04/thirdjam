# Frame-to-frame local rotation jumps over 60 deg per bone in every take of a clip FBX (twist flips that a hinge hides)
import bpy, sys, math
from mathutils import Quaternion
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=sys.argv[-1], automatic_bone_orientation=False)
out = []
for a in bpy.data.actions:
    cn = a.name.split("|")[-1]; Q = {}
    for fc in a.fcurves:
        if fc.data_path.endswith("rotation_quaternion"):
            b = fc.data_path.split('"')[1]; Q.setdefault(b, [None] * 4)[fc.array_index] = [k.co[1] for k in fc.keyframe_points]
    for b, comps in Q.items():
        if None in comps: continue
        qs = [Quaternion((comps[0][i], comps[1][i], comps[2][i], comps[3][i])) for i in range(len(comps[0]))]
        for i in range(1, len(qs)):
            ang = math.degrees(qs[i - 1].rotation_difference(qs[i]).angle)
            if ang > 60:
                out.append((cn, b, i, round(ang)))
print("FLIPS", len(out), out[:20])
