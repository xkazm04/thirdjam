#!/bin/sh
# Lane A198 item 2: the two-process lockstep rows on protocol 9, loopback only (UDP 4796/4797 bound to 127.0.0.1; the late guest 4795; the guest's
# remote console TCP 4798 on 127.0.0.1). One row at a time (2 processes, 3 for latejoin). Out: tools/soak/lockstep/out/a198-*/ and a198-rows.txt.
cd "$(dirname "$0")/../../.."
P="powershell -NoProfile -ExecutionPolicy Bypass -File tools/soak/lockstep/twoproc5.ps1"
R=tools/soak/lockstep/out/a198-rows.txt; mkdir -p tools/soak/lockstep/out; : > $R
row() { name=$1; shift; echo "== $name $(date +%H:%M:%S)" >> $R; $P -Name a198-$name -HostPort 4796 -GuestPort 4797 "$@" > tools/soak/lockstep/out/a198-$name.txt 2>&1; echo "exit $? $(grep -E '^(PASS|FAIL)' tools/soak/lockstep/out/a198-$name.txt | tr '\n' ' ')" >> $R; }
row marathon10 -Cut modes/hockey.mode.marathon -Player good -Verbs -Frames 36000 -ToEnd -TimeoutSec 900
row refuse8 -Mode refuse -TestVersion 8 -Cut hockey.regular -TimeoutSec 120
row guestleaves607 -Mode guestleaves -LeaveAt 607 -Cut hockey.regular -Player good -Verbs -TimeoutSec 240
row guestleaves1803 -Mode guestleaves -LeaveAt 1803 -Cut hockey.regular -Player good -Verbs -TimeoutSec 240
row pause10 -Cut hockey.regular -Player good -Verbs -Frames 5400 -GuestRcPort 4798 -PauseAt 20 -PauseFor 10 -TimeoutSec 360
row pause45 -Cut hockey.regular -Player good -Verbs -Frames 5400 -GuestRcPort 4798 -PauseAt 20 -PauseFor 45 -TimeoutSec 420
row latejoin -Mode latejoin -LatePort 4795 -LateDelaySec 30 -Cut hockey.regular -Player good -Verbs -Frames 5400 -TimeoutSec 360
echo "done $(date +%H:%M:%S)" >> $R
