// Lane A19 (ruling R-0020 point 3): turn a list of items that need a human verdict into one self-contained review page.
//   node tools/review/build.mjs --in items.json --out page.html --title "Bark Bank Review" [--intro note.txt] [--description "..."] [--no-listen] [--listen-note]
// items.json: [{ id, group, subgroup?, text, note?, tags?: [] }]  (unknown keys are errors; absent value = omit the key)
// The page has no external requests and no dependencies; decisions export as JSON and as a markdown summary (see README.md).
import { readFileSync, writeFileSync, mkdirSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const here = dirname(fileURLToPath(import.meta.url));
export const MAX_BYTES = 3 * 1024 * 1024;
const KEYS = new Set(['id', 'group', 'subgroup', 'text', 'note', 'tags']);

const esc = (s) => String(s).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;');

export function validateItems(items) {
  const errors = [];
  if (!Array.isArray(items) || !items.length) return ['items must be a non-empty array'];
  const seen = new Set();
  items.forEach((it, i) => {
    const at = `item ${i}${it && it.id ? ` (${it.id})` : ''}`;
    if (!it || typeof it !== 'object' || Array.isArray(it)) { errors.push(`${at}: not an object`); return; }
    for (const k of Object.keys(it)) if (!KEYS.has(k)) errors.push(`${at}: unknown key "${k}"`);
    for (const k of ['id', 'group', 'text']) if (typeof it[k] !== 'string' || !it[k].trim()) errors.push(`${at}: "${k}" must be a non-empty string`);
    for (const k of ['subgroup', 'note']) if (k in it && (typeof it[k] !== 'string' || !it[k].trim())) errors.push(`${at}: "${k}" must be a non-empty string or omitted`);
    if ('tags' in it && (!Array.isArray(it.tags) || !it.tags.length || it.tags.some((t) => typeof t !== 'string' || !t))) errors.push(`${at}: "tags" must be a non-empty array of strings or omitted`);
    if (typeof it.id === 'string') { if (seen.has(it.id)) errors.push(`${at}: duplicate id`); seen.add(it.id); }
  });
  return errors;
}

// Plain-text note to HTML: blank-line separated paragraphs, "- " lines become a list, **x** is bold. Everything else is escaped.
export function introHtml(text) {
  if (!text || !text.trim()) return '';
  const inline = (s) => esc(s).replace(/\*\*(.+?)\*\*/g, '<strong>$1</strong>');
  const blocks = text.replace(/\r\n/g, '\n').trim().split(/\n\s*\n/).map((b) => {
    const lines = b.split('\n');
    if (lines.every((l) => /^\s*- /.test(l))) return `<ul>${lines.map((l) => `<li>${inline(l.replace(/^\s*- /, ''))}</li>`).join('')}</ul>`;
    return `<p>${inline(lines.join(' '))}</p>`;
  });
  return `<section class="intro" aria-label="About this review">${blocks.join('')}</section>`;
}

export function buildPage(items, { title, intro = '', description, listen = true, listenNote = false }) {
  const errors = validateItems(items);
  if (errors.length) throw new Error(`invalid items:\n  ${errors.slice(0, 20).join('\n  ')}${errors.length > 20 ? `\n  ... and ${errors.length - 20} more` : ''}`);
  if (!title || !title.trim()) throw new Error('--title is required');
  // Items keep their input order; the page groups consecutive and later items of the same group together in first-seen order.
  const BS = String.fromCharCode(92); // a backslash, so the escapes below land in the page as text
  const data = JSON.stringify({ title, listen, listenNote, items }).replace(/</g, `${BS}u003c`)
    .split(String.fromCharCode(0x2028)).join(`${BS}u2028`).split(String.fromCharCode(0x2029)).join(`${BS}u2029`);
  const tpl = readFileSync(resolve(here, 'template.html'), 'utf8');
  const desc = description ?? `Review ${items.length} items: pass, reject or comment on each, then paste the exported decisions back.`;
  const fill = { TITLE: esc(title), DESCRIPTION: esc(desc), INTRO: introHtml(intro), DATA: data };
  const html = tpl.replace(/\{\{(TITLE|DESCRIPTION|INTRO|DATA)\}\}/g, (_, k) => fill[k]);
  const bytes = Buffer.byteLength(html);
  if (bytes > MAX_BYTES) throw new Error(`page is ${bytes} bytes, over the ${MAX_BYTES} limit; split the list`);
  return { html, bytes };
}

function parseArgs(argv) {
  const a = { listen: true, listenNote: false };
  for (let i = 0; i < argv.length; i += 1) {
    const k = argv[i];
    if (k === '--no-listen') a.listen = false;
    else if (k === '--listen-note') a.listenNote = true;
    else if (['--in', '--out', '--title', '--intro', '--description'].includes(k)) a[k.slice(2)] = argv[++i];
    else throw new Error(`unknown argument ${k}`);
  }
  return a;
}

if (process.argv[1] && /build\.mjs$/.test(process.argv[1].replace(/\\/g, '/'))) {
  try {
    const a = parseArgs(process.argv.slice(2));
    if (!a.in || !a.out || !a.title) throw new Error('usage: build.mjs --in <items.json> --out <page.html> --title "<2-4 words>" [--intro <note.txt>] [--description "..."] [--no-listen] [--listen-note]');
    const words = a.title.trim().split(/\s+/).length;
    if (words < 2 || words > 4) console.warn(`WARN title "${a.title}" has ${words} words; the artifact contract asks for 2-4`);
    const items = JSON.parse(readFileSync(a.in, 'utf8'));
    const intro = a.intro ? readFileSync(a.intro, 'utf8') : '';
    const { html, bytes } = buildPage(items, { title: a.title, intro, description: a.description, listen: a.listen, listenNote: a.listenNote });
    mkdirSync(dirname(resolve(a.out)), { recursive: true });
    writeFileSync(a.out, html);
    const groups = new Set(items.map((i) => i.group)).size;
    console.log(`PASS review page ${a.out}: ${items.length} items in ${groups} groups, ${(bytes / 1024).toFixed(0)} KB`);
    console.log(JSON.stringify({ tool: 'thirdjam.review.build/1', ok: true, out: a.out, items: items.length, groups, bytes }));
  } catch (e) {
    console.error(`FAIL ${e.message}`);
    process.exit(2);
  }
}
