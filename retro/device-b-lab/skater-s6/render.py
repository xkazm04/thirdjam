import sys; sys.path.insert(0,r"C:/Users/kazda/jam/skater-lab/s6")
from common import *
import mathutils
# args after --: texture png, outdir, tag
argv=sys.argv[sys.argv.index('--')+1:]
tex,outdir,tag=argv[0],argv[1],argv[2]
import os; os.makedirs(outdir,exist_ok=True)
ob=load()
if tex=='UV':
    N=256; g=np.zeros((N,N,4),np.float32); g[...,0]=np.linspace(0,1,N)[None,:]; g[...,1]=np.linspace(0,1,N)[:,None]; g[...,3]=1
    im=bpy.data.images.new('uvg',N,N,float_buffer=True); im.colorspace_settings.name='Non-Color'; im.pixels.foreach_set(g.ravel())
else:
    im=bpy.data.images.load(tex); im.colorspace_settings.name='sRGB'
for n in ob.data.materials[0].node_tree.nodes:
    if n.type=='TEX_IMAGE' and n.image.name=='diffuse': n.image=im
sc=bpy.context.scene
sc.render.engine='BLENDER_WORKBENCH'
sh=sc.display.shading; sh.light='FLAT'; sh.color_type='TEXTURE'; sh.show_object_outline=False
sc.view_settings.view_transform='Raw' if tex=='UV' else 'Standard'
sc.display.shading.show_specular_highlight=False
sc.render.resolution_x=sc.render.resolution_y=640
sc.render.film_transparent=False
w=bpy.data.worlds.new('w'); w.color=(0.55,0.6,0.65); sc.world=w
cam=bpy.data.cameras.new('c'); co=bpy.data.objects.new('c',cam); sc.collection.objects.link(co); sc.camera=co
def shoot(name,target,dist,az,lens=50):
    cam.lens=lens
    a=math.radians(az); t=mathutils.Vector(target)
    co.location=t+mathutils.Vector((math.sin(a)*dist,-math.cos(a)*dist,0))
    d=t-co.location; co.rotation_euler=d.to_track_quat('-Z','Y').to_euler()
    sc.render.filepath=f"{outdir}/{tag}_{name}.png"; bpy.ops.render.render(write_still=True)
mode=argv[3] if len(argv)>3 else 'close'
if mode=='upper':
    for az in (0,90,180,270):
        shoot(f"head{az:03d}",(0,0,0.55),1.5,az,lens=70)
        shoot(f"chest{az:03d}",(0,0,0.0),1.7,az,lens=60)
elif mode=='turn':
    for i in range(8): shoot(f"turn{i}",(0,0,0),4.2,i*45)
else:
    # shins: lower legs; skate sides
    for az in (0,45,90,135,180,225,270,315):
        shoot(f"shin{az:03d}",(0,0,-0.55),1.6,az,lens=60)
