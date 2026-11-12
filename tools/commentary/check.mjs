// Lane A32: the commentator manifest check (B-to-A-0026, brief A32 job 1). Every line of commentary.json (hokej.commentary/1) must have exactly
// the ten keys, an id of the form <event>.<situation>.<nn> whose first segment is its event, a team in blue|orange|any, an integer priority 1..5,
// a cooldown_s of 0 or more, a sound path whose .uasset exists under Game/Plugins/JamAssets/Content, positive seconds, and a text that passes the
// bark content filter (tools/barks/filter.mjs). Two allowances the commentator register needs and the bark filter does not have, printed as INFO
// and never as a rejection: a stoppage line (events period, period_end, season_take, win, lose, goal.pause.*) may run to 64 characters (B's README
// section 2: full sentences only in stoppages), and the hockey phrase "sudden death" is not a threat.
//   node tools/commentary/check.mjs [manifest]      exit 0 clean, 1 any error, 2 could not run
import { existsSync, readFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { checkLine, MAX_CHARS } from '../barks/filter.mjs';

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..');

export const KEYS = ['id', 'event', 'kind', 'team', 'priority', 'cooldown_s', 'when', 'text', 'sound', 'seconds'];
export const TOP_KEYS = ['schema', 'voice', 'player', 'sound_class', 'count', 'rules', 'lines'];
export const SCHEMA = 'hokej.commentary/1';
export const TEAMS = ['blue', 'orange', 'any'];
export const STOPPAGE_EVENTS = ['period', 'period_end', 'season_take', 'win', 'lose'];
export const STOPPAGE_MAX_CHARS = 64;
// Events the game sends (contracts/events.md) plus the seven this lane adds.
export const KNOWN_EVENTS = ['shot', 'board', 'goal', 'faceoff', 'check', 'knockdown', 'pickup_spawn', 'pickup_take', 'win', 'lose', 'directors_call', 'period',
  'bark', 'signal', 'announce', 'pass', 'effect_start', 'effect_end', 'zone_enter', 'zone_exit', 'poke', 'lift', 'foul', 'shot_blocked',
  'save', 'post', 'breakaway', 'carry', 'period_end', 'clock', 'season_take'];

export const situationOf = (id) => String(id).replace(/\.\d+$/, '');
export const isStoppage = (line) => STOPPAGE_EVENTS.includes(line.event) || situationOf(line.id) === 'goal.pause';

// The content-filter verdict for one line with the two allowances: { reasons: [...], infos: [...] }.
export function checkText(line) {
  const reasons = [];
  const infos = [];
  for (const r of checkLine(line.text)) {
    if (/^longer than/.test(r) && isStoppage(line) && String(line.text).trim().length <= STOPPAGE_MAX_CHARS) {
      infos.push(`stoppage line over ${MAX_CHARS} characters (${line.text.length}), allowed to ${STOPPAGE_MAX_CHARS}`);
      continue;
    }
    if (/banned word "death"/.test(r) && /\bsudden death\b/i.test(line.text)) { infos.push('"sudden death" is the hockey phrase, allowed'); continue; }
    reasons.push(r);
  }
  return { reasons, infos };
}

function defaultSoundExists(root) {
  return (p) => {
    const pkg = String(p).split('.')[0].replace(/^\/JamAssets\//, '');
    return existsSync(resolve(root, 'Game/Plugins/JamAssets/Content', pkg + '.uasset'));
  };
}

// Checks a parsed manifest. soundExists(path) answers whether a sound asset path ("/JamAssets/Audio/VO/X.X") is on disk; the default looks under
// Game/Plugins/JamAssets/Content in root. Returns { errors: ['at <pointer>: ...'], infos: [...], lines, situations }.
export function checkManifest(manifest, { root = repo, soundExists } = {}) {
  const errors = [];
  const infos = [];
  const exists = soundExists ?? defaultSoundExists(root);
  if (!manifest || typeof manifest !== 'object' || Array.isArray(manifest)) return { errors: ['at /: the manifest must be an object'], infos, lines: 0, situations: 0 };
  for (const k of Object.keys(manifest)) if (!TOP_KEYS.includes(k)) errors.push(`at /${k}: unknown key`);
  if (manifest.schema !== SCHEMA) errors.push(`at /schema: must be "${SCHEMA}"`);
  if (!Array.isArray(manifest.lines)) { errors.push('at /lines: must be an array'); return { errors, infos, lines: 0, situations: 0 }; }
  if (manifest.count !== undefined && manifest.count !== manifest.lines.length) errors.push(`at /count: ${manifest.count} but there are ${manifest.lines.length} lines`);
  const ids = new Set();
  const situations = new Set();
  manifest.lines.forEach((l, i) => {
    const at = `at /lines/${i}`;
    if (!l || typeof l !== 'object' || Array.isArray(l)) { errors.push(`${at}: must be an object`); return; }
    for (const k of Object.keys(l)) if (!KEYS.includes(k)) errors.push(`${at}/${k}: unknown key`);
    for (const k of KEYS) if (!(k in l)) errors.push(`${at}: missing key "${k}"`);
    for (const k of ['id', 'event', 'kind', 'team', 'when', 'text', 'sound']) if (k in l && typeof l[k] !== 'string') errors.push(`${at}/${k}: must be a string`);
    if (typeof l.id === 'string') {
      if (!/^[a-z][a-z0-9_]*(\.[a-z0-9_]+)+\.\d{2}$/.test(l.id)) errors.push(`${at}/id: "${l.id}" is not <event>.<situation>.<nn>`);
      else if (l.id.split('.')[0] !== l.event) errors.push(`${at}/id: first segment "${l.id.split('.')[0]}" is not the event "${l.event}"`);
      if (ids.has(l.id)) errors.push(`${at}/id: duplicate id ${l.id}`);
      ids.add(l.id);
      situations.add(situationOf(l.id));
    }
    if (typeof l.event === 'string' && !KNOWN_EVENTS.includes(l.event)) errors.push(`${at}/event: "${l.event}" is not an event the game sends`);
    if (!TEAMS.includes(l.team)) errors.push(`${at}/team: must be blue, orange or any`);
    if (!Number.isInteger(l.priority) || l.priority < 1 || l.priority > 5) errors.push(`${at}/priority: must be an integer 1..5`);
    if (typeof l.cooldown_s !== 'number' || !Number.isFinite(l.cooldown_s) || l.cooldown_s < 0) errors.push(`${at}/cooldown_s: must be a number of 0 or more`);
    if (typeof l.when === 'string' && !l.when.trim()) errors.push(`${at}/when: must not be empty`);
    if (typeof l.seconds !== 'number' || !(l.seconds > 0)) errors.push(`${at}/seconds: must be a positive number`);
    if (typeof l.text === 'string') {
      if (/\d/.test(l.text)) errors.push(`${at}/text: digits are spelled out in the booth (README section 1)`);
      const v = checkText(l);
      for (const r of v.reasons) errors.push(`${at}/text: ${r}`);
      for (const r of v.infos) infos.push(`${l.id}: ${r}`);
    }
    if (typeof l.sound === 'string' && typeof l.id === 'string') {
      const name = `VO_Commentator_${l.id.replace(/\./g, '_')}`;
      const expected = `/JamAssets/Audio/VO/${name}.${name}`;
      if (l.sound !== expected) errors.push(`${at}/sound: expected ${expected}`);
      else if (!exists(l.sound)) errors.push(`${at}/sound: ${l.sound} has no .uasset on disk`);
    }
  });
  return { errors, infos, lines: manifest.lines.length, situations: situations.size };
}

if (process.argv[1] && /check\.mjs$/.test(process.argv[1].replace(/\\/g, '/'))) {
  const file = process.argv[2] ?? resolve(repo, 'Game/Plugins/JamAssets/Content/Audio/VO/commentary.json');
  let manifest;
  try { manifest = JSON.parse(readFileSync(file, 'utf8')); } catch (e) {
    console.log(`FAIL read ${file}: ${e.message}`);
    console.log(JSON.stringify({ tool: 'thirdjam.commentary.check/1', ok: false, could_not_run: true }));
    process.exit(2);
  }
  const r = checkManifest(manifest);
  for (const e of r.errors.slice(0, 40)) console.log(`ERROR ${e}`);
  for (const i of r.infos) console.log(`INFO ${i}`);
  console.log(`${r.errors.length ? 'FAIL' : 'PASS'} commentary check ${r.lines} lines, ${r.situations} situations, ${r.errors.length} errors, ${r.infos.length} allowances`);
  console.log(JSON.stringify({ tool: 'thirdjam.commentary.check/1', ok: r.errors.length === 0, lines: r.lines, situations: r.situations, errors: r.errors.length, allowances: r.infos.length }));
  process.exit(r.errors.length ? 1 : 0);
}
