# Rclay: clay base colour from the S6 clean PNG + a 1024 tangent-space thumb-dent normal map computed on R's rest mesh.
#   blender -b --factory-startup --python clay_r.py
# Height field (object space, metres): h(p) = -sum_c D_c * exp(-0.5 * q_c(p - P_c)), a jittered grid of dent centres (cell S),
# one dent per cell, each an oval Gaussian (radii RA x RB, random orientation, random depth), evaluated over the 27 neighbour cells.
# Normal: finite differences of h along the per-texel MikkTSpace tangent and bitangent; written as DirectX (green = -dh/dB) for UE,
# and an OpenGL copy for Blender previews.
import bpy, sys, os, numpy as np, mathutils
sys.path.insert(0, r"C:/Users/kazda/jam/skater-lab/skh/tex")
from texlib import *
T = r"C:/Users/kazda/jam/skater-lab/skh/tex"; E = r"C:/Users/kazda/jam/skater-lab/skh/export/Rclay"
N = 1024; S = 0.022; RA = (0.010, 0.016); RB = (0.006, 0.010); DEPTH = (0.0011, 0.0025); EPS = 0.002; SEED = 7
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=os.path.join(T, "R", "R_rigged.fbx"))
ob = next(o for o in bpy.data.objects if o.type == "MESH")
me = ob.data; me.calc_tangents(); me.calc_loop_triangles()
M = ob.matrix_world; R3 = M.to_3x3()
co = np.array([tuple(M @ v.co) for v in me.vertices])
nl = len(me.loops)
ln = np.zeros(nl * 3); me.loops.foreach_get("normal", ln); ln = ln.reshape(-1, 3) @ np.array(R3).T
lt = np.zeros(nl * 3); me.loops.foreach_get("tangent", lt); lt = lt.reshape(-1, 3) @ np.array(R3).T
ls = np.zeros(nl); me.loops.foreach_get("bitangent_sign", ls)
lv = np.zeros(nl, int); me.loops.foreach_get("vertex_index", lv)
uv = np.zeros(nl * 2); me.uv_layers.active.data.foreach_get("uv", uv); uv = uv.reshape(-1, 2)
# rasterise every loop triangle: per texel barycentric -> position, normal, tangent, sign
P = np.zeros((N, N, 3)); Nn = np.zeros((N, N, 3)); Tt = np.zeros((N, N, 3)); Sg = np.zeros((N, N)); cov = np.zeros((N, N), bool)
for tri in me.loop_triangles:
    L = list(tri.loops); p = uv[L] * N; p[:, 1] = N - p[:, 1]
    x0, y0 = np.floor(p.min(0)).astype(int); x1, y1 = np.ceil(p.max(0)).astype(int)
    x0 = max(x0, 0); y0 = max(y0, 0); x1 = min(x1, N - 1); y1 = min(y1, N - 1)
    if x1 < x0 or y1 < y0: continue
    xs, ys = np.meshgrid(np.arange(x0, x1 + 1) + .5, np.arange(y0, y1 + 1) + .5)
    d = (p[1, 1] - p[2, 1]) * (p[0, 0] - p[2, 0]) + (p[2, 0] - p[1, 0]) * (p[0, 1] - p[2, 1])
    if abs(d) < 1e-12: continue
    a = ((p[1, 1] - p[2, 1]) * (xs - p[2, 0]) + (p[2, 0] - p[1, 0]) * (ys - p[2, 1])) / d
    b = ((p[2, 1] - p[0, 1]) * (xs - p[2, 0]) + (p[0, 0] - p[2, 0]) * (ys - p[2, 1])) / d
    c = 1 - a - b; e = -0.02; ins = (a >= e) & (b >= e) & (c >= e)
    if not ins.any(): continue
    yy = ys[ins].astype(int); xx = xs[ins].astype(int); w = np.stack([a[ins], b[ins], c[ins]], -1)
    P[yy, xx] = w @ co[lv[L]]; Nn[yy, xx] = w @ ln[L]; Tt[yy, xx] = w @ lt[L]; Sg[yy, xx] = ls[L[0]]; cov[yy, xx] = True
idx = np.nonzero(cov); p = P[idx]; n = Nn[idx]; n /= np.linalg.norm(n, axis=1, keepdims=True)
t = Tt[idx]; t = t - n * (t * n).sum(1, keepdims=True); t /= np.maximum(np.linalg.norm(t, axis=1, keepdims=True), 1e-9)
bt = np.cross(n, t) * Sg[idx][:, None]
print("CLAY texels", len(p))
def rnd(ci, k):   # hash of integer cell coords -> uniform [0,1)
    h = (ci[:, 0] * 73856093) ^ (ci[:, 1] * 19349663) ^ (ci[:, 2] * 83492791) ^ (k * 2654435761 + SEED * 97)
    h = (h ^ (h >> 13)) * 1274126177; h = h ^ (h >> 16)
    return (h & 0xFFFFFF) / float(0x1000000)
def height(q):
    base = np.floor(q / S).astype(np.int64); h = np.zeros(len(q))
    for dx in (-1, 0, 1):
        for dy in (-1, 0, 1):
            for dz in (-1, 0, 1):
                ci = base + np.array([dx, dy, dz])
                c = (ci + np.stack([rnd(ci, 1), rnd(ci, 2), rnd(ci, 3)], -1)) * S
                v = q - c
                # oval: random unit axis u; the distance along u scaled by 1/RA, across by 1/RB
                th = rnd(ci, 4) * 2 * np.pi; ph = np.arccos(2 * rnd(ci, 5) - 1)
                u = np.stack([np.sin(ph) * np.cos(th), np.sin(ph) * np.sin(th), np.cos(ph)], -1)
                ra = RA[0] + (RA[1] - RA[0]) * rnd(ci, 6); rb = RB[0] + (RB[1] - RB[0]) * rnd(ci, 7)
                dep = DEPTH[0] + (DEPTH[1] - DEPTH[0]) * rnd(ci, 8)
                al = (v * u).sum(1); pe2 = (v * v).sum(1) - al * al
                h -= dep * np.exp(-0.5 * (al * al / (ra * ra) + pe2 / (rb * rb)))
    return h
dt = (height(p + t * EPS) - height(p - t * EPS)) / (2 * EPS)
db = (height(p + bt * EPS) - height(p - bt * EPS)) / (2 * EPS)
nt = np.stack([-dt, -db, np.ones_like(dt)], -1); nt /= np.linalg.norm(nt, axis=1, keepdims=True)
print("CLAY slope deg p50/p99", np.degrees(np.arccos(nt[:, 2])).round(2)[np.argsort(nt[:, 2])][[len(nt) // 2, len(nt) // 100]])
img = np.zeros((N, N, 3)); img[..., 2] = 1.0; img[idx] = nt
# pad islands outward 6 px (average of covered neighbours), then neutral
filled = cov.copy()
for _ in range(6):
    acc = np.zeros_like(img); cnt = np.zeros((N, N))
    for sy, sx in ((1, 0), (-1, 0), (0, 1), (0, -1)):
        acc += np.roll(np.roll(img * filled[..., None], sy, 0), sx, 1); cnt += np.roll(np.roll(filled, sy, 0), sx, 1)
    new = (~filled) & (cnt > 0); img[new] = acc[new] / cnt[new][:, None]; filled |= new
img /= np.linalg.norm(img, axis=-1, keepdims=True)
gl = img * 0.5 + 0.5
dx_ = gl.copy(); dx_[..., 1] = 1.0 - dx_[..., 1]
os.makedirs(E, exist_ok=True)
write_png(os.path.join(E, "T_Skater_Rclay_Normal.png"), dx_)          # DirectX (green down), for UE
write_png(os.path.join(T, "Rclay_normal_gl.png"), gl)                  # OpenGL, for Blender previews
# clay base colour
a = read_raw(bpy, r"C:/Users/kazda/jam/skater-lab/s6/T_SkaterTB2_BaseColor_clean.png")[..., :3].astype(np.float64)
soft = gblur(a, 1.6); c = 0.5 * a + 0.5 * soft
g = (c @ np.array([.2126, .7152, .0722]))[..., None]; c = g + (c - g) * 0.85
c = 0.045 + c * 0.95
write_png(os.path.join(E, "T_Skater_Rclay_BaseColor.png"), c)
print("CLAY done")
