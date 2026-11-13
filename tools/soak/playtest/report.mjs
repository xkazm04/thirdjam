#!/usr/bin/env node
// Writes docs/experiments/SOAK-PLAYTEST.md from the soak results (lane A53): "What A3 should change" as numbers, the table, the fault list, matches done, memory.
//   node tools/soak/playtest/report.mjs [--out tools/soak/playtest/out] [--note "text for the status line"] [--final]
import { writeFileSync } from 'node:fs';
import { execFileSync } from 'node:child_process';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { load, cells, table, faults, faultTable, TARGET } from './aggregate.mjs';

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..', '..');
function arg(name, dflt) { const i = process.argv.indexOf(`--${name}`); return i > 0 && i + 1 < process.argv.length ? process.argv[i + 1] : dflt; }
const out = resolve(arg('out', join(here, 'out')));
const r1 = (x) => (Math.round(x * 10) / 10).toString();
let mem = '?';
try { mem = (Number(execFileSync('powershell', ['-NoProfile', '-Command', '(Get-CimInstance Win32_OperatingSystem).FreePhysicalMemory'], { encoding: 'utf8' }).trim()) / 1048576).toFixed(1); } catch { /* keep ? */ }

const rows = load(out);
const cs = cells(rows);
const fs = faults(rows);
const n = rows.filter((r) => r.m).length;
const crashes = rows.filter((r) => r.crash).length;
const reg = cs.filter((c) => c.cart === 'hockey.regular');
const off = reg.filter((c) => c.target !== undefined).map((c) => {
  const d = c.win_pct - c.target;
  return `- ${c.cls} at ${c.lvl} on hockey.regular: ${r1(c.win_pct)} % (95 % ${r1(c.lo)} - ${r1(c.hi)}, n ${c.n}) against ${c.target}: ${d >= 0 ? '+' : ''}${r1(d)} points, ${c.flag === 'ok' ? 'the interval holds the target' : `interval ${c.flag}`}`;
});
const stamp = new Date().toString().replace(/ GMT.*/, '');
const md = `# Playtester soak (lane A53, device A)

Status: ${arg('final', '') ? 'FINAL' : 'refreshed'} ${stamp}. ${n} matches done (${crashes} crashed), free memory at this refresh ${mem} GB. Nothing here changes a profile or a sim number.
${arg('note', '')}

How it ran: \`node tools/soak/playtest/soak.mjs --until 15:00 --rounds 20\` (8 headless \`UnrealEditor-Cmd -run=JamTournament -player=\` processes at a time, 50 seeds each, \`tools/playtest/run.mjs\`'s
command line). Seeds 1-100 of each 150 are \`hockey.regular\`, 101-150 \`hockey.boost\`. Cells: 3 classes x 3 levels. A match is deterministic, so each seed is played once; 50 matches take about a minute
after a start-up of about 90 s per process. Tables: \`node tools/soak/playtest/aggregate.mjs\`. Raw results: \`tools/soak/playtest/out/results/\`.

## 1. What A3 should change (numbers; the targets are R-0029 point 1, on hockey.regular)

${off.join('\n')}

Where the brief and the measurement differ: the brief lists targets 80/85, 50/55, 30/25; R-0029 itself says 85 / about 50 / about 25 on the regular rink, which is what the flags use. The soak also
plays one third of its seeds on hockey.boost, which is much easier for the human than regular (see the boost rows); the pooled rows are therefore not the curve.

## 2. The table (class x level; hockey.regular rows are compared with the target, boost and pooled rows are information)

${table(cs)}

## 3. Faults (deduplicated by the first distinct line; reproduce with the command in the row, which needs the matching seed on that cartridge)

${faultTable(fs)}
`;
writeFileSync(join(repo, 'docs', 'experiments', 'SOAK-PLAYTEST.md'), md);
console.log(`wrote docs/experiments/SOAK-PLAYTEST.md: ${n} matches, ${fs.length} fault groups`);
