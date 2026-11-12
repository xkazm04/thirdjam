import sys; sys.path.insert(0,r"C:/Users/kazda/jam/skater-lab/s6")
from common import *
for n,box in [("shin000",(90,290,190,360)),("shin180",(0,0,1,1)),("shin090",(130,250,300,330))]:
    A=read_raw(OUT+f"r_before/before_{n}.png")[...,:3]; Uv=read_raw(OUT+f"r_uv/uv_{n}.png")[...,:3]
    # render rows: read_raw is top-first
    x0,y0,x1,y1=box; sub=A[y0:y1,x0:x1]; L=sub@np.array([.2126,.7152,.0722],np.float32)
    med=np.median(L); m=L<med-0.05
    ys,xs=np.nonzero(m); print(n,"median",med,"n",m.sum())
    if m.sum():
        u=Uv[y0:y1,x0:x1][m]; px=u[:,0]*2048; py=(1-u[:,1])*2048
        print(" uv px x",px.min(),px.max(),"y",py.min(),py.max(), "mean",px.mean(),py.mean())
        print(np.histogram2d(py,px,bins=[8,8],range=[[0,2048],[0,2048]])[0].astype(int))
