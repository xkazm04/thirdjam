# Remove light glyph-like marks (wordmark) from gB's helmet shell. blender -b --factory-startup --python clean_gB.py -- <cand> <zmin_frac>
import bpy, sys, os, pickle, json, numpy as np
sys.path.insert(0, r"C:/Users/kazda/jam/skater-lab/skh/tex")
from texlib import *
T = r"C:/Users/kazda/jam/skater-lab/skh/tex"
cand, zmin = sys.argv[sys.argv.index("--") + 1:][:2]; zmin = float(zmin)
D = os.path.join(T, cand)
a = read_raw(bpy, os.path.join(D, "basecolor_orig.png")); rgb = a[..., :3].copy(); N = rgb.shape[0]
C, U, _ = pickle.load(open(os.path.join(D, "faces.pkl"), "rb"))
zn = (C[:, 2] - C[:, 2].min()) / (C[:, 2].max() - C[:, 2].min())
uvc = np.array([np.mean(u, 0) for u in U]); px = np.clip((uvc[:, 0] * N).astype(int), 0, N - 1); py = np.clip(((1 - uvc[:, 1]) * N).astype(int), 0, N - 1)
fc = rgb[py, px]; shell = (fc[:, 2] > fc[:, 0] + 0.08) & (fc @ np.array([.2126, .7152, .0722]) < 0.35)
region = region_mask((zn > zmin) & shell, U, N)
print("CLEAN shell faces", int(((zn > zmin) & shell).sum()), "of", int((zn > zmin).sum()))
L = rgb @ np.array([.2126, .7152, .0722], np.float32); sat = rgb.max(-1) - rgb.min(-1)
k = 9
op = shift_max(shift_max(shift_min(shift_min(L, k, 0), k, 1), k, 0), k, 1)
th = L - op
cand_ = (th > 0.10) & (op < 0.30) & region            # light stroke on a dark (navy) surround
cl = closing(L, k); cand_ |= ((cl - L) > 0.10) & (cl < 0.35) & region   # dark stroke on navy
keep = np.zeros_like(cand_); info = []
for ys, xs in components(cand_, 4):
    h = ys.max() - ys.min() + 1; w = xs.max() - xs.min() + 1
    if max(h, w) <= 260 and len(ys) >= 6:
        keep[ys, xs] = True; info.append([int(xs.min()), int(ys.min()), int(xs.max()), int(ys.max()), int(len(ys))])
print("CLEAN clusters", len(info), info[:40])
mask = dilate(keep, 4) & region
# hand-picked wordmark boxes (x0,y0,x1,y1), found by looking at the helmet front (z_helmet_front.png, crops.png)
navy0 = (rgb[..., 2] > rgb[..., 0] + 0.08) & (L < 0.35)
navyfrac = gblur(navy0.astype(np.float32), 6)
med = np.median(rgb[navy0 & region], 0)
for x0, y0, x1, y1 in ((1170, 185, 1235, 240), (128, 938, 172, 1032), (180, 930, 220, 990)):
    box = np.zeros_like(mask); box[y0:y1 + 1, x0:x1 + 1] = True
    off = np.linalg.norm(rgb - med, axis=-1) > 0.10
    mask |= dilate(box & off & (navyfrac > 0.2), 3) & box
print("CLEAN navy median", med)
navy = (rgb[..., 2] > rgb[..., 0] + 0.08) & (L < 0.35)
known = (navy & ~mask).astype(np.float32)[..., None]
res = rgb.copy(); todo = mask.copy()
for s in (5, 14, 40):
    num = gblur(rgb * known, s); den = gblur(known, s); ok = todo & (den[..., 0] > 0.15)
    res[ok] = (num / np.maximum(den, 1e-4))[ok]; todo &= ~ok
out = np.clip(res, 0, 1)
changed = np.abs(out - rgb).max(-1) > 1.5 / 255
print("CLEAN masked", int(mask.sum()), "changed", int(changed.sum()), "outside mask", int((changed & ~mask).sum()), "unfilled", int(todo.sum()))
write_png(os.path.join(D, "basecolor_clean.png"), out)
ov = rgb * 0.5; ov[mask] = (1, 0, 1); write_png(os.path.join(D, "clean_overlay.png"), ov)
json.dump({"clusters": info, "masked": int(mask.sum()), "changed": int(changed.sum()), "outside": int((changed & ~mask).sum())}, open(os.path.join(D, "clean.json"), "w"))
