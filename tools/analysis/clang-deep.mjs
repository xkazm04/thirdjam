#!/usr/bin/env node
// Lane A55: the clang static analyzer over Game/Source/Jam* with the checkers UE 5.9 turns off by default put back on.
// UBT's `-StaticAnalyzer=Clang` passes `-analyzer-disable-checker` for core.NullDereference, core.DivideZero, core.uninitialized.*, deadcode.DeadStores,
// cplusplus.NewDelete/Move and others (ModuleRules.StaticAnalyzerDisabledCheckers; no command-line switch changes it), and -Wno-uninitialized /
// -Wno-unused-variable among the warnings. This script reuses the response files that run left in Game/Intermediate (so it must run after
// `tools/analysis/ubt.ps1 -Label analyze-clang -Extra '-StaticAnalyzer=Clang -StaticAnalyzerProjectOnly'`), drops those switches, and runs
// clang-cl per translation unit (analysis only, no object file). Output: Game/logs/a55-clang-deep-<stamp>.log, parsed by warnings.mjs.
// Usage: node tools/analysis/clang-deep.mjs [--jobs 6] [--clang <clang-cl.exe>]
import fs from 'node:fs';
import path from 'node:path';
import { spawn } from 'node:child_process';
import { fileURLToPath } from 'node:url';

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..', '..');
const arg = (k, d) => { const i = process.argv.indexOf(k); return i > 0 ? process.argv[i + 1] : d; };
const JOBS = Number(arg('--jobs', 6));
const ue = process.env.UE_ROOT || 'C:/Program Files/Epic Games/UE_5.9';
const clang = arg('--clang', 'C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/Llvm/x64/bin/clang-cl.exe');
const inter = path.join(root, 'Game', 'Intermediate', 'Build', 'Win64', 'x64', 'UnrealEditor', 'Development');
const out = path.join(root, 'Game', 'Intermediate', 'A55ClangDeep');
fs.mkdirSync(out, { recursive: true });
const stamp = new Date().toISOString().replace(/[-:]/g, '').replace('T', '-').slice(0, 15);
const logPath = path.join(root, 'Game', 'logs', `a55-clang-deep-${stamp}.log`);
const reWarnOn = ['uninitialized', 'unused-variable', 'unused-but-set-variable', 'unused-private-field', 'unused-lambda-capture', 'inconsistent-missing-override', 'unused-function', 'unused-value', 'sometimes-uninitialized'];

const tasks = [];
for (const mod of fs.readdirSync(inter)) {
  const shared = path.join(inter, mod, `${mod}.Shared.rsp`);
  if (!fs.existsSync(shared)) continue;
  let s = fs.readFileSync(shared, 'utf8');
  if (!s.includes('--analyze')) { console.log(`FAIL clang-deep ${mod}.Shared.rsp is not an analyzer response file: run the UBT clang analyzer first`); process.exit(2); }
  s = s.split(/\r?\n/).filter((l) => !l.includes('-analyzer-disable-checker')).join('\n');
  for (const w of reWarnOn) s = s.split(`-Wno-${w} `).join('').split(`-Wno-${w}\n`).join('\n');
  s += `\n${reWarnOn.map((w) => `-W${w}`).join(' ')}\n-Xclang -analyzer-checker=core,cplusplus,deadcode,unix,security.FloatLoopCounter\n`;
  const newShared = path.join(out, `${mod}.Shared.rsp`);
  fs.writeFileSync(newShared, s);
  for (const f of fs.readdirSync(path.join(inter, mod))) {
    if (!f.endsWith('.cpp.d.rsp') || f.includes('.gen.cpp')) continue;
    let r = fs.readFileSync(path.join(inter, mod, f), 'utf8');
    r = r.split(/\r?\n/).map((l) => (l.startsWith('@') && l.includes('.Shared.rsp') ? `@"${newShared.split(path.sep).join('/')}"` : l)).filter((l) => !l.includes('-MF')).join('\n');
    const rsp = path.join(out, f);
    fs.writeFileSync(rsp, r);
    tasks.push({ mod, tu: f.replace('.d.rsp', ''), rsp });
  }
}
const log = fs.createWriteStream(logPath);
console.log(`INFO clang-deep: ${tasks.length} TUs, ${JOBS} at a time, log ${logPath}`);
let next = 0, done = 0, failed = 0;
const t0 = Date.now();
await new Promise((resolve) => {
  const start = () => {
    if (next >= tasks.length) { if (done === tasks.length) resolve(); return; }
    const t = tasks[next++];
    const p = spawn(clang, [`@${t.rsp}`], { cwd: path.join(ue, 'Engine', 'Source') });
    let buf = '';
    p.stdout.on('data', (d) => (buf += d));
    p.stderr.on('data', (d) => (buf += d));
    p.on('close', (code) => {
      log.write(`[${++done}/${tasks.length}] ${t.mod}/${t.tu} exit ${code}\n${buf}\n`);
      if (code !== 0) failed++;
      start();
    });
  };
  for (let i = 0; i < JOBS; i++) start();
});
log.end();
const secs = Math.round((Date.now() - t0) / 1000);
const line = `${failed === 0 ? 'PASS' : 'FAIL'} clang-deep ${tasks.length} TUs in ${secs}s, ${failed} exited non-zero`;
console.log(line);
console.log(JSON.stringify({ schema: 'a55.clang-deep/1', tus: tasks.length, failed, seconds: secs, log: logPath }));
process.exit(failed === 0 ? 0 : 1);
