// The cut list of cut-reach.mjs (lane A159): every -cut= file the acceptance run and the matrix use is in it, twelve modes, eight takes, ids found.
import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readdirSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { cutList } from './cut-reach.mjs';

const REPO = resolve(dirname(fileURLToPath(import.meta.url)), '..', '..');
const ACC = join(REPO, 'tools', 'qa', 'accept');

test('every matrix cut and every accept cut file is listed, with an id', () => {
  const l = cutList();
  for (const f of readdirSync(join(ACC, 'matrix-cuts')).filter((x) => x.endsWith('.json'))) assert.ok(l.some((c) => c.label === `matrix/${f}`), `matrix cut ${f} not listed`);
  for (const f of readdirSync(ACC).filter((x) => /^(accept-cut|a93-cut).*\.json$/.test(x))) assert.ok(l.some((c) => c.label === `accept/${f}`), `accept cut ${f} not listed`);
  for (const c of l) assert.ok(c.id && /^[a-z]/.test(c.id), `${c.label} has no id`);
  assert.equal(l.filter((c) => c.label.startsWith('mode ')).length, 12);
  assert.equal(l.filter((c) => c.label.startsWith('take ')).length, 8);
  assert.equal(new Set(l.map((c) => c.label)).size, l.length);
});
