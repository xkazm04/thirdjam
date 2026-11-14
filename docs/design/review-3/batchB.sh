#!/usr/bin/env bash
# Lane A169 (REVIEW-3), batch B: the three traced cells (three processes), then the drills and the stop's sweep (one process) beside two carry traces, then the third.
D=docs/design/review-3
bash $D/traces.sh
JAM_SWEEP_FILE="$(pwd -W)/$D/sweep.txt" bash $D/auto.sh automation "Jam.Core.Contact+Jam.Core.Movement+Bar.Movement.StopAndGoSweep+Jam.Bot.Hockey2+Jam.Bot.Hockey4+Jam.Hud+Jam.Menu+Jam.Camera" &
JAM_CARRY_TRACE="level=rookie;player=novice;seeds=100" bash $D/auto.sh carry-rk-novice "Bar.Movement.CarryTrace" &
JAM_CARRY_TRACE="level=pro;player=regular;seeds=100" bash $D/auto.sh carry-pro-regular "Bar.Movement.CarryTrace" &
wait
JAM_CARRY_TRACE="level=all_star;player=good;seeds=100" bash $D/auto.sh carry-as-good "Bar.Movement.CarryTrace" &
JAM_CARRY_TRACE="level=pro;player=novice;seeds=100" bash $D/auto.sh carry-pro-novice "Bar.Movement.CarryTrace" &
wait
echo done > $D/raw/batchB.done
