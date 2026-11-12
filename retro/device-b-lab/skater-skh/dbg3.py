import bpy, sys
f, act = sys.argv[-2], sys.argv[-1]
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=f, automatic_bone_orientation=False)
arm = next(o for o in bpy.data.objects if o.type == "ARMATURE"); sc = bpy.context.scene
a = [x for x in bpy.data.actions if x.name.split("|")[-1] == act][0]; arm.animation_data_create(); arm.animation_data.action = a
for fr in (1, 2, 3, 59, 60, 61):
    sc.frame_set(fr); mw = arm.matrix_world
    print("D3", fr, *["%s %s" % (n, [round(x * 100, 1) for x in (mw @ arm.pose.bones[n].head)]) for n in ("pelvis", "hand_r", "stick_socket", "hand_l")])
