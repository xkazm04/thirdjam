// Lane A19: tests for the review page builder and the bark decisions applier.   node --test tools/review
import { test } from 'node:test';
import assert from 'node:assert/strict';
import { mkdtempSync, readFileSync, writeFileSync, rmSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { buildPage, validateItems, introHtml, MAX_BYTES } from './build.mjs';
import { parseDecisions, planApply, applyDecisions, reviewSection } from './apply-barks.mjs';
import { barkItems, barkIntro } from './barks-items.mjs';
import { feelItems } from './feel-items.mjs';

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..');
const items = [
  { id: 'a.1', group: 'Alpha', subgroup: 'one', text: 'First line', tags: ['mild'] },
  { id: 'a.2', group: 'Alpha', subgroup: 'one', text: 'Second </script><b>line</b>', note: 'a note' },
  { id: 'b.1', group: 'Beta', text: 'Third line' },
];

function scriptOf(html) {
  const m = [...html.matchAll(/<script>([\s\S]*?)<\/script>/g)];
  assert.equal(m.length, 1, 'one executable script');
  return m[0][1];
}

test('validateItems accepts good items and names every problem', () => {
  assert.deepEqual(validateItems(items), []);
  assert.ok(validateItems([{ id: 'x', group: 'g', text: 't', extra: 1 }])[0].includes('unknown key'));
  assert.ok(validateItems([{ id: 'x', group: 'g', text: 't', note: '' }])[0].includes('note'));
  assert.ok(validateItems([{ id: 'x', group: 'g', text: 't', tags: [] }])[0].includes('tags'));
  assert.ok(validateItems([{ id: 'x', group: 'g', text: 't' }, { id: 'x', group: 'g', text: 'u' }]).some((e) => e.includes('duplicate')));
  assert.ok(validateItems([]).length);
});

test('the page is one self-contained file with the contract markers', () => {
  const { html, bytes } = buildPage(items, { title: 'Test Review', intro: 'Hello **bold**.\n\n- one\n- two' });
  assert.ok(bytes < MAX_BYTES);
  assert.match(html, /<title>Test Review<\/title>/);
  assert.match(html, /name="claude:viewport-fit-sides"/);
  assert.match(html, /prefers-color-scheme: dark/);
  assert.match(html, /:root\[data-theme="dark"\]/);
  assert.match(html, /body \{ background:var\(--bg\)/);
  assert.match(html, /<strong>bold<\/strong>/);
  assert.match(html, /<li>two<\/li>/);
  assert.doesNotMatch(html, /https?:\/\//, 'no external request or link');
  assert.doesNotMatch(html, /<link |<script src|@import/);
  assert.doesNotMatch(html, /\{\{[A-Z]+\}\}/, 'every placeholder is filled');
});

test('item text cannot break out of the data block, and the script parses', () => {
  const { html } = buildPage(items, { title: 'Test Review' });
  const data = html.match(/<script type="application\/json" id="data">([\s\S]*?)<\/script>/)[1];
  const parsed = JSON.parse(data);
  assert.equal(parsed.items[1].text, 'Second </script><b>line</b>');
  assert.equal(parsed.items.length, 3);
  new Function(scriptOf(html)); // a syntax error throws
});

test('the page has the verdict, listen, keyboard and export machinery', () => {
  const js = scriptOf(buildPage(items, { title: 'Test Review' }).html);
  for (const needle of ['speechSynthesis', 'SpeechSynthesisUtterance', 'u.rate = 1.0', 'voiceschanged', 'localStorage', 'clipboard.writeText', 'decided_at', 'Rejected (', "'jkprlc'"]) {
    assert.ok(js.includes(needle), `script mentions ${needle}`);
  }
  assert.ok(/try \{ var raw = localStorage/.test(js), 'storage read is wrapped');
  assert.ok(/function save\(\) \{ try \{ localStorage/.test(js), 'storage write is wrapped');
});

test('no-listen pages say so in the data', () => {
  const { html } = buildPage(items, { title: 'Test Review', listen: false });
  assert.equal(JSON.parse(html.match(/id="data">([\s\S]*?)<\/script>/)[1]).listen, false);
});

test('oversized pages are refused', () => {
  const big = Array.from({ length: 30000 }, (_, i) => ({ id: `i${i}`, group: 'g', text: 'x'.repeat(100) }));
  assert.throws(() => buildPage(big, { title: 'Big Page' }), /limit/);
});

test('introHtml escapes', () => {
  assert.equal(introHtml('<img src=x>'), '<section class="intro" aria-label="About this review"><p>&lt;img src=x&gt;</p></section>');
  assert.equal(introHtml(''), '');
});

// ---- apply ---------------------------------------------------------------------------------------------------------------------------------
function tempBank(perCombo = 13) {
  const dir = mkdtempSync(join(tmpdir(), 'review-'));
  const lines = [];
  for (const tone of ['mock', 'praise']) for (let i = 1; i <= perCombo; i += 1) lines.push({ id: `win.opponent.${tone}.${String(i).padStart(2, '0')}`, situation: 'win', speaker: 'opponent', tone, text: `Line number ${tone} ${'abcdefghijklmnopqrstuvwxyz'[i]}` });
  const bank = { schema: 'hokej.barks/1', note: 'n', generated: 'g', model: 'm', counts: { 'win.opponent.mock': perCombo, 'win.opponent.praise': perCombo }, lines };
  writeFileSync(join(dir, 'barks.json'), JSON.stringify(bank, null, 2) + '\n');
  writeFileSync(join(dir, 'REVIEW.md'), '# Review\n\n- old\n');
  return { dir, bank, bankPath: join(dir, 'barks.json'), reviewPath: join(dir, 'REVIEW.md') };
}

test('parseDecisions takes the exported shape and refuses nonsense', () => {
  const p = parseDecisions(JSON.stringify({ title: 'T', decided_at: 'now', decisions: [{ id: 'a', verdict: 'reject' }, { id: 'b', comment: 'x' }] }));
  assert.equal(p.decisions.length, 2);
  assert.throws(() => parseDecisions('nope'), /not JSON/);
  assert.throws(() => parseDecisions('{}'), /decisions/);
  assert.throws(() => parseDecisions(JSON.stringify({ decisions: [{ id: 'a', verdict: 'maybe' }] })), /verdict/);
  assert.throws(() => parseDecisions(JSON.stringify({ decisions: [{ id: 'a' }, { id: 'a' }] })), /twice/);
});

test('planApply removes rejected lines, keeps counts true, logs comments, flags short combinations', () => {
  const { bank, dir } = tempBank(13);
  const parsed = parseDecisions(JSON.stringify({ decisions: [
    { id: 'win.opponent.mock.01', verdict: 'reject', comment: 'too soft' },
    { id: 'win.opponent.mock.02', verdict: 'reject' },
    { id: 'win.opponent.praise.01', verdict: 'pass', comment: 'keep, rewrite later' },
    { id: 'nope.1', verdict: 'reject' },
  ] }));
  const plan = planApply(bank, parsed);
  assert.equal(plan.rejected.length, 2);
  assert.equal(plan.next.lines.length, 24);
  assert.equal(plan.next.counts['win.opponent.mock'], 11);
  assert.equal(plan.next.counts['win.opponent.praise'], 13);
  assert.deepEqual(plan.unknown, ['nope.1']);
  assert.equal(plan.comments.length, 2);
  assert.ok(plan.problems.some((p) => p.startsWith('short combination win.opponent.mock')));
  assert.equal(bank.lines.length, 26, 'the input bank is not mutated');
  rmSync(dir, { recursive: true });
});

test('planApply reports a line that no longer passes the filter', () => {
  const { bank, dir } = tempBank(13);
  bank.lines[0].text = 'that is shit';
  const plan = planApply(bank, { decisions: [] });
  assert.ok(plan.problems.some((p) => p.startsWith('filter win.opponent.mock.01')));
  rmSync(dir, { recursive: true });
});

test('applyDecisions writes the bank and appends to REVIEW.md; dry run writes nothing', () => {
  const t = tempBank(14);
  const parsed = parseDecisions(JSON.stringify({ title: 'Bark Bank Review', decided_at: '2026-11-12T20:00:00Z', decisions: [
    { id: 'win.opponent.mock.03', verdict: 'reject', comment: 'meh' }, { id: 'win.opponent.praise.04', comment: 'sharpen' }] }));
  const before = readFileSync(t.bankPath, 'utf8');
  applyDecisions(parsed, { bankPath: t.bankPath, reviewPath: t.reviewPath, dryRun: true, doEmbed: false });
  assert.equal(readFileSync(t.bankPath, 'utf8'), before);
  assert.equal(readFileSync(t.reviewPath, 'utf8'), '# Review\n\n- old\n');
  const plan = applyDecisions(parsed, { bankPath: t.bankPath, reviewPath: t.reviewPath, doEmbed: false });
  const after = JSON.parse(readFileSync(t.bankPath, 'utf8'));
  assert.equal(after.lines.length, 27);
  assert.ok(!after.lines.some((l) => l.id === 'win.opponent.mock.03'));
  assert.equal(after.counts['win.opponent.mock'], 13);
  assert.equal(plan.problems.length, 0);
  const review = readFileSync(t.reviewPath, 'utf8');
  assert.match(review, /^# Review\n\n- old\n/);
  assert.match(review, /## Operator review "Bark Bank Review" \(2026-11-12T20:00:00Z\)/);
  assert.match(review, /- win\.opponent\.mock\.03: /);
  assert.match(review, /sharpen/);
  assert.ok(reviewSection(parsed, plan, 'x').includes('1 rejected and removed'));
  rmSync(t.dir, { recursive: true });
});

// ---- the real lists ---------------------------------------------------------------------------------------------------------------------------
test('the committed bark bank turns into valid items and a page with a register note', () => {
  const bank = JSON.parse(readFileSync(resolve(repo, 'Game/Content/Barks/barks.json'), 'utf8'));
  const its = barkItems(bank);
  assert.equal(its.length, bank.lines.length);
  assert.deepEqual(validateItems(its), []);
  assert.ok(its.some((i) => i.tags && i.tags[0] === 'mild'));
  const { html, bytes } = buildPage(its, { title: 'Bark Bank Review', intro: barkIntro(bank) });
  assert.ok(bytes < MAX_BYTES);
  assert.match(html, /The bar you asked for/);
  assert.match(html, /What the filter already forbids/);
});

test('the feel proposals turn into valid items with their numbers in the note', () => {
  const its = feelItems(readFileSync(resolve(repo, 'docs/experiments/FEEL.md'), 'utf8'), readFileSync(resolve(repo, 'docs/experiments/BALANCING.md'), 'utf8'));
  assert.deepEqual(validateItems(its), []);
  assert.ok(its.length >= 9, `at least the nine FEEL.md rows, got ${its.length}`);
  assert.ok(its.every((i) => i.note && /\d/.test(i.note)), 'every proposal carries numbers');
  assert.ok(its.some((i) => i.text.includes('puck.drag')));
});
