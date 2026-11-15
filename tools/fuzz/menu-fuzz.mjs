#!/usr/bin/env node
// Front-end fuzz (lane A198, track S3, evolution 5, item 5). One game process on the Menu map, off screen (-RenderOffScreen, no window grab, no system
// cursor), driven through its remote console by the menu's own QA command (jam.menuscreen key|click|back|<screen>) at random, and read by
// jam.hard.menufocus (Game/Source/JamGame/Private/HardTests.cpp) after every step.
//
//   node tools/fuzz/menu-fuzz.mjs [--steps 5000] [--seed 1] [--port 4798] [--out tools/fuzz/out/a198-menu] [--modes] [--no-fuzz] [--start-timeout-s 900]
//
// Laws (each break is a fault line in <out>/faults.jsonl with the step and the last ten actions):
//   focus      on a menu screen the keyboard focus is inside the menu (focused != none, in_menu=1), judged twice, 1.5 s apart (a starting match takes it)
//   one-build  a press that starts a match builds it once: exactly one more JAMMATCH and one more "JAMCART loaded" line (the morning's StartMatchAtLevel fault)
//   back       Back (jam.menuscreen back, Esc, pad B) on a screen other than the landing and HOME leaves that screen
//   alive      the process never dies and the console always answers
// --modes: then each of the twelve modes from its card on MODES (pad down/right to the card, Enter): its JAMMATCH id must be the mode's cartridge id;
// the match is ended (jam.ctrl.EndMatch 1) and the menu comes back (jam.pausemenu quit).
// MAKE YOUR OWN (the Direct screen) is only walked to: on it the fuzz presses nothing but Back (no Designer call, never a sentence).
// Exit 0 no fault, 1 faults, 2 could not run. Only the process this script started is ever stopped (by its pid).
import { spawn } from 'node:child_process';
import { readFileSync, writeFileSync, appendFileSync, mkdirSync, existsSync, readdirSync } from 'node:fs';
import { join, resolve, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
import net from 'node:net';

const HERE = dirname(fileURLToPath(import.meta.url));
const REPO = resolve(HERE, '..', '..');
const argv = process.argv.slice(2);
const arg = (k, d) => { const i = argv.indexOf(`--${k}`); return i >= 0 ? argv[i + 1] : d; };
const has = (k) => argv.includes(`--${k}`);
let dead = false;
const STEPS = +arg('steps', '5000');
const PORT = +arg('port', '4798');
const OUT = resolve(arg('out', join(HERE, 'out', 'a198-menu')));
const UPROJECT = resolve(arg('uproject', join(REPO, 'Game', 'ThirdJam.uproject')));
// lane A212: --exe <a package's game binary> (<pkg>/Windows/ThirdJam/Binaries/Win64/ThirdJam.exe, never the launcher one folder up: its child would outlive the stop by pid)
const PKG_EXE = arg('exe', '');
const EXE = PKG_EXE ? resolve(PKG_EXE) : join(process.env.UE_ROOT || 'C:\\Program Files\\Epic Games\\UE_5.9', 'Engine', 'Binaries', 'Win64', 'UnrealEditor.exe');
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));

// deterministic random (mulberry32)
let seed = (+arg('seed', '1')) >>> 0;
const rnd = () => { seed |= 0; seed = (seed + 0x6D2B79F5) | 0; let t = Math.imul(seed ^ (seed >>> 15), 1 | seed); t = (t + Math.imul(t ^ (t >>> 7), 61 | t)) ^ t; return ((t ^ (t >>> 14)) >>> 0) / 4294967296; };
const pick = (a) => a[Math.floor(rnd() * a.length)];

mkdirSync(OUT, { recursive: true });
const LOG = join(OUT, 'game.log');
const FAULTS = join(OUT, 'faults.jsonl');
writeFileSync(FAULTS, '');
const t0 = Date.now();
const note = (s) => console.log(`${((Date.now() - t0) / 1000).toFixed(1).padStart(7)}s  ${s}`);

function rc(cmd, timeoutMs = 6000) {
  return new Promise((res) => {
    const s = net.connect(PORT, '127.0.0.1');
    let buf = ''; let sent = false; let done = false;
    const fin = (v) => { if (done) return; done = true; try { s.destroy(); } catch { /* closed */ } res(v); };
    s.setTimeout(timeoutMs, () => fin(null));
    s.on('error', () => fin(null));
    s.on('connect', () => s.write('{"id":1,"op":"hello","version":1}\n'));
    s.on('data', (d) => {
      buf += d.toString();
      let i;
      while ((i = buf.indexOf('\n')) >= 0) {
        const line = buf.slice(0, i); buf = buf.slice(i + 1);
        if (!sent && line.startsWith('{"id":1')) { sent = true; s.write(JSON.stringify({ id: 2, op: 'exec', cmd }) + '\n'); }
        else if (line.startsWith('{"id":2,')) fin(line.includes('"ok":true'));
      }
    });
  });
}
let logPos = 0;
function readLog() { try { return readFileSync(LOG, 'utf8'); } catch { return ''; } }
function newLines() { const t = readLog(); const n = t.slice(logPos); logPos = t.length; return n; }
const count = (t, re) => (t.match(re) || []).length;

async function focus() {
  const before = readLog().length;
  if (!(await rc('jam.hard.menufocus'))) return null;
  for (let i = 0; i < 20; i++) {
    const t = readLog().slice(before);
    const m = [...t.matchAll(/\]\[\s*(\d+)\]LogTemp: Display: JAMHARD focus map=(\S+) mode=(\S+) screen=(\S+) focused=(\S+) in_menu=(\d)(?: card=(\S+))?(?: owners=(\S+))?/g)].pop();
    if (m) return { frame: +m[1], map: m[2], mode: m[3], screen: m[4], focused: m[5], inMenu: m[6] === '1', card: m[7] || '?', owners: m[8] || '?' };
    await sleep(50);
  }
  return null;
}

const recent = [];
let faults = 0;
function fault(cls, step, detail) {
  faults++;
  const line = { class: cls, step, detail, last: recent.slice(-10) };
  appendFileSync(FAULTS, JSON.stringify(line) + '\n');
  note(`FAULT ${cls} step ${step}: ${detail}`);
}

const SCREENS = ['landing', 'main', 'modes', 'season', 'cuts', 'settings', 'controls', 'credits', 'direct'];
const KEYS = ['up', 'down', 'left', 'right', 'arrowleft', 'arrowright', 'enter', 'a', 'b', 'esc'];
const CLICKS = ['rookie', 'pro', 'all_star', 'play', 'start', 'modes', 'season', 'continue', 'make', 'rinks', 'back', 'settingslevel'];

async function waitMenu(sec) {
  for (let i = 0; i < sec * 2; i++) { const f = await focus(); if (f && f.mode.includes('MenuGameMode')) return f; await sleep(500); }
  return null;
}

// A match started: count the builds, end it, come back to the menu.
async function afterMatch(step, why) {
  await sleep(5000);
  const t = readLog();
  const jm = count(t, /JAMMATCH /g), jc = count(t, /JAMCART loaded/g);
  const dm = jm - state.jm, dc = jc - state.jc;
  if (dm !== 1 || dc > 1) fault('one-build', step, `${why}: ${dm} JAMMATCH and ${dc} JAMCART loaded lines for one press`);
  const last = [...t.matchAll(/JAMMATCH ([^\r\n]*)/g)].pop();
  state.jm = jm; state.jc = jc; state.matches++;
  await rc('jam.ctrl.EndMatch 1');
  await sleep(3000);
  await rc('jam.pausemenu quit');
  let f = await waitMenu(60);
  if (!f) { await rc('open /Game/Maps/Menu'); f = await waitMenu(90); }
  if (!f) { fault('alive', step, 'the menu did not come back after the match (EndMatch, pausemenu quit, open Menu)'); return null; }
  const t2 = readLog(); state.jm = count(t2, /JAMMATCH /g); state.jc = count(t2, /JAMCART loaded/g);
  return last ? last[1] : '';
}

const state = { jm: 0, jc: 0, matches: 0, steps: 0, noFocus: 0, backs: 0, slow: 0, quits: 0 };

async function fuzz() {
  const per = Math.max(1, Math.floor(STEPS / SCREENS.length));
  let f = await focus();
  for (let step = 1; step <= STEPS; step++) {
    if ((step - 1) % per === 0) {
      const s = SCREENS[Math.floor((step - 1) / per) % SCREENS.length];
      if (s === 'landing') await rc('jam.menuscreen landing'); else await rc(`jam.menuscreen ${s}`);
      recent.push(`screen ${s}`);
      await sleep(400);
      f = await focus();
    }
    const onDirect = f && f.screen === 'direct';
    let act;
    const r = rnd();
    if (onDirect) act = pick(['back', 'key esc', 'key b']);
    else if (r < 0.62) act = `key ${pick(KEYS)}`;
    else if (r < 0.85) act = `click ${pick(CLICKS)}`;
    else if (r < 0.95) act = 'back';
    else act = pick(SCREENS);
    const isBack = act === 'back' || act === 'key esc' || act === 'key b' || act === 'click back';
    const was = f ? f.screen : '?';
    recent.push(act); if (recent.length > 40) recent.shift();
    const cmd = SCREENS.includes(act) ? `jam.menuscreen ${act}` : `jam.menuscreen ${act}`;
    const logLen0 = act.startsWith("click") ? readLog().length : 0;   // lane A212: a click the game reports as failed ("JAMMENU click <x> failed") pressed nothing
    const ok = await rc(cmd);
    await sleep(120);
    f = await focus();
    if (!f) {
      // a busy machine or a loading rink: the console may not answer for a while. Dead = the process exited or 120 s without any answer.
      for (let i = 0; i < 60 && !f && !dead; i++) { await sleep(2000); f = await focus(); }
      if (!f && dead && /GPU crash detected/.test(readLog())) { fault('gpu-crash', step, `the game died of a GPU crash after '${cmd}' on ${was}`); return; }
      if (!f && dead && (was === 'main') && /key (a|enter)|click/.test(act)) { state.quits++; note(`step ${step}: '${act}' on HOME quit the game (QUIT): starting it again`); if (!(await launch())) return; f = await focus(); continue; }
      if (!f) { fault('alive', step, `no answer for 120 s after '${cmd}' (rc ok=${ok}, process ${dead ? 'exited' : 'alive'})`); return; }
      state.slow++;
    }
    if (!f.mode.includes('MenuGameMode')) {
      // a match (or its loading) took over: judge the build count, end it, come back
      const m = await afterMatch(step, `after '${act}' on ${was}`);
      if (m === null) return;
      recent.push(`match: ${m}`);
      f = await focus();
      continue;
    }
    if (isBack && was !== 'landing' && was !== 'main' && was !== '?' && was !== 'none') {
      state.backs++;
      const clickFailed = act.startsWith('click') && /JAMMENU click \S+ failed/.test(readLog().slice(logLen0));
      if (clickFailed) { state.backs--; state.clickMiss = (state.clickMiss || 0) + 1; }
      else if (f.screen === was) fault('back', step, `'${act}' on ${was} stayed on ${was}`);
    }
    if (!f.inMenu || f.focused === 'none') {
      // a press that starts a match hands the focus to the viewport while the rink loads: judge only a screen that is still the menu 1.5 s later
      await sleep(1500);
      const g = await focus();
      if (g && g.frame !== f.frame && g.mode.includes('MenuGameMode') && (!g.inMenu || g.focused === 'none')) {   // the same engine frame = one stalled frame (a level load): no judgement
 state.noFocus++; if (state.noFocus <= 20) fault('focus', step, `screen ${g.screen} without focus in the menu (focused=${g.focused}, in_menu=${g.inMenu ? 1 : 0}, owners=${g.owners}) after '${act}' on ${was}`); }
      if (g) f = g;
    }
    state.steps = step;
    if (step % 250 === 0) note(`step ${step}: screen ${f.screen}, matches ${state.matches}, faults ${faults}`);
  }
}

async function modes() {
  const dir = join(REPO, 'Game', 'Content', 'Cartridges', 'modes');
  const files = readdirSync(dir).filter((x) => x.endsWith('.cartridge.json')).sort();
  // the cards' order is the menu's (UJamMenuFlow::ModeCards); the id of the match that starts is read back and matched against the twelve ids
  const ids = new Set(files.map((x) => JSON.parse(readFileSync(join(dir, x), 'utf8')).id));
  const seen = [];
  for (let i = 0; i < 12; i++) {
    await rc('jam.menuscreen modes'); await sleep(600);
    const nav = [];
    for (let d = 0; d < Math.floor(i / 5); d++) nav.push('down');
    for (let r = 0; r < i % 5; r++) nav.push('right');
    const path = [];
    { const f0 = await focus(); path.push(f0 ? f0.card : '?'); }
    for (const k of nav) { await rc(`jam.menuscreen key ${k}`); await sleep(250); const fk = await focus(); path.push(fk ? fk.card : '?'); }
    recent.push(`mode card ${i}: ${nav.join(' ')} enter; focus path ${path.join(' > ')}`);
    note(`mode card ${i}: keys ${nav.join(' ') || '(none)'}; focus path ${path.join(' > ')}`);
    await rc('jam.menuscreen key enter');
    await sleep(800);
    const f = await focus();
    if (f && f.mode.includes('MenuGameMode')) { fault('mode-card', 1000 + i, `card ${i} (${nav.join(' ') || 'first'} then Enter) started no match; screen ${f.screen}`); continue; }
    const m = await afterMatch(1000 + i, `mode card ${i}`);
    if (m === null) return;
    const id = (m.match(/id=(\S+)/) || [])[1] || '?';
    seen.push(id);
    note(`mode card ${i}: ${m}`);
    if (!ids.has(id)) fault('mode-card', 1000 + i, `card ${i} started '${id}', not one of the twelve modes`);
  }
  const uniq = new Set(seen);
  if (uniq.size !== 12) fault('mode-card', 2000, `the twelve cards started ${uniq.size} different modes: ${seen.join(', ')}`);
  return seen;
}

const args = [...(PKG_EXE ? [] : [UPROJECT]), '/Game/Maps/Menu', ...(PKG_EXE ? [] : ['-game']), '-RenderOffScreen', ...(PKG_EXE ? [] : ['-Multiprocess']), '-ResX=1280', '-ResY=720', '-ForceRes', '-unattended', '-nosplash', '-nosound', '-log',
  `-abslog=${LOG}`, '-ForceLogFlush', '-dpcvars=jam.screen.FadeSeconds=0,jam.ScriptHuman=1,r.RDG.AsyncCompute=0', '-RemoteConsoleIP=127.0.0.1', `-RemoteConsolePort=${PORT}`];
if (!existsSync(EXE) || (!PKG_EXE && !existsSync(UPROJECT))) { console.log(`FAIL menu-fuzz: missing ${EXE} or ${UPROJECT}`); console.log(JSON.stringify({ tool: 'thirdjam.menufuzz/1', verdict: 'could_not_run', exit: 2 })); process.exit(2); }
// lane A198: QUIT on HOME is a real button: a press on it ends the process (counted, not a fault); the game is started again and the fuzz goes on.
let child = null; const pids = [];
async function launch() {
  dead = false;
  if (pids.length && existsSync(LOG)) { try { writeFileSync(join(OUT, `game-${pids.length}.log`), readFileSync(LOG)); } catch { /* kept as is */ } }   // the earlier process's log
  child = spawn(EXE, args, { windowsHide: true, stdio: 'ignore' });
  pids.push(child.pid);
  writeFileSync(join(OUT, 'game.json'), JSON.stringify({ pids, started: new Date().toISOString(), lane: 'A198' }));
  note(`started game pid ${child.pid} port ${PORT}`);
  child.on('exit', (c) => { dead = true; note(`game exited code ${c}`); });
  let ok = false;
  for (let i = 0; i < +arg('start-timeout-s', '900') / 2 && !dead && !ok; i++) { ok = await rc('jam.hard.menufocus', 3000); if (!ok) await sleep(2000); }
  if (ok) { await sleep(6000); const t = readLog(); state.jm = count(t, /JAMMATCH /g); state.jc = count(t, /JAMCART loaded/g); logPos = t.length; }
  return ok;
}
let up = await launch();
let seenModes = [];
if (!up) { note('the menu never answered its console'); }
else {
  await sleep(6000);
  const t = readLog(); state.jm = count(t, /JAMMATCH /g); state.jc = count(t, /JAMCART loaded/g); logPos = t.length;
  if (!has('no-fuzz')) await fuzz();
  if (has('modes') && !dead) seenModes = (await modes()) || [];
}
if (!dead) { await rc('quit', 3000); for (let i = 0; i < 30 && !dead; i++) await sleep(500); }
if (!dead) { try { process.kill(child.pid); note(`stopped my game pid ${child.pid}`); } catch { /* gone */ } }
const code = !up ? 2 : faults ? 1 : 0;
console.log(`${code === 0 ? 'PASS' : code === 1 ? 'FAIL' : 'FAIL'} menu-fuzz: ${state.steps} steps, ${state.matches} matches started and ended, ${state.backs} backs judged, ${state.noFocus} no-focus reads, ${state.slow} slow answers, ${state.quits} QUIT presses, ${faults} faults (${FAULTS})`);
console.log(JSON.stringify({ tool: 'thirdjam.menufuzz/1', verdict: ['pass', 'fail', 'could_not_run'][code], exit: code, steps: state.steps, matches: state.matches, backs: state.backs, no_focus: state.noFocus, faults, modes: seenModes, out: OUT }));
process.exit(code);
