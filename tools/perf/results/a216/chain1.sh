#!/bin/sh
cd "$(dirname "$0")"
echo "chain1 $$" >> out/pids.txt
( sh jam.sh gate1 Jam > out/gate1.out 2>&1; echo done > out/gate1.done ) &
( sh diag.sh h-rule Bar.A216.Hum "cart=hockey.arena.large;from=1;to=20" > out/h-rule.txt 2>&1
  sh diag.sh h-fo0 Bar.A216.Hum "cart=hockey.arena.large;from=1;to=20;fo=0" > out/h-fo0.txt 2>&1
  sh diag.sh t-rule Bar.A216.Trap "from=1;to=40;q=1" > out/t-rule.txt 2>&1
  sh diag.sh t-wall0 Bar.A216.Trap "from=1;to=40;q=1;wall=0" > out/t-wall0.txt 2>&1
  sh diag.sh t-net0 Bar.A216.Trap "from=1;to=40;q=1;net=0;fo=0" > out/t-net0.txt 2>&1
  echo done > out/diag1.done ) &
wait
