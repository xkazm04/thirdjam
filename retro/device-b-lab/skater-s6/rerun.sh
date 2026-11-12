#!/bin/sh
# S6 rerun: texture extract + UV face table, then clean, then verify (diff overlay)
B="C:/Program Files/Blender Foundation/Blender 4.2/blender.exe"; cd C:/Users/kazda/jam/skater-lab/s6
"$B" -b -P extract.py && "$B" -b -P clean_texture.py && "$B" -b -P verify.py
