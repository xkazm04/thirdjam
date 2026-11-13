// Lane A114 (track Q3): the native tournament build compiles (JamAICore.h uses JAMCORE_API, defined by UBT only; run.mjs defines it empty).
// Runs in the premerge gate: node --test tools/**/*.test.mjs. The compile step is skipped when no clang++ is on the machine.
import { test } from 'node:test';
import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { mkdtempSync, existsSync, rmSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import { nativeArgs, findCxx } from './run.mjs';

test('the native build defines JAMCORE_API empty', () => {
  assert.ok(nativeArgs('x.exe').includes('-DJAMCORE_API='));
});

test('the native tournament driver compiles', (t) => {
  const cxx = findCxx();
  const probe = spawnSync(cxx, ['--version'], { encoding: 'utf8' });
  if (probe.error || probe.status !== 0) { t.skip(`no C++ compiler (${cxx})`); return; }
  const dir = mkdtempSync(join(tmpdir(), 'jam-tournament-'));
  try {
    const exe = join(dir, 'jam_tournament.exe');
    const r = spawnSync(cxx, nativeArgs(exe), { encoding: 'utf8' });
    assert.equal(r.status, 0, r.stderr);
    assert.ok(existsSync(exe));
  } finally { rmSync(dir, { recursive: true, force: true }); }
});
