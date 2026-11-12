# tools/p4/glb_texture.py (lane A28): dump the images embedded in a GLB as PNG files (Blender headless).
#   blender -b --factory-startup --python tools/p4/glb_texture.py -- <in.glb> <out_dir>
import bpy, sys, os
a = sys.argv[sys.argv.index("--") + 1:]
src, out = a[0], a[1]
os.makedirs(out, exist_ok=True)
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=src)
for im in bpy.data.images:
    if im.size[0] == 0: continue
    p = os.path.join(out, im.name.replace("/", "_").replace(" ", "_") + ".png")
    im.filepath_raw = p; im.file_format = "PNG"; im.save()
    print("A28 glbtex", im.name, list(im.size), p, os.path.getsize(p))
m = [o for o in bpy.data.objects if o.type == "MESH"]
print("A28 glbtex uv_layers", [[u.name for u in o.data.uv_layers] for o in m])
