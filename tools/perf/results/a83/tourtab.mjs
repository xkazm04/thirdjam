// Lane A83: the bot-vs-bot (playerbot, JamTournament) table from tour-*.jsonl: node tourtab.mjs <label=file> ...
import { readFileSync } from 'node:fs';
const avg = (a) => a.reduce((s, x) => s + x, 0) / (a.length || 1); const r1 = (x) => Math.round(x * 10) / 10;
console.log('| arm | n | match s | goals | blue win % | shots | stalls (/min) | puck path m/s | human touch share of team % | human touch share of all % | unfinished |');
console.log('|---|---|---|---|---|---|---|---|---|---|---|');
for (const a of process.argv.slice(2)) {
  const [label, file] = a.split('=');
  const ms = readFileSync(file, 'utf8').split('\n').filter(Boolean).map((l) => JSON.parse(l));
  const half = (m) => m.touches.length / 2;
  const team = (m) => m.touches.slice(0, half(m)).reduce((s, x) => s + x, 0);
  const all = (m) => m.touches.reduce((s, x) => s + x, 0);
  console.log(`| ${label} | ${ms.length} | ${r1(avg(ms.map((m) => m.frames)) / 60)} | ${r1(avg(ms.map((m) => m.goals[0] + m.goals[1])))} | ${Math.round(100 * ms.filter((m) => m.winner === 0).length / ms.length)} | ${r1(avg(ms.map((m) => m.shots.reduce((s, x) => s + x, 0))))} | ${r1(avg(ms.map((m) => m.stalls)))} (${r1(avg(ms.map((m) => m.stalls / (m.play_frames / 3600))))}) | ${r1(avg(ms.map((m) => m.puck_path_m / (m.play_frames / 60))))} | ${r1(100 * avg(ms.map((m) => m.touches[m.player] / team(m))))} | ${r1(100 * avg(ms.map((m) => m.touches[m.player] / all(m))))} | ${ms.filter((m) => !m.over).length} |`);
}
