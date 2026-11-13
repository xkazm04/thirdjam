// Lane A83: the 5 v 5 play table from playtest .jsonl files: node agg.mjs <label=file.jsonl> ...
import { readFileSync } from 'node:fs';
const avg = (a) => a.reduce((s, x) => s + x, 0) / (a.length || 1);
const r1 = (x) => Math.round(x * 10) / 10;
console.log('| arm | cart | n | W-L | win % | match s | goals F/A | shots / opp shots | checks L/T (human) | knockdowns (human) | fouls | touch/min | human carry % | faceoffs w/o human touch % | max no touch s | passes rcv | idle spells | unfinished | puck loose % | mates % | opp % |');
console.log('|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|');
for (const a of process.argv.slice(2)) {
  const [label, file] = a.split('=');
  const ms = readFileSync(file, 'utf8').split('\n').filter(Boolean).map((l) => JSON.parse(l));
  const groups = {};
  for (const m of ms) (groups[m.cartridge.split('/').pop()] ??= []).push(m);
  for (const [cart, g] of Object.entries(groups)) {
    const n = g.length, w = g.filter((m) => m.result === 'win').length, l = g.filter((m) => m.result === 'loss').length;
    const fo = g.reduce((s, m) => s + m.faceoffs, 0), fow = g.reduce((s, m) => s + m.faceoffs_without_touch, 0);
    const wo = [0, 1, 2].map((k) => g.reduce((s, m) => s + m.wo_frames[k], 0)); const wt = wo[0] + wo[1] + wo[2] || 1;
    console.log(`| ${label} | ${cart} | ${n} | ${w}-${l} | ${Math.round(100 * w / n)} | ${r1(avg(g.map((m) => m.frames)) / 60)} | ${r1(avg(g.map((m) => m.goals[0])))} / ${r1(avg(g.map((m) => m.goals[1])))} | ${r1(avg(g.map((m) => m.shots)))} / ${r1(avg(g.map((m) => m.shots_opp)))} | ${r1(avg(g.map((m) => m.checks_landed)))} / ${r1(avg(g.map((m) => m.checks_taken)))} | ${r1(avg(g.map((m) => m.knockdowns)))} | ${r1(avg(g.map((m) => m.fouls)))} | ${r1(avg(g.map((m) => m.touches_per_min)))} | ${r1(100 * g.reduce((s, m) => s + m.carry_frames, 0) / g.reduce((s, m) => s + m.play_frames, 0))} | ${r1(100 * fow / fo)} (${fow}/${fo}) | ${r1(Math.max(...g.map((m) => m.max_no_touch)) / 60)} | ${r1(avg(g.map((m) => m.passes_received || 0)))} | ${r1(avg(g.map((m) => m.idle_spells)))} | ${g.filter((m) => !m.over).length} | ${r1(100 * wo[2] / wt)} | ${r1(100 * wo[0] / wt)} | ${r1(100 * wo[1] / wt)} |`);
  }
}
