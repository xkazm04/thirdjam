#!/usr/bin/env node
// thirdjam fal.ai queue client (lane A28), used for Tripo3D (tripo3d/tripo/v2.5/text-to-3d). Zero dependencies, Node >= 18.
// Key: FAL_API_KEY or FAL_KEY from the environment, else from <repo>/.env. Sent only in the Authorization header; never printed.
// Queue API (fal.ai/docs/model-apis/queue, seen 2026-11-12): POST https://queue.fal.run/<model_id>; status/result at the URLs the
// submit answer returns (owner/app only, never the full endpoint path). Last stdout line: {"tool":"thirdjam.fal/1", ...}.
import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";

const TOOL = "thirdjam.fal/1";
const QUEUE = "https://queue.fal.run";
const HERE = path.dirname(fileURLToPath(import.meta.url));
const REPO = path.resolve(HERE, "..", "..");
const log = (...a) => console.error("[fal]", ...a);
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
class FalError extends Error { constructor(kind, message, http) { super(message); this.kind = kind; this.http = http; } }

let KEY;
function loadKey() {
  if (KEY) return KEY;
  const k = process.env.FAL_API_KEY || process.env.FAL_KEY;
  if (k) return (KEY = k.trim());
  const envFile = process.env.FAL_ENV_FILE || path.join(REPO, ".env");
  if (fs.existsSync(envFile)) {
    for (const line of fs.readFileSync(envFile, "utf8").split(/\r?\n/)) {
      const m = line.match(/^\s*(?:export\s+)?(FAL_API_KEY|FAL_KEY)\s*=\s*(.*?)\s*$/);
      if (m && m[2]) return (KEY = m[2].replace(/^["']|["']$/g, "").trim());
    }
  }
  throw new FalError("auth", "no key: set FAL_API_KEY in the environment or in " + envFile);
}

function parseArgs(argv) {
  const pos = [], opt = {};
  for (let i = 0; i < argv.length; i++) {
    const a = argv[i];
    if (a.startsWith("--")) {
      const eq = a.indexOf("=");
      if (eq > 0) opt[a.slice(2, eq)] = a.slice(eq + 1);
      else if (i + 1 < argv.length && !argv[i + 1].startsWith("--")) opt[a.slice(2)] = argv[++i];
      else opt[a.slice(2)] = true;
    } else pos.push(a);
  }
  return { pos, opt };
}
function clean(o) { for (const k of Object.keys(o)) if (o[k] === undefined) delete o[k]; return o; }
function ledger(opt, rec) {
  const f = opt.ledger || process.env.FAL_LEDGER;
  if (f) fs.appendFileSync(f, JSON.stringify({ at: new Date().toISOString(), ...rec }) + "\n");
}

async function call(method, url, body, attempt = 0) {
  const key = loadKey();
  let res;
  try {
    res = await fetch(url, { method, headers: { Authorization: `Key ${key}`, ...(body ? { "Content-Type": "application/json" } : {}) },
      body: body ? JSON.stringify(body) : undefined, signal: AbortSignal.timeout(120000) });
  } catch (e) {
    if (method === "GET" && attempt < 2) { await sleep(3000); return call(method, url, body, attempt + 1); }
    throw new FalError("network", String((e && e.message) || e));
  }
  const text = await res.text();
  let json; try { json = text ? JSON.parse(text) : {}; } catch { json = { raw: text.slice(0, 300) }; }
  if (!res.ok) {
    const msg = JSON.stringify(json.detail || json.error || json).slice(0, 400);
    const kind = res.status === 401 || res.status === 403 ? "auth" : res.status === 402 || res.status === 429 ? "quota"
      : res.status === 404 ? "not_found" : res.status === 400 || res.status === 422 ? "validation" : "api";
    throw new FalError(kind, `${res.status} ${msg}`, res.status);
  }
  return json;
}

async function waitRequest(statusUrl, responseUrl, opt) {
  const timeoutMs = Number(opt["timeout-s"] || 900) * 1000;
  const interval = Math.max(2, Number(opt["interval-s"] || 5)) * 1000;
  const t0 = Date.now();
  let last = "";
  for (;;) {
    const s = await call("GET", statusUrl + "?logs=0");
    const line = `${s.status}${s.queue_position !== undefined ? " queue " + s.queue_position : ""}`;
    if (line !== last) { log(`${line} t=${Math.round((Date.now() - t0) / 1000)}s`); last = line; }
    if (s.status === "COMPLETED") return { status: s, result: await call("GET", responseUrl), metrics: s.metrics };
    if (s.status === "FAILED" || s.error) return { status: s, result: null };
    if (Date.now() - t0 > timeoutMs) throw new FalError("timeout", `request still ${s.status} after ${timeoutMs / 1000}s (resume: status ${statusUrl})`);
    await sleep(interval);
  }
}

function urlsOf(result) {
  const out = {};
  for (const [k, v] of Object.entries(result || {})) {
    if (v && typeof v === "object" && typeof v.url === "string") out[k] = v.url;
    else if (Array.isArray(v)) v.forEach((f, i) => { if (f && typeof f.url === "string") out[`${k}${i}`] = f.url; });
  }
  return out;
}

const COMMANDS = {
  // submit --model tripo3d/tripo/v2.5/text-to-3d --input '{"prompt":"..."}' | --input @file.json [--wait]
  async submit(pos, opt, t0) {
    const model = opt.model || "tripo3d/tripo/v2.5/text-to-3d";
    if (!opt.input) throw new FalError("validation", "--input '<json>' or --input @file.json required");
    const raw = String(opt.input).startsWith("@") ? fs.readFileSync(String(opt.input).slice(1), "utf8") : String(opt.input);
    const input = JSON.parse(raw);
    if (opt["dry-run"]) return { cmd: "submit", ok: true, dry_run: true, model, input };
    const r = await call("POST", `${QUEUE}/${model}`, input);
    log(`submitted ${model} request ${r.request_id}`);
    const out = { cmd: "submit", ok: true, model, request_id: r.request_id, status_url: r.status_url, response_url: r.response_url, input, status: "SUBMITTED" };
    if (!opt.wait) return out;
    const w = await waitRequest(r.status_url, r.response_url, opt);
    out.status = w.status.status === "COMPLETED" ? "SUCCEEDED" : "FAILED";
    out.ok = out.status === "SUCCEEDED";
    out.inference_s = w.metrics && w.metrics.inference_time;
    out.error = w.status.error ? { kind: "task_failed", message: String(w.status.error).slice(0, 300) } : undefined;
    out.result = w.result ? { task_id: w.result.task_id } : undefined;
    out.urls = urlsOf(w.result);
    return clean(out);
  },
  // status <status_url> [--wait]  (the full status_url the submit answer returned)
  async status(pos, opt) {
    if (!pos[0]) throw new FalError("validation", "status <status_url> [--wait]");
    const s = await call("GET", pos[0] + "?logs=0");
    if (!opt.wait) return { cmd: "status", ok: true, status: s.status, response_url: s.response_url };
    const w = await waitRequest(pos[0], s.response_url, opt);
    return clean({ cmd: "status", ok: w.status.status === "COMPLETED", status: w.status.status, urls: urlsOf(w.result) });
  },
  // download <response_url> <dir> [--only glb,fbx,webp]
  async download(pos, opt) {
    const [responseUrl, dir] = pos;
    if (!responseUrl || !dir) throw new FalError("validation", "download <response_url> <dir> [--only glb]");
    const result = await call("GET", responseUrl);
    fs.mkdirSync(dir, { recursive: true });
    fs.writeFileSync(path.join(dir, "result.json"), JSON.stringify(result, null, 2));
    const only = opt.only ? String(opt.only).split(",") : null;
    const files = {};
    for (const [k, u] of Object.entries(urlsOf(result))) {
      const ext = ((new URL(u).pathname.match(/\.([a-z0-9]+)$/i) || [])[1] || "bin").toLowerCase();
      if (only && !only.includes(ext)) continue;
      const name = `${k}.${ext}`;
      const res = await fetch(u, { signal: AbortSignal.timeout(300000) });
      if (!res.ok) { log(`skip ${k}: HTTP ${res.status}`); continue; }
      const buf = Buffer.from(await res.arrayBuffer());
      fs.writeFileSync(path.join(dir, name), buf);
      files[name] = buf.length;
      log(`saved ${name} ${buf.length} B`);
    }
    return { cmd: "download", ok: true, dir: path.resolve(dir), files, task_id: result.task_id };
  },
};

const HELP = `fal.mjs <command> [options]   (key: FAL_API_KEY env or <repo>/.env)
  submit [--model tripo3d/tripo/v2.5/text-to-3d] --input '{"prompt":"..."}' | --input @job.json [--wait] [--dry-run]
  status <status_url> [--wait]
  download <response_url> <dir> [--only glb,fbx]
common: --wait --timeout-s 900 --interval-s 5 --ledger <file.jsonl> (or FAL_LEDGER)`;

async function main() {
  const [cmd, ...rest] = process.argv.slice(2);
  const t0 = Date.now();
  if (!cmd || !COMMANDS[cmd]) {
    console.error(HELP);
    console.log(JSON.stringify({ tool: TOOL, cmd: cmd || null, ok: false, error: { kind: "usage", message: cmd ? `unknown command ${cmd}` : "no command" } }));
    process.exitCode = 2; return;
  }
  const { pos, opt } = parseArgs(rest);
  try {
    const out = await COMMANDS[cmd](pos, opt, t0);
    if (out.elapsed_ms === undefined) out.elapsed_ms = Date.now() - t0;
    const line = { tool: TOOL, ...out };
    ledger(opt, line);
    console.log(JSON.stringify(line));
    process.exitCode = out.ok ? 0 : 1;
  } catch (e) {
    const kind = e instanceof FalError ? e.kind : "internal";
    const line = { tool: TOOL, cmd, ok: false, elapsed_ms: Date.now() - t0, error: clean({ kind, message: String(e.message || e), http: e.http }) };
    ledger(opt, line);
    console.log(JSON.stringify(line));
    process.exitCode = ["auth", "network", "internal"].includes(kind) ? 2 : 1;
  }
}
main();
