#!/bin/sh
# lane A201 item 1: the line as shipped (no gate), the scripted player ignoring it and obeying it, 200 seeds, pro then rookie (one process at a time)
cd "$(dirname "$0")"
echo "m1 $$" >> out/pids.txt
for l in pro rookie; do JAM_TRACE_ARMS="ignore@@|obey@brake@" sh trace.sh m1-$l "level=$l;player=regular;seeds=200"; done
echo m1-done
