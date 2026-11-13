#!/usr/bin/env node
// Aggregator for the playtester soak (lane A53): reads <out>/results/**.json and prints the class x level table (Markdown), the fault list with reproduction
// commands, and a JSON line last.
//
//   node tools/soak/playtest/aggregate.mjs [--out tools/soak/playtest/out] [--md <file>] [--slack 10]
//
// Targets (R-0029 describes hockey.regular) are compared on the hockey.regular rows only; the boost and pooled rows are information.
// Win % is the human side's wins over finished-or-not matches (a draw or an unfinished match is not a win), with the 95 % Wilson interval. Targets are R-0029 point 1:
// novice 85 at rookie, regular 50 at pro, good 25 at all_star; the other cells have no target. A flag where the interval misses [target - slack, target + slack]
// would be too lenient for a statement about the game, so the flag is on the interval alone: MISS when the interval excludes the target, and the distance of the
// nearest interval end is printed.
import { readdirSync, readFileSync, writeFileSync, existsSync } from 'node:fs';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const here = dirname(fileURLToPath(import.meta.url));
function arg(name, dflt) { const i = process.argv.indexOf(`--${name}`); return i > 0 && i + 1 < process.argv.length ? process.argv[i + 1] : dflt; }
const out = resolve(arg('out', join(here, 'out')));
const mdOut = arg('md', '');
export const TARGET = { 'novice/rookie': 85, 'regular/pro': 50, 'good/all_star': 25 };
const CLASSES = ['novice', 'regular', 'good'];
const LEVELS = ['rookie', 'pro', 'all_star'];

export function load(dir) {
  const rows = [];
  const base = join(dir, 'results');
  if (!existsSync(base)) return rows;
  for (const cart of readdirSync(base)) for (const f of readdirSync(join(base, cart))) {
    if (!f.endsWith('.json')) continue;
    try { rows.push(JSON.parse(readFileSync(join(base, cart, f), 'utf8'))); } catch { /* being written */ }
  }
  return rows;
}
export function wilson(k, n, z = 1.96) {
  if (!n) return [0, 100];
  const p = k / n, d = 1 + z * z / n, c = p + z * z / (2 * n), s = z * Math.sqrt(p * (1 - p) / n + z * z / (4 * n * n));
  return [100 * (c - s) / d, 100 * (c + s) / d];
}
const avg = (a) => (a.length ? a.reduce((s, x) => s + x, 0) / a.length : 0);
const r1 = (x) => (Math.round(x * 10) / 10).toString();
const secs = (f) => f / 60;
const quantile = (a, q) => { if (!a.length) return -1; const s = [...a].sort((x, y) => x - y); return s[Math.min(s.length - 1, Math.floor(q * s.length))]; };

export function cells(rows) {
  const out = [];
  for (const lvl of LEVELS) for (const cls of CLASSES) {
    for (const cart of ['hockey.regular', 'hockey.boost', null]) {
      const ms = rows.filter((r) => r.m && r.cls === cls && r.lvl === lvl && (!cart || r.cart === cart)).map((r) => r.m);
      if (!ms.length) continue;
      const n = ms.length, wins = ms.filter((m) => m.result === 'win').length;
      const [lo, hi] = wilson(wins, n);
      const key = `${cls}/${lvl}`;
      const t = cart === 'hockey.regular' ? TARGET[key] : undefined;
      out.push({
        cls, lvl, cart: cart || 'all', n, wins, losses: ms.filter((m) => m.result === 'loss').length, draws: ms.filter((m) => m.result === 'draw').length,
        unfinished: ms.filter((m) => !m.over).length, win_pct: 100 * wins / n, lo, hi, target: t,
        flag: t === undefined ? '' : (lo > t ? `ABOVE +${r1(lo - t)}` : hi < t ? `BELOW -${r1(t - hi)}` : 'ok'),
        gf: avg(ms.map((m) => m.goals[0])), ga: avg(ms.map((m) => m.goals[1])), touches: avg(ms.map((m) => m.touches)), tpm: avg(ms.map((m) => m.touches_per_min)),
        shots: avg(ms.map((m) => m.shots)), sot: avg(ms.map((m) => m.shots_on_target)), passes: avg(ms.map((m) => m.passes)), rcv: avg(ms.map((m) => m.passes_received || 0)),
        cl: avg(ms.map((m) => m.checks_landed)), ct: avg(ms.map((m) => m.checks_taken)), ot: avg(ms.map((m) => m.one_timers || 0)), dk: avg(ms.map((m) => m.dekes || 0)),
        ba: avg(ms.map((m) => m.breakaways || 0)), len: secs(avg(ms.map((m) => m.frames))), len_p95: secs(quantile(ms.map((m) => m.frames), 0.95)),
        scored2: ms.filter((m) => m.first_goal_by_human >= 0 && m.first_goal_by_human <= 7200).length,
        fo_no_touch: ms.reduce((s, m) => s + m.faceoffs_without_touch, 0), max_idle: secs(Math.max(...ms.map((m) => m.max_no_touch))),
        idle15: ms.filter((m) => m.idle_spells > 0).length,
      });
    }
  }
  return out;
}

export function table(cs) {
  const head = ['class', 'level', 'cart', 'n', 'W-L-D-unf', 'win %', '95 % interval', 'R-0029 target', 'flag', 'GF / GA', 'touches (per min)', 'shots (on tgt)', 'passes (rcv)', 'checks L / T', 'one-timers / dekes / breakaways', 'scored <2 min', 'match s (p95)'];
  const body = cs.map((c) => [c.cls, c.lvl, c.cart, c.n, `${c.wins}-${c.losses}-${c.draws}-${c.unfinished}`, c.cart === 'hockey.regular' ? `**${r1(c.win_pct)}**` : r1(c.win_pct), `${r1(c.lo)} - ${r1(c.hi)}`,
    c.target ?? '', c.flag, `${r1(c.gf)} / ${r1(c.ga)}`, `${r1(c.touches)} (${r1(c.tpm)})`, `${r1(c.shots)} (${r1(c.sot)})`, `${r1(c.passes)} (${r1(c.rcv)})`, `${r1(c.cl)} / ${r1(c.ct)}`,
    `${r1(c.ot)} / ${r1(c.dk)} / ${r1(c.ba)}`, `${c.scored2} of ${c.n}`, `${r1(c.len)} (${r1(c.len_p95)})`].map(String));
  const line = (x) => `| ${x.join(' | ')} |`;
  return [line(head), `|${head.map(() => '---').join('|')}|`, ...body.map(line)].join('\n');
}

const reproduce = (r) => `node tools/playtest/run.mjs --players ${r.cls} --levels ${r.lvl} --rinks ${r.cart === 'hockey.regular' ? 'regular' : r.cart} --seeds ${r.seed}-${r.seed}`;
// The same one-process form without run.mjs's rink aliases (any cartridge name is a rink there, so this works for hockey.boost too).

export function faults(rows) {
  const groups = new Map();
  const add = (kind, key, r, detail) => {
    const k = `${kind}|${key}|${r.cart}`;
    if (!groups.has(k)) groups.set(k, { kind: `${kind} (${r.cart.replace('hockey.', '')})`, key, count: 0, first: r, detail, cells: new Set() });
    const g = groups.get(k); g.count++; g.cells.add(`${r.cls}/${r.lvl}/${r.cart.replace('hockey.', '')}`);
  };
  for (const r of rows) {
    if (r.crash) add('crash', r.crash.head.slice(0, 120), r, `exit ${r.crash.exit}: ${r.crash.head}`);
    for (const f of r.faults || []) add(f.kind, f.line.replace(/0x[0-9a-f]+/gi, '0x_').replace(/\d+/g, 'N').slice(0, 120), r, f.line);
    const m = r.m; if (!m) continue;
    if (m.touches === 0) add('zero_human_touches', 'touches == 0', r, `0 touches in ${m.frames} frames`);
    if (!m.over) add('overtime_without_end', `unfinished at the ${m.frames}-frame cap`, r, `not over after ${m.frames} frames, score ${m.goals.join(':')}`);
    if (m.max_no_touch >= 1800) add('human_idle_30s', 'max_no_touch >= 1800 frames', r, `${m.max_no_touch} frames without a human touch`);
    // stuck puck: no shot by either side and no pass by the human for 20 s: the match totals cannot show a window, so the proxy is a whole match without a shot or a pass
    if (m.shots + m.shots_opp === 0 && m.passes === 0 && m.frames > 1200) add('stuck_puck_proxy', 'no shot, no pass in the whole match', r, `${m.frames} frames, 0 shots, 0 passes`);
    if (m.first_shot < 0 && m.shots_opp === 0 && m.over && m.frames > 1200) add('no_shot_either_side', 'no shot by either side', r, `${m.frames} frames`);
  }
  return [...groups.values()].sort((a, b) => b.count - a.count);
}

export function faultTable(fs) {
  if (!fs.length) return '_None found._';
  const head = ['kind', 'count', 'first line / detail', 'cells', 'reproduce (first seed)'];
  const body = fs.map((g) => [g.kind, g.count, `\`${g.detail.replace(/\|/g, '/').slice(0, 160)}\``, [...g.cells].slice(0, 4).join(', '), `\`${reproduce(g.first)}\` (seed ${g.first.seed})`]);
  return [`| ${head.join(' | ')} |`, `|${head.map(() => '---').join('|')}|`, ...body.map((x) => `| ${x.join(' | ')} |`)].join('\n');
}

if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  const rows = load(out);
  const cs = cells(rows);
  const fs = faults(rows);
  const md = `${table(cs)}\n\n${faultTable(fs)}\n`;
  if (mdOut) writeFileSync(mdOut, md);
  console.log(md);
  const matches = rows.filter((r) => r.m).length;
  console.log(JSON.stringify({ tool: 'thirdjam.soak.aggregate/1', verdict: 'pass', matches, crashes: rows.filter((r) => r.crash).length, fault_groups: fs.length, flags: cs.filter((c) => c.flag.startsWith('A') || c.flag.startsWith('B')).map((c) => `${c.cls}/${c.lvl} ${c.flag}`) }));
}
