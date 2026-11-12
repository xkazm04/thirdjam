#!/usr/bin/env node
// thirdjam Meshy client: zero dependencies, Node >= 18 (global fetch).
// Key: MESHI_API_KEY (operator spelling) or MESHY_API_KEY from the environment, else from <repo>/.env.
// The key is never printed, logged or put in a URL. Last stdout line is always one JSON object:
//   {"tool":"thirdjam.meshy/1","cmd":...,"ok":bool,"task_id":...,"status":...,"elapsed_ms":...,"credits":...,"urls":{...}}
// Exit 0 ok, 1 task failed or API error, 2 could not run (usage, auth, network).
import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";

const TOOL = "thirdjam.meshy/1";
const BASE = process.env.MESHY_BASE_URL || "https://api.meshy.ai";
const HERE = path.dirname(fileURLToPath(import.meta.url));
const REPO = path.resolve(HERE, "..", "..");

// kind -> collection path (docs.meshy.ai/en/api, seen 2026-11-12)
const KINDS = {
  "text-to-3d": "/openapi/v2/text-to-3d",
  "image-to-3d": "/openapi/v1/image-to-3d",
  "rigging": "/openapi/v1/rigging",
  "animations": "/openapi/v1/animations",
  "text-to-motion": "/openapi/v1/text-to-motion",
};

class MeshyError extends Error {
  constructor(kind, message, http) { super(message); this.kind = kind; this.http = http; }
}

let KEY;
function loadKey() {
  if (KEY) return KEY;
  const k = process.env.MESHI_API_KEY || process.env.MESHY_API_KEY;
  if (k) return (KEY = k.trim());
  const envFile = process.env.MESHY_ENV_FILE || path.join(REPO, ".env");
  if (fs.existsSync(envFile)) {
    for (const line of fs.readFileSync(envFile, "utf8").split(/\r?\n/)) {
      const m = line.match(/^\s*(?:export\s+)?(MESHI_API_KEY|MESHY_API_KEY)\s*=\s*(.*?)\s*$/);
      if (m && m[2]) return (KEY = m[2].replace(/^["']|["']$/g, "").trim());
    }
  }
  throw new MeshyError("auth", "no key: set MESHI_API_KEY in the environment or in " + envFile);
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
  const msg = (body && (body.message || body.error)) || JSON.stringify(body || "").slice(0, 300);
  if (status === 401) return new MeshyError("auth", `401 ${msg}`, status);
  if (status === 402) return new MeshyError("quota", `402 insufficient credits or plan: ${msg}`, status);
  if (status === 429) return new MeshyError("quota", `429 rate/concurrency limit: ${msg}`, status);
  if (status === 403) return new MeshyError("validation", `403 forbidden/moderated: ${msg}`, status);
  if (status === 404) return new MeshyError("not_found", `404 ${msg}`, status);
  if (status === 400 || status === 422) return new MeshyError("validation", `${status} ${msg}`, status);
  return new MeshyError("api", `${status} ${msg}`, status);
}

const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
const log = (...a) => console.error("[meshy]", ...a);

async function api(method, p, body, attempt = 0) {
  const key = loadKey();
  let res;
  try {
    res = await fetch(BASE + p, {
      method,
      headers: { Authorization: `Bearer ${key}`, ...(body ? { "Content-Type": "application/json" } : {}) },
      body: body ? JSON.stringify(body) : undefined,
      signal: AbortSignal.timeout(120000),
    });
  } catch (e) {
    // retry only reads; a POST that may have reached the server is never re-sent (it would spend credits twice)
    if (method === "GET" && attempt < 2) { await sleep(3000); return api(method, p, body, attempt + 1); }
    throw new MeshyError("network", String((e && e.message) || e));
  }
  const text = await res.text();
  let json; try { json = text ? JSON.parse(text) : {}; } catch { json = { raw: text.slice(0, 300) }; }
  if (res.status === 429 && /Rate limit exceeded/i.test(text) && attempt < 3) {
    const ra = Number(res.headers.get("retry-after") || 2);
    await sleep(ra * 1000); return api(method, p, body, attempt + 1);
  }
  if (res.status >= 500 && method === "GET" && attempt < 2) { await sleep(3000); return api(method, p, body, attempt + 1); }
  if (!res.ok) throw classify(res.status, json);
  return json;
}

function urlsOf(task) {
  const out = {};
  const add = (k, v) => { if (typeof v === "string" && /^https?:/.test(v)) out[k] = v; };
  for (const [k, v] of Object.entries(task.model_urls || {})) add("model." + k, v);
  (task.texture_urls || []).forEach((t, i) => { for (const [k, v] of Object.entries(t || {})) add(`texture${i}.${k}`, v); });
  add("thumbnail", task.thumbnail_url);
  const r = task.result && typeof task.result === "object" ? task.result : {};
  for (const [k, v] of Object.entries(r)) {
    if (v && typeof v === "object") for (const [k2, v2] of Object.entries(v)) add(`${k}.${k2}`, v2);
    else add(k, v);
  }
  return out;
}

async function getTask(id, kind) {
  if (kind) {
    if (!KINDS[kind]) throw new MeshyError("validation", `unknown kind ${kind}; one of ${Object.keys(KINDS).join(", ")}`);
    return { kind, task: await api("GET", `${KINDS[kind]}/${id}`) };
  }
  // Verified 2026-11-12: GET /openapi/v2/text-to-3d/<id> answers 200 for ANY task id (a rig task comes back with
  // type "rig" in the text-to-3d shape, without its result block). So read `type`, then refetch from the right endpoint.
  const TYPE_TO_KIND = { "text-to-3d-preview": "text-to-3d", "text-to-3d-refine": "text-to-3d", "image-to-3d": "image-to-3d",
    "rig": "rigging", "animate": "animations", "text-to-motion": "text-to-motion" };
  for (const k of Object.keys(KINDS)) {
    let task;
    try { task = await api("GET", `${KINDS[k]}/${id}`); }
    catch (e) { if (e.kind === "not_found" || (e.kind === "validation" && e.http === 400)) continue; throw e; }
    const real = TYPE_TO_KIND[task.type] || k;
    if (real === k) return { kind: k, task };
    return { kind: real, task: await api("GET", `${KINDS[real]}/${id}`) };
  }
  throw new MeshyError("not_found", `task ${id} not found under any kind`);
}

async function waitTask(id, kind, opt) {
  const timeoutMs = Number(opt["timeout-s"] || 1200) * 1000;
  const interval = Math.max(3, Number(opt["interval-s"] || 10)) * 1000;
  const t0 = Date.now();
  let last = "";
  for (;;) {
    const { task } = await getTask(id, kind);
    const line = `${task.status} ${task.progress ?? ""}%${task.preceding_tasks ? " queue " + task.preceding_tasks : ""}`;
    if (line !== last) { log(`${id} ${line} t=${Math.round((Date.now() - t0) / 1000)}s`); last = line; }
    if (["SUCCEEDED", "FAILED", "CANCELED", "EXPIRED"].includes(task.status)) return task;
    if (Date.now() - t0 > timeoutMs) throw new MeshyError("timeout", `task ${id} still ${task.status} after ${timeoutMs / 1000}s (it keeps running server-side; resume with: status ${id} --wait)`);
    await sleep(interval);
  }
}

function ledger(opt, rec) {
  const f = opt.ledger || process.env.MESHY_LEDGER;
  if (f) fs.appendFileSync(f, JSON.stringify({ at: new Date().toISOString(), ...rec }) + "\n");
}

function taskSummary(cmd, kind, task, t0) {
  const s = {
    cmd, ok: task.status === "SUCCEEDED", kind, task_id: task.id, status: task.status, progress: task.progress,
    elapsed_ms: Date.now() - t0,
    server_ms: task.finished_at && task.started_at ? task.finished_at - task.started_at : undefined,
    queue_ms: task.started_at && task.created_at ? task.started_at - task.created_at : undefined,
    credits: task.consumed_credits, urls: urlsOf(task),
    expires_at: task.expires_at || undefined,
    error: task.task_error && task.task_error.message ? { kind: "task_failed", message: task.task_error.message } : undefined,
  };
  return clean(s);
}

async function createAndMaybeWait(cmd, kind, body, opt, t0) {
  if (opt["dry-run"]) return { cmd, ok: true, kind, dry_run: true, body: { ...body, image_url: body.image_url ? body.image_url.slice(0, 40) + "..." : undefined } };
  const created = await api("POST", KINDS[kind], body);
  const id = created.result || created.id;
  log(`created ${kind} task ${id}`);
  if (!opt.wait) return { cmd, ok: true, kind, task_id: id, status: "SUBMITTED", elapsed_ms: Date.now() - t0 };
  return taskSummary(cmd, kind, await waitTask(id, kind, opt), t0);
}

const list = (v) => String(v).split(",").map((x) => x.trim()).filter(Boolean);
const num = (v) => (v === undefined ? undefined : Number(v));
const bool = (v) => (v === undefined ? undefined : v === true || v === "true");
function clean(o) { for (const k of Object.keys(o)) if (o[k] === undefined) delete o[k]; return o; }

function imageToUrl(src) {
  if (/^(https?:|data:)/.test(src)) return src;
  const ext = path.extname(src).toLowerCase().replace(".", "");
  const mime = { png: "image/png", jpg: "image/jpeg", jpeg: "image/jpeg", webp: "image/webp" }[ext];
  if (!mime) throw new MeshyError("validation", `image must be png/jpg/webp: ${src}`);
  return `data:${mime};base64,${fs.readFileSync(src).toString("base64")}`;
}

const COMMANDS = {
  async balance() {
    const b = await api("GET", "/openapi/v1/balance");
    return clean({ cmd: "balance", ok: true, balance: b.balance, monthly_credits: b.monthly_credits, permanent_credits: b.permanent_credits, monthly_resets_at: b.monthly_resets_at ?? undefined });
  },

  async "text-to-3d"(pos, opt, t0) {
    const mode = opt["preview-task"] ? "refine" : "preview";
    let body;
    if (mode === "preview") {
      if (!opt.prompt) throw new MeshyError("validation", "--prompt required for preview");
      body = clean({
        mode, prompt: opt.prompt, ai_model: opt["ai-model"], model_type: opt["model-type"],
        pose_mode: opt.pose, topology: opt.topology, target_polycount: num(opt.polycount),
        should_remesh: bool(opt.remesh), geometry_resolution: opt["geometry-res"],
        target_formats: opt.formats ? list(opt.formats) : undefined,
      });
    } else {
      body = clean({
        mode, preview_task_id: opt["preview-task"], ai_model: opt["ai-model"], enable_pbr: opt.pbr ? true : undefined,
        texture_resolution: opt["texture-res"], texture_prompt: opt["texture-prompt"],
        remove_lighting: bool(opt["remove-lighting"]),
        target_formats: opt.formats ? list(opt.formats) : undefined,
      });
    }
    return createAndMaybeWait(`text-to-3d:${mode}`, "text-to-3d", body, opt, t0);
  },

  async "image-to-3d"(pos, opt, t0) {
    if (!opt.image) throw new MeshyError("validation", "--image <png|jpg|url> required");
    const body = clean({
      image_url: imageToUrl(opt.image), ai_model: opt["ai-model"], model_type: opt["model-type"],
      should_texture: bool(opt.texture), texture_prompt: opt["texture-prompt"], pose_mode: opt.pose,
      topology: opt.topology, target_polycount: num(opt.polycount), should_remesh: bool(opt.remesh),
      texture_resolution: opt["texture-res"], enable_pbr: opt.pbr ? true : undefined,
      target_formats: opt.formats ? list(opt.formats) : undefined,
    });
    return createAndMaybeWait("image-to-3d", "image-to-3d", body, opt, t0);
  },

  async rig(pos, opt, t0) {
    const body = clean({
      input_task_id: opt["input-task"], model_url: opt["model-url"],
      height_meters: num(opt.height), skeleton_template: opt.template || "ue5_mannequin",
      include_fingers: opt.fingers ? true : undefined,
    });
    if (!body.input_task_id && !body.model_url) throw new MeshyError("validation", "--input-task <id> or --model-url <glb url> required");
    return createAndMaybeWait("rig", "rigging", body, opt, t0);
  },

  async animate(pos, opt, t0) {
    if (!opt["rig-task"]) throw new MeshyError("validation", "--rig-task required");
    const body = clean({
      rig_task_id: opt["rig-task"],
      action_id: opt.action ? Number(opt.action) : undefined,
      action_ids: opt.actions ? list(opt.actions).map(Number) : undefined,
      motion_task_id: opt["motion-task"],
      post_process: opt.fps ? { operation_type: "change_fps", fps: Number(opt.fps) } : undefined,
    });
    const n = ["action_id", "action_ids", "motion_task_id"].filter((k) => body[k] !== undefined).length;
    if (n !== 1) throw new MeshyError("validation", "exactly one of --action, --actions, --motion-task");
    return createAndMaybeWait("animate", "animations", body, opt, t0);
  },

  async motion(pos, opt, t0) {
    if (!opt.prompt || !opt.duration) throw new MeshyError("validation", "--prompt and --duration (2..10, step 0.5) required");
    const body = { prompt: opt.prompt, duration: Number(opt.duration), mode: opt.mode || "swift" };
    return createAndMaybeWait(`text-to-motion:${body.mode}`, "text-to-motion", body, opt, t0);
  },

  async library(pos, opt) {
    const q = new URLSearchParams();
    for (const k of ["search", "category", "sub_category", "action_ids", "page_size", "page_num"]) if (opt[k]) q.set(k, opt[k]);
    const r = await api("GET", "/openapi/v1/animations/library" + (q.toString() ? "?" + q : ""));
    const items = Array.isArray(r) ? r : r.result || r.items || r.data || [];
    for (const it of items) console.log(JSON.stringify(it));
    return { cmd: "library", ok: true, count: items.length };
  },

  async status(pos, opt, t0) {
    if (!pos[0]) throw new MeshyError("validation", "status <task_id> [--kind k] [--wait]");
    const { kind, task } = await getTask(pos[0], opt.kind);
    if (opt.wait) return taskSummary("status", kind, await waitTask(pos[0], kind, opt), t0);
    if (opt.raw) console.log(JSON.stringify(task, null, 2));
    return taskSummary("status", kind, task, t0);
  },

  async download(pos, opt, t0) {
    const [id, dir] = pos;
    if (!id || !dir) throw new MeshyError("validation", "download <task_id> <dir> [--kind k] [--only glb,fbx]");
    const { kind, task } = await getTask(id, opt.kind);
    fs.mkdirSync(dir, { recursive: true });
    fs.writeFileSync(path.join(dir, `${id}.task.json`), JSON.stringify(task, null, 2));
    const only = opt.only ? list(opt.only) : null;
    const files = {};
    for (const [k, u] of Object.entries(urlsOf(task))) {
      const ext = ((new URL(u).pathname.match(/\.([a-z0-9]+)$/i) || [])[1] || "bin").toLowerCase();
      if (only && !only.includes(ext)) continue;
      const base = k.replace(/[^a-z0-9_.-]/gi, "_");
      const name = base.toLowerCase().endsWith("." + ext) ? base : `${base}.${ext}`;
      const res = await fetch(u, { signal: AbortSignal.timeout(300000) });
      if (!res.ok) { log(`skip ${k}: HTTP ${res.status}`); continue; }
      const buf = Buffer.from(await res.arrayBuffer());
      fs.writeFileSync(path.join(dir, name), buf);
      files[name] = buf.length;
      log(`saved ${name} ${buf.length} B`);
    }
    const s = taskSummary("download", kind, task, t0);
    delete s.urls;
    return { ...s, dir: path.resolve(dir), files };
  },
};

const HELP = `meshy.mjs <command> [options]   (key: MESHI_API_KEY env or <repo>/.env)
  balance
  text-to-3d --prompt "..." [--ai-model meshy-6-lite|meshy-6|meshy-7.1|latest|meshy-t2.5] [--model-type standard|smart-topology]
             [--pose a-pose|t-pose] [--polycount N] [--topology triangle|quad] [--remesh true|false] [--formats glb,fbx] --wait
  text-to-3d --preview-task <id> [--texture-res 2k] [--pbr] [--texture-prompt "..."] [--remove-lighting false] --wait   (refine)
  image-to-3d --image <file.png|url> [--ai-model ...] [--pose a-pose] [--polycount N] [--texture false] --wait
  rig --input-task <id> | --model-url <glb> [--template ue5_mannequin] [--height 1.2] [--fingers] --wait
  animate --rig-task <id> (--action N | --actions 1,2,3 | --motion-task <id>) [--fps 30|60] --wait
  motion --prompt "..." --duration 4 [--mode swift|prime] --wait          (text to motion)
  library [--search run] [--category WalkAndRun]
  status <task> [--kind text-to-3d|image-to-3d|rigging|animations|text-to-motion] [--wait] [--raw]
  download <task> <dir> [--kind ...] [--only glb,fbx]
common: --wait --timeout-s 1200 --interval-s 10 --dry-run --ledger <file.jsonl> (or MESHY_LEDGER)`;

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
    console.log(JSON.stringify(line));
    process.exitCode = out.ok ? 0 : 1;
  } catch (e) {
    const kind = e instanceof MeshyError ? e.kind : "internal";
    const line = { tool: TOOL, cmd, ok: false, elapsed_ms: Date.now() - t0, error: clean({ kind, message: String(e.message || e), http: e.http }) };
    ledger(opt, line);
    console.log(JSON.stringify(line));
    process.exitCode = ["auth", "network", "internal"].includes(kind) ? 2 : 1;
  }
}
main();
