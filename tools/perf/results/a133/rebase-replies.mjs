#!/usr/bin/env node
// Lane A133: rebase the checked-in Designer-reply parity corpus (tools/fuzz/replies/manifest.json, lane A141) onto a changed hockey.regular: each "from"
// text in a case becomes "to", then Node's parseDesignerReply re-judges every case against the current hockey.regular and the manifest takes its verdict.
// Prints the cases whose verdict changed (it should be none: the base and the replies moved together).
//   node tools/perf/results/a133/rebase-replies.mjs '"penalties": true' '"penalties": false' ['"tier": "l2_night"' '"tier": "p3_clay"']
import { readFileSync, writeFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { parseDesignerReply } from '../../../../schema/validate.mjs';
const repo = resolve(dirname(fileURLToPath(import.meta.url)), '..', '..', '..', '..');
const path = resolve(repo, 'tools/fuzz/replies/manifest.json');
const m = JSON.parse(readFileSync(path, 'utf8'));
const base = JSON.parse(readFileSync(resolve(repo, 'Game/Content/Cartridges/hockey.regular.cartridge.json'), 'utf8'));
const pairs = process.argv.slice(2);
const pointers = (errors) => [...new Set(errors.map((e) => e.match(/^at (\S+?):/)?.[1] ?? '?'))].sort();
let changed = 0, rewritten = 0;
for (const c of m.cases) {
  let t = c.text;
  for (let i = 0; i < pairs.length; i += 2) {
    for (const sp of ['', ' ']) { const f = pairs[i].replace(': ', ':' + sp), to = pairs[i + 1].replace(': ', ':' + sp); t = t.split(f).join(to); }
  }
  if (t !== c.text) rewritten++;
  c.text = t;
  const r = parseDesignerReply(t, base);
  const kind = r.kind, ptrs = kind === 'invalid' ? pointers(r.errors) : [];
  if (kind !== c.kind || JSON.stringify(ptrs) !== JSON.stringify([...c.pointers].sort())) { changed++; console.log(`CHANGED ${c.name}: ${c.kind} [${c.pointers}] -> ${kind} [${ptrs}]`); c.kind = kind; c.pointers = ptrs; }
}
writeFileSync(path, JSON.stringify(m));   // compact, no newline: as gen-replies.mjs writes it
console.log(JSON.stringify({ tool: 'a133.rebase-replies', ok: true, cases: m.cases.length, rewritten, changed }));
