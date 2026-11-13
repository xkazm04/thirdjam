// Lane A100 (track F0, REVIEW-1): same-team changes of the carrier of record with no flight between (the puck moved under 1.5 m, the gap 3 frames or less).
import { createReadStream } from 'node:fs';
import { createInterface } from 'node:readline';
import { parseTrace } from '../../../tools/playtest/shape.mjs';
const rl = createInterface({ input: createReadStream(process.argv[2]) });
let human = -1, team = [], last = null, matches = 0; const n = { 'mate takes from human': 0, 'human takes from mate': 0, 'mate takes from mate': 0, 'opp takes from opp': 0 }; let frames = 0;
for await (const l of rl) {
  if (l.includes('thirdjam.playtest/1')) { matches++; human = -1; last = null; continue; }
  if (!l.includes('TRACE f=')) continue;
  const T = parseTrace(l); if (!T || !T.sk.length) continue;
  if (human < 0) { human = T.sk.findIndex((k) => k.role === 'H'); team = T.sk.map((k) => k.team); }
  if (T.frozen || T.fo) { last = null; continue; }
  frames++;
  if (T.car >= 0) {
    if (last && last.car !== T.car && team[last.car] === team[T.car] && T.f - last.f <= 3 && Math.hypot(T.px - last.px, T.py - last.py) < 1.5 && last.run >= 20) {
      const k = team[T.car] !== team[human] ? 'opp takes from opp' : last.car === human ? 'mate takes from human' : T.car === human ? 'human takes from mate' : 'mate takes from mate';
      n[k]++;
    }
    last = { car: T.car, f: T.f, px: T.px, py: T.py, run: last && last.car === T.car ? last.run + 1 : 1 };
  }
}
console.log(JSON.stringify({ log: process.argv[2], matches, per_match: Object.fromEntries(Object.entries(n).map(([k, v]) => [k, +(v / matches).toFixed(2)])) }));
