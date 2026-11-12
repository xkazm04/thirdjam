# make_engine_clip.py: turn a Meshy animation FBX into an engine-ready clip.
#
#   blender -b --factory-startup --python make_engine_clip.py -- --in A.fbx --out Stride.fbx [options]
#
#   --in FILE          source FBX (Meshy animate output: one armature, one mesh, one or more actions)
#   --out FILE         output FBX. Armature only by default (import into Unreal with the mesh off).
#   --preview FILE     optional second FBX with the mesh too (for render.py contact sheets)
#   --action SUB       substring of the action name to use (default: first action)
#   --start F --end F  trim to source frames F..END inclusive (source numbering, first frame is 1; default whole clip)
#   --auto-end         pick END automatically: the frame in the last 40 % of the (trimmed) range whose pose is closest to START
#   --strip MODE       pelvis horizontal travel: lock (default; pelvis X/Y = first frame, Z kept) |
#                      linear (remove only the straight-line drift first to last; keeps the sway) | none
#   --loop N           make a seamless loop: cross-fade the last N frames into the first (slerp on bone rotations,
#                      lerp on locations); the clip gets N frames shorter. 0 = no loop (default). Use 15 for strides.
#   --closed           with --loop: append a copy of frame 1 as the last frame (last == first exactly)
#   --name NAME        name of the baked action (default: input action name)
#   --fps N            bake at N fps (default: the clip's own rate, read from the scene after import)
#   --report FILE      write the measurements (JSON) to FILE
# After export the result is re-imported and pelvis travel and loop seam are printed as a line starting MEASURE.
#
# Export settings (bpy.ops.export_scene.fbx): object_types ARMATURE (+MESH for --preview), use_armature_deform_only False,
# add_leaf_bones False (the 28 bones, incl. head_end, headfront, *_End, *_end, are already real bones and stay as they are),
# bake_anim True, bake_anim_use_all_bones True, bake_anim_use_all_actions False, bake_anim_use_nla_strips False,
# bake_anim_force_startend_keying True, bake_anim_step 1.0, bake_anim_simplify_factor 0.0,
# axis_forward -Z, axis_up Y (the importer's own defaults, so the round trip keeps the axes), apply_unit_scale True,
# apply_scale_options FBX_SCALE_NONE, bone axes Y primary / X secondary (defaults), mesh_smooth_type OFF, path_mode COPY off.
import bpy, sys, os, json, math
from mathutils import Quaternion, Vector, Matrix

argv = sys.argv[sys.argv.index("--") + 1:]
def opt(name, default=None, cast=str):
    if name in argv:
        i = argv.index(name)
        return cast(argv[i + 1])
    return default
SRC = opt("--in"); OUT = opt("--out"); PREV = opt("--preview")
ACT = opt("--action"); START = opt("--start", None, int); END = opt("--end", None, int)
STRIP = opt("--strip", "lock"); LOOP = opt("--loop", 0, int); CLOSED = "--closed" in argv
AUTOEND = "--auto-end" in argv
NAME = opt("--name"); FPS = opt("--fps", None, int); REPORT = opt("--report")
if not SRC or not OUT:
    raise SystemExit("usage: --in X.fbx --out Y.fbx [--action S --start F --end F --strip lock|linear|none --loop N --closed --name N --fps N --preview P --report R]")

def end_pose_diff(arm, f_a, f_b):
    """largest local bone rotation difference (deg) and largest pelvis-relative position difference (cm) between two frames"""
    sc = bpy.context.scene
    def pose(f):
        sc.frame_set(f)
        rot = {pb.name: pb.matrix_basis.to_quaternion() for pb in arm.pose.bones}
        loc = {pb.name: (arm.matrix_world @ pb.head).copy() for pb in arm.pose.bones}
        return rot, loc
    ra, la = pose(f_a); rb, lb = pose(f_b)
    angs = {n: math.degrees(2 * math.acos(min(1.0, abs(ra[n].dot(rb[n]))))) for n in ra if n != "pelvis"}
    worst = max(angs, key=angs.get)
    ends = ["foot_l", "foot_r", "hand_l", "hand_r", "head"]
    pos = max(((la[n] - la["pelvis"]) - (lb[n] - lb["pelvis"])).length for n in ends) * 100
    return angs[worst], worst, sum(angs.values()) / len(angs), pos

def measure(path, tag):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=path)
    arm = [o for o in bpy.data.objects if o.type == "ARMATURE"][0]
    sc = bpy.context.scene
    arm.animation_data_create()
    out = []
    for act in bpy.data.actions:
        arm.animation_data.action = act
        f0, f1 = [int(round(x)) for x in act.frame_range]
        sc.frame_set(f0); p0 = (arm.matrix_world @ arm.pose.bones["pelvis"].head).copy()
        sc.frame_set(f1); p1 = (arm.matrix_world @ arm.pose.bones["pelvis"].head).copy()
        zs = []
        for f in range(f0, f1 + 1):
            sc.frame_set(f); zs.append((arm.matrix_world @ arm.pose.bones["pelvis"].head).z)
        a, bone, mean, pos = end_pose_diff(arm, f0, f1)
        # typical one-frame step, for judging the wrap step of a loop
        steps = []
        for f in range(f0, f1, max(1, (f1 - f0) // 12)):
            steps.append(end_pose_diff(arm, f, f + 1)[0])
        steps.sort()
        out.append(dict(tag=tag, action=act.name, frames=f1 - f0 + 1, fps=sc.render.fps, seconds=round((f1 - f0 + 1) / sc.render.fps, 2),
            pelvis_travel_cm=round(math.hypot(p1.x - p0.x, p1.y - p0.y) * 100, 2),
            pelvis_max_horizontal_cm=round(0, 2), pelvis_dz_cm=round((p1.z - p0.z) * 100, 1), pelvis_z_range_cm=round((max(zs) - min(zs)) * 100, 1),
            seam_max_deg=round(a, 1), seam_bone=bone, seam_mean_deg=round(mean, 1), seam_pos_cm=round(pos, 1),
            median_step_deg=round(steps[len(steps) // 2], 1)))
    return out

before = measure(SRC, "before")
print("MEASURE_BEFORE " + json.dumps(before))

# ---- load, sample, edit, bake, export
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=SRC)
sc = bpy.context.scene
arm = [o for o in bpy.data.objects if o.type == "ARMATURE"][0]
arm.animation_data_create()
acts = list(bpy.data.actions)
act = [a for a in acts if (ACT or "") in a.name][0] if ACT else acts[0]
arm.animation_data.action = act
ACT_NAME = act.name
f0, f1 = [int(round(x)) for x in act.frame_range]
s = START if START is not None else f0
e = END if END is not None else f1
s = max(s, f0); e = min(e, f1)
if AUTOEND:
    best = None
    for j in range(e - int((e - s) * 0.4), e + 1):
        d = end_pose_diff(arm, s, j)
        if best is None or d[0] < best[1]: best = (j, d[0])
    print("AUTO_END %d (%.1f deg)" % best)
    e = best[0]
fps = FPS or sc.render.fps
sc.render.fps = fps

names = [pb.name for pb in arm.pose.bones]
frames = []   # per source frame: {bone: (loc, quat, scale)}
pelvis_w = [] # world head of the pelvis
mw3 = arm.matrix_world.to_3x3()
for f in range(s, e + 1):
    sc.frame_set(f)
    d = {}
    for pb in arm.pose.bones:
        l, q, sc_ = pb.matrix_basis.decompose()
        d[pb.name] = [l.copy(), q.copy(), sc_.copy()]
    frames.append(d)
    pelvis_w.append((arm.matrix_world @ arm.pose.bones["pelvis"].head).copy())

# strip pelvis horizontal travel (world X/Y), keep Z. pelvis has no parent: pose location is bone-rest-space.
pel = arm.data.bones["pelvis"]
rest3 = pel.matrix_local.to_3x3()
n = len(frames)
for i in range(n):
    if STRIP == "none": break
    if STRIP == "lock":
        dw = Vector((pelvis_w[i].x - pelvis_w[0].x, pelvis_w[i].y - pelvis_w[0].y, 0.0))
    else:  # linear
        t = i / max(1, n - 1)
        dw = Vector(((pelvis_w[-1].x - pelvis_w[0].x) * t, (pelvis_w[-1].y - pelvis_w[0].y) * t, 0.0))
    d_arm = mw3.inverted() @ dw
    frames[i]["pelvis"][0] = frames[i]["pelvis"][0] - rest3.inverted() @ d_arm

# loop: cross-fade the last N frames into the first
if LOOP > 0:
    N = min(LOOP, n // 2)
    L = n - N
    out = []
    for i in range(L):
        if i < N:
            w = i / N
            tail = frames[L + i]; head = frames[i]
            d = {}
            for b in names:
                lt, qt, st = tail[b]; lh, qh, sh = head[b]
                d[b] = [lt.lerp(lh, w), qt.slerp(qh, w), st.lerp(sh, w)]
            out.append(d)
        else:
            out.append(frames[i])
    if CLOSED:
        out.append(out[0])
    frames = out
n = len(frames)

# bake: new action, one key per frame on every bone, frame numbering from 1
new_name = NAME or act.name.split("|")[-1]
new = bpy.data.actions.new(new_name)
arm.animation_data.action = new
for pb in arm.pose.bones:
    pb.rotation_mode = "QUATERNION"
for i, d in enumerate(frames):
    for pb in arm.pose.bones:
        l, q, sc_ = d[pb.name]
        pb.location = l; pb.rotation_quaternion = q; pb.scale = sc_
        pb.keyframe_insert("location", frame=i + 1)
        pb.keyframe_insert("rotation_quaternion", frame=i + 1)
        pb.keyframe_insert("scale", frame=i + 1)
for a in list(bpy.data.actions):
    if a is not new:
        bpy.data.actions.remove(a)
sc.frame_start = 1; sc.frame_end = n

def export(path, with_mesh):
    os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
    bpy.ops.object.select_all(action="DESELECT")
    for o in bpy.data.objects:
        if o.type == "ARMATURE" or (with_mesh and o.type == "MESH"):
            o.select_set(True)
    bpy.ops.export_scene.fbx(filepath=path, use_selection=True,
        object_types={"ARMATURE", "MESH"} if with_mesh else {"ARMATURE"},
        use_armature_deform_only=False, add_leaf_bones=False,
        bake_anim=True, bake_anim_use_all_bones=True, bake_anim_use_all_actions=False, bake_anim_use_nla_strips=False,
        bake_anim_force_startend_keying=True, bake_anim_step=1.0, bake_anim_simplify_factor=0.0,
        axis_forward="-Z", axis_up="Y", apply_unit_scale=True, apply_scale_options="FBX_SCALE_NONE",
        mesh_smooth_type="OFF", embed_textures=with_mesh, path_mode="COPY" if with_mesh else "AUTO")
export(OUT, False)
if PREV:
    export(PREV, True)

after = measure(OUT, "after")
print("MEASURE_AFTER " + json.dumps(after))
if REPORT:
    json.dump(dict(src=SRC, out=OUT, action=ACT_NAME, trim=[s, e], strip=STRIP, loop=LOOP, closed=CLOSED, fps=fps, before=before, after=after), open(REPORT, "w"), indent=1)
