// Lane A143: A's commentator texts equal B's, line by line; A's supplement lines carry the lengths B measured (the caption and the no-overlap rule run on them).
// Runs in the premerge gate: node --test tools/**/*.test.mjs
import { test } from 'node:test';
import assert from 'node:assert/strict';
import { aSide, bManifest, bTable, compare, parseTable, PENDING_B } from './parity.mjs';

test('every line id on both sides has the same text (or is a known pending one B merges next)', () => {
  const r = compare(aSide(), bManifest(), bTable());
  assert.deepEqual(r.textMismatch, []);
  assert.ok(r.compared >= 340, `${r.compared} ids compared`);
  assert.deepEqual(r.onlyB, [], 'a line B has and A does not');
  for (const id of r.pending) assert.ok(PENDING_B[id], id);
});

test('A holds B\'s re-recorded lose.blowout.01', () => {
  assert.equal(aSide().get('lose.blowout.01').text, 'A night to forget. Back to practice.');
});

test('the supplement lines match what B recorded, text and measured seconds', () => {
  const t = bTable();
  assert.ok(t.size >= 73, `${t.size} rows in B's table`);
  const r = compare(aSide(), bManifest(), t);
  assert.deepEqual(r.tableText, []);
  assert.deepEqual(r.seconds, []);
});

test('the comparison finds a changed text, a pending one, a stale pending one and a wrong length', () => {
  const a = new Map([['x.a.01', { text: 'One.' }], ['lose.blowout.01', { text: 'New.' }], ['y.b.01', { text: 'Two.', seconds: 1.5 }]]);
  const b = new Map([['x.a.01', { text: 'Uno.' }], ['lose.blowout.01', { text: PENDING_B['lose.blowout.01'] }], ['z.c.01', { text: 'B only.' }]]);
  const r = compare(a, b, new Map([['y.b.01', { text: 'Two.', seconds: 2.25 }]]));
  assert.deepEqual(r.textMismatch.map((m) => m.id), ['x.a.01']);
  assert.deepEqual(r.pending, ['lose.blowout.01']);
  assert.deepEqual(r.onlyA, ['y.b.01']);
  assert.deepEqual(r.onlyB, ['z.c.01']);
  assert.deepEqual(r.seconds, [{ id: 'y.b.01', a: 1.5, b: 2.25 }]);
  const after = compare(new Map([['lose.blowout.01', { text: 'Same.' }]]), new Map([['lose.blowout.01', { text: 'Same.' }]]), new Map());
  assert.deepEqual(after.stalePending, ['lose.blowout.01']);
});

test('the table parser reads B\'s rows and the over-2.5 s mark', () => {
  const md = '| id | text as written | s | words |\n|---|---|---|---|\n| a.b.01 | Hi there. | 2.74 * | 2 |\n| a.b.02 | Yo. | 1.1 | 1 |\n\nafter';
  const t = parseTable(md);
  assert.deepEqual([...t.entries()], [['a.b.01', { text: 'Hi there.', seconds: 2.74 }], ['a.b.02', { text: 'Yo.', seconds: 1.1 }]]);
});
