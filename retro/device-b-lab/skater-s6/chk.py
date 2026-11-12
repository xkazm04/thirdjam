import sys; sys.path.insert(0,r"C:/Users/kazda/jam/skater-lab/s6")
from common import *
a=read_raw(OUT+"T_SkaterTB2_BaseColor_orig.png")[...,:3]; c=read_raw(OUT+"T_SkaterTB2_BaseColor_clean.png")[...,:3]
for y0,x0,x1,y1 in [(1826,400,409,1906),(1238,379,391,1320)]:
    print(a[y0:y1,x0:x1+1].reshape(-1,3).mean(0), c[y0:y1,x0:x1+1].reshape(-1,3).mean(0), c[y0:y1:20,x0+4])
