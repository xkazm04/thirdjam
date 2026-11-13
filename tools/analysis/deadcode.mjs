#!/usr/bin/env node
// Lane A55: dead-code candidates in Game/Source/Jam* by a grep of declarations against uses (the editor build has no /VERBOSE link step we can ask for
// without changing the shared link line; a name-level count is the cheaper, honest proxy).
// A function name counts as used when it appears anywhere outside its own declaration and definition lines: in code, in a test, in a string
// (console commands, Python, Blueprint-visible UFUNCTIONs called by name). UFUNCTION / virtual / override / operator / constructors are excluded:
// reflection, the engine and Blueprints reach them without a C++ call site.
// Usage: node tools/analysis/deadcode.mjs   Prints DEAD lines (name, declaration site) and a JSON line last.
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..', '..');
const src = path.join(root, 'Game', 'Source');
function walk(d, out = []) {
  for (const e of fs.readdirSync(d, { withFileTypes: true })) {
    const p = path.join(d, e.name);
    if (e.isDirectory()) walk(p, out); else if (/\.(h|cpp|inl)$/.test(e.name)) out.push(p);
  }
  return out;
}
const files = walk(src).filter((f) => /[\\/]Jam[A-Za-z]+[\\/]/.test(f));
// extra places that call C++ by name: Python tools, config, json under tools/ and Game/Config
const extra = [];
for (const d of ['tools', path.join('Game', 'Config')]) {
  const full = path.join(root, d);
  if (!fs.existsSync(full)) continue;
  (function w(x) {
    for (const e of fs.readdirSync(x, { withFileTypes: true })) {
      const p = path.join(x, e.name);
      if (e.isDirectory()) { if (!['node_modules', 'analysis'].includes(e.name)) w(p); } else if (/\.(py|ini|mjs|ps1|json)$/.test(e.name) && fs.statSync(p).size < 2e6) extra.push(p);
    }
  })(full);
}
const strip = (t) => t.replace(/\/\*[\s\S]*?\*\//g, ' ').replace(/\/\/.*$/gm, ' ');
const texts = new Map(files.map((f) => [f, fs.readFileSync(f, 'utf8')]));
const allTokens = new Map();
const bump = (tok) => allTokens.set(tok, (allTokens.get(tok) || 0) + 1);
for (const [, t] of texts) for (const tok of strip(t).match(/[A-Za-z_]\w*/g) || []) bump(tok);
for (const f of extra) for (const tok of fs.readFileSync(f, 'utf8').match(/[A-Za-z_]\w*/g) || []) bump(tok);

const decls = [];
for (const [f, t] of texts) {
  if (!f.endsWith('.h')) continue;
  const lines = t.split(/\r?\n/);
  lines.forEach((l, i) => {
    const s = l.replace(/\/\/.*$/, '');
    if (/\b(virtual|override|operator|UFUNCTION|DECLARE_|GENERATED_BODY|return|typedef|using|#define)\b/.test(s)) return;
    const prev = (lines[i - 1] || '') + (lines[i - 2] || '');
    if (/UFUNCTION\s*\(/.test(prev)) return;
    const m = s.match(/^\s*(?:static\s+|inline\s+|FORCEINLINE\s+|constexpr\s+|[A-Z0-9_]+_API\s+|explicit\s+)*(?:const\s+)?[A-Za-z_][\w:<>,]*(?:\s*[*&])?\s+[*&]?([A-Z][A-Za-z0-9_]*)\s*\(/);
    if (!m) return;
    const name = m[1];
    if (/^(TEXT|UE_LOG|check|ensure|UPROPERTY|UCLASS|USTRUCT|UENUM|UMETA|IMPLEMENT_|DEFINE_|DECLARE_)/.test(name)) return;
    decls.push({ name, file: path.relative(root, f).split(path.sep).join('/'), line: i + 1 });
  });
}
// a name declared once, defined once (header + cpp) has 2 tokens; inline definitions have 1. Any third mention is a use.
const dead = [];
const seenName = new Set();
for (const d of decls) {
  if (seenName.has(d.name)) continue; seenName.add(d.name);
  const n = allTokens.get(d.name) || 0;
  const declCount = decls.filter((x) => x.name === d.name).length;
  // out-of-line definitions: a line that starts at column 0 and names Class::Name( (one per overload)
  const defRe = new RegExp(`^[A-Za-z_][^\\n;]*::${d.name}\\s*\\(`, 'gm');
  let defs = 0;
  for (const [f, t] of texts) if (f.endsWith('.cpp')) defs += (t.match(defRe) || []).length;
  if (n <= declCount + defs) dead.push({ ...d, mentions: n });
}
for (const d of dead) console.log(`DEAD ${d.name} ${d.file}:${d.line} (${d.mentions} mention${d.mentions === 1 ? '' : 's'} in all sources)`);
console.log(JSON.stringify({ schema: 'a55.deadcode/1', declarations: decls.length, candidates: dead.length }));
