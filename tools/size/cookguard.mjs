#!/usr/bin/env node
// Lane A175 (track Q3, evolution 7): the cook guard, a gate step proposal (not wired into tools/gate/premerge.ps1: the Director decides).
// Seconds, no Unreal start. Two checks:
//   1. nevercook: the never-cook block in Game/Config/DefaultGame.ini lists exactly device B's unshipped figure families as they are on disk now
//      (a new TB2 / SkaterGen / P4Lab asset from B that is not in the block would be cooked: FAIL, fix with `node tools/size/nevercook.mjs --write`);
//   2. refwalk: no shipped root reaches a never-cooked package, and no cooked package names one (the cooker's "Content is missing from cook" error).
// Usage: node tools/size/cookguard.mjs        PASS/FAIL lines, a JSON line last ({"tool":"thirdjam.gate/1","step":"cookguard",...}); exit 0 pass, 1 fail, 2 could not run.
import { spawnSync } from 'node:child_process';
import path from 'node:path';

const here = path.dirname(new URL(import.meta.url).pathname.replace(/^\/([A-Za-z]:)/, '$1'));
const t0 = Date.now();
const steps = [];
for (const [name, script] of [['nevercook', 'nevercook.mjs'], ['refwalk', 'refwalk.mjs']]) {
  const t = Date.now();
  const r = spawnSync(process.execPath, [path.join(here, script)], { encoding: 'utf8' });
  const out = (r.stdout || '') + (r.stderr || '');
  const verdict = out.split(/\r?\n/).filter(l => /^(PASS|FAIL) /.test(l)).pop() || `FAIL ${name}: no verdict line (exit ${r.status})`;
  const ok = r.status === 0 && verdict.startsWith('PASS');
  if (!ok) for (const l of out.split(/\r?\n/).filter(l => /^  \S/.test(l)).slice(0, 12)) console.log('  |' + l);
  console.log(`${ok ? 'PASS' : 'FAIL'} cookguard/${name}: ${verdict.replace(/^(PASS|FAIL) /, '')} (${((Date.now() - t) / 1000).toFixed(1)} s)`);
  steps.push({ step: name, status: ok ? 'pass' : 'fail', ms: Date.now() - t, exit: r.status });
}
const pass = steps.every(s => s.status === 'pass');
console.log(JSON.stringify({ tool: 'thirdjam.gate/1', step: 'cookguard', verdict: pass ? 'pass' : 'fail', seconds: (Date.now() - t0) / 1000, steps }));
process.exit(pass ? 0 : 1);
