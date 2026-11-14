#!/bin/sh
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 4 > /dev/null; }
AS="mate_speed_scale=1;opp.hunt_radius_m=30;opp.shoot_extra_m=60"
run L11-as-block good all_star regular 600 "$AS;opp.block_shots=1;opp.block_min_dist_m=10"
run L11-as-draw25 good all_star regular 600 "$AS;opp.draw_jump_pct=25"
