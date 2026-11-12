// Lane A19: the feel and balance proposals (docs/experiments/FEEL.md "Proposals" table, A3's sweep in docs/experiments/BALANCING.md) as review items.
//   node tools/review/feel-items.mjs --out <items.json> --intro <intro.txt>
import { readFileSync, writeFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..');

function section(md, heading) {
  const m = md.match(new RegExp(`^## ${heading}[^\\n]*\\n([\\s\\S]*?)(?=^## |$(?![\\s\\S]))`, 'm'));
  return m ? m[1] : '';
}
function tableRows(block) {
  const rows = block.split('\n').filter((l) => l.startsWith('|'));
  return rows.slice(2).map((l) => l.replace(/^\||\|$/g, '').split('|').map((c) => c.trim()));
}
const plain = (s) => s.replace(/`/g, '').replace(/\s+/g, ' ').trim();

const GROUPS = [
  [/puck\.drag|reach_m|rink/, 'Puck, rink and pace'],
  [/skater\./, 'Skating'],
  [/bBrake|camera|HUD/, 'Controls, camera and hints'],
];

export function feelItems(feelMd, balMd) {
  const props = tableRows(section(feelMd, 'Proposals'));
  const sweep = tableRows(section(balMd, 'Feel proposals'));
  const sweepFor = (key) => sweep.find((r) => r[0].replace(/`/g, '').startsWith(key.replace(/^game\./, '').replace(/`/g, '')));
  const items = [];
  for (const [rank, key, now, proposed, why, risk] of props) {
    const k = plain(key).replace('(with 1)', '(with the rink change)'), group = (GROUPS.find(([re]) => re.test(k)) ?? [0, 'Other'])[1];
    const lines = [`Now: ${plain(now)}`, `Proposed: ${plain(proposed)}`, `Why: ${plain(why)}`, `Risk: ${plain(risk)}`];
    const sw = sweepFor(k.split(' ')[0]);
    if (sw) lines.push(`A3 sweep, this change alone (win % rookie / pro / all_star, baseline 75 / 45 / 30 regular, 5 / 20 / 5 large): regular ${plain(sw[1])}, large ${plain(sw[4])}; human touches per minute at pro ${plain(sw[2])}, match seconds at pro ${plain(sw[3])}`);
    if (/puck\.drag/.test(k)) lines.push('Decided already: R-0011 shipped puck.drag 0.5 in every preset and re-targeted the AI levels (regular 90 / 45 / 25 % wins for rookie / pro / all_star). Pass to confirm, reject to revert to 0.');
    if (/rink/.test(k)) lines.push('Decided already: R-0010 made 90 x 45 the default. Final sweep at pro: 45 % wins, 32 s per goal, 15.0 touches/min (large 120 x 60: 45 %, 72 s per goal, 13.4 touches/min).');
    items.push({ id: `feel.${String(rank).padStart(2, '0')}`, group, text: `${k}: ${plain(now)} -> ${plain(proposed)}`, note: lines.join('\n') });
  }
  const all = sweep.find((r) => /all five/.test(r[0]));
  if (all) items.push({ id: 'feel.all', group: 'Puck, rink and pace', text: 'All five measured feel changes together', note: `A3 sweep (drag 0, scripted human): regular ${plain(all[1])} win % (rookie / pro / all_star), ${plain(all[2])} touches/min at pro, ${plain(all[3])} s per match at pro; large ${plain(all[4])} win %.\nBaseline for comparison: ${plain(sweep[0].slice(1).join(' ; '))}.\nR-0011 adopted only puck.drag 0.5; the other four stay open for this verdict.` });
  return items;
}

export function feelIntro() {
  return [
    '**What this is.** The proposals from the first-play QA (lane A10, docs/experiments/FEEL.md) with the numbers behind each, and the difficulty sweep from lane A3 (docs/experiments/BALANCING.md). Each item is one change to a game setting. Pass means "make this change" (or "keep it", where the note says it is already in); reject means "leave the setting as it is".',
    '**How to read the numbers.** Win percentages are the scripted human against the AI over 20 seeds, rookie / pro / all_star. The curve targets are 85 / 50 / 25. Touches per minute is how often the human gets the puck. Risk says whether bots and tests must be re-run after the change.',
    '**What is already decided.** The default rink is 90 x 45 (R-0010) and every preset has puck.drag 0.5 (R-0011). Those two are here so you can overrule them. The rest of the table is open.',
  ].join('\n\n');
}

if (process.argv[1] && /feel-items\.mjs$/.test(process.argv[1].replace(/\\/g, '/'))) {
  const a = process.argv.slice(2), arg = (k) => (a.includes(k) ? a[a.indexOf(k) + 1] : undefined);
  const out = arg('--out'), intro = arg('--intro');
  if (!out || !intro) { console.error('FAIL usage: feel-items.mjs --out items.json --intro intro.txt'); process.exit(2); }
  const items = feelItems(readFileSync(resolve(repo, 'docs/experiments/FEEL.md'), 'utf8'), readFileSync(resolve(repo, 'docs/experiments/BALANCING.md'), 'utf8'));
  writeFileSync(out, JSON.stringify(items, null, 1) + '\n');
  writeFileSync(intro, feelIntro() + '\n');
  console.log(`PASS ${items.length} feel items -> ${out}`);
}
