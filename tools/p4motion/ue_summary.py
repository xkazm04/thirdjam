"""tools/p4motion/ue_summary.py: writes the gate's clip summary (same shape as clip.py's) from AnimSequences already in the editor, on the Manny
skeleton, and optionally makes in-place duplicates of engine clips first. Runs inside the editor's Python (MCP port 8016). Arguments from P4M_ARGS:

  {"out_dir": "C:/.../work", "fps": 60,
   "duplicates": [{"from": "/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_02", "to": "/Game/P4/Anim/AS_P4_check_manny"}],
   "clips": ["/Game/P4/Anim/AS_P4_check_manny", "/Game/P4/Anim/AS_P4_skate_fwd"]}

A duplicate gets `force_root_lock` on and `enable_root_motion` off (the sim owns displacement; the root track is left in the asset but never applied).
The summary samples the component-space position of pelvis, feet, balls, head and hands at every frame by walking the parent chain of local bone poses
(AnimationLibrary.get_bone_poses_for_time), in metres, with the root kept at the origin, so "raw" and "stripped" are the same frames and the root travel is
read from the root bone's own track. Prints P4M_RESULT {...} and returns json.
"""
import unreal, json, os

A = json.load(open(os.environ["P4M_ARGS"]))
lib = unreal.EditorAssetLibrary
out = {"tool": "thirdjam.p4motion-ue-summary/1", "clips": {}, "duplicates": []}
TRACK = {"pelvis": "pelvis", "foot_l": "foot_l", "toe_l": "ball_l", "foot_r": "foot_r", "toe_r": "ball_r", "head": "head", "hand_l": "hand_l", "hand_r": "hand_r"}

for d in A.get("duplicates", []):
    if lib.does_asset_exist(d["to"]):
        lib.delete_asset(d["to"])
    ok = lib.duplicate_asset(d["from"], d["to"]) is not None
    if ok:
        a = lib.load_asset(d["to"])
        a.set_editor_property("force_root_lock", True)
        a.set_editor_property("enable_root_motion", False)
        lib.save_asset(d["to"], only_if_is_dirty=False)
    out["duplicates"].append({"to": d["to"], "ok": ok})


def chains(skel_mesh):
    comp = unreal.SkeletalMeshComponent()
    comp.set_skeletal_mesh_asset(skel_mesh)
    names = [str(comp.get_bone_name(i)) for i in range(comp.get_num_bones())]
    parent = {n: str(comp.get_parent_bone(n)) for n in names}
    need = set()
    for b in TRACK.values():
        x = b
        while x and x != "None":
            need.add(x)
            x = parent.get(x, "None")
    order = [n for n in names if n in need]   # parents before children
    return order, parent


for path in A["clips"]:
    anim = lib.load_asset(path)
    name = path.split("/")[-1]
    if anim is None:
        out["clips"][name] = {"error": "missing"}
        continue
    mesh = lib.load_asset(A.get("mesh", "/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple"))
    order, parent = chains(mesh)
    length = unreal.AnimationLibrary.get_sequence_length(anim)
    fps = float(A.get("fps", 60))
    n = int(round(length * fps)) + 1
    frames = []
    root_first = None
    root_last = None
    for k in range(n):
        t = min(k / fps, length)
        locals_ = unreal.AnimationLibrary.get_bone_poses_for_time(anim, order, t, True)   # extract root motion: the root's own track is read
        comp_space = {}
        for b, tr in zip(order, locals_):
            p = parent.get(b, "None")
            if p == "None" or p not in comp_space:
                comp_space[b] = tr if b != "root" else unreal.Transform()   # root held at the origin: the clip is judged in place
                if b == "root":
                    if root_first is None:
                        root_first = tr.translation
                    root_last = tr.translation
            else:
                comp_space[b] = tr * comp_space[p]
        rec = {"f": k + 1}
        for key, b in TRACK.items():
            if b in comp_space:
                v = comp_space[b].translation
                rec[key] = [round(v.x / 100.0, 4), round(v.y / 100.0, 4), round(v.z / 100.0, 4)]
        frames.append(rec)
    travel = [0.0, 0.0, 0.0]
    if root_first is not None and root_last is not None:
        travel = [round((root_last.x - root_first.x) / 100.0, 4), round((root_last.y - root_first.y) / 100.0, 4), round((root_last.z - root_first.z) / 100.0, 4)]
    summ = {"tool": "thirdjam.p4motion-clip/1", "clip": name, "source": path, "profile": "ue5-manny", "armature": str(mesh.get_name()), "bone_count": len(order),
            "root_bone": "root", "fps": fps, "frames_raw": [1, n], "frames": [1, n], "tracked": TRACK, "raw": frames, "stripped": frames,
            "root_travel_before_m": travel, "root_travel_after_m": [0.0, 0.0, 0.0], "root_locked": True,
            "note": "sampled in the editor from the Manny clip; root held at the origin (force_root_lock), travel read from the root track"}
    f = os.path.join(A["out_dir"], name + ".ue.summary.json")
    os.makedirs(A["out_dir"], exist_ok=True)
    with open(f, "w") as fh:
        json.dump(summ, fh)
    out["clips"][name] = {"summary": f, "frames": n, "seconds": round(length, 4), "root_travel_m": travel}
out["ok"] = True
print("P4M_RESULT " + json.dumps(out))
json.dumps(out)
