#!/usr/bin/env node
// Lane A54: write docs/experiments/EYES-GRID.md from tools/qa/out/eyesgrid/<run>/{rows,findings,manifest}.json (run table.mjs first).
//   node tools/qa/eyesgrid/report.mjs --run h19 [--runs h19,h20]   (--runs: every grid so far, oldest first; the last gets the full tables, earlier ones a "what changed" diff)
import { readFileSync, writeFileSync, existsSync, readdirSync } from 'node:fs';
import { join, resolve, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';

const here = dirname(fileURLToPath(import.meta.url));
const root = resolve(here, '..', '..', '..');
const argv = process.argv.slice(2);
const val = (n, d) => { const i = argv.indexOf('--' + n); return i < 0 ? d : argv[i + 1]; };
const runs = (val('runs', val('run', 'h19'))).split(',');
const note = val('note', '');
const load = (run, f) => JSON.parse(readFileSync(join(root, 'tools', 'qa', 'out', 'eyesgrid', run, f), 'utf8'));
const meta = (run) => existsSync(join(root, 'tools', 'qa', 'out', 'eyesgrid', run, 'meta.json')) ? load(run, 'meta.json') : {};

const run = runs[runs.length - 1];
const rows = load(run, 'rows.json'), findings = load(run, 'findings.json'), manifest = load(run, 'manifest.json');
const m = meta(run);
const stillRel = (s) => `img/eyesgrid/${run}/${s}`;
const sevName = ['', 'cosmetic', 'minor', 'noticeable', 'serious', 'breaks the picture'];
const jsonFiles = readdirSync(join(root, 'docs', 'experiments', 'eyes')).filter((f) => f.startsWith(`a54-${run}-`) && !f.endsWith('-rows.json'));
const calls = jsonFiles.length;

// per still: worst severity of a finding (not a score row), number of findings of severity >= 3, readability overall, hud legibility
const per = new Map();
for (const mm of manifest) per.set(mm.still, { still: mm.still, max: 0, n3: 0, overall: null, legibility: null, text: '' });
for (const r of rows) {
  const p = per.get(r.still); if (!p) continue;
  if (r.overall !== undefined) p.overall = r.overall;
  else if (r.legibility !== undefined) p.legibility = r.legibility;
  else if (r.theme !== 'noise') { if (r.severity > p.max) { p.max = r.severity; p.text = r.text; } if (r.severity >= 3) p.n3++; }
}
const stills = [...per.values()].sort((a, b) => b.max - a.max || b.n3 - a.n3 || a.still.localeCompare(b.still));
// the ten worst: distinct themes first so ten stills do not all show the same fault
const ten = []; const seenTheme = new Map();
const themeOf = (s) => rows.filter((r) => r.still === s && r.severity === per.get(s).max && r.theme !== 'score' && r.theme !== 'noise').map((r) => r.theme)[0] ?? 'other';
for (const s of stills) { const th = themeOf(s.still); if ((seenTheme.get(th) ?? 0) >= 2) continue; seenTheme.set(th, (seenTheme.get(th) ?? 0) + 1); ten.push({ ...s, theme: th }); if (ten.length === 10) break; }

const L = [];
L.push(`# EYES-GRID: the shipped build photographed from every angle, judged by Gemini (lane A54, device A)`);
L.push('');
L.push(`Latest grid: **${run}** (${m.build ?? 'build not recorded'}). ${note}`);
L.push(`Tool: \`tools/qa/eyesgrid/\` (\`shoot.ps1\` one launch, \`grid.mjs\` the grid, \`table.mjs\` the findings, \`report.mjs\` this file); answers in \`docs/experiments/eyes/a54-${run}-*.json\`; stills in \`docs/experiments/img/eyesgrid/${run}/\`.`);
L.push(`**Advice, not instructions.** Gemini reads one still at a time: it cannot see motion, it calls film-set dressing "noise", it sometimes misreads a camera angle for a missing mesh. Every "wrong" or "right" below that I checked with my own eyes says so.`);
L.push('');
L.push(`## 1. What was photographed (${run})`);
L.push('');
L.push(`${manifest.length} stills, ${calls} Gemini calls (bug-hunt on every still, readability at 6 m on the game-camera stills, hud on every still with a HUD; 6 stills per call, flash model, no thinking).`);
L.push('');
L.push(m.how ?? '');
L.push('');
L.push(`## 2. Findings, deduplicated across cuts (sorted by severity, then by how many stills show it)`);
L.push('');
L.push('| # | Severity | Owner | Finding (theme) | Stills | Cuts | Gemini, one example |');
L.push('|---|---|---|---|---|---|---|');
findings.forEach((f, i) => {
  const ex = f.example.replace(/\|/g, '/').slice(0, 150);
  L.push(`| ${i + 1} | ${f.max_severity} (${sevName[f.max_severity]}) | ${f.owner} | ${f.theme.replace(/-/g, ' ')} | ${f.stills.length} | ${f.cuts.length} | ${ex} |`);
});
L.push('');
L.push('### Which stills carry each finding');
L.push('');
findings.forEach((f, i) => {
  const shown = f.stills.slice(0, 8).map((s) => `[${s.replace('.jpg', '')}](${stillRel(s)})`).join(', ');
  L.push(`${i + 1}. **${f.theme.replace(/-/g, ' ')}** (${f.owner}): ${shown}${f.stills.length > 8 ? `, and ${f.stills.length - 8} more` : ''}`);
});
L.push('');
L.push(`## 3. The ten worst stills`);
L.push('');
ten.forEach((t, i) => {
  L.push(`**${i + 1}. \`${t.still.replace('.jpg', '')}\`** severity ${t.max} (${t.theme.replace(/-/g, ' ')}): ${t.text.slice(0, 200)}`);
  L.push('');
  L.push(`![${t.still}](${stillRel(t.still)})`);
  L.push('');
});
L.push(`## 4. Every still, worst first`);
L.push('');
L.push('| Still | Worst severity | Findings of 3 or more | Readability (1-5) | HUD legibility (1-5) | The worst finding |');
L.push('|---|---|---|---|---|---|');
for (const s of stills) L.push(`| [${s.still.replace('.jpg', '')}](${stillRel(s.still)}) | ${s.max} | ${s.n3} | ${s.overall ?? ''} | ${s.legibility ?? ''} | ${s.text.replace(/\|/g, '/').slice(0, 110)} |`);
L.push('');
L.push(`## 5. Rows of severity 4 and 5 (still, preset, severity, the finding)`);
L.push('');
L.push('| Still | Preset | Sev | Finding |');
L.push('|---|---|---|---|');
for (const r of rows.filter((x) => x.severity >= 4 && x.theme !== 'score' && x.theme !== 'noise')) L.push(`| ${r.still.replace('.jpg', '')} | ${r.preset} | ${r.severity} | ${r.text.replace(/\|/g, '/').split(' ').slice(0, 14).join(' ')} |`);
L.push('');
L.push(`The complete table (every finding row of every preset, ${rows.length} rows) is \`tools/qa/out/eyesgrid/${run}/rows.json\`, copied to \`docs/experiments/eyes/a54-${run}-rows.json\`.`);
L.push('');
if (runs.length > 1) {
  L.push(`## 6. What changed since the last grid`);
  L.push('');
  for (let k = runs.length - 1; k >= 1; k--) {
    const a = runs[k - 1], b = runs[k];
    const fa = new Map(load(a, 'findings.json').map((f) => [f.theme, f])), fb = new Map(load(b, 'findings.json').map((f) => [f.theme, f]));
    L.push(`### ${a} to ${b} (${meta(a).build ?? a} to ${meta(b).build ?? b})`);
    L.push('');
    L.push('| Finding | ' + a + ' (stills, max severity) | ' + b + ' (stills, max severity) | Change |');
    L.push('|---|---|---|---|');
    for (const th of new Set([...fa.keys(), ...fb.keys()])) {
      const x = fa.get(th), y = fb.get(th);
      const ch = !x ? 'new' : !y ? 'gone' : (y.stills.length < x.stills.length ? 'fewer' : y.stills.length > x.stills.length ? 'more' : 'same');
      L.push(`| ${th.replace(/-/g, ' ')} | ${x ? `${x.stills.length}, ${x.max_severity}` : '-'} | ${y ? `${y.stills.length}, ${y.max_severity}` : '-'} | ${ch} |`);
    }
    L.push('');
  }
}
writeFileSync(join(root, 'docs', 'experiments', 'EYES-GRID.md'), L.join('\n'));
writeFileSync(join(root, 'docs', 'experiments', 'eyes', `a54-${run}-rows.json`), JSON.stringify(rows));
console.log('wrote EYES-GRID.md', { stills: manifest.length, findings: findings.length, ten: ten.map((t) => t.still) });
