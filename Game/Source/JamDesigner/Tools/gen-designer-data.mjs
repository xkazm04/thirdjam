// Lane A7. Two jobs, run after any change to prompts/, schema/ or the cached cuts below:
//   node Game/Source/JamDesigner/Tools/gen-designer-data.mjs
// 1. Writes Game/Content/Designer/cached-cuts.json: the offline Designer (no key) answers from these sentence -> cut pairs.
//    Every cut is hockey.regular edited the way the mock model edits (values, theme, editable game paths) with hand-tuned
//    numbers, and must pass validateDesignerOutput(base, cut) from schema/validate.mjs, or this script exits 1.
// 2. Writes Game/Source/JamDesigner/Private/JamDesignerData.inl: the two prompt templates, the three schema files and the
//    cached cuts as bytes (a packaged game has no repo folders), plus parity hashes of what tools/designer-eval/assemble.mjs
//    builds, so the C++ test Jam.Designer.PromptParity proves the C++ request body is byte-identical to the Node one.
import { mkdirSync, readFileSync, writeFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { validateDesignerOutput } from '../../../../schema/validate.mjs';
import { buildSystemText, assembleRequest, MODELS } from '../../../../tools/designer-eval/assemble.mjs';

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..', '..', '..');
const base = JSON.parse(readFileSync(resolve(repo, 'Game/Content/Cartridges/hockey.regular.cartridge.json'), 'utf8'));
const clone = (x) => structuredClone(x);

// ---------- 1. cached cuts ----------
const theme = (name, mood, palette) => ({ name, mood_words: mood, palette: palette.map(([n, h]) => ({ name: n, hex: h })) });
const PLANS = [
  {
    name: 'Fast Cut', sentence: 'make it faster', keywords: ['fast', 'quick', 'speed', 'speedy', 'rapid', 'hurry'],
    edit: (c) => { c.values = { skater_speed: 11, shot_speed: 24, puck_max_speed: 38 }; c.theme.name = 'Fast Cut'; },
  },
  {
    name: 'Calm Cut', sentence: 'a calm version for my kid', keywords: ['calm', 'kid', 'easy', 'slow', 'gentle', 'beginner', 'relax', 'child'],
    edit: (c) => {
      c.values = { skater_speed: 6, ai_speed_scale: 0.6, ai_red_skill: 0.3, shot_speed: 15 };
      c.game.rules.score_to_win = 2;
      c.theme = theme('Calm Cut', ['calm', 'gentle', 'soft'], [['ice', '#E6F0F7'], ['sky', '#7FB3D5'], ['blue', '#1D4ED8'], ['orange', '#E8651A'], ['dark', '#1B2A3A']]);
    },
  },
  {
    name: 'Big Glass Cut', sentence: 'twice the rink, half the friction', keywords: ['twice', 'double', 'big', 'huge', 'large', 'friction', 'slide', 'slippery'],
    edit: (c) => { c.values = { ice_friction: 0.0075 }; c.game.rink.length_m = 160; c.game.rink.width_m = 80; c.game.rink.ice_friction = 0.0075; c.theme.name = 'Big Glass Cut'; },
  },
  {
    name: 'Glass Ice Cut', sentence: 'ice like glass and bouncy boards', keywords: ['glass', 'bouncy', 'bounce', 'boards', 'pinball', 'slick'],
    edit: (c) => { c.values = { ice_friction: 0.002, board_restitution: 0.85 }; c.game.rink.ice_friction = 0.002; c.game.rink.board_restitution = 0.85; c.theme.name = 'Glass Ice Cut'; },
  },
  {
    name: 'Power-Up Cut', sentence: 'add power-ups to grab on the ice', keywords: ['power', 'powerup', 'pickup', 'boost', 'item', 'bonus', 'grab'],
    edit: (c) => {
      c.game.pickups = [
        { kind: 'speed', spawn_every_frames: 600, duration_frames: 300 },
        { kind: 'big_puck', spawn_every_frames: 900, duration_frames: 360 },
        { kind: 'precision', spawn_every_frames: 720, duration_frames: 300 },
      ];
      c.theme.name = 'Power-Up Cut';
    },
  },
  {
    name: 'One Minute Cut', sentence: 'a short timed game of one minute', keywords: ['short', 'timer', 'timed', 'minute', 'clock', 'time'],
    edit: (c) => { c.game.rules.mode = 'timer'; delete c.game.rules.score_to_win; c.game.rules.timer_frames = 3600; c.values = { goal_pause_frames: 60 }; c.theme.name = 'One Minute Cut'; },
  },
  {
    name: 'Neon Pillars Cut', sentence: 'a neon party with pillars in the way', keywords: ['neon', 'party', 'pillar', 'obstacle', 'disco', 'light'],
    edit: (c) => {
      c.game.look.tier = 'l3_neon';
      c.game.rink.obstacles = [
        { kind: 'pillar', x: -20, z: -15, size: 2 }, { kind: 'pillar', x: -20, z: 15, size: 2 },
        { kind: 'pillar', x: 20, z: -15, size: 2 }, { kind: 'pillar', x: 20, z: 15, size: 2 },
      ];
      c.theme = theme('Neon Pillars Cut', ['neon', 'loud', 'party'], [['ice', '#1A1035'], ['pink', '#FF3DB8'], ['cyan', '#22D3EE'], ['blue', '#1D4ED8'], ['orange', '#E8651A']]);
    },
  },
  {
    name: 'Small Fast Rink Cut', sentence: 'a small rink where everything is tight', keywords: ['small', 'tight', 'tiny', 'little', 'close', 'cramped'],
    edit: (c) => { c.values = { skater_speed: 10, skater_turn_rate: 7 }; c.game.rink.length_m = 80; c.game.rink.width_m = 40; c.theme.name = 'Small Fast Rink Cut'; },
  },
];

let bad = 0;
const cuts = [];
for (const p of PLANS) {
  const out = clone(base);
  p.edit(out);
  const res = validateDesignerOutput(base, out);
  if (!res.ok) { bad += 1; console.log(`FAIL cached-cut "${p.name}"`); for (const e of res.errors) console.log(`  ${e}`); continue; }
  console.log(`PASS cached-cut "${p.name}"`);
  cuts.push({ name: p.name, sentence: p.sentence, keywords: p.keywords, base: 'hockey.regular', cartridge: out });
}
if (bad) { console.log(JSON.stringify({ tool: 'gen-designer-data', ok: false, bad })); process.exit(1); }
mkdirSync(resolve(repo, 'Game/Content/Designer'), { recursive: true });
const cacheText = JSON.stringify({ schema: 'tj.designer-cache/1', cuts }, null, 2) + '\n';
writeFileSync(resolve(repo, 'Game/Content/Designer/cached-cuts.json'), cacheText);

// ---------- 2. embedded data + parity hashes ----------
function fnv1a64(str) {
  let h = 0xcbf29ce484222325n;
  for (const b of Buffer.from(str, 'utf8')) { h ^= BigInt(b); h = (h * 0x100000001b3n) & 0xffffffffffffffffn; }
  return '0x' + h.toString(16).padStart(16, '0') + 'ull';
}
const system = buildSystemText();
const PARITY_SENTENCE = '  make it <faster> & "meaner"  ';
const PARITY_PREVIOUS = 'Sure! <here> {"x":1}';
const PARITY_ERRORS = ['at /values/skater_speed: 99 is above the maximum 16.', 'at /: second error.'];
const first = JSON.stringify(assembleRequest({ cartridge: base, sentence: PARITY_SENTENCE, model: MODELS.first }));
const repair = JSON.stringify(assembleRequest({
  cartridge: base, sentence: PARITY_SENTENCE, model: MODELS.retry, previous: { output: PARITY_PREVIOUS, errors: PARITY_ERRORS },
}));

const files = [
  ['designer.system.md', resolve(repo, 'prompts/designer.system.md')],
  ['designer.repair.md', resolve(repo, 'prompts/designer.repair.md')],
  ['cartridge.schema.json', resolve(repo, 'schema/cartridge.schema.json')],
  ['game.schema.json', resolve(repo, 'schema/game.schema.json')],
  ['limits.json', resolve(repo, 'schema/limits.json')],
  ['cached-cuts.json', resolve(repo, 'Game/Content/Designer/cached-cuts.json')],
];
const cstr = (s) => 'TEXT(' + JSON.stringify(s) + ')';
let out = '// GENERATED by Game/Source/JamDesigner/Tools/gen-designer-data.mjs from prompts/, schema/ and the cached cuts. Do not edit.\n#pragma once\n\n';
const names = [];
for (const [name, path] of files) {
  const bytes = readFileSync(path);
  const id = 'JamDesignerData_' + name.replace(/[^A-Za-z0-9]/g, '_');
  names.push([name, id]);
  const rows = [];
  for (let i = 0; i < bytes.length; i += 32) rows.push('\t' + Array.from(bytes.subarray(i, i + 32)).join(',') + ',');
  out += `static const unsigned char ${id}[] = {\n${rows.join('\n')}\n\t0\n};\n\n`;
}
out += 'struct FJamDesignerEmbeddedFile { const TCHAR* Name; const unsigned char* Data; };\n';
out += 'static const FJamDesignerEmbeddedFile JamDesignerEmbeddedFiles[] = {\n' + names.map(([f, id]) => `\t{ TEXT("${f}"), ${id} },`).join('\n') + '\n};\n\n';
out += '// Parity with tools/designer-eval/assemble.mjs: FNV-1a 64 over UTF-8 and the length in UTF-8 bytes of buildSystemText() and of\n';
out += '// JSON.stringify(assembleRequest(...)) for hockey.regular, the first attempt (haiku) and a repair attempt (sonnet).\n';
out += `static const TCHAR* JamDesignerParitySentence = ${cstr(PARITY_SENTENCE)};\n`;
out += `static const TCHAR* JamDesignerParityPrevious = ${cstr(PARITY_PREVIOUS)};\n`;
out += `static const TCHAR* JamDesignerParityErrors[] = { ${PARITY_ERRORS.map(cstr).join(', ')} };\n`;
out += `static const uint64 JamDesignerParitySystemHash = ${fnv1a64(system)}; static const int32 JamDesignerParitySystemBytes = ${Buffer.byteLength(system)};\n`;
out += `static const uint64 JamDesignerParityFirstHash = ${fnv1a64(first)}; static const int32 JamDesignerParityFirstBytes = ${Buffer.byteLength(first)};\n`;
out += `static const uint64 JamDesignerParityRepairHash = ${fnv1a64(repair)}; static const int32 JamDesignerParityRepairBytes = ${Buffer.byteLength(repair)};\n`;
writeFileSync(resolve(here, '..', 'Private', 'JamDesignerData.inl'), out);
console.log(`PASS gen-designer-data ${cuts.length} cuts, ${files.length} files, system prompt ${Buffer.byteLength(system)} bytes`);
console.log(JSON.stringify({ tool: 'gen-designer-data', ok: true, cuts: cuts.length }));
