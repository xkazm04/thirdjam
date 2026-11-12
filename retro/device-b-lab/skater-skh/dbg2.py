import bpy, sys, numpy as np
f, act = sys.argv[-2], sys.argv[-1]
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=f, automatic_bone_orientation=False)
arm = next(o for o in bpy.data.objects if o.type == "ARMATURE")
ms = [o for o in bpy.data.objects if o.type == "MESH"]; body = max(ms, key=lambda o: len(o.data.vertices))
gn = {g.index: g.name for g in body.vertex_groups}
dom = [gn[max(v.groups, key=lambda g: g.weight).group] if v.groups else None for v in body.data.vertices]
a = [x for x in bpy.data.actions if x.name.split("|")[-1] == act][0]; arm.animation_data_create(); arm.animation_data.action = a
dg = bpy.context.evaluated_depsgraph_get(); sc = bpy.context.scene
f0, f1 = [int(x) for x in a.frame_range]
for fr in range(f0, f1 + 1, 3):
    sc.frame_set(fr); ev = body.evaluated_get(dg); me = ev.to_mesh()
    c = np.array([tuple(v.co) for v in me.vertices]); ev.to_mesh_clear(); c = c @ np.array(body.matrix_world)[:3,:3].T
    i = int(c[:, 2].argmin())
    fl = [j for j in range(len(dom)) if dom[j] in ("foot_l", "ball_l")]; fr_ = [j for j in range(len(dom)) if dom[j] in ("foot_r", "ball_r")]
    print("DBG", fr, "lowest", round(c[i,2]*100,1), dom[i], "footL", round(c[fl,2].min()*100,1), "footR", round(c[fr_,2].min()*100,1))
