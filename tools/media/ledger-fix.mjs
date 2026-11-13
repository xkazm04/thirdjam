// One-off (lane A96): ledger fixes for A's own rows. Idempotent. Run from the repo root: node tools/media/ledger-fix.mjs
import fs from 'node:fs';
const p = 'ASSETS.md';
import { spawnSync } from 'node:child_process';
const out = spawnSync('node', ['tools/gate/check-assets-ledger.mjs'], { encoding: 'utf8' }).stdout;
const sizes = new Map([...out.matchAll(/WARN (Game\/Content\/\S+): size_mb [\d.]+ but the file is ([\d.]+) MB on disk/g)].map((m) => [m[1], m[2]]));
const L = fs.readFileSync(p, 'utf8').split('\n');
let n = { ring: 0, lic: 0, size: 0 };
for (let i = 0; i < L.length; i++) {
  if (!L[i].startsWith('| Game/Content/')) continue;
  const c = L[i].split('|'); // c[0] '' ; c[1] path ... c[4] origin ... c[7] url; c[3] size
  const path = c[1].trim();
  if (path === 'Game/Content/UI/T_UI_RingPlain.uasset' && c[4].trim() === 'generated') {
    c[4] = ' original ';
    c[5] = ' code-drawn: PowerShell System.Drawing script (lane A42), no generative model ';
    c[11] = c[11].replace(/^ /, ' made by a script of ours (a drawn circle), so original, not ai-generated; ');
    n.ring++;
  } else if (/^Game\/Content\/P4\/(Meshy|Rodin|Tripo)\/Anims\//.test(path) && c[4].trim() === 'sourced' && !c[7].trim()) {
    c[5] = ' Epic template Manny clip, retargeted by us ';
    c[6] = ' Unreal Engine EULA (Epic content, used inside a UE product) ';
    c[7] = ' https://www.unrealengine.com/en-US/eula/unreal ';
    n.lic++;
  }
  if (sizes.has(path)) { c[3] = ' ' + sizes.get(path) + ' '; n.size++; }
  L[i] = c.join('|');
}
fs.writeFileSync(p, L.join('\n'));
console.log(n);
