#!/bin/sh
# lane A207: the default match with this lane's keys at their neutral values, 600 matches a level and rink (= the game before the lane on this tree) -> raw/neu-<level>-<rink>.jsonl;
# the first 100 seeds of each are the base's (raw/base-*, the pre-edit build): same.mjs compares them (the neutral proof)
cd "$(dirname "$0")"
echo "neutral.sh $$" >> out/pids.txt
SET="pass.power.full_frames=0;pass.aim.cone_deg=0;pass.catch_m=0;pass.clear_own_body=false;shot.speed_scale=1"
for r in regular large; do for l in rookie pro all_star; do node curve.mjs neu-$l-$r regular $l $r 600 --check closing --workers 2 --set "$SET" > /dev/null; node same.mjs base-$l-$r neu-$l-$r; done; done
echo neutral-done
