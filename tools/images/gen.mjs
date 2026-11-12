// Codex image generation runner for shipped 2D content (RULINGS R-0008). Lifted from prep-reference/tools/gen.mjs by lane A8.
//
//   node tools/images/gen.mjs <jobs.json> [--only name1,name2] [--out <dir>] [--force]
//
// One image per Codex run (codex exec with the built-in image_gen tool), sequential, a job list file per batch. Outputs go to a
// scratch folder (default: <TEMP>/thirdjam-images/<batch>/), never into Game/Content: only the picked files are imported, by hand,
// as T_UI_* under Game/Content/UI/ and then ledgered in ASSETS.md (origin ai-generated, tool "Codex image generation", url = the
// path of the job list that holds the prompt, size_mb). Prompts stay with the generator in the job list.
//
// Job list: { "batch": "menu", "style": "<appended to every prompt>", "jobs": [ { "name": "title-slate", "aspect": "landscape|square|portrait", "prompt": "..." } ] }
// Content law (R-0008 3): invented text only, English, no real brand, team, league or person, teen-appropriate.
// Exit: 0 all requested jobs produced (or skipped as existing), 1 some failed, 2 could not run. Last line is a JSON verdict.
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import { spawnSync } from 'node:child_process';

const args = process.argv.slice(2);
const jobsFile = args.find((a) => !a.startsWith('--'));
const opt = (k) => { const i = args.indexOf(`--${k}`); return i >= 0 ? args[i + 1] : undefined; };
const force = args.includes('--force');
if (!jobsFile || !fs.existsSync(jobsFile)) {
	console.log('usage: node tools/images/gen.mjs <jobs.json> [--only a,b] [--out dir] [--force]');
	console.log(JSON.stringify({ tool: 'thirdjam.images/1', verdict: 'could-not-run', reason: 'no job list' }));
	process.exit(2);
}
const cfg = JSON.parse(fs.readFileSync(jobsFile, 'utf8'));
const batch = cfg.batch || path.basename(jobsFile, '.json');
const outDir = opt('out') || path.join(process.env.TJ_IMAGES_OUT || path.join(os.tmpdir(), 'thirdjam-images'), batch);
const work = path.join(outDir, '.work');
fs.mkdirSync(outDir, { recursive: true });
const logFile = path.join(outDir, 'gen.log');
const log = (m) => { const l = `[${new Date().toISOString()}] ${m}`; console.log(l); fs.appendFileSync(logFile, l + '\n'); };
const only = opt('only') ? new Set(opt('only').split(',')) : null;
const codex = process.env.CODEX_JS || 'C:/nvm4w/nodejs/node_modules/@openai/codex/bin/codex.js';
if (!fs.existsSync(codex)) {
	console.log(JSON.stringify({ tool: 'thirdjam.images/1', verdict: 'could-not-run', reason: `codex not found at ${codex} (set CODEX_JS)` }));
	process.exit(2);
}

const results = [];
let fails = 0;
for (const job of cfg.jobs) {
	if (only && !only.has(job.name)) continue;
	const target = path.join(outDir, `${job.name}.png`);
	if (fs.existsSync(target) && !force) { log(`${job.name}: exists, skip`); results.push({ name: job.name, status: 'exists', file: target }); continue; }
	const dir = path.join(work, job.name);
	fs.rmSync(dir, { recursive: true, force: true });
	fs.mkdirSync(path.join(dir, 'out'), { recursive: true });
	const aspect = job.aspect || 'landscape';
	const prompt = `Use the built-in image_gen tool (imagegen skill) to generate exactly ONE image, ${aspect}. ${job.prompt}\n\n${cfg.style || ''}\n\nThen copy the generated file to ./out/${job.name}.png and print the original path. Do not generate more than one image.`;
	fs.writeFileSync(path.join(dir, 'prompt.txt'), prompt);
	const t0 = Date.now();
	log(`${job.name}: start`);
	const r = spawnSync(process.execPath, [codex, 'exec', '--skip-git-repo-check', '-s', 'workspace-write', '-o', 'last_message.txt', '-C', dir, '--', prompt],
		{ cwd: dir, encoding: 'utf8', timeout: 15 * 60_000, maxBuffer: 64 * 1024 * 1024 });
	fs.writeFileSync(path.join(dir, 'codex.log'), `${r.stdout || ''}\n--- stderr ---\n${r.stderr || ''}`);
	const produced = path.join(dir, 'out', `${job.name}.png`);
	const secs = Math.round((Date.now() - t0) / 1000);
	if (fs.existsSync(produced)) {
		fs.copyFileSync(produced, target);
		const mb = (fs.statSync(target).size / 1048576).toFixed(1);
		log(`${job.name}: ok ${secs}s ${mb} MB -> ${target}`);
		results.push({ name: job.name, status: 'ok', seconds: secs, size_mb: Number(mb), file: target });
		fails = 0;
		continue;
	}
	const tail = `${r.stderr || ''}${r.stdout || ''}`.slice(-600).replace(/\s+/g, ' ');
	log(`${job.name}: FAILED exit ${r.status} ${secs}s :: ${tail}`);
	results.push({ name: job.name, status: 'failed', seconds: secs, exit: r.status });
	if (++fails >= 2) { log('2 failures in a row: stopping'); break; }
}
const failed = results.filter((r) => r.status === 'failed').length;
log(`done: ${results.filter((r) => r.status === 'ok').length} ok, ${results.filter((r) => r.status === 'exists').length} existing, ${failed} failed; out ${outDir}`);
console.log(JSON.stringify({ tool: 'thirdjam.images/1', verdict: failed ? 'fail' : 'pass', batch, out: outDir, jobs: results }));
process.exit(failed ? 1 : 0);
