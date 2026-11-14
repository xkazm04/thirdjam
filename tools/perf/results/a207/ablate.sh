#!/bin/sh
# lane A207: which half moves the cells: the pass rules on with the shot at 1x (shot1), and the shot doubled with the pass rules off (pass0); pro large and all_star regular, 600 each
cd "$(dirname "$0")"
echo "ablate.sh $$" >> out/pids.txt
P0="pass.power.full_frames=0;pass.aim.cone_deg=0;pass.catch_m=0;pass.clear_own_body=false"
for c in "pro large" "all_star regular" "pro regular"; do set -- $c
  node curve.mjs shot1-$1-$2 regular $1 $2 600 --check closing --workers 2 --set "shot.speed_scale=1" > /dev/null
  node curve.mjs pass0-$1-$2 regular $1 $2 600 --check closing --workers 2 --set "$P0" > /dev/null
done
echo ablate-done
