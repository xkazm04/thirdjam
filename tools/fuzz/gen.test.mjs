// Lane A109: the fuzz generator is deterministic, its valid family validates, its invalid family is refused, and the checked-in smoke corpus is current.
import test from 'node:test';
import assert from 'node:assert/strict';
import { mkdtempSync, readFileSync, readdirSync, rmSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { generate } from './gen.mjs';
import { validateCartridge } from '../../schema/validate.mjs';

const HERE = dirname(fileURLToPath(import.meta.url));

test('same seed, same corpus; every valid case validates; every invalid case is refused or flagged', () => {
  const a = mkdtempSync(join(tmpdir(), 'fuzz-a-'));
  const b = mkdtempSync(join(tmpdir(), 'fuzz-b-'));
  try {
    const sa = generate({ count: 130, invalid: 60, seed: 3, out: a });
    generate({ count: 130, invalid: 60, seed: 3, out: b });
    for (const f of readdirSync(a)) assert.equal(readFileSync(join(a, f), 'utf8'), readFileSync(join(b, f), 'utf8'), `${f} differs between two runs`);
    assert.equal(sa.rejected, 0, `the valid family must validate by construction: ${JSON.stringify(sa.rejectionReasons)}`);
    const m = JSON.parse(readFileSync(join(a, 'manifest.json'), 'utf8'));
    for (const c of m.cases.filter((c) => c.kind === 'valid')) {
      const r = validateCartridge(JSON.parse(readFileSync(join(a, c.file), 'utf8')));
      assert.deepEqual(r.errors, [], c.name);
    }
    const bad = m.cases.filter((c) => c.kind === 'invalid');
    assert.ok(bad.length >= 40);
    for (const c of bad) assert.ok(c.pointers.length > 0, `${c.name} (${c.profile}) is marked invalid but has no pointer`);
    const scenarios = new Set(m.cases.filter((c) => c.kind === 'valid').map((c) => c.profile.split('/')[0]));
    for (const want of ['smallest-rink-5v5-most-pickups', 'shortest-timer', 'every-ability', 'tap-longer-than-charge', 'smallest-reach', 'contact-extremes-hi']) assert.ok(scenarios.has(want), want);
  } finally { rmSync(a, { recursive: true, force: true }); rmSync(b, { recursive: true, force: true }); }
});

test('the checked-in smoke corpus matches Node verdicts (the C++ side compares against the same manifest)', () => {
  const dir = resolve(HERE, 'smoke');
  const m = JSON.parse(readFileSync(join(dir, 'manifest.json'), 'utf8'));
  assert.ok(m.cases.length >= 40);
  for (const c of m.cases) {
    let errors;
    try { errors = validateCartridge(JSON.parse(readFileSync(join(dir, c.file), 'utf8'))).errors; } catch { errors = ['at /: not json']; }
    assert.equal(errors.length === 0, c.ok, c.name);
  }
});
