// Lane A27 (ruling R-0024 point 2): the pure part of tools/eyes. No network, no key handling beyond reading and redacting.
// eyes.mjs is the CLI that talks to generativelanguage.googleapis.com; everything it can test offline lives here.

import { existsSync, readFileSync, readdirSync } from 'node:fs';
import { basename, dirname, extname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { inflateSync, deflateSync } from 'node:zlib';

export const here = dirname(fileURLToPath(import.meta.url));
export const repo = resolve(here, '..', '..');
export const presetsDir = resolve(here, 'presets');

// ---------------------------------------------------------------- key and redaction

/** Reads one variable from a .env file without ever returning the rest of the file. */
export function readEnvKey(envPath, name = 'GOOGLE_AI_API_KEY') {
  if (!existsSync(envPath)) return undefined;
  for (const raw of readFileSync(envPath, 'utf8').split(/\r?\n/)) {
    const line = raw.trim();
    if (!line || line.startsWith('#')) continue;
    const eq = line.indexOf('=');
    if (eq < 0) continue;
    let k = line.slice(0, eq).trim();
    if (k.startsWith('export ')) k = k.slice(7).trim();
    if (k !== name) continue;
    let v = line.slice(eq + 1).trim();
    if ((v.startsWith('"') && v.endsWith('"')) || (v.startsWith("'") && v.endsWith("'"))) v = v.slice(1, -1);
    return v || undefined;
  }
  return undefined;
}

/** Replaces the key (and anything that looks like a Google API key) in any string. Every log line goes through this. */
export function redact(text, key) {
  let s = String(text ?? '');
  if (key && key.length >= 8) s = s.split(key).join('[REDACTED]');
  // Google AI Studio keys start with AIza and are 39 characters; catch any key-shaped token too.
  s = s.replace(/AIza[0-9A-Za-z_-]{20,}/g, '[REDACTED]');
  s = s.replace(/([?&]key=)[^&\s"']+/gi, '$1[REDACTED]');
  s = s.replace(/(x-goog-api-key["']?\s*[:=]\s*["']?)[^"',\s}]+/gi, '$1[REDACTED]');
  return s;
}

export function makeLogger(key, sink = (l) => console.log(l)) {
  return (...parts) => sink(redact(parts.map((p) => (typeof p === 'string' ? p : JSON.stringify(p))).join(' '), key));
}

// ---------------------------------------------------------------- presets

/**
 * A preset file: header lines `key: value`, a `## Prompt` section (with {{distance}}, {{bar}}, {{stills}} placeholders)
 * and a `## Shape` section holding one JSON object in a fence: the answer shape the reply is validated against.
 */
export function loadPreset(name, dir = presetsDir) {
  const file = resolve(dir, `${name}.md`);
  if (!existsSync(file)) throw new Error(`unknown preset "${name}" (have: ${listPresets(dir).join(', ')})`);
  return parsePreset(readFileSync(file, 'utf8'), name);
}

export function listPresets(dir = presetsDir) {
  return readdirSync(dir).filter((f) => f.endsWith('.md') && !f.startsWith('_')).map((f) => f.slice(0, -3)).sort();
}

export function parsePreset(text, name = 'preset') {
  const lines = text.split(/\r?\n/);
  const meta = { name, role: 'flash', stills: '1-8', thumbnail: false, distance: false };
  let i = 0;
  for (; i < lines.length && !lines[i].startsWith('## '); i++) {
    const m = lines[i].match(/^([a-z_]+):\s*(.*)$/);
    if (m) meta[m[1]] = m[2] === 'true' ? true : m[2] === 'false' ? false : m[2];
  }
  const section = (title) => {
    const start = lines.findIndex((l) => l.trim() === `## ${title}`);
    if (start < 0) throw new Error(`preset ${name}: missing "## ${title}" section`);
    let end = lines.findIndex((l, j) => j > start && l.startsWith('## '));
    if (end < 0) end = lines.length;
    return lines.slice(start + 1, end).join('\n').trim();
  };
  const prompt = section('Prompt');
  const shapeText = section('Shape');
  const fence = shapeText.match(/```(?:json)?\s*([\s\S]*?)```/);
  let shape;
  try { shape = JSON.parse(fence ? fence[1] : shapeText); } catch (e) { throw new Error(`preset ${name}: shape is not JSON: ${e.message}`); }
  const [minS, maxS] = parseRange(meta.stills, 1, 8);
  return { ...meta, prompt, shape, minStills: minS, maxStills: maxS };
}

function parseRange(s, dmin, dmax) {
  const m = String(s ?? '').match(/^(\d+)(?:\s*-\s*(\d+))?$/);
  if (!m) return [dmin, dmax];
  return [Number(m[1]), m[2] ? Number(m[2]) : Number(m[1])];
}

// ---------------------------------------------------------------- prompt

export const PREAMBLE = `You are a second pair of eyes for an art team making "Hokej", a 3-on-3 arcade ice hockey game (no goalies, no referees, no crowd): toy-figure skaters in two jersey colours (deep blue and warm orange), a small near-black puck, two red nets, a rink with boards, seen from a third-person camera behind one skater or from a lab camera along the ice. You are shown real screenshots from the game engine (not concept art). Judge only what is in the pixels; never invent elements you cannot see; if something is not visible say so. Your answer is advice for humans who decide, so be concrete and specific (pixels, colours, positions, sizes), never flattering, never hedged into uselessness.`;

export const JSON_RULES = `Answer with ONE JSON object only (no prose, no code fence) that matches this shape exactly: the same keys, no extra keys, no missing keys. Value specs: "string" = free text (one sentence unless told otherwise); "int a-b" = integer in that range; "number a-b" = number in that range; "enum x|y|z" = exactly one of those; "string[] a-b" = array of strings with that many items; "bool" = true or false; an array written as [ {...}, "a-b" ] = that many objects of that shape.`;

/**
 * Builds the text and image parts for one call.
 * stills: [{label, path, mime, data (base64)}]; bar: {path, mime, data} | undefined; barText: string | undefined.
 */
export function buildPrompt(preset, { stills, distance, bar, barText, thumbnails = [] }) {
  const stillList = stills.map((s) => `Still ${s.label}: ${basename(s.path)}`).join('\n');
  let body = preset.prompt
    .replace(/\{\{distance\}\}/g, String(distance ?? 24))
    .replace(/\{\{stills\}\}/g, stillList)
    .replace(/\{\{bar\}\}/g, barText ?? '');
  const text = `${PREAMBLE}\n\n${body}\n\n${JSON_RULES}\nShape:\n${JSON.stringify(preset.shape, null, 1)}`;
  const parts = [{ text }];
  if (bar) {
    parts.push({ text: 'REFERENCE (the bar to judge against; this is a concept image, not the game):' });
    parts.push({ inline_data: { mime_type: bar.mime, data: bar.data } });
  }
  for (const s of stills) {
    parts.push({ text: `Still ${s.label} (${basename(s.path)}${s.width ? `, ${s.width} x ${s.height} px` : ''}):` });
    parts.push({ inline_data: { mime_type: s.mime, data: s.data } });
    const t = thumbnails.find((x) => x.label === s.label);
    if (t) {
      parts.push({ text: `Still ${s.label} downscaled to ${t.width} x ${t.height} px (how it reads at distance):` });
      parts.push({ inline_data: { mime_type: t.mime, data: t.data } });
    }
  }
  return { text, parts };
}

export function retryPrompt(errors, previous) {
  return `Your previous answer did not match the required shape. Errors:\n- ${errors.join('\n- ')}\n\nYour previous answer was:\n${previous.slice(0, 4000)}\n\nAnswer again with ONE JSON object that matches the shape exactly (same keys, no extra keys, no missing keys, values in range).`;
}

// ---------------------------------------------------------------- reply parsing and validation

export function parseReply(text) {
  let t = String(text ?? '').trim();
  const fence = t.match(/```(?:json)?\s*([\s\S]*?)```/);
  if (fence) t = fence[1].trim();
  const start = t.indexOf('{');
  const end = t.lastIndexOf('}');
  if (start < 0 || end < start) throw new Error('reply has no JSON object');
  return JSON.parse(t.slice(start, end + 1));
}

/** Returns a list of error strings; empty = valid. Unknown keys are errors. */
export function validateShape(value, shape, path = '$') {
  const errors = [];
  const push = (m) => errors.push(`${path}: ${m}`);
  if (typeof shape === 'string') {
    const spec = shape.trim();
    const optional = spec.endsWith('?');
    const s = optional ? spec.slice(0, -1) : spec;
    if (value === undefined) { if (!optional) push('missing'); return errors; }
    if (s === 'any') return errors;
    if (s === 'string') { if (typeof value !== 'string' || (!optional && !value.trim())) push('must be a non-empty string'); return errors; } // "string?" may be absent or empty
    if (s === 'bool') { if (typeof value !== 'boolean') push('must be true or false'); return errors; }
    let m;
    if ((m = s.match(/^(int|number)(?:\s+(-?\d+(?:\.\d+)?)\s*-\s*(-?\d+(?:\.\d+)?))?$/))) {
      if (typeof value !== 'number' || Number.isNaN(value)) { push(`must be a ${m[1]}`); return errors; }
      if (m[1] === 'int' && !Number.isInteger(value)) push('must be an integer');
      if (m[2] !== undefined && (value < Number(m[2]) || value > Number(m[3]))) push(`must be between ${m[2]} and ${m[3]} (got ${value})`);
      return errors;
    }
    if ((m = s.match(/^enum\s+(.+)$/))) {
      const opts = m[1].split('|').map((x) => x.trim());
      if (!opts.includes(value)) push(`must be one of ${opts.join(' | ')} (got ${JSON.stringify(value)})`);
      return errors;
    }
    if ((m = s.match(/^string\[\](?:\s+(\d+)\s*-\s*(\d+))?$/))) {
      if (!Array.isArray(value) || value.some((x) => typeof x !== 'string')) { push('must be an array of strings'); return errors; }
      if (m[1] !== undefined && (value.length < Number(m[1]) || value.length > Number(m[2]))) push(`must have ${m[1]} to ${m[2]} items (got ${value.length})`);
      return errors;
    }
    push(`preset bug: unknown spec "${spec}"`);
    return errors;
  }
  if (Array.isArray(shape)) {
    const [itemShape, range] = shape;
    if (!Array.isArray(value)) { push('must be an array'); return errors; }
    const [lo, hi] = parseRange(range, 0, 1000);
    if (value.length < lo || value.length > hi) push(`must have ${lo} to ${hi} items (got ${value.length})`);
    value.forEach((v, i) => errors.push(...validateShape(v, itemShape, `${path}[${i}]`)));
    return errors;
  }
  if (shape && typeof shape === 'object') {
    if (!value || typeof value !== 'object' || Array.isArray(value)) { push('must be an object'); return errors; }
    for (const k of Object.keys(value)) if (!(k in shape)) push(`unknown key "${k}"`);
    for (const [k, sub] of Object.entries(shape)) errors.push(...validateShape(value[k], sub, `${path}.${k}`));
    return errors;
  }
  push('preset bug: unsupported shape');
  return errors;
}

// ---------------------------------------------------------------- models

// Image generators ("-image", nano banana), speech, transcription, omni, robotics, computer use and the custom-tools variants are not critics.
const EXCLUDE = /(embedding|tts|transcribe|audio|-image|nano-banana|imagen|veo|live|aqa|learnlm|computer-use|robotics|deep-research|gemma|omni|customtools)/i;

/** From the /v1beta/models list, the newest multimodal "pro" and "flash" that support generateContent; stable before preview. */
export function pickModels(models) {
  const usable = (models ?? [])
    .filter((m) => (m.supportedGenerationMethods ?? m.supported_generation_methods ?? []).includes('generateContent'))
    .map((m) => ({ ...m, id: String(m.name ?? '').replace(/^models\//, '') }))
    .filter((m) => /^gemini-/.test(m.id) && !EXCLUDE.test(m.id) && !/-lite/.test(m.id) && !/-\d{3}$/.test(m.id) && !/-\d{2}-\d{2}$/.test(m.id) && !/-8b/.test(m.id));
  const version = (id) => { const m = id.match(/gemini-(\d+)(?:\.(\d+))?/); return m ? Number(m[1]) + Number(m[2] ?? 0) / 100 : 0; };
  const rank = (a, b) => {
    const dv = version(b.id) - version(a.id);
    if (dv) return dv;
    const pa = /preview|exp/.test(a.id) ? 1 : 0; const pb = /preview|exp/.test(b.id) ? 1 : 0;
    if (pa !== pb) return pa - pb;
    return a.id.length - b.id.length; // the plain alias before dated or suffixed variants
  };
  const pro = usable.filter((m) => /-pro/.test(m.id)).sort(rank)[0]?.id;
  const flash = usable.filter((m) => /-flash/.test(m.id)).sort(rank)[0]?.id;
  return { pro, flash, usable: usable.map((m) => m.id).sort() };
}

/**
 * USD per 1M tokens, paid tier, prompts under 200k tokens, text and image input; thinking tokens are billed as output.
 * Read from aistudio.google.com/docs/pricing on 2026-11-12. Announced rises: gemini-4-* to 4 / 20 on 2026-12-01, gemini-3.8-flash to 1.5 / 7.5 on 2027-01-01.
 */
export const PRICES = [
  [/^gemini-4-(carbon|argon)/, { in: 2.0, out: 10.0 }],
  [/^gemini-3\.8-flash/, { in: 0.75, out: 3.75 }],
  [/^gemini-3.*pro/, { in: 2.0, out: 12.0 }],
  [/^gemini-3.*flash-lite/, { in: 0.25, out: 1.5 }],
  [/^gemini-3.*flash/, { in: 0.5, out: 3.0 }],
  [/^gemini-2\.5-pro/, { in: 1.25, out: 10.0 }],
  [/^gemini-2\.5-flash-lite/, { in: 0.1, out: 0.4 }],
  [/^gemini-2\.5-flash/, { in: 0.3, out: 2.5 }],
  [/^gemini-2\.0-flash/, { in: 0.1, out: 0.4 }],
];

export function estimateCost(model, usage) {
  const price = PRICES.find(([re]) => re.test(model))?.[1];
  if (!price) return undefined;
  const inTok = usage.prompt ?? 0;
  const outTok = (usage.output ?? 0) + (usage.thoughts ?? 0); // thinking tokens are billed as output
  return Math.round((inTok * price.in + outTok * price.out) / 1e6 * 1e5) / 1e5;
}

export function readUsage(resp) {
  const u = resp?.usageMetadata ?? {};
  return { prompt: u.promptTokenCount ?? 0, output: u.candidatesTokenCount ?? 0, thoughts: u.thoughtsTokenCount ?? 0, total: u.totalTokenCount ?? 0 };
}

// ---------------------------------------------------------------- images

export function mimeOf(path) {
  const e = extname(path).toLowerCase();
  return e === '.png' ? 'image/png' : e === '.jpg' || e === '.jpeg' ? 'image/jpeg' : e === '.webp' ? 'image/webp' : undefined;
}

export function imageSize(buf) {
  if (buf.length > 24 && buf.readUInt32BE(0) === 0x89504e47) return { width: buf.readUInt32BE(16), height: buf.readUInt32BE(20) };
  if (buf[0] === 0xff && buf[1] === 0xd8) {
    let i = 2;
    while (i + 9 < buf.length) {
      if (buf[i] !== 0xff) { i++; continue; }
      const m = buf[i + 1];
      if (m >= 0xc0 && m <= 0xc3) return { height: buf.readUInt16BE(i + 5), width: buf.readUInt16BE(i + 7) };
      i += 2 + buf.readUInt16BE(i + 2);
    }
  }
  return {};
}

/** Decodes an 8-bit RGB or RGBA non-interlaced PNG into {width, height, channels, pixels}. Enough for our own captures. */
export function decodePng(buf) {
  if (buf.readUInt32BE(0) !== 0x89504e47) throw new Error('not a PNG');
  let pos = 8; const idat = []; let width, height, depth, ctype, interlace;
  while (pos < buf.length) {
    const len = buf.readUInt32BE(pos); const type = buf.toString('ascii', pos + 4, pos + 8); const data = buf.subarray(pos + 8, pos + 8 + len);
    if (type === 'IHDR') { width = data.readUInt32BE(0); height = data.readUInt32BE(4); depth = data[8]; ctype = data[9]; interlace = data[12]; }
    else if (type === 'IDAT') idat.push(data);
    else if (type === 'IEND') break;
    pos += 12 + len;
  }
  if (depth !== 8 || (ctype !== 2 && ctype !== 6) || interlace !== 0) throw new Error(`PNG must be 8-bit RGB/RGBA non-interlaced (depth ${depth}, type ${ctype}, interlace ${interlace})`);
  const ch = ctype === 6 ? 4 : 3; const stride = width * ch;
  const raw = inflateSync(Buffer.concat(idat));
  const px = Buffer.alloc(width * height * ch);
  let prev = Buffer.alloc(stride);
  for (let y = 0; y < height; y++) {
    const f = raw[y * (stride + 1)]; const line = raw.subarray(y * (stride + 1) + 1, (y + 1) * (stride + 1)); const out = px.subarray(y * stride, (y + 1) * stride);
    for (let x = 0; x < stride; x++) {
      const a = x >= ch ? out[x - ch] : 0; const b = prev[x]; const c = x >= ch ? prev[x - ch] : 0; let v = line[x];
      if (f === 1) v += a; else if (f === 2) v += b; else if (f === 3) v += (a + b) >> 1;
      else if (f === 4) { const p = a + b - c; const pa = Math.abs(p - a), pb = Math.abs(p - b), pc = Math.abs(p - c); v += pa <= pb && pa <= pc ? a : pb <= pc ? b : c; }
      out[x] = v & 255;
    }
    prev = out;
  }
  return { width, height, channels: ch, pixels: px };
}

export function encodePng({ width, height, channels, pixels }) {
  const stride = width * channels; const raw = Buffer.alloc((stride + 1) * height);
  for (let y = 0; y < height; y++) { raw[y * (stride + 1)] = 0; pixels.copy(raw, y * (stride + 1) + 1, y * stride, (y + 1) * stride); }
  const crcTable = new Int32Array(256);
  for (let n = 0; n < 256; n++) { let c = n; for (let k = 0; k < 8; k++) c = c & 1 ? 0xedb88320 ^ (c >>> 1) : c >>> 1; crcTable[n] = c; }
  const crc = (b) => { let c = -1; for (const x of b) c = crcTable[(c ^ x) & 255] ^ (c >>> 8); return (c ^ -1) >>> 0; };
  const chunk = (type, data) => { const len = Buffer.alloc(4); len.writeUInt32BE(data.length); const td = Buffer.concat([Buffer.from(type, 'ascii'), data]); const c = Buffer.alloc(4); c.writeUInt32BE(crc(td)); return Buffer.concat([len, td, c]); };
  const ihdr = Buffer.alloc(13); ihdr.writeUInt32BE(width, 0); ihdr.writeUInt32BE(height, 4); ihdr[8] = 8; ihdr[9] = channels === 4 ? 6 : 2; ihdr[10] = 0; ihdr[11] = 0; ihdr[12] = 0;
  return Buffer.concat([Buffer.from([0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a]), chunk('IHDR', ihdr), chunk('IDAT', deflateSync(raw)), chunk('IEND', Buffer.alloc(0))]);
}

/** Box-filter downscale so the longest side is `target` px. */
export function downscale(img, target = 128) {
  const scale = Math.max(img.width, img.height) / target;
  if (scale <= 1) return img;
  const w = Math.max(1, Math.round(img.width / scale)); const h = Math.max(1, Math.round(img.height / scale)); const ch = img.channels;
  const out = Buffer.alloc(w * h * ch);
  for (let y = 0; y < h; y++) {
    const y0 = Math.floor(y * scale), y1 = Math.min(img.height, Math.max(y0 + 1, Math.floor((y + 1) * scale)));
    for (let x = 0; x < w; x++) {
      const x0 = Math.floor(x * scale), x1 = Math.min(img.width, Math.max(x0 + 1, Math.floor((x + 1) * scale)));
      const acc = new Array(ch).fill(0); let n = 0;
      for (let yy = y0; yy < y1; yy++) for (let xx = x0; xx < x1; xx++) { const o = (yy * img.width + xx) * ch; for (let c = 0; c < ch; c++) acc[c] += img.pixels[o + c]; n++; }
      const o = (y * w + x) * ch; for (let c = 0; c < ch; c++) out[o + c] = Math.round(acc[c] / n);
    }
  }
  return { width: w, height: h, channels: ch, pixels: out };
}

export function pngThumbnail(buf, target = 128) {
  const small = downscale(decodePng(buf), target);
  return { buffer: encodePng(small), width: small.width, height: small.height };
}

// ---------------------------------------------------------------- markdown

/** A generic, paste-able rendering of a validated answer: one bullet per key, nested lists for arrays of objects. */
export function renderMarkdown(answer) {
  return markdownLines(answer, '').join('\n');
}

function markdownLines(answer, indent) {
  const lines = [];
  const scalar = (v) => (typeof v === 'string' ? v : JSON.stringify(v));
  for (const [k, v] of Object.entries(answer ?? {})) {
    if (Array.isArray(v)) {
      if (v.every((x) => typeof x !== 'object')) { lines.push(`${indent}- ${k}: ${v.map(scalar).join('; ') || '(none)'}`); continue; }
      lines.push(`${indent}- ${k}:`);
      for (const item of v) {
        if (item && typeof item === 'object') {
          const flat = Object.entries(item).filter(([, x]) => typeof x !== 'object');
          lines.push(`${indent}  - ${flat.map(([a, b]) => `${a}: ${scalar(b)}`).join('; ')}`);
          for (const [a, b] of Object.entries(item).filter(([, x]) => x && typeof x === 'object')) lines.push(...markdownLines({ [a]: b }, indent + '    '));
        } else lines.push(`${indent}  - ${scalar(item)}`);
      }
    } else if (v && typeof v === 'object') {
      const flat = Object.entries(v).filter(([, x]) => typeof x !== 'object');
      if (flat.length === Object.keys(v).length && flat.length <= 2) lines.push(`${indent}- ${k}: ${flat.map(([a, b]) => `${a} ${scalar(b)}`).join(', ')}`);
      else { lines.push(`${indent}- ${k}:`); lines.push(...markdownLines(v, indent + '  ')); }
    } else lines.push(`${indent}- ${k}: ${scalar(v)}`);
  }
  return lines;
}

export function headerLine({ preset, model, stills, distance, usage, cost, ms, retried }) {
  const names = stills.map((s) => `${s.label}=${basename(s.path)}`).join(' ');
  const c = cost === undefined ? 'cost n/a' : `~$${cost.toFixed(4)}`;
  return `**eyes/${preset}** ${names}${distance !== undefined ? ` at ${distance} m` : ''} (${model}, in ${usage.prompt} out ${usage.output}${usage.thoughts ? ` think ${usage.thoughts}` : ''} tokens, ${c}, ${Math.round(ms / 100) / 10} s${retried ? ', 1 retry' : ''})`;
}
