// Lane A20 (ruling R-0021 point 2): the bark bank is hockey first. The Director's Cut film words stay in the menu and the Designer; on the ice the
// skaters talk rink. This tool finds every line that leans on film vocabulary and applies the hand-written rewrites (same id, situation, speaker,
// tone, register, length band and mild budget).
//   node tools/barks/theme.mjs list [--before]  # writes tools/barks/out/theme-hits.json: every hit, the words it matched, rewrite or keep (with the reason); --before restores the pre-rewrite texts from out/theme-pairs.json
//   node tools/barks/theme.mjs apply    # applies tools/barks/out/theme-rewrites.json to barks.json, checks every new line with filter.mjs,
//                                       # rewrites Game/Content/Barks/REVIEW.md in the generator's format and embeds (BarkData.inl)
// Words in the hit list beyond R-0021's are the stems the first pass missed (cast list, stunt double, edit, deleted scenes, movies, acting, picture).
import { existsSync, readFileSync, writeFileSync, mkdirSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { checkLine, filterBank, filmWords, mildCount, stats } from './filter.mjs';
import { embed } from './embed.mjs';

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..');
const bankPath = resolve(repo, 'Game/Content/Barks/barks.json');
const reviewPath = resolve(repo, 'Game/Content/Barks/REVIEW.md');
const outDir = resolve(here, 'out');
const hitsPath = resolve(outDir, 'theme-hits.json');
const rewritesPath = resolve(outDir, 'theme-rewrites.json');

// R-0021's list plus the stems the first pass missed. Whole words, any case.
export const FILM_WORDS = ['cut', 'cuts', 'take', 'takes', 'director', "director's", 'directors', 'script', 'scene', 'scenes', 'reel', 'credits', 'reshoot', 'rehearsal',
  'rehearse', 'frame', 'footnote', 'headline', 'makeup', 'blooper', 'editor', 'edit', 'prop', 'props', 'camera', 'clip', 'sequel', 'draft', 'tape', 'rewind', 'movie',
  'movies', 'screen test', 'wrap', 'rewrite', 'plot', 'slate', 'studio', 'extra', 'extras', 'final cut', 'filming', 'actor', 'acting', 'lead', 'cast list', 'stunt double',
  'picture', 'rave review'];
const RE = new RegExp(`(^|[^a-z'])(${FILM_WORDS.map((w) => w.replace(/[.*+?^${}()|[\]\\]/g, '\\$&')).join('|')})(?=$|[^a-z'])`, 'gi');

// Hits whose film word is also hockey or plain English: they stay, with the reason the reviewer reads.
export const KEEP = {
  'check_landed.opponent.bravado.13': '"take your hit" is hockey',
  'check_received.opponent.bravado.02': '"every shift you take" is hockey',
  'check_received.opponent.bravado.05': '"take notes" is plain English',
  'check_received.teammate.complain.07': '"lead with your face" is plain English',
  'check_received.teammate.praise.02': '"take that (hit)" is hockey',
  'directors_call.opponent.bravado.12': '"kept the lead" is the score',
  'directors_call.teammate.encourage.07': '"lead the play" is hockey',
  'faceoff.opponent.mock.11': '"take the faceoff" is hockey',
  'goal_against.opponent.mock.08': '"take your time" is plain English',
  'goal_for.opponent.bravado.03': '"the lead" is the score',
  'goal_for.opponent.bravado.07': '"take your moment, take the period" is plain English and hockey',
  'goal_for.opponent.bravado.09': '"take the rest" is plain English',
  'goal_for.teammate.encourage.03': '"frame it" (hang it on the wall) is plain English',
  'goal_for.teammate.praise.07': '"I take it back" is plain English',
  'lose.opponent.bravado.06': '"take it as a lesson" is plain English',
  'lose.opponent.savage.10': '"take the points" is hockey',
  'lose.teammate.complain.01': '"the ice takes the blame" is plain English',
  'near_miss.opponent.bravado.04': '"the lead" is the score',
  'near_miss.opponent.bravado.19': '"take the net" is hockey',
  'near_miss.opponent.mock.03': '"the frame" is the net frame',
  'near_miss.teammate.encourage.16': '"take the hint" is plain English',
  'stall.opponent.bravado.03': '"take it from you" is hockey',
  'stall.opponent.bravado.05': '"take your time, our lead" is plain English and the score',
  'stall.opponent.mock.10': '"take your time" is plain English',
  'stall.opponent.savage.08': '"take the puck" is hockey',
  'stall.teammate.encourage.10': '"take the corner" is hockey',
  'steal.opponent.bravado.10': '"we take it" is hockey',
  'steal.opponent.mock.08': '"take the puck back" is hockey',
  'steal.opponent.savage.03': '"every touch you take" is hockey',
  'steal.teammate.encourage.02': '"take it back" is hockey',
  'win.opponent.mock.05': '"take the win" is plain English',
  'win.opponent.mock.12': '"take that win home" is plain English',
  'win.opponent.savage.03': '"lead the scoreboard" is the score',
  'win.opponent.savage.09': '"take the win" is plain English',
  'win.teammate.encourage.13': '"take corners" is hockey',
  'win.teammate.encourage.17': '"something extra" is plain English',
  'win.teammate.praise.16': '"I\'ll take it" is plain English',
};

export function findHits(bank) {
  const hits = [];
  for (const l of bank.lines) {
    const words = [...new Set([...l.text.matchAll(RE)].map((m) => m[2].toLowerCase()))];
    if (!words.length) continue;
    const h = { id: l.id, words, text: l.text, action: KEEP[l.id] ? 'keep' : 'rewrite' };
    if (KEEP[l.id]) h.reason = KEEP[l.id];
    if (mildCount(l.text) > 0) h.mild = true;
    hits.push(h);
  }
  return hits;
}

export function mildPerGroup(lines) {
  const g = {};
  for (const l of lines) { const k = `${l.situation}.${l.speaker}.${l.tone}`; g[k] = g[k] ?? { lines: 0, mild: 0 }; g[k].lines += 1; if (mildCount(l.text) > 0) g[k].mild += 1; }
  return g;
}

// list: the hits in the committed bank; list --before: the hits as they were before the rewrites (the texts restored from out/theme-pairs.json).
function list(before = false) {
  const bank = JSON.parse(readFileSync(bankPath, 'utf8'));
  if (before) {
    const pairsPath = resolve(outDir, 'theme-pairs.json');
    if (!existsSync(pairsPath)) { console.log(`FAIL --before needs ${pairsPath}`); process.exit(2); }
    const old = new Map(JSON.parse(readFileSync(pairsPath, 'utf8')).pairs.map((p) => [p.id, p.before]));
    for (const l of bank.lines) if (old.has(l.id)) l.text = old.get(l.id);
  }
  const hits = findHits(bank);
  const rewrite = hits.filter((h) => h.action === 'rewrite').length;
  mkdirSync(outDir, { recursive: true });
  writeFileSync(hitsPath, JSON.stringify({ schema: 'hokej.barks.theme-hits/1', ruling: 'R-0021 point 2', generated: new Date().toISOString(), words: FILM_WORDS,
    lines: bank.lines.length, hits: hits.length, rewrite, keep: hits.length - rewrite, items: hits }, null, 1) + '\n');
  console.log(`PASS theme list: ${hits.length} hits of ${bank.lines.length} lines, ${rewrite} to rewrite, ${hits.length - rewrite} kept -> ${hitsPath}`);
  return hits;
}

export function reviewMarkdown(bank, { rewritten, mildTotal, softLines }) {
  let md = `# Bark bank review (R-0015 register, hockey first per R-0021 point 2, model ${bank.model}, generated ${bank.generated})\n\n` +
    'Skim every line; delete any you dislike from barks.json, then run node tools/barks/embed.mjs and node --test tools/barks/*.test.mjs.\n' +
    'Two sonnet runs (26 + 6 calls, $0.36): 1,582 raw lines; the filter rejected the hard list (25 + 4), over-length (28), duplicates (12),\n' +
    'one "ass" outside a kick-your-ass phrase and four off-world words (goalie, rent...), then capped every combination at 20 and the mild words at one in ten.\n' +
    `Lane A20 (R-0021 point 2, ${new Date().toISOString().slice(0, 16).replace('T', ' ')}): ${rewritten.size} lines that leaned on film vocabulary (cut, take, director, script, scene, reel...) were rewritten by hand as rink talk,\n` +
    'same id, situation, speaker, tone, register and mild budget; they are marked (rewritten) below. The filter now rejects director, cut, scene, script, reel, take two, take three.\n' +
    `Shipped: ${bank.lines.length} lines, ${mildTotal} with a mild word (damn, hell, crap, kick your ass: ${((100 * mildTotal) / bank.lines.length).toFixed(1)}%, quota 10%), ` +
    `${Object.keys(bank.counts).length} combinations of 12..20 lines, ${softLines} lines with a soft film word in its hockey sense (take the puck, the lead, the frame of the net).\n` +
    'Tones: opponent mock / savage / complain / bravado, teammate encourage / complain / praise. Intensity: quiet = complain (opponents) and encourage/praise/complain (teammates),\n' +
    'normal = mock and the rest, loud = savage allowed. Regular ships normal, Raw ships loud.\n';
  let group = '';
  for (const l of bank.lines) {
    const g = `${l.situation} / ${l.speaker} / ${l.tone}`;
    if (g !== group) { group = g; md += `\n## ${g} (${bank.counts[`${l.situation}.${l.speaker}.${l.tone}`]})\n`; }
    md += `- ${l.text}${rewritten.has(l.id) ? ' (rewritten)' : ''}\n`;
  }
  return md;
}

function apply() {
  const bank = JSON.parse(readFileSync(bankPath, 'utf8'));
  if (!existsSync(rewritesPath)) { console.log(`FAIL no ${rewritesPath}`); process.exit(2); }
  const rewrites = JSON.parse(readFileSync(rewritesPath, 'utf8'));
  const byId = new Map(bank.lines.map((l) => [l.id, l]));
  const before = mildPerGroup(bank.lines);
  const problems = [];
  const pairs = [];
  for (const [id, text] of Object.entries(rewrites.lines)) {
    const l = byId.get(id);
    if (!l) { problems.push(`${id}: no such line`); continue; }
    if (typeof text !== 'string') { problems.push(`${id}: text is not a string`); continue; }
    const r = checkLine(text);
    if (r.length) problems.push(`${id} ${JSON.stringify(text)}: ${r.join('; ')}`);
    const f = filmWords(text);
    if (f.soft.length) problems.push(`${id} ${JSON.stringify(text)}: soft film word ${f.soft.join(', ')}; write it as hockey`);
    if (mildCount(text) > mildCount(l.text)) problems.push(`${id}: a new mild word (the line had ${mildCount(l.text)})`);
    pairs.push({ id, before: l.text, after: text });
    l.text = text;
  }
  const { rejected } = filterBank(bank.lines);
  for (const r of rejected) problems.push(`${r.id} ${JSON.stringify(r.text)}: ${r.reasons.join('; ')}`);
  const stillFilm = findHits(bank).filter((h) => h.action === 'rewrite');
  for (const h of stillFilm) problems.push(`${h.id} ${JSON.stringify(h.text)}: still leans on ${h.words.join(', ')}`);
  if (problems.length) { for (const p of problems) console.log(`FAIL ${p}`); console.log(`FAIL theme apply: ${problems.length} problems, nothing written`); process.exit(1); }
  const after = mildPerGroup(bank.lines);
  for (const k of Object.keys(before)) if (before[k].mild !== after[k].mild) { console.log(`FAIL mild count moved in ${k}: ${before[k].mild} -> ${after[k].mild}`); process.exit(1); }
  bank.note = 'Reviewed bark bank (RULINGS R-0007 point 1; hockey first per R-0021 point 2). Generated by tools/barks/generate.mjs through the Claude Code CLI, every line passed tools/barks/filter.mjs; the film-vocabulary lines were rewritten by hand as rink talk (tools/barks/theme.mjs, tools/barks/out/theme-rewrites.json). Edit by hand if you must, then run node tools/barks/embed.mjs. Situations are seen from the human player; speakers are opponent (mock, savage, complain, bravado) and teammate (encourage, complain, praise).';
  writeFileSync(bankPath, JSON.stringify(bank, null, 2) + '\n');
  const rewritten = new Set(Object.keys(rewrites.lines));
  const mildTotal = bank.lines.filter((l) => mildCount(l.text) > 0).length;
  const softLines = bank.lines.filter((l) => filmWords(l.text).soft.length > 0).length;
  writeFileSync(reviewPath, reviewMarkdown(bank, { rewritten, mildTotal, softLines }));
  writeFileSync(resolve(outDir, 'theme-pairs.json'), JSON.stringify({ schema: 'hokej.barks.theme-pairs/1', applied: new Date().toISOString(), pairs, mild_before: before, mild_after: after }, null, 1) + '\n');
  embed();
  console.log(`PASS theme apply: ${pairs.length} lines rewritten, ${mildTotal} mild of ${bank.lines.length}, ${softLines} soft film lines; REVIEW.md rebuilt`);
  console.log(JSON.stringify({ tool: 'thirdjam.barks.theme/1', ok: true, rewritten: pairs.length, lines: bank.lines.length, mild: mildTotal, soft_film_lines: softLines, reasons: stats(rejected) }));
}

if (process.argv[1] && /theme\.mjs$/.test(process.argv[1].replace(/\\/g, '/'))) {
  const cmd = process.argv[2];
  if (cmd === 'list') list(process.argv.includes('--before'));
  else if (cmd === 'apply') apply();
  else { console.log('FAIL usage: node tools/barks/theme.mjs list [--before] | apply'); process.exit(2); }
}
