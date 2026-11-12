# Removes the animation tracks of `root` and of the `Armature` null from every take of a binary FBX, in place.
# The Blender exporter writes a T/R/S curve node for every bone and for the armature object in every take, with no option
# to leave a bone out; this rewrites the file without those curve nodes and their curves (everything else byte-for-byte as parsed).
#   blender -b --factory-startup --python strip_tracks.py -- <file.fbx> [model names...]   (default: root Armature)
import sys
from io_scene_fbx import parse_fbx, encode_bin
from io_scene_fbx.parse_fbx import FBXElem as PElem

args = sys.argv[sys.argv.index("--") + 1:]
path = args[0]
drop_models = set((args[1:] or ["root", "Armature"]))
root, version = parse_fbx.parse(path)

def name_of(e):
    return e.props[1].decode(errors="ignore").split("\x00")[0]

models, cnode_ids, curve_ids = {}, set(), set()
objs = next(e for e in root.elems if e.id == b"Objects")
conns = next(e for e in root.elems if e.id == b"Connections")
for m in objs.elems:
    if m.id == b"Model":
        models[m.props[0]] = name_of(m)
    elif m.id == b"AnimationCurveNode":
        cnode_ids.add(m.props[0])
    elif m.id == b"AnimationCurve":
        curve_ids.add(m.props[0])
drop_ids = {i for i, n in models.items() if n in drop_models}
dead_nodes = {c.props[1] for c in conns.elems if c.props[1] in cnode_ids and c.props[2] in drop_ids}
dead_curves = {c.props[1] for c in conns.elems if c.props[1] in curve_ids and c.props[2] in dead_nodes}
dead = dead_nodes | dead_curves

def keep_obj(m):
    return not (m.id in (b"AnimationCurveNode", b"AnimationCurve") and m.props[0] in dead)

def keep_conn(c):
    return not (c.props[1] in dead or c.props[2] in dead)

WRITERS = {b"C": "add_bool", b"Y": "add_int16", b"I": "add_int32", b"L": "add_int64", b"F": "add_float32", b"D": "add_float64",
           b"S": "add_bytes_string", b"R": "add_bytes", b"b": "add_bool_array", b"i": "add_int32_array", b"l": "add_int64_array",
           b"f": "add_float32_array", b"d": "add_float64_array", b"c": "add_byte_array"}

def convert(pe, parent=None):
    ne = encode_bin.FBXElem(pe.id)
    for v, t in zip(pe.props, pe.props_type):
        t = bytes([t]) if isinstance(t, int) else t
        if t == b"S":
            ne.add_string(v)
        elif t == b"C":
            ne.add_bool(bool(v))
        else:
            getattr(ne, WRITERS[t])(v)
    for ch in pe.elems:
        if pe.id == b"Objects" and not keep_obj(ch):
            continue
        if pe.id == b"Connections" and not keep_conn(ch):
            continue
        ne.elems.append(convert(ch))
    return ne

def fix_definitions(ne):
    if ne.id != b"Definitions":
        return
    for ot in ne.elems:
        if ot.id == b"ObjectType":
            pass   # counts are advisory in FBX; left as written

out_root = encode_bin.FBXElem(b"")
for top in root.elems:
    out_root.elems.append(convert(top))
encode_bin.write(path, out_root, version)
print("STRIP", path, "removed curve nodes", len(dead_nodes), "curves", len(dead_curves), "for", sorted(drop_models))
