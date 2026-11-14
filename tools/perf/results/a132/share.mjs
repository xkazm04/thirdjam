#!/usr/bin/env node
// Lane A132: the share of faceoffs without a human touch inside 600 frames (Jam.Bot.HumanHasSomethingToDo's number) over an arm's novice-at-rookie matches,
// and over its seeds 1-20 (the test's own sample).   node share.mjs <arm> [...]
import { readFileSync } from 'node:fs';
import { join, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
const here = dirname(fileURLToPath(import.meta.url));
for (const arm of process.argv.slice(2)) {
  const ms = readFileSync(join(here, 'raw', `${arm}-rk.jsonl`), 'utf8').split(/\r?\n/).filter(Boolean).map((l) => JSON.parse(l));
  const f = (xs) => { const a = xs.reduce((s, m) => s + m.faceoffs_without_touch, 0), b = xs.reduce((s, m) => s + m.faceoffs, 0); return `${a} of ${b} = ${(100 * a / Math.max(1, b)).toFixed(2)} %`; };
  console.log(`${arm}: ${ms.length} matches ${f(ms)} | seeds 1-20: ${f(ms.filter((m) => m.seed <= 20))}`);
}
