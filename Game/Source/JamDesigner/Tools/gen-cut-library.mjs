// Lane A7, ruling R-0004 point 2: generates the shipped offline cut library with the real Designer through the Claude Code CLI.
//   node Game/Source/JamDesigner/Tools/gen-cut-library.mjs [--jobs 4] [--only <n>]
// Every sentence below goes through the same attempt chain the game uses (tools/designer-eval/loop.mjs: haiku, haiku repair,
// sonnet repair) over hockey.regular. A cut is kept only when validateDesignerOutput(hockey.regular, cut) passes again here.
// Writes Game/Content/Designer/cached-cuts.json (schema tj.designer-cache/1). Then run gen-designer-data.mjs to embed it.
// Names and keywords are hand-written per sentence (the name becomes theme.name): the offline matcher (UJamDesignerSubsystem::ResolveOffline) scores a player's words
// against the sentence (2 points a word) and the keywords (1 point).
import { readFileSync, writeFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { validateDesignerOutput } from '../../../../schema/validate.mjs';
import { runDesigner } from '../../../../tools/designer-eval/loop.mjs';
import { createCliModel } from '../../../../tools/designer-eval/cli-model.mjs';

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..', '..', '..');
const base = JSON.parse(readFileSync(resolve(repo, 'Game/Content/Cartridges/hockey.regular.cartridge.json'), 'utf8'));

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
];

function argValue(name, fallback) {
  const i = process.argv.indexOf(name);
  return i === -1 ? fallback : process.argv[i + 1];
}

async function main() {
  const jobs = Number(argValue('--jobs', '4'));
  const only = argValue('--only', undefined);
  const callModel = createCliModel();
  const items = LIBRARY.map(([sentence, name, keywords], index) => ({ sentence, name, keywords: keywords.split(' '), index }))
    .filter((x) => only === undefined || String(x.index) === only);
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

  const cuts = [];
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
    const name = x.name;
    if (names.has(name)) throw new Error(`duplicate cut name ${name}`);
    names.add(name);
    cuts.push({ name, sentence: x.sentence, keywords: x.keywords, base: 'hockey.regular', outcome: x.r.outcome, cartridge: x.r.cartridge });
  }
  if (only === undefined) {
    writeFileSync(resolve(repo, 'Game/Content/Designer/cached-cuts.json'), JSON.stringify({ schema: 'tj.designer-cache/1', generated_by: 'claude CLI, tools/designer-eval/loop.mjs chain over hockey.regular', cuts }, null, 2) + '\n');
  }
  console.log(JSON.stringify({ tool: 'gen-cut-library', kept: cuts.length, of: items.length, ...totals }));
  return cuts.length >= 24 || only !== undefined ? 0 : 1;
}

main().then((c) => { process.exitCode = c; }, (e) => { console.error(e.message); process.exitCode = 2; });
