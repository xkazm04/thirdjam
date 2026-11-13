#!/usr/bin/env node
// Lane A55: include candidates. For every .cpp under Game/Source/Jam*, every #include whose header declares nothing the .cpp names.
// Heuristic only: a candidate is removed by hand and the compile proves it. Engine headers are resolved under UE_ROOT (default UE_5.9).
// Usage: node tools/analysis/includes.mjs [--json]   Prints one CAND line per candidate and a tl-style JSON line last.
import fs from 'node:fs';
import path from 'node:path';

const root = path.resolve(path.dirname(new URL(import.meta.url).pathname.replace(/^\/([A-Z]:)/, '$1')), '..', '..');
const src = path.join(root, 'Game', 'Source');
const ue = process.env.UE_ROOT || 'C:/Program Files/Epic Games/UE_5.9';

function walk(d, out = []) {
  for (const e of fs.readdirSync(d, { withFileTypes: true })) {
    const p = path.join(d, e.name);
    if (e.isDirectory()) walk(p, out); else out.push(p);
  }
  return out;
}
const files = walk(src).filter((f) => /[\\/]Jam[A-Za-z]+[\\/]/.test(f));
const byName = new Map();
for (const f of files) if (/\.(h|inl)$/.test(f)) byName.set(path.basename(f), f);

const engineRoots = ['Engine/Source/Runtime', 'Engine/Source/Editor', 'Engine/Plugins'].map((r) => path.join(ue, r));
const engineCache = new Map();
function findEngine(inc) {
  if (engineCache.has(inc)) return engineCache.get(inc);
  let hit = null;
  const stack = [...engineRoots];
  // breadth search restricted to Public/Classes folders: cheap enough for a few dozen headers
  const seen = new Set();
  while (stack.length && !hit) {
    const d = stack.shift();
    if (seen.has(d)) continue; seen.add(d);
    let ents; try { ents = fs.readdirSync(d, { withFileTypes: true }); } catch { continue; }
    for (const e of ents) {
      if (!e.isDirectory()) continue;
      const p = path.join(d, e.name);
      if (e.name === 'Public' || e.name === 'Classes') { const c = path.join(p, inc); if (fs.existsSync(c)) { hit = c; break; } }
      if (!['Private', 'Internal', 'Tests', 'Resources', 'Content', 'Binaries', 'Intermediate', 'ThirdParty', 'Shaders'].includes(e.name)) stack.push(p);
    }
  }
  engineCache.set(inc, hit);
  return hit;
}

function strip(t) { return t.replace(/\/\*[\s\S]*?\*\//g, ' ').replace(/\/\/.*$/gm, ' ').replace(/"(?:[^"\\\n]|\\.)*"/g, '""'); }
function declared(h) {
  const t = strip(fs.readFileSync(h, 'utf8'));
  const names = new Set();
  const re = [
    /\b(?:class|struct|union)\s+(?:[A-Z0-9_]+_API\s+)?([A-Za-z_]\w*)\s*(?:final\s*)?[:{]/g,
    /\benum\s+(?:class\s+)?([A-Za-z_]\w*)/g,
    /\bnamespace\s+([A-Za-z_]\w*)/g,
    /#define\s+([A-Za-z_]\w*)/g,
    /\b(?:using|typedef)\s+(?:[^;]*\s)?([A-Za-z_]\w*)\s*(?:=|;)/g,
    /^\s*(?:[A-Z0-9_]+_API\s+)?(?:static\s+|inline\s+|FORCEINLINE\s+|constexpr\s+)*[A-Za-z_][\w:<>,\s*&]*?\s+\**&?([A-Za-z_]\w*)\s*\(/gm,
    /\bDECLARE_[A-Z_]*\(\s*([A-Za-z_]\w*)/g,
  ];
  for (const r of re) for (const m of t.matchAll(r)) names.add(m[1]);
  for (const k of ['if', 'for', 'while', 'switch', 'return', 'sizeof', 'class', 'struct', 'namespace', 'UE', 'Private', 'Detail', 'Impl']) names.delete(k);
  return names;
}
const declCache = new Map();
function decl(h) { if (!declCache.has(h)) declCache.set(h, declared(h)); return declCache.get(h); }

const out = [];
for (const cpp of files.filter((f) => f.endsWith('.cpp'))) {
  const raw = fs.readFileSync(cpp, 'utf8');
  const body = strip(raw.replace(/^\s*#include.*$/gm, ''));
  const ids = new Set(body.match(/[A-Za-z_]\w*/g) || []);
  const own = path.basename(cpp, '.cpp') + '.h';
  const lines = raw.split(/\r?\n/);
  lines.forEach((l, i) => {
    const m = l.match(/^\s*#include\s+"([^"]+)"/);
    if (!m) return;
    const inc = m[1];
    if (path.basename(inc) === own || inc === 'CoreMinimal.h' || inc.endsWith('.inl') || inc.endsWith('.generated.h')) return;
    const h = byName.get(path.basename(inc)) || findEngine(inc);
    if (!h) { out.push({ file: path.relative(root, cpp), line: i + 1, inc, why: 'unresolved' }); return; }
    // names a header provides that the declaration scan cannot see (platform typedefs behind COMPILED_PLATFORM_HEADER, extern globals, include guards)
    const KNOWN = { 'HAL/PlatformTime.h': ['FPlatformTime'], 'HAL/PlatformMisc.h': ['FPlatformMisc'], 'Engine/Engine.h': ['GEngine'], 'UObject/Package.h': ['GetTransientPackage'],
      'Windows/AllowWindowsPlatformTypes.h': null, 'Windows/HideWindowsPlatformTypes.h': null, 'Windows/WindowsHWrapper.h': null };
    if (inc in KNOWN && (KNOWN[inc] === null || KNOWN[inc].some((n) => ids.has(n)))) return;
    const names = decl(h);
    const used = [...names].filter((n) => ids.has(n));
    if (used.length === 0) out.push({ file: path.relative(root, cpp).split(path.sep).join('/'), line: i + 1, inc, why: 'nothing declared there is named' });
  });
}
for (const o of out) console.log(`CAND ${o.file}:${o.line} ${o.inc} (${o.why})`);
console.log(JSON.stringify({ schema: 'a55.includes/1', candidates: out.length }));
