#!/usr/bin/env node
// Lane A151 (track F1, evolution 5, item 3): the hit the player makes is the hit he sees. Reads a headless game log (tools/commentary/match.ps1) and lays
// the sim's contact events beside the presentation's own log lines, frame by frame:
//   JAMEVENT check / knockdown (the sim), JAMHITSTOP (the hold, HokejGameMode), JAMKICK (the camera, CameraFollow), JAMHUD (the contact line, HUDWidget).
// For the human's own landed checks and for the hits on him: how many had a hold, a kick, a HUD line on the same frame; and what stayed silent.
//   node tools/qa/ctrl/a151-feel.mjs <game.log> [humanIndex]
import { readFileSync } from 'node:fs';

const [file, humanArg] = process.argv.slice(2);
if (!file) { console.error('usage: a151-feel.mjs <game.log> [humanIndex]'); process.exit(2); }
const lines = readFileSync(file, 'utf8').split(/\r?\n/);
const checks = [], downs = [], stops = [], kicks = [], huds = [];
let human = humanArg !== undefined ? Number(humanArg) : -1;
for (const l of lines) {
  let m;
  if ((m = l.match(/JAMEVENT check who=(-?\d+) .* strength=([0-9.]+) frame=(\d+) kind=(\S*) outcome=(\S+)/))) checks.push({ who: +m[1], strength: +m[2], frame: +m[3], kind: m[4], outcome: m[5] });
  else if ((m = l.match(/JAMEVENT knockdown who=(-?\d+) .* frame=(\d+) kind=(\S*)/))) downs.push({ who: +m[1], frame: +m[2], kind: m[3] });
  else if ((m = l.match(/JAMHITSTOP frame=(\d+) frames=(\d+) victim=(-?\d+) checker=(-?\d+) human=(-?\d+) kind=(\S*) strength=([0-9.]+) outcome=(\d+) ratio=([0-9.]+)/))) {
    stops.push({ frame: +m[1], frames: +m[2], victim: +m[3], checker: +m[4], outcome: +m[8], ratio: +m[9] });
    if (human < 0) human = +m[5];
  }
  else if ((m = l.match(/JAMKICK frame=(-?\d+) (\S+) strength=([0-9.]+) .* cm=([0-9.]+) frames=(\d+)/))) kicks.push({ frame: +m[1], what: m[2], cm: +m[4], frames: +m[5] });
  else if ((m = l.match(/JAMHUD frame=(\d+) contact_line=(\S*) refused=(\d+) recoils=(\d+) soft=(\d+) missed=(\d+) graced=(\d+)/))) huds.push({ frame: +m[1], line: m[2].replace(/_/g, ' ') });
}
if (human < 0) human = 0;
const near = (list, frame, pred = () => true, slack = 1) => list.filter((x) => Math.abs(x.frame - frame) <= slack && pred(x));
const inHold = (frame) => stops.some((s) => frame > s.frame && frame <= s.frame + s.frames - 1);
const dist = (xs) => { const o = {}; for (const x of xs) o[x] = (o[x] || 0) + 1; return Object.entries(o).sort().map(([k, v]) => `${k}: ${v}`).join(', ') || 'none'; };
const out = { tool: 'thirdjam.a151.feel/1', log: file, human, events: { checks: checks.length, knockdowns: downs.length, hitstops: stops.length, kicks: kicks.length, hud_lines: huds.length } };
console.log(`log ${file}: human = skater ${human}; check events ${checks.length}, knockdown events ${downs.length}, JAMHITSTOP ${stops.length}, JAMKICK ${kicks.length}, JAMHUD ${huds.length}`);

// 1. The human's own landed checks.
for (const oc of ['down', 'stumble']) {
  const mine = checks.filter((c) => c.who === human && c.outcome === oc);
  let hold = 0, kick = 0, held = 0, silent = 0; const holdFrames = [], kickWhat = [], kickCm = [];
  for (const c of mine) {
    const s = near(stops, c.frame, (x) => x.checker === human, 0), k = near(kicks, c.frame);
    if (s.length) { ++hold; holdFrames.push(s[0].frames); } else if (inHold(c.frame)) ++held;
    if (k.length) { ++kick; kickWhat.push(k[0].what); kickCm.push(k[0].cm.toFixed(0) + ' cm'); }
    if (!s.length && !k.length) ++silent;
  }
  out[`human_${oc}`] = { n: mine.length, hold, inside_an_earlier_hold: held, kick, neither: silent };
  console.log(`the human's own check, outcome ${oc}: ${mine.length} | a hold on the same frame ${hold} (frames ${dist(holdFrames)}), no hold because an earlier hold was still running ${held} | a camera kick ${kick} (${dist(kickWhat)}; ${dist(kickCm)}) | neither hold nor kick ${silent}`);
}
// 2. Hits on the human that put him down, and the stumbles he took (the camera's own line names those).
{
  const mine = downs.filter((d) => d.who === human && d.kind === 'check');
  let hold = 0, kick = 0, held = 0, silent = 0; const holdFrames = [], kickWhat = [];
  for (const d of mine) {
    const s = near(stops, d.frame, (x) => x.victim === human, 0), k = near(kicks, d.frame);
    if (s.length) { ++hold; holdFrames.push(s[0].frames); } else if (inHold(d.frame)) ++held;
    if (k.length) { ++kick; kickWhat.push(k[0].what); }
    if (!s.length && !k.length) ++silent;
  }
  out.human_downed = { n: mine.length, hold, inside_an_earlier_hold: held, kick, neither: silent };
  console.log(`the human knocked down by a check: ${mine.length} | a hold ${hold} (frames ${dist(holdFrames)}), inside an earlier hold ${held} | a camera kick ${kick} (${dist(kickWhat)}) | neither ${silent}`);
  const st = kicks.filter((k) => k.what === 'stumble_taken'), bo = kicks.filter((k) => k.what === 'bounce');
  const stHold = st.filter((k) => near(stops, k.frame, (x) => x.victim === human, 0).length).length;
  const boHud = bo.filter((k) => near(huds, k.frame, () => true, 2).length).length;
  out.human_stumbled = { kicks: st.length, with_hold: stHold }; out.human_bounced = { kicks: bo.length, with_hud_line: boHud };
  console.log(`stumbles the human took (the camera's stumble_taken): ${st.length}, with a hold ${stHold} | the human's own bounce off a braced skater (the camera's bounce): ${bo.length}, with a HUD line within 2 frames ${boHud}`);
}
// 3. The HUD's contact line.
out.hud = {}; for (const h of huds) out.hud[h.line] = (out.hud[h.line] || 0) + 1;
console.log(`the HUD's contact line: ${dist(huds.map((h) => h.line))}`);
// 4. Holds and kicks by what they are.
console.log(`holds by outcome and frames: ${dist(stops.map((s) => `${s.outcome === 3 ? 'down' : s.outcome === 2 ? 'stumble' : 'bounce/other'} ${s.frames}f ${s.checker === human ? 'by the human' : s.victim === human ? 'on the human' : 'bot on bot'}`))}`);
console.log(`kicks by kind: ${dist(kicks.map((k) => k.what))}`);
console.log(JSON.stringify(out));
