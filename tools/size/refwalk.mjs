#!/usr/bin/env node
// Lane A161 (track Q3, evolution 6): a reference walk over the project's packages, without an editor (restored by lane A175: the A161 landing
// did not carry it).
// Every .uasset / .umap is read as bytes; each long package name it carries (its import table's package names and soft paths are plain
// ASCII strings in the package's name table) is an edge. Over-approximates (a name that only appears in a string still counts), which is
// the safe direction for the question asked: "can a shipped package reach X?". Roots: the MapsToCook and DirectoriesToAlwaysCook of
// Game/Config/DefaultGame.ini (minus the DirectoriesToNeverCook entries, folders or single packages) plus the long package names written in
// the shipped C++ (Game/Source, test files and Tests folders excluded).
//   node tools/size/refwalk.mjs [--candidates <regex>] [--json out.json]
// Exit 0 if no shipped root reaches a candidate and no cooked package names a never-cook package (the cooker's "Content is missing from
// cook" error), 1 otherwise. The code's own generated_b2 class paths (HokejGameMode.cpp) are counted apart: only test cuts select them.
import fs from 'node:fs';
import path from 'node:path';

const repo = path.resolve(path.dirname(new URL(import.meta.url).pathname.replace(/^\/([A-Za-z]:)/, '$1')), '..', '..');
const game = path.join(repo, 'Game');
const args = process.argv.slice(2);
const opt = (k, d) => { const i = args.indexOf(k); return i >= 0 ? args[i + 1] : d; };
const candRe = new RegExp(opt('--candidates', '^/JamAssets/(Skaters/[^/]*(TB2|SkaterGen|P4Lab)[^/]*|Look/MI_Bcast_SkaterP4|Look/P4Lab/MI?_P4_FigureE?)$'));
const jsonOut = opt('--json', '');

const mounts = [
  ['/Game/', path.join(game, 'Content')],
  ['/JamAssets/', path.join(game, 'Plugins', 'JamAssets', 'Content')],
];
const pkgs = new Map();   // long package name -> file
function walk(dir, mount, base) {
  for (const e of fs.readdirSync(dir, { withFileTypes: true })) {
    const p = path.join(dir, e.name);
    if (e.isDirectory()) { if (e.name !== '__ExternalActors__' && e.name !== '__ExternalObjects__') walk(p, mount, base); continue; }
    if (!/\.(uasset|umap)$/.test(e.name)) continue;
    const rel = path.relative(base, p).split(path.sep).join('/').replace(/[.](uasset|umap)$/, '');
    pkgs.set(mount + rel, p);
  }
}
for (const [m, d] of mounts) if (fs.existsSync(d)) walk(d, m, d);

const pathRe = /\/(?:Game|JamAssets|Engine)\/[A-Za-z0-9_\-\/]+/g;
const edges = new Map();
for (const [name, file] of pkgs) {
  const s = fs.readFileSync(file).toString('latin1');
  const out = new Set();
  for (const m of s.matchAll(pathRe)) { const t = m[0].replace(/\/+$/, ''); if (t !== name && pkgs.has(t)) out.add(t); }
  edges.set(name, out);
}

// Roots from the packaging settings.
const ini = fs.readFileSync(path.join(game, 'Config', 'DefaultGame.ini'), 'utf8');
const grab = (key, field) => [...ini.matchAll(new RegExp('^[+]' + key + '=[(]' + field + '="([^"]+)"[)]', 'gm'))].map(m => m[1]);
const maps = grab('MapsToCook', 'FilePath');
const always = grab('DirectoriesToAlwaysCook', 'Path');
const never = grab('DirectoriesToNeverCook', 'Path');
const isNever = n => never.some(d => n === d || n.startsWith(d + '/'));
const roots = new Map();   // package -> why
for (const m of maps) if (pkgs.has(m)) roots.set(m, 'MapsToCook');
for (const d of always) for (const n of pkgs.keys()) if ((n === d || n.startsWith(d + '/')) && !isNever(n)) roots.set(n, 'AlwaysCook ' + d);
// Code roots: long package names in shipped C++.
const codeRefs = new Map();   // package -> [file:line]
function walkSrc(dir) {
  for (const e of fs.readdirSync(dir, { withFileTypes: true })) {
    const p = path.join(dir, e.name);
    if (e.isDirectory()) { if (!/^(Tests?|Intermediate)$/.test(e.name)) walkSrc(p); continue; }
    if (!/\.(cpp|h)$/.test(e.name) || /Tests?\.cpp$/.test(e.name)) continue;
    fs.readFileSync(p, 'utf8').split(/\r?\n/).forEach((line, i) => {
      for (const m of line.matchAll(pathRe)) {
        const t = m[0].replace(/\/+$/, '');
        if (!pkgs.has(t)) continue;
        if (!codeRefs.has(t)) codeRefs.set(t, []);
        codeRefs.get(t).push(path.relative(repo, p).split(path.sep).join('/') + ':' + (i + 1));
      }
    });
  }
}
walkSrc(path.join(game, 'Source'));
for (const [t, where] of codeRefs) if (!roots.has(t)) roots.set(t, 'code ' + where[0]);

// BFS from the roots; never-cook packages are not expanded (the cooker does not save them).
const prev = new Map();
const q = [];
for (const r of roots.keys()) if (!isNever(r)) { prev.set(r, null); q.push(r); }
while (q.length) {
  const n = q.shift();
  for (const t of edges.get(n) || []) if (!prev.has(t) && !isNever(t)) { prev.set(t, n); q.push(t); }
}
const chain = n => { const c = []; for (let x = n; x; x = prev.get(x)) c.unshift(x); return c; };

const cands = [...pkgs.keys()].filter(n => candRe.test(n)).sort();
let bytes = 0; const reached = [], codeOnly = [];
for (const c of cands) {
  bytes += fs.statSync(pkgs.get(c)).size;
  if (!prev.has(c)) continue;
  const ch = chain(c);
  const why = roots.get(ch[0]);
  if (why.startsWith('code ') && candRe.test(ch[0])) codeOnly.push({ pkg: c, chain: ch, root: why }); else reached.push({ pkg: c, chain: ch, root: why });
}
// A cooked package that names a never-cook package is a hole in the cook (the cooker's 'Content is missing from cook' error).
const holes = [];
for (const n of prev.keys()) for (const t of edges.get(n) || []) if (isNever(t)) holes.push({ from: n, to: t, chain: chain(n), root: roots.get(chain(n)[0]) });
// Who outside the candidates names a candidate (direct referencers), for the record.
const referencers = new Map();
for (const [n, out] of edges) if (!candRe.test(n)) for (const t of out) if (candRe.test(t)) { if (!referencers.has(n)) referencers.set(n, new Set()); referencers.get(n).add(t); }

console.log(`refwalk: ${pkgs.size} packages, ${roots.size} roots (${maps.length} maps, ${always.length} always-cook dirs, ${never.length} never-cook entries, ${codeRefs.size} code refs)`);
console.log(`candidates: ${cands.length} packages, ${(bytes / 1048576).toFixed(1)} MB on disk`);
console.log(`direct referencers outside the candidates: ${referencers.size}`);
for (const [n, ts] of referencers) console.log(`  ${n} -> ${[...ts].slice(0, 4).join(', ')}${ts.size > 4 ? ` (+${ts.size - 4})` : ''}${prev.has(n) ? '  [shipped]' : '  [not shipped]'}`);
console.log(`code references to candidates (the generated_b2 visual's class paths, chosen only by a cut whose game.skater.visual is generated_b2):`);
for (const [t, where] of codeRefs) if (candRe.test(t)) console.log(`  ${t}  ${where.join(' ')}`);
console.log(`reached from the code's generated_b2 branch only: ${codeOnly.length}`);
console.log(`reached from a shipped root: ${reached.length}`);
for (const r of reached.slice(0, 40)) console.log(`  ${r.pkg}  <- ${r.chain.join(' <- ')}  [${r.root}]`);
console.log(`cooked packages that name a never-cook package: ${holes.length}`);
for (const h of holes.slice(0, 40)) console.log(`  ${h.from} -> ${h.to}  (cooked by ${h.chain.join(' <- ')} [${h.root}])`);
if (jsonOut) fs.writeFileSync(jsonOut, JSON.stringify({ candidates: cands, bytes, reached, codeOnly, holes, referencers: Object.fromEntries([...referencers].map(([k, v]) => [k, [...v]])) }, null, 1));
const bad = reached.length + holes.length;
console.log(bad === 0 ? 'PASS refwalk: no shipped root reaches a candidate and no cooked package names a never-cook one' : `FAIL refwalk: ${reached.length} candidates reached from shipped roots, ${holes.length} cooked packages name a never-cook package`);
process.exit(bad === 0 ? 0 : 1);
