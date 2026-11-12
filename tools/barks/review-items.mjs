// Lane A20 (ruling R-0021 point 2): the bark bank as review items for tools/review/build.mjs, with the tag "rewritten" on every line lane A20 rewrote
// as rink talk and the old film-vocabulary text in its note, so the operator can filter the page to the rewrites and judge them against the original.
// Wraps lane A19's tools/review/barks-items.mjs (group = situation, subgroup = speaker + tone, tag "mild") without changing it.
//   node tools/barks/review-items.mjs --out items.json --intro intro.txt
//   node tools/review/build.mjs --in items.json --out docs/reviews/barks-2026-11-12.html --title "Barks Review" --intro intro.txt
import { existsSync, readFileSync, writeFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { barkItems, barkIntro } from '../review/barks-items.mjs';

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..');

// id -> the text the line had before R-0021, from theme-pairs.json (written by node tools/barks/theme.mjs apply); {} when there is none.
export function rewrittenBefore() {
  const p = resolve(here, 'out', 'theme-pairs.json');
  if (!existsSync(p)) return {};
  const out = {};
  for (const pair of JSON.parse(readFileSync(p, 'utf8')).pairs) out[pair.id] = pair.before;
  return out;
}

export function themedItems(bank, before = rewrittenBefore()) {
  return barkItems(bank).map((it) => {
    if (!(it.id in before)) return it;
    return { ...it, note: `was (film words, before R-0021): ${before[it.id]}`, tags: [...(it.tags ?? []), 'rewritten'] };
  });
}

export function themedIntro(bank, before = rewrittenBefore()) {
  const rewritten = bank.lines.filter((l) => l.id in before).length;
  const paragraphs = barkIntro(bank).split('\n\n');
  paragraphs.splice(2, 0,
    `**Hockey first (R-0021).** "We are pushing the theme too far, it should be hockey first theme, keeping director cut in the menu." ${rewritten} lines that leaned on film words (cut, take, director, script, scene, reel, blooper...) are now rink talk at the same register, same id and same mild budget; they are tagged rewritten and show the old text in their note, so search for rewritten to judge them. Your earlier decisions on the other lines still apply.`);
  return paragraphs.join('\n\n');
}

if (process.argv[1] && /review-items\.mjs$/.test(process.argv[1].replace(/\\/g, '/'))) {
  const a = process.argv.slice(2), arg = (k, d) => (a.includes(k) ? a[a.indexOf(k) + 1] : d);
  const bank = JSON.parse(readFileSync(resolve(repo, arg('--bank', 'Game/Content/Barks/barks.json')), 'utf8'));
  const out = arg('--out'), intro = arg('--intro');
  if (!out || !intro) { console.error('FAIL usage: review-items.mjs [--bank f] --out items.json --intro intro.txt'); process.exit(2); }
  const before = rewrittenBefore();
  const items = themedItems(bank, before);
  writeFileSync(out, JSON.stringify(items, null, 1) + '\n');
  writeFileSync(intro, themedIntro(bank, before) + '\n');
  console.log(`PASS ${items.length} bark items (${items.filter((i) => i.tags?.includes('rewritten')).length} rewritten, ${items.filter((i) => i.tags?.includes('mild')).length} mild) -> ${out}`);
}
