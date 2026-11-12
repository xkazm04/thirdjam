# Counts, in the FBX file itself, which models have animation curve nodes (per stack is not separated; totals over all takes).
#   blender -b --factory-startup --python fbx_tracks.py -- <file.fbx>
import sys
from io_scene_fbx import parse_fbx
root, _ = parse_fbx.parse(sys.argv[sys.argv.index("--") + 1])
names, cnodes, stacks = {}, set(), []
for e in root.elems:
    if e.id == b"Objects":
        for m in e.elems:
            if m.id == b"Model":
                names[m.props[0]] = m.props[1].decode(errors="ignore").split("\x00")[0]
            elif m.id == b"AnimationCurveNode":
                cnodes.add(m.props[0])
            elif m.id == b"AnimationStack":
                stacks.append(m.props[1].decode(errors="ignore").split("\x00")[0])
per = {}
for e in root.elems:
    if e.id == b"Connections":
        for c in e.elems:
            if c.props[0] == b"OP" and c.props[1] in cnodes and c.props[2] in names:
                per[names[c.props[2]]] = per.get(names[c.props[2]], 0) + 1
print("TRACKS stacks", len(stacks), "per model (curve nodes over all takes):", per)
