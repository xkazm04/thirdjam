#!/usr/bin/env node
// Lane A56: tabulates the lockstep soak. Reads thirdjam.soaklock/1 lines (one per match), .mismatch/1 and .timing/1 lines from the .jsonl files
// Bar.Net.Soak* wrote (tools/soak/lockstep/run.ps1 -Out), and prints the markdown tables docs/experiments/SOAK-LOCKSTEP.md carries.
// Usage: node tools/soak/lockstep/summarize.mjs <file.jsonl | dir>... [--doc docs/experiments/SOAK-LOCKSTEP.md]
// --doc replaces the text between <!-- soak:tables --> and <!-- /soak:tables --> in that file (the hourly refresh).
// Prints the tables, then a thirdjam.soaklock.report/1 JSON line last. Exit 0 always when it could read (a desync is data here, not an error).
import { readFileSync, writeFileSync, existsSync, readdirSync, statSync } from 'node:fs';
import { join } from 'node:path';
import { pathToFileURL } from 'node:url';

export function parseLines(text) {
  const out = [];
  for (const line of text.split(/\r?\n/)) {
    const s = line.trim();
    if (!s.startsWith('{')) continue;
    try { out.push(JSON.parse(s)); } catch { /* a torn last line while a run is still writing */ }
  }
  return out;
}

export function summarize(rows) {
  const matches = rows.filter((r) => r.schema === 'thirdjam.soaklock/1');
  const mism = rows.filter((r) => r.schema === 'thirdjam.soaklock.mismatch/1');
  const timing = rows.filter((r) => r.schema === 'thirdjam.soaklock.timing/1');
  const byProfile = new Map();
  for (const m of matches) {
    const key = m.profile;
    if (!byProfile.has(key)) byProfile.set(key, { profile: key, seeds: new Set(), matches: 0, desyncs: 0, drifts: 0, lost: 0, unfinished: 0, frames: 0, ticks: 0, stalls: 0, maxStall: 0, maxGap: 0, maxRun: 0, dropped: 0, dup: 0, reordered: 0, edges: {}, edgeMatches: 0 });
    const p = byProfile.get(key);
    p.matches++; p.seeds.add(m.seed);
    const diff = m.desync || m.first_diff_frame >= 0;
    if (diff) p.desyncs++;
    if (m.first_diff_frame >= 0 && m.first_lockhash_diff < 0) p.drifts++;
    if (m.lost) p.lost++;
    if (!m.lost && !diff && (m.frames[0] !== m.frames[1] || m.agreed[0] < m.expected || m.agreed[1] < m.expected || !m.final_equal)) p.unfinished++;
    p.frames += Math.min(m.frames[0], m.frames[1]); p.ticks += m.ticks; p.stalls += m.stalls[0] + m.stalls[1];
    p.maxStall = Math.max(p.maxStall, m.max_stall); p.maxGap = Math.max(p.maxGap, m.max_gap); p.maxRun = Math.max(p.maxRun, m.max_loss_run);
    p.dropped += m.dropped + m.blacked; p.dup += m.dup; p.reordered += m.reordered;
    let lostEdge = false;
    for (const [k, [pressed, lost]] of Object.entries(m.edges || {})) {
      const e = (p.edges[k] ||= [0, 0]); e[0] += pressed; e[1] += lost; if (lost) lostEdge = true;
    }
    if (lostEdge) p.edgeMatches++;
  }
  const desyncRows = matches.filter((m) => m.desync || m.first_diff_frame >= 0).map((m) => ({
    profile: m.profile, seed: m.seed, first_diff_frame: m.first_diff_frame, field: m.first_diff_field, lockhash_from: m.first_lockhash_diff, driver: m.desync ? m.driver_desync_frame : -1, dump: m.dump || '',
  }));
  return { matches: matches.length, profiles: [...byProfile.values()].map((p) => ({ ...p, seeds: p.seeds.size })), desyncRows, mism, timing };
}

function edgeText(edges) {
  return Object.entries(edges).map(([k, [p, l]]) => `${k} ${l}/${p}`).join(', ');
}

export function markdown(s) {
  const L = [];
  L.push(`Matches: **${s.matches}**.`, '');
  L.push('### Loss table (per fault profile)', '');
  L.push('| profile | matches | desync | lost | unfinished | speed (frames per tick) | stalled attempts | worst stall (ticks) | max peer gap | longest loss run | datagrams lost / dup / reordered | edge presses lost (lost/pressed) |');
  L.push('|---|---|---|---|---|---|---|---|---|---|---|---|');
  for (const p of s.profiles) {
    L.push(`| ${p.profile} | ${p.matches} | ${p.desyncs}${p.drifts ? ` (${p.drifts} silent)` : ''} | ${p.lost} | ${p.unfinished} | ${(p.ticks ? p.frames / p.ticks : 0).toFixed(3)} | ${p.stalls} | ${p.maxStall} | ${p.maxGap} | ${p.maxRun} | ${p.dropped} / ${p.dup} / ${p.reordered} | ${edgeText(p.edges)} |`);
  }
  L.push('', '### Desync table', '');
  // corrupt-* profiles flip bytes on purpose (no checksum in the packet): their desyncs are the expected outcome, listed apart from the honest ones.
  const meant = s.desyncRows.filter((d) => d.profile.startsWith('corrupt'));
  const honest = s.desyncRows.length - meant.length;
  L.push(honest ? `**${honest} desync(s) in honest profiles.**` : 'No desync in any honest profile: every compared frame of every match was bit-identical on both peers.'
    + (meant.length ? ` The ${meant.length} rows below are the corrupt-byte profile, which flips bytes on purpose.` : ''), '');
  if (!s.desyncRows.length) { /* nothing to list */ }
  else {
    L.push('| profile | seed | first differing frame (bit-exact) | first differing field (host / guest) | lockstep hash differs from | driver checkpoint | dump |', '|---|---|---|---|---|---|---|');
    for (const d of s.desyncRows.slice(0, 60)) L.push(`| ${d.profile} | ${d.seed} | ${d.first_diff_frame} | ${String(d.field).replace(/\|/g, '/')} | ${d.lockhash_from} | ${d.driver} | ${d.dump ? d.dump.split('/').slice(-1)[0] : ''} |`);
    if (s.desyncRows.length > 60) L.push(`| ... | ${s.desyncRows.length - 60} more | | | | | |`);
  }
  if (s.mism.length) {
    L.push('', '### Mismatch table', '', '| case | what the protocol-5 host does | reason the player sees | crash |', '|---|---|---|---|');
    for (const m of s.mism) L.push(`| ${m.case} | ${m.host_does.replace(/\|/g, '/')} | ${m.reason_shown} | ${m.crash ? 'yes' : 'no'} |`);
  }
  if (s.timing.length) {
    L.push('', '### Lockstep step timing', '');
    for (const t of s.timing) L.push(`- ${t.label}: ${t.line}`);
  }
  return L.join('\n');
}

const isMain = process.argv[1] && import.meta.url === pathToFileURL(process.argv[1]).href;
if (isMain) {
  const args = process.argv.slice(2);
  const docAt = args.indexOf('--doc');
  const doc = docAt >= 0 ? args[docAt + 1] : '';
  let files = args.filter((a, i) => !a.startsWith('--') && !(docAt >= 0 && i === docAt + 1));
  files = files.flatMap((f) => (existsSync(f) && statSync(f).isDirectory() ? readdirSync(f).filter((x) => x.endsWith('.jsonl')).map((x) => join(f, x)) : [f]));
  if (!files.length) {
    const dir = join(process.cwd(), 'Game', 'Saved', 'SoakLockstep');
    files = existsSync(dir) ? readdirSync(dir).filter((f) => f.endsWith('.jsonl')).map((f) => join(dir, f)) : [];
  }
  const rows = [];
  for (const f of files) { if (existsSync(f)) rows.push(...parseLines(readFileSync(f, 'utf8'))); }
  const s = summarize(rows);
  const md = markdown(s);
  console.log(md);
  if (doc) {
    const text = readFileSync(doc, 'utf8');
    const open = '<!-- soak:tables -->', close = '<!-- /soak:tables -->';
    const i = text.indexOf(open), j = text.indexOf(close);
    if (i < 0 || j < i) { console.error(`no ${open} ... ${close} block in ${doc}`); process.exit(2); }
    writeFileSync(doc, text.slice(0, i + open.length) + `

Generated ${new Date().toISOString().slice(0, 16).replace('T', ' ')} UTC by tools/soak/lockstep/summarize.mjs from ${files.length} files.

` + md + `

` + text.slice(j));
  }
  console.log(JSON.stringify({ tool: 'thirdjam.soaklock.report/1', matches: s.matches, desyncs: s.desyncRows.length, profiles: s.profiles.map((p) => ({ profile: p.profile, matches: p.matches, desyncs: p.desyncs, lost: p.lost, edge_loss_matches: p.edgeMatches })) }));
}
