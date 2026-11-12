import sys; sys.path.insert(0,r"C:/Users/kazda/jam/skater-lab/s6")
from common import *; from uvmask import *
import pickle
C,U=load_faces(); (isl,)=pickle.load(open(OUT+"islands.pkl","rb"))
a=read_raw(OUT+"T_SkaterTB2_BaseColor_orig.png")[...,:3]
for k in (268,424,428,400,399,384,269,371):
    sel=np.zeros(len(U),bool); sel[isl[k]]=True; m=region_mask(sel,U); ys,xs=np.nonzero(m)
    x0,x1,y0,y1=xs.min(),xs.max(),ys.min(),ys.max(); print(k,"bbox",x0,y0,x1,y1,"z",C[isl[k],2].mean())
    c=np.clip(a[max(y0-4,0):y1+4,max(x0-4,0):x1+4]**0.7,0,1).copy(); mm=m[max(y0-4,0):y1+4,max(x0-4,0):x1+4]; c[~mm]*=0.3
    s=max(1,min(4,500//max(c.shape))); write_png(OUT+f"is{k}.png",np.repeat(np.repeat(c,s,0),s,1))
