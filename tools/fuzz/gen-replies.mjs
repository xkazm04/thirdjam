// Designer-reply parity corpus (lane A141, track S3, evolution 3). Mutated replies a model might send to the "direct a cut" panel, each with Node's verdict:
// kind (cartridge | refusal | invalid) and the sorted set of JSON pointers of its errors. The C++ mirror (FJamDesignerPrompt::ParseReply) must answer the same kind
// and the same pointer set for every reply (Jam.Designer.ReplyParity reads the corpus; JAM_FUZZ_REPLIES names a folder, the checked-in smoke is tools/fuzz/replies).
//
//   node tools/fuzz/gen-replies.mjs --out tools/fuzz/out/replies --count 1000 --seed 1
//
// Writes <out>/manifest.json: { base, cases: [{ name, kind, pointers, text }] } (the text inline: one file, not 1,000 small ones).
import { readFileSync, writeFileSync, mkdirSync } from 'node:fs';
import { dirname, resolve, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import { parseDesignerReply } from '../../schema/validate.mjs';
import { rng } from './gen.mjs';

const HERE = dirname(fileURLToPath(import.meta.url));
const REPO = resolve(HERE, '..', '..');
const BASE_FILE = join(REPO, 'Game', 'Content', 'Cartridges', 'hockey.regular.cartridge.json');
const base = JSON.parse(readFileSync(BASE_FILE, 'utf8'));
const clone = (v) => JSON.parse(JSON.stringify(v));
const pointers = (errors) => [...new Set(errors.map((e) => e.match(/^at (\S+?):/)?.[1] ?? '?'))].sort();

function leaves(v, p = [], out = []) {
  if (Array.isArray(v)) v.forEach((x, i) => leaves(x, [...p, i], out));
  else if (v && typeof v === 'object') for (const [k, x] of Object.entries(v)) leaves(x, [...p, k], out);
  else out.push(p);
  return out;
}
const get = (o, p) => p.reduce((c, k) => c[k], o);
const set = (o, p, val) => { get(o, p.slice(0, -1))[p.at(-1)] = val; };
const objectPaths = (v, p = [], out = []) => {
  if (v && typeof v === 'object') { if (!Array.isArray(v)) out.push(p); for (const [k, x] of Object.entries(v)) objectPaths(x, [...p, k], out); }
  return out;
};

const NUMERIC = leaves(base).filter((p) => typeof get(base, p) === 'number' && p[0] === 'game');
const OBJECTS = objectPaths(base);
// legal Designer edits (DESIGNER_GAME_EDITABLE) and edits it may not make (immutable keys, host-owned game keys)
const EDITS = [
  (c) => { c.game.rules.mode = "timer"; c.game.rules.timer_frames = 7200; delete c.game.rules.score_to_win; },
  (c) => { c.game.rink.length_m = 80; c.game.rink.width_m = 40; c.game.teams.forEach((t) => t.skaters.forEach((s) => { s.spawn[0] = Math.max(-30, Math.min(30, s.spawn[0])); s.spawn[1] = Math.max(-15, Math.min(15, s.spawn[1])); })); },
  (c) => { (c.game.stick ??= {}).foul_grace_frames = 30; },
  (c) => { c.game.look.tier = 'l3_neon'; },
  (c) => { c.game.ai = { ...(c.game.ai ?? {}), difficulty: 'all_star' }; },
  (c) => { c.game.rules.hooks = [{ id: 'h1', when: 'on_goal', then: [{ do: 'announce', text: 'GOAL' }] }]; },
  (c) => { c.theme.name = 'Smoke Rink'; },
  (c) => { c.values = { skater_speed: 12 }; },
];
const LOCKED = [
  (c) => { c.id = 'something-else'; }, (c) => { c.title = 'Other'; }, (c) => { c.seed = 99; }, (c) => { c.tunables[0].default = 5; },
  (c) => { (c.game.camera ??= {}).mode = "arena"; }, (c) => { (c.game.skater ??= {}).speed = 12; }, (c) => { c.game.teams[0].skaters[0].role = 'chaser'; }, (c) => { (c.game.shot ??= {}).speed = 30; },
];

// repeat a key inside an object by textual insertion right after one of its opening braces
function dupKey(text, r) {
  const open = [...text.matchAll(/\{\s*"([^"\\]+)":/g)];
  const m = r.pick(open);
  return text.slice(0, m.index + 1) + ` "${m[1]}": 1,` + text.slice(m.index + 1);
}

const TEXT = {
  'plain': (s) => s,
  'bom': (s) => '\uFEFF' + s,
  'leading-ws': (s) => '\n\n  ' + s,
  'trailing-ws': (s) => s + '\n\n',
  'prose-before': (s) => `Sure! Here is the cut you asked for:\n${s}`,
  'prose-after': (s) => `${s}\nLet me know if you want it faster.`,
  'prose-both': (s) => `Here:\n${s}\nEnjoy.`,
  'fence': (s) => '```json\n' + s + '\n```',
  'fence-bare': (s) => '```\n' + s + '\n```',
  'two-objects': (s) => `${s}\n${s}`,
  'trailing-comma': (s) => s.replace(/\n?\}\s*$/, ',\n}'),
  'comment': (s) => s.replace('{', '{ // note\n'),
  'single-quotes': (s) => s.replace(/"/g, "'"),
  'nan': (s) => s.replace(/"seed": \d+/, '"seed": NaN'),
  'huge-number': (s) => s.replace(/"seed": \d+/, '"seed": 1e999'),
  'control-char': (s) => s.replace('"title": "', '"title": "a\u0001'),
  'bad-escape': (s) => s.replace('"title": "', '"title": "\\q'),
  'bad-unicode': (s) => s.replace('"title": "', '"title": "\\u12G4'),
};

export function generateReplies({ count, seed }) {
  const r = rng(seed);
  const cases = [];
  const add = (name, text) => {
    const res = parseDesignerReply(text, base);
    const errors = res.kind === 'invalid' ? res.errors : [];
    cases.push({ name, kind: res.kind, pointers: pointers(errors), text });
  };
  const textNames = Object.keys(TEXT).filter((n) => n !== 'plain');
  for (let i = 0; cases.length < count; i++) {
    const tag = `r${String(i + 1).padStart(4, '0')}`;
    const c = clone(base);
    switch (i % 14) {
      case 0: add(`${tag}-exact`, TEXT[r.pick(['plain', 'bom', 'leading-ws', 'trailing-ws'])](JSON.stringify(c, null, r.pick([0, 1, 2])))); break;
      case 1: r.pick(EDITS)(c); add(`${tag}-edit`, TEXT[r.pick(['plain', 'bom'])](JSON.stringify(c, null, r.pick([0, 1])))); break;
      case 2: r.pick(LOCKED)(c); add(`${tag}-locked`, JSON.stringify(c)); break;
      case 3: {
        const p = r.pick(NUMERIC); const v = get(c, p);
        set(c, p, r.pick([v + 1e6, -1e6, -v - 1, 1e300, v * 1000 + 0.5]));
        add(`${tag}-range-${p.join('.')}`, JSON.stringify(c)); break;
      }
      case 4: {
        const p = r.pick(OBJECTS); get(c, p)[r.pick(['foo', 'speedd', 'Speed', 'x'])] = r.pick([1, 'x', true, null]);
        add(`${tag}-unknown-${p.join('.') || 'root'}`, JSON.stringify(c, null, 1)); break;
      }
      case 5: add(`${tag}-dupkey`, dupKey(JSON.stringify(c, null, r.pick([0, 1])), r)); break;
      case 6: { const s = JSON.stringify(c, null, r.pick([0, 1])); add(`${tag}-truncated`, s.slice(0, r.int(0, s.length - 1))); break; }
      case 7: { const t = r.pick(textNames); add(`${tag}-${t}`, TEXT[t](JSON.stringify(c, null, 1))); break; }
      case 8: {
        const refs = ['{"refusal":"I cannot do that."}', '{"refusal":""}', '{"refusal":"   "}', '{"refusal":5}', '{"refusal":null}', '{"refusal":"x","values":{}}',
          `{"refusal":"${'y'.repeat(301)}"}`, `{"refusal":"${'y'.repeat(300)}"}`, '{"refusal":"no","refusal":"yes"}', '\uFEFF{"refusal":"bom"}', 'Sorry. {"refusal":"prose"}', '{"refusal":"a"} thanks'];
        add(`${tag}-refusal`, r.pick(refs)); break;
      }
      case 9: add(`${tag}-root`, r.pick(['', '   ', '[]', '[1,2]', '"text"', '12', 'null', 'true', '{}', '{"game":1}', '[{"a":1}]', '\uFEFF', '{', '}', '{"a":', '{"a" 1}'])); break;
      case 10: {
        const p = r.pick(leaves(base)); set(c, p, r.pick(['text', true, null, [], {}, 1.5, -1]));
        add(`${tag}-type-${p.join('.')}`, JSON.stringify(c)); break;
      }
      case 11: {
        r.pick(LOCKED)(c); const p = r.pick(NUMERIC); set(c, p, get(c, p) + 1e6);
        add(`${tag}-two`, TEXT[r.pick(['plain', 'bom', 'prose-before', 'fence'])](JSON.stringify(c, null, 1))); break;
      }
      case 12: {
        const d = r.pick([10, 63, 64, 65, 200]);
        add(`${tag}-deep${d}`, r.coin() ? '['.repeat(d) + ']'.repeat(d) : '{"a":'.repeat(d) + '1' + '}'.repeat(d)); break;
      }
      default: r.pick(EDITS)(c); r.pick(LOCKED)(c); add(`${tag}-edit-and-locked`, JSON.stringify(c)); break;
    }
  }
  return cases;
}

if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  const arg = (k, d) => { const i = process.argv.indexOf(`--${k}`); return i >= 0 ? process.argv[i + 1] : d; };
  const out = resolve(arg('out', join(HERE, 'out', 'replies')));
  const cases = generateReplies({ count: Number(arg('count', '1000')), seed: Number(arg('seed', '1')) });
  mkdirSync(out, { recursive: true });
  writeFileSync(join(out, 'manifest.json'), JSON.stringify({ tool: 'gen-replies', base: 'Game/Content/Cartridges/hockey.regular.cartridge.json', cases }));
  const kinds = {};
  for (const c of cases) kinds[c.kind] = (kinds[c.kind] ?? 0) + 1;
  console.log(`PASS gen-replies ${cases.length} cases ${JSON.stringify(kinds)} -> ${out}`);
}
