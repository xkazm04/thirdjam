import sys; sys.path.insert(0,r"C:/Users/kazda/jam/skater-lab/s6")
from common import *; from uvmask import *
import clean_texture_lib as ct, pickle
C,U=load_faces(); nF=len(U)
par=list(range(nF))
def f(x):
    while par[x]!=x: par[x]=par[par[x]]; x=par[x]
    return x
key={}
for i,u in enumerate(U):
    for p in u:
        k=(round(p[0]*20000),round(p[1]*20000))
        if k in key: par[f(i)]=f(key[k])
        else: key[k]=i
roots={}
for i in range(nF): roots.setdefault(f(i),[]).append(i)
isl=[np.array(v) for v in roots.values()]
print("islands",len(isl))
a=read_raw(OUT+"T_SkaterTB2_BaseColor_orig.png")[...,:3]
L=a@np.array([.2126,.7152,.0722],np.float32); sat=a.max(-1)-a.min(-1)
cl=ct.closing(L,13); strong=((cl-L)>0.06)
res=[]
for k,ids in enumerate(isl):
    if C[ids,2].mean()>-0.1: continue
    m=region_mask(np.isin(np.arange(nF),ids),U)
    ys,xs=np.nonzero(m)
    if len(ys)==0: continue
    res.append((int((strong&m).sum()),k,m.sum(),xs.min(),ys.min(),xs.max(),ys.max(),C[ids,2].mean()))
res.sort(reverse=True)
for r in res[:25]: print("strong %d isl %d area %d bbox x%d y%d..x%d y%d zmean %.2f"%r)
pickle.dump((isl,),open(OUT+"islands.pkl","wb"))
