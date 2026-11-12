#!/usr/bin/env node
// thirdjam Hyper3D Rodin client (lane A28): zero dependencies, Node >= 18.
// Key: RODIN_API_KEY from the environment, else from <repo>/.env. Sent only in the Authorization header; never printed.
// API (docs.hyper3d.ai, seen 2026-11-12): POST /api/v2/rodin (multipart), POST /api/v2/status {subscription_key},
// POST /api/v2/download {task_uuid}. Last stdout line is one JSON object {"tool":"thirdjam.rodin/1", ...}.
// Exit 0 ok, 1 failed task or API error, 2 could not run.
import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";

const TOOL = "thirdjam.rodin/1";
const BASE = process.env.RODIN_BASE_URL || "https://api.hyper3d.com/api/v2";
const HERE = path.dirname(fileURLToPath(import.meta.url));
const REPO = path.resolve(HERE, "..", "..");
const log = (...a) => console.error("[rodin]", ...a);
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));

class RodinError extends Error { constructor(kind, message, http) { super(message); this.kind = kind; this.http = http; } }

let KEY;
function loadKey() {
  if (KEY) return KEY;
  if (process.env.RODIN_API_KEY) return (KEY = process.env.RODIN_API_KEY.trim());
  const envFile = process.env.RODIN_ENV_FILE || path.join(REPO, ".env");
  if (fs.existsSync(envFile)) {
    for (const line of fs.readFileSync(envFile, "utf8").split(/\r?\n/)) {
      const m = line.match(/^\s*(?:export\s+)?RODIN_API_KEY\s*=\s*(.*?)\s*$/);
      if (m && m[1]) return (KEY = m[1].replace(/^["']|["']$/g, "").trim());
    }
  }
  throw new RodinError("auth", "no key: set RODIN_API_KEY in the environment or in " + envFile);
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

function classify(status, body) {
  const msg = (body && (body.message || body.error || body.detail)) || JSON.stringify(body || "").slice(0, 300);
  if (status === 401) return new RodinError("auth", `401 ${msg}`, status);
  if (status === 402) return new RodinError("quota", `402 ${msg}`, status);
  if (status === 429) return new RodinError("quota", `429 ${msg}`, status);
  if (status === 404) return new RodinError("not_found", `404 ${msg}`, status);
  if (status === 400 || status === 422) return new RodinError("validation", `${status} ${msg}`, status);
  return new RodinError("api", `${status} ${msg}`, status);
}

async function api(p, body, { multipart = false, attempt = 0 } = {}) {
  const key = loadKey();
  let res;
  try {
    res = await fetch(BASE + p, {
      method: "POST",
      headers: { Authorization: `Bearer ${key}`, ...(multipart ? {} : { "Content-Type": "application/json" }) },
      body: multipart ? body : JSON.stringify(body),
      signal: AbortSignal.timeout(120000),
    });
  } catch (e) {
    // status/download are idempotent reads: retry twice; a generation POST is never re-sent (it would spend credits twice)
    if (!multipart && attempt < 2) { await sleep(3000); return api(p, body, { multipart, attempt: attempt + 1 }); }
    throw new RodinError("network", String((e && e.message) || e));
  }
  const text = await res.text();
  let json; try { json = text ? JSON.parse(text) : {}; } catch { json = { raw: text.slice(0, 300) }; }
  if (!res.ok) throw classify(res.status, json);
  if (json.error) throw new RodinError("api", `error: ${JSON.stringify(json.error).slice(0, 300)}`, res.status);
  return json;
}

function ledger(opt, rec) {
  const f = opt.ledger || process.env.RODIN_LEDGER;
  if (f) fs.appendFileSync(f, JSON.stringify({ at: new Date().toISOString(), ...rec }) + "\n");
}
function clean(o) { for (const k of Object.keys(o)) if (o[k] === undefined) delete o[k]; return o; }

async function waitJobs(subKey, opt) {
  const timeoutMs = Number(opt["timeout-s"] || 900) * 1000;
  const interval = Math.max(3, Number(opt["interval-s"] || 5)) * 1000;
  const t0 = Date.now();
  let last = "";
  for (;;) {
    const s = await api("/status", { subscription_key: subKey });
    const jobs = s.jobs || [];
    const line = jobs.map((j) => j.status).join(",");
    if (line !== last) { log(`status ${line} t=${Math.round((Date.now() - t0) / 1000)}s`); last = line; }
    if (jobs.length && jobs.every((j) => ["Done", "Failed", "Canceled", "Cancelled"].includes(j.status))) return jobs;
    if (Date.now() - t0 > timeoutMs) throw new RodinError("timeout", `jobs still ${line} after ${timeoutMs / 1000}s`);
    await sleep(interval);
  }
}

const COMMANDS = {
  // generate --prompt "..." [--tier Gen-2.5-Low] [--format glb|fbx] [--material PBR|Shaded|All] [--quality medium|high|low|extra-low]
  //          [--quality-override 20000] [--mesh-mode Raw|Quad] [--tapose] [--texture-mode low] [--image file.png]* [--wait]
  async generate(pos, opt, t0) {
    if (!opt.prompt && !opt.image) throw new RodinError("validation", "--prompt (or --image) required");
    const form = new FormData();
    const fields = clean({
      prompt: opt.prompt, tier: opt.tier || "Gen-2.5-Low", geometry_file_format: opt.format || "glb",
      material: opt.material, quality: opt.quality, quality_override: opt["quality-override"],
      mesh_mode: opt["mesh-mode"], texture_mode: opt["texture-mode"], seed: opt.seed,
      TAPose: opt.tapose ? "true" : undefined, use_hyper: opt.hyper ? "true" : undefined,
    });
    for (const [k, v] of Object.entries(fields)) form.append(k, String(v));
    for (const img of [].concat(opt.image || [])) {
      if (img === true) continue;
      form.append("images", new Blob([fs.readFileSync(img)]), path.basename(img));
    }
    if (opt["dry-run"]) return { cmd: "generate", ok: true, dry_run: true, fields };
    const r = await api("/rodin", form, { multipart: true });
    const uuid = r.uuid, subKey = r.jobs && r.jobs.subscription_key;
    log(`created task ${uuid} consumed=${r.consumed}`);
    const out = { cmd: "generate", ok: true, task_uuid: uuid, subscription_key: subKey, consumed: r.consumed, fields, status: "SUBMITTED" };
    if (!opt.wait) return out;
    const jobs = await waitJobs(subKey, opt);
    out.status = jobs.every((j) => j.status === "Done") ? "SUCCEEDED" : "FAILED";
    out.ok = out.status === "SUCCEEDED";
    out.jobs = jobs;
    out.elapsed_ms = Date.now() - t0;
    if (out.ok) {
      const d = await api("/download", { task_uuid: uuid });
      out.files = (d.list || []).map((f) => ({ name: f.name, url: f.url }));
    }
    return out;
  },
  async status(pos, opt, t0) {
    if (!pos[0]) throw new RodinError("validation", "status <subscription_key> [--wait]");
    const jobs = opt.wait ? await waitJobs(pos[0], opt) : (await api("/status", { subscription_key: pos[0] })).jobs;
    return { cmd: "status", ok: true, jobs };
  },
  // download <task_uuid> <dir> [--only glb,fbx,png]
  async download(pos, opt, t0) {
    const [uuid, dir] = pos;
    if (!uuid || !dir) throw new RodinError("validation", "download <task_uuid> <dir> [--only glb]");
    const d = await api("/download", { task_uuid: uuid });
    fs.mkdirSync(dir, { recursive: true });
    fs.writeFileSync(path.join(dir, `${uuid}.download.json`), JSON.stringify(d, null, 2));
    const only = opt.only ? String(opt.only).split(",") : null;
    const files = {};
    for (const f of d.list || []) {
      const name = f.name || path.basename(new URL(f.url).pathname);
      const ext = (name.match(/\.([a-z0-9]+)$/i) || [])[1]?.toLowerCase() || "bin";
      if (only && !only.includes(ext)) continue;
      const res = await fetch(f.url, { signal: AbortSignal.timeout(300000) });
      if (!res.ok) { log(`skip ${name}: HTTP ${res.status}`); continue; }
      const buf = Buffer.from(await res.arrayBuffer());
      fs.writeFileSync(path.join(dir, name), buf);
      files[name] = buf.length;
      log(`saved ${name} ${buf.length} B`);
    }
    return { cmd: "download", ok: true, task_uuid: uuid, dir: path.resolve(dir), files, links: (d.list || []).map((f) => ({ name: f.name, url: f.url })) };
  },
};

const HELP = `rodin.mjs <command> [options]   (key: RODIN_API_KEY env or <repo>/.env)
  generate --prompt "..." [--tier Gen-2.5-Low] [--format glb|fbx] [--material PBR|Shaded|All] [--quality medium] [--quality-override 20000]
           [--mesh-mode Raw|Quad] [--tapose] [--texture-mode low] [--image f.png] [--wait] [--dry-run]
  status <subscription_key> [--wait]
  download <task_uuid> <dir> [--only glb,fbx]
common: --wait --timeout-s 900 --interval-s 5 --ledger <file.jsonl> (or RODIN_LEDGER)`;

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
    const kind = e instanceof RodinError ? e.kind : "internal";
    const line = { tool: TOOL, cmd, ok: false, elapsed_ms: Date.now() - t0, error: clean({ kind, message: String(e.message || e), http: e.http }) };
    ledger(opt, line);
    console.log(JSON.stringify(line));
    process.exitCode = ["auth", "network", "internal"].includes(kind) ? 2 : 1;
  }
}
main();
