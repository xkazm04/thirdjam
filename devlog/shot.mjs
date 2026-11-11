#!/usr/bin/env node
// Copy an image into devlog/shots under the right name, stamped with the current jam clock time (CET).
//   node devlog/shot.mjs <A|B> <label> <image-file> [caption words...]
// The name is YYYYMMDD-HHMMSS-<A|B>-<label>.<ext>. A caption, if given, goes in a sidecar .txt next to it.
// Exit 0 done, 2 could not run.
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { shotStamp } from './lib/time.mjs';

const HERE = path.dirname(fileURLToPath(import.meta.url));

export function shotName(device, label, ext, ms = Date.now()) {
  if (!/^[AB]$/.test(device)) throw new Error('device must be A or B');
  if (!/^[A-Za-z0-9][A-Za-z0-9_-]*$/.test(label)) throw new Error('label may use letters, digits, - and _ only');
  return `${shotStamp(ms)}-${device}-${label}.${ext.toLowerCase().replace(/^\./, '')}`;
}

function main(argv) {
  const [device, label, file, ...cap] = argv;
  if (!device || !label || !file) {
    console.error('usage: node devlog/shot.mjs <A|B> <label> <image-file> [caption words...]');
    return 2;
  }
  try {
    const ext = path.extname(file);
    if (!/^\.(png|jpe?g|webp)$/i.test(ext)) throw new Error('image must be .png, .jpg, .jpeg or .webp');
    if (!fs.existsSync(file)) throw new Error(`no such file: ${path.basename(file)}`);
    const dir = path.join(HERE, 'shots');
    fs.mkdirSync(dir, { recursive: true });
    const name = shotName(device, label, ext);
    fs.copyFileSync(file, path.join(dir, name));
    if (cap.length) fs.writeFileSync(path.join(dir, name.replace(/\.[^.]+$/, '.txt')), cap.join(' ') + '\n');
    console.log(`PASS devlog/shots/${name}`);
    return 0;
  } catch (e) {
    console.error(`FAIL ${e.message}`);
    return 2;
  }
}

if (process.argv[1] && path.resolve(process.argv[1]) === fileURLToPath(import.meta.url)) process.exitCode = main(process.argv.slice(2));
