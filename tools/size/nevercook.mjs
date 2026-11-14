#!/usr/bin/env node
// Lane A161 (track Q3, evolution 6; B-to-A-0041 point 5): keeps device B's unshipped figure families out of the cook without deleting them.
// The cooker (UE 5.9 UCookOnTheFlyServer::GetNeverCookPackageNames) matches DirectoriesToNeverCook entries against long package names with
// TDirectoryTree::ContainsPathOrParent, so an entry may name a folder or one package. /JamAssets/Skaters is one flat folder that also holds the
// shipped figure (TB1), so the unshipped families are listed package by package, between two marker lines in Game/Config/DefaultGame.ini.
//   node tools/size/nevercook.mjs            print the block it would write and whether the ini matches (exit 0 matches, 1 differs)
//   node tools/size/nevercook.mjs --write    rewrite the block in place (the rest of the file is untouched)
//   node tools/size/nevercook.mjs --remove   take the block out (the previous package, the revert path)
// A new TB2 / SkaterGen / P4Lab asset from B lands in the cook until this is run again: `tools/size/cookguard.mjs` fails until then.
// (Lane A175: restored; the A161 landing carried the ini block but not this file, refwalk.mjs or cookonly.sh.)
import fs from 'node:fs';
import path from 'node:path';

const repo = path.resolve(path.dirname(new URL(import.meta.url).pathname.replace(/^\/([A-Za-z]:)/, '$1')), '..', '..');
const iniPath = path.join(repo, 'Game', 'Config', 'DefaultGame.ini');
const skaters = path.join(repo, 'Game', 'Plugins', 'JamAssets', 'Content', 'Skaters');
const BEGIN = '; >>> A161 never-cook: device B unshipped figures (tools/size/nevercook.mjs writes this block; do not edit by hand)';
const END = '; <<< A161 never-cook';
// The families (B-to-A-0041 point 5): the generated_b2 figure (TB2, its clay and broadcast versions), the generated meshes it was made from
// (SkaterGen P2C / P3C / P4D / P4E) and the P4 lab figure; plus MI_Bcast_SkaterP4, a broadcast skater instance whose parent is MI_SkaterTB2_P4
// and which nothing else references; the P4 lab figure's own materials in Look/P4Lab (M_P4_Figure, M_P4_FigureE and their instances: only
// SK_SkaterGen_P4E names MI_P4_FigureE, nothing names MI_P4_Figure; the two masters read SkaterGen textures). The rest of Look/P4Lab stays:
// LR_P4_Broadcast (still a selectable look tier) uses its materials.
const family = /(TB2|SkaterGen|P4Lab)/;
const extra = ['/JamAssets/Look/MI_Bcast_SkaterP4', '/JamAssets/Look/P4Lab/M_P4_Figure', '/JamAssets/Look/P4Lab/M_P4_FigureE',
  '/JamAssets/Look/P4Lab/MI_P4_Figure', '/JamAssets/Look/P4Lab/MI_P4_FigureE'];

const names = fs.readdirSync(skaters).filter(f => f.endsWith('.uasset') && family.test(f)).map(f => '/JamAssets/Skaters/' + f.replace(/\.uasset$/, '')).sort();
const all = [...names, ...extra];
const block = [BEGIN, '; ' + all.length + ' packages; the editor, the tests and the test cuts still load them; a packaged test cut falls back to the toy figure (AHokejGameMode::ResolveVisualClass).',
  ...all.map(n => `+DirectoriesToNeverCook=(Path="${n}")`), END];

const ini = fs.readFileSync(iniPath, 'utf8');
const eol = ini.includes('\r\n') ? '\r\n' : '\n';
const lines = ini.split(/\r?\n/);
const b = lines.indexOf(BEGIN), e = lines.indexOf(END);
const has = b >= 0 && e > b;
const current = has ? lines.slice(b, e + 1).join('\n') : '';
const mode = process.argv.includes('--write') ? 'write' : process.argv.includes('--remove') ? 'remove' : 'check';

if (mode === 'check') {
  const ok = current === block.join('\n');
  if (!ok && has) {
    const cur = new Set(lines.slice(b, e + 1)), want = new Set(block);
    for (const l of block) if (!cur.has(l) && l.startsWith('+')) console.log('  missing from the ini: ' + l);
    for (const l of cur) if (!want.has(l) && l.startsWith('+')) console.log('  in the ini, no longer on disk: ' + l);
  }
  console.log(`${ok ? 'PASS' : 'FAIL'} nevercook: ${all.length} packages; the ini ${has ? (ok ? 'matches' : 'differs (run --write)') : 'has no block'}`);
  process.exit(ok ? 0 : 1);
}
let out;
if (mode === 'remove') out = has ? [...lines.slice(0, b), ...lines.slice(e + 1)] : lines;
else if (has) out = [...lines.slice(0, b), ...block, ...lines.slice(e + 1)];
else {
  // After the last DirectoriesToNeverCook line of the packaging section.
  let at = -1;
  lines.forEach((l, i) => { if (l.startsWith('+DirectoriesToNeverCook=')) at = i; });
  if (at < 0) { console.log('FAIL nevercook: no DirectoriesToNeverCook line to anchor on'); process.exit(2); }
  out = [...lines.slice(0, at + 1), ...block, ...lines.slice(at + 1)];
}
fs.writeFileSync(iniPath, out.join(eol));
console.log(`PASS nevercook: ${mode === 'remove' ? 'block removed' : all.length + ' packages written'} (${iniPath})`);
