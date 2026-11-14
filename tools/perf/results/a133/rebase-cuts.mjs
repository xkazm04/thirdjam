#!/usr/bin/env node
// Lane A133: rebase the Designer's cached cuts (Game/Content/Designer/cached-cuts.json) onto a changed hockey.regular without asking a model again.
// A cut may not change what the Designer may not edit, so a base change the curve lane makes (rules.penalties false; look.tier p3_clay) is copied into
// every cut that still carried the old base's value; a cut whose Designer chose another value for an editable key (a neon or day look) keeps its choice.
//   node tools/perf/results/a133/rebase-cuts.mjs "rules.penalties=true>false" "look.tier=l2_night>p3_clay"
// Then: node Game/Source/JamDesigner/Tools/gen-designer-data.mjs (re-validates every cut and embeds them).
import { readFileSync, writeFileSync } from 'node:fs';
import { createHash } from 'node:crypto';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
const repo = resolve(dirname(fileURLToPath(import.meta.url)), '..', '..', '..', '..');
const cachePath = resolve(repo, 'Game/Content/Designer/cached-cuts.json');
const base = JSON.parse(readFileSync(resolve(repo, 'Game/Content/Cartridges/hockey.regular.cartridge.json'), 'utf8'));
const cache = JSON.parse(readFileSync(cachePath, 'utf8'));
const parse = (v) => (v === 'true' ? true : v === 'false' ? false : v);
let moved = 0;
for (const spec of process.argv.slice(2)) {
  const [path, change] = spec.split('=');
  const [from, to] = change.split('>').map(parse);
  const keys = path.split('.');
  const get = (o) => keys.reduce((a, k) => a?.[k], o);
  if (get(base.game) !== to) { console.log(`FAIL hockey.regular game.${path} is ${get(base.game)}, not ${to}`); process.exit(1); }
  for (const c of cache.cuts) {
    const g = c.cartridge.game;
    const parent = keys.slice(0, -1).reduce((a, k) => a[k], g);
    if (parent[keys.at(-1)] === from) { parent[keys.at(-1)] = to; moved++; }
  }
}
cache.base_sha256 = createHash('sha256').update(JSON.stringify(base)).digest('hex');
writeFileSync(cachePath, JSON.stringify(cache, null, 2) + '\n');
console.log(JSON.stringify({ tool: 'a133.rebase-cuts', ok: true, moved, base_sha256: cache.base_sha256 }));
