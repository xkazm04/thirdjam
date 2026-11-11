// Turn the raw inputs into one model: per-hour buckets for the ruler, per-hour entries for the
// journal, outside-the-window buckets, and the summary strip. Pure function: same inputs, same model.
import { RULER_HOURS, DEVICES } from './constants.mjs';
import { hoursSinceH0 } from './time.mjs';

const emptyDev = () => ({ turns: 0, prompts: 0, commits: 0, shots: 0, outTokens: 0 });

function bucketOf(h) {
  if (h < 0) return 'pre';
  if (h >= RULER_HOURS) return 'post';
  return Math.floor(h);
}

export function buildModel({ commits = [], sessions = [], shots = [], notes = [], status = [], gates = [] }) {
  const ruler = Array.from({ length: RULER_HOURS }, () => Object.fromEntries(DEVICES.map((d) => [d, emptyDev()])));
  const outside = {
    pre: { commits: [], shots: [], turns: 0, prompts: 0, sessions: new Set() },
    post: { commits: [], shots: [], turns: 0, prompts: 0, sessions: new Set() },
  };
  const journal = new Map(); // hour bucket (int, may exceed ruler for notes) -> entry
  const entry = (b) => {
    if (!journal.has(b)) journal.set(b, { hour: b, notes: [], shots: [], commits: [], status: [], gates: [] });
    return journal.get(b);
  };
  const dev = (d) => (DEVICES.includes(d) ? d : 'A');

  const tokens = { input: 0, output: 0, cacheRead: 0, cacheWrite: 0, any: false };
  let turns = 0;
  let prompts = 0;
  let lastMs;
  let firstMs;
  const touch = (ms) => {
    if (lastMs === undefined || ms > lastMs) lastMs = ms;
    if (firstMs === undefined || ms < firstMs) firstMs = ms;
  };

  sessions.forEach((s, si) => {
    for (const t of s.turns) {
      turns++;
      touch(t.ms);
      const u = t.usage;
      if (u.input || u.output || u.cacheRead || u.cacheWrite) tokens.any = true;
      tokens.input += u.input;
      tokens.output += u.output;
      tokens.cacheRead += u.cacheRead;
      tokens.cacheWrite += u.cacheWrite;
      const b = bucketOf(hoursSinceH0(t.ms));
      if (typeof b === 'number') {
        const c = ruler[b][dev(s.device)];
        c.turns++;
        c.outTokens += u.output;
      } else {
        outside[b].turns++;
        outside[b].sessions.add(si);
      }
    }
    for (const p of s.prompts) {
      prompts++;
      touch(p.ms);
      const b = bucketOf(hoursSinceH0(p.ms));
      if (typeof b === 'number') ruler[b][dev(s.device)].prompts++;
      else outside[b].prompts++;
    }
  });

  for (const c of commits) {
    touch(c.ms);
    const b = bucketOf(hoursSinceH0(c.ms));
    if (typeof b === 'number') {
      ruler[b][dev(c.device)].commits++;
      entry(b).commits.push(c);
    } else outside[b].commits.push(c);
  }

  for (const s of shots) {
    touch(s.ms);
    const b = bucketOf(hoursSinceH0(s.ms));
    if (typeof b === 'number') {
      ruler[b][dev(s.device)].shots++;
      entry(b).shots.push(s);
    } else outside[b].shots.push(s);
  }

  for (const n of notes) entry(Math.floor(n.hour)).notes.push(n);
  for (const st of status) entry(Math.floor(st.hour)).status.push(st);
  for (const g of gates) entry(Math.floor(g.hour)).gates.push(g);

  const noteHours = new Set(notes.map((n) => Math.floor(n.hour)));
  const statusHours = new Set(status.map((n) => Math.floor(n.hour)));

  const hours = [...journal.keys()].sort((a, b) => a - b).map((k) => journal.get(k));
  for (const h of hours) {
    h.shots.sort((a, b) => a.ms - b.ms);
    h.commits.sort((a, b) => a.ms - b.ms);
    h.perDevice = typeof h.hour === 'number' && h.hour < RULER_HOURS ? ruler[h.hour] : undefined;
  }

  const sessionCount = { main: 0, subagent: 0 };
  for (const s of sessions) sessionCount[s.kind === 'subagent' ? 'subagent' : 'main']++;

  const peak = Object.fromEntries(DEVICES.map((d) => [d, Math.max(0, ...ruler.map((r) => r[d].turns))]));

  return {
    ruler,
    peak,
    hours,
    outside,
    noteHours,
    statusHours,
    gates,
    sessions,
    summary: {
      commits: commits.length,
      sessions: sessionCount,
      turns,
      prompts,
      shots: shots.length,
      tokens,
      firstMs,
      lastMs,
      asOfHour: lastMs === undefined ? undefined : hoursSinceH0(lastMs),
    },
  };
}
