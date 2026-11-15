// Lane A213, wave 3: pastes the generated controls table (jam.ui.playsheet, from the live bindings) into docs/experiments/PLAY-ME-DRAFT.md between
// the markers <!-- controls:begin --> and <!-- controls:end -->. Usage: node tools/qa/screen/a213/play-sheet.mjs <table.md>
import { readFileSync, writeFileSync } from 'node:fs';
import { resolve, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';

const root = resolve(dirname(fileURLToPath(import.meta.url)), '../../../..');
const draft = resolve(root, 'docs/experiments/PLAY-ME-DRAFT.md');
const table = readFileSync(process.argv[2], 'utf8').trim();
const text = readFileSync(draft, 'utf8');
const begin = '<!-- controls:begin -->', end = '<!-- controls:end -->';
const i = text.indexOf(begin), j = text.indexOf(end);
if (i < 0 || j < i) { console.log('FAIL play-sheet: markers not found'); process.exit(1); }
const rows = table.split('\n').length - 2;
writeFileSync(draft, text.slice(0, i + begin.length) + '\n' + table + '\n' + text.slice(j));
console.log(`PASS play-sheet ${rows} rows pasted from ${process.argv[2]}`);
