#!/bin/sh
# Lane A198 item 2, second pass: the rows FAULT 3 cut short (the match died at frame 0 as the slow guest arrived), each tried up to 4 times until its
# match gets past frame 0. Loopback only, as a198-twoproc.sh. Out: tools/soak/lockstep/out/a198b-*.txt and a198b-rows.txt.
cd "$(dirname "$0")/../../.."
P="powershell -NoProfile -ExecutionPolicy Bypass -File tools/soak/lockstep/twoproc5.ps1"
R=tools/soak/lockstep/out/a198b-rows.txt; mkdir -p tools/soak/lockstep/out; : > $R
row() { name=$1; shift
  for k in 1 2 3 4; do
    echo "== $name try $k $(date +%H:%M:%S)" >> $R
    $P -Name a198b-$name -HostPort 4796 -GuestPort 4797 "$@" > tools/soak/lockstep/out/a198b-$name.txt 2>&1; code=$?
    echo "exit $code $(grep -E '^(PASS|FAIL)' tools/soak/lockstep/out/a198b-$name.txt | tr '\n' ' ')" >> $R
    if grep -q 'lost frame=0 ' tools/soak/lockstep/out/a198b-$name/host.log 2>/dev/null; then echo "   (FAULT 3: died at frame 0; retry)" >> $R; continue; fi
    break
  done
}
row guestleaves607 -Mode guestleaves -LeaveAt 607 -Cut hockey.regular -Player good -Verbs -TimeoutSec 240
row guestleaves1803 -Mode guestleaves -LeaveAt 1803 -Cut hockey.regular -Player good -Verbs -TimeoutSec 240
row latejoin -Mode latejoin -LatePort 4795 -LateDelaySec 45 -Cut hockey.regular -Player good -Verbs -Frames 5400 -TimeoutSec 360
echo "done $(date +%H:%M:%S)" >> $R
