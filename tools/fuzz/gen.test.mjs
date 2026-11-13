// Lane A109: the fuzz generator is deterministic, its valid family validates, its invalid family is refused, and the checked-in smoke corpus is current.
import test from 'node:test';
import assert from 'node:assert/strict';
import { mkdtempSync, readFileSync, readdirSync, rmSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { generate } from './gen.mjs';
import { validateCartridge, validateCartridgeText } from '../../schema/validate.mjs';

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
    const errors = validateCartridgeText(readFileSync(join(dir, c.file), 'utf8')).errors;   // lane A118: the strict parser both sides share
    assert.equal(errors.length === 0, c.ok, c.name);
    assert.deepEqual([...new Set(errors.map((e) => e.match(/^at (\S+?):/)?.[1] ?? '?'))].sort(), c.pointers, `${c.name} pointers`);
  }
});

// Lane A118 (track S3, evolution 2): the regression corpus of the seven closed fault classes is current, and the two parsers agree on what A109 found.
test('the regression corpus (tools/fuzz/regress) matches Node verdicts; set_param, duplicate keys and the BOM follow one rule', () => {
  const dir = resolve(HERE, 'regress');
  const m = JSON.parse(readFileSync(join(dir, 'manifest.json'), 'utf8'));
  assert.equal(m.cases.length, 19);
  for (const c of m.cases) assert.equal(validateCartridgeText(readFileSync(join(dir, c.file), 'utf8')).ok, c.ok, c.name);
  const byName = Object.fromEntries(m.cases.map((c) => [c.name, c]));
  assert.deepEqual(byName['nan--min-setparam-puck-max-speed-neg'].pointers, ['/game/rules/hooks/0/then/0/value']);
  assert.equal(byName['parity--bom'].ok, true);
  assert.deepEqual(byName['parity--duplicate-key'].pointers, ['/game/rink']);
  assert.deepEqual(byName['parity--neg-infinity-number'].pointers, ['/game/skater/speed']);
  for (const n of ['never-ends-unminimized--fuzz-00600', 'never-ends-unminimized--fuzz-04552']) { assert.equal(byName[n].ok, false); assert.equal(byName[`${n}-valid`].ok, true); }
});

// Lane A141 (track S3, evolution 3): the generator knows every key of the schema. The sweep is derived from game.schema.json, so a key added tomorrow is covered
// without an edit; this test proves each scalar key (and every key limits.json lists without a list marker) runs at both of its ends, alone, and that no sweep case
// was refused by the validator (a refused case is a hole in the corpus).
import { schemaLeaves, PAIRS } from './keys.mjs';
test('the key sweep covers every scalar key of game.schema.json at both ends, alone and in the named pairs', () => {
  const out = mkdtempSync(join(tmpdir(), 'fuzz-keys-'));
  try {
    const s = generate({ count: 0, invalid: 0, seed: 1, out });
    assert.equal(s.sweep.skipped, 0, 'every sweep case must validate');
    const m = JSON.parse(readFileSync(join(out, 'manifest.json'), 'utf8'));
    const names = new Set(m.cases.map((c) => c.name));
    const leaves = schemaLeaves();
    assert.ok(leaves.length > 130, `only ${leaves.length} keys found in the schema`);
    for (const l of leaves) {
      const ends = l.type === 'enum' ? l.values : [l.values[0], l.values[1]];
      for (const v of ends) assert.ok(names.has(`key-${l.path}=${v}`), `no sweep case for ${l.path}=${v}`);
    }
    // every limits.json key that names a scalar (no list marker) is one the sweep reaches, except the ones the schema holds as a const
    const have = new Set(leaves.map((l) => l.path));
    const lim = JSON.parse(readFileSync(resolve(HERE, '..', '..', 'schema', 'limits.json'), 'utf8')).game_ranges;
    for (const k of Object.keys(lim)) if (!k.includes('[]')) assert.ok(have.has(k), `limits.json game_ranges ${k} is not swept`);
    for (const [a, b] of PAIRS) assert.ok([...names].some((n) => n.startsWith(`pair-key-`) === false && n.startsWith(`pair-${a}=`) && n.includes(`-${b}=`)), `pair ${a} + ${b} missing`);
    for (const k of ['contact.board_bonus', 'contact.release_grace_frames', 'stick.passive_take', 'stick.body_shield', 'stick.take_grace_frames', 'skater.reverse_brake_decel', 'skater.burst.own_stop']) assert.ok(have.has(k), k);
    // the hook combinations the brief names are present
    const profiles = new Set(m.cases.map((c) => c.profile.split('+')[0]));
    for (const p of ['combo/graces', 'combo/board_bonus', 'combo/take_grace', 'combo/burst']) assert.ok(profiles.has(p), p);
  } finally { rmSync(out, { recursive: true, force: true }); }
});

// Lane A141 (evolution 3, item 2): the Designer-reply corpus. The checked-in smoke manifest (tools/fuzz/replies) still carries Node's own verdicts, every defect class is in it, and a
// duplicate key and a number past double range are refused with a pointer (parseDesignerReply goes through the strict parser the C++ mirror uses).
import { parseDesignerReply } from '../../schema/validate.mjs';
import { generateReplies } from './gen-replies.mjs';
test('the Designer-reply corpus: smoke manifest matches parseDesignerReply; the classes are all there; same seed same corpus', () => {
  const base = JSON.parse(readFileSync(resolve(HERE, '..', '..', 'Game', 'Content', 'Cartridges', 'hockey.regular.cartridge.json'), 'utf8'));
  const m = JSON.parse(readFileSync(join(HERE, 'replies', 'manifest.json'), 'utf8'));
  assert.ok(m.cases.length >= 56);
  for (const c of m.cases) {
    const r = parseDesignerReply(c.text, base);
    assert.equal(r.kind, c.kind, c.name);
    assert.deepEqual([...new Set((r.errors ?? []).map((e) => e.match(/^at (\S+?):/)?.[1] ?? '?'))].sort(), c.pointers, `${c.name} pointers`);
  }
  const a = generateReplies({ count: 300, seed: 9 }), b = generateReplies({ count: 300, seed: 9 });
  assert.deepEqual(a, b);
  const kinds = new Set(a.map((c) => c.kind));
  for (const k of ['cartridge', 'refusal', 'invalid']) assert.ok(kinds.has(k), k);
  for (const tag of ['dupkey', 'truncated', 'range-', 'unknown-', 'locked', 'refusal']) assert.ok(a.some((c) => c.name.includes(tag)), tag);
  assert.ok(a.some((c) => c.text.startsWith('﻿')), 'a BOM reply');
  assert.ok(a.some((c) => c.text.includes('```')), 'a fenced reply');
  assert.ok(a.some((c) => c.text.startsWith('Sure!')), 'a prose reply');
  const dup = parseDesignerReply('{"refusal":"a","refusal":"b"}', base);
  assert.equal(dup.kind, 'invalid');
  assert.match(dup.errors[0], /duplicate key/);
  assert.equal(parseDesignerReply('﻿{"refusal":"ok"}', base).kind, 'refusal');
});
