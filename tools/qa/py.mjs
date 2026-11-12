#!/usr/bin/env node
// Lane A10 QA: run a Python file inside the live editor through MCP (execute_python) and print the result text.
//   node tools/qa/py.mjs <script.py> [--url http://127.0.0.1:8010/mcp] [--timeout 120] [--var KEY=VALUE ...]
// --var replaces the token __KEY__ in the script before it is sent (small parameterisation without f-strings).
// Exit codes follow tools/mcp/mcp.mjs: 0 pass, 1 tool error, 2 unreachable, 3 timeout.
import { readFileSync, writeFileSync, mkdirSync } from 'node:fs';
import { spawnSync } from 'node:child_process';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = join(HERE, '..', '..');
const argv = process.argv.slice(2);
function opt(name, dflt) {
  const i = argv.indexOf('--' + name);
  if (i < 0) return dflt;
  const v = argv[i + 1];
  argv.splice(i, 2);
  return v;
}
const url = opt('url', process.env.UE_MCP_URL || 'http://127.0.0.1:8010/mcp');
const timeout = opt('timeout', '120');
const vars = {};
for (;;) {
  const v = opt('var', null);
  if (v === null) break;
  const k = v.slice(0, v.indexOf('='));
  vars[k] = v.slice(k.length + 1);
}
const file = argv[0];
if (!file) {
  console.error('usage: node tools/qa/py.mjs <script.py> [--url u] [--timeout s] [--var K=V]');
  process.exit(2);
}
let src = readFileSync(file, 'utf8');
for (const [k, v] of Object.entries(vars)) src = src.split('__' + k + '__').join(v);
const outDir = join(HERE, 'out');
mkdirSync(outDir, { recursive: true });
const argFile = join(outDir, '_py_args.json');
writeFileSync(argFile, JSON.stringify({ script: { content: src, language: 'Python' } }));
const r = spawnSync(process.execPath, [
  join(ROOT, 'tools', 'mcp', 'mcp.mjs'), 'call',
  'editor_toolset.toolsets.python_scripting.PythonScriptingTools.execute_python',
  '@' + argFile, '--url', url, '--timeout', timeout,
], { encoding: 'utf8', env: { ...process.env, MSYS_NO_PATHCONV: '1' } });
process.stdout.write(r.stdout);
process.stderr.write(r.stderr);
process.exit(r.status ?? 2);
