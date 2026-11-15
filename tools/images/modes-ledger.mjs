// Lane A231 (track U6): the ledger rows of the MODES illustrations (R-0008 point 2). Idempotent. Run from the repo root:
//
//   node tools/images/modes-ledger.mjs          writes / refreshes one row per imported picture in ASSETS.md
//   node tools/images/modes-ledger.mjs --check  exit 1 when a row is missing or differs (writes nothing)
//
// Reads tools/images/modes/import.json (written by tools/images/import-modes.py: the assets and their bytes on disk) and
// tools/images/modes-picks.json (the mode's name, the job and the try that was picked). A row: origin ai-generated, tool
// "Codex image generation", url = the job list that holds the prompt, size_mb = the .uasset on disk. The rows go after the last
// Game/Content/UI/ row of the ledger; a row whose path is already there is replaced in place.
import fs from 'node:fs';

const check = process.argv.includes('--check');
const ledger = 'ASSETS.md';
const report = JSON.parse(fs.readFileSync('tools/images/modes/import.json', 'utf8'));
const picks = JSON.parse(fs.readFileSync('tools/images/modes-picks.json', 'utf8'));
const date = '2026-11-15';
const rows = new Map();
for (const p of report.pictures) {
	if (p.status !== 'imported' && p.status !== 'kept') continue;
	const file = `Game/Content/UI/Modes/T_UI_ModeArt_${p.id}.uasset`;
	if (!fs.existsSync(file)) continue;
	const pick = picks[p.id] || {};
	const mb = (fs.statSync(file).size / 1048576).toFixed(1);
	const job = (pick.file || `${p.id}.png`).replace(/\.png$/, '');
	const note = `MODES illustration for ${pick.name || p.id} (lane A231, track U6): ${p.width}x${p.height} RGB, UI group, BC7, no mips; look ${pick.look || '?'}; job ${job}, try ${pick.try || 1}; source tools/images/modes/${p.id}.png; device B's T_UI_Mode picture of the row stays as the fallback`;
	rows.set(file, `| ${file} | texture | ${mb} | ai-generated | Codex image generation |  | tools/images/modes-jobs.json | ${date} | A | y | ${note} |`);
}
const lines = fs.readFileSync(ledger, 'utf8').split('\n');
let changed = 0;
for (let i = 0; i < lines.length; i++) {
	const m = /^\| (Game\/Content\/UI\/Modes\/T_UI_ModeArt_\w+\.uasset) \|/.exec(lines[i]);
	if (!m || !rows.has(m[1])) continue;
	if (lines[i] !== rows.get(m[1])) { lines[i] = rows.get(m[1]); changed++; }
	rows.delete(m[1]);
}
if (rows.size) {
	let at = -1;
	for (let i = 0; i < lines.length; i++) if (lines[i].startsWith('| Game/Content/UI/')) at = i;
	if (at < 0) { console.log('FAIL modes-ledger: no Game/Content/UI/ row to add after'); process.exit(2); }
	lines.splice(at + 1, 0, ...rows.values());
	changed += rows.size;
}
if (check) {
	console.log(`${changed ? 'FAIL' : 'PASS'} modes-ledger: ${changed} row(s) missing or different`);
	process.exit(changed ? 1 : 0);
}
if (changed) fs.writeFileSync(ledger, lines.join('\n'));
console.log(`PASS modes-ledger: ${changed} row(s) written, ${report.pictures.filter((p) => p.status === 'imported' || p.status === 'kept').length} picture(s) in the import report`);
