// Claude Code transcripts: JSONL, one record per line. Shape learned from a real file (read-only):
//  - records carry `type` ("assistant", "user", "ai-title", ...), most carry `timestamp` (ISO, UTC),
//    `sessionId`, `isSidechain`; subagent files live in <session>/subagents/agent-*.jsonl.
//  - one assistant API response is written as SEVERAL records (one per content block) that share
//    `message.id` and repeat `message.usage`. We count one turn per message id and take the largest
//    value of each usage field (streaming partials only grow).
//  - human prompts are `type:"user"` with string content, not `isMeta`; tool results are arrays.
// Everything may be absent: a missing field never fails the build, it is just not counted.
import fs from 'node:fs';
import path from 'node:path';
import { hoursSinceH0 } from './time.mjs';

export function findJsonl(dir) {
  const out = [];
  const walk = (d) => {
    let entries;
    try {
      entries = fs.readdirSync(d, { withFileTypes: true });
    } catch {
      return;
    }
    for (const e of entries.sort((a, b) => a.name.localeCompare(b.name))) {
      const p = path.join(d, e.name);
      if (e.isDirectory()) walk(p);
      else if (e.isFile() && e.name.endsWith('.jsonl')) out.push(p);
    }
  };
  const st = fs.existsSync(dir) ? fs.statSync(dir) : null;
  if (!st) return out;
  if (st.isFile()) return [dir];
  walk(dir);
  return out;
}

const num = (v) => (typeof v === 'number' && Number.isFinite(v) ? v : 0);

/**
 * Parse one transcript file.
 * @returns {{id:string, kind:'main'|'subagent', device:string, title?:string, firstMs?:number, lastMs?:number,
 *   turns:{ms:number, usage:{input:number,output:number,cacheRead:number,cacheWrite:number}, model?:string}[],
 *   prompts:{ms:number}[], badLines:number}}
 */
export function parseTranscript(file, device, redact = (s) => s) {
  const text = fs.readFileSync(file, 'utf8');
  const byId = new Map(); // message id -> turn
  const prompts = [];
  let title;
  let firstMs;
  let lastMs;
  let badLines = 0;
  let anon = 0;
  let sidechain = false;

  for (const line of text.split(/\r?\n/)) {
    if (!line.trim()) continue;
    let o;
    try {
      o = JSON.parse(line);
    } catch {
      badLines++;
      continue;
    }
    if (!o || typeof o !== 'object') continue;
    const ms = typeof o.timestamp === 'string' ? Date.parse(o.timestamp) : NaN;
    if (!Number.isNaN(ms)) {
      if (firstMs === undefined || ms < firstMs) firstMs = ms;
      if (lastMs === undefined || ms > lastMs) lastMs = ms;
    }
    if (o.isSidechain === true) sidechain = true;
    if (o.type === 'ai-title' && typeof o.aiTitle === 'string') title = o.aiTitle;
    if (o.type === 'assistant' && o.message && typeof o.message === 'object') {
      const m = o.message;
      const key = typeof m.id === 'string' ? m.id : `anon-${anon++}`;
      const u = m.usage && typeof m.usage === 'object' ? m.usage : {};
      let t = byId.get(key);
      if (!t) {
        t = { ms: Number.isNaN(ms) ? undefined : ms, usage: { input: 0, output: 0, cacheRead: 0, cacheWrite: 0 }, model: undefined };
        byId.set(key, t);
      }
      if (t.ms === undefined && !Number.isNaN(ms)) t.ms = ms;
      t.usage.input = Math.max(t.usage.input, num(u.input_tokens));
      t.usage.output = Math.max(t.usage.output, num(u.output_tokens));
      t.usage.cacheRead = Math.max(t.usage.cacheRead, num(u.cache_read_input_tokens));
      t.usage.cacheWrite = Math.max(t.usage.cacheWrite, num(u.cache_creation_input_tokens));
      if (typeof m.model === 'string' && m.model && m.model !== '<synthetic>') t.model = m.model;
    }
    if (o.type === 'user' && !o.isMeta && o.message && typeof o.message.content === 'string' && !Number.isNaN(ms)) {
      if (!/^<(local-command|command-name|command-message)/.test(o.message.content)) prompts.push({ ms });
    }
  }

  const turns = [...byId.values()].filter((t) => t.ms !== undefined);
  const base = path.basename(file, '.jsonl');
  const kind = sidechain || /[\\/]subagents[\\/]/.test(file) ? 'subagent' : 'main';
  return {
    id: /^[0-9a-f]{8}-[0-9a-f]{4}-/.test(base) ? base.slice(0, 8) : base.replace(/^agent-/, '').slice(0, 20),
    kind,
    device,
    title: title ? redact(title) : undefined,
    firstMs,
    lastMs,
    turns,
    prompts,
    badLines,
    droppedTurns: byId.size - turns.length,
    file: path.resolve(file), // for --project-only (build.mjs); never rendered
  };
}

/** Parse every transcript under the given [{device, dir}] sources. */
export function loadTranscripts(sources, redact) {
  const sessions = [];
  const seen = new Set();
  for (const { device, dir } of sources) {
    for (const f of findJsonl(dir)) {
      const key = path.resolve(f);
      if (seen.has(key)) continue;
      seen.add(key);
      const s = parseTranscript(f, device, redact);
      if (s.turns.length || s.prompts.length) sessions.push(s);
    }
  }
  return sessions;
}

export function sessionHours(s) {
  return {
    start: s.firstMs === undefined ? undefined : hoursSinceH0(s.firstMs),
    end: s.lastMs === undefined ? undefined : hoursSinceH0(s.lastMs),
  };
}
