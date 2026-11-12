// Lane A19: the bark bank as review items, and the register note a reviewer needs at the top of the page.
//   node tools/review/barks-items.mjs [--bank Game/Content/Barks/barks.json] --out <items.json> --intro <intro.txt>
// group = situation, subgroup = "speaker, tone", tags = ["mild"] for a line with a mild word (R-0015).
import { readFileSync, writeFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { mildCount, MAX_CHARS, MILD_QUOTA } from '../barks/filter.mjs';

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..');

export function barkItems(bank) {
  return bank.lines.map((l) => {
    const it = { id: l.id, group: l.situation, subgroup: `${l.speaker}, ${l.tone}`, text: l.text };
    if (mildCount(l.text) > 0) it.tags = ['mild'];
    return it;
  });
}

export function barkIntro(bank) {
  const mild = bank.lines.filter((l) => mildCount(l.text) > 0).length;
  return [
    `**What this is.** ${bank.lines.length} trash-talk lines the game shows over a skater when something happens (a check, a goal, a stall). Opponents mock, complain, brag or, on the Raw cut, go savage; teammates encourage, complain or praise. Situations are seen from you, the human player. Listen to a line or read it, then pass or reject it; add a comment where a rewrite would help.`,
    `**The bar you asked for (R-0015).** "More offensive with the mockery, the current scripts are too safe and childish." So the register is cutting sarcasm about your skating, aim, speed, courage and decisions, the bored voice of a veteran, genuine annoyance from teammates. It is not a kids' show.`,
    `**What the filter already forbids.** Slurs of any kind, sexual content, threats of real harm, real people, brands, teams, bodies or identities, anything aimed at who you are rather than how you play, drugs and gambling, politics, religion, URLs, emoji. Lines are 6 to ${MAX_CHARS} characters. Things this game does not have (goalie, referee, crowd, coach, penalty) are out.`,
    `**Mild language.** Allowed sparingly: damn, hell, crap, and "ass" only in "kick your ass", never two in a line. ${mild} of ${bank.lines.length} lines carry a mild word (${((100 * mild) / bank.lines.length).toFixed(1)}%, quota ${(MILD_QUOTA * 100).toFixed(0)}%); they are tagged mild so you can find them. Mild lines are the usual rejects if the Teen bar worries you.`,
    `**What happens to your decisions.** Rejected lines are removed from the bank, comments are logged in the review file, the content filter runs again and the bank is rebuilt into the game. A group that drops under 12 lines gets topped up by the generator.`,
  ].join('\n\n');
}

if (process.argv[1] && /barks-items\.mjs$/.test(process.argv[1].replace(/\\/g, '/'))) {
  const a = process.argv.slice(2), arg = (k, d) => (a.includes(k) ? a[a.indexOf(k) + 1] : d);
  const bank = JSON.parse(readFileSync(resolve(repo, arg('--bank', 'Game/Content/Barks/barks.json')), 'utf8'));
  const out = arg('--out'), intro = arg('--intro');
  if (!out || !intro) { console.error('FAIL usage: barks-items.mjs [--bank f] --out items.json --intro intro.txt'); process.exit(2); }
  writeFileSync(out, JSON.stringify(barkItems(bank), null, 1) + '\n');
  writeFileSync(intro, barkIntro(bank) + '\n');
  console.log(`PASS ${bank.lines.length} bark items -> ${out}`);
}
