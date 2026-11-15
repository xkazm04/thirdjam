#!/usr/bin/env node
// Lane A198: a front-end repeat, step by step. One off-screen editor-binary game on the Menu map, a list of console commands, and after each the
// JAMHARD focus line (jam.hard.menufocus: screen, focused widget, inside the menu or not, its owners).
//   node tools/fuzz/menu-repro.mjs --port 4796 --out tools/fuzz/out/repro-x "jam.menuscreen start" "jam.lobby" "jam.menuscreen key down" ...
import { spawn } from 'node:child_process';
import { readFileSync, mkdirSync } from 'node:fs';
import { join, resolve, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
import net from 'node:net';

const HERE = dirname(fileURLToPath(import.meta.url));
const REPO = resolve(HERE, '..', '..');
const argv = process.argv.slice(2);
const opt = (k, d) => { const i = argv.indexOf(`--${k}`); if (i < 0) return d; const v = argv[i + 1]; argv.splice(i, 2); return v; };
const PORT = +opt('port', '4796');
const OUT = resolve(opt('out', join(HERE, 'out', 'menu-repro')));
const cmds = argv;
mkdirSync(OUT, { recursive: true });
const LOG = join(OUT, 'game.log');
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
function rc(cmd, timeoutMs = 5000) {
  return new Promise((res) => {
    const s = net.connect(PORT, '127.0.0.1'); let buf = ''; let sent = false; let done = false;
    const fin = (v) => { if (done) return; done = true; try { s.destroy(); } catch { /* closed */ } res(v); };
    s.setTimeout(timeoutMs, () => fin(null)); s.on('error', () => fin(null));
    s.on('connect', () => s.write('{"id":1,"op":"hello","version":1}\n'));
    s.on('data', (d) => { buf += d.toString(); let i; while ((i = buf.indexOf('\n')) >= 0) { const l = buf.slice(0, i); buf = buf.slice(i + 1);
      if (!sent && l.startsWith('{"id":1')) { sent = true; s.write(JSON.stringify({ id: 2, op: 'exec', cmd }) + '\n'); } else if (l.startsWith('{"id":2,')) fin(l.includes('"ok":true')); } });
  });
}
const readLog = () => { try { return readFileSync(LOG, 'utf8'); } catch { return ''; } };
async function focus() { const n = readLog().length; await rc('jam.hard.menufocus'); await sleep(300); const m = [...readLog().slice(n).matchAll(/JAMHARD focus (.*)/g)].pop(); return m ? m[1].trim() : '(no line)'; }
const exe = join(process.env.UE_ROOT || 'C:\\Program Files\\Epic Games\\UE_5.9', 'Engine', 'Binaries', 'Win64', 'UnrealEditor.exe');
const child = spawn(exe, [join(REPO, 'Game', 'ThirdJam.uproject'), '/Game/Maps/Menu', '-game', '-RenderOffScreen', '-Multiprocess', '-ResX=1280', '-ResY=720', '-ForceRes', '-unattended', '-nosplash', '-nosound', '-log', `-abslog=${LOG}`,
  '-ForceLogFlush', '-dpcvars=jam.screen.FadeSeconds=0,jam.ScriptHuman=1,r.RDG.AsyncCompute=0', '-RemoteConsoleIP=127.0.0.1', `-RemoteConsolePort=${PORT}`], { windowsHide: true, stdio: 'ignore' });
console.log(`game pid ${child.pid}`);
let dead = false; child.on('exit', () => { dead = true; });
let up = false; for (let i = 0; i < 300 && !up && !dead; i++) { up = await rc('jam.hard.menufocus', 3000); if (!up) await sleep(2000); }
await sleep(6000);
console.log(`(start) ${await focus()}`);
for (const c of cmds) { await rc(c); await sleep(700); console.log(`${c.padEnd(28)} -> ${await focus()}`); }
const lines = readLog().split(/\r?\n/).filter((l) => /JAMMENU|JAMNET lobby|jam\.lobby/.test(l)).slice(-12).map((l) => l.slice(30, 220));
console.log(lines.join('\n'));
if (!dead) { await rc('quit', 3000); for (let i = 0; i < 20 && !dead; i++) await sleep(500); }
if (!dead) { try { process.kill(child.pid); } catch { /* gone */ } }
