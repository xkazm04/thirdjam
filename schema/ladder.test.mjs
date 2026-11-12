// LadderValidates (lane A17): the shipped season and the ways a ladder is rejected.
import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { spawnSync } from 'node:child_process';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { validateLadder } from './ladder.mjs';

const HERE = dirname(fileURLToPath(import.meta.url));
const DIR = resolve(HERE, '..', 'Game', 'Content', 'Cartridges', 'ladder');
const FILE = resolve(DIR, 'season.ladder.json');
const season = () => JSON.parse(readFileSync(FILE, 'utf8'));

test('LadderValidates: the shipped season and its eight take cartridges', () => {
  const r = validateLadder(season(), { dir: DIR });
  assert.deepEqual(r.errors, []);
  assert.equal(season().ladder.takes.length, 8);
});

test('LadderValidates: the CLI exits 0 and prints a JSON last line', () => {
  const r = spawnSync(process.execPath, [resolve(HERE, 'ladder.mjs'), FILE], { encoding: 'utf8' });
  assert.equal(r.status, 0, r.stdout);
  assert.equal(JSON.parse(r.stdout.trim().split('\n').at(-1)).ok, true);
});

test('LadderValidates: difficulty never falls across the season', () => {
  const order = { rookie: 0, pro: 1, all_star: 2 };
  let last = -1;
  for (const t of season().ladder.takes) {
    const c = JSON.parse(readFileSync(resolve(DIR, `${t.cartridge}.cartridge.json`), 'utf8'));
    const lv = order[c.game.ai.difficulty];
    assert.ok(lv >= last, `${t.cartridge} is ${c.game.ai.difficulty} after a harder take`);
    last = lv;
  }
  assert.equal(last, 2, 'the season ends at all_star');
});

const bad = (edit) => { const o = season(); edit(o); return validateLadder(o, { dir: DIR }).errors.join('\n'); };

test('LadderValidates: unknown keys are errors', () => {
  assert.match(bad((o) => { o.ladder.takes[0].fall_back = 'take-2'; }), /\/ladder\/takes\/0\/fall_back: unknown key/);
  assert.match(bad((o) => { o.extra = 1; }), /\/extra: unknown key/);
});

test('LadderValidates: null is not absent', () => {
  assert.match(bad((o) => { o.ladder.takes[1].twist_text = null; }), /\/ladder\/takes\/1\/twist_text: expected a string/);
});

test('LadderValidates: the first take must be open', () => {
  assert.match(bad((o) => { o.ladder.takes[0].unlock = 'previous_won'; }), /\/ladder\/takes\/0\/unlock: the first take must unlock "always"/);
});

test('LadderValidates: unknown unlock, duplicate and missing cartridges', () => {
  assert.match(bad((o) => { o.ladder.takes[2].unlock = 'stars'; }), /\/ladder\/takes\/2\/unlock: "stars" is not allowed/);
  assert.match(bad((o) => { o.ladder.takes[3].cartridge = 'take-1'; }), /\/ladder\/takes\/3\/cartridge: "take-1" is already used by take 1/);
  assert.match(bad((o) => { o.ladder.takes[4].cartridge = 'take-99'; }), /no file take-99\.cartridge\.json/);
});

test('LadderValidates: an empty season is rejected', () => {
  assert.match(bad((o) => { o.ladder.takes = []; }), /\/ladder\/takes: the list has 0 items/);
});

test('LadderValidates: fallbacks (R-0014) exist, validate and are not shared', () => {
  const s = season();
  assert.ok(s.ladder.takes.slice(4).every((t) => typeof t.fallback === 'string'), 'takes 5-8 name a fallback');
  assert.match(bad((o) => { o.ladder.takes[4].fallback = 'take-1'; }), /\/ladder\/takes\/4\/fallback: "take-1" is already used by take 1/);
  assert.match(bad((o) => { o.ladder.takes[5].fallback = 'take-99'; }), /\/ladder\/takes\/5\/fallback: no file take-99/);
  assert.match(bad((o) => { o.ladder.takes[6].fallback = null; }), /\/ladder\/takes\/6\/fallback: expected a string/);
});
