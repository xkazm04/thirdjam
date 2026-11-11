// A small markdown subset for the notes: paragraphs, lists (bulleted and numbered), fenced code,
// **bold**, *italic*, `code`, [links](http...). Everything else is plain text. Output is HTML;
// every text node is escaped and passed through the redactor first.

export const escapeHtml = (s) =>
  String(s).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;');

/** Curly quotes and a real ellipsis for prose (never applied inside code). */
export function typography(s) {
  return s
    .replace(/\.\.\./g, '…')
    .replace(/(^|[\s(\[{—–])"/g, '$1“')
    .replace(/"/g, '”')
    .replace(/(^|[\s(\[{—–])'/g, '$1‘')
    .replace(/'/g, '’');
}

/** Inline markup to HTML. `redact` is applied to the raw text before anything else. */
export function inline(raw, redact = (s) => s) {
  const text = redact(raw);
  // split out code spans first so nothing inside them is touched
  const parts = text.split(/(`[^`\n]+`)/g);
  return parts
    .map((part, i) => {
      if (i % 2 === 1) return `<code>${escapeHtml(part.slice(1, -1))}</code>`;
      let h = escapeHtml(typography(part));
      h = h.replace(/\[([^\]\n]+)\]\(((?:https?:\/\/|\.{0,2}\/|#)[^\s)]*)\)/g, (_, label, url) => {
        const external = /^https?:/.test(url);
        return `<a href="${url}"${external ? ' rel="noopener noreferrer"' : ''}>${label}</a>`;
      });
      h = h.replace(/\*\*([^*\n]+)\*\*/g, '<strong>$1</strong>');
      h = h.replace(/(^|[^*\w])\*([^*\s][^*\n]*?)\*(?![*\w])/g, '$1<em>$2</em>');
      return h;
    })
    .join('');
}

/** Block markdown to HTML. */
export function markdownToHtml(src, redact = (s) => s) {
  const lines = String(src).replace(/\r\n?/g, '\n').split('\n');
  const out = [];
  let para = [];
  let list = null; // {tag, items:[]}
  let fence = null; // {lang, lines:[]}

  const flushPara = () => {
    if (para.length) out.push(`<p>${inline(para.join(' '), redact)}</p>`);
    para = [];
  };
  const flushList = () => {
    if (list) {
      out.push(`<${list.tag}>${list.items.map((t) => `<li>${inline(t, redact)}</li>`).join('')}</${list.tag}>`);
    }
    list = null;
  };

  for (const line of lines) {
    if (fence) {
      if (/^\s*```/.test(line)) {
        out.push(`<pre><code>${escapeHtml(redact(fence.lines.join('\n')))}</code></pre>`);
        fence = null;
      } else fence.lines.push(line);
      continue;
    }
    if (/^\s*```/.test(line)) {
      flushPara();
      flushList();
      fence = { lines: [] };
      continue;
    }
    if (/^\s*$/.test(line)) {
      flushPara();
      flushList();
      continue;
    }
    const bullet = line.match(/^\s*[-*+]\s+(.*)$/);
    const numbered = line.match(/^\s*\d+[.)]\s+(.*)$/);
    if (bullet || numbered) {
      flushPara();
      const tag = bullet ? 'ul' : 'ol';
      if (list && list.tag !== tag) flushList();
      if (!list) list = { tag, items: [] };
      list.items.push((bullet || numbered)[1]);
      continue;
    }
    // continuation of a list item (indented), else a paragraph line
    if (list && /^\s{2,}\S/.test(line)) {
      list.items[list.items.length - 1] += ' ' + line.trim();
      continue;
    }
    flushList();
    para.push(line.trim());
  }
  if (fence) out.push(`<pre><code>${escapeHtml(redact(fence.lines.join('\n')))}</code></pre>`);
  flushPara();
  flushList();
  return out.join('\n');
}

/**
 * Split a notes file into `## H12 title` blocks.
 * @returns {{hour:number, title:string, body:string}[]} sorted by hour then file order
 */
export function parseNotes(src) {
  const blocks = [];
  let cur = null;
  for (const line of String(src).replace(/\r\n?/g, '\n').split('\n')) {
    const m = line.match(/^##\s+H(\d+(?:\.\d+)?)\b[\s:.\-–—]*(.*)$/i);
    if (m) {
      cur = { hour: parseFloat(m[1]), title: m[2].trim(), body: [], order: blocks.length };
      blocks.push(cur);
    } else if (cur) cur.body.push(line);
  }
  return blocks
    .map((b) => ({ hour: b.hour, title: b.title, body: b.body.join('\n').trim(), order: b.order }))
    .sort((a, b) => a.hour - b.hour || a.order - b.order);
}
