// A108: one line per matrix run folder: verdict, fails with facts. node summarize-matrix.mjs [prefix]
import fs from 'node:fs'; import path from 'node:path';
const dir = path.resolve(path.dirname(new URL(import.meta.url).pathname.replace(/^\/([A-Za-z]:)/, '$1')), '..', 'out', 'accept');
const pre = process.argv[2] || 'm-';
for (const n of fs.readdirSync(dir).filter(n => n.startsWith(pre)).sort()) {
  const f = path.join(dir, n, 'verdict.txt'); if (!fs.existsSync(f)) { console.log(n, 'no verdict yet'); continue; }
  const t = fs.readFileSync(f, 'utf8'); const last = t.trim().split('\n').pop(); let j; try { j = JSON.parse(last); } catch { console.log(n, 'unparsable'); continue; }
  const bad = []; for (const s of j.steps || []) for (const v of s.verbs || []) if (!v.ok) bad.push(v.verb + ': ' + String(v.facts).slice(0, 140));
  console.log(n, j.verdict, 'warn=' + (j.warnings || []).length, bad.length ? 'FAIL ' + bad.join(' | ') : '');
}
