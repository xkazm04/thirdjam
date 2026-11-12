import numpy as np, pickle
def load_faces():
    C,U,_=pickle.load(open(r"C:/Users/kazda/jam/skater-lab/s6/faces.pkl","rb"))
    return np.array(C),U
def raster(tris,N=2048):
    """tris: list of 3x2 uv arrays -> bool mask (row0=top of image)"""
    m=np.zeros((N,N),bool)
    for t in tris:
        p=np.array(t,float)*N; p[:,1]=N-p[:,1]
        x0,y0=np.floor(p.min(0)).astype(int); x1,y1=np.ceil(p.max(0)).astype(int)
        x0=max(x0,0);y0=max(y0,0);x1=min(x1,N-1);y1=min(y1,N-1)
        if x1<x0 or y1<y0: continue
        xs,ys=np.meshgrid(np.arange(x0,x1+1)+.5,np.arange(y0,y1+1)+.5)
        d=(p[1,1]-p[2,1])*(p[0,0]-p[2,0])+(p[2,0]-p[1,0])*(p[0,1]-p[2,1])
        if abs(d)<1e-9: continue
        a=((p[1,1]-p[2,1])*(xs-p[2,0])+(p[2,0]-p[1,0])*(ys-p[2,1]))/d
        b=((p[2,1]-p[0,1])*(xs-p[2,0])+(p[0,0]-p[2,0])*(ys-p[2,1]))/d
        c=1-a-b; e=-0.15
        inside=(a>=e)&(b>=e)&(c>=e)
        m[y0:y1+1,x0:x1+1]|=inside
    return m
def tris_for(sel,U):
    out=[]
    for i in np.nonzero(sel)[0]:
        u=U[i]
        for k in range(1,len(u)-1): out.append([u[0],u[k],u[k+1]])
    return out
def region_mask(sel,U): return raster(tris_for(sel,U))
