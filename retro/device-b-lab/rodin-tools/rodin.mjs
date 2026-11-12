#!/usr/bin/env node
// Hyper3D Rodin client in the style of tools/meshy/meshy.mjs: zero dependencies, Node >= 18, node:https only.
// Key: RODIN_API_KEY from the environment, else from the .env file (RODIN_ENV_FILE overrides the path; default
// C:/Users/kazda/jam/thirdjam/.env). The key is sent only in the Authorization header; it is never printed,
// logged, written to a file or put in a URL. Download links are fetched WITHOUT the Authorization header.
// Last stdout line is always one JSON object: {"tool":"thirdjam.rodin/1","cmd":...,"ok":bool,...}
// Exit 0 ok, 1 task failed or API error, 2 could not run (usage, auth, network).
// API facts: docs.hyper3d.ai, read 2026-11-12 (see README.md for URLs and what is unverified).
import fs from "node:fs";
import path from "node:path";
import https from "node:https";
import { randomBytes } from "node:crypto";

const TOOL = "thirdjam.rodin/1";
const BASE = process.env.RODIN_BASE_URL || "https://api.hyper3d.com/api/v2";
const DEFAULT_ENV = "C:/Users/kazda/jam/thirdjam/.env";

class RodinError extends Error {
  constructor(kind, message, http) { super(message); this.kind = kind; this.http = http; }
}

let KEY;
function loadKey() {
  if (KEY) return KEY;
  const k = process.env.RODIN_API_KEY;
  if (k && k.trim()) return (KEY = k.trim());
  const envFile = process.env.RODIN_ENV_FILE || DEFAULT_ENV;
  if (fs.existsSync(envFile)) {
    for (const line of fs.readFileSync(envFile, "utf8").split(/\r?\n/)) {
      const m = line.match(/^\s*(?:export\s+)?RODIN_API_KEY\s*=\s*(.*?)\s*$/);
      if (m && m[1]) return (KEY = m[1].replace(/^["']|["']$/g, "").trim());
    }
  }
  throw new RodinError("auth", "no key: set RODIN_API_KEY in the environment or in " + envFile);
}
// belt and braces: never let the key reach stdout/stderr/ledger even if a server echoed it
const scrub = (s) => (KEY ? String(s).split(KEY).join("[redacted]") : String(s));

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

// HTTP-level and body-level (HTTP 201 with {error,message}) classification. Codes: docs "Errors" page.
function classify(status, body) {
  const msg = (body && (body.message || body.error)) || JSON.stringify(body || "").slice(0, 300);
  const code = body && typeof body.error === "string" ? body.error : "";
  if (status === 401) return new RodinError("auth", `401 ${msg}`, status);
  if (status === 429) return new RodinError("quota", `429 throttled: ${msg}`, status);
  if (status === 400) return new RodinError("validation", `400 ${msg}`, status);
  if (code) {
    const kind = { API_NO_ACTIVE_SUBSCRIPTION: "auth", API_SUBSCRIPTION_PLAN_TOO_LOW: "auth", PERMISSION_DENIED: "auth",
      API_INSUFFICIENT_FUNDS: "quota", API_PARALLELISM_LIMIT_REACHED: "quota", NO_SUCH_TASK: "not_found",
      INVALID_REQUEST: "validation", API_OBJECT_NOT_FOUND_ON_IMAGE: "validation", IMAGE_CONTENT_VIOLATION: "validation",
      IMAGE_LABEL_LENGTH_TOO_LONG: "validation", ASSET_NOT_AVAILABLE: "validation" }[code] || "api";
    return new RodinError(kind, `${code}: ${body.message || ""}`.trim(), status);
  }
  return new RodinError("api", `${status} ${msg}`, status);
}

const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
const log = (...a) => console.error("[rodin]", ...a);

// one raw https round trip -> {status, headers, buf}
function rawRequest(method, url, headers, body, timeoutMs = 120000) {
  return new Promise((resolve, reject) => {
    const u = new URL(url);
    const req = https.request({ method, hostname: u.hostname, port: u.port || 443, path: u.pathname + u.search, headers }, (res) => {
      const chunks = [];
      res.on("data", (c) => chunks.push(c));
      res.on("end", () => resolve({ status: res.statusCode, headers: res.headers, buf: Buffer.concat(chunks) }));
      res.on("error", reject);
    });
    req.setTimeout(timeoutMs, () => req.destroy(new Error("timeout after " + timeoutMs / 1000 + "s")));
    req.on("error", reject);
    if (body) req.write(body);
    req.end();
  });
}

// api(): auth'd call. retryable = GET, or the read-only POSTs /status and /download. Submit is NEVER retried.
async function api(method, p, { json, form, retryable = method === "GET" } = {}, attempt = 0) {
  const key = loadKey();
  const headers = { Authorization: `Bearer ${key}`, Accept: "application/json" };
  let body;
  if (json !== undefined) { body = Buffer.from(JSON.stringify(json)); headers["Content-Type"] = "application/json"; }
  else if (form) { body = form.body; headers["Content-Type"] = form.type; }
  if (body) headers["Content-Length"] = body.length;
  let res;
  try { res = await rawRequest(method, BASE + p, headers, body); }
  catch (e) {
    if (retryable && attempt < 2) { await sleep(3000); return api(method, p, { json, form, retryable }, attempt + 1); }
    throw new RodinError("network", scrub((e && e.message) || e));
  }
  const text = res.buf.toString("utf8");
  let j; try { j = text ? JSON.parse(text) : {}; } catch { j = { raw: text.slice(0, 300) }; }
  if (res.status === 429 && retryable && attempt < 3) {
    const ra = res.headers["retry-after"];
    let secs = Number(ra);
    if (!Number.isFinite(secs)) { const d = Date.parse(ra); secs = Number.isFinite(d) ? Math.max(1, (d - Date.now()) / 1000) : 5; }
    await sleep(Math.min(secs, 60) * 1000); return api(method, p, { json, form, retryable }, attempt + 1);
  }
  if (res.status >= 500 && retryable && attempt < 2) { await sleep(3000); return api(method, p, { json, form, retryable }, attempt + 1); }
  if (res.status >= 400) throw classify(res.status, j);
  if (j && typeof j.error === "string" && j.error) throw classify(res.status, j); // body-level failure on a 2xx
  return j;
}

// multipart/form-data builder: fields [[name,value]], files [[name,filename,mime,Buffer]]
function multipart(fields, files) {
  const b = "----rodin" + randomBytes(12).toString("hex");
  const parts = [];
  for (const [n, v] of fields) parts.push(Buffer.from(`--${b}\r\nContent-Disposition: form-data; name="${n}"\r\n\r\n${v}\r\n`));
  for (const [n, fn, mime, buf] of files) {
    parts.push(Buffer.from(`--${b}\r\nContent-Disposition: form-data; name="${n}"; filename="${fn.replace(/"/g, "")}"\r\nContent-Type: ${mime}\r\n\r\n`), buf, Buffer.from("\r\n"));
  }
  parts.push(Buffer.from(`--${b}--\r\n`));
  return { body: Buffer.concat(parts), type: `multipart/form-data; boundary=${b}` };
}

// GET a download link WITHOUT credentials, following up to 4 redirects
async function fetchPublic(url, depth = 0) {
  const res = await rawRequest("GET", url, {}, null, 300000);
  if ([301, 302, 303, 307, 308].includes(res.status) && res.headers.location && depth < 4) return fetchPublic(new URL(res.headers.location, url).toString(), depth + 1);
  return res;
}

function ledger(opt, rec) {
  const f = opt.ledger || process.env.RODIN_LEDGER;
  if (f) fs.appendFileSync(f, scrub(JSON.stringify({ at: new Date().toISOString(), ...rec })) + "\n");
}
function clean(o) { for (const k of Object.keys(o)) if (o[k] === undefined) delete o[k]; return o; }
const list = (v) => String(v).split(",").map((x) => x.trim()).filter(Boolean);
const flag = (v) => (v === true || v === "true" ? "true" : v === "false" ? "false" : undefined);

const MIME = { png: "image/png", jpg: "image/jpeg", jpeg: "image/jpeg", webp: "image/webp" };
function loadImages(spec) {
  const imgs = list(spec);
  if (imgs.length < 1 || imgs.length > 5) throw new RodinError("validation", "--image takes 1 to 5 files (comma separated); the first drives the material");
  return imgs.map((f) => {
    const mime = MIME[path.extname(f).toLowerCase().slice(1)];
    if (!mime) throw new RodinError("validation", `image must be png/jpg/webp: ${f}`);
    if (!fs.existsSync(f)) throw new RodinError("validation", `no such file: ${f}`);
    return ["images", path.basename(f), mime, fs.readFileSync(f)];
  });
}

// Documented additive price (Gen-2.5 page): 0.5 base, +0.5 tier Gen-2.5-Extreme-High, +2.0 texture_mode extreme-high.
// Other tiers/options: "parameters not listed add no charge". The submit response's `consumed` is authoritative.
function estimateCredits(f) {
  const g = Object.fromEntries(f);
  return 0.5 + (g.tier === "Gen-2.5-Extreme-High" ? 0.5 : 0) + (g.texture_mode === "extreme-high" ? 2 : 0);
}

// ---------- generation ----------
function buildFields(opt, hasImages) {
  const prompt = opt.prompt === true ? undefined : opt.prompt;
  if (!hasImages && !prompt) throw new RodinError("validation", "text-to-3d needs --prompt");
  const fields = [];
  const add = (k, v) => { if (v !== undefined && v !== null && v !== "") fields.push([k, String(v)]); };
  add("prompt", prompt);
  add("tier", opt.tier || "Gen-2.5-Low");                       // Basic API access covers Gen-2.5-Low and -Extreme-Low
  add("mesh_mode", opt["mesh-mode"] || "Raw");                  // "Raw" documented; "Quad" only per third parties
  add("quality_override", opt.polycount ?? 20000);              // <= 20000 keeps Fast mode (docs)
  add("geometry_file_format", opt.format || "glb");             // glb | fbx | usdz documented; obj/stl third-party
  add("material", opt.material);                                // third-party: PBR | Shaded (Matte|Shiny in Comfy node)
  add("TAPose", flag(opt.tapose));                              // third-party: boolean, T/A pose for rigging
  add("texture_mode", opt["texture-mode"]);                     // extreme-low|low|medium|high|extreme-high
  add("uhd_texture", flag(opt.uhd));                            // true => 4K (with no addon)
  if (opt.highpack) add("addons", "HighPack");                  // wire encoding of the array is not documented; unverified
  add("seed", opt.seed);
  if (opt.extra) for (const kv of list(opt.extra)) { const i = kv.indexOf("="); if (i > 0) add(kv.slice(0, i), kv.slice(i + 1)); }
  return fields;
}

async function submit(cmd, opt, t0, withImages) {
  const files = withImages ? (opt.image ? loadImages(opt.image) : (() => { throw new RodinError("validation", "--image <file[,file...]> required (1 to 5 png/jpg/webp)"); })()) : [];
  const fields = buildFields(opt, withImages);
  const est = estimateCredits(fields);
  if (opt["dry-run"]) {
    return { cmd, ok: true, dry_run: true, request: { method: "POST", url: BASE + "/rodin", auth: "Authorization: Bearer <RODIN_API_KEY, not shown>", content_type: "multipart/form-data",
      fields: Object.fromEntries(fields), files: files.map((f) => ({ field: f[0], name: f[1], mime: f[2], bytes: f[3].length })) }, estimated_credits: est, estimate_basis: "docs.hyper3d.ai Gen-2.5 page; `consumed` in the real response is authoritative" };
  }
  const resp = await api("POST", "/rodin", { form: multipart(fields, files), retryable: false });
  if (!resp.uuid) throw new RodinError("api", "submission not accepted: no uuid in response " + JSON.stringify(resp).slice(0, 300));
  const subscription_key = resp.jobs && resp.jobs.subscription_key;
  const job_uuids = resp.jobs && resp.jobs.uuids;
  log(`submitted task ${resp.uuid} consumed=${resp.consumed}`);
  const base = { cmd, task_uuid: resp.uuid, subscription_key, job_uuids, credits: resp.consumed, format: Object.fromEntries(fields).geometry_file_format };
  if (!opt.wait) return { ...base, ok: true, status: "SUBMITTED", elapsed_ms: Date.now() - t0 };
  const final = await waitTask(subscription_key, opt);
  const out = { ...base, ok: final.state === "Done", status: final.state, elapsed_ms: Date.now() - t0 };
  if (final.state === "Done" && opt.out) Object.assign(out, await downloadTask(resp.uuid, opt.out, opt));
  if (final.state === "Failed") out.error = { kind: "task_failed", message: "a job reported Failed" };
  return out;
}

// ---------- status / wait ----------
async function getStatus(sub) {
  const r = await api("POST", "/status", { json: { subscription_key: sub }, retryable: true });
  const jobs = Array.isArray(r.jobs) ? r.jobs : [];
  const failed = jobs.some((j) => j.status === "Failed");
  const done = jobs.length > 0 && jobs.every((j) => j.status === "Done");
  const queue = jobs.map((j) => j.queue_length).filter((x) => x !== undefined);
  return { state: failed ? "Failed" : done ? "Done" : jobs.some((j) => j.status === "Generating") ? "Generating" : "Waiting", jobs, queue_length: queue.length ? Math.max(...queue) : undefined };
}

async function waitTask(sub, opt) {
  const timeoutMs = Number(opt["timeout-s"] || 1200) * 1000;
  const interval = Math.max(5, Number(opt["interval-s"] || 10)) * 1000;
  const t0 = Date.now();
  let last = "";
  await sleep(5000); // docs: wait at least 5 s before the first check
  for (;;) {
    const s = await getStatus(sub);
    const line = `${s.state}${s.queue_length !== undefined ? " queue " + s.queue_length : ""}`;
    if (line !== last) { log(`${line} t=${Math.round((Date.now() - t0) / 1000)}s`); last = line; }
    if (s.state === "Done" || s.state === "Failed") return s;
    if (Date.now() - t0 > timeoutMs) throw new RodinError("timeout", `still ${s.state} after ${timeoutMs / 1000}s (it keeps running server-side; resume with: status <task_uuid> --sub <subscription_key> --wait)`);
    await sleep(interval);
  }
}

function subFromLedger(file, uuid) {
  if (!file || !fs.existsSync(file)) return undefined;
  let found;
  for (const line of fs.readFileSync(file, "utf8").split(/\r?\n/)) {
    try { const r = JSON.parse(line); if (r.task_uuid === uuid && r.subscription_key) found = r.subscription_key; } catch { /* skip */ }
  }
  return found;
}

// ---------- download ----------
async function downloadTask(uuid, dir, opt) {
  const r = await api("POST", "/download", { json: { task_uuid: uuid }, retryable: true });
  const items = Array.isArray(r.list) ? r.list : [];
  if (!items.length) throw new RodinError("not_found", "download list is empty (task not finished, expired after the 7-day retention, or not owned by this key)");
  const only = opt.only ? list(opt.only).map((x) => x.toLowerCase()) : null;
  fs.mkdirSync(dir, { recursive: true });
  const files = {};
  const links = items.map((it) => ({ ...it }));
  for (const it of items) {
    if (!it || !it.url || !it.name) continue;
    const name = path.basename(String(it.name)).replace(/[^a-z0-9_.-]/gi, "_");
    const ext = (path.extname(name).slice(1) || "bin").toLowerCase();
    if (only && !only.includes(ext)) continue;
    const res = await fetchPublic(it.url);
    if (res.status !== 200) { log(`skip ${name}: HTTP ${res.status}`); continue; }
    fs.writeFileSync(path.join(dir, name), res.buf);
    files[name] = res.buf.length;
    log(`saved ${name} ${res.buf.length} B`);
  }
  return { dir: path.resolve(dir), files, links, raw_keys: Object.keys(r) };
}

const COMMANDS = {
  async balance(pos, opt) {
    if (opt["dry-run"]) return { cmd: "balance", ok: true, dry_run: true, request: { method: "GET", url: BASE + "/check_balance" + (opt.usage ? "?usage_page=" + opt.usage : "") } };
    const b = await api("GET", "/check_balance" + (opt.usage ? "?usage_page=" + encodeURIComponent(opt.usage) : ""));
    return clean({ cmd: "balance", ok: true, balance: b.balance, usage: b.usage });
  },

  async "text-to-3d"(pos, opt, t0) { return submit("text-to-3d", opt, t0, false); },
  async "image-to-3d"(pos, opt, t0) { return submit("image-to-3d", opt, t0, true); },

  async status(pos, opt, t0) {
    const sub = opt.sub || subFromLedger(opt.ledger || process.env.RODIN_LEDGER, pos[0]);
    if (!sub) throw new RodinError("validation", "status <task_uuid> needs the subscription key: --sub <key> (printed at submit) or --ledger <file> that holds the submit line");
    if (opt["dry-run"]) return { cmd: "status", ok: true, dry_run: true, request: { method: "POST", url: BASE + "/status", json: { subscription_key: "<subscription_key>" } } };
    const s = opt.wait ? await waitTask(sub, opt) : await getStatus(sub);
    return clean({ cmd: "status", ok: s.state !== "Failed", task_uuid: pos[0], status: s.state, queue_length: s.queue_length, jobs: s.jobs, elapsed_ms: Date.now() - t0,
      error: s.state === "Failed" ? { kind: "task_failed", message: "a job reported Failed" } : undefined });
  },

  async download(pos, opt, t0) {
    const [uuid, dir] = pos;
    if (!uuid || !dir) throw new RodinError("validation", "download <task_uuid> <dir> [--only glb,fbx]");
    if (opt["dry-run"]) return { cmd: "download", ok: true, dry_run: true, request: { method: "POST", url: BASE + "/download", json: { task_uuid: uuid } }, then: "GET each url in response.list without credentials into " + dir };
    const out = await downloadTask(uuid, dir, opt);
    return { cmd: "download", ok: true, task_uuid: uuid, ...out, elapsed_ms: Date.now() - t0 };
  },
};

const HELP = `rodin.mjs <command> [options]   (key: RODIN_API_KEY env, else RODIN_ENV_FILE or ${DEFAULT_ENV})
  balance [--usage <page>]                          free read-only credit check
  text-to-3d  --prompt "..." [gen options] [--wait [--out <dir>]]
  image-to-3d --image a.png[,b.png...] [--prompt "..."] [gen options] [--wait [--out <dir>]]
  status <task_uuid> [--sub <subscription_key>] [--wait]     (key also found via --ledger)
  download <task_uuid> <dir> [--only glb,fbx]
gen options (defaults): --tier Gen-2.5-Low  --mesh-mode Raw  --polycount 20000 (quality_override)  --format glb (glb|fbx|usdz)
  --material PBR|Shaded  --tapose true|false  --texture-mode low|medium|high|...  --uhd true  --highpack  --seed N  --extra k=v,k=v
common: --wait --timeout-s 1200 --interval-s 10 --dry-run --ledger <file.jsonl> (or RODIN_LEDGER)`;

async function main() {
  const [cmd, ...rest] = process.argv.slice(2);
  const t0 = Date.now();
  if (!cmd || !COMMANDS[cmd]) {
    console.error(HELP);
    console.log(JSON.stringify({ tool: TOOL, cmd: cmd || null, ok: false, error: { kind: "usage", message: cmd ? `unknown command ${cmd}` : "no command" } }));
    process.exitCode = cmd === "help" || cmd === "--help" ? 0 : 2; return;
  }
  const { pos, opt } = parseArgs(rest);
  try {
    const out = await COMMANDS[cmd](pos, opt, t0);
    if (out.elapsed_ms === undefined) out.elapsed_ms = Date.now() - t0;
    const line = { tool: TOOL, ...out };
    ledger(opt, line);
    console.log(scrub(JSON.stringify(line)));
    process.exitCode = out.ok ? 0 : 1;
  } catch (e) {
    const kind = e instanceof RodinError ? e.kind : "internal";
    const line = { tool: TOOL, cmd, ok: false, elapsed_ms: Date.now() - t0, error: clean({ kind, message: scrub(e.message || e), http: e.http }) };
    ledger(opt, line);
    console.log(scrub(JSON.stringify(line)));
    process.exitCode = ["auth", "network", "internal"].includes(kind) ? 2 : 1;
  }
}
main();
