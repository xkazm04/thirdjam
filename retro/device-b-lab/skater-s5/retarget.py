# S5 hybrid: S4's scripted clips drive the Rodin+Meshy mesh R.
#   blender -b --factory-startup --python retarget.py -- <s5_dir>
# Inputs (snapshots in <s5_dir>/in/): SK_Skater_S4.fbx (S4 mesh+stick+24 bones+clips), R_rigged.fbx (Meshy rig of Rodin mesh, 28 bones).
# Output: <s5_dir>/export/SK_Skater_S5.fbx, <s5_dir>/s5_skater.blend, <s5_dir>/retarget_report.json
#
# Method (direct matrix maths, no constraints, so nothing to remove):
#  0. R is turned to face +X like S4 (Meshy delivers -Y facing with a 90 deg X on the armature object); transforms applied.
#     A real `root` bone at the origin becomes the parent of `pelvis`; sockets added. R's skate boot was weighted to `calf`
#     (no working ankle): the boot below the sock band is re-weighted to `foot` and `foot`'s pivot moved to the boot top.
#     Vertex positions, UVs, material and textures are not touched.
#  1. For each mapped bone: world rotation(R, t) = [S4 world rotation(t) * S4 rest rotation^-1] * R rest rotation
#     (same world-space change from rest). Positions come from R's own hierarchy (FK), so R keeps its proportions.
#     Pelvis: R rest x,y (no horizontal travel), z = R rest + K * (S4 pelvis z - S4 rest pelvis z), K = leg-length ratio.
#  2. Left hand: two-bone IK onto the stick shaft at the same shaft fraction as S4's glove, weighted by how close S4's glove is to its shaft.
#  3. Head clamp: if a mitten centre comes closer to the helmet centre than HEAD_CLEAR, the upper arm is swung about the shoulder
#     by the smallest rotation that puts it on that sphere (the stick follows the right hand).
#  4. Foot pass: per frame the whole figure is moved vertically (pelvis z) so its lowest body vertex sits at K * S4's lowest body vertex
#     (0 when S4 stands on the ice; S4 has no jumps, so this is "lower skate on the ice").
#  5. Stick pass: the stick slides along its own shaft through the mitten (max STICK_SLIDE) so the blade reaches the ice when S4's blade is on
#     the ice, and is pushed up out of the ice whenever it would sink.
import bpy, sys, os, math, json, time
import numpy as np
from mathutils import Matrix, Vector, Quaternion

S5 = sys.argv[sys.argv.index("--") + 1]
IN = os.path.join(S5, "in")
OUT_FBX = os.path.join(S5, "export", "SK_Skater_S5.fbx")
os.makedirs(os.path.dirname(OUT_FBX), exist_ok=True)
T0 = time.time()

HEAD_CLEAR = 0.40      # helmet radius (~0.28) + mitten radius (~0.12): mitten may touch, not sink
STICK_HEAD_CLEAR = 0.36  # helmet half-size (0.28-0.30) plus the chin, which juts to ~0.33 from the helmet centre
STICK_SLIDE = 0.25     # max slide of the stick through the mitten, m
DZ_RATE = 0.02         # foot pass: max change of the vertical correction per frame (60 fps), m
ANKLE_Z, ANKLE_BLEND = 0.30, 0.04   # boot/sock boundary of R and half-width of the weight blend

MAP = ["pelvis", "spine_01", "spine_02", "spine_03", "neck_01", "head",
       "clavicle_l", "upperarm_l", "lowerarm_l", "hand_l", "clavicle_r", "upperarm_r", "lowerarm_r", "hand_r",
       "thigh_l", "calf_l", "foot_l", "thigh_r", "calf_r", "foot_r", "stick_socket"]

def log(*a):
    print("S5", *a, flush=True)

# ---------------------------------------------------------------- load R, normalise
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=os.path.join(IN, "R_rigged.fbx"), automatic_bone_orientation=False)
rarm = next(o for o in bpy.data.objects if o.type == "ARMATURE")
rmesh = next(o for o in bpy.data.objects if o.type == "MESH")
rarm.name = "R_ARM"; rmesh.name = "SK_Skater_S5"
if rarm.animation_data:
    rarm.animation_data.action = None
for a in list(bpy.data.actions):
    bpy.data.actions.remove(a)
for pb in rarm.pose.bones:
    pb.rotation_mode = "QUATERNION"
    pb.matrix_basis = Matrix.Identity(4)
bpy.context.view_layer.update()

turn = Matrix.Rotation(math.radians(90), 4, "Z")          # -Y facing -> +X facing
mw_mesh = rmesh.matrix_world.copy(); mw_arm = rarm.matrix_world.copy()
rmesh.data.transform(turn @ mw_mesh)
rarm.data.transform(turn @ mw_arm)
rarm.matrix_world = Matrix.Identity(4)
rmesh.matrix_parent_inverse = Matrix.Identity(4)
rmesh.matrix_basis = Matrix.Identity(4)
bpy.context.view_layer.update()

V = np.zeros(len(rmesh.data.vertices) * 3, dtype=np.float64)
rmesh.data.vertices.foreach_get("co", V); V = V.reshape(-1, 3)
log("R rest bounds", V.min(0).round(3).tolist(), V.max(0).round(3).tolist())

# vertex weights as a dense table
gname = {g.index: g.name for g in rmesh.vertex_groups}
W = {}
for v in rmesh.data.vertices:
    for g in v.groups:
        W.setdefault(gname[g.group], {})[v.index] = g.weight

def dominant(name):
    out = []
    for v in rmesh.data.vertices:
        if v.groups:
            g = max(v.groups, key=lambda g: g.weight)
            if gname[g.group] == name:
                out.append(v.index)
    return out

# ---- re-weight the skate boots to foot, pivot at the boot top
def smooth(e0, e1, x):
    t = min(1.0, max(0.0, (x - e0) / (e1 - e0)))
    return t * t * (3 - 2 * t)

ankle = {}
for s, toe in (("l", "LeftToe_end"), ("r", "RightToe_end")):
    calf, foot, ball = "calf_" + s, "foot_" + s, "ball_" + s
    leg_vs = set(W.get(calf, {})) | set(W.get(foot, {})) | set(W.get(ball, {})) | set(W.get(toe, {}))
    band = [V[i] for i in leg_vs if abs(V[i][2] - ANKLE_Z) < 0.02 and W.get(calf, {}).get(i, 0) > 0.5]
    c = np.mean(band, axis=0)
    ankle[s] = Vector((float(c[0]), float(c[1]), ANKLE_Z))
    gf = rmesh.vertex_groups[foot]; gc = rmesh.vertex_groups[calf]
    for i in leg_vs:
        z = V[i][2]
        wc = W.get(calf, {}).get(i, 0.0)
        wf = W.get(foot, {}).get(i, 0.0) + W.get(ball, {}).get(i, 0.0) + W.get(toe, {}).get(i, 0.0)
        t = 1.0 - smooth(ANKLE_Z - ANKLE_BLEND, ANKLE_Z + ANKLE_BLEND, z)   # 1 below the band
        move = wc * t
        wc2, wf2 = wc - move, wf + move
        for gn in (ball, toe):
            if i in W.get(gn, {}):
                rmesh.vertex_groups[gn].remove([i])
        if wf2 > 1e-4:
            gf.add([i], wf2, "REPLACE")
        elif i in W.get(foot, {}):
            gf.remove([i])
        if wc2 > 1e-4:
            gc.add([i], wc2, "REPLACE")
        elif i in W.get(calf, {}):
            gc.remove([i])
log("ankle pivots", {k: [round(x, 3) for x in v] for k, v in ankle.items()})

# mitten centres and helmet centre (rest, world = armature space)
mit = {s: Vector(np.mean([V[i] for i in dominant("hand_" + s)], axis=0).tolist()) for s in "lr"}
hv = np.array([V[i] for i in dominant("head")])
head_c = Vector(((hv.min(0) + hv.max(0)) / 2).tolist())
head_top = float(hv[:, 2].max())
log("mittens", {k: [round(x, 3) for x in v] for k, v in mit.items()}, "head centre", [round(x, 3) for x in head_c], "top", round(head_top, 3))

# ---------------------------------------------------------------- load S4 (after R so S4 keeps the name Armature)
bpy.ops.import_scene.fbx(filepath=os.path.join(IN, "SK_Skater_S4.fbx"), automatic_bone_orientation=False)
sarm = next(o for o in bpy.data.objects if o.type == "ARMATURE" and o is not rarm)
sbody = bpy.data.objects["SK_Skater_S4"]; sstick = bpy.data.objects["SK_Skater_S4_Stick"]
sarm.name = "S4_ARM"
SMW = sarm.matrix_world.copy()        # 12:09 S4 file is in cm: Blender re-imports it with an armature object scale of 0.01
log("S4 armature object matrix scale", [round(x, 5) for x in SMW.to_scale()])
def Wm(X):
    """S4 armature-space matrix -> world (metres), rotation part normalised (no scale)."""
    M = SMW @ X
    return Matrix.Translation(M.translation) @ M.to_3x3().normalized().to_4x4()
def world_co(obj):
    c = np.zeros(len(obj.data.vertices) * 3); obj.data.vertices.foreach_get("co", c); c = c.reshape(-1, 3)
    mw = np.array(obj.matrix_world)
    return c @ mw[:3, :3].T + mw[:3, 3]
clips = {}
for a in bpy.data.actions:
    clips[a.name.split("|")[-1]] = a
ORDER = ["Idle_Faceoff", "Stride", "Glide", "Stop", "Shot_Slap", "Shot_Wrist", "Check", "Fall", "GetUp", "Celebrate",
         "Fall_Forward", "GetUp_Front", "Crossover_Left", "Crossover_Right", "Lean_Left", "Lean_Right",
         "Poke", "Stumble", "Lift", "Pass", "Foul_Reaction", "Faceoff_Ready", "Faceoff_Draw", "Goalie_Hold",
         "Goalie_Block_Left", "Goalie_Block_Right", "Shot_Charge", "Aim_Front", "Aim_Left", "Aim_Right", "Aim_FarReach"]
# loops as S4 ships them (its loopcut file trims exactly these); one-shots that start and end in Glide are NOT loops
LOOPS = {"Idle_Faceoff", "Stride", "Glide", "Crossover_Left", "Crossover_Right", "Lean_Left", "Lean_Right", "Faceoff_Ready",
         "Goalie_Hold", "Shot_Charge", "Aim_Front", "Aim_Left", "Aim_Right", "Aim_FarReach"}
# frame (1-based in the clip) at which the stick pose is measured: the hold, the thrust, the block
MEASURE_AT = {"Aim_Front": 1, "Aim_Left": 1, "Aim_Right": 1, "Aim_FarReach": 1, "Poke": 10, "Faceoff_Ready": 1,
              "Goalie_Hold": 1, "Goalie_Block_Left": 25, "Goalie_Block_Right": 25, "Shot_Charge": 1, "Lift": 10, "Pass": 11}
ORDER = [c for c in ORDER if c in clips] + sorted(c for c in clips if c not in ORDER)
log("S4 clips", ORDER)
sR = {b.name: Wm(b.matrix_local) for b in sarm.data.bones}
s4_shaft_local = Vector((0, 1, 0))                     # stick_socket's Y axis points down the shaft (head -> tail)
# S4 glove centre in hand_l space, stick vertices in stick_socket space
sbv = world_co(sbody)
sg = {g.index: g.name for g in sbody.vertex_groups}
def s4_dom(name):
    return [v.index for v in sbody.data.vertices if v.groups and sg[max(v.groups, key=lambda g: g.weight).group] == name]
s4_glove_l = sR["hand_l"].inverted() @ Vector(np.mean([sbv[i] for i in s4_dom("hand_l")], axis=0).tolist())
s4_skate = {s: [i for i in s4_dom("foot_" + s)] for s in "lr"}
stv = world_co(sstick)
stick_local = [sR["stick_socket"].inverted() @ Vector(p.tolist()) for p in stv]   # stick verts in socket rest space
blade_local = [p for p in stick_local if p.y > 1.2]                                 # far end of the shaft: the blade
log("stick verts", len(stick_local), "blade verts", len(blade_local))

# ---------------------------------------------------------------- R armature edits: root, foot pivots, sockets
bpy.context.view_layer.objects.active = rarm
for o in bpy.context.selected_objects:
    o.select_set(False)
rarm.select_set(True)
bpy.ops.object.mode_set(mode="EDIT")
eb = rarm.data.edit_bones
root = eb.new("root"); root.head = (0, 0, 0); root.tail = (0, 0.15, 0); root.roll = 0   # along +Y, roll 0: rest matrix = identity
eb["pelvis"].use_connect = False; eb["pelvis"].parent = root
for s in "lr":
    f = eb["foot_" + s]
    kids = [(c, c.head.copy(), c.tail.copy()) for c in f.children]
    f.use_connect = False
    f.head = ankle[s]; f.tail = ankle[s] + Vector((0.14, 0, 0)); f.roll = 0
    for c, h, t in kids:
        c.use_connect = False; c.head = h; c.tail = t
# stick_socket in the right mitten with S4's socket rest rotation, head_socket above the helmet, puck_socket as S4
ss = eb.new("stick_socket"); ss.parent = eb["hand_r"]
srot = sR["stick_socket"].to_3x3()
ss.head = mit["r"]; ss.tail = mit["r"] + srot.col[1] * 0.065
ss.align_roll(srot.col[2])
hs = eb.new("head_socket"); hs.parent = eb["head"]
hs.head = (head_c.x, head_c.y, head_top + 0.13); hs.tail = (head_c.x, head_c.y, head_top + 0.13 + 0.25)
ps = eb.new("puck_socket"); ps.parent = root
ps.head = sR["puck_socket"].translation; ps.tail = ps.head + Vector((0, 0, 0.25))
bpy.ops.object.mode_set(mode="OBJECT")
rR = {b.name: b.matrix_local.copy() for b in rarm.data.bones}
log("socket rot error deg", round(math.degrees(rR["stick_socket"].to_quaternion().rotation_difference(sR["stick_socket"].to_quaternion()).angle), 3))

# leg-length ratio: hip -> ankle pivot (R) over hip -> ankle (S4)
def leg_len(R, s):
    return (R["thigh_" + s].translation - R["calf_" + s].translation).length + (R["calf_" + s].translation - R["foot_" + s].translation).length
K = (leg_len(rR, "l") + leg_len(rR, "r")) / (leg_len(sR, "l") + leg_len(sR, "r"))
log("leg ratio K", round(K, 4))

# ---------------------------------------------------------------- stick mesh onto R
stk = sstick.copy(); stk.data = sstick.data.copy(); bpy.context.scene.collection.objects.link(stk)
stk.name = "SK_Skater_S5_Stick"
for m in list(stk.modifiers):
    stk.modifiers.remove(m)
_smw = sstick.matrix_world.copy()
stk.parent = None; stk.matrix_world = Matrix.Identity(4)
stk.data.transform(rR["stick_socket"] @ sR["stick_socket"].inverted() @ _smw)
stk.vertex_groups.clear()
g = stk.vertex_groups.new(name="stick_socket"); g.add(list(range(len(stk.data.vertices))), 1.0, "REPLACE")
stk.parent = rarm
md = stk.modifiers.new("Armature", "ARMATURE"); md.object = rarm

# ---------------------------------------------------------------- retarget maths
rbones = sorted(rarm.data.bones, key=lambda b: len(b.parent_recursive))
rpar = {b.name: (b.parent.name if b.parent else None) for b in rarm.data.bones}
kids = {}
for b in rarm.data.bones:
    if b.parent:
        kids.setdefault(b.parent.name, []).append(b.name)
mit_local = {s: rR["hand_" + s].inverted() @ mit[s] for s in "lr"}
head_local = rR["head"].inverted() @ head_c
s_rest_rot = {n: sR[n].to_quaternion() for n in MAP}
r_rest_rot = {n: rR[n].to_quaternion() for n in MAP}

def rot_between(a, b):
    a = a.normalized(); b = b.normalized()
    return a.rotation_difference(b)

def subtree(n):
    out = [n]
    for c in kids.get(n, []):
        out += subtree(c)
    return out

def rotate_subtree(M, n, q, pivot):
    R = q.to_matrix().to_4x4()
    T = Matrix.Translation(pivot) @ R @ Matrix.Translation(-pivot)
    for b in subtree(n):
        M[b] = T @ M[b]

def solve_frame(S):
    """S: dict S4 bone -> armature-space pose matrix. Returns R pose matrices (armature space) and stats."""
    st = {}
    M = {}
    for b in rbones:
        n = b.name; p = rpar[n]
        if n == "root":
            M[n] = Matrix.Identity(4); continue
        base = (M[p] @ rR[p].inverted() @ rR[n]) if p else rR[n].copy()
        if n == "pelvis":
            pos = rR[n].translation + Vector((0, 0, K * (S["pelvis"].translation.z - sR["pelvis"].translation.z)))
        else:
            pos = base.translation
        if n in MAP:
            dq = S[n].to_quaternion() @ s_rest_rot[n].inverted()
            q = dq @ r_rest_rot[n]
            M[n] = Matrix.Translation(pos) @ q.to_matrix().to_4x4()
        else:
            M[n] = Matrix.Translation(pos) @ base.to_3x3().to_4x4()
    st["clamp_deg"] = 0.0
    hc = M["head"] @ head_local
    head_clamp(M, "r", hc, st)
    # --- stick vs helmet: tilt the shaft about the grip until it clears the helmet sphere
    st["stick_tilt_deg"] = 0.0
    G = M["stick_socket"].translation
    u = (M["stick_socket"].to_3x3() @ s4_shaft_local).normalized()
    cg = hc - G
    tpar = cg.dot(u)
    if 0.0 < tpar < 1.6 and cg.length > STICK_HEAD_CLEAR + 1e-3:
        dist = (cg - u * tpar).length
        if dist < STICK_HEAD_CLEAR:
            need = math.asin(STICK_HEAD_CLEAR / cg.length)          # smallest allowed angle between shaft and grip->helmet
            have = u.angle(cg)
            axis = cg.cross(u)
            if axis.length < 1e-6:
                axis = Vector((0, 0, 1)).cross(u)
            q = Quaternion(axis.normalized(), need - have)
            rotate_subtree(M, "stick_socket", q, G)
            st["stick_tilt_deg"] = math.degrees(need - have)
    # --- left hand onto the shaft
    s_sock = S["stick_socket"]; s_u = (s_sock.to_3x3() @ s4_shaft_local).normalized()
    s_glove = S["hand_l"] @ s4_glove_l
    s_par = (s_glove - s_sock.translation).dot(s_u)
    s_dist = ((s_glove - s_sock.translation) - s_u * s_par).length
    w = 1.0 - smooth(0.05, 0.12, s_dist)
    st["grip_w"] = w; st["grip_gap"] = 0.0
    if w > 0:
        r_sock = M["stick_socket"]; r_u = (r_sock.to_3x3() @ s4_shaft_local).normalized()
        # same fraction of the shaft measured from the grip; keep the bottom hand at least a mitten below the top mitten
        target_m = r_sock.translation + r_u * max(s_par, 0.22)
        cur_m = M["hand_l"] @ mit_local["l"]
        T = cur_m.lerp(target_m, w)
        hand_q = M["hand_l"].to_quaternion()
        off = M["hand_l"].to_3x3() @ mit_local["l"]           # wrist -> mitten centre, world
        Sh = M["upperarm_l"].translation; E = M["lowerarm_l"].translation; Wr = M["hand_l"].translation
        Tw = T - off
        a = (E - Sh).length; bl = (Wr - E).length
        d = min(max((Tw - Sh).length, abs(a - bl) + 1e-3), a + bl - 1e-3)
        axis = (Tw - Sh).normalized()
        pole = (E - Sh) - axis * (E - Sh).dot(axis)
        if pole.length < 1e-5:
            pole = Vector((0, 0, -1)) - axis * axis.z
        pole.normalize()
        ca = (a * a + d * d - bl * bl) / (2 * a * d)
        ang = math.acos(max(-1, min(1, ca)))
        E2 = Sh + axis * (a * math.cos(ang)) + pole * (a * math.sin(ang))
        rotate_subtree(M, "upperarm_l", rot_between(E - Sh, E2 - Sh), Sh)
        Wr1 = M["hand_l"].translation
        W2 = Sh + axis * d
        rotate_subtree(M, "lowerarm_l", rot_between(Wr1 - E2, W2 - E2), E2)
        M["hand_l"] = Matrix.Translation(M["hand_l"].translation) @ hand_q.to_matrix().to_4x4()
        for c in kids.get("hand_l", []):
            for x in subtree(c):
                M[x] = M["hand_l"] @ rR["hand_l"].inverted() @ rR[x]
        st["grip_gap"] = ((M["hand_l"] @ mit_local["l"]) - T).length * w
    head_clamp(M, "l", hc, st)
    return M, st

def head_clamp(M, s, hc, st):
    """Swing the upper arm about the shoulder (smallest rotation) so the mitten centre stays HEAD_CLEAR from the helmet centre."""
    Sh = M["upperarm_" + s].translation
    H = M["hand_" + s] @ mit_local[s]
    if (H - hc).length >= HEAD_CLEAR:
        return
    L = (H - Sh).length; dsc = (hc - Sh).length
    if dsc < 1e-4:
        return
    ax = (hc - Sh) / dsc
    x = (L * L - HEAD_CLEAR * HEAD_CLEAR + dsc * dsc) / (2 * dsc)   # circle centre distance from shoulder
    rr = math.sqrt(max(L * L - x * x, 0.0))
    radial = (H - Sh) - ax * (H - Sh).dot(ax)
    if radial.length < 1e-5:
        radial = Vector((0, 0, 1)) - ax * ax.z
    P = Sh + ax * x + radial.normalized() * rr
    q = rot_between(H - Sh, P - Sh)
    rotate_subtree(M, "upperarm_" + s, q, Sh)
    st["clamp_deg"] = max(st["clamp_deg"], math.degrees(q.angle))

def basis_of(M, n):
    p = rpar[n]
    if p is None:
        return rR[n].inverted() @ M[n]
    return rR[n].inverted() @ rR[p] @ M[p].inverted() @ M[n]

# ---------------------------------------------------------------- per clip: solve, key, foot pass, stick pass
sc = bpy.context.scene
sc.render.fps = 60
dg = bpy.context.evaluated_depsgraph_get()

def lowest(obj, idx=None):
    ev = obj.evaluated_get(dg)
    me = ev.to_mesh()
    co = np.zeros(len(me.vertices) * 3); me.vertices.foreach_get("co", co); co = co.reshape(-1, 3)
    ev.to_mesh_clear()
    co = co @ np.array(obj.matrix_world.to_3x3()).T + np.array(obj.matrix_world.translation)
    if idx is None:
        return float(co[:, 2].min()), co
    return float(co[idx, 2].min()), co

r_skate = {s: np.array([v.index for v in rmesh.data.vertices
                        if any(gname[g.group] == "foot_" + s and g.weight > 0.5 for g in v.groups)]) for s in "lr"}
log("R skate verts", {s: len(v) for s, v in r_skate.items()})

def key_action(act, frames, basis_by_frame, names):
    for n in names:
        locs = [basis_by_frame[i][n].to_translation() for i in range(len(frames))]
        rots = []
        prev = None
        for i in range(len(frames)):
            q = basis_by_frame[i][n].to_quaternion()
            if prev is not None and prev.dot(q) < 0:
                q.negate()
            rots.append(q); prev = q
        grp = act.groups.get(n) or act.groups.new(n)
        for comp, vals, k in (("location", locs, 3), ("rotation_quaternion", rots, 4)):
            path = 'pose.bones["%s"].%s' % (n, comp)
            for j in range(k):
                fc = act.fcurves.find(path, index=j) or act.fcurves.new(path, index=j, action_group=n)
                fc.keyframe_points.clear()
                fc.keyframe_points.add(len(frames))
                co = []
                for f, v in zip(frames, vals):
                    co += [float(f), float(v[j])]
                fc.keyframe_points.foreach_set("co", co)
                fc.keyframe_points.foreach_set("interpolation", [1] * len(frames))   # LINEAR
                fc.update()

report = {"K_leg_ratio": round(K, 4), "head_clear_m": HEAD_CLEAR, "stick_slide_max_m": STICK_SLIDE,
          "ankle_z": ANKLE_Z, "clips": {}}
all_names = [b.name for b in rarm.data.bones]
loops = {}
for cname in ORDER:
    sa = clips[cname]
    f0, f1 = [int(round(x)) for x in sa.frame_range]
    frames = list(range(f0, f1 + 1))
    sarm.animation_data_create(); sarm.animation_data.action = sa
    ra = bpy.data.actions.new(cname); ra.use_fake_user = True
    rarm.animation_data_create(); rarm.animation_data.action = ra
    Ms, stats, s4low, s4skate, s4blade, s4sock, s4pel = [], [], [], [], [], [], []
    for f in frames:
        sc.frame_set(f)
        S = {n: Wm(sarm.pose.bones[n].matrix) for n in MAP}
        M, st = solve_frame(S)
        Ms.append(M); stats.append(st)
        lo, co = lowest(sbody)
        s4low.append(lo)
        s4skate.append(min(float(co[s4_skate["l"], 2].min()), float(co[s4_skate["r"], 2].min())))
        sk = Wm(sarm.pose.bones["stick_socket"].matrix); s4sock.append(sk.copy()); s4pel.append(S["pelvis"].translation.copy())
        s4blade.append(min((sk @ p).z for p in blade_local))
    # first key (no foot pass yet) to measure R's lowest point
    key_action(ra, frames, [{n: basis_of(M, n) for n in all_names} for M in Ms], all_names)
    dz, rlow_pre = [], []
    for i, f in enumerate(frames):
        sc.frame_set(f)
        lo, _ = lowest(rmesh)
        rlow_pre.append(lo)
        dz.append(K * s4low[i] - lo)
    # rate limit as an upper envelope: dz'[i] = max_j dz[j] - RATE*|i-j| (never lower than needed, so never into the ice;
    # a sudden lift, e.g. when R's big helmet becomes the lowest point in a fall, is spread over the frames before it)
    n_f = len(dz); raw_dz = list(dz)
    sc.frame_set(frames[0]); s_a = {n: sarm.pose.bones[n].head.copy() for n in MAP}
    sc.frame_set(frames[-1]); s_b = {n: sarm.pose.bones[n].head.copy() for n in MAP}
    cyc = cname in LOOPS
    dz = [max(raw_dz[j] - DZ_RATE * (min(abs(i - j), n_f - 1 - abs(i - j)) if cyc else abs(i - j)) for j in range(n_f))
          for i in range(n_f)]
    # foot pass: shift pelvis (and so everything) by dz; stick pass: slide along the shaft
    slide, blade_after, rskate_after = [], [], []
    for i, M in enumerate(Ms):
        T = Matrix.Translation(Vector((0, 0, dz[i])))
        sock = M["stick_socket"]
        u = (sock.to_3x3() @ s4_shaft_local).normalized()
        b = min((sock @ p).z for p in blade_local) + dz[i]
        want = 0.0
        contact = 1.0 - smooth(0.03, 0.20, s4blade[i])            # 1 while S4's blade is on the ice
        if abs(u.z) > 0.15:
            want = contact * (s4blade[i] - b) / u.z
            if b + want * u.z < 0:                                 # never into the ice
                want = -b / u.z
        want = max(-STICK_SLIDE, min(STICK_SLIDE, want))
        M["stick_socket"] = Matrix.Translation(u * want) @ M["stick_socket"]
        slide.append(want)
        blade_after.append(b + want * u.z)
        M["pelvis"] = T @ M["pelvis"]
        # keep children consistent with the shifted pelvis (absolute matrices for later measurement)
        for x in subtree("pelvis"):
            if x != "pelvis":
                M[x] = T @ M[x]
    key_action(ra, frames, [{n: basis_of(M, n) for n in all_names} for M in Ms], all_names)
    # measure after the passes, from the evaluated mesh
    ferr, low_after = [], []
    for i, f in enumerate(frames):
        sc.frame_set(f)
        lo, co = lowest(rmesh)
        low_after.append(lo)
        lsk = min(float(co[r_skate["l"], 2].min()), float(co[r_skate["r"], 2].min()))
        if s4skate[i] < 0.005:           # S4's lower skate is on the ice: R's must be too
            ferr.append(lsk)
        rskate_after.append(lsk)
    pel_z = [M["pelvis"].translation.z for M in Ms]
    # loop = S4's first and last poses are identical (S4 copies frame 1 into the last frame)
    sc.frame_set(frames[0]); s_first = {n: sarm.pose.bones[n].head.copy() for n in MAP}
    sc.frame_set(frames[-1]); s_last = {n: sarm.pose.bones[n].head.copy() for n in MAP}
    is_loop = cname in LOOPS and len(frames) > 2 and max((s_first[n] - s_last[n]).length for n in MAP) < 1e-4
    loops[cname] = is_loop
    rec = {
        "frames": len(frames), "seconds": round((len(frames) - 1) / 60, 3), "loop": is_loop,
        "seam_max_joint_cm": round(max((Ms[0][n].translation - Ms[-1][n].translation).length for n in all_names) * 100, 4),
        "pelvis_horizontal_max_cm": round(max((Vector((M["pelvis"].translation.x, M["pelvis"].translation.y)) -
                                                rR["pelvis"].translation.xy).length for M in Ms) * 100, 4),
        "pelvis_z_range_cm": round((max(pel_z) - min(pel_z)) * 100, 2),
        "foot_pass_dz_cm": [round(min(dz) * 100, 2), round(max(dz) * 100, 2)],
        "foot_pass_max_step_cm": round(max([abs(dz[i + 1] - dz[i]) for i in range(len(dz) - 1)] or [0]) * 100, 2),
        "foot_pass_max_step_before_rate_limit_cm": round(max([abs(raw_dz[i + 1] - raw_dz[i]) for i in range(len(raw_dz) - 1)] or [0]) * 100, 2),
        "rate_limit_lift_max_cm": round(max(dz[i] - raw_dz[i] for i in range(len(dz))) * 100, 2),
        "lower_skate_error_cm_max_abs": round(max([abs(x) for x in ferr] or [0]) * 100, 3),
        "lower_skate_error_frames": len(ferr),
        "lowest_body_point_min_cm": round(min(low_after) * 100, 3),
        "blade_min_cm": round(min(blade_after) * 100, 2),
        "blade_contact_frames_s4": sum(1 for x in s4blade if x < 0.03),
        "blade_contact_err_cm_max": round(max([abs(blade_after[i] - s4blade[i]) for i in range(len(frames)) if s4blade[i] < 0.03] or [0]) * 100, 2),
        "stick_slide_cm": [round(min(slide) * 100, 1), round(max(slide) * 100, 1)],
        "left_grip_frames": sum(1 for s in stats if s["grip_w"] > 0.5),
        "left_grip_gap_cm_max": round(max(s["grip_gap"] for s in stats) * 100, 2),
        "head_clamp_frames": sum(1 for s in stats if s["clamp_deg"] > 0),
        "head_clamp_max_deg": round(max(s["clamp_deg"] for s in stats), 1),
        "stick_tilt_frames": sum(1 for s in stats if s["stick_tilt_deg"] > 0),
        "stick_tilt_max_deg": round(max(s["stick_tilt_deg"] for s in stats), 1),
    }
    # stick vs helmet over the clip: nearest stick vertex to the helmet centre (helmet half-size about 0.28-0.30 m)
    hc_all = [M["head"] @ head_local for M in Ms]
    rec["stick_to_helmet_centre_min_cm"] = round(min(min(((M["stick_socket"] @ p) - hc).length for p in stick_local)
                                                     for M, hc in zip(Ms, hc_all)) * 100, 1)
    rec["mitten_to_helmet_centre_min_cm"] = round(min(min(((M["hand_" + s] @ mit_local[s]) - hc).length for s in "lr")
                                                      for M, hc in zip(Ms, hc_all)) * 100, 1)
    if cname in MEASURE_AT:
        i = min(MEASURE_AT[cname], len(frames)) - 1
        bc_local = sum(blade_local, Vector()) / len(blade_local)
        def aim(sock, pel):
            b = sock @ bc_local; d = b - pel
            return {"angle_deg": round(math.degrees(math.atan2(d.y, d.x)), 1), "reach_m": round(math.hypot(d.x, d.y), 3),
                    "blade_centre_z_cm": round(b.z * 100, 1)}
        M = Ms[i]
        rec["measure_frame"] = frames[i]
        rec["aim_R"] = aim(M["stick_socket"], M["pelvis"].translation)
        rec["aim_S4"] = aim(s4sock[i], s4pel[i])
        zs = [(M["stick_socket"] @ p).z for p in stick_local]
        rec["stick_z_cm_at_measure"] = [round(min(zs) * 100, 1), round(max(zs) * 100, 1)]
        zs4 = [(s4sock[i] @ p).z for p in stick_local]
        rec["stick_z_cm_at_measure_S4"] = [round(min(zs4) * 100, 1), round(max(zs4) * 100, 1)]
        rec["left_grip_gap_cm_at_measure"] = round(stats[i]["grip_gap"] * 100, 1)
        rec["left_grip_w_at_measure"] = round(stats[i]["grip_w"], 2)
    report["clips"][cname] = rec
    log("CLIP", cname, json.dumps(rec))

# ---------------------------------------------------------------- clean up and export
sarm.animation_data.action = None
for o in (sbody, sstick, sarm):
    bpy.data.objects.remove(o, do_unlink=True)
for a in list(bpy.data.actions):
    if a.name.split("|")[-1] not in ORDER or a.users == 0 and not a.use_fake_user:
        bpy.data.actions.remove(a)
for a in list(bpy.data.actions):
    if a.name not in ORDER:
        bpy.data.actions.remove(a)
rarm.name = "Armature"; rarm.data.name = "Armature"
rarm.animation_data.action = bpy.data.actions["Stride"]
sc.frame_start, sc.frame_end = 1, 61
for img in bpy.data.images:
    if img.source == "FILE" and not img.packed_file:
        try:
            img.pack()
        except Exception as e:
            log("pack failed", img.name, e)
bpy.ops.wm.save_as_mainfile(filepath=os.path.join(S5, "s5_skater.blend"))   # metres, for inspection

# ---- body material: base colour only, referencing the lead's cleaned PNG by path (nothing embedded)
CLEAN_PNG = "C:/Users/kazda/jam/skater-lab/s6/T_SkaterTB2_BaseColor_clean.png"
mat = rmesh.material_slots[0].material
nt = mat.node_tree
bsdf = next(n for n in nt.nodes if n.type == "BSDF_PRINCIPLED")
for n in list(nt.nodes):
    if n.type in ("TEX_IMAGE", "NORMAL_MAP", "SEPARATE_COLOR", "SEPRGB", "SEPARATE_RGB"):
        nt.nodes.remove(n)
tex = nt.nodes.new("ShaderNodeTexImage")
tex.image = bpy.data.images.load(CLEAN_PNG, check_existing=True)
nt.links.new(tex.outputs["Color"], bsdf.inputs["Base Color"])
for im in list(bpy.data.images):
    if im.users == 0:
        bpy.data.images.remove(im)
log("body material", mat.name, "base colour", tex.image.filepath)

# ---- centimetres in the data (S4's 12:09 recipe): bones, meshes and location keys x100, scene unit 0.01 m, FBX_SCALE_NONE
CM = Matrix.Scale(100.0, 4)
rarm.data.transform(CM)
rmesh.data.transform(CM)
stk.data.transform(CM)
for a in bpy.data.actions:
    for fc in a.fcurves:
        if fc.data_path.endswith(".location"):
            co = np.zeros(len(fc.keyframe_points) * 2); fc.keyframe_points.foreach_get("co", co)
            co[1::2] *= 100.0
            fc.keyframe_points.foreach_set("co", co); fc.update()
sc.unit_settings.system = "METRIC"; sc.unit_settings.scale_length = 0.01
rb = rarm.data.bones["root"]
log("root rest matrix_local", [[round(x, 6) for x in row] for row in rb.matrix_local])

for o in bpy.context.selected_objects:
    o.select_set(False)
for o in (rarm, rmesh, stk):
    o.select_set(True)
bpy.context.view_layer.objects.active = rarm
EXPORT = dict(filepath=OUT_FBX, use_selection=True, object_types={"ARMATURE", "MESH"},
              axis_forward="-Z", axis_up="Y", apply_unit_scale=True, apply_scale_options="FBX_SCALE_NONE", global_scale=1.0,
              use_mesh_modifiers=False, mesh_smooth_type="FACE", add_leaf_bones=False,
              primary_bone_axis="Y", secondary_bone_axis="X", armature_nodetype="NULL", use_armature_deform_only=False,
              bake_anim=True, bake_anim_use_all_bones=False, bake_anim_use_nla_strips=False, bake_anim_use_all_actions=True,
              bake_anim_force_startend_keying=True, bake_anim_step=1.0, bake_anim_simplify_factor=0.0,
              path_mode="AUTO", embed_textures=False)
rarm.animation_data.action = None          # written from the rest pose, as S4
for pb in rarm.pose.bones:
    pb.matrix_basis = Matrix.Identity(4)
bpy.ops.export_scene.fbx(**EXPORT)
# loopcut variant (as S4's): looping takes without the repeated last frame, so a wrapping player shows no 1/60 s hold
for cname, lp in loops.items():
    if lp:
        a = bpy.data.actions[cname]
        r0, r1 = a.frame_range[0], a.frame_range[1]
        a.use_frame_range = True
        a.frame_start, a.frame_end = r0, r1 - 1
EXPORT_LC = dict(EXPORT, filepath=OUT_FBX.replace(".fbx", "_loopcut.fbx"))
bpy.ops.export_scene.fbx(**EXPORT_LC)
report["loops"] = loops
report["export"] = {k: (sorted(v) if isinstance(v, set) else v) for k, v in EXPORT.items()}
report["seconds_wall"] = round(time.time() - T0, 1)
report["bones"] = [(b.name, b.parent.name if b.parent else None) for b in rarm.data.bones]
with open(os.path.join(S5, "retarget_report.json"), "w") as fh:
    json.dump(report, fh, indent=1)
log("DONE", OUT_FBX, os.path.getsize(OUT_FBX), "bytes", report["seconds_wall"], "s")
