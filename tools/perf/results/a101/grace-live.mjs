#!/usr/bin/env node
// Lane A113 (track F1, R-0049 point 1): reads the game logs of headless matches (tools/commentary/match.ps1: -game -nullrhi, the scripted human) and counts,
// inside the first <frames> sim frames: knockdowns of the human and of bots (JAMVIS knockdown), figures down again within 120 frames of their release
// (JAMVIS getup), the shortest release-to-down gap, the hit-stop lines by kind and the HUD's contact lines.
//   node grace-live.mjs <frames> <game.log> [<game.log> ...]
import { readFileSync } from 'node:fs';

const [framesArg = '6000', ...logs] = process.argv.slice(2);
const frames = Number(framesArg);
for (const file of logs) {
  const up = new Map();   // skater -> frame of its last release
  const row = { log: file.split('\\').join('/'), frames, human_downs: 0, bot_downs: 0, down_again_120: 0, down_again_60: 0, min_gap: null, human_gaps: [], hitstop: {}, hud: {}, last_frame: 0 };
  let lastHumanUp = null;
  for (const l of readFileSync(file, 'utf8').split(/\r?\n/)) {
    let m;
    if ((m = /JAMVIS knockdown frame=(\d+) skater=(\d+) team=\d+ human=(\d)/.exec(l))) {
      const f = +m[1], s = +m[2], human = m[3] === '1';
      if (f > frames) continue;
      row.last_frame = Math.max(row.last_frame, f);
      if (human) ++row.human_downs; else ++row.bot_downs;
      if (up.has(s)) {
        const gap = f - up.get(s);
        if (row.min_gap === null || gap < row.min_gap) row.min_gap = gap;
        if (gap <= 120) ++row.down_again_120;
        if (gap <= 60) ++row.down_again_60;
        if (human) row.human_gaps.push(gap);
      }
    } else if ((m = /JAMVIS getup frame=(\d+) skater=(\d+)/.exec(l))) {
      if (+m[1] <= frames) up.set(+m[2], +m[1]);
    } else if ((m = /JAMHITSTOP frame=(\d+) frames=(\d+) .* kind=(\S+) strength=([0-9.]+) outcome=(\d)/.exec(l))) {
      if (+m[1] > frames) continue;
      const k = m[5] === '2' ? `stumble:${m[3]}` : m[5] === '3' ? 'down' : 'other';
      (row.hitstop[k] ??= []).push({ frame: +m[1], hold: +m[2], strength: +m[4] });
    } else if ((m = /JAMHUD frame=(\d+) contact_line=(\S+)/.exec(l))) {
      if (+m[1] > frames) continue;
      row.hud[m[2]] = 1 + (row.hud[m[2]] ?? 0);
    }
  }
  console.log(JSON.stringify(row));
}
