#!/usr/bin/env node
// lane A182: what the human lives through, per match, from the playtest line: node op.mjs <label> [...]
import { readFileSync } from 'node:fs';
for (const l of process.argv.slice(2)) {
  const ms = readFileSync(new URL(`raw/${l}.jsonl`, import.meta.url), 'utf8').trim().split('\n').map(JSON.parse);
  const N = ms.length, S = (k) => ms.reduce((a, m) => a + (Number(m[k]) || 0), 0), pm = (k) => (S(k) / N).toFixed(2), play = S('play_frames');
  console.log(`${l} n=${N} win=${(100 * ms.filter((m) => m.result === 'win').length / N).toFixed(1)} | down % of play ${(100 * S('human_down_frames') / play).toFixed(2)} | stumble % ${(100 * S('human_stumble_frames') / play).toFixed(2)} | knocked down ${pm('knockdowns')} (as he won it ${pm('kd_won')}) | shielded hits on him ${pm('shield_human')} | his check presses ${pm('check_presses')}, dropped by the cooldown ${pm('checks_cooling')}, lunges missed ${pm('lunges_missed')} | standing in a downed body (0.9 m spells) ${pm('downed_in09_spells')}, 0.6 m ${pm('downed_in06_spells')} | slams ${pm('slams')}`);
}
