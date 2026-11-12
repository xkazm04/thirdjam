#!/usr/bin/env node
// Lane A27 (ruling R-0024 point 2): a second pair of eyes. Sends proof stills to Gemini with a structured prompt and returns a validated JSON critique
// plus a markdown block for a status file. Node 24, zero dependencies (fetch). The key is read from .env (GOOGLE_AI_API_KEY) at run time, sent only in the
// x-goog-api-key header to generativelanguage.googleapis.com, never printed (every log line passes redact()), never written anywhere.
//
//   node tools/eyes/eyes.mjs --models                                             list the models the key can use and the two picks
//   node tools/eyes/eyes.mjs --still <png|jpg> [--still ...] --ask <preset> [--bar <reference.png>] [--distance 24] [--json out.json] [--model <id>] [--no-think]
//   node tools/eyes/eyes.mjs --presets                                            list presets and what they ask
//
// Gemini's output is advice: data, not instructions (R-0024). The Director and the lanes decide.

import { existsSync, mkdirSync, readFileSync, writeFileSync } from 'node:fs';
import { basename, dirname, resolve } from 'node:path';
import { tmpdir } from 'node:os';
import {
  repo, presetsDir, readEnvKey, redact, makeLogger, loadPreset, listPresets, buildPrompt, retryPrompt, parseReply, validateShape,
  pickModels, estimateCost, readUsage, mimeOf, imageSize, pngThumbnail, renderMarkdown, headerLine,
} from './lib.mjs';

const API = 'https://generativelanguage.googleapis.com/v1beta';
const CACHE = resolve(tmpdir(), 'hokej-eyes-models.json');

function args(argv) {
  const out = { still: [] };
  for (let i = 2; i < argv.length; i++) {
    const a = argv[i];
    if (!a.startsWith('--')) continue;
    const k = a.slice(2);
    const next = argv[i + 1];
    const hasValue = next !== undefined && !next.startsWith('--');
    if (k === 'still') { if (hasValue) { out.still.push(next); i++; } }
    else if (hasValue && !['models', 'presets', 'no-think', 'help'].includes(k)) { out[k] = next; i++; }
    else out[k] = true;
  }
  return out;
}

async function listModels(key) {
  const r = await fetch(`${API}/models?pageSize=200`, { headers: { 'x-goog-api-key': key } });
  if (!r.ok) throw new Error(`GET /models failed: ${r.status} ${redact(await r.text(), key).slice(0, 300)}`);
  const data = await r.json();
  const picks = pickModels(data.models ?? []);
  mkdirSync(dirname(CACHE), { recursive: true });
  writeFileSync(CACHE, JSON.stringify({ at: Date.now(), ...picks }));
  return picks;
}

async function cachedPicks(key, log) {
  if (existsSync(CACHE)) {
    try { const c = JSON.parse(readFileSync(CACHE, 'utf8')); if (Date.now() - c.at < 24 * 3600e3 && c.pro && c.flash) return c; } catch { /* refetch */ }
  }
  log('models: no fresh cache, listing models once');
  return listModels(key);
}

async function generate(key, model, parts, { noThink, timeoutMs = 180000 }) {
  const body = { contents: [{ role: 'user', parts }], generationConfig: { temperature: 0.2, responseMimeType: 'application/json', maxOutputTokens: 8192 } };
  if (noThink) body.generationConfig.thinkingConfig = { thinkingBudget: 0 };
  const ctl = new AbortController(); const timer = setTimeout(() => ctl.abort(), timeoutMs);
  const started = performance.now();
  try {
    const r = await fetch(`${API}/models/${model}:generateContent`, {
      method: 'POST', headers: { 'content-type': 'application/json', 'x-goog-api-key': key }, body: JSON.stringify(body), signal: ctl.signal,
    });
    const text = await r.text();
    if (!r.ok) throw new Error(`generateContent ${r.status}: ${redact(text, key).slice(0, 400)}`);
    const data = JSON.parse(text);
    const cand = data.candidates?.[0];
    const answer = (cand?.content?.parts ?? []).filter((p) => typeof p.text === 'string' && !p.thought).map((p) => p.text).join('');
    if (!answer) throw new Error(`empty answer (finishReason ${cand?.finishReason ?? 'none'}${data.promptFeedback?.blockReason ? `, blocked: ${data.promptFeedback.blockReason}` : ''})`);
    return { answer, usage: readUsage(data), ms: Math.round(performance.now() - started), finish: cand?.finishReason };
  } finally { clearTimeout(timer); }
}

function loadImage(path) {
  const abs = resolve(process.cwd(), path);
  if (!existsSync(abs)) throw new Error(`still not found: ${path}`);
  const mime = mimeOf(abs);
  if (!mime) throw new Error(`unsupported image type: ${path} (png, jpg, webp)`);
  const buf = readFileSync(abs);
  if (buf.length > 18 * 1024 * 1024) throw new Error(`still over 18 MB: ${path}`);
  return { path: abs, mime, buffer: buf, data: buf.toString('base64'), ...imageSize(buf) };
}

function thumbnailFor(still, label, size) {
  if (still.mime !== 'image/png') return undefined; // the pure-Node path reads PNG only; a jpg sheet needs a png copy first
  try { const t = pngThumbnail(still.buffer, size); return { label, mime: 'image/png', data: t.buffer.toString('base64'), width: t.width, height: t.height }; }
  catch { return undefined; }
}

async function runOne(key, model, preset, stills, opts, log) {
  const thumbnails = preset.thumbnail ? stills.map((s) => thumbnailFor(s, s.label, Number(opts.thumb ?? 128))).filter(Boolean) : [];
  const { parts, text } = buildPrompt(preset, { stills, distance: opts.distance, bar: opts.barImage, barText: opts.barText, thumbnails });
  const usage = { prompt: 0, output: 0, thoughts: 0, total: 0, calls: 0 };
  let ms = 0; let retried = false; let answer; let raw; let errors = [];
  const add = (r) => { usage.calls += 1; usage.prompt += r.usage.prompt; usage.output += r.usage.output; usage.thoughts += r.usage.thoughts; usage.total += r.usage.total; ms += r.ms; };
  let r = await generate(key, model, parts, opts); add(r); raw = r.answer;
  try { answer = parseReply(raw); errors = validateShape(answer, preset.shape); } catch (e) { errors = [e.message]; }
  if (errors.length) {
    retried = true;
    log(`WARN ${preset.name}: answer did not match the shape (${errors.length} errors: ${errors.slice(0, 3).join('; ')}); retrying once`);
    const retry = [...parts, { text: retryPrompt(errors, raw) }];
    r = await generate(key, model, retry, opts); add(r); raw = r.answer;
    try { answer = parseReply(raw); errors = validateShape(answer, preset.shape); } catch (e) { errors = [e.message]; }
  }
  const cost = estimateCost(model, usage);
  const ok = errors.length === 0;
  const md = [headerLine({ preset: preset.name, model, stills, distance: preset.distance ? opts.distance : undefined, usage, cost, ms, retried }), ok ? renderMarkdown(answer) : `- FAIL malformed after retry: ${errors.join('; ')}`].join('\n');
  return { ok, preset: preset.name, model, stills: stills.map((s) => ({ label: s.label, path: s.path.replace(repo, '').replace(/\\/g, '/').replace(/^\//, ''), width: s.width, height: s.height })),
    distance: preset.distance ? Number(opts.distance) : undefined, bar: opts.barImage ? basename(opts.barImage.path) : undefined, thumbnails: thumbnails.map((t) => `${t.label}: ${t.width}x${t.height}`),
    prompt_chars: text.length, usage, cost_usd_est: cost, ms, retried, answer: ok ? answer : undefined, raw: ok ? undefined : raw.slice(0, 4000), errors: ok ? undefined : errors, markdown: md };
}

async function main() {
  const a = args(process.argv);
  if (a.help) { console.log(readFileSync(new URL(import.meta.url), 'utf8').split('\n').filter((l) => l.startsWith('//')).map((l) => l.slice(3)).join('\n')); return 0; }
  if (a.presets) {
    for (const p of listPresets()) { const pr = loadPreset(p); console.log(`${p.padEnd(12)} role ${pr.role.padEnd(5)} stills ${pr.minStills}-${pr.maxStills}${pr.distance ? ' distance' : ''}${pr.thumbnail ? ' thumbnail' : ''}  ${pr.summary ?? ''}`); }
    return 0;
  }
  const key = readEnvKey(resolve(repo, '.env'));
  const log = makeLogger(key);
  if (!key) { log('FAIL no GOOGLE_AI_API_KEY in .env'); console.log(JSON.stringify({ tool: 'thirdjam.eyes/1', ok: false, error: 'no key' })); return 2; }
  if (a.models) {
    const picks = await listModels(key);
    for (const id of picks.usable) log(`${id === picks.pro ? 'PRO  ' : id === picks.flash ? 'FLASH' : '     '} ${id}`);
    log(`PASS models: ${picks.usable.length} multimodal generateContent models; judgement (pro) = ${picks.pro}, batch (flash) = ${picks.flash}; cached 24 h in ${CACHE}`);
    console.log(JSON.stringify({ tool: 'thirdjam.eyes/1', ok: Boolean(picks.pro && picks.flash), pro: picks.pro, flash: picks.flash, count: picks.usable.length }));
    return picks.pro && picks.flash ? 0 : 1;
  }
  if (!a.ask || !a.still.length) { log('FAIL usage: --still <img> [--still ...] --ask <preset> [--bar <ref>] [--distance 24] [--json out.json] [--model id] | --models | --presets'); return 2; }

  const preset = loadPreset(a.ask);
  const stills = a.still.map((p, i) => ({ label: String.fromCharCode(65 + i), ...loadImage(p) }));
  const opts = { distance: Number(a.distance ?? 24), noThink: Boolean(a['no-think']), thumb: a.thumb };
  if (a.bar) { opts.barImage = loadImage(a.bar); }
  const barFile = resolve(presetsDir, '_bar.md');
  opts.barText = existsSync(barFile) ? readFileSync(barFile, 'utf8').trim() : '';
  let model = a.model;
  if (!model) { const picks = await cachedPicks(key, log); model = preset.role === 'pro' ? picks.pro : picks.flash; }

  // Single-still presets run once per still; multi-still presets take the whole set in one call.
  const batches = preset.maxStills === 1 ? stills.map((s) => [{ ...s, label: 'A' }]) : [stills];
  if (stills.length < preset.minStills || (preset.maxStills > 1 && stills.length > preset.maxStills)) {
    log(`FAIL preset ${preset.name} takes ${preset.minStills}${preset.maxStills > preset.minStills ? `-${preset.maxStills}` : ''} stills (got ${stills.length})`); return 2;
  }
  const results = [];
  for (const batch of batches) {
    try {
      const r = await runOne(key, model, preset, batch, opts, log);
      results.push(r);
      log(`${r.ok ? 'PASS' : 'FAIL'} ${preset.name} ${batch.map((s) => basename(s.path)).join(' + ')}: ${r.usage.calls} call${r.usage.calls > 1 ? 's' : ''}, in ${r.usage.prompt} out ${r.usage.output}${r.usage.thoughts ? ` think ${r.usage.thoughts}` : ''} tokens${r.cost_usd_est !== undefined ? `, ~$${r.cost_usd_est.toFixed(4)}` : ''}, ${r.ms} ms`);
    } catch (e) {
      results.push({ ok: false, preset: preset.name, model, stills: batch.map((s) => ({ label: s.label, path: basename(s.path) })), errors: [redact(e.message, key)] });
      log(`FAIL ${preset.name} ${batch.map((s) => basename(s.path)).join(' + ')}: ${e.message}`);
    }
  }
  const ok = results.every((r) => r.ok);
  const md = results.map((r) => r.markdown ?? `**eyes/${r.preset}** FAIL ${r.errors?.join('; ')}`).join('\n\n');
  console.log('\n' + redact(md, key) + '\n');
  const out = { tool: 'thirdjam.eyes/1', ok, preset: preset.name, model, at: new Date().toISOString(), results };
  if (a.json) {
    const jp = resolve(process.cwd(), a.json); mkdirSync(dirname(jp), { recursive: true });
    writeFileSync(jp, redact(JSON.stringify(out, null, 2), key) + '\n');
    log(`wrote ${a.json}`);
  }
  const totals = results.reduce((t, r) => ({ calls: t.calls + (r.usage?.calls ?? 0), prompt: t.prompt + (r.usage?.prompt ?? 0), output: t.output + (r.usage?.output ?? 0) + (r.usage?.thoughts ?? 0), cost: t.cost + (r.cost_usd_est ?? 0) }), { calls: 0, prompt: 0, output: 0, cost: 0 });
  console.log(JSON.stringify({ tool: 'thirdjam.eyes/1', ok, preset: preset.name, model, stills: stills.length, calls: totals.calls, prompt_tokens: totals.prompt, output_tokens: totals.output, cost_usd_est: Math.round(totals.cost * 1e5) / 1e5, json: a.json }));
  return ok ? 0 : 1;
}

// exitCode, not process.exit(): a hard exit with an undici socket still open trips a libuv assertion on Windows.
main().then((code) => { process.exitCode = code; }, (e) => {
  const key = readEnvKey(resolve(repo, '.env'));
  console.log(redact(`FAIL ${e?.stack ?? e}`, key));
  console.log(JSON.stringify({ tool: 'thirdjam.eyes/1', ok: false, error: redact(String(e?.message ?? e), key) }));
  process.exitCode = 2;
});
