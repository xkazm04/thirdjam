#!/usr/bin/env node
// lane A217 PASS FATES: the human's passes by fate, from raw/<label>.passes.txt (hand.mjs --passlog): node fates.mjs <label> [...]
import { readFileSync } from 'node:fs';
const pct = (k, n) => n ? (100 * k / n).toFixed(0) : '-';
for (const label of process.argv.slice(2)) {
  const ps = readFileSync(new URL(`raw/${label}.passes.txt`, import.meta.url), 'utf8').trim().split('\n').map((l) => Object.fromEntries(l.split(' ').slice(1).map((kv) => kv.split('=')).map(([k, v]) => [k, isNaN(+v) ? v : +v])));
  const N = ps.length;
  const by = (f) => ps.filter((p) => p.fate === f);
  console.log(`${label}: ${N} passes; mate ${pct(by('mate').length, N)} %, opp ${pct(by('opp').length, N)} %, loose ${pct(by('loose').length, N)} %, repassed ${pct(by('repass').length, N)} %`);
  for (const f of ['opp', 'mate']) {
    const s = by(f), n = s.length; if (!n) continue;
    const c = (fn) => `${pct(s.filter(fn).length, n)}`;
    console.log(`  ${f} (${n}): frames to the touch 0-6 ${c((p) => p.frames <= 6)} / 7-20 ${c((p) => p.frames > 6 && p.frames <= 20)} / 21-60 ${c((p) => p.frames > 20 && p.frames <= 60)} / >60 ${c((p) => p.frames > 60)} %;`
      + ` puck from the passer at the touch <3 m ${c((p) => p.puck_d < 3)} / 3-8 ${c((p) => p.puck_d >= 3 && p.puck_d <= 8)} / >8 ${c((p) => p.puck_d > 8)} %;`
      + ` a target at the release ${c((p) => p.to >= 0)} % (his distance <5 ${c((p) => p.to >= 0 && p.to_d < 5)} / 5-15 ${c((p) => p.to_d >= 5 && p.to_d <= 15)} / >15 ${c((p) => p.to_d > 15)} %);`
      + ` the target within 3 m of the puck at the touch ${c((p) => p.to >= 0 && p.to_puck_d >= 0 && p.to_puck_d < 3)} %;`
      + ` an opponent within 2 m of the passer at the release ${c((p) => p.press >= 0)} %, the taker was that man ${c((p) => p.taker_is_press === 1)} %; saucer ${c((p) => p.saucer === 1)} %`);
  }
}
