import sys; sys.path.insert(0,r"C:/Users/kazda/jam/skater-lab/s6")
from common import *; from uvmask import *
C,U=load_faces()
z=C[:,2]
tex=bpy.data.images.load(OUT+"T_SkaterTB2_BaseColor_orig.png"); a=img_to_np(tex)[::-1,:,:3]  # top-first
bands=[(-0.9,-0.68,(1,0,0)),(-0.68,-0.45,(0,1,0)),(-0.45,-0.2,(0,0.6,1))]
ov=a*0.45
for lo,hi,col in bands:
    m=region_mask((z>=lo)&(z<hi),U); print(lo,hi,m.sum())
    ov[m]=ov[m]*0.5+np.array(col)*0.5
out=np.concatenate([ov,np.ones((2048,2048,1),np.float32)],2)[::-1]
im=np_to_img(out.astype(np.float32),'loc'); im.filepath_raw=OUT+"uv_locator_bands.png"; im.file_format='PNG'; im.save()
print("zhist",np.histogram(z,bins=12)[0])
