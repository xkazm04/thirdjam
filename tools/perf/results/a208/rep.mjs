// lane A208: exact one-occurrence replace. node rep.mjs <file> <oldfile> <newfile>
import { readFileSync, writeFileSync } from 'node:fs';
const [f, o, n] = process.argv.slice(2);
const s = readFileSync(f, 'utf8'), a = readFileSync(o, 'utf8').replace(/\n$/, ''), b = readFileSync(n, 'utf8').replace(/\n$/, '');
const i = s.indexOf(a);
if (i < 0 || s.indexOf(a, i + 1) >= 0) { console.error('FAIL match count'); process.exit(1); }
writeFileSync(f, s.slice(0, i) + b + s.slice(i + a.length));
console.log('ok');
