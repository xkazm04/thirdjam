#!/bin/sh
# lane A133 round 2 (600, regular, pro): levers on top of the hunt back at 8 m (law 3)
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 2 > /dev/null; }
H="opp.hunt_radius_m=8"
run R2-pro-s60 regular pro regular 600 "$H;opp.shoot_extra_m=60" &
run R2-pro-passout regular pro regular 600 "$H;opp.pass_out=1" &
run R2-pro-deke regular pro regular 600 "$H;opp.deke=1" &
run R2-pro-hkstar regular pro regular 600 "$H;opp.hk_share=1;opp.hk_late_frames=0;opp.hk_read_human_pressure=1" &
run R2-pro-fy-1 regular pro regular 600 "$H;opp.forecheck_yield_frames=-1" &
run R2-pro-carry regular pro regular 600 "$H;opp.hk_pressure_s=0.3;opp.hk_better_by_m=12" &
wait
run R2-pro-s80 regular pro regular 600 "$H;opp.shoot_extra_m=80" &
run R2-pro-chg6 regular pro regular 600 "$H;opp.shot_charge_frames=6" &
run R2-pro-matepass02 regular pro regular 600 "$H;mate.pass_to_human=0.2" &
run R2-pro-red11 regular pro regular 600 "$H;red_skill=1.1" &
run R2-pro-gap regular pro regular 600 "$H;opp.hk_gap_control=1" &
run R2-pro-sprint07-l45 regular pro regular 600 "$H;opp.sprint_share=0.7;opp.lift_cooldown_frames=45" &
wait
