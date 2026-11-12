import bpy, numpy as np, math
GLB=r"C:/Users/kazda/jam/skater-lab/s3/skater_notapose/base_basic_pbr.glb"
OUT="C:/Users/kazda/jam/skater-lab/s6/"
def load():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.gltf(filepath=GLB)
    ob=[o for o in bpy.data.objects if o.type=='MESH'][0]
    return ob
def img_to_np(im):
    w,h=im.size; a=np.empty(w*h*4,np.float32); im.pixels.foreach_get(a); return a.reshape(h,w,4)  # row0 = bottom
def np_to_img(a,name):
    h,w=a.shape[:2]
    im=bpy.data.images.new(name,w,h,alpha=True,float_buffer=True); im.colorspace_settings.name='Non-Color'; im.pixels.foreach_set(a.ravel()); return im
import zlib, struct
def write_png(path,rgb_top_first):
    """rgb float 0..1 (H,W,3 or 4), top row first -> 8-bit RGB PNG (no Blender colour management)."""
    a=(np.clip(rgb_top_first[...,:3],0,1)*255+0.5).astype(np.uint8); h,w=a.shape[:2]
    raw=b''.join(b'\x00'+a[y].tobytes() for y in range(h))
    def ch(t,d): c=struct.pack('>I',len(d))+t+d; return c+struct.pack('>I',zlib.crc32(t+d)&0xffffffff)
    open(path,'wb').write(b'\x89PNG\r\n\x1a\n'+ch(b'IHDR',struct.pack('>IIBBBBB',w,h,8,2,0,0,0))+ch(b'IDAT',zlib.compress(raw,6))+ch(b'IEND',b''))
def read_raw(path):
    im=bpy.data.images.load(path); im.colorspace_settings.name='Non-Color'; im.reload()
    return img_to_np(im)[::-1].copy()
