import sys; sys.path.insert(0,r"C:/Users/kazda/jam/skater-lab/s6")
from common import *
pts=[]
for az in range(0,360,45):
    n=f"shin{az:03d}"
    A=read_raw(OUT+f"r_before/before_{n}.png")[...,:3]   # wrong for png load? fine
    Uv=read_raw(OUT+f"r_uv/uv_{n}.png")[...,:3]
    L=A@np.array([.2126,.7152,.0722],np.float32)
    # dark and not near background; skip pure edges by requiring neighbours dark too
    sat=A.max(-1)-A.min(-1); d=(L<0.125)&(L>0.03)&(sat>0.12)
    ys,xs=np.nonzero(d)
    for y,x in zip(ys,xs): pts.append((Uv[y,x,0],Uv[y,x,1],az))
pts=np.array(pts); print(len(pts))
px=pts[:,0]*2048; py=(1-pts[:,1])*2048
H,_,_=np.histogram2d(py,px,bins=[32,32],range=[[0,2048],[0,2048]])
idx=np.dstack(np.unravel_index(np.argsort(-H.ravel())[:20],H.shape))[0]
for y,x in idx: print("cell y",y*64,"x",x*64,"n",int(H[y,x]))
