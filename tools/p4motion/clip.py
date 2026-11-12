"""tools/p4motion/clip.py: Blender headless clip prep for lane A29 (P4 motion).

  blender -b --factory-startup --python tools/p4motion/clip.py -- --in <clip.fbx|bvh|glb> --summary <out.json>
          [--out-fbx <stripped.fbx>] [--name <clip>] [--trim <first,last>] [--loop-blend <n>] [--no-strip] [--keep-mesh-textures] [--zup] [--normalize [prerot_deg]]

What it does, in order:
  1. imports the clip (FBX, BVH or GLB) and finds the armature, its skeleton profile (meshy-reference, ue5-manny, mixamo/rokoko) and its root bone;
  2. writes the per-frame world positions (metres, Blender Z-up) of pelvis, both feet (ankle and toe), head and both hands BEFORE any edit ("raw");
  3. optional --trim first,last: keeps only that frame range (the loop cut the gate suggested);
  4. optional --loop-blend n: blends the last n frames of every curve toward the first frame so the loop closes;
  5. strips root motion: the horizontal (X, Y) world travel of the root bone (and of the armature object) is removed frame by frame; Z (the bob, the fall) stays.
     The sim owns displacement (contracts/skater-visual.md); every clip that enters a build is in place;
  6. writes the same positions AFTER the edits ("stripped"), with the root travel before/after, to --summary;
  7. optional --normalize [prerot]: lane A28's normalize_fbx.py steps (pass 3): bake the mesh parent-inverse and the armature's object transform (the vendor's
     100x / 90-degree Armature node) into the bones, centimetre scene with scale 100 applied, optional pre-rotation about X, the rest skeleton lifted so its lowest
     bone sits on z 0 (the vendor rigs bind with the pelvis at the origin, which made the IK Retargeter scale the pelvis motion by 100), the root bone's pose
     re-keyed so the animation stays where it was, armature named root;
  8. optional --out-fbx: exports the armature + mesh with the baked edited animation, textures dropped; --zup writes Z-up / Y-forward like A28's normalizer.
The summary is what tools/p4motion/gate.mjs reads. Frames are reported at the clip's own fps; the gate converts to seconds. Positions are metres whatever the scene unit.
Last stdout line: {"tool":"thirdjam.p4motion-clip/1", ...}
"""
import bpy, sys, json, os, math

PROFILES = {
    # name: (root candidates, pelvis, feet (ankle, toe) left/right, head, hands left/right)
    "meshy-reference": (["Pelvis"], "Pelvis", (("L_Ankle", "L_Foot"), ("R_Ankle", "R_Foot")), "Head", ("L_Wrist", "R_Wrist")),
    "ue5-manny": (["root", "pelvis"], "pelvis", (("foot_l", "ball_l"), ("foot_r", "ball_r")), "head", ("hand_l", "hand_r")),
    "mixamo": (["mixamorig:Hips"], "mixamorig:Hips", (("mixamorig:LeftFoot", "mixamorig:LeftToeBase"), ("mixamorig:RightFoot", "mixamorig:RightToeBase")),
               "mixamorig:Head", ("mixamorig:LeftHand", "mixamorig:RightHand")),
    "rokoko-hips": (["Root", "Hips"], "Hips", (("LeftFoot", "LeftToeBase"), ("RightFoot", "RightToeBase")), "Head", ("LeftHand", "RightHand")),
}


def arg(name, default=None):
    a = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    if name in a:
        i = a.index(name)
        nxt = a[i + 1] if i + 1 < len(a) else None
        return nxt if (nxt is not None and not nxt.startswith("--")) else True
    return default


def unit():
    return bpy.context.scene.unit_settings.scale_length or 1.0


def load(path):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    ext = os.path.splitext(path)[1].lower()
    if ext == ".fbx":
        bpy.ops.import_scene.fbx(filepath=path, automatic_bone_orientation=False, use_anim=True)
    elif ext == ".bvh":
        bpy.ops.import_anim.bvh(filepath=path, update_scene_fps=True, update_scene_duration=True, axis_forward='-Z', axis_up='Y')
    elif ext in (".glb", ".gltf"):
        bpy.ops.import_scene.gltf(filepath=path)
    else:
        raise SystemExit("unsupported " + ext)


def detect_profile(arm):
    names = {b.name for b in arm.data.bones}
    for prof, (roots, pelvis, feet, head, hands) in PROFILES.items():
        if pelvis in names and feet[0][0] in names:
            return prof
    return "unknown"


def world_pos(arm, bone, frame):
    bpy.context.scene.frame_set(frame)
    pb = arm.pose.bones.get(bone)
    if pb is None:
        return None
    v = (arm.matrix_world @ pb.head) * unit()   # metres whatever the scene unit
    return [round(v.x, 4), round(v.y, 4), round(v.z, 4)]


def sample(arm, prof, f0, f1):
    roots, pelvis, feet, head, hands = PROFILES.get(prof, PROFILES["meshy-reference"])
    tracked = {"pelvis": pelvis, "foot_l": feet[0][0], "toe_l": feet[0][1], "foot_r": feet[1][0], "toe_r": feet[1][1],
               "head": head, "hand_l": hands[0], "hand_r": hands[1]}
    frames = []
    for f in range(f0, f1 + 1):
        rec = {"f": f}
        for k, bn in tracked.items():
            p = world_pos(arm, bn, f)
            if p is not None:
                rec[k] = p
        frames.append(rec)
    return tracked, frames


def action_of(arm):
    if arm.animation_data and arm.animation_data.action:
        return arm.animation_data.action
    acts = list(bpy.data.actions)
    if acts:
        arm.animation_data_create()
        arm.animation_data.action = acts[0]
        return acts[0]
    return None


def trim(action, first, last):
    """Keep keys in [first,last] and move them so the clip starts at frame 1."""
    for fc in action.fcurves:
        pts = fc.keyframe_points
        for i in range(len(pts) - 1, -1, -1):
            x = pts[i].co.x
            if x < first - 0.5 or x > last + 0.5:
                pts.remove(pts[i])
        for p in pts:
            p.co.x -= (first - 1)
            p.handle_left.x -= (first - 1)
            p.handle_right.x -= (first - 1)
        fc.update()
    scn = bpy.context.scene
    scn.frame_start, scn.frame_end = 1, last - first + 1


def loop_blend(action, n, f0, f1):
    for fc in action.fcurves:
        v0 = fc.evaluate(f0)
        for f in range(f1 - n, f1 + 1):
            t = (f - (f1 - n)) / float(n)
            v = fc.evaluate(f) * (1.0 - t) + v0 * t
            fc.keyframe_points.insert(f, v, options={'REPLACE'})
        fc.update()


def strip_root(arm, root_name, f0, f1):
    """Remove the horizontal world travel of the root bone (and the object), frame by frame, keeping Z."""
    scn = bpy.context.scene
    obj_anim = arm.animation_data and arm.animation_data.action and any(fc.data_path == "location" for fc in arm.animation_data.action.fcurves)
    pb = arm.pose.bones[root_name]
    scn.frame_set(f0)
    m0 = (arm.matrix_world @ pb.matrix).translation.copy()
    before = None
    u = unit()
    for f in range(f0, f1 + 1):
        scn.frame_set(f)
        world = arm.matrix_world @ pb.matrix
        if f == f1:
            before = [round((world.translation.x - m0.x) * u, 4), round((world.translation.y - m0.y) * u, 4), round((world.translation.z - m0.z) * u, 4)]
        world.translation.x = m0.x
        world.translation.y = m0.y
        pb.matrix = arm.matrix_world.inverted() @ world
        bpy.context.view_layer.update()
        pb.keyframe_insert("location", frame=f)
    if obj_anim:
        for fc in list(arm.animation_data.action.fcurves):
            if fc.data_path == "location":
                arm.animation_data.action.fcurves.remove(fc)
        arm.location = (0, 0, 0)
    return before


def normalize(arm, root_name, f0, f1, prerot, out):
    """Lane A28's normalize_fbx.py steps plus the ground lift, after the strip/trim/blend happened on the vendor's own scene."""
    import mathutils
    scn = bpy.context.scene
    meshes = [o for o in bpy.data.objects if o.type == "MESH"]
    pb = arm.pose.bones[root_name]
    # the root bone's world pose per frame, recorded now in metres: transform_apply scales the rest bones but not the location keys, so it is written back after
    recorded = []
    for f in range(f0, f1 + 1):
        scn.frame_set(f)
        recorded.append((arm.matrix_world @ pb.matrix).copy())
    bpy.ops.object.select_all(action="DESELECT")
    for o in [arm] + meshes:
        o.select_set(True)
    bpy.context.view_layer.objects.active = arm
    for o in meshes:
        mw = o.matrix_world.copy(); o.matrix_parent_inverse = mathutils.Matrix.Identity(4); o.matrix_world = mw
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    scn.unit_settings.system = "METRIC"; scn.unit_settings.scale_length = 0.01; scn.unit_settings.length_unit = "CENTIMETERS"
    arm.scale = (100.0, 100.0, 100.0)
    for o in meshes:
        if o.parent is not arm:
            o.scale = (100.0, 100.0, 100.0)
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    if prerot:
        arm.rotation_euler = (math.radians(prerot), 0.0, 0.0)
        bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    # recorded poses to centimetres (translation x100, rotation kept)
    rec_cm = []
    for m in recorded:
        mm = m.copy()
        mm.translation = m.translation * 100.0
        rec_cm.append(mm)
    # the vendor rigs bind with the pelvis at the origin and the feet below it: lift the rest skeleton and every mesh so the lowest bone sits on z 0
    zmin = min(min((arm.matrix_world @ b.head_local).z, (arm.matrix_world @ b.tail_local).z) for b in arm.data.bones)
    lift = -zmin
    if abs(lift) > 0.5:
        bpy.ops.object.select_all(action="DESELECT")
        for o in [arm] + meshes:
            o.select_set(True)
        bpy.context.view_layer.objects.active = arm
        arm.location.z += lift
        for o in meshes:
            if o.parent is not arm:
                o.location.z += lift
        bpy.ops.object.transform_apply(location=True, rotation=False, scale=False)   # armature and every mesh move together
        for m in rec_cm:
            m.translation.z += lift
    for f, m in zip(range(f0, f1 + 1), rec_cm):
        scn.frame_set(f)
        pb.matrix = arm.matrix_world.inverted() @ m
        bpy.context.view_layer.update()
        pb.keyframe_insert("location", frame=f)
        pb.keyframe_insert("rotation_quaternion", frame=f)
    arm.name = "root"; arm.data.name = "root"
    scn.frame_set(f0)
    out["normalized"] = {"prerot": prerot, "rest_lift_cm": round(lift, 1), "pelvis_z_cm_rest": round((arm.matrix_world @ arm.data.bones[root_name].head_local).z, 1),
                         "pelvis_z_cm_t0": round((arm.matrix_world @ pb.matrix).translation.z, 1)}


def main():
    src = arg("--in")
    if not src:
        raise SystemExit("--in required")
    out_sum = arg("--summary")
    out_fbx = arg("--out-fbx")
    name = arg("--name", os.path.splitext(os.path.basename(src))[0])
    load(src)
    arms = [o for o in bpy.data.objects if o.type == "ARMATURE"]
    if not arms:
        raise SystemExit("no armature in " + src)
    arm = arms[0]
    bpy.context.view_layer.objects.active = arm
    prof = detect_profile(arm)
    act = action_of(arm)
    if act is None:
        raise SystemExit("no action in " + src)
    f0, f1 = [int(round(x)) for x in act.frame_range]
    scn = bpy.context.scene
    scn.frame_start, scn.frame_end = f0, f1
    fps = scn.render.fps / scn.render.fps_base
    roots = [b.name for b in arm.data.bones if b.parent is None]
    root_name = roots[0]
    cands = PROFILES.get(prof, (roots,))[0]
    for c in cands:
        if c in arm.data.bones:
            root_name = c
            break
    out = {"tool": "thirdjam.p4motion-clip/1", "clip": name, "source": os.path.basename(src), "bytes": os.path.getsize(src),
           "profile": prof, "armature": arm.name, "bone_count": len(arm.data.bones), "bones": [b.name for b in arm.data.bones],
           "root_bone": root_name, "fps": fps, "scene_unit_scale": unit(), "frames_raw": [f0, f1], "action": act.name}
    tracked, raw = sample(arm, prof, f0, f1)
    out["tracked"] = tracked
    out["raw"] = raw

    t = arg("--trim")
    if t:
        a, b = [int(x) for x in str(t).split(",")]
        trim(act, a, b)
        f0, f1 = 1, b - a + 1
        out["trim"] = [a, b]
    lb = arg("--loop-blend")
    if lb:
        loop_blend(act, int(lb), f0, f1)
        out["loop_blend"] = int(lb)
    if arg("--no-strip") is None:
        out["root_travel_before_m"] = strip_root(arm, root_name, f0, f1)
    out["frames"] = [f0, f1]
    _, stripped = sample(arm, prof, f0, f1)
    out["stripped"] = stripped
    p0, p1 = stripped[0].get("pelvis"), stripped[-1].get("pelvis")
    if p0 and p1:
        out["root_travel_after_m"] = [round(p1[i] - p0[i], 4) for i in range(3)]

    nz = arg("--normalize")
    if out_fbx and nz is not None:
        normalize(arm, root_name, f0, f1, float(nz) if nz is not True else 0.0, out)
    if out_fbx:
        if arg("--keep-mesh-textures") is None:
            for img in list(bpy.data.images):
                bpy.data.images.remove(img)
        os.makedirs(os.path.dirname(os.path.abspath(out_fbx)), exist_ok=True)
        zup = arg("--zup") is not None
        bpy.ops.export_scene.fbx(filepath=out_fbx, use_selection=False, object_types={'ARMATURE', 'MESH'}, add_leaf_bones=False,
                                 bake_anim=True, bake_anim_use_all_bones=True, bake_anim_use_nla_strips=False, bake_anim_use_all_actions=False,
                                 bake_anim_force_startend_keying=True, bake_anim_step=1.0, bake_anim_simplify_factor=0.0,
                                 path_mode='STRIP', embed_textures=False, mesh_smooth_type='OFF', use_mesh_modifiers=True,
                                 armature_nodetype='NULL', primary_bone_axis='Y', secondary_bone_axis='X',
                                 apply_unit_scale=True, apply_scale_options='FBX_SCALE_NONE',
                                 axis_forward=('Y' if zup else '-Z'), axis_up=('Z' if zup else 'Y'))
        out["out_fbx"] = os.path.abspath(out_fbx)
        out["out_fbx_bytes"] = os.path.getsize(out_fbx)
    if out_sum:
        os.makedirs(os.path.dirname(os.path.abspath(out_sum)), exist_ok=True)
        with open(out_sum, "w") as fh:
            json.dump(out, fh)
        out_line = {k: v for k, v in out.items() if k not in ("raw", "stripped", "bones")}
        out_line["summary"] = os.path.abspath(out_sum)
    else:
        out_line = {k: v for k, v in out.items() if k not in ("raw", "stripped", "bones")}
    print(json.dumps(out_line))


main()
