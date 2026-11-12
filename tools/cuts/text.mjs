#!/usr/bin/env node
// Lane A37, brief item 3: every cut's player-facing text (name, title, the sentence it answers, announce lines of its rule hooks;
// for season takes the title and the director's note) through tools/barks/filter.mjs: film words (R-0021 point 2: hard words
// are a rejection on the ice, soft words a statistic), mild words (R-0015), and the bank's content rules. Where a line is shown
// decides what counts: menu text (cut names, the sentence) may say "cut" (R-0021 point 3: the film framing lives in the menu);
// in-match text (announce lines on the scoreboard, the TAKE card's note) must be rink talk.
//
//   node tools/cuts/text.mjs [--out tools/cuts/out/text.md]
// Prints a markdown table and writes it; the one-line read of "does the sentence say what the cut does" is written by hand in
// docs/experiments/CUTS.md next to this table. Exit 0 when no in-match line fails, 1 otherwise.
import { readFileSync, writeFileSync, mkdirSync } from 'node:fs';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { filmWords, mildCount, checkLine } from '../barks/filter.mjs';

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..');
const cartDir = join(repo, 'Game', 'Content', 'Cartridges');

function arg(name, dflt) {
  const i = process.argv.indexOf(`--${name}`);
  return i > 0 && i + 1 < process.argv.length ? process.argv[i + 1] : dflt;
}

// Every player-facing line of a cartridge with where it is shown.
export function linesOf(cartridge, extra = {}) {
  const out = [];
  if (extra.name) out.push({ where: 'menu', field: 'name', text: extra.name });
  if (cartridge.title && cartridge.title !== extra.name) out.push({ where: 'menu', field: 'title', text: cartridge.title });
  if (cartridge.theme?.name && cartridge.theme.name !== extra.name) out.push({ where: 'menu', field: 'theme.name', text: cartridge.theme.name });
  if (extra.sentence) out.push({ where: 'menu', field: 'sentence', text: extra.sentence });
  if (extra.twist_text) out.push({ where: 'match card', field: 'twist_text', text: extra.twist_text });
  for (const h of cartridge.game?.rules?.hooks || []) {
    for (const a of h.then || []) if (a.do === 'announce' && a.text) out.push({ where: 'scoreboard', field: `hooks.${h.id}.announce`, text: a.text });
  }
  return out;
}

// One judged line: the filter's verdicts, with the menu allowance for the film framing.
export function judgeLine(l) {
  const film = filmWords(l.text);
  const reasons = checkLine(l.text);
  const mild = mildCount(l.text);
  const inMatch = l.where !== 'menu';
  // The bank's own rules (6..60 characters, no capitalised word mid-sentence) are for barks; on a title, a card note or a scoreboard
  // line they are notes. What fails: a banned word anywhere, two mild words in one line, and on the ice a hard film word
  // (R-0021 point 2); in the menu the film word is the theme (R-0021 point 3) and is only noted.
  const hard = reasons.filter((r) => /banned word|two mild|slur|URL|emoji|ASCII|newline/i.test(r));
  const fails = hard.length > 0 || (inMatch && film.hard.length > 0);
  const noted = reasons.length > 0 || film.hard.length > 0 || film.soft.length > 0;
  return { ...l, film_hard: film.hard, film_soft: film.soft, mild, reasons, verdict: fails ? 'FAIL' : noted ? 'note' : 'clean' };
}

export function collect() {
  const rows = [];
  const cache = JSON.parse(readFileSync(join(repo, 'Game', 'Content', 'Designer', 'cached-cuts.json'), 'utf8'));
  for (const c of cache.cuts) for (const l of linesOf(c.cartridge, { name: c.name, sentence: c.sentence })) rows.push(judgeLine({ cut: c.name, group: 'cuts', ...l }));
  const ladder = JSON.parse(readFileSync(join(cartDir, 'ladder', 'season.ladder.json'), 'utf8')).ladder.takes;
  for (const t of ladder) {
    for (const id of [t.cartridge, t.fallback].filter(Boolean)) {
      const cart = JSON.parse(readFileSync(join(cartDir, 'ladder', `${id}.cartridge.json`), 'utf8'));
      for (const l of linesOf(cart, { name: cart.title, twist_text: id === t.cartridge ? t.twist_text : undefined })) rows.push(judgeLine({ cut: id, group: 'ladder', ...l }));
    }
  }
  return rows;
}

export function table(rows) {
  const head = ['cut', 'where', 'field', 'text', 'film hard', 'film soft', 'mild', 'filter reasons', 'verdict'];
  const line = (c) => `| ${c.map((x) => String(x).replace(/\|/g, '\\|')).join(' | ')} |`;
  return [line(head), `|${head.map(() => '---').join('|')}|`, ...rows.map((r) => line([r.cut, r.where, r.field, r.text, r.film_hard.join(' '), r.film_soft.join(' '), r.mild, r.reasons.join('; '), r.verdict]))].join('\n');
}

if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  const rows = collect();
  const out = resolve(arg('out', join(here, 'out', 'text.md')));
  mkdirSync(dirname(out), { recursive: true });
  const md = table(rows);
  writeFileSync(out, `${md}\n`);
  console.log(md);
  const fails = rows.filter((r) => r.verdict === 'FAIL');
  const notes = rows.filter((r) => r.verdict === 'note');
  console.log(`${fails.length ? 'FAIL' : 'PASS'} text ${rows.length} lines: ${fails.length} fail on the ice, ${notes.length} carry film words (menu allowance or soft)`);
  console.log(JSON.stringify({ tool: 'thirdjam.cuts.text/1', verdict: fails.length ? 'fail' : 'pass', exit: fails.length ? 1 : 0, lines: rows.length, fails: fails.map((r) => `${r.cut}: ${r.field}`), notes: notes.length, out: out.split('\\').join('/') }));
  process.exit(fails.length ? 1 : 0);
}
