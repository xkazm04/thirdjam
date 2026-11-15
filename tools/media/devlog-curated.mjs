#!/usr/bin/env node
// Build the submitted dev log from the curated shot list (lane A96).
//   node tools/media/devlog-curated.mjs [--repo <main checkout>] [--out submission/devlog/index.html] [--no-embed] [--project-only]
// --project-only (lane A233) is passed through to devlog/build.mjs: only this jam's sessions (see its --help).
// Stages the pictures of devlog/curated/shots.txt (copies, outside the repo: %TEMP%/a96/curated-shots) under the names devlog/build.mjs wants
// (YYYYMMDD-HHMMSS-<A|B>-<label>.<ext> plus a .txt caption), then runs `node devlog/build.mjs ... --embed` on that folder only.
// devlog/build.mjs itself runs read-only `git log` on --repo; this script runs no git.
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import { spawnSync } from 'node:child_process';
const FF = process.env.FFMPEG || 'C:/ffmpeg/bin/ffmpeg.exe';
import { fileURLToPath } from 'node:url';

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..', '..');
const args = process.argv.slice(2);
const opt = (k, d) => (args.includes(k) ? args[args.indexOf(k) + 1] : d);
const repo = opt('--repo', path.resolve(root, '..', '..', '..')); // .claude/worktrees/a96 -> the main checkout
const out = path.resolve(root, opt('--out', 'submission/devlog/index.html'));
const stage = path.join(os.tmpdir(), 'a96', 'curated-shots');
fs.rmSync(stage, { recursive: true, force: true });
fs.mkdirSync(stage, { recursive: true });

let n = 0, bytes = 0, missing = [];
for (const line of fs.readFileSync(path.join(root, 'devlog/curated/shots.txt'), 'utf8').split(/\r?\n/)) {
  if (!line.trim() || line.startsWith('#')) continue;
  const [src, stamp, dev, label, caption] = line.split('|').map((s) => s.trim());
  const abs = path.join(root, src);
  if (!fs.existsSync(abs)) { missing.push(src); continue; }
  const ext = path.extname(src).toLowerCase();
  const name = `${stamp}-${dev}-${label}`;
  // 1280 px wide JPEG: the 59 originals are 69 MB, the page embeds them as base64
  const dst = path.join(stage, name + '.jpg');
  const f = spawnSync(FF, ['-y', '-loglevel', 'error', '-i', abs, '-vf', "scale='min(1280,iw)':-2", '-q:v', '4', dst]);
  if (f.status !== 0) fs.copyFileSync(abs, path.join(stage, name + ext));
  fs.writeFileSync(path.join(stage, name + '.txt'), caption + '\n');
  n++; bytes += fs.statSync(abs).size;
}
console.log(`INFO staged ${n} pictures (${(bytes / 1048576).toFixed(1)} MB) in ${stage}${missing.length ? '; MISSING ' + missing.join(', ') : ''}`);
if (missing.length) process.exit(2);

const proj = path.join(os.homedir(), '.claude', 'projects');
const tr = ['C--Users-kazda-kiro-third-life', 'C--Users-kazda-kiro-thirdjam'].map((d) => path.join(proj, d)).filter((d) => fs.existsSync(d));
const a = [path.join(root, 'devlog/build.mjs'), '--out', out, '--shots', stage, '--repo', repo,
  '--title', 'Hokej: the Third Jam dev log (Glass Box)', ...tr.flatMap((d) => ['--transcripts', 'A=' + d])];
if (!args.includes('--no-embed')) a.push('--embed');
if (args.includes('--project-only')) a.push('--project-only');
fs.mkdirSync(path.dirname(out), { recursive: true });
const r = spawnSync('node', a, { cwd: root, stdio: 'inherit' });
process.exit(r.status ?? 2);
