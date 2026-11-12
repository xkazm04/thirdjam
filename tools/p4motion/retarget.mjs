#!/usr/bin/env node
// tools/p4motion/retarget.mjs: runs tools/p4motion/ue_retarget.py inside the editor on the lane's MCP port (8016) for one clip.
//   node tools/p4motion/retarget.mjs --fbx <stripped.fbx> --clip AS_P4_skate_fwd [--vendor meshy] [--url http://127.0.0.1:8016/mcp] [--rate 60]
// Writes the argument file into the scratch dir (P4M_SCRATCH or %TEMP%), sends a 4-line Python wrapper through mcp.mjs, prints the P4M_RESULT JSON line.
// Exit 0 ok, 1 the retarget reported a failure, 2 could not run.
import { spawnSync } from 'node:child_process';
import { mkdirSync, writeFileSync } from 'node:fs';
import { resolve, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
import { tmpdir } from 'node:os';

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..');
const o = {};
for (let i = 2; i < process.argv.length; i += 2) o[process.argv[i].replace(/^--/, '')] = process.argv[i + 1];
if (!o.fbx || !o.clip) { console.error('usage: retarget.mjs --fbx <file> --clip <name> [--vendor meshy] [--url ...] [--rate 60]'); process.exit(2); }
const vendor = o.vendor || 'meshy';
const url = o.url || process.env.UE_MCP_URL || 'http://127.0.0.1:8016/mcp';
const scratch = process.env.P4M_SCRATCH || resolve(tmpdir(), 'p4motion');
mkdirSync(scratch, { recursive: true });
const args = {
  fbx: resolve(o.fbx).replace(/\\/g, '/'), clip: o.clip, vendor,
  bonemap: resolve(here, `bonemap-${vendor}.json`).replace(/\\/g, '/'),
  src_folder: `/Game/P4/Anim/Src/${vendor.charAt(0).toUpperCase() + vendor.slice(1)}`,
  src_mesh_name: o.src_mesh || (vendor === 'meshy' ? 'SK_P4_MeshyRef' : `SK_P4_${vendor}Src`),
  out_folder: '/Game/P4/Anim',
  target_mesh: o.target || '/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple',
  sample_rate: Number(o.rate || 60), reuse_source_skeleton: true,
};
const argsFile = resolve(scratch, `retarget-${o.clip}.json`).replace(/\\/g, '/');
writeFileSync(argsFile, JSON.stringify(args));
const script = resolve(here, 'ue_retarget.py').replace(/\\/g, '/');
const py = `import os, io, contextlib\nos.environ['P4M_ARGS'] = ${JSON.stringify(argsFile)}\nbuf = io.StringIO()\nwith contextlib.redirect_stdout(buf):\n    exec(compile(open(${JSON.stringify(script)}).read(), ${JSON.stringify(script)}, 'exec'), {})\n[l for l in buf.getvalue().splitlines() if l.startswith('P4M_RESULT')][-1][11:]`;
const callFile = resolve(scratch, `retarget-${o.clip}.call.json`);
writeFileSync(callFile, JSON.stringify({ script: { language: 'Python', content: py } }));
const r = spawnSync(process.execPath, [resolve(repo, 'tools/mcp/mcp.mjs'), 'call', 'editor_toolset.toolsets.python_scripting.PythonScriptingTools.execute_python', '@' + callFile, '--url', url, '--timeout', '600'], { encoding: 'utf8', maxBuffer: 1 << 26 });
// mcp.mjs prints the MCP result as pretty JSON, then one verdict line; the Python return value is content[0].text -> {"returnValue": "<our JSON>"}
const text = (r.stdout || '') + (r.stderr || '');
let result = null;
try {
  const lines = text.trimEnd().split('\n');
  const verdict = JSON.parse(lines.pop());
  const body = JSON.parse(lines.join('\n'));
  const inner = JSON.parse(body.content[0].text);
  if (inner.returnValue) result = JSON.parse(inner.returnValue);
  else result = { tool: 'thirdjam.p4motion-retarget/1', clip: o.clip, ok: false, error: (verdict.error || JSON.stringify(inner)).slice(0, 600) };
} catch (e) {
  const err = text.match(/"error":"((?:[^"\\]|\\.)*)"/);
  result = { tool: 'thirdjam.p4motion-retarget/1', clip: o.clip, ok: false, error: err ? JSON.parse('"' + err[1] + '"').slice(0, 600) : text.slice(-600) };
}
console.log(`${result.ok ? 'PASS' : 'FAIL'} retarget ${o.clip}${result.out ? ' -> ' + result.out : ''}${result.error ? ': ' + result.error : ''}`);
console.log(JSON.stringify(result));
process.exitCode = result.ok ? 0 : 1;
