import sys; sys.path.insert(0,r"C:/Users/kazda/jam/skater-lab/s6")
from common import *
a=read_raw(OUT+"T_SkaterTB2_BaseColor_orig.png"); b=read_raw(OUT+"T_SkaterTB2_BaseColor_clean.png")
ch=np.abs(a[...,:3]-b[...,:3]).max(-1)>1.5/255
m=np.load(OUT+"mask_changed.npy")
print("changed",ch.sum(),"outside mask",(ch&~m).sum())
# diff overlay
ov=a[...,:3]*0.5; ov[ch]=[1,0,1]
rg=np.load(OUT+"mask_region.npy")
edge=rg&~(np.roll(rg,1,0)&np.roll(rg,-1,0)&np.roll(rg,1,1)&np.roll(rg,-1,1))
ov[edge&~ch]=[1,1,0]
write_png(OUT+"diff_overlay.png",ov)
#,a[...,:3].mean(),b[...,:3].mean(),a[...,3].mean())
