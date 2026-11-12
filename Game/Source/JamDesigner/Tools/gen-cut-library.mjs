// Lane A7, ruling R-0004 point 2: generates the shipped offline cut library with the real Designer through the Claude Code CLI.
//   node Game/Source/JamDesigner/Tools/gen-cut-library.mjs [--jobs 4] [--only <n>] [--merge [--redo <n,n>]]
// Every sentence below goes through the same attempt chain the game uses (tools/designer-eval/loop.mjs: haiku, haiku repair,
// sonnet repair) over hockey.regular. A cut is kept only when validateDesignerOutput(hockey.regular, cut) passes again here.
// Writes Game/Content/Designer/cached-cuts.json (schema tj.designer-cache/1). Then run gen-designer-data.mjs to embed it.
// --merge (lane A44): keep the cuts the current file already has for this exact hockey.regular (same base_sha256, re-validated here,
// keywords refreshed from LIBRARY) and send only the sentences it lacks (plus --redo indexes) to the model; the file is written in
// LIBRARY order under the same base guard. --only <n> still runs one sentence and writes nothing.
// Names and keywords are hand-written per sentence (the name becomes theme.name): the offline matcher (UJamDesignerSubsystem::ResolveOffline) scores a player's words
// against the sentence (2 points a word) and the keywords (1 point).
import { readFileSync, writeFileSync } from 'node:fs';
import { createHash } from 'node:crypto';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { validateDesignerOutput } from '../../../../schema/validate.mjs';
import { runDesigner } from '../../../../tools/designer-eval/loop.mjs';
import { createCliModel } from '../../../../tools/designer-eval/cli-model.mjs';

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..', '..', '..');
const REGULAR = resolve(repo, 'Game/Content/Cartridges/hockey.regular.cartridge.json');
const base = JSON.parse(readFileSync(REGULAR, 'utf8'));
// The library is only valid over the exact hockey.regular it was made from (a Designer may not move spawns, so a changed
// preset invalidates every cut). Hash of the parsed JSON, compact: whitespace and line endings do not count.
export const cartridgeHash = (obj) => createHash('sha256').update(JSON.stringify(obj)).digest('hex');
const baseHash = cartridgeHash(base);

export const LIBRARY = [
  ['make it faster', 'Fast Cut', 'fast quick speed speedy rapid hurry zoom'],
  ['make it slower and easier to follow', 'Slow Motion Cut', 'slow slower easy relaxed lazy chill'],
  ['a calm version for my kid', 'Family Cut', 'calm kid child gentle beginner soft family'],
  ['make it chaotic with pinball boards', 'Pinball Cut', 'chaos chaotic wild crazy mayhem pinball madness'],
  ['twice the rink, half the friction', 'Big Glass Cut', 'twice double huge big enormous friction'],
  ['a tiny cramped rink', 'Shoebox Cut', 'small tiny cramped tight little close mini'],
  ['a wide open rink with room to breathe', 'Open Ice Cut', 'wide open spacious room roomy breathe'],
  ['ice like glass', 'Glass Ice Cut', 'glass slippery slide slick frictionless skate'],
  ['sticky slow ice like carpet', 'Carpet Cut', 'sticky carpet grippy rough grip'],
  ['add power-ups to grab on the ice', 'Power-Up Cut', 'power powerup pickup item bonus grab'],
  ['lots of speed boosts', 'Turbo Cut', 'boost turbo nitro pickup'],
  ['pickups that make the puck giant', 'Giant Puck Cut', 'giant big puck huge fat'],
  ['a neon party', 'Neon Cut', 'neon party disco glow colourful colorful'],
  ['a bright daytime game', 'Matinee Cut', 'day daytime bright sun sunny morning'],
  ['make it easier to score', 'Easy Scoring Cut', 'easy easier score scoring goals simple'],
  ['make the bots really hard', 'All-Star Cut', 'hard harder difficult challenge tough expert pro star allstar'],
  ['rookie opponents so I can learn', 'Rookie Cut', 'rookie learn beginner practice novice weak easier'],
  ['a one minute sprint', 'One Minute Cut', 'minute short sprint timer time quick clock'],
  ['a long match, first to nine', 'Marathon Cut', 'long nine marathon endurance epic'],
  ['sudden death, first goal wins', 'Sudden Death Cut', 'sudden death single decider overtime'],
  ['pillars in the way', 'Pillar Cut', 'pillar obstacle column post'],
  ['a maze of boxes on the ice', 'Maze Cut', 'maze box crate obstacle block'],
  ['rocket shots', 'Rocket Cut', 'rocket shot blast cannon strong slapshot'],
  ['bigger goals and more scoring', 'Big Net Cut', 'goal score scoring net high'],
  ['tiny goals for a defensive battle', 'Lockdown Cut', 'defensive defense defence tight battle grind'],
  ['a cozy winter pond game', 'Pond Cut', 'cozy cosy winter pond snow frozen lake'],
  ['a sunset championship final', 'Final Cut', 'sunset championship final warm golden trophy'],
  ['a retro arcade look', 'Arcade Cut', 'retro arcade old classic pixel vintage'],
  ['boards that bounce like rubber', 'Rubber Boards Cut', 'rubber bounce bouncy board wall'],
  ['a spooky midnight match', 'Midnight Cut', 'spooky midnight ghost eerie halloween creepy'],
  // Lane A44: A37's three empty spots in the fun space (CUTS.md: no_shoot in front of both nets, a goal_against boost, goal_shield on a clock)
  // and one cut per payoff (PAYOFFS.md section 2). Hockey words first in the names and the sentences.
  ['carry it in, no shots from far out', 'Carry It In Cut', 'carry slot close range net shooting shot shots zone crease dump long'],
  ['help whoever is losing', 'Comeback Cut', 'comeback losing behind trailing catch up underdog rally fair even'],
  ['a two metre shield in front of each net and a four minute clock', 'Shield Cut', 'shield wall guard block protect nets post wraparound'],
  ['one-timers that always find the net', 'One-Timer Night', 'onetimer timer timers pass catch shoot tip tap redirect night'],
  ['give breakaways a burst', 'Breakaway Cut', 'breakaway breakaways burst sprint rush alone open ice last man'],
  ['let me deke the last man', 'Deke Cut', 'deke dekes dangle dangles fake shimmy move moves goalie last man'],
];

function argValue(name, fallback) {
  const i = process.argv.indexOf(name);
  return i === -1 ? fallback : process.argv[i + 1];
}

async function main() {
  const jobs = Number(argValue('--jobs', '4'));
  const only = argValue('--only', undefined);
  const merge = process.argv.includes('--merge');
  const redo = new Set((argValue('--redo', '') || '').split(',').filter(Boolean));
  const cachePath = resolve(repo, 'Game/Content/Designer/cached-cuts.json');
  // --merge: the cuts already in the file stay when they were made over this exact hockey.regular.
  const existing = new Map();
  if (merge) {
    const cache = JSON.parse(readFileSync(cachePath, 'utf8'));
    if (cache.base_sha256 !== baseHash) { console.log(`FAIL --merge: the current library was made over hockey.regular ${String(cache.base_sha256).slice(0, 12)}, this one is ${baseHash.slice(0, 12)}; run without --merge`); return 2; }
    for (const c of cache.cuts) existing.set(c.sentence, c);
  }
  const inner = createCliModel();
  let calls = 0;
  let costUsd = 0;
  const callModel = async (body) => { calls += 1; const r = await inner(body); costUsd += r.cost_usd ?? 0; return r; };
  const all = LIBRARY.map(([sentence, name, keywords], index) => ({ sentence, name, keywords: keywords.split(' '), index }));
  const items = all.filter((x) => (only === undefined ? !merge || !existing.has(x.sentence) || redo.has(String(x.index)) : String(x.index) === only));
  const results = new Array(items.length);
  let next = 0;
  async function worker() {
    while (next < items.length) {
      const k = next++;
      const it = items[k];
      const r = await runDesigner({ cartridge: base, sentence: it.sentence, callModel });
      results[k] = { ...it, r };
      const via = r.attempts.map((a) => `${a.model.replace(/^claude-/, '')}:${a.kind}`).join(' > ');
      console.log(`${r.cartridge ? 'PASS' : 'FAIL'} [${r.outcome}] ${r.ms} ms "${it.sentence}" ${via}${r.refusal ? ` refusal: ${r.refusal}` : ''}`);
    }
  }
  await Promise.all(Array.from({ length: Math.max(1, jobs) }, worker));

  const made = new Map();
  const names = new Set();
  const totals = { first_try: 0, repaired: 0, refused: 0, failed: 0, ms: 0, tokens_in: 0, tokens_out: 0 };
  for (const x of results) {
    totals[x.r.outcome] += 1;
    totals.ms += x.r.ms; totals.tokens_in += x.r.tokens_in; totals.tokens_out += x.r.tokens_out;
    if (!x.r.cartridge) continue;
    // The display name is ours (the model mostly keeps "Night Rink"); theme.name is a Designer-editable field, re-validated below.
    x.r.cartridge.theme = { ...x.r.cartridge.theme, name: x.name };
    const check = validateDesignerOutput(base, x.r.cartridge);
    if (!check.ok) { console.log(`FAIL revalidate "${x.sentence}": ${check.errors[0]}`); continue; }
    made.set(x.sentence, { name: x.name, sentence: x.sentence, keywords: x.keywords, base: 'hockey.regular', outcome: x.r.outcome, cartridge: x.r.cartridge });
  }
  // The file in LIBRARY order: this run's cuts, then (with --merge) the kept ones, re-validated, with the keywords LIBRARY has now.
  const cuts = [];
  for (const x of all) {
    let cut = made.get(x.sentence);
    if (!cut && merge && existing.has(x.sentence) && !items.some((it) => it.sentence === x.sentence)) {
      const kept = existing.get(x.sentence);
      const check = validateDesignerOutput(base, kept.cartridge);
      if (!check.ok) { console.log(`FAIL kept cut "${kept.name}" no longer validates: ${check.errors[0]}`); continue; }
      cut = { ...kept, keywords: x.keywords };
    }
    if (!cut) continue;
    if (names.has(cut.name)) throw new Error(`duplicate cut name ${cut.name}`);
    names.add(cut.name);
    cuts.push(cut);
  }
  const nowHash = cartridgeHash(JSON.parse(readFileSync(REGULAR, 'utf8')));
  if (nowHash !== baseHash) {
    console.log(`FAIL hockey.regular changed during the run (${baseHash.slice(0, 12)} -> ${nowHash.slice(0, 12)}); nothing written, run again`);
    return 1;
  }
  if (only === undefined) {
    writeFileSync(cachePath, JSON.stringify({ schema: 'tj.designer-cache/1', base_sha256: baseHash, generated_by: 'claude CLI, tools/designer-eval/loop.mjs chain over hockey.regular', cuts }, null, 2) + '\n');
  }
  console.log(JSON.stringify({ tool: 'gen-cut-library', kept: cuts.length, of: all.length, generated: items.length, ...totals, calls, cost_usd: Math.round(costUsd * 10000) / 10000 }));
  return cuts.length >= 24 || only !== undefined ? 0 : 1;
}

main().then((c) => { process.exitCode = c; }, (e) => { console.error(e.message); process.exitCode = 2; });
