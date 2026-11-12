"""tools/p4/ue_retarget_pose.py (lane A28): a retarget-pose offset on the target's upper arms, then the walk retargeted again.
Run in the editor's Python: A28_RP = {"vendor": "Rodin", "dest": "/Game/P4/Rodin", "mesh": "/Game/P4/Rodin/SK_P4_Rodin",
    "axis": "Y"|"Z"|"none", "deg": 20, "clips": ["/Game/Characters/Mannequins/Anims/Unarmed/Walk/MF_Unarmed_Walk_Fwd"]}; exec(open(r".../ue_retarget_pose.py").read())
The offset swings upperarm_l / upperarm_r (mirrored) by <deg> about the bone's local <axis> in the target retarget pose, so the retargeted arms
start outside a big toy torso. The walk is re-retargeted over the existing MF_Unarmed_Walk_Fwd_P4_<Vendor>. The caller measures the result with a
capture (hand_l/r distance from the pelvis in the ground plane). Prints "A28 <vendor> rpose <key> <value>".
"""
import unreal, json, math
_ALIAS = globals().get("A28_ALIAS") or {}
def bn(n):
    return _ALIAS.get(n, n)

P = A28_RP; V = P["vendor"]; DEST = P["dest"]
_ALIAS = P.get("alias") or {}
lib = unreal.EditorAssetLibrary
out = {"tool": "thirdjam.p4-rpose/1", "vendor": V, "axis": P.get("axis", "none"), "deg": P.get("deg", 0)}
def say(k, v):
    print("A28 %s rpose %s %s" % (V, k, v)); out[k] = v
rtg = lib.load_asset(DEST + "/RTG_Manny_to_P4_" + V)
rc = unreal.IKRetargeterController.get_controller(rtg)
T = unreal.RetargetSourceOrTarget.TARGET
mesh = lib.load_asset(P["mesh"]); manny = lib.load_asset("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple")
axis = P.get("axis", "none"); deg = float(P.get("deg", 0))
try:
    rc.reset_retarget_pose(rc.get_current_retarget_pose_name(T), [], T)
except Exception as e:
    say("reset_err", str(e)[:120])
if axis != "none" and deg:
    for bone, sgn in ((bn("upperarm_l"), 1.0), (bn("upperarm_r"), -1.0)):
        ax = unreal.Vector(0, 1, 0) if axis == "Y" else unreal.Vector(0, 0, 1)
        h = math.radians(sgn * deg) / 2.0
        q = unreal.Quat(ax.x * math.sin(h), ax.y * math.sin(h), ax.z * math.sin(h), math.cos(h))
        try:
            rc.set_rotation_offset_for_retarget_pose_bone(bone, q, T)
            say("offset_" + bone, [axis, sgn * deg])
        except Exception as e:
            say("offset_err_" + bone, str(e)[:160])
lib.save_asset(DEST + "/RTG_Manny_to_P4_" + V, only_if_is_dirty=False)
clips = [lib.find_asset_data(p) for p in P.get("clips", [])]
clips = [c for c in clips if c.is_valid()]
for c in clips:   # the batch never overwrites: delete the previous result so the name is reused (no _P4_<V>1, 2, ...)
    old = DEST + "/Anims/" + str(c.asset_name) + "_P4_" + V
    if lib.does_asset_exist(old): lib.delete_asset(old)
    for k in range(1, 6):
        if lib.does_asset_exist(old + str(k)): lib.delete_asset(old + str(k))
res = unreal.IKRetargetBatchOperation.duplicate_and_retarget(clips, manny, mesh, rtg, prefix="", suffix="_P4_" + V, target_path=DEST + "/Anims")
paths = [str(a.package_name) for a in res]
for p in paths:
    a = lib.load_asset(p)
    if a:
        try: a.set_editor_property("enable_root_motion", False)
        except Exception: pass
say("retargeted", paths)
lib.save_directory(DEST + "/Anims", only_if_is_dirty=False)
print(json.dumps(out))
