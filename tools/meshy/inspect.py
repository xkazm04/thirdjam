# Blender headless inspector for a Meshy (or any) GLB/FBX.
#   blender -b --factory-startup --python tools/meshy/inspect.py -- <file.glb|file.fbx> [more files...]
# Prints, per file: armature name, bone count, bone names (parent>child), mesh triangle count, image sizes,
# actions with frame range, and root-motion figures (root and pelvis translation over each action).
# Last line per file: {"tool":"thirdjam.meshy-inspect/1", ...}
import bpy, sys, json, os

def reset():
    bpy.ops.wm.read_factory_settings(use_empty=True)

def load(path):
    ext = os.path.splitext(path)[1].lower()
    if ext in (".glb", ".gltf"):
        bpy.ops.import_scene.gltf(filepath=path)
    elif ext == ".fbx":
        bpy.ops.import_scene.fbx(filepath=path, automatic_bone_orientation=False)
    else:
        raise SystemExit("unsupported " + ext)

def tri_count(obj):
    me = obj.data
    me.calc_loop_triangles()
    return len(me.loop_triangles)

def world_bone_pos(arm, bone_name, frame):
    bpy.context.scene.frame_set(frame)
    pb = arm.pose.bones.get(bone_name)
    if not pb:
        return None
    m = arm.matrix_world @ pb.matrix
    return [round(v, 4) for v in m.translation]

def inspect(path):
    reset()
    load(path)
    out = {"tool": "thirdjam.meshy-inspect/1", "file": os.path.basename(path), "bytes": os.path.getsize(path)}
    arms = [o for o in bpy.data.objects if o.type == "ARMATURE"]
    meshes = [o for o in bpy.data.objects if o.type == "MESH"]
    out["meshes"] = len(meshes)
    out["triangles"] = sum(tri_count(m) for m in meshes)
    out["vertices"] = sum(len(m.data.vertices) for m in meshes)
    out["materials"] = sorted({s.material.name for m in meshes for s in m.material_slots if s.material})
    out["images"] = [{"name": i.name, "size": list(i.size)} for i in bpy.data.images if i.size[0] > 0]
    if meshes:
        import mathutils
        pts = [m.matrix_world @ mathutils.Vector(c) for m in meshes for c in m.bound_box]
        lo = [min(p[i] for p in pts) for i in range(3)]
        hi = [max(p[i] for p in pts) for i in range(3)]
        out["bounds_m"] = [round(hi[i] - lo[i], 3) for i in range(3)]
    if arms:
        a = arms[0]
        out["armature"] = a.name
        bones = a.data.bones
        out["bone_count"] = len(bones)
        out["bones"] = [b.name for b in bones]
        print("BONES (parent > child):")
        for b in bones:
            print("  ", (b.parent.name if b.parent else "-"), ">", b.name)
        acts = []
        for act in bpy.data.actions:
            a.animation_data_create()
            a.animation_data.action = act
            f0, f1 = [int(round(x)) for x in act.frame_range]
            rec = {"name": act.name, "frames": [f0, f1], "fcurves": len(act.fcurves)}
            for bn in ("root", "pelvis", "Hips", "mixamorig:Hips"):
                p0, p1 = world_bone_pos(a, bn, f0), world_bone_pos(a, bn, f1)
                if p0 and p1:
                    rec["move_" + bn] = [round(p1[i] - p0[i], 3) for i in range(3)]
            acts.append(rec)
        out["actions"] = acts
        out["fps"] = bpy.context.scene.render.fps
    print(json.dumps(out))

args = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
for p in args:
    inspect(p)
