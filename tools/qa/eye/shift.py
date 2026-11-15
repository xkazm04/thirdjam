import sys, numpy as np
from PIL import Image
# shift.py <shots dir A> <shots dir B> <prefix> <first frame> <last frame> : the picture's shift in px (dx, dy) of run B against run A on each sim frame
# (phase correlation on the ice region, HUD rows cut off): the camera kick's size on the screen when B differs from A only in jam.hitkick. Lane A210.
a, b, prefix, f0, f1 = sys.argv[1], sys.argv[2], sys.argv[3], int(sys.argv[4]), int(sys.argv[5])
def load(d, f):
    im = np.asarray(Image.open(f'{d}/{prefix}-f{f:06d}.png').convert('L'), dtype=np.float32)
    return im[60:440, :]
for f in range(f0, f1 + 1):
    try: A, B = load(a, f), load(b, f)
    except Exception as e: print(f, 'missing'); continue
    w = np.outer(np.hanning(A.shape[0]), np.hanning(A.shape[1]))
    FA, FB = np.fft.fft2(A * w), np.fft.fft2(B * w)
    R = FA * np.conj(FB); R /= np.abs(R) + 1e-9
    c = np.abs(np.fft.ifft2(R))
    y, x = np.unravel_index(np.argmax(c), c.shape)
    if y > A.shape[0] // 2: y -= A.shape[0]
    if x > A.shape[1] // 2: x -= A.shape[1]
    print(f'f{f} dx={-x:+d} dy={-y:+d} peak={c.max():.2f} meanabsdiff={np.abs(A-B).mean():.2f}')
