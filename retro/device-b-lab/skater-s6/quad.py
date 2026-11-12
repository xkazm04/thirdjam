import sys; sys.path.insert(0,r"C:/Users/kazda/jam/skater-lab/s6")
from common import *
a=read_raw(OUT+"T_SkaterTB2_BaseColor_orig.png")[...,:3]
rg=np.load(OUT+"mask_region.npy")
b=np.clip(a**0.45*1.0,0,1)      # brighten darks
b[~rg]*=0.35
for i,(y,x) in enumerate([(0,0),(0,1024),(1024,0),(1024,1024)]):
    write_png(OUT+f"quad{i}.png",b[y:y+1024,x:x+1024])
