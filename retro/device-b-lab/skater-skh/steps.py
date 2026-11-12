# Largest frame-to-frame jump of any bone head per take (cm), to catch pops. blender ... -- <fbx> [take...]
import bpy, sys, numpy as np
a = sys.argv[sys.argv.index("--") + 1:]
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=a[0], automatic_bone_orientation=False)
arm = next(o for o in bpy.data.objects if o.type == "ARMATURE"); sc = bpy.context.scene
names = [b.name for b in arm.data.bones]
for act in bpy.data.actions:
    cn = act.name.split("|")[-1]
    if len(a) > 1 and cn not in a[1:]: continue
    arm.animation_data_create(); arm.animation_data.action = act
    f0, f1 = [int(round(x)) for x in act.frame_range]
    P = []
    for f in range(f0, f1 + 1):
        sc.frame_set(f); mw = arm.matrix_world
        P.append(np.array([tuple(mw @ arm.pose.bones[n].head) for n in names]))
    P = np.array(P); d = np.linalg.norm(P[1:] - P[:-1], axis=2) * 100
    i, j = np.unravel_index(d.argmax(), d.shape)
    print("STEP", cn, "max %.1f cm at frame %d bone %s" % (d.max(), f0 + i, names[j]))
