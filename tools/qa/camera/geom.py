# Lane A214 (track U1): the geometry of the game camera, the way the eye counts it: the share of the 1280x720 picture that is ice inside the boards'
# inner line (rays through a grid of the frame onto z = 0), the pitch, where a world point lands in the frame. Mirrors UJamFollowCamera::IceShare and
# FrameFraction (UE axes: x forward, y right, z up; yaw about z, pitch positive up).
import math

def inside(x, y, L, W, R, margin=0.0):
    hx, hy = L / 2 - margin, W / 2 - margin
    r = max(0.0, min(R - margin, hx, hy)) if R > margin else 0.0
    if abs(x) > hx or abs(y) > hy: return False
    if r > 0 and abs(x) > hx - r and abs(y) > hy - r:
        cx, cy = hx - r, hy - r
        return (abs(x) - cx) ** 2 + (abs(y) - cy) ** 2 <= r * r + 1e-9
    return True

def basis(yaw, pitch):
    cy, sy = math.cos(math.radians(yaw)), math.sin(math.radians(yaw))
    cp, sp = math.cos(math.radians(pitch)), math.sin(math.radians(pitch))
    fwd = (cp * cy, cp * sy, sp)
    right = (-sy, cy, 0.0)
    up = (-sp * cy, -sp * sy, cp)
    return fwd, right, up

def ice_share(cam, yaw, pitch, L, W, R, fov=70.0, aspect=16 / 9, cols=64, rows=36):
    th = math.tan(math.radians(fov / 2)); tv = th / aspect
    f, r, u = basis(yaw, pitch)
    hit = 0
    for j in range(rows):
        v = 1 - (j + 0.5) / rows * 2
        for i in range(cols):
            uu = (i + 0.5) / cols * 2 - 1
            d = [f[k] + r[k] * uu * th + u[k] * v * tv for k in range(3)]
            if d[2] >= -1e-6: continue
            t = -cam[2] / d[2]
            x, y = cam[0] + d[0] * t, cam[1] + d[1] * t
            if inside(x, y, L, W, R): hit += 1
    return hit / (cols * rows)

def frac(cam, yaw, pitch, p, fov=70.0, aspect=16 / 9):
    f, r, u = basis(yaw, pitch)
    d = [p[k] - cam[k] for k in range(3)]
    x = sum(d[k] * f[k] for k in range(3)); y = sum(d[k] * r[k] for k in range(3)); z = sum(d[k] * u[k] for k in range(3))
    if x <= 1e-3: return (9.0, 9.0)
    th = math.tan(math.radians(fov / 2)); tv = th / aspect
    return (y / x / th, z / x / tv)
