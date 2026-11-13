"""tools/blender/arena/pixel_change.py (Third Jam, track F3, lane A136): how loud is a reaction in a still?
The share of a still's pixels that differ from a reference still (the arena at rest) by more than 24 of 255, three ways:
  mean3  the mean of the three channels' differences (the track's measure: it gives 4.1 % for evolution 3's goal light, ARENA.md's 4.0 %)
  L      the luminance's difference
  max    any one channel's difference (a red light on white ice moves this one long before the others)
Run with the system Python (PIL and numpy), never inside Unreal:
  python tools/blender/arena/pixel_change.py <folder of PNG stills> [glob of the reference inside it, default *rest*.png] [glob of the stills, default *.png]
"""
import glob
import os
import sys

import numpy as np
from PIL import Image

THRESHOLD = 24


def main(folder, ref_glob="*rest*.png", glob_="*.png"):
    refs = sorted(glob.glob(os.path.join(folder, ref_glob)))
    if not refs:
        print("no reference still matches %s in %s" % (ref_glob, folder))
        return 2
    ref_image = Image.open(refs[0])
    ref = np.asarray(ref_image.convert("RGB")).astype(np.int16)
    ref_l = np.asarray(ref_image.convert("L")).astype(np.int16)
    print("reference %s" % os.path.basename(refs[0]))
    for path in sorted(glob.glob(os.path.join(folder, glob_))):
        image = Image.open(path)
        a = np.asarray(image.convert("RGB")).astype(np.int16)
        if a.shape != ref.shape:
            continue
        lum = np.asarray(image.convert("L")).astype(np.int16)
        d = np.abs(a - ref)
        print("%-64s mean3 %5.1f %%   L %5.1f %%   max %5.1f %%   mean rgb %s" % (
            os.path.basename(path), (d.mean(axis=2) > THRESHOLD).mean() * 100.0, (np.abs(lum - ref_l) > THRESHOLD).mean() * 100.0,
            (d.max(axis=2) > THRESHOLD).mean() * 100.0, a.reshape(-1, 3).mean(0).round(0).astype(int).tolist()))
    return 0


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(2)
    sys.exit(main(*sys.argv[1:4]))
