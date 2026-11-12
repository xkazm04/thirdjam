#!/usr/bin/env node
// thirdjam ElevenLabs client: zero dependencies, Node >= 18 (global fetch). Mirrors tools/meshy/meshy.mjs.
// Key: ELEVEN_LABS_API_KEY (or ELEVENLABS_API_KEY) from the environment, else from ELEVEN_ENV_FILE
// (default C:/Users/kazda/jam/thirdjam/.env). Sent only in the xi-api-key header; never printed or logged.
// Last stdout line is always one JSON object: {"tool":"thirdjam.eleven/1","cmd":...,"ok":bool,...}
// Exit 0 ok, 1 API error, 2 could not run. A POST is never retried (it costs credits); GETs retry twice.
// Docs read 2026-11-12: elevenlabs.io/docs/api-reference/{text-to-speech/convert,text-to-sound-effects/convert,music/compose,voices/search,user/subscription/get}
import fs from "node:fs";
import path from "node:path";

const TOOL = "thirdjam.eleven/1";
const BASE = process.env.ELEVEN_BASE_URL || "https://api.elevenlabs.io";
let KEY;
class Err extends Error { constructor(kind, msg, http) { super(msg); this.kind = kind; this.http = http; } }
function loadKey() {
  if (KEY) return KEY;
  const k = process.env.ELEVEN_LABS_API_KEY || process.env.ELEVENLABS_API_KEY;
  if (k) return (KEY = k.trim());
  const f = process.env.ELEVEN_ENV_FILE || "C:/Users/kazda/jam/thirdjam/.env";
  if (fs.existsSync(f)) for (const line of fs.readFileSync(f, "utf8").split(/\r?\n/)) {
    const m = line.match(/^\s*(?:export\s+)?(ELEVEN_LABS_API_KEY|ELEVENLABS_API_KEY)\s*=\s*(.*?)\s*$/);
    if (m && m[2]) return (KEY = m[2].replace(/^["']|["']$/g, "").trim());
  }
  throw new Err("auth", "no key: set ELEVEN_LABS_API_KEY in the environment or " + f);
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
function classify(status, text) {
  let msg = text.slice(0, 300);
  try { const j = JSON.parse(text); msg = JSON.stringify(j.detail || j).slice(0, 300); } catch {}
  const kind = status === 401 ? "auth" : status === 402 || status === 429 ? "quota" : status === 400 || status === 422 || status === 403 ? "validation" : status === 404 ? "not_found" : "api";
  return new Err(kind, `${status} ${msg}`, status);
}
async function get(p, attempt = 0) {
  try {
    const res = await fetch(BASE + p, { headers: { "xi-api-key": loadKey() }, signal: AbortSignal.timeout(60000) });
    if (!res.ok) throw classify(res.status, await res.text());
    return await res.json();
  } catch (e) {
    if (e instanceof Err && e.http && e.http < 500) throw e;
    if (e instanceof Err && e.kind === "auth") throw e;
    if (attempt < 2) { await new Promise((r) => setTimeout(r, 1500)); return get(p, attempt + 1); }
    throw e instanceof Err ? e : new Err("network", String(e.message || e));
  }
}
// POST returning audio bytes. Never retried.
async function postAudio(p, body, out) {
  let res;
  try {
    res = await fetch(BASE + p, {
      method: "POST",
      headers: { "xi-api-key": loadKey(), "Content-Type": "application/json", Accept: "audio/mpeg" },
      body: JSON.stringify(body), signal: AbortSignal.timeout(180000),
    });
  } catch (e) { throw new Err("network", "POST failed (not retried): " + (e.message || e)); }
  if (!res.ok) throw classify(res.status, await res.text());
  const buf = Buffer.from(await res.arrayBuffer());
  fs.mkdirSync(path.dirname(path.resolve(out)), { recursive: true });
  fs.writeFileSync(out, buf);
  return { bytes: buf.length, cost: res.headers.get("character-cost"), request_id: res.headers.get("request-id") };
}
// POST returning JSON. Never retried.
async function postJson(p, body) {
  let res;
  try {
    res = await fetch(BASE + p, { method: "POST", headers: { "xi-api-key": loadKey(), "Content-Type": "application/json" }, body: JSON.stringify(body), signal: AbortSignal.timeout(240000) });
  } catch (e) { throw new Err("network", "POST failed (not retried): " + (e.message || e)); }
  if (!res.ok) throw classify(res.status, await res.text());
  return { json: await res.json(), cost: res.headers.get("character-cost") };
}
const num = (v, d) => (v === undefined || v === true ? d : Number(v));
function ledger(opt, rec) {
  const f = opt.ledger || process.env.ELEVEN_LEDGER;
  if (f) { fs.mkdirSync(path.dirname(path.resolve(f)), { recursive: true }); fs.appendFileSync(f, JSON.stringify(rec) + "\n"); }
}
function need(opt, ...names) { for (const n of names) if (!opt[n] || opt[n] === true) throw new Err("usage", `missing --${n}`); }
function usage() {
  return `eleven.mjs <cmd> [options]
  quota                                   plan, credits used/limit/remaining, reset
  voices [--search s] [--category premade] [--page-size 100]
  tts --voice <id> --text "..." --out f.mp3 [--model eleven_multilingual_v2] [--stability .5 --similarity .75 --style 0 --speed 1] [--format mp3_44100_128] [--seed N]
  sfx --prompt "..." --seconds N [--loop] [--influence .3] [--model eleven_text_to_sound_v2] --out f.mp3 [--format ...]
  voice-design --description "..." --text "100-1000 chars" --out base [--guidance 5 --loudness .5 --seed N --model eleven_multilingual_ttv_v2]
  voice-save --generated <id> --name N --description "..."   (uses a voice slot)
  music --prompt "..." --seconds N [--model music_v2_5] [--instrumental false to allow vocals] --out f.mp3
common: --dry-run --ledger <file.jsonl> (or ELEVEN_LEDGER)`;
}

async function main() {
  const { pos, opt } = parseArgs(process.argv.slice(2));
  const cmd = pos[0];
  const t0 = Date.now();
  const rec = { tool: TOOL, cmd, ok: false };
  const fin = (extra, code = 0) => {
    Object.assign(rec, extra, { elapsed_ms: Date.now() - t0 });
    ledger(opt, { ...rec, ts: new Date().toISOString() });
    console.log(JSON.stringify(rec));
    process.exit(code);
  };
  try {
    if (!cmd || cmd === "help") { console.error(usage()); return fin({ ok: false, error: "usage" }, 2); }
    if (cmd === "quota") {
      const s = await get("/v1/user/subscription");
      return fin({ ok: true, tier: s.tier, status: s.status, used: s.character_count, limit: s.character_limit,
        remaining: s.character_limit - s.character_count, reset_unix: s.next_character_count_reset_unix,
        extension: s.max_credit_limit_extension, ivc: s.can_use_instant_voice_cloning,
        voice_slots_used: s.voice_slots_used, voice_limit: s.voice_limit, can_extend_voice_limit: s.can_extend_voice_limit });
    }
    if (cmd === "voices") {
      const q = new URLSearchParams({ page_size: String(num(opt["page-size"], 100)) });
      if (opt.search && opt.search !== true) q.set("search", opt.search);
      if (opt.category && opt.category !== true) q.set("category", opt.category);
      const j = await get("/v2/voices?" + q);
      const voices = j.voices.map((v) => ({ voice_id: v.voice_id, name: v.name, category: v.category, labels: v.labels, description: v.description, preview_url: v.preview_url }));
      console.log(JSON.stringify(voices, null, 1));
      return fin({ ok: true, count: voices.length, total: j.total_count, has_more: j.has_more });
    }
    if (cmd === "voice-design") {
      // POST /v1/text-to-voice/design -> previews (base64 mp3) saved as <out>_1.mp3 ... plus <out>.json (ids, durations)
      need(opt, "description", "text", "out");
      const body = { voice_description: opt.description, text: opt.text, model_id: opt.model || "eleven_multilingual_ttv_v2" };
      if (opt.guidance !== undefined) body.guidance_scale = Number(opt.guidance);
      if (opt.loudness !== undefined) body.loudness = Number(opt.loudness);
      if (opt.seed !== undefined) body.seed = Number(opt.seed);
      Object.assign(rec, { description: opt.description, chars: opt.text.length });
      if (opt["dry-run"]) return fin({ ok: true, dry_run: true, endpoint: "/v1/text-to-voice/design", body });
      const r = await postJson("/v1/text-to-voice/design", body);
      const base = opt.out.replace(/.json$|.mp3$/, "");
      fs.mkdirSync(path.dirname(path.resolve(base)), { recursive: true });
      const previews = r.json.previews.map((pv, i) => {
        const f = base + "_" + (i + 1) + ".mp3";
        fs.writeFileSync(f, Buffer.from(pv.audio_base_64, "base64"));
        return { file: f, generated_voice_id: pv.generated_voice_id, duration_secs: pv.duration_secs };
      });
      fs.writeFileSync(base + ".json", JSON.stringify({ description: opt.description, text: r.json.text, previews }, null, 1));
      return fin({ ok: true, previews, credits_header: r.cost });
    }
    if (cmd === "voice-save") {
      // POST /v1/text-to-voice: save a preview as an account voice (uses one voice slot)
      need(opt, "generated", "name", "description");
      const body = { voice_name: opt.name, voice_description: opt.description, generated_voice_id: opt.generated };
      Object.assign(rec, { name: opt.name, generated: opt.generated });
      if (opt["dry-run"]) return fin({ ok: true, dry_run: true, endpoint: "/v1/text-to-voice", body });
      const r = await postJson("/v1/text-to-voice", body);
      return fin({ ok: true, voice_id: r.json.voice_id, name: r.json.name });
    }
    let p, body, kind;
    if (cmd === "tts") {
      need(opt, "voice", "text", "out");
      p = `/v1/text-to-speech/${encodeURIComponent(opt.voice)}`;
      body = { text: opt.text, model_id: opt.model || "eleven_multilingual_v2" };
      const vs = {};
      for (const [o, k] of [["stability", "stability"], ["similarity", "similarity_boost"], ["style", "style"], ["speed", "speed"]]) if (opt[o] !== undefined) vs[k] = Number(opt[o]);
      if (Object.keys(vs).length) body.voice_settings = vs;
      if (opt.seed !== undefined) body.seed = Number(opt.seed);
      kind = { voice: opt.voice, text: opt.text, chars: opt.text.length };
    } else if (cmd === "sfx") {
      need(opt, "prompt", "out");
      p = "/v1/sound-generation";
      body = { text: opt.prompt, model_id: opt.model || "eleven_text_to_sound_v2", prompt_influence: num(opt.influence, 0.3) };
      if (opt.seconds !== undefined) body.duration_seconds = Number(opt.seconds);
      if (opt.loop) body.loop = true;
      kind = { prompt: opt.prompt, seconds: body.duration_seconds, loop: !!opt.loop };
    } else if (cmd === "music") {
      need(opt, "prompt", "out");
      p = "/v1/music";
      body = { prompt: opt.prompt, model_id: opt.model || "music_v2_5", force_instrumental: opt.instrumental !== "false" };
      if (opt.seconds !== undefined) body.music_length_ms = Math.round(Number(opt.seconds) * 1000);
      kind = { prompt: opt.prompt, seconds: Number(opt.seconds) };
    } else return fin({ ok: false, error: "unknown command " + cmd }, 2);
    if (opt.format) p += "?output_format=" + encodeURIComponent(opt.format);
    rec.out = opt.out;
    Object.assign(rec, kind);
    if (opt["dry-run"]) { console.error("[eleven] dry run, would POST", p, JSON.stringify(body)); return fin({ ok: true, dry_run: true, endpoint: p, body }); }
    const r = await postAudio(p, body, opt.out);
    return fin({ ok: true, bytes: r.bytes, credits_header: r.cost, request_id: r.request_id });
  } catch (e) {
    console.error("[eleven]", e.kind || "error", e.message);
    fin({ ok: false, error: e.kind || "error", http: e.http, message: String(e.message).slice(0, 300) }, e.kind === "usage" || e.kind === "auth" || e.kind === "network" ? 2 : 1);
  }
}
main();
