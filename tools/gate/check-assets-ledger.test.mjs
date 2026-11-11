import { test, beforeEach, afterEach } from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import { spawnSync } from 'node:child_process';
import { fileURLToPath } from 'node:url';
import { run, parseLedger, COLUMNS, budgetReport } from './check-assets-ledger.mjs';

const HERE = path.dirname(fileURLToPath(import.meta.url));
const SCRIPT = path.join(HERE, 'check-assets-ledger.mjs');
const REAL_LEDGER = fs.readFileSync(path.join(HERE, '..', '..', 'ASSETS.md'), 'utf8');
const HEADER = `| ${COLUMNS.join(' | ')} |\n|${COLUMNS.map(() => '---').join('|')}|\n`;
const ASSET_DIR = 'Game/Plugins/JamAssets/Content';

let root;
beforeEach(() => {
  root = fs.mkdtempSync(path.join(os.tmpdir(), 'jam-ledger-test-'));
});
afterEach(() => fs.rmSync(root, { recursive: true, force: true }));

const row = (o) => {
  const d = { path: `${ASSET_DIR}/SM_TestAsset.uasset`, kind: 'mesh', size_mb: '1.0', origin: 'original', tool: '', licence: '', url: '', date: '2026-11-12', device: 'B', shipped: 'y', note: 'test-asset', ...o };
  return `| ${COLUMNS.map((c) => d[c]).join(' | ')} |\n`;
};
const ledger = (...rows) => fs.writeFileSync(path.join(root, 'ASSETS.md'), `# Assets\n\nprose | with a pipe\n\n${HEADER}${rows.join('')}`);
const touch = (rel, bytes = 100) => {
  const p = path.join(root, rel);
  fs.mkdirSync(path.dirname(p), { recursive: true });
  fs.writeFileSync(p, Buffer.alloc(bytes, 1));
};
const check = () => {
  const lines = [];
  const code = run(['--root', root], { log: (s) => lines.push(s), error: (s) => lines.push(s) });
  return { code, out: lines.join('\n') };
};

test('the shipped ASSETS.md template parses, has no rows, and passes against an absent folder', () => {
  const { rows } = parseLedger(REAL_LEDGER);
  assert.equal(rows.length, 0);
  fs.writeFileSync(path.join(root, 'ASSETS.md'), REAL_LEDGER);
  const r = check();
  assert.equal(r.code, 0, r.out);
  assert.match(r.out, /PASS assets ledger: 0 row/);
  // and through the real CLI against the real repository files
  const cli = spawnSync(process.execPath, [SCRIPT, '--root', path.join(HERE, '..', '..')], { encoding: 'utf8' });
  assert.equal(cli.status, 0, cli.stdout + cli.stderr);
  const last = JSON.parse(cli.stdout.trim().split('\n').pop());
  assert.equal(last.step, 'assets-ledger');
  assert.equal(last.ok, true);
});

test('absent asset folder passes with a ledger of rows too', () => {
  ledger(row({ path: 'somewhere/else.png' }));
  assert.equal(check().code, 0);
});

test('a file without a row fails and is listed', () => {
  ledger(row());
  touch(`${ASSET_DIR}/SM_TestAsset.uasset`, 1024 * 1024);
  touch(`${ASSET_DIR}/Sub/T_Missing.uasset`);
  touch(`${ASSET_DIR}/.gitkeep`);
  const r = check();
  assert.equal(r.code, 1, r.out);
  assert.match(r.out, /FAIL no row in ASSETS\.md for Game\/Plugins\/JamAssets\/Content\/Sub\/T_Missing\.uasset/);
  assert.ok(!/no row in ASSETS\.md for .*SM_TestAsset/.test(r.out));
  assert.ok(!/gitkeep/.test(r.out));
});

test('every file with a row passes; companions fold into their uasset; case and slashes are normalized', () => {
  touch(`${ASSET_DIR}/SM_TestAsset.uasset`, 524288);
  touch(`${ASSET_DIR}/SM_TestAsset.uexp`, 524288);
  touch(`${ASSET_DIR}/SM_TestAsset.ubulk`, 10);
  touch(`${ASSET_DIR}/T_Other.uasset`, 524288);
  ledger(row({ size_mb: '1.0' }), row({ path: ASSET_DIR.replace(/\//g, '\\') + '\\t_other.uasset', kind: 'texture', size_mb: '0.5' }));
  const r = check();
  assert.equal(r.code, 0, r.out);
  assert.ok(!/WARN/.test(r.out), r.out);
});

test('size_mb far from disk size is a warning, not a failure', () => {
  touch(`${ASSET_DIR}/SM_TestAsset.uasset`, 1024 * 1024);
  ledger(row({ size_mb: '9.0' }));
  const r = check();
  assert.equal(r.code, 0, r.out);
  assert.match(r.out, /WARN .*size_mb 9\.0 but the file is 1\.0 MB/);
});

test('art over 350 MB fails, exactly 350 passes; only shipped=y counts', () => {
  ledger(row({ path: 'a.uasset', size_mb: '200.0', device: 'A' }), row({ path: 'b.uasset', kind: 'texture', size_mb: '150.0', device: 'B' }), row({ path: 'c.uasset', kind: 'texture', size_mb: '400.0', shipped: 'n' }));
  let r = check();
  assert.equal(r.code, 0, r.out);
  assert.match(r.out, /device A: art 200 MB/);
  assert.match(r.out, /device B: art 150 MB/);
  assert.match(r.out, /total: art 350 MB/);
  ledger(row({ path: 'a.uasset', size_mb: '200.0', device: 'A' }), row({ path: 'b.uasset', kind: 'texture', size_mb: '150.1', device: 'B' }));
  r = check();
  assert.equal(r.code, 1, r.out);
  assert.match(r.out, /FAIL art budget exceeded: 350\.1 MB shipped, budget 350 MB/);
});

test('audio over 80 MB fails; art and audio are counted separately', () => {
  ledger(row({ path: 'a.ogg', kind: 'audio', size_mb: '80.0' }), row({ path: 'b.ogg', kind: 'audio', size_mb: '0.5' }), row({ path: 'm.uasset', size_mb: '300.0' }));
  const r = check();
  assert.equal(r.code, 1, r.out);
  assert.match(r.out, /FAIL audio budget exceeded: 80\.5 MB/);
  assert.ok(!/art budget exceeded/.test(r.out));
});

test('vfx, font and other are reported with a warning over their slice, never failed', () => {
  ledger(row({ path: 'n.uasset', kind: 'vfx', size_mb: '45.0' }));
  const r = check();
  assert.equal(r.code, 0, r.out);
  assert.match(r.out, /WARN vfx, font and other total 45 MB/);
});

test('malformed rows fail with the reason', () => {
  ledger(
    row({ path: 'a', kind: 'sound' }),
    row({ path: 'b', origin: 'found' }),
    row({ path: 'c', device: 'C' }),
    row({ path: 'd', shipped: 'yes' }),
    row({ path: 'e', size_mb: 'big' }),
    row({ path: 'f', origin: 'sourced' }),
    row({ path: 'g', origin: 'ai-generated' }),
    row({ path: 'h', date: '12/11/2026' }),
    row({ path: 'a' }),
  );
  const r = check();
  assert.equal(r.code, 1, r.out);
  for (const re of [/kind "sound"/, /origin "found"/, /device "C"/, /shipped "yes"/, /size_mb "big"/, /sourced assets need licence and url/, /ai-generated assets need the tool/, /date "12\/11\/2026"/, /duplicate of line/]) {
    assert.match(r.out, re);
  }
});

test('a sourced row with licence and url, and an ai-generated row with a tool, pass', () => {
  ledger(row({ path: 'a', origin: 'sourced', licence: 'CC0', url: 'https://example.com/test-asset' }), row({ path: 'b', origin: 'ai-generated', tool: 'test-tool' }));
  assert.equal(check().code, 0);
});

test('exit 2: missing ledger, no table, wrong columns, bad flag', () => {
  assert.equal(check().code, 2, 'missing file');
  fs.writeFileSync(path.join(root, 'ASSETS.md'), '# nothing here\n');
  assert.equal(check().code, 2);
  fs.writeFileSync(path.join(root, 'ASSETS.md'), '| path | kind |\n|---|---|\n');
  const r = check();
  assert.equal(r.code, 2);
  assert.match(r.out, /columns must be exactly/);
  assert.equal(run(['--nope'], { log() {}, error() {} }), 2);
});

test('budgetReport sums per device and in total', () => {
  const rep = budgetReport([
    { shipped: 'y', kind: 'mesh', size_mb: '1.25', device: 'A' },
    { shipped: 'y', kind: 'texture', size_mb: '2.5', device: 'B' },
    { shipped: 'y', kind: 'audio', size_mb: '3', device: 'B' },
    { shipped: 'n', kind: 'audio', size_mb: '99', device: 'B' },
  ]);
  assert.deepEqual(rep.A, { art: 1.3, audio: 0, other: 0 });
  assert.deepEqual(rep.B, { art: 2.5, audio: 3, other: 0 });
  assert.deepEqual(rep.total, { art: 3.8, audio: 3, other: 0 });
});
