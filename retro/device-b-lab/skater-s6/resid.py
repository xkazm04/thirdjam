import sys; sys.path.insert(0,r"C:/Users/kazda/jam/skater-lab/s6")
from common import *
import clean_texture_lib as ct
c=read_raw(OUT+"T_SkaterTB2_BaseColor_clean.png")[...,:3]
L=c@np.array([.2126,.7152,.0722],np.float32); cl=ct.closing(L,13)
rg=np.load(OUT+"mask_region.npy"); r=((cl-L)>0.03)&(cl<0.35)&rg
H=r.reshape(32,64,32,64).sum((1,3)); idx=np.dstack(np.unravel_index(np.argsort(-H.ravel())[:10],H.shape))[0]
for y,x in idx: print("cell y",y*64,"x",x*64,"n",H[y,x])
