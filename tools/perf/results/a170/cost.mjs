#!/usr/bin/env node
// lane A170 DIAL COST: what the human lives through, per match, on the same seeds (raw/<label>.jsonl rows of the playtest line)
// node cost.mjs <label> [<label> ...]
import { readFileSync } from 'node:fs';
const med = (a) => { const s = [...a].sort((x, y) => x - y); return s.length ? s[Math.floor(s.length / 2)] : 0; };
for (const l of process.argv.slice(2)) {
  const ms = readFileSync(new URL(`raw/${l}.jsonl`, import.meta.url), 'utf8').trim().split('\n').map(JSON.parse);
  const N = ms.length, S = (k) => ms.reduce((a, m) => a + (Number(m[k]) || 0), 0), pm = (k) => (S(k) / N).toFixed(2);
  const play = S('play_frames');
  const se = (k) => { const mu = S(k) / N; const v = ms.reduce((a, m) => a + ((Number(m[k]) || 0) - mu) ** 2, 0) / (N - 1); return Math.sqrt(v / N).toFixed(2); };
  console.log(`${l} n=${N} win=${(100 * ms.filter((m) => m.result === 'win').length / N).toFixed(1)} | human knockdowns/match ${pm('knockdowns')} (se ${se('knockdowns')}) | checks taken ${pm('checks_taken')} | stumbles ${pm('stumbles')} | all skaters down % of play x10 skaters ${(100 * S('downed_frames') / play / 10).toFixed(2)} | touches/play-min ${(S('touches') / (play / 3600)).toFixed(2)} | carry % of play ${(100 * S('carry_frames') / play).toFixed(1)} | carries 3 s+/match ${pm('carries_3s')} | shots against/match ${pm('shots_opp')} | goals against/match ${(ms.reduce((a, m) => a + m.goals[1], 0) / N).toFixed(2)} | his shots ${pm('shots')} | idle spells ${pm('idle_spells')} | max no-touch median s ${(med(ms.map((m) => m.max_no_touch)) / 60).toFixed(1)}`);
}
