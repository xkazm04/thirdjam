#!/usr/bin/env node
// Build the dev log page. See devlog/README.md. Exit 0 built, 1 build refused (a leak was found
// after redaction), 2 could not run (bad arguments, unreadable input).
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { parseArgs, HELP } from './lib/cli.mjs';
import { createRedactor, redactHtml } from './lib/redact.mjs';
import { loadTranscripts } from './lib/transcripts.mjs';
import { loadCommits } from './lib/gitlog.mjs';
import { loadShots, loadStatus, loadGates } from './lib/inputs.mjs';
import { parseNotes } from './lib/markdown.mjs';
import { buildModel } from './lib/model.mjs';
import { renderPage } from './lib/render.mjs';
import { writeFixtureShots } from './fixtures/make-fixtures.mjs';

const HERE = path.dirname(fileURLToPath(import.meta.url));
export const ROOT = path.resolve(HERE, '..');

// B-to-A-0013 (lane A21): the notes of both devices. Without --notes every `devlog/notes*.md` is read, A's `notes.md` first and
// then the rest by name (`notes-b.md` is device B's; a file whose name ends in `-b` is B's, anything else is A's). Each block keeps
// its device, so the page shows it on that device's lane. With --notes <file> only that file is read, as before.
export function deviceOfNotesFile(file) {
  return /-b\.md$/i.test(path.basename(file)) ? 'B' : 'A';
}

export function notesFilesIn(dir) {
  if (!fs.existsSync(dir)) return [];
  const names = fs.readdirSync(dir).filter((n) => /^notes.*\.md$/i.test(n)).sort((a, b) => a.localeCompare(b));
  const first = names.filter((n) => n.toLowerCase() === 'notes.md');
  return [...first, ...names.filter((n) => n.toLowerCase() !== 'notes.md')].map((n) => path.join(dir, n));
}

export function loadNotes(files) {
  const out = [];
  files.forEach((file, fi) => {
    if (!fs.existsSync(file)) return;
    const device = deviceOfNotesFile(file);
    for (const b of parseNotes(fs.readFileSync(file, 'utf8'))) out.push({ ...b, device, file: fi });
  });
  // One timeline: by hour, then A before B within the hour, then the order written in the file.
  return out.sort((a, b) => a.hour - b.hour || a.file - b.file || a.order - b.order);
}

/**
 * Build a page. Returns {html, stats, warnings, counts}. Throws on unreadable input.
 * @param {object} o parsed arguments (see lib/cli.mjs); `root` overrides the repository root for defaults
 */
export function buildPage(o) {
  const root = o.root || ROOT;
  const redactor = createRedactor();
  const { redact } = redactor;
  const warnings = [];
  const abs = (p) => path.resolve(p);

  let transcriptSrc = o.transcripts.map((t) => ({ device: t.device || 'A', dir: abs(t.dir) }));
  let repos = o.repos.map((r) => ({ device: r.device, dir: abs(r.dir) }));
  let shotsDir = o.shots ? abs(o.shots) : path.join(root, 'devlog', 'shots');
  let statusDir = o.status ? abs(o.status) : path.join(root, 'status');
  let notesFiles = o.notes ? [abs(o.notes)] : notesFilesIn(path.join(root, 'devlog'));
  const gatesFile = o.gates ? abs(o.gates) : path.join(root, 'docs', 'GATES.md');

  if (o.fixtures) {
    const fx = path.join(root, 'devlog', 'fixtures');
    transcriptSrc = [
      { device: 'A', dir: path.join(fx, 'transcripts', 'A') },
      { device: 'B', dir: path.join(fx, 'transcripts', 'B') },
    ];
    shotsDir = path.join(root, 'devlog', 'out', 'fixture-shots');
    writeFixtureShots(shotsDir);
    statusDir = path.join(fx, 'status');
    notesFiles = [path.join(fx, 'notes.md')];
  }

  for (const t of transcriptSrc) if (!fs.existsSync(t.dir)) warnings.push(`transcript path not found: ${redact(path.basename(t.dir))}`);
  for (const r of repos) if (!fs.existsSync(r.dir)) throw new Error(`repository not found: ${redact(r.dir)}`);

  let sessions = loadTranscripts(transcriptSrc, redact);
  if (o.since) {
    const sinceMs = Date.parse(o.since);
    if (Number.isNaN(sinceMs)) throw new Error(`--since is not a time: ${redact(o.since)}`);
    sessions = sessions
      .map((s) => ({ ...s, turns: s.turns.filter((t) => t.ms >= sinceMs), prompts: s.prompts.filter((p) => p.ms >= sinceMs) }))
      .filter((s) => s.turns.length || s.prompts.length)
      .map((s) => ({ ...s, firstMs: Math.max(s.firstMs ?? sinceMs, sinceMs) }));
  }
  for (const s of sessions) if (s.badLines) warnings.push(`${s.badLines} unreadable line(s) skipped in a transcript (${redact(s.id)})`);
  const commits = loadCommits(repos, redact);
  const shotResult = loadShots(shotsDir, redact);
  warnings.push(...shotResult.warnings);
  const statusResult = loadStatus(statusDir, redact);
  warnings.push(...statusResult.warnings);
  const notes = loadNotes(notesFiles);
  const gates = loadGates(gatesFile);
  if (!gates.length) warnings.push('no gates found (docs/GATES.md missing or without "## Name (H<hour>)" headings)');

  const model = buildModel({ commits, sessions, shots: shotResult.shots, notes, status: statusResult.entries, gates });
  const outDir = path.dirname(abs(o.out));
  let html = renderPage(model, {
    redact,
    embed: o.embed,
    outDir,
    title: o.title || 'Third Jam dev log',
    warnings,
    stats: redactor.stats,
    inputs: {
      transcripts: sessions.length,
      repos: repos.length,
      shots: shotResult.shots.length,
      notes: notes.length,
      status: statusResult.entries.length,
    },
  });

  // Final pass over the whole document (embedded images masked), then verify it is a fixed point.
  html = redactHtml(html, redactor);
  const verify = createRedactor();
  const again = redactHtml(html, verify);
  if (again !== html) throw Object.assign(new Error('redaction is not stable; refusing to write the page'), { leak: true });

  return { html, stats: redactor.stats, warnings, model };
}

function main() {
  let o;
  try {
    o = parseArgs(process.argv.slice(2));
  } catch (e) {
    console.error(`FAIL ${e.message}\n\n${HELP}`);
    return 2;
  }
  if (o.help) {
    console.log(HELP);
    return 0;
  }
  if (!o.out) {
    console.error(`FAIL --out is required\n\n${HELP}`);
    return 2;
  }
  let result;
  try {
    result = buildPage(o);
  } catch (e) {
    console.error(`FAIL ${e.message}`);
    return e.leak ? 1 : 2;
  }
  try {
    fs.mkdirSync(path.dirname(path.resolve(o.out)), { recursive: true });
    fs.writeFileSync(o.out, result.html);
  } catch (e) {
    console.error(`FAIL cannot write ${path.basename(o.out)}: ${e.message}`);
    return 2;
  }
  const s = result.model.summary;
  for (const w of result.warnings) console.log(`WARN ${w}`);
  console.log(
    `PASS wrote ${o.out} (${Math.round(result.html.length / 1024)} KB): ${s.commits} commits, ${s.sessions.main + s.sessions.subagent} sessions, ${s.turns} turns, ${s.shots} screenshots; redacted ${result.stats.key} secrets, ${result.stats.email} emails, ${result.stats.path} paths`,
  );
  return 0;
}

if (process.argv[1] && path.resolve(process.argv[1]) === fileURLToPath(import.meta.url)) process.exitCode = main();
