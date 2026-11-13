#!/usr/bin/env node
// Lane A55: count and classify compiler / analyzer warnings in a UBT log (MSVC "file(line[,col]) : warning Cnnnn: text", clang "file:line:col: warning: text [-Wx]").
// Dedupes by file + line + code + text (a header warning repeats once per translation unit). Only Game/Source/Jam* files are counted; the rest is reported as "outside".
// Usage: node tools/analysis/warnings.mjs <ubt log> [--list]   Prints per-module, per-file, per-code counts, then a JSON line last.
import fs from 'node:fs';

const [logPath, ...flags] = process.argv.slice(2);
if (!logPath || !fs.existsSync(logPath)) { console.log('FAIL warnings no log given or not found'); console.log(JSON.stringify({ schema: 'a55.warnings/1', verdict: 'could_not_run' })); process.exit(2); }
const list = flags.includes('--list');

// classification: real = a defect class that can misbehave at run time; smell = legal but suspicious; noise = style or false-positive prone
const CLASS = {
  C4701: 'real', C4703: 'real', C6001: 'real', C6011: 'real', C28182: 'real', C6385: 'real', C6386: 'real', C6201: 'real', C6200: 'real', C6297: 'real',
  C6031: 'smell', C6246: 'smell', C6244: 'smell', C4456: 'smell', C4457: 'smell', C4458: 'smell', C4459: 'smell', C4244: 'smell', C4267: 'smell', C4389: 'smell',
  C4018: 'smell', C4100: 'noise', C4189: 'smell', C4996: 'smell', C6326: 'noise', C6287: 'smell', C6237: 'smell', C6239: 'smell', C6240: 'smell', C6262: 'noise',
  C6294: 'smell', C6295: 'smell', C6340: 'smell', C26495: 'smell', C26451: 'noise', C26812: 'noise', C26439: 'noise', C26800: 'real', C6255: 'noise', C6387: 'real', C6336: 'noise',
  'core.CallAndMessage': 'real', 'core.NullDereference': 'real', 'core.DivideZero': 'real', 'deadcode.DeadStores': 'smell', 'security.FloatLoopCounter': 'noise',
};
const text = fs.readFileSync(logPath, 'utf8');
const re = /^\s*(?:\[\d+\/\d+\][^\n]*?\s)?([A-Za-z]:[^\n(:]+?\.(?:cpp|h|inl|hpp))(?:\((\d+)(?:,(\d+))?\)\s*:|:(\d+):(\d+):)\s*warning\s*:?\s*(C\d+)?:?\s*([^\n]*)$/gm;
const seen = new Map();
let outside = 0, raw = 0;
for (const m of text.matchAll(re)) {
  raw++;
  const file = m[1].split('\\').join('/');
  const line = Number(m[2] || m[4]);
  let code = m[6];
  let msg = m[7].trim();
  if (!code) { const w = msg.match(/\[(-W[\w-]+|[\w.-]+)\]\s*$/); code = w ? w[1] : 'clang'; }
  const at = file.search(/Game\/Source\/Jam/);
  if (at < 0) { outside++; continue; }
  const rel = file.slice(at);
  const key = `${rel}:${line}:${code}:${msg}`;
  if (!seen.has(key)) seen.set(key, { file: rel, line, code, msg, cls: CLASS[code] || (code.startsWith('-W') && /uninitialized|null|use-after|core\./.test(code + msg) ? 'real' : 'unclassified') });
}
const items = [...seen.values()].sort((a, b) => a.file.localeCompare(b.file) || a.line - b.line);
const count = (f) => { const o = {}; for (const i of items) { const k = f(i); o[k] = (o[k] || 0) + 1; } return Object.fromEntries(Object.entries(o).sort((a, b) => b[1] - a[1])); };
const byModule = count((i) => i.file.split('/')[2]);
const byFile = count((i) => i.file);
const byCode = count((i) => i.code);
const byClass = count((i) => i.cls);
console.log(`INFO warnings: ${raw} raw lines, ${items.length} unique in Game/Source/Jam*, ${outside} outside`);
for (const [k, v] of Object.entries(byModule)) console.log(`MODULE ${v} ${k}`);
for (const [k, v] of Object.entries(byCode)) console.log(`CODE ${v} ${k} (${CLASS[k] || 'unclassified'})`);
for (const [k, v] of Object.entries(byFile)) console.log(`FILE ${v} ${k}`);
if (list) for (const i of items) console.log(`W ${i.cls} ${i.code} ${i.file}:${i.line} ${i.msg}`);
console.log(JSON.stringify({ schema: 'a55.warnings/1', log: logPath, raw, unique: items.length, outside, byModule, byCode, byClass }));
