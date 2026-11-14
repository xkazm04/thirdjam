#!/usr/bin/env node
// lane A209: the hit audit's table from raw/<label>.jsonl files (the "hand" object of each match line, JamHitHand.h). node hand-table.mjs <label> [label...]
import { readFileSync } from 'node:fs';
import { join, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
const here = dirname(fileURLToPath(import.meta.url));
const pct = (a, b) => (b ? (100 * a / b).toFixed(1) : '-');
for (const label of process.argv.slice(2)) {
  const ms = readFileSync(join(here, 'raw', `${label}.jsonl`), 'utf8').split('\n').filter(Boolean).map((l) => JSON.parse(l));
  const H = {}; for (const m of ms) for (const [k, v] of Object.entries(m.hand || {})) H[k] = (H[k] || 0) + v;
  const n = ms.length, s = (k) => H[k] || 0;
  const sum = (k) => ms.reduce((a, m) => a + (Number(m[k]) || 0), 0);
  const bMiss = ['b_carrier', 'b_cooling', 'b_dropped', 'b_mandown', 'b_grace', 'b_early', 'b_late', 'b_gap03', 'b_gap06', 'b_gap10', 'b_gapfar'];
  const cMiss = ['c_nopuck', 'c_reach', 'c_cone', 'c_arm', 'c_back', 'c_shield', 'c_moved', 'c_cooldown', 'c_l_far', 'c_l_cone', 'c_l_back', 'c_l_down'];
  const out = {
    label, n, win: pct(ms.filter((m) => m.result === 'win').length, n),
    body: { press: s('b_press'), per_match: +(s('b_press') / n).toFixed(1), landed_pct: pct(s('b_landed'), s('b_press')), contact_pct: pct(s('b_landed') + s('b_bump') + s('b_other'), s('b_press')), down_pct: pct(s('b_down'), s('b_press')), bump_pct: pct(s('b_bump'), s('b_press')), other_pct: pct(s('b_other'), s('b_press')),
      in25: s('b_press25'), in25_landed_pct: pct(s('b_landed25'), s('b_press25')), miss_pct: Object.fromEntries(bMiss.map((k) => [k.slice(2), pct(s(k), s('b_press'))])), wide_of_gap_misses: s('b_wide'),
      landed_mean: { dist_m: +(s('b_land_dist_cm') / 100 / Math.max(1, s('b_landed'))).toFixed(2), angle_deg: +(s('b_land_ang_deg') / Math.max(1, s('b_landed'))).toFixed(1), press_to_contact_frames: +(s('b_land_frames') / Math.max(1, s('b_landed'))).toFixed(1) } },
    cross: { press: s('c_press'), per_match: +(s('c_press') / n).toFixed(1), landed_pct: pct(s('c_landed'), s('c_press')), contact_pct: pct(s('c_landed') + s('c_bump'), s('c_press')), down_pct: pct(s('c_down'), s('c_press')), bump_pct: pct(s('c_bump'), s('c_press')), strips: s('c_strip'), accepted_pct: pct(s('c_accepted'), s('c_press')),
      in25: s('c_press25'), in25_landed_pct: pct(s('c_landed25'), s('c_press25')), miss_pct: Object.fromEntries(cMiss.map((k) => [k.slice(2), pct(s(k), s('c_press'))])) },
    brace: { holds: s('h_holds'), landed_pct: pct(s('h_landed'), s('h_holds')), bump_pct: pct(s('h_bump'), s('h_holds')), none_pct: pct(s('h_none'), s('h_holds')) },
    nobody: { press: s('n_press'), landed: s('n_landed') }, elsewhere: s('x_elsewhere'),
    human_down_pct: pct(sum('human_down_frames'), sum('play_frames')), knockdowns: +(sum('knockdowns') / n).toFixed(2), kd_given: +(sum('kd_given') / n).toFixed(2),
    bot_checks_landed: +(sum('bot_checks_landed') / n).toFixed(2), bot_hits_on_human: +(sum('bot_hits_on_human') / n).toFixed(2), bot_check_presses: +(sum('bot_check_presses') / n).toFixed(1),
  };
  console.log(JSON.stringify(out));
}
