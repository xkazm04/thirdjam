// Redaction pass. Every string that reaches the page goes through createRedactor().redact.
// The dev log is published: no secrets, no email addresses, no absolute paths under a home directory.
//
// Order matters: secrets first (a key inside a path must not survive as a path component), then
// emails, then home paths.
import os from 'node:os';

const RULES = [
  // [kind, regex, replacement]. Each regex must be global.
  ['key', /\bsk-ant-[A-Za-z0-9_\-]{6,}/g, '[redacted-key]'],
  ['key', /\bsk-(?:proj-|live-|test-)?[A-Za-z0-9_\-]{20,}/g, '[redacted-key]'],
  ['key', /\bgithub_pat_[A-Za-z0-9_]{20,}/g, '[redacted-key]'],
  ['key', /\bgh[pousr]_[A-Za-z0-9]{20,}/g, '[redacted-key]'],
  ['key', /\bglpat-[A-Za-z0-9_\-]{16,}/g, '[redacted-key]'],
  ['key', /\bxox[abprs]-[A-Za-z0-9\-]{10,}/g, '[redacted-key]'],
  ['key', /\bAKIA[0-9A-Z]{16}\b/g, '[redacted-key]'],
  ['key', /\bAIza[0-9A-Za-z_\-]{30,}/g, '[redacted-key]'],
  ['key', /\bhf_[A-Za-z0-9]{30,}/g, '[redacted-key]'],
  ['key', /\bnpm_[A-Za-z0-9]{30,}/g, '[redacted-key]'],
  ['key', /\bey[A-Za-z0-9_\-]{10,}\.ey[A-Za-z0-9_\-]{10,}\.[A-Za-z0-9_\-]{10,}/g, '[redacted-key]'],
  ['key', /-----BEGIN [A-Z ]*PRIVATE KEY-----[\s\S]*?(?:-----END [A-Z ]*PRIVATE KEY-----|$)/g, '[redacted-key]'],
  ['key', /\b(Bearer|Basic)\s+[A-Za-z0-9._~+\/\-]{16,}=*/gi, '$1 [redacted-key]'],
  // KEY=value / "key": "value" assignments with telling names.
  [
    'key',
    /(\b[A-Za-z0-9_]*(?:api[_-]?key|secret|password|passwd|access[_-]?token|auth[_-]?token|client[_-]?secret|private[_-]?key)[A-Za-z0-9_]*["']?\s*[:=]\s*["']?)(?!\[redacted)[^\s"',;)}\]]{6,}/gi,
    '$1[redacted-key]',
  ],
  ['email', /[A-Za-z0-9._%+\-]+@[A-Za-z0-9\-]+(?:\.[A-Za-z0-9\-]+)*\.[A-Za-z]{2,}/g, '[email]'],
  // C:\Users\name, C:/Users/name, C:\\Users\\name (JSON-escaped), /c/Users/name, /Users/name, /home/name
  ['path', /[A-Za-z]:[\\/]+Users[\\/]+[^\\/\s"'<>|:*?]+/gi, '~'],
  ['path', /\/[a-z]\/Users\/[^\\/\s"'<>|:*?]+/gi, '~'],
  ['path', /(?<![A-Za-z0-9_.\-])\/(?:Users|home)\/[^\\/\s"'<>|:*?]+/g, '~'],
  // Claude Code project slugs: C--Users-name-kiro-project
  ['path', /\b[A-Za-z]--Users-[A-Za-z0-9_.]+/g, '~'],
];

/** The home directory of this machine, in the spellings a path can take. */
function homeSpellings() {
  const out = new Set();
  const add = (s) => {
    if (s && s.length > 3) out.add(s);
  };
  for (const raw of [os.homedir(), process.env.USERPROFILE, process.env.HOME]) {
    if (!raw) continue;
    add(raw);
    add(raw.replace(/\\/g, '/'));
    add(raw.replace(/\\/g, '\\\\'));
    const m = raw.match(/^([A-Za-z]):[\\/](.*)$/);
    if (m) add(`/${m[1].toLowerCase()}/${m[2].replace(/\\/g, '/')}`);
  }
  return [...out].sort((a, b) => b.length - a.length);
}

const escapeRe = (s) => s.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');

/**
 * @returns {{redact:(s:any)=>any, stats:{key:number,email:number,path:number}, scan:(s:string)=>string[]}}
 */
export function createRedactor() {
  const stats = { key: 0, email: 0, path: 0 };
  const homes = homeSpellings().map((h) => new RegExp(escapeRe(h), 'gi'));

  function redact(value) {
    if (typeof value !== 'string' || value === '') return value;
    let s = value;
    let homesDone = false;
    for (const [kind, re, rep] of RULES) {
      if (kind === 'path' && !homesDone) {
        // before the generic path rules, take this machine's own home in all its spellings
        homesDone = true;
        for (const h of homes) {
          s = s.replace(h, () => {
            stats.path++;
            return '~';
          });
        }
      }
      s = s.replace(re, (...args) => {
        stats[kind]++;
        // honour $1 in the replacement
        return rep.replace(/\$(\d)/g, (_, i) => args[Number(i)] ?? '');
      });
    }
    return s;
  }

  /** Report what a redact pass would still change (used to verify finished pages). */
  function scan(text) {
    const probe = createRedactor();
    const clean = probe.redact(text);
    return clean === text ? [] : ['unredacted content found'];
  }

  return { redact, stats, scan };
}

/** Redact a whole HTML document while leaving embedded image data untouched. */
export function redactHtml(html, redactor) {
  const blobs = [];
  const masked = html.replace(/data:image\/[a-z+.\-]+;base64,[A-Za-z0-9+\/=]+/g, (m) => {
    blobs.push(m);
    return `\u0000BLOB${blobs.length - 1}\u0000`;
  });
  const clean = redactor.redact(masked);
  return clean.replace(/\u0000BLOB(\d+)\u0000/g, (_, i) => blobs[Number(i)]);
}

/** Recursively redact every string in plain data. */
export function redactDeep(value, redact) {
  if (typeof value === 'string') return redact(value);
  if (Array.isArray(value)) return value.map((v) => redactDeep(v, redact));
  if (value && typeof value === 'object') {
    const o = {};
    for (const [k, v] of Object.entries(value)) o[k] = redactDeep(v, redact);
    return o;
  }
  return value;
}
