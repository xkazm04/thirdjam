// Sequential Codex image_gen runner for the look studies (reference images only, never game assets).
// usage: node gen.mjs [name1,name2]    reads ./jobs.json, writes ../images/<name>.png, appends gen.log
// Method copied from the earlier spike: codex exec with the built-in image_gen tool, one image per run.
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { spawnSync } from 'node:child_process';

const here = path.dirname(fileURLToPath(import.meta.url));
const cfg = JSON.parse(fs.readFileSync(path.join(here, 'jobs.json'), 'utf8'));
const outDir = path.join(here, '..', 'images');
const work = path.join(here, '.work');
fs.mkdirSync(outDir, { recursive: true });
const log = (m) => { const l = `[${new Date().toISOString()}] ${m}`; console.log(l); fs.appendFileSync(path.join(here, 'gen.log'), l + '\n'); };
const only = process.argv[2] ? new Set(process.argv[2].split(',')) : null;

let fails = 0;
for (const job of cfg.jobs) {
  if (only && !only.has(job.name)) continue;
  const target = path.join(outDir, `${job.name}.png`);
  if (fs.existsSync(target)) { log(`${job.name}: exists, skip`); continue; }
  const dir = path.join(work, job.name);
  fs.mkdirSync(path.join(dir, 'out'), { recursive: true });
  const prompt = `Use the built-in image_gen tool (imagegen skill) to generate exactly ONE image, landscape. ${job.prompt}\n\n${cfg.style}\n\nThen copy the generated file to ./out/${job.name}.png and print the original path. Do not generate more than one image.`;
  const t0 = Date.now();
  log(`${job.name}: start`);
  const r = spawnSync(process.execPath, ['C:/nvm4w/nodejs/node_modules/@openai/codex/bin/codex.js', 'exec', '--skip-git-repo-check', '-s', 'workspace-write', '-o', 'last_message.txt', '-C', dir, '--', prompt], { cwd: dir, encoding: 'utf8', timeout: 15 * 60_000, maxBuffer: 64 * 1024 * 1024 });
  fs.writeFileSync(path.join(dir, 'codex.log'), `${r.stdout || ''}\n--- stderr ---\n${r.stderr || ''}`);
  const produced = path.join(dir, 'out', `${job.name}.png`);
  if (fs.existsSync(produced)) { fs.copyFileSync(produced, target); log(`${job.name}: ok ${Math.round((Date.now() - t0) / 1000)}s`); fails = 0; continue; }
  const tail = `${r.stderr || ''}${r.stdout || ''}`.slice(-600).replace(/\s+/g, ' ');
  log(`${job.name}: FAILED exit ${r.status} ${Math.round((Date.now() - t0) / 1000)}s :: ${tail}`);
  if (++fails >= 2) { log('2 failures in a row: stopping'); process.exit(4); }
}
log('done');
