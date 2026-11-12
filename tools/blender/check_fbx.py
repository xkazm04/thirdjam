"""tools/blender/check_fbx.py: re-import an exported skater FBX into a clean Blender scene and measure, per action and frame, how far the forearm
tip is from the hand on the stick (lane A22, third pass: the mittens must stay on the shaft in every stick verb).

  blender -b -P tools/blender/check_fbx.py -- tools/blender/out/SK_SkaterToy.fbx

Prints one "A22 check" line per action with the max gap (cm) between the tail of arm_*_fore and the head of ik_hand_* over the action's frames, and
a PASS/FAIL line (gap under 2 cm everywhere = the arms reach the mittens with plain keys, no constraint needed).
"""
import bpy
import sys

path = sys.argv[sys.argv.index("--") + 1]
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=path, use_anim=True, ignore_leaf_bones=False, automatic_bone_orientation=False)
arm = next(o for o in bpy.data.objects if o.type == "ARMATURE")
if arm.animation_data is None:
    arm.animation_data_create()
print("A22 check bones", " ".join(b.name for b in arm.data.bones))
worst = 0.0
stretch = {}
pairs = (("arm_l_fore", "ik_hand_l"), ("arm_r_fore", "ik_hand_r"))
for act in sorted(bpy.data.actions, key=lambda a: a.name):
    arm.animation_data.action = act
    f0, f1 = int(act.frame_range[0]), int(act.frame_range[1])
    gap = 0.0
    for f in range(f0, f1 + 1):
        bpy.context.scene.frame_set(f)
        bpy.context.view_layer.update()
        for bn in ("arm_l_upper", "arm_l_fore", "arm_r_upper", "arm_r_fore"):
            sc = arm.pose.bones[bn].scale.y
            if sc > stretch.get(act.name, 1.0):
                stretch[act.name] = sc
        for fore, hand in pairs:
            tail = (arm.matrix_world @ arm.pose.bones[fore].tail)
            head = (arm.matrix_world @ arm.pose.bones[hand].head)
            gap = max(gap, (tail - head).length)
    worst = max(worst, gap)
    print("A22 check %-40s frames %3d-%3d max gap %.2f cm  max arm scale %.2f" % (act.name, f0, f1, gap, stretch.get(act.name, 1.0)))
mesh = next(o for o in bpy.data.objects if o.type == "MESH")
slots = [m.name if m else "?" for m in mesh.data.materials]
print("A22 check slots", " ".join(slots))
if "number" in slots:
    idx = slots.index("number")
    uv = mesh.data.uv_layers.active.data
    polys = [pg for pg in mesh.data.polygons if pg.material_index == idx]
    us, vs = [], []
    for pg in polys:
        for li in pg.loop_indices:
            us.append(uv[li].uv.x)
            vs.append(uv[li].uv.y)
    print("A22 check number slot: %d polygons, %d loops, u %.3f..%.3f, v %.3f..%.3f, verts %s" % (
        len(polys), len(us), min(us), max(us), min(vs), max(vs), sorted(set(v for pg in polys for v in pg.vertices))))
    ok = len(polys) <= 2 and abs(min(us)) < 1e-4 and abs(max(us) - 1) < 1e-4 and abs(min(vs)) < 1e-4 and abs(max(vs) - 1) < 1e-4
    print("A22 check number slot %s (one square, UVs exactly 0..1)" % ("PASS" if ok else "FAIL"))
print("A22 check %s worst gap %.2f cm over %d actions, %d bones" % ("PASS" if worst < 2.0 else "FAIL", worst, len(bpy.data.actions), len(arm.data.bones)))
