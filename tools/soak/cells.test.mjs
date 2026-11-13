// node --test tools/soak/cells.test.mjs   (no Unreal needed: a fake runner stands in for the commandlet)
import { test } from 'node:test';
import assert from 'node:assert/strict';
import { mkdtempSync, mkdirSync, writeFileSync, readFileSync, existsSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
import { seedList, chunksOf, mayStart, cpuLimit, parseChunk, repeatCommand, run } from './cells.mjs';

const here = dirname(fileURLToPath(import.meta.url));
const runner = join(here, 'test', 'fake-runner.mjs');
const lines = (f) => readFileSync(f, 'utf8').split('\n').filter(Boolean).map((l) => JSON.parse(l));
async function quiet(fn) { const log = console.log; const outl = []; console.log = (...x) => outl.push(x.join(' ')); try { return [await fn(), outl]; } finally { console.log = log; } }
async function drive(plan, extra = []) {
  const dir = mkdtempSync(join(tmpdir(), 'soak-')); const pf = join(dir, 'plan.json'); writeFileSync(pf, JSON.stringify(plan));
  const [code, stdout] = await quiet(() => run([pf, '--runner', runner, '--out', join(dir, 'out'), '--max', '4', '--chunk', '5', ...extra]));
  return { code, dir, out: join(dir, 'out'), stdout };
}
const cell = (label, cart, seeds, more = {}) => ({ label, cls: 'regular', lvl: 'pro', cart, seeds, ...more });

test('seedList: ranges, lists, the regular band', () => {
  assert.deepEqual(seedList('1-3'), [1, 2, 3]);
  assert.deepEqual(seedList('3,5,9-10'), [3, 5, 9, 10]);
  const b = seedList('regularband:150'); assert.equal(b.length, 150); assert.equal(b[100], 151); assert.equal(b[99], 100);
});
test('chunksOf: contiguous runs of at most n', () => {
  assert.deepEqual(chunksOf([1, 2, 3, 4, 5, 6, 7], 3), [[1, 3], [4, 6], [7, 7]]);
  assert.deepEqual(chunksOf([1, 2, 5, 6], 10), [[1, 2], [5, 6]]);
  assert.deepEqual(chunksOf([], 10), []);
});
test('mayStart: the 38-50 GB band', () => {
  assert.equal(mayStart({ usedGB: 30, active: 15, max: 16 }), true);
  assert.equal(mayStart({ usedGB: 30, active: 16, max: 16 }), false);
  assert.equal(mayStart({ usedGB: 40, active: 3, max: 16 }), true);
  assert.equal(mayStart({ usedGB: 40, active: 4, max: 16 }), false);
  assert.equal(mayStart({ usedGB: 51, active: 0, max: 16 }), false);
  assert.equal(mayStart({ usedGB: 57, active: 0, max: 16 }), false);
});
test('cpuLimit and mayStart: the processor band slides the worker limit, never below the floor', () => {
  assert.equal(cpuLimit({ cpu: undefined, max: 16 }), 16);
  assert.equal(cpuLimit({ cpu: 0.5, max: 16 }), 16);
  assert.equal(cpuLimit({ cpu: 0.97, max: 16 }), 4);
  assert.equal(cpuLimit({ cpu: 1, max: 16 }), 4);
  assert.equal(cpuLimit({ cpu: 0.91, max: 16 }), 10);   // halfway: 4 + 12 * 0.5
  assert.equal(cpuLimit({ cpu: 1, max: 2 }), 2);        // the floor never exceeds max
  assert.equal(mayStart({ usedGB: 30, active: 3, max: 16, cpu: 1 }), true);
  assert.equal(mayStart({ usedGB: 30, active: 4, max: 16, cpu: 1 }), false);
  assert.equal(mayStart({ usedGB: 30, active: 12, max: 16, cpu: 0.5 }), true);
  assert.equal(mayStart({ usedGB: 51, active: 0, max: 16, cpu: 0.1 }), false);   // memory still wins
});
test('parseChunk: pins a fault to the next match, lists missing seeds', () => {
  const buf = ['junk', 'x Error: boom', 'L {"tool":"thirdjam.playtest/1","seed":2}', "Failed to load 'aqProf", 'Fatal error: dead'].join('\n');
  const r = parseChunk(buf, 1, 3);
  assert.deepEqual(r.matches.map((m) => m.seed), [2]);
  assert.deepEqual(r.faults.map((f) => [f.kind, f.seed]), [['error_line', 2]]);
  assert.equal(r.pending[0].kind, 'crash'); assert.deepEqual(r.missing, [1, 3]);
});
test('repeatCommand carries -Multiprocess and the cap', () => {
  const s = repeatCommand({ cart: 'x', lvl: 'pro', cls: 'good', maxframes: 42000, extra: ['-playersprint'] }, 9);
  assert.match(s, /-Multiprocess/); assert.match(s, /-seeds=9-9/); assert.match(s, /-maxframes=42000/); assert.match(s, /-playersprint/);
});

test('driver: one line per match, every start carries -Multiprocess, the cap travels, done files', async () => {
  const { code, out } = await drive([cell('a', 'hockey.regular', '1-12'), cell('b', 'ladder/take-9', '1-3', { maxframes: 42000 })]);
  assert.equal(code, 0);
  const a = lines(join(out, 'a.jsonl')); assert.deepEqual(a.map((m) => m.seed), [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12]);
  assert.ok(a.every((m) => m.mp === true));
  assert.equal(lines(join(out, 'b.jsonl'))[0].maxframes, '42000');
  assert.ok(existsSync(join(out, 'a.done')) && existsSync(join(out, 'b.done')));
});
test('driver: a crash is a fault with seed and repro command, the rest of the chunk is requeued', async () => {
  const { code, out } = await drive([cell('c', 'crashy', '1-12')]);
  assert.equal(code, 1);
  assert.deepEqual(lines(join(out, 'c.jsonl')).map((m) => m.seed), [1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12]);
  const led = lines(join(out, 'faults.jsonl'));
  assert.ok(led.some((f) => f.label === 'c' && f.kind === 'missing_seeds' && f.seed === 7 && /-seeds=7-7/.test(f.cmd)));
  assert.ok(led.some((f) => f.kind === 'crash' && f.seed === 7));
});
test('driver: an error line is pinned to its seed in the ledger', async () => {
  const { out } = await drive([cell('e', 'ensure', '1-5')]);
  const led = lines(join(out, 'faults.jsonl')); assert.equal(led.length, 1); assert.equal(led[0].seed, 3); assert.equal(led[0].kind, 'ensure');
});
test('driver: the start guard exits 2 when the first chunk prints nothing', async () => {
  const { code, stdout } = await drive([cell('s', 'silent', '1-3')]);
  assert.equal(code, 2); assert.match(stdout.join('\n'), /could_not_run/);
});
test('driver: a duplicate label and a missing uproject exit 2', async () => {
  assert.equal((await drive([cell('d', 'hockey.regular', '1-2'), cell('d', 'hockey.regular', '1-2')])).code, 2);
  const dir = mkdtempSync(join(tmpdir(), 'soak-')); writeFileSync(join(dir, 'p.json'), JSON.stringify([cell('z', 'x', '1-2')]));
  const [c] = await quiet(() => run([join(dir, 'p.json'), '--out', join(dir, 'o'), '--uproject', join(dir, 'nope.uproject')]));
  assert.equal(c, 2);
});
test('driver: resumes from partial files', async () => {
  const dir = mkdtempSync(join(tmpdir(), 'soak-')); const out = join(dir, 'out'); const pf = join(dir, 'plan.json');
  writeFileSync(pf, JSON.stringify([cell('r', 'hockey.regular', '1-6')])); mkdirSync(out, { recursive: true });
  writeFileSync(join(out, 'r.partial.jsonl'), [1, 2, 3].map((s) => JSON.stringify({ tool: 'thirdjam.playtest/1', seed: s, hash: 'old', mp: 'old' })).join('\n') + '\n');
  const [code] = await quiet(() => run([pf, '--runner', runner, '--out', out, '--chunk', '5']));
  assert.equal(code, 0); const m = lines(join(out, 'r.jsonl'));
  assert.deepEqual(m.map((x) => x.seed), [1, 2, 3, 4, 5, 6]); assert.equal(m[0].hash, 'old'); assert.equal(m[3].mp, true);
});
