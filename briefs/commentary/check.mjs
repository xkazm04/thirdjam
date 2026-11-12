// Validates briefs/commentary/lines.json (lane N2). Run: node briefs/commentary/check.mjs
// Exit code 1 on any error. Prints counts and the credit estimate.
import { readFileSync } from 'node:fs';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';

const here = dirname(fileURLToPath(import.meta.url));
const lines = JSON.parse(readFileSync(join(here, 'lines.json'), 'utf8'));

// C-0003 v1 (ten) + the compiled additions in JamEventBus.h (directors_call, period, bark, signal)
const C0003 = ['shot', 'board', 'goal', 'faceoff', 'check', 'knockdown', 'pickup_spawn', 'pickup_take', 'win', 'lose', 'directors_call', 'period', 'bark', 'signal'];
// C-0005 section 6 and R-0013 item 1
const RULED = ['announce', 'effect_start', 'effect_end', 'zone_enter', 'zone_exit', 'pass', 'poke', 'lift', 'foul'];
// Assumed by lane N2: not in any contract or ruling yet (see README "Assumed triggers")
const ASSUMED = ['save', 'post', 'breakaway', 'carry', 'period_end', 'clock', 'season_take', 'directors_call_open', 'menu'];
const KEYS = new Set(['id', 'speaker', 'event', 'kind', 'when', 'team', 'priority', 'cooldown_s', 'text']);
const REQUIRED = ['id', 'speaker', 'event', 'when', 'team', 'priority', 'cooldown_s', 'text'];
const MAX_QUICK = 90;
// Words that must never appear (real broadcasters' trademark calls, brands, leagues); case-insensitive substrings.
const BANNED = ['he shoots, he scores', 'nhl', 'stanley', 'top shelf where', 'boom goes', 'oh baby', 'holy mackinaw', 'hello, canada', 'downtown'];

const errors = [];
const assumedUsed = new Set();
const ids = new Set();
let prevSituation = null;
const situations = new Map(); // situation id -> {speaker, n}
for (const [i, l] of lines.entries()) {
  const where = `#${i} ${l.id ?? '(no id)'}`;
  for (const k of Object.keys(l)) if (!KEYS.has(k)) errors.push(`${where}: unknown key ${k}`);
  for (const k of REQUIRED) if (l[k] === undefined || l[k] === null || l[k] === '') errors.push(`${where}: missing ${k}`);
  if ('kind' in l && (typeof l.kind !== 'string' || !l.kind)) errors.push(`${where}: kind present but empty (omit it instead)`);
  if (ids.has(l.id)) errors.push(`${where}: duplicate id`);
  ids.add(l.id);
  if (!/^[a-z0-9_]+(\.[a-z0-9_]+)+\.\d\d$/.test(l.id)) errors.push(`${where}: id shape`);
  if (!['commentator', 'director'].includes(l.speaker)) errors.push(`${where}: speaker`);
  if (!['blue', 'orange', 'any'].includes(l.team)) errors.push(`${where}: team`);
  if (!Number.isInteger(l.priority) || l.priority < 1 || l.priority > 5) errors.push(`${where}: priority`);
  if (typeof l.cooldown_s !== 'number' || l.cooldown_s < 0) errors.push(`${where}: cooldown_s`);
  if (C0003.includes(l.event) || RULED.includes(l.event)) { /* ok */ }
  else if (ASSUMED.includes(l.event)) assumedUsed.add(l.event);
  else errors.push(`${where}: unknown event ${l.event}`);
  if (l.speaker === 'commentator' && l.id.split('.')[0] !== l.event) errors.push(`${where}: commentator id must start with its event`);
  if (l.speaker === 'director' && !l.id.startsWith('dir.')) errors.push(`${where}: director id must start with dir.`);
  if (l.text.length > MAX_QUICK) errors.push(`${where}: ${l.text.length} characters (max ${MAX_QUICK})`);
  if (/\d/.test(l.text)) errors.push(`${where}: digits in text (spell numbers, ten at most)`);
  if (/\b(eleven|twelve|twenty|thirty|forty|fifty|hundred|thousand)\b/i.test(l.text)) errors.push(`${where}: number above ten`);
  for (const b of BANNED) if (l.text.toLowerCase().includes(b)) errors.push(`${where}: banned phrase "${b}"`);
  if (/\b(he|she|his|her|him)\b/i.test(l.text)) errors.push(`${where}: gendered pronoun (skaters are unnamed toy figures)`);
  const t = l.text.toLowerCase();
  if (l.team === 'blue' && !t.includes('blue')) errors.push(`${where}: team blue but the text does not say blue`);
  if (l.team === 'orange' && !t.includes('orange')) errors.push(`${where}: team orange but the text does not say orange`);
  if (l.team === 'any' && l.speaker === 'commentator' && /\b(blue|orange)\b/.test(t) && !l.id.startsWith('period.')) errors.push(`${where}: names a colour but team is any`);
  const sit = l.id.replace(/\.\d\d$/, '');
  const s = situations.get(sit) ?? { speaker: l.speaker, n: 0, event: l.event, chars: 0, words: 0 };
  s.n++; s.chars += l.text.length; s.words += l.text.split(/\s+/).length;
  if (s.speaker !== l.speaker || s.event !== l.event) errors.push(`${where}: situation mixes speaker or event`);
  situations.set(sit, s);
  prevSituation = sit;
}

const sum = (sp, f) => [...situations.values()].filter(s => s.speaker === sp).reduce((a, s) => a + f(s), 0);
const rows = ['commentator', 'director'].map(sp => {
  const lc = lines.filter(l => l.speaker === sp);
  const chars = lc.reduce((a, l) => a + l.text.length, 0);
  const words = lc.reduce((a, l) => a + l.text.split(/\s+/).length, 0);
  const long = lc.filter(l => l.text.split(/\s+/).length > 8).length;
  return { speaker: sp, situations: sum(sp, () => 1), lines: lc.length, chars, mean_chars: +(chars / lc.length).toFixed(1), mean_words: +(words / lc.length).toFixed(1), over_8_words: long, max_chars: Math.max(...lc.map(l => l.text.length)) };
});
const alt = n => [...situations.values()].filter(s => s.speaker === 'commentator' && s.n === n).length;
console.table(rows);
console.log(`commentator situations with 3 / 2 / 1 alternates: ${alt(3)} / ${alt(2)} / ${alt(1)}`);
const total = rows[0].chars + rows[1].chars;
console.log(`total characters ${total}; credits at 1 per character, one voice per speaker: ${total} (cap 12,000: ${total < 12000 ? 'under' : 'OVER'})`);
console.log(`assumed events used: ${[...assumedUsed].sort().join(', ')}`);
console.log(`events with lines: ${[...new Set(lines.map(l => l.event))].sort().join(', ')}`);
console.log(`events with no line: ${[...C0003, ...RULED].filter(e => !lines.some(l => l.event === e)).join(', ') || 'none'}`);
if (errors.length) { console.error(errors.join('\n')); console.error(`${errors.length} error(s)`); process.exit(1); }
console.log('OK');
