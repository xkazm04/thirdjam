// Lane A32: the committed commentator manifest is clean (tools/commentary/check.mjs), and the embedded C++ copy matches it. Runs in the premerge gate:
// node --test tools/**/*.test.mjs
import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { checkManifest, checkText, situationOf, KEYS } from './check.mjs';
import { render, parseInl, MANIFEST, OUTPUT, SUPPLEMENT, SUPPLEMENT_OUTPUT, renderSupplement } from './embed.mjs';

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..');
const manifest = JSON.parse(readFileSync(resolve(repo, MANIFEST), 'utf8'));

const line = (over = {}) => ({
  id: 'goal.blue.01', event: 'goal', kind: '', team: 'blue', priority: 5, cooldown_s: 0, when: 'blue scores', text: 'Blue scores!',
  sound: '/JamAssets/Audio/VO/VO_Commentator_goal_blue_01.VO_Commentator_goal_blue_01', seconds: 1.1, ...over,
});
const bank = (...lines) => ({ schema: 'hokej.commentary/1', count: lines.length, lines });
const always = () => true;

test('the committed manifest passes the check; every sound is on disk', () => {
  const r = checkManifest(manifest, { root: repo });
  assert.deepEqual(r.errors, []);
  assert.equal(r.lines, 340);
  assert.ok(r.situations >= 130, `${r.situations} situations`);
  // The two allowances B's register needs are the only ones (two stoppage lines over 60 characters; "sudden death" is
  // hockey vocabulary and allowed by the bark filter since lane A41, so it no longer counts as an allowance).
  assert.equal(r.infos.length, 2, r.infos.join('; '));
});

test('the check rejects what the manifest contract forbids', () => {
  const errs = (m) => checkManifest(m, { soundExists: always }).errors;
  assert.deepEqual(errs(bank(line())), []);
  assert.ok(errs(bank(line({ extra: 1 }))).some((e) => /unknown key/.test(e)), 'unknown key');
  assert.ok(errs(bank(line({ team: 'red' }))).some((e) => /team/.test(e)), 'team enum');
  assert.ok(errs(bank(line({ priority: 6 }))).some((e) => /priority/.test(e)), 'priority range');
  assert.ok(errs(bank(line({ priority: 2.5 }))).some((e) => /priority/.test(e)), 'priority integer');
  assert.ok(errs(bank(line({ cooldown_s: -1 }))).some((e) => /cooldown_s/.test(e)), 'cooldown');
  assert.ok(errs(bank(line({ id: 'shot.blue.01' }))).some((e) => /first segment/.test(e)), 'id event mismatch');
  assert.ok(errs(bank(line({ id: 'goal.blue.1' }))).some((e) => /not <event>/.test(e)), 'id shape');
  assert.ok(errs(bank(line(), line())).some((e) => /duplicate id/.test(e)), 'duplicate id');
  assert.ok(errs(bank(line({ event: 'nope', id: 'nope.any.01' }))).some((e) => /not an event/.test(e)), 'unknown event');
  assert.ok(errs(bank(line({ text: 'Up 2 to 1!' }))).some((e) => /digits/.test(e)), 'digits');
  assert.ok(errs(bank(line({ text: 'Gretzky would weep.' }))).some((e) => /text/.test(e)), 'the bark filter bites');
  assert.ok(errs(bank(line({ sound: '/JamAssets/Audio/VO/VO_Commentator_goal_orange_01.VO_Commentator_goal_orange_01' }))).some((e) => /expected/.test(e)), 'sound name from the id');
  assert.ok(errs(bank(line({ seconds: 0 }))).some((e) => /seconds/.test(e)), 'seconds');
  assert.ok(checkManifest(bank(line()), { soundExists: () => false }).errors.some((e) => /no .uasset/.test(e)), 'missing wave');
  assert.ok(checkManifest({ ...bank(line()), count: 7 }, { soundExists: always }).errors.some((e) => /count/.test(e)), 'count');
  assert.ok(checkManifest({ ...bank(line()), schema: 'hokej.barks/1' }, { soundExists: always }).errors.some((e) => /schema/.test(e)), 'schema');
});

test('the two allowances: long stoppage lines and "sudden death"; neither applies to live play', () => {
  const long = 'Good evening from the rink. Let us see what these two can do.';
  assert.deepEqual(checkText({ id: 'period.take_1.03', event: 'period', text: long }).reasons, []);
  assert.ok(checkText({ id: 'shot.hard.01', event: 'shot', text: long }).reasons.length, 'a live line over 60 is rejected');
  assert.deepEqual(checkText({ id: 'clock.sudden.02', event: 'clock', text: 'Sudden death. Do not blink.' }).reasons, []);
  assert.ok(checkText({ id: 'clock.sudden.02', event: 'clock', text: 'Death to the clock.' }).reasons.length, 'death alone stays banned');
  assert.equal(situationOf('goal.pause.03'), 'goal.pause');
  assert.equal(KEYS.length, 10);
});

test('the embedded C++ copy matches commentary.json (node tools/commentary/embed.mjs)', () => {
  const bytes = readFileSync(resolve(repo, MANIFEST));
  const inl = readFileSync(resolve(repo, OUTPUT), 'utf8');
  const embedded = parseInl(inl);
  assert.ok(embedded, 'CommentaryData.inl has the byte array');
  assert.equal(embedded.length, bytes.length);
  assert.ok(embedded.equals(bytes), 'bytes differ: re-run node tools/commentary/embed.mjs');
  assert.ok(inl.includes(`JamCommentaryBankJsonSize = ${bytes.length};`));
  assert.ok(parseInl(render(Buffer.from('{"a":1}'))).equals(Buffer.from('{"a":1}')), 'render and parse round-trip');
});

// Lane A84 (R-0041 point 3): A's ability lines pass the same check (the waves are B's to record: a missing wave is the only error allowed),
// one situation per ability kind, and the embedded copy matches the file.
test('the ability supplement is clean, one situation per kind, and embedded', () => {
  const sup = JSON.parse(readFileSync(resolve(repo, SUPPLEMENT), 'utf8'));
  const r = checkManifest(sup, { soundExists: () => true });
  assert.deepEqual(r.errors, []);
  assert.equal(r.infos.length, 0);
  const kinds = new Set(sup.lines.map((l) => l.kind));
  assert.deepEqual([...kinds].sort(), ['hip_check', 'shield', 'shotgun', 'slow_puck']);
  for (const l of sup.lines) assert.equal(l.event, 'ability', l.id);
  const ids = new Set(manifest.lines.map((l) => l.id));
  for (const l of sup.lines) assert.ok(!ids.has(l.id), `${l.id} collides with B's manifest`);
  assert.equal(readFileSync(resolve(repo, SUPPLEMENT_OUTPUT), 'utf8').replace(/\r/g, ''), renderSupplement(readFileSync(resolve(repo, SUPPLEMENT))));
});
