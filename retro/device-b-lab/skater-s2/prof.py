import bpy, sys, math
for path in sys.argv[sys.argv.index("--")+1:]:
    bpy.ops.wm.read_factory_settings(use_empty=True); bpy.ops.import_scene.fbx(filepath=path)
    arm=[o for o in bpy.data.objects if o.type=='ARMATURE'][0]; sc=bpy.context.scene
    arm.animation_data_create()
    for act in bpy.data.actions:
        arm.animation_data.action=act; f0,f1=[int(round(x)) for x in act.frame_range]
        prev=None; rows=[]
        for f in range(f0,f1+1):
            sc.frame_set(f)
            q={pb.name:pb.matrix_basis.to_quaternion() for pb in arm.pose.bones}
            sp=0
            if prev: sp=sum(math.degrees(2*math.acos(min(1,abs(q[n].dot(prev[n]))))) for n in q)
            prev=q
            pz=(arm.matrix_world@arm.pose.bones['pelvis'].head).z
            hz=(arm.matrix_world@arm.pose.bones['head'].head).z
            rows.append((f,sp,pz,hz))
        print("PROF",act.name,f0,f1)
        print(" ".join("%d:%.0f/%.0f/%.0f"%(f,s,pz*100,hz*100) for f,s,pz,hz in rows[::5]))
