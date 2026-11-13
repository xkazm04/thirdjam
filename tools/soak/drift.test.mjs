// node --test tools/soak/drift.test.mjs   (no Unreal: the comparison and the plan only)
import { test } from 'node:test';
import assert from 'node:assert/strict';
import { mkdtempSync, mkdirSync, writeFileSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import { compare, measure, buildPlan } from './drift.mjs';

const cell = (win, digest = 'd', extra = {}) => ({ n: 100, win, unfinished: 0, frames: 9000, digest, ...extra });
const mode = (value, win, digest = 'd') => cell(win, digest, { character: 'checks landed / match', value });

test('compare: a bit-identical cell cannot have moved', () => {
  const r = compare({ 't-take-1-regular': cell(50, 'x') }, { 't-take-1-regular': cell(50, 'x') });
  assert.equal(r.identical, 1); assert.equal(r.moved.length, 0);
});
test('compare: a win rate that moved more than 10 percent and 2 points is flagged with its sigma', () => {
  const r = compare({ 't-take-3-good': cell(50, 'a') }, { 't-take-3-good': cell(58, 'b') });
  assert.equal(r.moved.length, 1); assert.equal(r.moved[0].delta, 8); assert.equal(r.moved[0].pct, 16); assert.ok(r.moved[0].sigmas > 1);
});
test('compare: a move under 10 percent or under the floor is not flagged', () => {
  assert.equal(compare({ t: cell(50, 'a') }, { t: cell(54, 'b') }).moved.length, 0);   // 8 %
  assert.equal(compare({ t: cell(1, 'a') }, { t: cell(2, 'b') }).moved.length, 0);     // +100 % but one point
});
test('compare: a mode character number is judged apart from its win rate', () => {
  const r = compare({ 'm-cage': mode(5, 25, 'a') }, { 'm-cage': mode(4, 26, 'b') });
  assert.deepEqual(r.moved.map((m) => m.key), ['m-cage checks landed / match']);   // 20 % of 5; win 25 -> 26 is 4 %
  assert.equal(r.moved[0].pct, -20);
});
test('compare: new and removed cells are listed, a changed unfinished count is flagged', () => {
  const r = compare({ a: cell(10, 'x'), gone: cell(1, 'x') }, { a: { ...cell(10, 'y'), unfinished: 3 }, fresh: cell(1, 'x') });
  assert.deepEqual(r.added, ['fresh']); assert.deepEqual(r.removed, ['gone']);
  assert.ok(r.moved.some((m) => m.key === 'a unfinished'));
});
test('measure: win rate, unfinished and a deterministic digest; marathon reads its character number', () => {
  const ms = [1, 2, 3, 4].map((s) => ({ seed: s, result: s <= 2 ? 'win' : s === 3 ? 'loss' : 'unfinished', hash: `h${s}`, frames: 100, play_frames: 3600, sprint_frames: 10 * s }));   // lane A139: Marathon's number is per play minute (100 sprint frames in 4 play minutes)
  const c = measure('m-marathon', ms);
  assert.equal(c.win, 50); assert.equal(c.unfinished, 1); assert.equal(c.value, 25); assert.equal(c.digest, measure('m-marathon', ms).digest);
  assert.notEqual(c.digest, measure('m-marathon', ms.slice(1)).digest);
});
test('buildPlan: 12 modes (marathon capped and sprinting) and 8 takes x 3 classes from the cartridge folders', () => {
  const dir = mkdtempSync(join(tmpdir(), 'cart-')); mkdirSync(join(dir, 'modes')); mkdirSync(join(dir, 'ladder'));
  const w = (f, lvl) => writeFileSync(f, JSON.stringify({ game: { ai: { difficulty: lvl } } }));
  for (const s of ['cage', 'marathon']) w(join(dir, 'modes', `hockey.mode.${s}.cartridge.json`), 'pro');
  w(join(dir, 'ladder', 'take-1.cartridge.json'), 'rookie'); w(join(dir, 'ladder', 'take-1-fallback.cartridge.json'), 'rookie');
  const p = buildPlan(dir, 50);
  assert.equal(p.length, 2 + 3);
  const mar = p.find((c) => c.label === 'm-marathon'); assert.equal(mar.maxframes, 42000); assert.deepEqual(mar.extra, ['-playersprint']);
  assert.deepEqual(p.filter((c) => c.label.startsWith('t-')).map((c) => [c.cls, c.lvl, c.seeds]), ['novice', 'regular', 'good'].map((c) => [c, 'rookie', '1-50']));
});
