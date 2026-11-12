import sys; sys.path.insert(0,r"C:/Users/kazda/jam/skater-lab/s6")
from common import *
ob=load()
im=bpy.data.images['diffuse']; print(im.size, im.colorspace_settings.name)
im.filepath_raw=OUT+"T_SkaterTB2_BaseColor_orig.png"; im.file_format='PNG'; im.save()
# per-face data: centroid (world), uv tris, to npz
me=ob.data; uv=me.uv_layers.active.data
import mathutils
M=ob.matrix_world
P=[];U=[];C=[]
for p in me.polygons:
    vs=[M@me.vertices[v].co for v in p.vertices]
    c=sum(vs,mathutils.Vector())/len(vs); C.append(tuple(c))
    U.append([tuple(uv[l].uv) for l in p.loop_indices])
import pickle; pickle.dump((C,U,[len(u) for u in U]),open(OUT+"faces.pkl","wb"))
cs=np.array(C); print(cs.min(0),cs.max(0))
print(ob.location,ob.rotation_euler,ob.scale)
