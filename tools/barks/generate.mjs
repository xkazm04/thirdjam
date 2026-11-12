// Lane A12: generates the bark line bank through the CLI transport pattern of ruling R-0004 (`claude -p`, haiku first, sonnet on request),
// filters every line with filter.mjs, writes Game/Content/Barks/barks.json and a review file the Director skims, then runs embed.mjs so the
// C++ copy (Game/Source/JamGame/Private/BarkData.inl) matches the committed bank.
//   node tools/barks/generate.mjs [--model claude-haiku-5-5|claude-sonnet-5-5] [--per 14] [--concurrency 4] [--only goal_for,win] [--keep]
// --keep merges into the existing bank instead of replacing it (used to top up a situation that came out short).
// The whole prompt is ours (no settings, no tools, no MCP); the model's text never reaches the player: only lines that pass the filter are kept,
// and the bank is a committed data file read by UJamBarkSystem.

import { existsSync, mkdirSync, readFileSync, writeFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { spawn } from 'node:child_process';
import { tmpdir } from 'node:os';
import { findClaude, cliArgs } from '../designer-eval/cli-model.mjs';
import { filterBank, stats, MAX_CHARS } from './filter.mjs';
import { embed } from './embed.mjs';

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..');
const bankPath = resolve(repo, 'Game/Content/Barks/barks.json');
const reviewPath = resolve(repo, 'Game/Content/Barks/REVIEW.md');

export const SITUATIONS = {
  goal_for: 'the human player\'s team just scored a goal',
  goal_against: 'the opposing team just scored a goal against the human player\'s team',
  check_landed: 'the human player just body-checked someone and knocked them down',
  check_received: 'the human player just got body-checked and knocked down',
  steal: 'the puck just changed hands: the human player lost it to an opponent, or took it from one',
  stall: 'nobody has done anything useful for a while; the puck is stuck in a corner or everyone is circling',
  near_miss: 'a shot just rang off the post or the boards right next to the net, no goal',
  faceoff: 'a faceoff at centre ice is about to start (match start or after a goal)',
  win: 'the match is over and the human player\'s team won',
  lose: 'the match is over and the human player\'s team lost',
  directors_call: 'the human player just paused the match and changed one rule live (ice friction, shot power, rink size...) and the scoreboard announced the new rule; speak of "the rules changed", "new rule", "your tweak", never of directors, cuts or scenes',
  signal_ack_defend: 'the human player just shouted "defend!" and this teammate confirms they are falling back to protect the net',
  signal_ack_space: 'the human player just shouted "space!" and this teammate confirms they are skating to open ice to be passable',
  signal_ack_pass: 'the human player just shouted "pass!" and this teammate confirms they will pass the puck to the human',
  praise_reply: 'the human player just complimented this teammate ("nice one!") and the teammate answers',
};
export const SPEAKERS = ['opponent', 'teammate'];
export const TONES = { opponent: ['mock', 'savage', 'complain', 'bravado'], teammate: ['encourage', 'complain', 'praise'] };
const TEAMMATE_ONLY = new Set(['signal_ack_defend', 'signal_ack_space', 'signal_ack_pass', 'praise_reply']);

const TONE_HINT = {
  mock: 'mock the human player: cutting sarcasm about their skating, aim, speed, courage or decisions; dry, deadpan, bored-veteran voice',
  savage: 'go for the throat: merciless gloating and ridicule of how badly the human plays, the harshest clean trash talk a bored veteran uses; a few lines may use damn, hell, crap or "kick your ass"',
  complain: 'genuinely annoyed, bitter complaining about what just happened (the ice, the luck, the bounce, the human) in a tired-veteran voice, never whiny or cute',
  bravado: 'cold, arrogant gloating about how much better your line is; contempt, not cheer',
  encourage: 'a gruff veteran teammate pushing the human to do better: blunt, impatient, no sugar',
  praise: 'grudging respect from a veteran teammate: dry, understated, a little surprised the human managed it',
};

export function systemPrompt() {
  return `You write short in-game one-liners ("barks") for cartoon toy-figure hockey players in a 3-on-3 arcade ice hockey game called Hokej.
The barks are hockey first (ruling R-0021): rink talk only. The game's menu has a film theme, but on the ice nobody says cut, take, director, script, scene,
reel, blooper, rehearsal, camera, movie, wrap, credits or any film-set word; the lines are rejected. Lean on shift, line, bench, boards, glass, crease, slot,
blue line, faceoff, forecheck, backcheck, rebound, one-timer, snipe, dangle, deke, top shelf, five-hole, pylon, turnover, warm-up, scrimmage, playoff, season.
The game has no goalies, no referees, no crowd, no penalties and no money: never mention them. It has nets, boards, posts, pucks, sticks, skates, ice,
body checks, pickups, a scoreboard, a radar and the human's live rule changes (speak of "the rules changed", "new rule", "your tweak").
Register: harsh. These are not kids'-show lines. The speakers are bored veterans who have seen a thousand rookies and find this one funny: cutting sarcasm
about the human's skating, aim, speed, courage and decisions; gloating; cold contempt; teammates who are genuinely annoyed. Never cute, never "golly".
Rules (every line is filtered by a machine and a human reviewer; broken lines are thrown away):
- Teen-appropriate (13+). Mild language is allowed in about one line in ten and never two in one line: damn, hell, crap, "kick your ass". Nothing stronger.
  No slurs, no sexual content, no threats of real harm (gloating about the game's own body checks is fine), nothing about who the player is (body, mind,
  gender, age, family, origin, faith, looks): only about how they play. No alcohol, drugs, money, politics, religion.
- No real people, brands, teams, leagues, countries or cities. No capitalised words except the first word of a sentence and "I". No emoji, no quotes,
  no markup, no hashtags, no numbers above 99.
- English only, plain ASCII, ${MAX_CHARS} characters at most per line, one sentence, spoken aloud by a skater during play. Each line must be different in
  wording and idea from the others; vary rhythm (questions, two-word jabs, longer quips).
- Examples of the voice: "back to the bench, rookie" / "nice stick, shame about the aim" / "you skate like the ice owes you an apology" /
  "aim for the net next time, it's the big thing with the posts" / "I've seen pucks make braver decisions" / "new rule, same ending, same excuse" /
  "damn, even the boards felt sorry for that one"
Output: JSON only, no prose, no code fence: {"lines": {"<tone>": ["line", ...], ...}} with exactly the tones asked for.`;
}

export function userPrompt(situation, speaker, tones, per) {
  const who = speaker === 'opponent' ? 'an OPPONENT skater speaking to or about the human player' : 'a TEAMMATE skater on the human player\'s team speaking to the human player';
  return `Situation: ${SITUATIONS[situation]}.\nSpeaker: ${who}.\nWrite ${per} lines for each of these tones:\n` +
    tones.map((t) => `- ${t}: ${TONE_HINT[t]}`).join('\n') + `\nReturn {"lines": {${tones.map((t) => `"${t}": [...]`).join(', ')}}}.`;
}

function runCli(exe, model, systemFile, user, timeoutMs = 120000) {
  const args = cliArgs(model, 'low', systemFile);
  const shell = /\.(cmd|bat)$/i.test(exe);
  const workDir = resolve(tmpdir(), 'hokej-barks-cli');
  mkdirSync(workDir, { recursive: true });
  return new Promise((done) => {
    const child = spawn(exe, args, { cwd: workDir, shell, windowsHide: true });
    let out = ''; let err = ''; let killed = false;
    const timer = setTimeout(() => { killed = true; child.kill(); }, timeoutMs);
    child.stdout.on('data', (d) => { out += d; });
    child.stderr.on('data', (d) => { err += d; });
    child.on('error', (e) => { clearTimeout(timer); done({ code: -1, stdout: out, stderr: String(e.message), timedOut: false }); });
    child.on('close', (c) => { clearTimeout(timer); done({ code: c, stdout: out, stderr: err, timedOut: killed }); });
    child.stdin.end(user);
  });
}

export function parseReply(text) {
  let t = String(text ?? '').trim();
  const fence = t.match(/```(?:json)?\s*([\s\S]*?)```/);
  if (fence) t = fence[1].trim();
  const start = t.indexOf('{');
  if (start > 0) t = t.slice(start);
  const data = JSON.parse(t);
  if (!data || typeof data.lines !== 'object') throw new Error('reply has no "lines" object');
  return data.lines;
}

async function main() {
  const arg = (k, d) => { const i = process.argv.indexOf(`--${k}`); return i >= 0 ? process.argv[i + 1] : d; };
  const model = arg('model', 'claude-haiku-5-5');
  const per = Number(arg('per', '14'));
  const concurrency = Number(arg('concurrency', '4'));
  const only = arg('only', '') ? arg('only', '').split(',') : null;
  const keep = process.argv.includes('--keep');
  const exe = findClaude();
  if (!exe) { console.log('FAIL claude executable not on PATH'); process.exit(2); }

  const systemFile = resolve(tmpdir(), 'hokej-barks-cli', 'system.txt');
  mkdirSync(dirname(systemFile), { recursive: true });
  writeFileSync(systemFile, systemPrompt());

  const jobs = [];
  for (const situation of Object.keys(SITUATIONS)) {
    if (only && !only.includes(situation)) continue;
    for (const speaker of SPEAKERS) {
      if (TEAMMATE_ONLY.has(situation) && speaker !== 'teammate') continue;
      jobs.push({ situation, speaker, tones: TONES[speaker] });
    }
  }
  const raw = [];
  const errors = [];
  let usage = { in: 0, out: 0, cost: 0, ms: 0, calls: 0 };
  let next = 0;
  async function worker() {
    while (next < jobs.length) {
      const j = jobs[next++];
      const started = performance.now();
      const r = await runCli(exe, model, systemFile, userPrompt(j.situation, j.speaker, j.tones, per));
      const ms = Math.round(performance.now() - started);
      let data;
      try { data = JSON.parse(r.stdout); } catch { data = undefined; }
      if (r.timedOut || !data || data.is_error || r.code !== 0) {
        errors.push(`${j.situation}/${j.speaker}: exit ${r.code}${r.timedOut ? ' timeout' : ''} ${String(data?.result ?? r.stderr).slice(0, 160)}`);
        console.log(`FAIL ${j.situation}/${j.speaker} (${ms} ms)`);
        continue;
      }
      usage.calls += 1; usage.ms += ms; usage.cost += data.total_cost_usd ?? 0;
      usage.in += (data.usage?.input_tokens ?? 0) + (data.usage?.cache_read_input_tokens ?? 0) + (data.usage?.cache_creation_input_tokens ?? 0);
      usage.out += data.usage?.output_tokens ?? 0;
      let lines;
      try { lines = parseReply(data.result); } catch (e) { errors.push(`${j.situation}/${j.speaker}: ${e.message}`); console.log(`FAIL ${j.situation}/${j.speaker} parse`); continue; }
      let n = 0;
      for (const tone of j.tones) {
        for (const text of (Array.isArray(lines[tone]) ? lines[tone] : [])) { raw.push({ situation: j.situation, speaker: j.speaker, tone, text: String(text) }); n += 1; }
      }
      console.log(`PASS ${j.situation}/${j.speaker} ${n} lines (${ms} ms)`);
    }
  }
  await Promise.all(Array.from({ length: Math.min(concurrency, jobs.length) }, worker));

  let previous = [];
  if (keep && existsSync(bankPath)) previous = JSON.parse(readFileSync(bankPath, 'utf8')).lines;
  const { kept, rejected, mildKept } = filterBank([...previous, ...raw]);
  // Stable ids: situation.speaker.tone.NN in bank order.
  const counters = {};
  for (const l of kept) {
    const k = `${l.situation}.${l.speaker}.${l.tone}`;
    counters[k] = (counters[k] ?? 0) + 1;
    l.id = `${k}.${String(counters[k]).padStart(2, '0')}`;
  }
  kept.sort((a, b) => a.id.localeCompare(b.id));
  const bank = {
    schema: 'hokej.barks/1',
    note: 'Reviewed bark bank (RULINGS R-0007 point 1). Generated by tools/barks/generate.mjs through the Claude Code CLI, every line passed tools/barks/filter.mjs. Edit by hand if you must, then run node tools/barks/embed.mjs. Situations are seen from the human player; speakers are opponent (mock, complain, bravado) and teammate (encourage, complain, praise).',
    generated: new Date().toISOString(),
    model,
    counts: counters,
    lines: kept.map(({ id, situation, speaker, tone, text }) => ({ id, situation, speaker, tone, text })),
  };
  mkdirSync(dirname(bankPath), { recursive: true });
  writeFileSync(bankPath, JSON.stringify(bank, null, 2) + '\n');

  // Review file: every kept line grouped, then the rejections with reasons.
  let md = `# Bark bank review (generated ${bank.generated}, model ${model})\n\nSkim every line; delete any you dislike from barks.json and run node tools/barks/embed.mjs.\n` +
    `Generated ${previous.length + raw.length} lines, kept ${kept.length} (${mildKept} with a mild word, quota one in ten), rejected ${rejected.length}. Reasons: ${JSON.stringify(stats(rejected))}\n\n`;
  let group = '';
  for (const l of kept) {
    const g = `${l.situation} / ${l.speaker} / ${l.tone}`;
    if (g !== group) { group = g; md += `\n## ${g} (${counters[`${l.situation}.${l.speaker}.${l.tone}`]})\n`; }
    md += `- ${l.text}\n`;
  }
  md += `\n## Rejected (${rejected.length})\n`;
  for (const r of rejected) md += `- ${r.situation}/${r.speaker}/${r.tone}: ${JSON.stringify(r.text)} -- ${r.reasons[0]}\n`;
  if (errors.length) md += `\n## Errors (${errors.length})\n` + errors.map((e) => `- ${e}`).join('\n') + '\n';
  writeFileSync(reviewPath, md);

  const short = Object.entries(counters).filter(([, n]) => n < 12).map(([k, n]) => `${k}=${n}`);
  embed();
  console.log(`${errors.length ? 'WARN' : 'PASS'} generate: ${kept.length} kept, ${rejected.length} rejected, ${usage.calls} calls, ${Math.round(usage.ms / 1000)} s, $${usage.cost.toFixed(3)}, in ${usage.in} out ${usage.out} tokens; short combos: ${short.length ? short.join(' ') : 'none'}`);
  console.log(JSON.stringify({ tool: 'thirdjam.barks.generate/1', ok: errors.length === 0, kept: kept.length, mild: mildKept, rejected: rejected.length, calls: usage.calls, cost_usd: usage.cost, reasons: stats(rejected), short, errors }));
  process.exit(errors.length ? 1 : 0);
}

if (process.argv[1] && /generate\.mjs$/.test(process.argv[1].replace(/\\/g, '/'))) main();
