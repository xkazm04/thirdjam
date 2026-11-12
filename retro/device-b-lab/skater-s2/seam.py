# usage: blender -b --factory-startup --python seam.py -- <fbx> [<fbx>...]
# per action: frames, fps, duration, pelvis travel (cm), loop seam (first vs last frame) as max/mean joint
# rotation difference in degrees (local bone rotation, rest-relative), max foot/hand position diff (cm, pelvis-relative),
# and best trim: the frame j in the last 40 % whose pose is closest to frame 0, with its max joint diff.
import bpy, sys, os, json, math
args = sys.argv[sys.argv.index("--")+1:]
def quat_ang(a, b):
    d = abs(a.dot(b)); d = min(1.0, d)
    return math.degrees(2*math.acos(d))
res = []
for path in args:
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=path)
    arm = [o for o in bpy.data.objects if o.type == 'ARMATURE'][0]
    sc = bpy.context.scene
    arm.animation_data_create()
    for act in list(bpy.data.actions):
        arm.animation_data.action = act
        f0, f1 = [int(round(x)) for x in act.frame_range]
        # pose sampler
        def pose(f):
            sc.frame_set(f)
            rot = {pb.name: pb.rotation_quaternion.copy() if pb.rotation_mode == 'QUATERNION' else pb.rotation_euler.to_quaternion() for pb in arm.pose.bones}
            loc = {n: (arm.matrix_world @ arm.pose.bones[n].head).copy() for n in arm.pose.bones.keys()}
            return rot, loc
        r0, l0 = pose(f0)
        rl, ll = pose(f1)
        def diff(ra, la, rb, lb):
            angs = {n: quat_ang(ra[n], rb[n]) for n in ra if n != 'pelvis'}
            mx = max(angs, key=angs.get)
            # pelvis-relative positions of feet/hands, horizontal pelvis offset removed
            pa, pb_ = la['pelvis'], lb['pelvis']
            ends = ['foot_l','foot_r','hand_l','hand_r','head']
            pos = max(((la[n]-pa)-(lb[n]-pb_)).length for n in ends)*100
            return angs[mx], mx, sum(angs.values())/len(angs), pos
        a, n, mean, pos = diff(r0, l0, rl, ll)
        # pelvis travel
        p0, p1 = l0['pelvis'], ll['pelvis']
        zs = []
        for f in range(f0, f1+1, max(1,(f1-f0)//40)):
            sc.frame_set(f); zs.append((arm.matrix_world @ arm.pose.bones['pelvis'].head).z)
        # best trim in last 40%
        best = None
        for j in range(f1 - int((f1-f0)*0.4), f1+1):
            rj, lj = pose(j)
            d = diff(r0, l0, rj, lj)
            if best is None or d[0] < best[1][0]: best = (j, d)
        res.append(dict(file=os.path.basename(os.path.dirname(path))+"/"+os.path.basename(path), action=act.name, f0=f0, f1=f1,
            frames=f1-f0+1, fps=sc.render.fps, dur_s=round((f1-f0+1)/sc.render.fps,2),
            pelvis_dx_cm=round((p1.x-p0.x)*100,1), pelvis_dy_cm=round((p1.y-p0.y)*100,1), pelvis_dz_cm=round((p1.z-p0.z)*100,1),
            pelvis_travel_cm=round(math.hypot(p1.x-p0.x,p1.y-p0.y)*100,1), pelvis_z_range_cm=round((max(zs)-min(zs))*100,1),
            seam_max_deg=round(a,1), seam_bone=n, seam_mean_deg=round(mean,1), seam_pos_cm=round(pos,1),
            best_trim_frame=best[0], best_trim_max_deg=round(best[1][0],1), best_trim_bone=best[1][1], best_trim_pos_cm=round(best[1][3],1)))
print("SEAMJSON" + json.dumps(res))
