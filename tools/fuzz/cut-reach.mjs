#!/usr/bin/env node
// Launch-cut reach (lane A159, track S3, evolution 4, item 2; R-0057): every `-cut=` the acceptance run and the matrix use reaches a match with that cut's id in its
// JAMMATCH line. The menu's PLAY NOW once ignored `-cut=` and loaded the regular cut (H27, second attempt); the acceptance run then judged the wrong game.
//
//   node tools/fuzz/cut-reach.mjs [--jobs 3] [--only <substring>] [--out tools/fuzz/out/a159-cutreach] [--uproject Game/ThirdJam.uproject] [--timeout-s 150] [--port 4759]
//
// One headless `UnrealEditor-Cmd` per cut (-nullrhi -Multiprocess), the Menu map, `-cut=<value>` on the command line exactly as accept.ps1 passes it, and the QA console
// command `jam.menuscreen play` (PLAY NOW itself: UJamMenuWidget::PlayNow) sent over the game's remote console once the menu is up. The match's start line
//   JAMMATCH level=<l> source=<launch|settings|cartridge> cut=<CutName> id=<cartridge id>
// is read from the log; the driver then stops its own child (never another process). A cut passes when cut= equals the argument, id= equals the cartridge file's `id`,
// and source=launch (a launch cut plays its own level), judged from the LAST JAMMATCH line, and exactly one JAMMATCH and one JAMCART loaded in the log (a match built twice by
// the menu's press, the second build at the cartridge's own level instead of the player's, was the fault of 14 Nov). Fails: no JAMMATCH, the regular cut's id where another was asked, a wrong id.
//
// The cuts: the acceptance run's own (accept-cut.json and the a93/reach variants), the matrix's twelve modes and tiers (matrix-cuts/), the regular cut by path and by name,
// the twelve mode cuts and the season's eight takes by path, the regular cut and arenas by name. Exit 0 all pass, 1 a cut failed, 2 could not run.
import { spawn, spawnSync } from 'node:child_process';
import { existsSync, mkdirSync, readFileSync, readdirSync, writeFileSync, rmSync } from 'node:fs';
import net from 'node:net';
import os from 'node:os';
import { dirname, join, resolve, basename } from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = dirname(fileURLToPath(import.meta.url));
const REPO = resolve(HERE, '..', '..');
const argv = process.argv.slice(2);
const arg = (k, d) => { const i = argv.indexOf(`--${k}`); return i >= 0 ? argv[i + 1] : d; };
const jobs = Number(arg('jobs', '3'));
const only = arg('only', '');
const out = resolve(arg('out', join(HERE, 'out', 'a159-cutreach')));
const uproject = resolve(arg('uproject', join(REPO, 'Game', 'ThirdJam.uproject')));
const timeoutMs = Number(arg('timeout-s', '150')) * 1000;
const basePort = Number(arg('port', '4759'));
const ueRoot = process.env.UE_ROOT || 'C:\\Program Files\\Epic Games\\UE_5.9';
const exe = join(ueRoot, 'Engine', 'Binaries', 'Win64', 'UnrealEditor-Cmd.exe');
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
const usedGb = () => (os.totalmem() - os.freemem()) / 2 ** 30;

export function cutList() {
  const list = [];
  const add = (label, value, file) => list.push({ label, value, id: JSON.parse(readFileSync(file, 'utf8')).id });
  const acc = join(REPO, 'tools', 'qa', 'accept');
  for (const f of readdirSync(acc).filter((x) => /^(accept-cut|a93-cut).*\.json$/.test(x)).sort()) add(`accept/${f}`, join(acc, f), join(acc, f));
  for (const f of readdirSync(join(acc, 'matrix-cuts')).filter((x) => x.endsWith('.json')).sort()) add(`matrix/${f}`, join(acc, 'matrix-cuts', f), join(acc, 'matrix-cuts', f));
  const cdir = join(REPO, 'Game', 'Content', 'Cartridges');
  add('regular (path, as run-matrix regcart)', join(cdir, 'hockey.regular.cartridge.json'), join(cdir, 'hockey.regular.cartridge.json'));
  for (const n of ['hockey.regular', 'hockey.arena.wide', 'hockey.arena.large']) add(`name ${n}`, n, join(cdir, `${n}.cartridge.json`));
  for (const f of readdirSync(join(cdir, 'modes')).filter((x) => x.endsWith('.cartridge.json')).sort()) add(`mode ${f.replace('.cartridge.json', '')}`, join(cdir, 'modes', f), join(cdir, 'modes', f));
  for (const f of readdirSync(join(cdir, 'ladder')).filter((x) => /^take-\d\.cartridge\.json$/.test(x)).sort()) add(`take ${f.replace('.cartridge.json', '')}`, join(cdir, 'ladder', f), join(cdir, 'ladder', f));
  // the player's own press: no -cut= at all (value null). PLAY NOW starts the regular cut at the player's level through the Designer (the path of the double build, 14 Nov)
  list.push({ label: 'PLAY NOW without -cut (the player)', value: null, id: 'hockey-regular', player: true });
  return list;
}

function rcExec(port, cmd) {
  return new Promise((resolveP) => {
    const s = net.connect(port, '127.0.0.1');
    let buf = '';
    let stage = 0;
    const done = (v) => { try { s.destroy(); } catch { /* closed */ } resolveP(v); };
    s.setTimeout(8000, () => done(null));
    s.on('error', () => done(null));
    s.on('connect', () => s.write('{"id":1,"op":"hello","version":1}\n'));
    s.on('data', (d) => {
      buf += d.toString('utf8');
      let i;
      while ((i = buf.indexOf('\n')) >= 0) {
        const line = buf.slice(0, i); buf = buf.slice(i + 1);
        if (stage === 0) { stage = 1; s.write(`${JSON.stringify({ id: 2, op: 'exec', cmd })}\n`); }
        else if (line.startsWith('{"id":2,')) return done(line);
      }
    });
  });
}

async function playOne(cut, k) {
  const dir = join(out, `j${k}`);
  mkdirSync(dir, { recursive: true });
  const log = join(dir, `ue-${basename(cut.label).replace(/[^\w.-]/g, '_')}.log`);
  rmSync(log, { force: true });
  const port = basePort + k;
  const args = [uproject, '/Game/Maps/Menu', '-game', ...(cut.value === null ? [] : [`-cut=${cut.value.includes(' ') ? `"${cut.value}"` : cut.value}`]), '-nullrhi', '-Multiprocess', '-unattended', '-nopause', '-nosplash', '-log', `-ABSLOG=${log}`,
    '-ForceLogFlush', '-RemoteConsoleIP=127.0.0.1', `-RemoteConsolePort=${port}`];
  const child = spawn(exe, args, { stdio: 'ignore', windowsHide: true });
  const t0 = Date.now();
  let pressed = 0, match = null, menuSeen = false, rc = '';
  const read = () => { try { return readFileSync(log, 'utf8'); } catch { return ''; } };
  let exited = false;
  child.on('exit', () => { exited = true; });
  while (Date.now() - t0 < timeoutMs && !exited) {
    await sleep(1500);
    const text = read();
    const m = /JAMMATCH level=(\S+) source=(\S+) cut=(.*?) id=(\S+)/.exec(text);
    if (m) { match = { level: m[1], source: m[2], cut: m[3], id: m[4] }; break; }
    if (!menuSeen && /\/Game\/Maps\/Menu/.test(text)) { menuSeen = true; await sleep(4000); }
    if (menuSeen && pressed < 6 && Date.now() - t0 > pressed * 12000) { rc = await rcExec(port, 'jam.menuscreen play'); pressed++; }
  }
  spawnSync('taskkill', ['/PID', String(child.pid), '/T', '/F'], { stdio: 'ignore' });   // our own child only
  const fin = read();
  const all = [...fin.matchAll(/JAMMATCH level=(\S+) source=(\S+) cut=(.*?) id=(\S+)/g)];
  const lm = all[all.length - 1];
  if (lm) match = { level: lm[1], source: lm[2], cut: lm[3], id: lm[4], matches: all.length, carts: (fin.match(/JAMCART loaded/g) ?? []).length, taken_back: /scheduled apply taken back/.test(fin) };
  const reject = /JAMMENU cut .* rejected: (.*)/.exec(fin)?.[1];
  return { label: cut.label, value: cut.value, want_id: cut.id, got: match, pressed, rc: String(rc).slice(0, 80), reject, seconds: Math.round((Date.now() - t0) / 1000), log };
}

async function main() {
  if (!existsSync(exe)) { console.log(`FAIL cut-reach no ${exe} (set UE_ROOT)`); process.exit(2); }
  const cuts = cutList().filter((c) => !only || c.label.includes(only));
  mkdirSync(out, { recursive: true });
  console.log(`INFO cut-reach ${cuts.length} cuts, ${jobs} job(s), ${usedGb().toFixed(1)} GB in use`);
  const results = [];
  let next = 0;
  await Promise.all(Array.from({ length: jobs }, async (_, k) => {
    await sleep(k * 6000);
    while (next < cuts.length) {
      const c = cuts[next++];
      while (usedGb() > 50) { console.log(`INFO job ${k}: ${usedGb().toFixed(1)} GB in use, waiting`); await sleep(15000); }
      let r = await playOne(c, k);
      if (!r.got) { console.log(`INFO ${c.label}: no JAMMATCH (machine load?), one retry`); const r2 = await playOne(c, k); r2.retried = true; r = r2; }
      const g = r.got;
      const ok = !!g && (c.player ? g.source === 'settings' : g.cut === c.value && g.source === 'launch') && g.id === c.id && g.matches === 1 && g.carts === 1;
      r.ok = ok;
      results.push(r);
      console.log(`${ok ? 'PASS' : 'FAIL'} ${c.label}: ${g ? `JAMMATCH level=${g.level} source=${g.source} id=${g.id} (want ${c.id}) JAMMATCH x${g.matches}, JAMCART loaded x${g.carts}${g.taken_back ? ', apply taken back' : ''}` : `no JAMMATCH${r.reject ? `, rejected: ${r.reject}` : ''} after ${r.seconds}s (pressed ${r.pressed}, rc ${r.rc})`}`);
    }
  }));
  const failed = results.filter((r) => !r.ok);
  writeFileSync(join(out, 'report.json'), JSON.stringify({ tool: 'thirdjam.cutreach/1', total: results.length, failed: failed.length, results }, null, 1));
  console.log(`${failed.length ? 'FAIL' : 'PASS'} cut-reach ${results.length - failed.length} of ${results.length} cuts reach their match`);
  process.exit(failed.length ? 1 : 0);
}
if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) main();
