// A108/A131: one line per matrix run folder, classified. node summarize-matrix.mjs [prefix] [--md]
// A run is PASS (every row ok), PASS+DESIGN (rows that fail or differ only by a named by-design entry of mode-table.json), MACHINE (exit 3: the hand was late, not the game),
// or FAIL (rows no table entry explains). A folder's slug comes from its name: m-mode-<slug>[-r1|-t1|-d1], m-tier-<tier>, m-reg*, m-regcart.
import fs from 'node:fs'; import path from 'node:path';
const here = path.dirname(new URL(import.meta.url).pathname.replace(/^\/([A-Za-z]:)/, '$1'));
const dir = path.resolve(here, '..', 'out', 'accept');
const args = process.argv.slice(2); const md = args.includes('--md'); const pre = args.find((a) => !a.startsWith('--')) || 'm-';
const table = JSON.parse(fs.readFileSync(path.join(here, 'mode-table.json'), 'utf8'));
const kv = (s) => Object.fromEntries([...String(s).matchAll(/(\w+)=(-?[\d.]+)/g)].map((m) => [m[1], +m[2]]));
function entryFor(name, verb) {
  const m = name.match(/^m-mode-(.+?)(?:-(?:r\d+|t\d+|d\d+))*$/); const t = name.match(/^m-tier-(.+?)(?:-(?:r\d+|t\d+|d\d+))*$/);
  const c = name.match(/^m-(regcart)(?:-(?:r\d+|t\d+|d\d+))*$/);
  const e = m ? table.modes?.[m[1]]?.[verb] : t ? table.tiers?.[t[1]]?.[verb] : c ? table.cuts?.[c[1]]?.[verb] : undefined; return e;
}
const rows = [];
for (const n of fs.readdirSync(dir).filter((n) => n.startsWith(pre)).sort()) {
  const f = path.join(dir, n, 'verdict.txt'); if (!fs.existsSync(f)) { rows.push({ n, v: 'NO-VERDICT', note: '' }); continue; }
  const t = fs.readFileSync(f, 'utf8'); const last = t.trim().split('\n').pop(); let j; try { j = JSON.parse(last); } catch { rows.push({ n, v: 'UNPARSABLE', note: '' }); continue; }
  const failing = (j.steps || []).flatMap((s) => (s.verbs || []).filter((v) => !v.ok));
  const allDesign = failing.length > 0 && failing.every((v) => entryFor(n, v.verb)?.by_design);   // a row the mode's rules remove fails whatever the machine did
  if (j.verdict === 'machine' && !allDesign) { const why = []; for (const s of j.steps || []) for (const v of s.verbs || []) if (!v.ok && v.machine) why.push(`${v.verb}: ${v.machine}`); rows.push({ n, v: 'MACHINE', note: [...new Set(why)].join(' | ').slice(0, 200) }); continue; }
  const design = [], bad = [];
  for (const s of j.steps || []) for (const v of s.verbs || []) {
    const e = entryFor(n, v.verb);
    if (v.ok) { if (e?.by_design) design.push(`${v.verb} passes (by-design row)`); continue; }
    if (e?.by_design) { design.push(`${v.verb}: ${e.by_design.split(':')[0]}`); continue; }
    if (e?.accept) { const f = kv(v.facts); if (e.accept.every((c) => f[c.key] !== undefined && (c.min === undefined || f[c.key] >= c.min) && (c.max === undefined || f[c.key] <= c.max))) { design.push(`${v.verb}: ${e.accept.filter((c) => c.max !== undefined).map((c) => `${c.key} ${f[c.key]} <= ${c.max}`).join(', ')}`); continue; } }
    bad.push(`${v.verb}${v.machine ? '(machine)' : ''}: ${String(v.facts).slice(0, 150)}`);
  }
  const warn = (j.warnings || []).length;
  rows.push({ n, v: bad.length ? 'FAIL' : design.length ? 'PASS+DESIGN' : 'PASS', note: [...bad.map((b) => 'UNEXPLAINED ' + b), ...design].join(' | '), warn });
}
if (md) { console.log('| run | verdict | named differences / unexplained rows |\n|---|---|---|'); for (const r of rows) console.log(`| ${r.n} | ${r.v} | ${r.note.replace(/\|/g, '/')} |`); }
else for (const r of rows) console.log(r.n, r.v, r.warn ? `warn=${r.warn}` : '', r.note);
