#!/usr/bin/env node
// Lane A91 (R-0044): A85's analyzer with the bars as R-0044 restates them: the sweep and the spin by their shield (every opponent poke and lift inside the window
// refused: JAMDRILL stick_result's shield_refused / shield_clean), the saucer by its flight over a blade when the cue had a blade on the lane, the shot heights
// 10 / 10 / 10 at the line within 8 cm (a tap is charge 0 since game.shot.tap_frames), five one-timers. The puck kept against a body check is reported, not a bar.
// Lane A85 (R-0042): A82's analyzer (the same bars and read-outs), plus the hand-puck count and the releases' reach. Lane A82 (R-0040): A77's analyzer with the brief's bars (ten of ten kept; the saucer over a blade and landed before the receiver, ten of ten; 10/10/10 heights; five
// one-timers) and the A82 read-outs (JAMSHOT release lines, JAMDRILL hand_puck, the lift misses the shield produced).
//   node tools/qa/ctrl/a85-analyze.mjs tools/qa/out/ctrl/a85-sweep/game.log
// The drill is read from the log (the JAMCUE kinds). PASS/FAIL/INFO lines and one JSON line last (exit 0 pass, 1 a bar missed, 2 could not run).
import { readFileSync } from 'node:fs';

const file = process.argv[2];
if (!file) { console.error('usage: a77-analyze.mjs <game.log>'); process.exit(2); }
const lines = readFileSync(file, 'utf8').split(/\r?\n/);
const out = { tool: 'thirdjam.a91/1', file };
let fails = 0;
const pass = (name, ok, detail) => { console.log(`${ok ? 'PASS' : 'FAIL'} ${name} ${detail}`); if (!ok) fails++; out[name] = { ok, detail }; };
const info = (name, detail) => { console.log(`INFO ${name} ${detail}`); out[name] = detail; };
const num = (re, l, d = NaN) => { const m = l.match(re); return m ? parseFloat(m[1]) : d; };
const strip = (l) => l.replace(/^.*?(JAM[A-Z]+)/, '$1');

const cues = {};
for (const l of lines) { const m = l.match(/JAMCUE (\w+) frame=/); if (m) cues[m[1]] = (cues[m[1]] || 0) + 1; }
info('cues', JSON.stringify(cues));
const drill = cues.sweep ? 7 : cues.spin ? 8 : cues.saucer ? 9 : cues.shot ? 10 : (cues.onetimer || cues.pass_to_mate) ? 11 : 0;
out.drill = drill;

// The real input path: the stick from the mouse, E as the spin edge, F as the held pass, the left button as the held shot.
const src = {};
for (const l of lines) { const m = l.match(/JAMCTRL frame=\d+ stick=\([^)]*\) src=(\w+)/); if (m) src[m[1]] = (src[m[1]] || 0) + 1; }
info('stick_sources', JSON.stringify(src));
const spinBtn = lines.filter((l) => /JAMCTRL frame=\d+ btn=spin \(E \/ B\)/.test(l)).length;
const passHeld = lines.filter((l) => /JAMCTRL frame=\d+ btn=pass held=1/.test(l)).length;
const shootHeld = lines.filter((l) => /JAMCTRL frame=\d+ btn=shoot held=1/.test(l)).length;
info('buttons', `E edges ${spinBtn}, F holds ${passHeld}, left button holds ${shootHeld}`);

if (drill === 7) {
  pass('mouse_reaches_stick', (src.mouse || 0) > 60, `stick frames from the mouse ${src.mouse || 0}`);
  const sweeps = lines.filter((l) => /JAMCTRL frame=\d+ sweep side=/.test(l));
  pass('sweeps_seen_by_sim', sweeps.length >= 10, `JAMCTRL sweep ${sweeps.length} on ${cues.sweep} cues (the cursor crossings the sim counted as sweeps on the carrier); bar 10`);
  const res = lines.filter((l) => /JAMDRILL stick_result sweep/.test(l)).map(strip);
  const kept = res.filter((l) => / kept=1/.test(l)).length, poked = res.filter((l) => / poked_clean=1/.test(l)).length;
  const refused = res.reduce((n, l) => n + num(/shield_refused=(\d+)/, l, 0), 0), clean = res.reduce((n, l) => n + num(/shield_clean=(\d+)/, l, 0), 0);
  pass('sweep_shield', res.length >= 10 && clean === 0, `attempts ${res.length}; inside the shield windows (${num(/shield_frames=(\d+)/, res[0] || '', 0)} frames) opponent pokes and lifts refused ${refused}, clean on the human ${clean}; bar: 10 attempts, 0 clean (R-0044 point 3)`);
  info('sweep_keeps_puck', `puck kept 30 frames later ${kept} of ${res.length}, poked clean in the 30 frames ${poked}, knocked ${res.filter((l) => / knocked=1/.test(l)).length} (reported; the body check is the risk the ruling gave the move)`);
  const pokesMiss = lines.filter((l) => /JAMEVENT poke .*kind=miss/.test(l)).length, pokesClean = lines.filter((l) => /JAMEVENT poke .*kind=clean/.test(l)).length;
  info('pokes', `clean ${pokesClean}, miss ${pokesMiss} (every opponent, every carrier)`);
}
if (drill === 8) {
  pass('spin_edges_arrive', spinBtn >= 10, `E presses seen as btn=spin ${spinBtn} on ${cues.spin} cues`);
  const spins = lines.filter((l) => /JAMCTRL frame=\d+ spin frames=/.test(l)).map(strip);
  const carrying = spins.filter((l) => / carrying=1/.test(l)).length;
  pass('spins_with_puck', carrying >= 10, `JAMCTRL spin ${spins.length}, with the puck ${carrying}; bar 10 with the puck`);
  const res = lines.filter((l) => /JAMDRILL stick_result spin/.test(l)).map(strip);
  const kept = res.filter((l) => / kept=1/.test(l)).length, knocked = res.filter((l) => / knocked=1/.test(l));
  const withPuck = res.length;
  const refused = res.reduce((n, l) => n + num(/shield_refused=(\d+)/, l, 0), 0), clean = res.reduce((n, l) => n + num(/shield_clean=(\d+)/, l, 0), 0);
  pass('spin_shield', withPuck >= 10 && clean === 0, `attempts ${withPuck}; inside the shield windows (${num(/shield_frames=(\d+)/, res[0] || '', 0)} frames) opponent pokes and lifts refused ${refused}, clean on the human ${clean}; bar: 10 attempts, 0 clean (R-0044 point 3)`);
  info('spin_keeps_puck', `puck kept 30 frames later ${kept} of ${res.length}, poked clean in the 30 frames ${res.filter((l) => / poked_clean=1/.test(l)).length} (reported; the body check is the risk the ruling gave the move)`);
  info('spin_checked_mid_turn', `knocked mid-spin ${knocked.length}, knock frames ${knocked.map((l) => num(/knock_frames=(\d+)/, l)).join(',') || '-'} (1.5x = 135)`);
  const lifts = lines.filter((l) => /JAMEVENT lift .*kind=clean/.test(l)).length;
  info('lifts_on_anyone', `clean stick lifts in the run ${lifts}, misses ${lines.filter((l) => /JAMEVENT lift .*kind=miss/.test(l)).length} (A82: the spin and the sweep shield the lift)`);
}
if (drill === 9) {
  pass('pass_held_arrives', passHeld >= 10, `pass (Q) holds seen ${passHeld} on ${cues.saucer} cues`);
  const sauc = lines.filter((l) => /JAMDRILL saucer n=/.test(l)).map(strip);
  const over = sauc.filter((l) => / over_blade=1/.test(l)).length, received = sauc.filter((l) => / received=1/.test(l)).length, stolen = sauc.filter((l) => / opp_touch=1/.test(l)).length;
  const landedBefore = sauc.filter((l) => num(/landed_frame=(-?\d+)/, l) >= 0 && num(/land_dist_to_receiver=([-\d.]+)/, l) > 0.5).length;
  pass('saucers_passed', sauc.length >= 10, `human saucers ${sauc.length}; bar 10`);
  const first10 = sauc.slice(0, 10), over10 = first10.filter((l) => / over_blade=1/.test(l)).length;
  pass('saucers_over_blade', first10.length >= 10 && over10 >= 10, `the first ten saucers (every cue had an opponent blade within 0.45 m of the lane, airborne part): over a blade (within 0.6 m of a blade at Z >= 0.2) ${over10} of ${first10.length}; all saucers ${over} of ${sauc.length}; nearest blade under the flight [${sauc.map((l) => num(/nearest_blade_air_m=([-\d.]+)/, l).toFixed(2)).join(', ')}]; bar 10 of 10 (R-0044 point 3)`);
  info('saucers_landed_before_receiver', `landed on the ice before the receiver ${landedBefore} of ${sauc.length}`);
  const cueBlades = lines.filter((l) => /JAMCUE saucer frame=/.test(l)).map((l) => num(/blade_lane_m=([\d.]+)/, l));
  info('saucer_cues', `blade to the lane at the cue [${cueBlades.map((v) => v.toFixed(2)).join(', ')}]`);
  info('saucer_landing', `received by a teammate ${received}; touched by an opponent ${stolen}; max Z [${sauc.map((l) => num(/max_z=([\d.]+)/, l).toFixed(2)).join(', ')}]`);
}
if (drill === 10 || drill === 11) {
  const shots = lines.filter((l) => /JAMDRILL shot n=/.test(l)).map(strip);
  const bins = { tap: [], half: [], full: [] };
  for (const l of shots) { const k = (l.match(/kind=(\w+)/) || [])[1]; if (bins[k]) bins[k].push({ charge: num(/charge=([\d.]+)/, l), target: num(/lift_target=([\d.]+)/, l), z: num(/arrival_z=([-\d.]+)/, l), goal: / goal=1/.test(l), save: / save=1/.test(l), crossed: / crossed=1/.test(l) }); }
  if (drill === 10) {
    pass('shots_from_10m', shots.length >= 30, `shots ${shots.length} on ${cues.shot} cues: taps ${bins.tap.length}, half ${bins.half.length}, full ${bins.full.length}; bar 10 each`);
    for (const k of ['tap', 'half', 'full']) {
      const arrived = bins[k].filter((s) => s.z >= 0);
      const within = arrived.filter((s) => Math.abs(s.z - s.target) <= 0.08).length;
      pass(`${k}_arrival_height`, bins[k].length >= 10 && within >= 10,
        `${bins[k].length} ${k}s, ${arrived.length} reached the line; arrival Z within 8 cm of shot.lift's target on ${within}; z [${arrived.map((s) => s.z.toFixed(2)).join(', ')}] targets [${arrived.map((s) => s.target.toFixed(2)).join(', ')}] charges [${bins[k].map((s) => s.charge.toFixed(2)).join(', ')}]; goals ${bins[k].filter((s) => s.goal).length}, saves ${bins[k].filter((s) => s.save).length}`);
    }
  }
  if (drill === 11) {
    const ot = lines.filter((l) => /JAMDRILL onetimer n=/.test(l)).map(strip);
    const made = ot.filter((l) => / one_timer=1/.test(l)).length;
    pass('one_timers_off_a_pass', made >= 5, `shots after the teammate's pass ${ot.length}, one-timers (kind one_timer: the full charge without the hold) ${made} on ${cues.onetimer} pass cues; since_pass frames [${ot.map((l) => num(/since_pass=(-?\d+)/, l)).join(', ')}]; bar 5`);
    info('one_timer_shots', shots.map((l) => l.replace(/^JAMDRILL shot /, '')).join(' | '));
  }
}
const releases = lines.filter((l) => /JAMSHOT release frame=/.test(l)).map(strip);
if (releases.length) info('shot_releases', `${releases.length} left-button releases: ${releases.slice(0, 12).map((l) => l.replace(/^JAMSHOT release /, '')).join(' | ')}${releases.length > 12 ? ' ...' : ''}`);
const passRel = lines.filter((l) => /JAMPASS release frame=/.test(l)).map(strip);
if (passRel.length) info('pass_releases', `${passRel.length} F releases: ${passRel.map((l) => l.replace(/^JAMPASS release /, '')).join(' | ')}`);
const handed = lines.filter((l) => /JAMDRILL hand_puck/.test(l)).length;
if (handed) info('hand_puck', `the drill handed the puck ${handed} times`);
const stills = lines.filter((l) => l.includes('JAMSTILLS start')).map(strip);
info('stills', `${stills.length} sequences: ${stills.join(' | ')}`);
const hashes = lines.filter((l) => l.includes('JAMSIM frame=')).map(strip);
if (hashes.length) info('sim', hashes[hashes.length - 1]);
out.verdict = fails ? 'fail' : 'pass';
console.log(JSON.stringify(out));
process.exit(fails ? 1 : 0);
