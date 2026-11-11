#!/usr/bin/env node
// Minimal client for the Unreal editor's embedded MCP server (Streamable HTTP, JSON-RPC 2.0).
// No dependencies (node:http only). Every request has an explicit timeout.
//
//   node tools/mcp/mcp.mjs wait      [--timeout 300]             poll until the server answers initialize
//   node tools/mcp/mcp.mjs list                                    tools/list (names only)
//   node tools/mcp/mcp.mjs toolsets                                list_toolsets
//   node tools/mcp/mcp.mjs describe  <Toolset> [<Toolset> ...] [--schemas]
//   node tools/mcp/mcp.mjs tools     <Toolset.tool> [...]          describe_tools (input schemas)
//   node tools/mcp/mcp.mjs call      <Toolset.tool> ['<json args>' | @args.json] [--timeout 120] [--save-images dir]
//                                   (use @file from PowerShell 5.1: it strips the double quotes inside native arguments)
//   node tools/mcp/mcp.mjs raw       <method> ['<json params>']
//   node tools/mcp/mcp.mjs inventory --out <file.md> [--json <file.json>]
//
// Options: --url (default $UE_MCP_URL or http://127.0.0.1:8000/mcp), --timeout seconds (default 120), --quiet.
// Output: the result as JSON on stdout. Last line is a status line:
//   {"tool":"thirdjam.mcp/1","verdict":"pass"|"fail"|"timeout"|"unreachable","ms":...,"error":...}
// Exit: 0 pass, 1 tool or protocol error (server answered), 2 unreachable, 3 timeout (request sent, no full answer).
// A timeout is never reported as a pass: a stream cut before the JSON-RPC response arrives is "timeout".

import { writeFileSync, mkdirSync, readFileSync } from 'node:fs';
import { join } from 'node:path';
import http from 'node:http';

const argv = process.argv.slice(2);
function opt(name, def) {
  const i = argv.indexOf('--' + name);
  if (i < 0) return def;
  const v = argv[i + 1];
  argv.splice(i, 2);
  return v;
}
function flag(name) {
  const i = argv.indexOf('--' + name);
  if (i < 0) return false;
  argv.splice(i, 1);
  return true;
}
const URL_ = opt('url', process.env.UE_MCP_URL || 'http://127.0.0.1:8000/mcp');
const TIMEOUT = Number(opt('timeout', '120'));
const OUT = opt('out', null);
const JSON_OUT = opt('json', null);
const SAVE_IMAGES = opt('save-images', null);
const SCHEMAS = flag('schemas');
const QUIET = flag('quiet');
const [cmd, ...rest] = argv;

class McpError extends Error { constructor(kind, msg) { super(msg); this.kind = kind; } }

let sessionId = null;
let nextId = 1;
const PROTOCOLS = ['2026-07-28', '2025-11-25', '2025-06-18', '2025-03-26'];

function parseSseFor(buf, id) {
  // Returns { msg, rest } when the JSON-RPC response with our id is in buf.
  let idx;
  while ((idx = buf.search(/\r?\n\r?\n/)) >= 0) {
    const event = buf.slice(0, idx);
    buf = buf.slice(idx).replace(/^\r?\n\r?\n/, '');
    const data = event.split(/\r?\n/).filter(l => l.startsWith('data:')).map(l => l.slice(5).replace(/^ /, '')).join('\n');
    if (!data) continue;
    let msg;
    try { msg = JSON.parse(data); } catch { continue; }
    if (msg.id === id) return { msg, rest: buf };
  }
  return { msg: null, rest: buf };
}

// One POST over node:http with no keep-alive, so the process can exit cleanly when done
// (fetch's pooled sockets trip a libuv assertion on process.exit under Node 24 on Windows).
function post(bodyText, headers, timeoutSec, id) {
  return new Promise((resolve, reject) => {
    const u = new URL(URL_);
    let settled = false;
    const done = (fn, v) => { if (!settled) { settled = true; clearTimeout(timer); try { req.destroy(); } catch {} fn(v); } };
    const req = http.request({ hostname: u.hostname, port: u.port, path: u.pathname + u.search, method: 'POST', agent: false,
      headers: { ...headers, 'content-length': Buffer.byteLength(bodyText), connection: 'close' } }, res => {
      const sid = res.headers['mcp-session-id'];
      if (sid) sessionId = sid;
      const ct = res.headers['content-type'] || '';
      res.setEncoding('utf8');
      let buf = '';
      if (id === undefined) { res.resume(); res.on('end', () => done(resolve, null)); return; }
      res.on('data', c => {
        buf += c;
        if (res.statusCode < 300 && ct.includes('text/event-stream')) {
          const r = parseSseFor(buf, id); buf = r.rest;
          if (r.msg) done(resolve, r.msg);
        }
      });
      res.on('end', () => {
        if (res.statusCode >= 300) return done(reject, new McpError('error', `HTTP ${res.statusCode}: ${buf.slice(0, 500)}`));
        if (ct.includes('text/event-stream')) return done(reject, new McpError('timeout', 'SSE stream closed before the response arrived (truncated)'));
        try { done(resolve, JSON.parse(buf)); } catch { done(reject, new McpError('timeout', `truncated or non-JSON body (${buf.length} bytes)`)); }
      });
      res.on('error', e => done(reject, new McpError('timeout', `response cut: ${e.message} (truncated)`)));
    });
    const timer = setTimeout(() => done(reject, new McpError('timeout', `no complete response in ${timeoutSec}s`)), timeoutSec * 1000);
    req.on('error', e => done(reject, new McpError(e.code === 'ECONNREFUSED' || e.code === 'ECONNRESET' && !req.reusedSocket ? 'unreachable' : 'timeout', `${URL_}: ${e.code || e.message}`)));
    req.end(bodyText);
  });
}

async function rpc(method, params, { notify = false, timeoutSec = TIMEOUT } = {}) {
  const id = notify ? undefined : nextId++;
  const body = { jsonrpc: '2.0', method };
  if (!notify) body.id = id;
  if (params !== undefined) body.params = params;
  const headers = { 'content-type': 'application/json', accept: 'application/json, text/event-stream' };
  if (sessionId) headers['mcp-session-id'] = sessionId;
  const msg = await post(JSON.stringify(body), headers, timeoutSec, id);
  if (notify) return null;
  if (msg.error) throw new McpError('error', `JSON-RPC ${msg.error.code}: ${msg.error.message}`);
  return msg.result;
}

async function connect(timeoutSec = 30) {
  let lastErr;
  for (const v of PROTOCOLS) {
    try {
      const r = await rpc('initialize', { protocolVersion: v, capabilities: {}, clientInfo: { name: 'thirdjam-smoke', version: '1.0' } }, { timeoutSec });
      await rpc('notifications/initialized', undefined, { notify: true, timeoutSec });
      return r;
    } catch (e) { lastErr = e; if (e.kind !== 'error') throw e; }
  }
  throw lastErr;
}

function textOf(result) {
  return (result?.content || []).filter(c => c.type === 'text').map(c => c.text).join('\n');
}
function parseText(result) {
  if (result?.structuredContent) return result.structuredContent;
  const t = textOf(result);
  try { return JSON.parse(t); } catch { return t; }
}
function toolCallArgs(full, args) {
  // "Toolset.tool" -> call_tool(toolset_name, tool_name); a name with no dot is a top-level tool.
  const dot = full.lastIndexOf('.');
  const a = { tool_name: dot < 0 ? full : full.slice(dot + 1), arguments: args || {} };
  if (dot >= 0) a.toolset_name = full.slice(0, dot);
  return a;
}
async function callTool(full, args, timeoutSec = TIMEOUT) {
  const list = await rpc('tools/list', {});
  const native = (list.tools || []).some(t => t.name === full);
  if (native) return rpc('tools/call', { name: full, arguments: args || {} }, { timeoutSec });
  return rpc('tools/call', { name: 'call_tool', arguments: toolCallArgs(full, args) }, { timeoutSec });
}
function saveImages(result, dir) {
  const saved = [];
  if (!dir) return saved;
  mkdirSync(dir, { recursive: true });
  (result?.content || []).forEach((c, i) => {
    if (c.type === 'image' && c.data) {
      const ext = (c.mimeType || 'image/png').split('/')[1].replace('jpeg', 'jpg');
      const f = join(dir, `mcp-image-${Date.now()}-${i}.${ext}`);
      writeFileSync(f, Buffer.from(c.data, 'base64'));
      saved.push(f);
    }
  });
  return saved;
}
function access(t) {
  const a = t.annotations || {};
  if (a.accessMode) return a.accessMode; // Unreal's own annotation: read | write | execute
  if (a.readOnlyHint === true) return 'read';
  if (a.destructiveHint === true) return 'execute';
  if (a.readOnlyHint === false || a.destructiveHint === false) return 'write';
  return t.access || 'unknown';
}

async function inventory() {
  const init = await connect();
  const list = await rpc('tools/list', {});
  const top = list.tools || [];
  const ts = parseText(await rpc('tools/call', { name: 'list_toolsets', arguments: {} }));
  const toolsets = typeof ts === 'string' ? [...ts.matchAll(/^- ([^:\s]+): ?(.*)$/gm)].map(m => ({ name: m[1], description: m[2] })) : (Array.isArray(ts) ? ts : ts.toolsets || []).map(t => (typeof t === 'string' ? { name: t } : t));
  const names = toolsets.map(t => t.name);
  const described = {};
  for (let i = 0; i < names.length; i += 8) {
    const r = parseText(await rpc('tools/call', { name: 'describe_toolsets', arguments: { toolset_names: names.slice(i, i + 8) } }));
    const arr = Array.isArray(r) ? r : (r.toolsets && !Array.isArray(r.toolsets) ? Object.values(r.toolsets) : r.toolsets || Object.values(r));
    for (const d of arr) if (d && d.name) described[d.name] = d;
  }
  return { init, top, toolsets, described };
}

function oneLine(s) { return String(s || '').replace(/\s+/g, ' ').trim().split(/(?<=\.)\s/)[0].slice(0, 160).replace(/\|/g, '/'); }

function inventoryMarkdown(inv) {
  const L = [];
  const total = Object.values(inv.described).reduce((n, d) => n + (d.tools || []).length, 0);
  L.push('# Unreal MCP tool inventory', '');
  L.push(`Generated by \`node tools/mcp/mcp.mjs inventory\` against UE ${inv.init.serverInfo?.version || '5.9'} ` +
    `(server \`${inv.init.serverInfo?.name || '?'}\`, protocol ${inv.init.protocolVersion}) on ${new Date().toISOString().slice(0, 10)}, ` +
    'from a blank C++ project with the plugins `ModelContextProtocol`, `ToolsetRegistry`, `AllToolsets` enabled.', '');
  L.push(`Tool search is on (default), so \`tools/list\` returns ${inv.top.length} tools: ${inv.top.map(t => '`' + t.name + '`').join(', ')}. ` +
    'Every toolset tool below is reached through `call_tool` with `toolset_name` and `tool_name`.', '');
  L.push(`Totals: ${Object.keys(inv.described).length} toolsets, ${total} tools. Access is Unreal's own \`annotations.accessMode\` (read, write, execute); unknown = no annotation.`, '');
  L.push('| Toolset | Tools | read | write | execute | unknown |', '|---|---:|---:|---:|---:|---:|');
  for (const name of Object.keys(inv.described).sort()) {
    const tools = inv.described[name].tools || [];
    const c = { read: 0, write: 0, execute: 0, unknown: 0 };
    tools.forEach(t => { const k = access(t); c[k in c ? k : 'unknown']++; });
    L.push(`| ${name} | ${tools.length} | ${c.read} | ${c.write} | ${c.execute} | ${c.unknown} |`);
  }
  L.push('');
  for (const name of Object.keys(inv.described).sort()) {
    const d = inv.described[name];
    L.push(`## ${name}`, '');
    if (d.description) L.push(oneLine(d.description), '');
    L.push('| Tool | Access | What it does |', '|---|---|---|');
    for (const t of (d.tools || []).slice().sort((a, b) => a.name.localeCompare(b.name))) L.push(`| \`${t.name}\` | ${access(t)} | ${oneLine(t.description)} |`);
    L.push('');
  }
  return L.join('\n');
}

const t0 = Date.now();
function status(verdict, extra = {}) {
  console.log(JSON.stringify({ tool: 'thirdjam.mcp/1', cmd, verdict, ms: Date.now() - t0, ...extra }));
}
function print(o) { if (!QUIET) console.log(typeof o === 'string' ? o : JSON.stringify(o, null, 2)); }

try {
  if (cmd === 'wait') {
    const deadline = Date.now() + TIMEOUT * 1000;
    let last = '';
    for (;;) {
      try { const r = await connect(10); print(r); status('pass', { server: r.serverInfo, protocol: r.protocolVersion }); process.exit(0); }
      catch (e) { last = e.message; if (e.kind === 'error') throw e; }
      if (Date.now() > deadline) { status('timeout', { error: `server not ready in ${TIMEOUT}s: ${last}` }); process.exit(3); }
      await new Promise(r => setTimeout(r, 2000));
    }
  }
  if (cmd === 'inventory') {
    const inv = await inventory();
    if (OUT) writeFileSync(OUT, inventoryMarkdown(inv));
    if (JSON_OUT) writeFileSync(JSON_OUT, JSON.stringify({ top: inv.top, toolsets: inv.toolsets, described: inv.described }, null, 2));
    const counts = Object.fromEntries(Object.entries(inv.described).map(([k, v]) => [k, (v.tools || []).length]));
    print(counts);
    status('pass', { toolsets: Object.keys(counts).length, tools: Object.values(counts).reduce((a, b) => a + b, 0) });
    process.exit(0);
  }
  await connect();
  let result;
  if (cmd === 'list') { result = await rpc('tools/list', {}); print((result.tools || []).map(t => `${t.name}: ${oneLine(t.description)}`).join('\n')); }
  else if (cmd === 'toolsets') { result = await rpc('tools/call', { name: 'list_toolsets', arguments: {} }); print(parseText(result)); }
  else if (cmd === 'describe') { result = await rpc('tools/call', { name: 'describe_toolsets', arguments: { toolset_names: rest, include_schemas: SCHEMAS } }); print(parseText(result)); }
  else if (cmd === 'tools') { result = await rpc('tools/call', { name: 'describe_tools', arguments: { tool_names: rest } }); print(parseText(result)); }
  else if (cmd === 'call') {
    const args = rest[1] ? JSON.parse(rest[1].startsWith('@') ? readFileSync(rest[1].slice(1), 'utf8').replace(/^﻿/, '') : rest[1]) : {};
    result = await callTool(rest[0], args);
    const imgs = saveImages(result, SAVE_IMAGES);
    const shown = { ...result, content: (result.content || []).map(c => (c.data ? { ...c, data: `<${c.data.length} base64 chars>` } : c)) };
    print(shown);
    if (imgs.length) print({ saved_images: imgs });
  }
  else if (cmd === 'raw') { result = await rpc(rest[0], rest[1] ? JSON.parse(rest[1]) : {}); print(result); }
  else { console.error('usage: see the header of tools/mcp/mcp.mjs'); status('fail', { error: 'unknown command' }); process.exit(2); }
  if (result && result.isError) { status('fail', { error: oneLine(textOf(result)) || 'isError' }); process.exit(1); }
  status('pass');
  process.exit(0);
} catch (e) {
  const kind = e.kind || 'error';
  const verdict = kind === 'unreachable' ? 'unreachable' : kind === 'timeout' ? 'timeout' : 'fail';
  status(verdict, { error: e.message });
  process.exit(kind === 'unreachable' ? 2 : kind === 'timeout' ? 3 : 1);
}
