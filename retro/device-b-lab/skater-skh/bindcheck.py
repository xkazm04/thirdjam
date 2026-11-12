# Is the skeleton and bind pose (and skinned mesh) of two exports the same, so that only the animations differ?
#   blender -b --factory-startup --python bindcheck.py -- <a.fbx> <b.fbx> <out.json>
import bpy, sys, json, hashlib
import numpy as np

a_fbx, b_fbx, out = sys.argv[sys.argv.index("--") + 1:][:3]

def grab(fbx):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=fbx, automatic_bone_orientation=False)
    arm = next(o for o in bpy.data.objects if o.type == "ARMATURE")
    if arm.animation_data:
        arm.animation_data.action = None
    bones = {b.name: (b.parent.name if b.parent else None, np.array(b.matrix_local), np.array(b.head_local), np.array(b.tail_local))
             for b in arm.data.bones}
    meshes = {}
    for o in bpy.data.objects:
        if o.type != "MESH":
            continue
        co = np.array([tuple(v.co) for v in o.data.vertices])
        gn = {g.index: g.name for g in o.vertex_groups}
        w = sorted((v.index, gn[g.group], round(g.weight, 6)) for v in o.data.vertices for g in v.groups)
        uv = np.array([tuple(l.uv) for l in o.data.uv_layers.active.data]) if o.data.uv_layers else np.zeros((0, 2))
        meshes[o.name] = {"co": co, "w": hashlib.sha256(repr(w).encode()).hexdigest(), "uv": uv,
                          "polys": hashlib.sha256(repr([tuple(p.vertices) for p in o.data.polygons]).encode()).hexdigest(),
                          "mats": [s.material.name if s.material else None for s in o.material_slots],
                          "mw": np.array(o.matrix_world)}
    return {"arm_mw": np.array(arm.matrix_world), "bones": bones, "meshes": meshes}

A = grab(a_fbx); B = grab(b_fbx)
r = {"a": a_fbx, "b": b_fbx}
r["bone_names_equal"] = sorted(A["bones"]) == sorted(B["bones"])
r["parents_equal"] = all(A["bones"][n][0] == B["bones"][n][0] for n in A["bones"] if n in B["bones"])
r["bone_rest_max_diff"] = float(max(max(np.abs(A["bones"][n][i] - B["bones"][n][i]).max() for i in (1, 2, 3)) for n in A["bones"] if n in B["bones"]))
r["armature_matrix_max_diff"] = float(np.abs(A["arm_mw"] - B["arm_mw"]).max())
r["mesh_names"] = [sorted(A["meshes"]), sorted(B["meshes"])]
r["meshes"] = {}
for n in A["meshes"]:
    if n not in B["meshes"]:
        r["meshes"][n] = "missing in b"; continue
    x, y = A["meshes"][n], B["meshes"][n]
    r["meshes"][n] = {"verts": [len(x["co"]), len(y["co"])],
                      "co_max_diff": float(np.abs(x["co"] - y["co"]).max()) if x["co"].shape == y["co"].shape else None,
                      "weights_equal": x["w"] == y["w"], "polys_equal": x["polys"] == y["polys"],
                      "uv_max_diff": (float(np.abs(x["uv"] - y["uv"]).max()) if x["uv"].size else 0.0) if x["uv"].shape == y["uv"].shape else None,
                      "materials": [x["mats"], y["mats"]], "object_matrix_max_diff": float(np.abs(x["mw"] - y["mw"]).max())}
json.dump(r, open(out, "w"), indent=1)
print("BIND", json.dumps(r))
