#!/bin/sh
# lane A208: one cell (level, rink) under several stick.pickup arms, n seeds each. Usage: arms.sh <prefix> <lv: pro|as|rk> <rink: regular|large> <n> "<label>=<set>|..."
cd "$(dirname "$0")"
P=$1; lv=$2; rink=$3; N=$4; ARMS=$5
case $lv in pro) cls=regular; lvl=pro;; as) cls=good; lvl=all_star;; rk) cls=novice; lvl=rookie;; esac
echo "chain-arms-$P $$" >> out/pids.txt
echo "$ARMS" | tr '|' '\n' | while IFS='=' read -r lab set; do
  [ -z "$lab" ] && continue
  if [ "$set" = "-" ]; then node curve.mjs $P-$lab $cls $lvl $rink $N "" --workers 2 --check closing > /dev/null
  else node curve.mjs $P-$lab $cls $lvl $rink $N "" --workers 2 --check closing --set "$set" > /dev/null; fi
  node -e "const r=require('fs').readFileSync('cells.jsonl','utf8').trim().split('\n').map(JSON.parse).filter(x=>x.label=='$P-$lab').pop();console.log('$P-$lab',r.n,'win',r.win,'gf',r.gf,'ga',r.ga,'carry',r.carry_pct,'shots',r.shots,'shots_opp',r.shots_opp,'touches',r.touches_pm)"
done
echo "chain-arms-$P done"
