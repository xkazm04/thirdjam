# blender -b --factory-startup --python extract_detect.py -- <cand>
import bpy, sys, os, pickle, json, numpy as np
sys.path.insert(0, r"C:/Users/kazda/jam/skater-lab/skh/tex")
from texlib import *
T = r"C:/Users/kazda/jam/skater-lab/skh/tex"
cand = sys.argv[sys.argv.index("--") + 1]
D = os.path.join(T, cand)
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=os.path.join(D, cand + "_rigged.fbx"))
ob = next(o for o in bpy.data.objects if o.type == "MESH")
mat = ob.material_slots[0].material
bsdf = next(n for n in mat.node_tree.nodes if n.type == "BSDF_PRINCIPLED")
lk = bsdf.inputs["Base Color"].links
node = lk[0].from_node
while node.type != "TEX_IMAGE":
    node = node.inputs[0].links[0].from_node
im = node.image
print("DET source", im.name, im.filepath, list(im.size), "packed", bool(im.packed_file))
a = img_to_np(im)[::-1].copy()   # stored values; colour space conversion is not applied by pixels for 8-bit
N = a.shape[0]
write_png(os.path.join(D, "basecolor_orig.png"), a)
me = ob.data; uv = me.uv_layers.active.data; M = ob.matrix_world
C = []; U = []
import mathutils
for p in me.polygons:
    vs = [M @ me.vertices[v].co for v in p.vertices]
    C.append(tuple(sum(vs, mathutils.Vector()) / len(vs))); U.append([tuple(uv[l].uv) for l in p.loop_indices])
C = np.array(C); pickle.dump((C, U, N), open(os.path.join(D, "faces.pkl"), "wb"))
z0, z1 = C[:, 2].min(), C[:, 2].max(); H = z1 - z0; zn = (C[:, 2] - z0) / H
bands = {"head": zn > 0.80, "torso": (zn > 0.45) & (zn <= 0.80), "upper_leg": (zn > 0.20) & (zn <= 0.45), "lower_leg": zn <= 0.20}
reg = {k: region_mask(v, U, N) for k, v in bands.items()}
np.savez_compressed(os.path.join(D, "regions.npz"), **reg)
rgb = a[..., :3]; L = rgb @ np.array([.2126, .7152, .0722], np.float32); sat = rgb.max(-1) - rgb.min(-1)
cl = closing(L, 13); diff = cl - L; bl = gblur(L, 8)
anyreg = reg["head"] | reg["torso"] | reg["upper_leg"] | reg["lower_leg"]
cand_ = {
 "A_dark_stroke_on_dark": (diff > 0.03) & (cl < 0.35) & (bl < 0.28),
 "B_black_blob_on_bright": (diff > 0.07) & (cl >= 0.35) & (L < 0.10) & (bl > 0.65),
 "C_grey_smear_on_white": (diff > 0.07) & (cl > 0.75) & (sat < 0.12) & (L < 0.8),
 "D_dark_on_mid": (diff > 0.12) & (cl >= 0.35) & (cl < 0.75) & (L < 0.25),
}
res = {"source": im.name, "size": [int(x) for x in im.size], "height_m": round(float(H), 3), "counts": {}}
flag = np.zeros((N, N), bool); clusters = []
for cn, m in cand_.items():
    m = m & anyreg
    comps = components(m, 5)
    for ys, xs in comps:
        h = ys.max() - ys.min() + 1; w = xs.max() - xs.min() + 1
        if max(h, w) > 400 or len(ys) < 12: continue   # big shapes (seams, outlines) and specks: not glyph-sized
        cy, cx = int(ys.mean()), int(xs.mean())
        rn = next((k for k in ("lower_leg", "upper_leg", "torso", "head") if reg[k][cy, cx]), "none")
        res["counts"].setdefault(rn, {}).setdefault(cn, 0); res["counts"][rn][cn] += 1
        flag[ys, xs] = True
        clusters.append({"cls": cn, "region": rn, "bbox": [int(xs.min()), int(ys.min()), int(xs.max()), int(ys.max())], "px": int(len(ys))})
res["clusters"] = clusters
json.dump(res, open(os.path.join(D, "detect.json"), "w"), indent=0)
ov = rgb * 0.55
tint = {"head": (0.3, 0.3, 0.0), "torso": (0, 0.25, 0), "upper_leg": (0, 0.15, 0.3), "lower_leg": (0.3, 0.1, 0)}
for k, c in tint.items():
    edge = reg[k] & ~dilate(~reg[k], 1) if False else None
ov[dilate(flag, 3)] = (1, 0, 1)
write_png(os.path.join(D, "overlay.png"), ov)
print("DET", cand, json.dumps({k: v for k, v in res.items() if k != "clusters"}))
