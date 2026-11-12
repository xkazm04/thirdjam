import bpy, sys
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=sys.argv[sys.argv.index("--")+1])
for im in bpy.data.images:
    if "baseColor" in im.name:
        im.filepath_raw = sys.argv[-1]; im.file_format = "PNG"; im.save(); print("SAVED", im.name, tuple(im.size))
