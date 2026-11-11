// Screenshots, status files, gates: the smaller inputs of the dev log.
import fs from 'node:fs';
import path from 'node:path';
import { parseShotStamp } from './time.mjs';

// ---------- screenshots: YYYYMMDD-HHMMSS-<A|B>-<label>.png ----------
export const SHOT_RE = /^(\d{8})-(\d{6})-([AB])-([A-Za-z0-9][A-Za-z0-9_\-]*)\.(png|jpe?g|webp)$/i;

const MIME = { png: 'image/png', jpg: 'image/jpeg', jpeg: 'image/jpeg', webp: 'image/webp' };

/** Width and height from a PNG header, or undefined. */
export function pngSize(buf) {
  if (buf.length < 24 || buf.readUInt32BE(0) !== 0x89504e47) return undefined;
  const w = buf.readUInt32BE(16);
  const h = buf.readUInt32BE(20);
  return w > 0 && h > 0 ? { w, h } : undefined;
}

export function captionFromLabel(label) {
  const s = label.replace(/[-_]+/g, ' ').trim();
  return s.charAt(0).toUpperCase() + s.slice(1);
}

/**
 * @returns {{shots:object[], warnings:string[]}}
 */
export function loadShots(dir, redact = (s) => s) {
  const shots = [];
  const warnings = [];
  if (!dir || !fs.existsSync(dir)) return { shots, warnings };
  for (const name of fs.readdirSync(dir).sort()) {
    const file = path.join(dir, name);
    if (!fs.statSync(file).isFile()) continue;
    if (/\.(txt|md)$/i.test(name) || name.startsWith('.')) continue;
    const m = name.match(SHOT_RE);
    if (!m) {
      warnings.push(`screenshot skipped, name does not match YYYYMMDD-HHMMSS-<A|B>-<label>.png: ${redact(name)}`);
      continue;
    }
    const ms = parseShotStamp(m[1], m[2]);
    if (ms === undefined) {
      warnings.push(`screenshot skipped, impossible date in name: ${redact(name)}`);
      continue;
    }
    const ext = m[5].toLowerCase();
    let caption = captionFromLabel(m[4]);
    const side = file.replace(/\.[^.]+$/, '.txt');
    if (fs.existsSync(side)) {
      const t = fs.readFileSync(side, 'utf8').trim();
      if (t) caption = t.replace(/\s+/g, ' ');
    }
    let size;
    if (ext === 'png') size = pngSize(fs.readFileSync(file).subarray(0, 32));
    shots.push({ file, name, ms, device: m[3].toUpperCase(), label: m[4], caption: redact(caption), mime: MIME[ext], size, bytes: fs.statSync(file).size });
  }
  shots.sort((a, b) => a.ms - b.ms || a.name.localeCompare(b.name));
  return { shots, warnings };
}

// ---------- status files: status/A.md and status/B.md, blocks "## H12 ..." ----------
/**
 * @returns {{entries:{device:string, hour:number, heading:string, fields:{key:string,value:string}[]}[], warnings:string[]}}
 */
export function loadStatus(dir, redact = (s) => s) {
  const entries = [];
  const warnings = [];
  if (!dir || !fs.existsSync(dir)) return { entries, warnings };
  for (const name of fs.readdirSync(dir).sort()) {
    const m = name.match(/^([AB])\.md$/i);
    if (!m) continue; // TEMPLATE.md, README.md and anything else is not a status file
    const device = m[1].toUpperCase();
    const text = fs.readFileSync(path.join(dir, name), 'utf8').replace(/\r\n?/g, '\n');
    let cur = null;
    let count = 0;
    for (const line of text.split('\n')) {
      const h = line.match(/^##\s+H(\d+(?:\.\d+)?)\b[\s:.\-–—]*(.*)$/i);
      if (h) {
        cur = { device, hour: parseFloat(h[1]), heading: redact(h[2].trim()), fields: [] };
        entries.push(cur);
        count++;
        continue;
      }
      if (!cur) continue;
      const f = line.match(/^\s*[-*]\s*([A-Za-z][A-Za-z0-9 _\-]*?)\s*:\s*(.*)$/);
      if (f) cur.fields.push({ key: redact(f[1].trim().toLowerCase()), value: redact(f[2].trim()) });
      else if (/^\s{2,}\S/.test(line) && cur.fields.length) cur.fields[cur.fields.length - 1].value += ' ' + redact(line.trim());
    }
    if (!count) warnings.push(`status file ${name} has no "## H<hour>" blocks`);
  }
  entries.sort((a, b) => a.hour - b.hour || a.device.localeCompare(b.device));
  return { entries, warnings };
}

// ---------- gates: docs/GATES.md headings "## Gate 1 (H6): core loop playable" ----------
export function parseGates(text) {
  const gates = [];
  for (const line of String(text).replace(/\r\n?/g, '\n').split('\n')) {
    const m = line.match(/^##\s+(.+?)\s+\(H(\d+(?:\.\d+)?)\)\s*(?::\s*(.*))?$/);
    if (m) gates.push({ name: m[1].trim(), hour: parseFloat(m[2]), sub: (m[3] || '').trim() });
  }
  return gates.sort((a, b) => a.hour - b.hour);
}

export function loadGates(file) {
  if (!file || !fs.existsSync(file)) return [];
  return parseGates(fs.readFileSync(file, 'utf8'));
}
