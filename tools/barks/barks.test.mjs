// Lane A12: the committed bark bank is clean (every line passes filter.mjs, no duplicates, every combination has at least 12 lines) and the
// embedded C++ copy matches it. Run with the gate: node --test tools/**/*.test.mjs
import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { checkLine, filterBank, filmWords, mildCount, ALLOW_PHRASES, MAX_CHARS, MILD_QUOTA } from './filter.mjs';
import { SITUATIONS, SPEAKERS, TONES } from './generate.mjs';

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..');
const bank = JSON.parse(readFileSync(resolve(repo, 'Game/Content/Barks/barks.json'), 'utf8'));
const TEAMMATE_ONLY = new Set(['signal_ack_defend', 'signal_ack_space', 'signal_ack_pass', 'praise_reply']);

test('the filter rejects what the content law forbids and keeps hockey flavour', () => {
  assert.deepEqual(checkLine('back to the bench, rookie'), []);
  assert.deepEqual(checkLine('nice stick, shame about the aim'), []);
  assert.deepEqual(checkLine('this ice is damn slow'), [], 'mild word allowed (R-0015)');
  assert.ok(checkLine('this ice is shit').length, 'hard profanity');
  assert.ok(checkLine('Gretzky would weep').length, 'real person');
  assert.ok(checkLine('go ask Bauer for a refund').length, 'brand');
  assert.ok(checkLine('the Leafs play better').length, 'team');
  assert.ok(checkLine('I will break your legs').length, 'threat');
  assert.ok(checkLine('x'.repeat(MAX_CHARS + 1)).length, 'too long');
  assert.ok(checkLine('nice goal \u{1F600}').length, 'emoji');
  assert.ok(checkLine('see http://example.com').length, 'url');
});

test('every committed line passes the filter and no two lines repeat', () => {
  assert.equal(bank.schema, 'hokej.barks/1');
  const { kept, rejected } = filterBank(bank.lines);
  assert.deepEqual(rejected.map((r) => `${r.id}: ${r.reasons[0]}`), []);
  assert.equal(kept.length, bank.lines.length);
  const ids = new Set(bank.lines.map((l) => l.id));
  assert.equal(ids.size, bank.lines.length, 'ids are unique');
});

test('every situation x speaker x tone has 12 to 20 lines; only known names appear', () => {
  const counts = {};
  for (const l of bank.lines) {
    assert.ok(l.situation in SITUATIONS, `situation ${l.situation}`);
    assert.ok(SPEAKERS.includes(l.speaker), `speaker ${l.speaker}`);
    assert.ok(TONES[l.speaker].includes(l.tone), `tone ${l.tone} for ${l.speaker}`);
    assert.deepEqual(Object.keys(l).sort(), ['id', 'situation', 'speaker', 'text', 'tone']);
    const k = `${l.situation}.${l.speaker}.${l.tone}`;
    counts[k] = (counts[k] ?? 0) + 1;
  }
  for (const situation of Object.keys(SITUATIONS)) {
    for (const speaker of SPEAKERS) {
      if (TEAMMATE_ONLY.has(situation) && speaker !== 'teammate') continue;
      for (const tone of TONES[speaker]) {
        const n = counts[`${situation}.${speaker}.${tone}`] ?? 0;
        assert.ok(n >= 12 && n <= 20, `${situation}.${speaker}.${tone} has ${n} lines`);
      }
    }
  }
});

test('the embedded C++ copy matches barks.json (node tools/barks/embed.mjs)', () => {
  const inl = readFileSync(resolve(repo, 'Game/Source/JamGame/Private/BarkData.inl'), 'utf8');
  const bytes = readFileSync(resolve(repo, 'Game/Content/Barks/barks.json'));
  const m = inl.match(/JamBarkBankJson\[\] = \{([\s\S]*?)\n\t0\n\};/);
  assert.ok(m, 'BarkData.inl has the byte array');
  const embedded = Buffer.from(m[1].split(/[\s,]+/).filter(Boolean).map(Number));
  assert.equal(embedded.length, bytes.length);
  assert.ok(embedded.equals(bytes), 'bytes differ: re-run node tools/barks/embed.mjs');
});

test('R-0015: mild words are at most one line in ten, never two in a line, and the hard list still bites', () => {
  const mild = bank.lines.filter((l) => mildCount(l.text) > 0);
  assert.ok(mild.length <= Math.floor(bank.lines.length * MILD_QUOTA), `${mild.length} mild lines of ${bank.lines.length}`);
  for (const l of bank.lines) assert.ok(mildCount(l.text) <= 1, l.id);
  assert.deepEqual(checkLine('damn, even the boards felt that one'), []);
  assert.ok(checkLine('damn hell, what a mess').length, 'two mild words');
  assert.ok(checkLine('you ass').length, 'bare ass');
  assert.ok(checkLine('that was fucking bad').length, 'hard list');
  assert.ok(checkLine('the goalie fell asleep').length, 'off-world word');
});

test('R-0021 point 2: barks are hockey first; film words are rejected, hockey senses of take, lead and frame stay', () => {
  for (const bad of ["that's a wrap, director", 'cut and print, rookie', 'you went down like it was in the script', 'take two, same ending', 'another one for the highlight reel', 'back to the scene'])
    assert.ok(checkLine(bad).some((r) => /film word/.test(r)), bad);
  for (const ok of ['take the puck back, if you can', 'we kept the lead', 'shooting at the frame instead of the hole', 'new rule, same footwork', 'that was a wrap, and not the good kind'])
    assert.deepEqual(checkLine(ok), [], ok);
  assert.deepEqual(filmWords('cut it, director').hard, ['director', 'cut']);
  assert.deepEqual(filmWords('take the puck').soft, ['take']);
  // The committed bank has no hard film word and its directors_call lines speak of rules, never of directors.
  for (const l of bank.lines) assert.deepEqual(filmWords(l.text).hard, [], l.id);
  for (const l of bank.lines.filter((l) => l.situation === 'directors_call')) assert.ok(!/\b(director|cut|scene)s?\b/i.test(l.text), l.id);
});

test('A41: "sudden death" is hockey vocabulary; "death" alone and every other banned word stay banned', () => {
  assert.deepEqual(ALLOW_PHRASES, ['sudden death']);
  assert.deepEqual(checkLine('sudden death, next goal wins it'), []);
  assert.deepEqual(checkLine('we are in Sudden Death now, boys'), [], 'the phrase capitalised mid-line is not a name');
  assert.deepEqual(checkLine('SUDDEN DEATH: that goal wins it'), [], "the loader's own announce line");
  assert.deepEqual(checkLine('Sudden Death Cut'.replace(/ Cut$/, '')), [], 'the cut name without its menu word');
  assert.ok(checkLine('death to the other line').some((r) => /banned word "death"/.test(r)), 'death alone');
  assert.ok(checkLine('slow death on the big ice').some((r) => /banned word "death"/.test(r)), 'another phrase');
  assert.ok(checkLine('sudden death, then we kill the clock').some((r) => /banned word "kill/.test(r)), 'the rest of the list still bites');
  assert.ok(checkLine('sudden death for the Leafs').some((r) => /banned word "leafs"/.test(r)), 'a team after the phrase');
});
