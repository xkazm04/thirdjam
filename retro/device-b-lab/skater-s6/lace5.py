import sys; sys.path.insert(0,r"C:/Users/kazda/jam/skater-lab/s6")
from common import *
import mathutils, pickle
from mathutils.bvhtree import BVHTree
ob=load(); me=ob.data
bm_verts=[v.co.copy() for v in me.vertices]; polys=[tuple(p.vertices) for p in me.polygons]
tree=BVHTree.FromPolygons(bm_verts,polys)
(isl,)=pickle.load(open(OUT+"islands.pkl","rb"))
face2isl={}
for k,ids in enumerate(isl):
    for f in ids: face2isl[int(f)]=k
cam=bpy.data.cameras.new('c'); cam.lens=60; co=bpy.data.objects.new('c',cam); bpy.context.scene.collection.objects.link(co)
sc=bpy.context.scene; sc.render.resolution_x=sc.render.resolution_y=640
bpy.context.view_layer.update()
from collections import Counter
cnt=Counter(); bad=Counter()
for az in range(0,360,45):
    A=read_raw(OUT+f"r_after/after_shin{az:03d}.png")[...,:3]
    a=math.radians(az); t=mathutils.Vector((0,0,-0.55))
    co.location=t+mathutils.Vector((math.sin(a)*1.6,-math.cos(a)*1.6,0))
    co.rotation_euler=(t-co.location).to_track_quat('-Z','Y').to_euler(); bpy.context.view_layer.update()
    fr=[co.matrix_world@v for v in cam.view_frame(scene=sc)]   # 4 corners
    # corners order: (+x,-y?) -> derive via cam space
    cs=[v for v in cam.view_frame(scene=sc)]
    xs=sorted(set(round(v.x,6) for v in cs)); ys=sorted(set(round(v.y,6) for v in cs)); z=cs[0].z
    if az not in (0,45,315): continue
    for y in range(300,350,1):
        for x in range(80,200,1):
            cx=xs[0]+(xs[1]-xs[0])*(x+0.5)/640; cy=ys[1]-(ys[1]-ys[0])*(y+0.5)/640
            d=(co.matrix_world.to_3x3()@mathutils.Vector((cx,cy,z))).normalized()
            loc,n,idx,dist=tree.ray_cast(co.location,d)
            if idx is not None:
                l=float(A[y,x]@np.array([.2126,.7152,.0722]))
                cnt[(az,face2isl[idx])]+=1
                if l<0.135: bad[(az,face2isl[idx])]+=1
print("TOP",cnt.most_common(12)); print("DARK",bad.most_common(12))
