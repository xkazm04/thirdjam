import { test, before, after } from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import { spawnSync } from 'node:child_process';
import { fileURLToPath } from 'node:url';
import { TmpRepo, cleanGitEnv } from './lib/tmprepo.mjs';
import { globToRegExp, loadOwnership, decide, ownersOf } from './check-ownership.mjs';

const HERE = path.dirname(fileURLToPath(import.meta.url));
const SCRIPT = path.join(HERE, 'check-ownership.mjs');
const OWNERSHIP = path.join(HERE, '..', '..', 'ownership.json');

let repo;
before(() => {
  repo = new TmpRepo();
  fs.copyFileSync(OWNERSHIP, path.join(repo.dir, 'ownership.json'));
  repo.write('README.md', 'jam\n');
  repo.write('Game/Source/JamCore/Cartridge.cpp', 'a\n');
  repo.write('Game/Plugins/JamAssets/Content/T_Old.uasset', 'b\n');
  repo.write('docs/PLAN.md', 'plan\n');
  repo.commit('main baseline', '2026-11-11T20:00:00+01:00');
});
after(() => repo.cleanup());

function check(device, extra = []) {
  const r = spawnSync(process.execPath, [SCRIPT, '--device', device, '--repo', repo.dir, ...extra], { encoding: 'utf8', env: cleanGitEnv() });
  return { code: r.status, out: r.stdout + r.stderr };
}
const reset = () => {
  repo.git('checkout', '-q', 'main');
  repo.git('reset', '-q', '--hard', 'HEAD');
  repo.git('clean', '-q', '-fd');
  for (const b of repo.git('branch', '--format=%(refname:short)').split('\n').filter((x) => x && x !== 'main')) repo.git('branch', '-q', '-D', b);
};

test('clean repository state passes for A, B and the Director', () => {
  reset();
  for (const d of ['A', 'B', 'director']) {
    const r = check(d);
    assert.equal(r.code, 0, r.out);
    assert.match(r.out, /PASS ownership: 0 changed/);
    assert.match(r.out, /"ok":true/);
  }
});

test('A changing only A paths and shared paths passes', () => {
  reset();
  repo.git('checkout', '-q', '-b', 'dev-a');
  repo.write('Game/Source/JamGame/Pawn.cpp');
  repo.write('Game/Content/Maps/L_Test.umap');
  repo.write('schema/cartridge.schema.json', '{}\n');
  repo.write('status/A.md', '## H1\n');
  repo.write('handoff/A-to-B-0001.md', 'req\n');
  repo.commit('a work', '2026-11-12T08:00:00+01:00');
  const r = check('A');
  assert.equal(r.code, 0, r.out);
  assert.match(r.out, /5 changed path/);
});

test('A touching a path owned by B fails and lists it (committed on the branch)', () => {
  reset();
  repo.git('checkout', '-q', '-b', 'dev-a');
  repo.write('Game/Source/JamGame/Pawn.cpp');
  repo.write('Game/Plugins/JamAssets/Content/T_New.uasset');
  repo.commit('a crosses the line', '2026-11-12T08:00:00+01:00');
  const r = check('A');
  assert.equal(r.code, 1, r.out);
  assert.match(r.out, /FAIL Game\/Plugins\/JamAssets\/Content\/T_New\.uasset: owned by device B/);
  assert.ok(!/FAIL Game\/Source/.test(r.out), 'owned path is not listed');
  assert.equal(check('B').code, 1, 'and B in turn may not touch A paths');
});

test('working-tree changes count: unstaged edit, staged add, untracked, deletion, rename', () => {
  reset();
  repo.git('checkout', '-q', '-b', 'dev-a');
  // unstaged edit of a B file
  repo.write('Game/Plugins/JamAssets/Content/T_Old.uasset', 'changed\n');
  let r = check('A');
  assert.equal(r.code, 1, r.out);
  assert.match(r.out, /T_Old\.uasset/);
  reset();
  repo.git('checkout', '-q', '-b', 'dev-a');
  // untracked file in a B folder
  repo.write('Game/Content/Look/PP_Test.uasset');
  r = check('A');
  assert.equal(r.code, 1, r.out);
  assert.match(r.out, /Game\/Content\/Look\/PP_Test\.uasset/);
  reset();
  repo.git('checkout', '-q', '-b', 'dev-a');
  // staged deletion of a B file
  repo.git('rm', '-q', 'Game/Plugins/JamAssets/Content/T_Old.uasset');
  r = check('A');
  assert.equal(r.code, 1, r.out);
  reset();
  repo.git('checkout', '-q', '-b', 'dev-a');
  // rename out of A territory into B territory: both ends are listed
  fs.mkdirSync(path.join(repo.dir, 'Game/Content/Audio'), { recursive: true });
  repo.git('mv', 'Game/Source/JamCore/Cartridge.cpp', 'Game/Content/Audio/Cartridge.cpp');
  r = check('A');
  assert.equal(r.code, 1, r.out);
  assert.match(r.out, /Game\/Content\/Audio\/Cartridge\.cpp/);
});

test('director-only and unowned paths fail for devices; Director may edit docs', () => {
  reset();
  repo.git('checkout', '-q', '-b', 'dev-b');
  repo.write('docs/PLAN.md', 'plan edited\n');
  repo.write('contracts/new.md');
  repo.write('mystery/file.txt');
  let r = check('B');
  assert.equal(r.code, 1, r.out);
  assert.match(r.out, /FAIL docs\/PLAN\.md: director only/);
  assert.match(r.out, /FAIL contracts\/new\.md: director only/);
  assert.match(r.out, /FAIL mystery\/file\.txt: owned by nobody/);
  repo.git('checkout', '-q', 'main');
  reset();
  repo.write('docs/PLAN.md', 'plan edited\n');
  repo.write('ownership.json', fs.readFileSync(OWNERSHIP, 'utf8') + '\n');
  r = check('director');
  assert.equal(r.code, 0, r.out);
  repo.write('Game/Source/x.cpp');
  r = check('director');
  assert.equal(r.code, 1, r.out);
});

test('B owns its own paths; editing ownership.json is director-only', () => {
  reset();
  repo.git('checkout', '-q', '-b', 'dev-b');
  repo.write('Game/Plugins/JamAssets/Content/SM_TestAsset.uasset');
  repo.write('Game/Content/Audio/SFX_Test.wav');
  repo.write('ASSETS.md', 'row\n');
  assert.equal(check('B').code, 0);
  repo.write('ownership.json', fs.readFileSync(OWNERSHIP, 'utf8').replace('"A": [', '"A": [\n      "docs/**",'));
  const r = check('B');
  assert.equal(r.code, 1, r.out);
  assert.match(r.out, /ownership\.json: director only/);
});

test('exit 2 when it cannot run', () => {
  reset();
  // missing base
  let r = check('A', ['--base', 'no-such-branch']);
  assert.equal(r.code, 2, r.out);
  assert.match(r.out, /base ref "no-such-branch" does not exist/);
  // bad device and missing device
  assert.equal(spawnSync(process.execPath, [SCRIPT, '--device', 'C', '--repo', repo.dir], { encoding: 'utf8' }).status, 2);
  assert.equal(spawnSync(process.execPath, [SCRIPT, '--repo', repo.dir], { encoding: 'utf8' }).status, 2);
  assert.equal(spawnSync(process.execPath, [SCRIPT, '--device', 'A', '--bogus'], { encoding: 'utf8' }).status, 2);
  // not a git repository
  const plain = fs.mkdtempSync(path.join(os.tmpdir(), 'jam-not-a-repo-'));
  try {
    const x = spawnSync(process.execPath, [SCRIPT, '--device', 'A', '--repo', plain], { encoding: 'utf8', env: cleanGitEnv({ GIT_CEILING_DIRECTORIES: os.tmpdir() }) });
    assert.equal(x.status, 2, x.stdout + x.stderr);
  } finally {
    fs.rmSync(plain, { recursive: true, force: true });
  }
  // unreadable or invalid table
  const bad = path.join(repo.dir, 'bad-table.json');
  fs.writeFileSync(bad, '{"schema":"thirdjam.ownership/1","owners":{"A":[],"B":[],"C":[]},"shared":{"any_device":[],"director_only":[]}}');
  r = check('A', ['--ownership', bad]);
  assert.equal(r.code, 2, r.out);
  assert.match(r.out, /unknown key "C"/);
  fs.rmSync(bad);
});

test('exit 2 when the table is ambiguous', () => {
  reset();
  const amb = path.join(repo.dir, 'amb.json');
  const t = JSON.parse(fs.readFileSync(OWNERSHIP, 'utf8'));
  t.owners.B.push('Game/Source/**');
  fs.writeFileSync(amb, JSON.stringify(t));
  repo.git('checkout', '-q', '-b', 'dev-a');
  repo.write('Game/Source/JamCore/X.cpp');
  const r = check('A', ['--ownership', amb]);
  assert.equal(r.code, 2, r.out);
  fs.rmSync(amb);
});

// ---------- units on the real table ----------
test('real ownership.json matches PLAN.md section 5', () => {
  const t = loadOwnership(OWNERSHIP);
  const o = (p) => ownersOf(p, t);
  for (const p of ['Game/Source/JamCore/A.cpp', 'Game/Content/Maps/L.umap', 'Game/Content/Cartridges/c.json', 'Game/Config/DefaultGame.ini', 'tools/x.mjs', 'schema/a.json', 'prompts/a.md']) {
    assert.deepEqual(o(p), ['A'], p);
  }
  for (const p of ['Game/Plugins/JamAssets/Content/a.uasset', 'Game/Content/Look/a.uasset', 'Game/Content/Audio/a.wav']) assert.deepEqual(o(p), ['B'], p);
  for (const p of ['docs/PLAN.md', 'contracts/a.md', 'ownership.json']) assert.deepEqual(o(p), ['director_only'], p);
  for (const p of ['handoff/A-to-B-0007.md', 'status/A.md', 'ASSETS.md', 'devlog/notes.md']) assert.deepEqual(o(p), ['any_device'], p);
  assert.deepEqual(o('Game/Content/Looks/x'), [], 'prefix must not match a sibling folder');
  assert.equal(decide('Game/Source', 'A', t).ok, true, 'the folder itself');
  assert.equal(decide('game/source/x.cpp', 'A', t).ok, true, 'case-insensitive');
});

test('glob matching', () => {
  const m = (g, p) => globToRegExp(g).test(p);
  assert.ok(m('a/**', 'a/b/c.txt'));
  assert.ok(m('a/**', 'a'));
  assert.ok(!m('a/**', 'ab/c'));
  assert.ok(m('Game/*.uproject', 'Game/Jam.uproject'));
  assert.ok(!m('Game/*.uproject', 'Game/Sub/Jam.uproject'));
  assert.ok(m('**/x.md', 'x.md'));
  assert.ok(m('**/x.md', 'a/b/x.md'));
  assert.ok(m('README.md', 'README.md'));
  assert.ok(!m('README.md', 'READMEXmd'));
});
