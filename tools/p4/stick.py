# tools/p4/stick.py (lane A28): a plain hockey stick mesh for the P4 figures' stick_socket (Blender headless, cm scene, FBX).
#   blender -b --factory-startup --python tools/p4/stick.py -- <out.fbx>
# Grip at the origin; the shaft runs down -Z (tilted 42 deg lie toward +X) for 130 cm; a 32 x 7.5 x 2.5 cm blade lies flat at the heel, turned 30 deg out. 2 materials.
import bpy, sys, math, os
out = sys.argv[sys.argv.index("--") + 1]
bpy.ops.wm.read_factory_settings(use_empty=True)
sc = bpy.context.scene
sc.unit_settings.system = "METRIC"; sc.unit_settings.scale_length = 0.01; sc.unit_settings.length_unit = "CENTIMETERS"
L = 130.0; lie = math.radians(42.0)
bpy.ops.mesh.primitive_cylinder_add(vertices=8, radius=1.5, depth=L, location=(0, 0, 0))
shaft = bpy.context.object; shaft.name = "shaft"
# cylinder axis is Z; move so the top (grip) sits at the origin, then tilt: the blade end goes forward (+X) and down
shaft.location = (0, 0, -L / 2)
bpy.ops.object.transform_apply(location=True)
shaft.rotation_euler = (0, lie, 0)
bpy.ops.object.transform_apply(rotation=True)
heel = (L * math.sin(lie), 0, -L * math.cos(lie))
bpy.ops.mesh.primitive_cube_add(size=1, location=(heel[0] + 14, heel[1], heel[2] + 1.5))
blade = bpy.context.object; blade.name = "blade"; blade.scale = (32, 2.5, 7.5)
blade.rotation_euler = (0, 0, math.radians(-30))
bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
m_shaft = bpy.data.materials.new("M_P4_StickShaft"); m_blade = bpy.data.materials.new("M_P4_StickBlade")
shaft.data.materials.append(m_shaft); blade.data.materials.append(m_blade)
bpy.ops.object.select_all(action="DESELECT"); shaft.select_set(True); blade.select_set(True); bpy.context.view_layer.objects.active = shaft
bpy.ops.object.join(); bpy.context.object.name = "SM_P4_Stick"
bpy.ops.export_scene.fbx(filepath=out, use_selection=True, apply_unit_scale=True, apply_scale_options="FBX_SCALE_NONE", global_scale=1.0, axis_forward="-Z", axis_up="Y",
                         object_types={"MESH"}, mesh_smooth_type="FACE", path_mode="COPY", embed_textures=False, bake_anim=False)
print("A28 stick wrote", out, os.path.getsize(out), "tris", sum(len(p.vertices) - 2 for p in bpy.context.object.data.polygons))
