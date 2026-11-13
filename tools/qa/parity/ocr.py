#!/usr/bin/env python3
"""Reads the scoreboard (blue score, period, clock, orange score) from 1280x720 stills of the HUD by OCR.
usage: ocr.py <dir> -> prints one JSON line per still: {"shot":..,"blue":..,"orange":..,"period":..,"clock":..,"over":bool}"""
import sys, os, json, re, glob
import numpy as np, cv2, pytesseract
pytesseract.pytesseract.tesseract_cmd = r'C:\Program Files\Tesseract-OCR\tesseract.exe'
# boxes at 1280x720 (x0, y0, x1, y1)
BOX = {'blue': (500, 8, 580, 72), 'orange': (700, 8, 780, 72), 'clock': (606, 32, 674, 59), 'period': (656, 14, 674, 30)}
def ocr(img, box, cfg, invert):
    x0, y0, x1, y1 = box
    c = img[y0:y1, x0:x1]
    g = cv2.cvtColor(c, cv2.COLOR_BGR2GRAY)
    g = cv2.resize(g, None, fx=4, fy=4, interpolation=cv2.INTER_CUBIC)
    _, t = cv2.threshold(g, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)
    if invert: t = 255 - t
    t = cv2.copyMakeBorder(t, 20, 20, 20, 20, cv2.BORDER_CONSTANT, value=255 if not invert else 255)
    return pytesseract.image_to_string(t, config=cfg).strip()
def read(path):
    img = cv2.imread(path)
    out = {'shot': os.path.basename(path)}
    for k, cfg in (('blue', '--psm 7 -c tessedit_char_whitelist=0123456789'), ('orange', '--psm 7 -c tessedit_char_whitelist=0123456789'),
                   ('clock', '--psm 7 -c tessedit_char_whitelist=0123456789:'), ('period', '--psm 10 -c tessedit_char_whitelist=123')):
        out[k] = ocr(img, BOX[k], cfg, k in ('blue', 'orange', 'clock', 'period'))
    return out
if __name__ == '__main__':
    for p in sorted(glob.glob(os.path.join(sys.argv[1], 't*.png'))):
        print(json.dumps(read(p)))
