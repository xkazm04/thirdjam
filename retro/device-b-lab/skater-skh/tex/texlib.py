# shared helpers (copied from s6 common.py/uvmask.py/clean_texture.py, paths parameterised)
import numpy as np, zlib, struct
def img_to_np(im):
    w,h=im.size; a=np.empty(w*h*4,np.float32); im.pixels.foreach_get(a); return a.reshape(h,w,4)
def write_png(path,rgb):
    a=(np.clip(rgb[...,:3],0,1)*255+0.5).astype(np.uint8); h,w=a.shape[:2]
    raw=b''.join(b'\x00'+a[y].tobytes() for y in range(h))
    def ch(t,d): c=struct.pack('>I',len(d))+t+d; return c+struct.pack('>I',zlib.crc32(t+d)&0xffffffff)
    open(path,'wb').write(b'\x89PNG\r\n\x1a\n'+ch(b'IHDR',struct.pack('>IIBBBBB',w,h,8,2,0,0,0))+ch(b'IDAT',zlib.compress(raw,6))+ch(b'IEND',b''))
def read_raw(bpy,path):
    im=bpy.data.images.load(path,check_existing=False); im.colorspace_settings.name='Non-Color'; im.reload()
    return img_to_np(im)[::-1].copy()
def raster(tris,N):
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
        m[y0:y1+1,x0:x1+1]|=(a>=e)&(b>=e)&(c>=e)
    return m
def region_mask(sel,U,N):
    out=[]
    for i in np.nonzero(sel)[0]:
        u=U[i]
        for k in range(1,len(u)-1): out.append([u[0],u[k],u[k+1]])
    return raster(out,N)
def shift_min(a,k,axis):
    out=a.copy()
    for s in range(1,k+1): out=np.minimum(out,np.roll(a,s,axis)); out=np.minimum(out,np.roll(a,-s,axis))
    return out
def shift_max(a,k,axis):
    out=a.copy()
    for s in range(1,k+1): out=np.maximum(out,np.roll(a,s,axis)); out=np.maximum(out,np.roll(a,-s,axis))
    return out
def closing(L,k): return shift_min(shift_min(shift_max(shift_max(L,k,0),k,1),k,0),k,1)
def dilate(m,k):
    f=m.astype(np.float32)
    for ax in (0,1): f=shift_max(f,k,ax)
    return f>0
def gblur(a,s):
    h,w=a.shape[:2]; fy=np.fft.fftfreq(h)[:,None]; fx=np.fft.fftfreq(w)[None,:]
    g=np.exp(-2*(np.pi**2)*s*s*(fx**2+fy**2))
    return np.stack([np.fft.ifft2(np.fft.fft2(a[...,c])*g).real for c in range(a.shape[2])],-1) if a.ndim==3 else np.fft.ifft2(np.fft.fft2(a)*g).real
def components(m,group=5):
    """clusters of m grouped by `group` px dilation -> list of (ys,xs) of m pixels"""
    g=dilate(m,group); H,W=g.shape; ys,xs=np.nonzero(g); idx=set((ys*W+xs).tolist()); out=[]
    while idx:
        st=[idx.pop()]; comp=[st[0]]
        while st:
            v=st.pop(); y,x=divmod(v,W)
            for dy in (-1,0,1):
                for dx in (-1,0,1):
                    n=(y+dy)*W+x+dx
                    if n in idx: idx.discard(n); st.append(n); comp.append(n)
        c=np.array(comp); cy=c//W; cx=c%W; sel=m[cy,cx]
        if sel.any(): out.append((cy[sel],cx[sel]))
    return out
