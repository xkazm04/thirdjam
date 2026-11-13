#!/usr/bin/env node
// Lane A54: parse every docs/experiments/eyes/a54-<run>-*.json into rows (still, preset, severity, finding) and cluster them into deduplicated findings.
//   node tools/qa/eyesgrid/table.mjs --run h19            -> tools/qa/out/eyesgrid/<run>/rows.json, findings.json and a markdown table on stdout
// Severity (1 cosmetic .. 5 breaks the picture): bug-hunt = the finding's own severity; readability = 6 - overall (an element scored 1 counts as 4);
// hud = 6 - legibility, and each named collision is a row of severity 3. Gemini's words are kept; the owner and theme come from THEMES below (regex on the words).
import { readFileSync, readdirSync, writeFileSync, existsSync } from 'node:fs';
import { join, resolve, dirname, basename } from 'node:path';
import { fileURLToPath } from 'node:url';

const here = dirname(fileURLToPath(import.meta.url));
const root = resolve(here, '..', '..', '..');
const argv = process.argv.slice(2);
const run = argv.includes('--run') ? argv[argv.indexOf('--run') + 1] : 'h19';
const jsonDir = join(root, 'docs', 'experiments', 'eyes');
const outDir = join(root, 'tools', 'qa', 'out', 'eyesgrid', run);

// theme id, owner, regex over the finding text (first match wins); a row that matches none goes to "other".
export const THEMES = JSON.parse(readFileSync(join(here, 'themes.json'), 'utf8'));

const words = (s, n = 14) => String(s ?? '').replace(/\s+/g, ' ').trim().split(' ').slice(0, n).join(' ');
const rows = [];
for (const f of readdirSync(jsonDir).filter((x) => x.startsWith(`a54-${run}-`)).sort()) {
  const j = JSON.parse(readFileSync(join(jsonDir, f), 'utf8'));
  for (const r of j.results ?? []) {
    if (!r.ok || !r.answer?.stills) continue;
    r.answer.stills.forEach((a, i) => {
      const still = basename(r.stills[i]?.path ?? `?${i}`);
      const add = (severity, text, extra = {}) => rows.push({ still, preset: r.preset, severity, text: words(text, 28), ...extra });
      if (r.preset === 'bug-hunt') for (const x of a.findings ?? []) add(x.severity, `${x.what} (${x.where})`, { confidence: x.confidence });
      else if (r.preset === 'readability') {
        const e = a.elements ?? {};
        const low = Object.entries(e).filter(([, v]) => v.score <= 1);
        add(Math.max(6 - a.overall, low.length ? 4 : 1), `overall ${a.overall}; first look: ${a.first_look}`, { overall: a.overall });
        for (const [k, v] of low) add(4, `${k} scored 1: ${v.evidence}`);
        for (const n of a.noise ?? []) add(2, `noise: ${n}`);
      } else if (r.preset === 'hud') {
        add(6 - a.legibility, `legibility ${a.legibility}, smallest text ${a.smallest_text_px} px`, { legibility: a.legibility });
        for (const c of a.collisions ?? []) add(3, `collision: ${c}`);
        for (const m of a.missing ?? []) add(2, `missing: ${m}`);
      }
    });
  }
}
for (const r of rows) {
  if (r.overall !== undefined || r.legibility !== undefined) { r.theme = 'score'; r.owner = '-'; continue; }   // score-only rows rank stills, they are not findings
  if (/^noise:/.test(r.text)) { r.theme = 'noise'; r.owner = '-'; continue; }
  const t = THEMES.find((th) => new RegExp(th.re, "i").test(`${r.still} ${r.text}`));
  r.theme = t ? t.id : 'other'; r.owner = t ? t.owner : 'unassigned';
}
rows.sort((a, b) => b.severity - a.severity || a.still.localeCompare(b.still));
const byTheme = new Map();
for (const r of rows.filter((x) => x.theme !== 'score' && x.theme !== 'noise')) {
  if (!byTheme.has(r.theme)) byTheme.set(r.theme, { theme: r.theme, owner: r.owner, max: 0, stills: new Set(), cuts: new Set(), rows: [] });
  const g = byTheme.get(r.theme);
  g.max = Math.max(g.max, r.severity); g.stills.add(r.still); g.cuts.add(r.still.split('-')[0]); g.rows.push(r);
}
const findings = [...byTheme.values()].map((g) => ({ theme: g.theme, owner: g.owner, max_severity: g.max, stills: [...g.stills], cuts: [...g.cuts], n_rows: g.rows.length,
  example: g.rows.sort((a, b) => b.severity - a.severity)[0].text })).sort((a, b) => b.max_severity - a.max_severity || b.stills.length - a.stills.length);
writeFileSync(join(outDir, 'rows.json'), JSON.stringify(rows, null, 1));
writeFileSync(join(outDir, 'findings.json'), JSON.stringify(findings, null, 1));
console.log(`rows ${rows.length}, findings ${findings.length}`);
for (const f of findings) console.log(`${f.max_severity} | ${f.owner} | ${f.theme} | ${f.stills.length} stills | ${f.example}`);
