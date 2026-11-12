# Run: blender -b -P clean_texture.py   (reads faces.pkl made by extract.py and the original diffuse PNG)
# Removes thin/small dark strokes (laces, blade dots/smears) inside the UV regions of the lower legs/boots/blades (mesh z < Z_MAX).
import sys; sys.path.insert(0,r"C:/Users/kazda/jam/skater-lab/s6")
from common import *; from uvmask import *
Z_MAX=-0.20     # faces whose centroid is below this (mesh is 1.7 m tall, centred): shins, pads, boots, blades
R=13            # closing half-size px: dark features thinner than ~2R+1=35 px are "strokes"; thicker black shapes survive
THRESH=0.07     # luminance (sRGB, 0..1) that a pixel must be darker than the closed neighbourhood
THRESH_DARK=0.035  # lower threshold on a dark surround (cl < DARK_SURR) because navy-on-navy strokes are low contrast
DARK_SURR=0.35
MAXA=400        # a stroke cluster (grouped by GROUP px dilation) must fit in this bbox (px) or it is a shape/seam and is left alone
MAXB=60         # same for blobs on bright/colour surrounds (blade dots ~25 px; thick black edge bars and long seams are longer)
BLACK=0.10; NEUTRAL=0.12; MAXC=220
MINBAR=110
GROUP=5
DILATE=4        # px
SIGMA=(5,14,40) # fill blur radii, tried in turn for pixels not yet filled
def shift_min(a,k,axis):
    out=a.copy()
    for s in range(1,k+1):
        out=np.minimum(out,np.roll(a,s,axis)); out=np.minimum(out,np.roll(a,-s,axis))
    return out
def shift_max(a,k,axis):
    out=a.copy()
    for s in range(1,k+1):
        out=np.maximum(out,np.roll(a,s,axis)); out=np.maximum(out,np.roll(a,-s,axis))
    return out
def closing(L,k):
    m=shift_max(shift_max(L,k,0),k,1); return shift_min(shift_min(m,k,0),k,1)
def dilate(m,k):
    f=m.astype(np.float32)
    for ax in (0,1): f=shift_max(f,k,ax)
    return f>0
def gblur(a,s):
    h,w=a.shape[:2]; fy=np.fft.fftfreq(h)[:,None]; fx=np.fft.fftfreq(w)[None,:]
    g=np.exp(-2*(np.pi**2)*s*s*(fx**2+fy**2))
    return np.stack([np.fft.ifft2(np.fft.fft2(a[...,c])*g).real for c in range(a.shape[2])],-1) if a.ndim==3 else np.fft.ifft2(np.fft.fft2(a)*g).real
def keep_small(cand,maxdim):
    g=dilate(cand,GROUP); H,W=g.shape; ys,xs=np.nonzero(g); idx=set((ys*W+xs).tolist()); keep=np.zeros_like(cand)
    while idx:
        st=[idx.pop()]; comp=list(st)
        while st:
            v=st.pop(); y,x=divmod(v,W)
            for dy in (-1,0,1):
                for dx in (-1,0,1):
                    n=(y+dy)*W+x+dx
                    if n in idx: idx.discard(n); st.append(n); comp.append(n)
        c=np.array(comp); cy=c//W; cx=c%W
        if max(cy.max()-cy.min(),cx.max()-cx.min())+1<=maxdim+2*GROUP: keep[cy,cx]=True
    return keep&cand
def big_black(region,L,sat):
    """neutral near-black structures >= MINBAR px long (runner edges, outlines, seams): these are shapes, never touched."""
    m=(L<0.15)&(sat<0.2)&region; g=dilate(m,2); H,W=g.shape; ys,xs=np.nonzero(g); idx=set((ys*W+xs).tolist()); prot=np.zeros_like(m)
    while idx:
        st=[idx.pop()]; comp=list(st)
        while st:
            v=st.pop(); y,x=divmod(v,W)
            for dy in (-1,0,1):
                for dx in (-1,0,1):
                    n=(y+dy)*W+x+dx
                    if n in idx: idx.discard(n); st.append(n); comp.append(n)
        c=np.array(comp); cy=c//W; cx=c%W
        if max(cy.max()-cy.min(),cx.max()-cx.min())+1>=MINBAR: prot[cy,cx]=True
    return dilate(prot,6)
def main():
    src=OUT+"T_SkaterTB2_BaseColor_orig.png"; dst=OUT+"T_SkaterTB2_BaseColor_clean.png"
    im=bpy.data.images.load(src); im.colorspace_settings.name='Non-Color'; im.reload()  # work on raw stored values, no conversion
    a=img_to_np(im)[::-1].copy()   # top-first, float 0..1 as stored (sRGB-encoded)
    rgb=a[...,:3]; N=2048
    C,U=load_faces(); region=region_mask(C[:,2]<Z_MAX,U)
    L=rgb@np.array([.2126,.7152,.0722],np.float32)
    cl=closing(L,R); diff=cl-L
    sat=rgb.max(-1)-rgb.min(-1)
    candA=(diff>THRESH_DARK)&(cl<DARK_SURR)&(gblur(L,8)<0.28)&region                    # dark stroke on a dark (navy) surround: laces
    candB=(diff>THRESH)&(cl>=DARK_SURR)&(L<BLACK)&(gblur(L,8)>0.65)&region               # near-black blob on a bright/colour surround: blade dots
    candC=(diff>THRESH)&(cl>0.75)&(sat<NEUTRAL)&(L<0.8)&region         # grey (neutral) smear on white: blurred blade marks
    prot=big_black(region,L,sat); candA&=~prot; candB&=~prot; candC&=~prot
    keep=keep_small(candA,MAXA)|keep_small(candB,MAXB)|keep_small(candC,MAXC)
    np.save(OUT+'candA.npy',candA)
    mask=dilate(keep,DILATE)&dilate(region,DILATE)
    # fill: normalised blur of the pixels that are not masked
    known=(~mask).astype(np.float32)[...,None]
    res=rgb.copy(); todo=mask.copy()
    for s in SIGMA:
        num=gblur(rgb*known,s); den=gblur(known,s)
        ok=todo&(den[...,0]>0.15)
        fill=num/np.maximum(den,1e-4)
        res[ok]=fill[ok]; todo&=~ok
    out=a.copy(); out[...,:3]=np.clip(res,0,1)
    changed=np.abs(out[...,:3]-a[...,:3]).max(-1)>1/255
    print("masked px",int(mask.sum()),"changed px",int(changed.sum()),"region px",int(region.sum()),"unfilled",int(todo.sum()))
    write_png(dst,out)
    np.save(OUT+"mask_changed.npy",changed); np.save(OUT+"mask_region.npy",region)

