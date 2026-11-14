#!/bin/sh
# lane A194 chain 1: law 3 by distance, as shipped (three levels) and the late-stop candidates (pro)
cd "$(dirname "$0")"
echo "p1 $$" >> out/pids.txt
AT="7,7.5,8,8.5,9,9.25,9.5,10,11"
JAM_LAW3_AT="$AT" sh law3.sh l3-base "rookie:-|pro:-|all_star:-"
for c in "brk12 carry_brake_mult=1.2" "brk14 carry_brake_mult=1.4" "fs22 burst_carry_accel_mult=2.2" "fs26 burst_carry_accel_mult=2.6" "brk12fs22 carry_brake_mult=1.2;burst_carry_accel_mult=2.2"; do
  set -- $c; JAM_LAW3_AT="$AT" JAM_LAW3_SET="$2" sh law3.sh l3-$1 "pro:-|all_star:-"
done
echo p1-done
