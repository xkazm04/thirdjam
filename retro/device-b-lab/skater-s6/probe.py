import bpy,sys
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=r"C:/Users/kazda/jam/skater-lab/s3/skater_notapose/base_basic_pbr.glb")
for o in bpy.data.objects:
    print(o.name,o.type,len(o.data.polygons) if o.type=='MESH' else '', [m.name for m in o.data.materials] if o.type=='MESH' else '', tuple(o.dimensions))
for im in bpy.data.images: print(im.name,im.size)
m=[o for o in bpy.data.objects if o.type=='MESH'][0]
mat=m.data.materials[0]
for n in mat.node_tree.nodes:
    print(n.type,n.name, getattr(n,'image',None))
    if n.type=='TEX_IMAGE' and n.image and 'olor' in n.image.name or (n.type=='TEX_IMAGE' and n.image and n.image.size[0]):
        pass
im=[n.image for n in mat.node_tree.nodes if n.type=='TEX_IMAGE'][0]
print(im.name)
