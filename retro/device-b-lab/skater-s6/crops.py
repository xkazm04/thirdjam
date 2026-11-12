import sys; sys.path.insert(0,r"C:/Users/kazda/jam/skater-lab/s6")
from common import *
a=read_raw(OUT+"T_SkaterTB2_BaseColor_orig.png")[...,:3]; c=read_raw(OUT+"T_SkaterTB2_BaseColor_clean.png")[...,:3]
def crop(y,x,h=160,w=160):
    r=lambda t: np.repeat(np.repeat(np.clip(t[y:y+h,x:x+w]**0.6,0,1),3,0),3,1)
    return np.concatenate([r(a),r(c)],1)
import os
for k,(y,x) in enumerate([(280,1380),(670,600),(990,0),(610,420),(1570,1760),(230,1760)]):
    write_png(OUT+f"crop{k}.png",crop(y,x))
