# Dumps what the FBX file itself says (no importer in between): GlobalSettings axes/unit, and the Lcl transforms of the
# armature null and the first bones.   blender -b --factory-startup --python fbx_nodes.py -- <file.fbx>
import sys
from io_scene_fbx import parse_fbx
path = sys.argv[sys.argv.index("--") + 1]
root, ver = parse_fbx.parse(path)
def s(x): return x.decode() if isinstance(x, bytes) else x
def props(elem):
    out = {}
    for p70 in elem.elems:
        if p70.id == b"Properties70":
            for p in p70.elems:
                out[s(p.props[0])] = [s(v) for v in p.props[4:]]
    return out
for e in root.elems:
    if e.id == b"GlobalSettings":
        g = props(e)
        print("GLOBAL", {k: g[k] for k in ("UpAxis", "UpAxisSign", "FrontAxis", "FrontAxisSign", "CoordAxis", "CoordAxisSign", "UnitScaleFactor") if k in g})
    if e.id == b"Objects":
        for m in e.elems:
            if m.id == b"Model":
                name = s(m.props[1]).split("\x00")[0]
                if name in ("Armature", "root", "pelvis", "SK_Skater_S4", "SK_Skater_S4_Stick"):
                    p = props(m)
                    print("MODEL", name, s(m.props[2]), {k: [round(v, 4) if isinstance(v, float) else v for v in p.get(k, [])] for k in ("Lcl Translation", "Lcl Rotation", "Lcl Scaling", "PreRotation")})
