// The CLI model (ruling R-0004 point 2): one `claude -p` process per attempt, through the operator's Claude Code login.
// Same contract as live-model.mjs: callModel(body) -> { text, stop_reason, usage, ms }. The body is the one assemble.mjs
// builds; this module maps it to CLI flags. The player's sentence goes on stdin, never on argv. The system prompt
// (31.8 KB, over the Windows command-line limit) goes in a file passed with --system-prompt-file. The process runs in
// a neutral temp folder with no setting sources, no tools and no MCP servers, so nothing but our prompt reaches the model.
// The C++ twin is FJamCliTransport (Game/Source/JamDesigner/Private/JamDesignerCli.cpp).

import { spawn } from 'node:child_process';
import { createHash } from 'node:crypto';
import { existsSync, mkdirSync, writeFileSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { delimiter, join } from 'node:path';

export function findClaude(env = process.env) {
  const exts = process.platform === 'win32' ? ['.exe', '.cmd', '.bat', ''] : [''];
  for (const dir of (env.PATH ?? env.Path ?? '').split(delimiter)) {
    if (!dir) continue;
    for (const ext of exts) {
      const p = join(dir, `claude${ext}`);
      if (existsSync(p)) return p;
    }
  }
  return undefined;
}

export const hasCli = (env = process.env) => findClaude(env) !== undefined;

export function cliArgs(model, effort, systemFile) {
  return ['-p', '--model', model, '--output-format', 'json', '--max-turns', '1', '--system-prompt-file', systemFile,
    '--tools', '', '--no-session-persistence', '--setting-sources', '', '--strict-mcp-config', '--effort', effort];
}

export function createCliModel({ exe = findClaude(), timeoutMs = 180000, workDir = join(tmpdir(), 'hokej-designer-cli') } = {}) {
  if (!exe) throw new Error('claude executable not found on PATH');
  mkdirSync(workDir, { recursive: true });

  return async function callModel(body) {
    const system = body.system.map((b) => b.text).join('\n\n');
    const systemFile = join(workDir, `system-${createHash('sha256').update(system).digest('hex').slice(0, 16)}.txt`);
    if (!existsSync(systemFile)) writeFileSync(systemFile, system);
    const user = body.messages.map((m) => m.content).join('\n\n');
    const args = cliArgs(body.model, body.output_config?.effort ?? 'low', systemFile);
    const started = performance.now();
    const shell = /\.(cmd|bat)$/i.test(exe);
    const { code, stdout, stderr, timedOut } = await new Promise((done) => {
      const child = spawn(exe, args, { cwd: workDir, shell, windowsHide: true });
      let out = '';
      let err = '';
      let killed = false;
      const timer = setTimeout(() => { killed = true; child.kill(); }, timeoutMs);
      child.stdout.on('data', (d) => { out += d; });
      child.stderr.on('data', (d) => { err += d; });
      child.on('error', (e) => { clearTimeout(timer); done({ code: -1, stdout: out, stderr: String(e.message), timedOut: false }); });
      child.on('close', (c) => { clearTimeout(timer); done({ code: c, stdout: out, stderr: err, timedOut: killed }); });
      child.stdin.end(user);
    });
    const ms = Math.round(performance.now() - started);
    if (timedOut) throw new Error(`claude CLI timed out after ${timeoutMs} ms`);
    let data;
    try { data = JSON.parse(stdout); } catch { data = undefined; }
    if (!data || data.is_error || code !== 0) {
      const status = data?.api_error_status;
      const err = new Error(`claude CLI failed (exit ${code}${status ? `, HTTP ${status}` : ''}): ${String(data?.result ?? stderr).slice(0, 200)}`);
      err.fatal = status === 400 || status === 401 || status === 403 || status === 404;
      throw err;
    }
    const u = data.usage ?? {};
    return {
      text: typeof data.result === 'string' ? data.result : '',
      stop_reason: data.stop_reason,
      usage: {
        input_tokens: (u.input_tokens ?? 0) + (u.cache_creation_input_tokens ?? 0) + (u.cache_read_input_tokens ?? 0),
        output_tokens: u.output_tokens ?? 0,
      },
      ms,
      cost_usd: data.total_cost_usd ?? 0,
    };
  };
}
