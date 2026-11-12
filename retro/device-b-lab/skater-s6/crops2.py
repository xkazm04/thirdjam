import sys; sys.path.insert(0,r"C:/Users/kazda/jam/skater-lab/s6")
from common import *
a=read_raw(OUT+"T_SkaterTB2_BaseColor_orig.png")[...,:3]
boxes={'c0':(1328,1456,1456,1584),'c1':(1264,48,1392,176),'c2':(1200,1200,1328,1328),'c3':(368,1776,496,1904),'c4':(-16,1520,112,1648),'c5':(368,240,496,368),'c6':(368,1200,496,1328),'c7':(368,1840,496,1968)}
for k,(x0,y0,x1,y1) in boxes.items():
    x0=max(x0,6);y0=max(y0,6);c=np.clip(a[y0-6:y1+6,x0-6:x1+6]**0.7,0,1); s=3
    write_png(OUT+f"isl{k}.png",np.repeat(np.repeat(c,s,0),s,1))
