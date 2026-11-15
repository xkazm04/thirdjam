#!/bin/sh
# lane A207 (wave 2): this lane's keys at their neutral values (with skater.goalie.read_mps 0), 100 seeds a level and rink, against the pre-edit build (raw/base-*)
cd "$(dirname "$0")"
echo "neutral2.sh $$" >> out/pids.txt
SET="pass.power.full_frames=0;pass.aim.cone_deg=0;pass.catch_m=0;pass.clear_own_body=false;shot.speed_scale=1;skater.goalie.read_mps=0"
for r in regular large; do for l in rookie pro all_star; do node curve.mjs neu2-$l-$r regular $l $r 100 --check closing --workers 3 --set "$SET" > /dev/null; node same.mjs base-$l-$r neu2-$l-$r; done; done
echo neutral2-done
