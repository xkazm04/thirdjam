import { test, before, after } from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import { spawnSync } from 'node:child_process';
import { fileURLToPath } from 'node:url';
import { buildPage } from './build.mjs';
import { parseArgs } from './lib/cli.mjs';
import { writeFixtureShots, makePng, FAKE_KEY, FAKE_EMAIL, FAKE_GH_TOKEN } from './fixtures/make-fixtures.mjs';
import { parseNotes, markdownToHtml } from './lib/markdown.mjs';
import { parseTranscript } from './lib/transcripts.mjs';
import { parseGates, SHOT_RE, pngSize } from './lib/inputs.mjs';
import { deviceForRef } from './lib/gitlog.mjs';
import { TmpRepo } from '../tools/gate/lib/tmprepo.mjs';

const HERE = path.dirname(fileURLToPath(import.meta.url));
const FX = path.join(HERE, 'fixtures');
let tmp;
let shotsDir;

before(() => {
  tmp = fs.mkdtempSync(path.join(os.tmpdir(), 'jam-devlog-test-'));
  shotsDir = path.join(tmp, 'shots');
  writeFixtureShots(shotsDir);
});
after(() => fs.rmSync(tmp, { recursive: true, force: true }));

function fixtureArgs(extra = {}) {
  return {
    ...parseArgs([
      '--out', path.join(tmp, 'index.html'),
      '--transcripts', `A=${path.join(FX, 'transcripts', 'A')}`,
      '--transcripts', `B=${path.join(FX, 'transcripts', 'B')}`,
      '--shots', shotsDir,
      '--status', path.join(FX, 'status'),
      '--notes', path.join(FX, 'notes.md'),
    ]),
    ...extra,
  };
}

const PLANTED = [FAKE_KEY, FAKE_EMAIL, FAKE_GH_TOKEN, 'TESTFIXTURE', 'fakeuser', 'test.person', 'C:\\Users'];

test('fixture page: structure', () => {
  const { html, model } = (() => {
    const r = buildPage(fixtureArgs({ embed: true }));
    return r;
  })();
  assert.match(html, /^<!doctype html>/);
  assert.match(html, /<html lang="en">/);
  assert.match(html, /<meta name="viewport"/);
  assert.match(html, /<title>Third Jam dev log<\/title>/);
  assert.match(html, /<svg [^>]*role="img"/);
  // hour ruler H0..H93
  for (const h of [0, 6, 12, 24, 48, 72, 90]) assert.ok(html.includes(`>H${h}</text>`), `ruler label H${h}`);
  assert.ok(html.includes('H93') || html.includes('0 0 2628'), 'ruler spans to H93');
  assert.equal((html.match(/class="tick/g) || []).length, 94 + 1, 'one tick per hour H0..H93 plus the axis line');
  // gates from docs/GATES.md, marked on the ruler and in the journal
  for (const g of ['Gate 1', 'Gate 2', 'Gate 3', 'Feature freeze', 'Submission candidate', 'Submission']) {
    assert.ok(html.includes(`>${g}<title>`), `gate on ruler: ${g}`);
  }
  assert.equal(model.gates.length, 6);
  assert.ok(html.includes('class="gate-banner"'));
  // lanes
  assert.ok(html.includes('bar-a') && html.includes('bar-b'));
  // journal
  for (const id of ['h6', 'h13', 'h24']) assert.ok(html.includes(`id="${id}"`), id);
  // 25 turns: 14 + 1 main A (duplicated records counted once), 3 subagent, 7 B
  assert.equal(model.summary.turns, 25);
  assert.deepEqual(model.summary.sessions, { main: 2, subagent: 1 });
  assert.equal(model.summary.shots, 3);
  assert.ok(model.summary.tokens.output > 0);
  // notes rendered with the markdown subset
  assert.ok(html.includes('<strong>rubric</strong>'));
  assert.ok(html.includes('<code>inline code</code>'));
  assert.match(html, /<ol><li>Greybox arena placed<\/li>/);
  assert.match(html, /<pre><code>node tools\/gate\/check-ownership\.mjs/);
  // raw html in notes is escaped
  assert.ok(!html.includes('<script>alert'));
  assert.ok(html.includes('&lt;script&gt;alert'));
  // status files
  assert.ok(html.includes('class="status-card"'));
  assert.ok(html.includes('packaged build age'));
  // screenshots embedded, captions present, bad name skipped
  assert.equal((html.match(/<img /g) || []).length, 3);
  assert.equal((html.match(/src="data:image\/png;base64,/g) || []).length, 3);
  assert.ok(html.includes('Look study test asset'));
  // summary strip and colophon
  assert.ok(html.includes('class="strip"'));
  assert.match(html, /Redaction pass: \d+ secrets?/);
  // light and dark
  assert.ok(html.includes('prefers-color-scheme: dark'));
  assert.ok(html.includes('color-scheme: light dark'));
  // narrow screens
  assert.ok(html.includes('minmax(0, 1fr)') && html.includes('overflow-x: auto'));
});

test('fixture page: self-contained, no external requests', () => {
  const { html } = buildPage(fixtureArgs({ embed: true }));
  assert.ok(!/<link\b/i.test(html), 'no <link>');
  assert.ok(!/@import/i.test(html), 'no @import');
  assert.ok(!/src\s*=\s*["']https?:/i.test(html), 'no external src');
  assert.ok(!/<(?!a\b)[a-z]+\b[^>]*\bhref\s*=\s*["']https?:/i.test(html), 'only <a> may carry an external href');
  assert.ok(!/url\(\s*["']?https?:/i.test(html), 'no external css url()');
  assert.ok(!/<script[^>]+src=/i.test(html), 'no external script');
  assert.ok(!/<iframe/i.test(html));
});

test('relative screenshots without --embed', () => {
  const { html } = buildPage(fixtureArgs({ embed: false }));
  assert.ok(!html.includes('data:image'));
  const srcs = [...html.matchAll(/<img src="([^"]+)"/g)].map((m) => m[1]);
  assert.equal(srcs.length, 3);
  for (const s of srcs) assert.ok(s.startsWith('shots/'), s);
});

test('redaction: planted secrets, emails and home paths never reach the page', () => {
  for (const embed of [true, false]) {
    const { html, stats } = buildPage(fixtureArgs({ embed }));
    const plain = html.replace(/data:image\/[a-z]+;base64,[A-Za-z0-9+/=]+/g, '');
    for (const needle of PLANTED) assert.ok(!plain.includes(needle), `leaked: ${needle}`);
    assert.ok(!plain.includes(os.homedir()), 'home directory leaked');
    assert.ok(!/[A-Za-z0-9._%+-]+@[A-Za-z0-9-]+\.[A-Za-z]{2,}/.test(plain), 'an email address leaked');
    assert.ok(!/sk-ant|ghp_|github_pat_/.test(plain));
    assert.ok(stats.key >= 3 && stats.email >= 2 && stats.path >= 1, JSON.stringify(stats));
    assert.ok(plain.includes('[redacted-key]') && plain.includes('[email]'));
  }
});

test('redaction: real home path in a transcript title and in a note', () => {
  const dir = fs.mkdtempSync(path.join(tmp, 'real-'));
  const home = os.homedir();
  const rec = (o) => JSON.stringify(o);
  fs.writeFileSync(
    path.join(dir, 'x.jsonl'),
    [
      rec({ type: 'ai-title', aiTitle: `Edit ${home}\\proj\\a.txt and ${home.replace(/\\/g, '/')}/b.txt`, sessionId: 's' }),
      rec({ type: 'assistant', timestamp: '2026-11-12T08:00:00+01:00', message: { id: 'm1', usage: { input_tokens: 1, output_tokens: 2 } } }),
    ].join('\n'),
  );
  const notes = path.join(dir, 'notes.md');
  fs.writeFileSync(notes, `## H3 Paths\n\nOpened ${home}\\x and ${home.replace(/\\/g, '/')}/y.\n`);
  const { html } = buildPage({ ...parseArgs(['--out', path.join(dir, 'o.html'), '--transcripts', dir, '--shots', path.join(dir, 'none'), '--status', path.join(dir, 'none'), '--notes', notes]) });
  assert.ok(!html.toLowerCase().includes(home.toLowerCase()), 'home leaked');
  assert.ok(!html.toLowerCase().includes(home.replace(/\\/g, '/').toLowerCase()));
  assert.ok(html.includes('~'));
});

test('git history: commits placed by hour, lanes by branch, subjects redacted', () => {
  const repo = new TmpRepo();
  try {
    repo.write('a.txt');
    repo.commit('prep: before the jam', '2026-11-11T20:00:00+01:00');
    repo.write('b.txt');
    repo.commit('first commit at H4, mail test.person@example.com', '2026-11-12T10:30:00+01:00');
    repo.git('checkout', '-q', '-b', 'dev-b');
    repo.write('c.txt');
    repo.commit(`device b commit ${FAKE_KEY}`, '2026-11-12T10:45:00+01:00');
    repo.git('checkout', '-q', 'main');
    repo.write('d.txt');
    repo.commit('another at H30', '2026-11-13T12:00:00+01:00');
    const { html, model } = buildPage({ ...parseArgs(['--out', path.join(tmp, 'g.html'), '--repo', repo.dir, '--shots', path.join(tmp, 'none'), '--status', path.join(tmp, 'none'), '--notes', path.join(tmp, 'none.md')]) });
    assert.equal(model.summary.commits, 4);
    assert.equal(model.ruler[4].A.commits, 1);
    assert.equal(model.ruler[4].B.commits, 1);
    assert.equal(model.ruler[30].A.commits, 1);
    assert.equal(model.outside.pre.commits.length, 1);
    // a commit that main and dev-b share is A's, whichever branch git names first (real-data finding, H6)
    assert.equal(model.outside.pre.commits[0].device, 'A');
    assert.ok(html.includes('id="pre"'), 'pre-jam bucket rendered');
    assert.ok(html.includes('<section class="hour" id="h4"'));
    assert.ok(!html.includes('test.person@example.com') && !html.includes('sk-ant'));
    assert.ok(!html.includes(repo.dir), 'temp path leaked');
    // explicit device label wins over branch heuristics
    const b = buildPage({ ...parseArgs(['--out', path.join(tmp, 'g2.html'), '--repo', `B=${repo.dir}`, '--shots', path.join(tmp, 'none'), '--status', path.join(tmp, 'none'), '--notes', path.join(tmp, 'none.md')]) });
    assert.equal(b.model.ruler[4].B.commits, 2);
  } finally {
    repo.cleanup();
  }
});

test('an empty repository and missing inputs are not errors', () => {
  const repo = new TmpRepo();
  try {
    const { html, model } = buildPage({ ...parseArgs(['--out', path.join(tmp, 'e.html'), '--repo', repo.dir, '--shots', path.join(tmp, 'none'), '--status', path.join(tmp, 'none'), '--notes', path.join(tmp, 'none.md'), '--gates', path.join(tmp, 'none-gates.md')]) });
    assert.equal(model.summary.commits, 0);
    assert.match(html, /Nothing recorded yet/);
  } finally {
    repo.cleanup();
  }
});

test('the page is deterministic', () => {
  const a = buildPage(fixtureArgs({ embed: true })).html;
  const b = buildPage(fixtureArgs({ embed: true })).html;
  assert.equal(a, b);
});

test('CLI: literal acceptance command exits 0 and writes a page; bad args exit 2', () => {
  const out = path.join(tmp, 'cli.html');
  const ok = spawnSync(process.execPath, [path.join(HERE, 'build.mjs'), '--out', out], { encoding: 'utf8' });
  assert.equal(ok.status, 0, ok.stdout + ok.stderr);
  assert.ok(fs.existsSync(out));
  assert.match(ok.stdout, /PASS wrote/);
  const bad = spawnSync(process.execPath, [path.join(HERE, 'build.mjs'), '--nonsense'], { encoding: 'utf8' });
  assert.equal(bad.status, 2);
  const noOut = spawnSync(process.execPath, [path.join(HERE, 'build.mjs')], { encoding: 'utf8' });
  assert.equal(noOut.status, 2);
  const missingRepo = spawnSync(process.execPath, [path.join(HERE, 'build.mjs'), '--out', out, '--repo', path.join(tmp, 'no-such-repo')], { encoding: 'utf8' });
  assert.equal(missingRepo.status, 2);
  const fx = spawnSync(process.execPath, [path.join(HERE, 'build.mjs'), '--out', path.join(tmp, 'cli-fx.html'), '--fixtures', '--embed'], { encoding: 'utf8' });
  assert.equal(fx.status, 0, fx.stdout + fx.stderr);
  assert.match(fx.stdout, /3 screenshots/);
});

// ---------- units ----------
test('transcripts: one turn per message id, max usage, tolerant of junk', () => {
  const f = path.join(tmp, 't.jsonl');
  const lines = [
    '{"type":"assistant","timestamp":"2026-11-12T06:00:00Z","message":{"id":"m1","usage":{"input_tokens":1,"output_tokens":5}}}',
    '{"type":"assistant","timestamp":"2026-11-12T06:00:01Z","message":{"id":"m1","usage":{"input_tokens":1,"output_tokens":9,"cache_read_input_tokens":100}}}',
    '{"type":"assistant","timestamp":"2026-11-12T06:05:00Z","message":{"id":"m2"}}',
    'not json',
    '{"type":"assistant","message":{"id":"m3","usage":{"output_tokens":1}}}',
    '[1,2]',
    '{"type":"user","timestamp":"2026-11-12T06:06:00Z","message":{"content":"hello"}}',
    '{"type":"user","timestamp":"2026-11-12T06:06:00Z","isMeta":true,"message":{"content":"caveat"}}',
    '{"type":"user","timestamp":"2026-11-12T06:07:00Z","message":{"content":[{"type":"tool_result"}]}}',
  ];
  fs.writeFileSync(f, lines.join('\n'));
  const s = parseTranscript(f, 'B');
  assert.equal(s.turns.length, 2);
  assert.equal(s.turns[0].usage.output, 9);
  assert.equal(s.turns[0].usage.cacheRead, 100);
  assert.equal(s.turns[1].usage.output, 0);
  assert.equal(s.badLines, 1);
  assert.equal(s.droppedTurns, 1);
  assert.equal(s.prompts.length, 1);
  assert.equal(s.device, 'B');
});

test('notes: blocks by hour, markdown subset', () => {
  const n = parseNotes('intro\n\n## H12 Twelve\n\nbody **b**\n\n## H3.5: Early\n\n- a\n- b\n');
  assert.deepEqual(n.map((x) => [x.hour, x.title]), [[3.5, 'Early'], [12, 'Twelve']]);
  const h = markdownToHtml('Para one\ncontinues "quoted" and it\'s fine...\n\n* x\n* y\n\n1. one\n2. two\n\n`a<b` and [l](https://e.com) and [bad](javascript:alert(1))');
  assert.match(h, /<p>Para one continues \u201Cquoted\u201D and it\u2019s fine\u2026<\/p>/);
  assert.match(h, /<ul><li>x<\/li><li>y<\/li><\/ul>/);
  assert.match(h, /<ol><li>one<\/li><li>two<\/li><\/ol>/);
  assert.match(h, /<code>a&lt;b<\/code>/);
  assert.match(h, /<a href="https:\/\/e\.com" rel="noopener noreferrer">l<\/a>/);
  assert.ok(!h.includes('href="javascript'));
});

test('gates and screenshot names', () => {
  const g = parseGates('# t\n## Gate 1 (H6): core loop\n- x\n## Not a gate\n## Feature freeze (H72)\n');
  assert.deepEqual(g, [
    { name: 'Gate 1', hour: 6, sub: 'core loop' },
    { name: 'Feature freeze', hour: 72, sub: '' },
  ]);
  assert.ok(SHOT_RE.test('20261112-190500-A-greybox.png'));
  assert.ok(SHOT_RE.test('20261112-190500-B-look-study_2.PNG'));
  assert.ok(!SHOT_RE.test('20261112-1905-A-greybox.png'));
  assert.ok(!SHOT_RE.test('20261112-190500-C-greybox.png'));
  assert.deepEqual(pngSize(makePng(96, 54)), { w: 96, h: 54 });
  assert.equal(deviceForRef('refs/heads/dev-b'), 'B');
  assert.equal(deviceForRef('refs/remotes/origin/dev-b'), 'B');
  assert.equal(deviceForRef('refs/heads/dev-a'), 'A');
  assert.equal(deviceForRef('refs/heads/main'), 'A');
});

test('the real docs/GATES.md names the six gates at the right hours', () => {
  const gates = parseGates(fs.readFileSync(path.join(HERE, '..', 'docs', 'GATES.md'), 'utf8'));
  assert.deepEqual(gates.map((g) => g.hour), [6, 24, 48, 72, 84, 88]);
});

test('shot helper: names it writes are names the builder reads', async () => {
  const { shotName } = await import('./shot.mjs');
  const { parseShotStamp } = await import('./lib/time.mjs');
  const ms = Date.parse('2026-11-12T19:05:09+01:00');
  const n = shotName('B', 'look-study', 'PNG', ms);
  assert.equal(n, '20261112-190509-B-look-study.png');
  const m = n.match(SHOT_RE);
  assert.ok(m);
  assert.equal(parseShotStamp(m[1], m[2]), ms);
  assert.throws(() => shotName('C', 'x', 'png'));
  assert.throws(() => shotName('A', 'bad label', 'png'));
});
