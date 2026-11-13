// Lane A83: rebase the Designer's cached cuts onto the five-a-side hockey.regular. game.teams is not Designer-editable (DESIGNER_GAME_EDITABLE),
// so every cut carries hockey.regular's teams verbatim: copy the new teams in and record the new base hash (the hash gen-cut-library.mjs uses).
// Then: node Game/Source/JamDesigner/Tools/gen-designer-data.mjs (re-validates every cut, rewrites JamDesignerData.inl).
import { readFileSync, writeFileSync } from 'node:fs';
import { createHash } from 'node:crypto';
const repo = '../../../..';
const base = JSON.parse(readFileSync(`${repo}/Game/Content/Cartridges/hockey.regular.cartridge.json`, 'utf8'));
const p = `${repo}/Game/Content/Designer/cached-cuts.json`;
const cache = JSON.parse(readFileSync(p, 'utf8'));
for (const c of cache.cuts) c.cartridge.game.teams = structuredClone(base.game.teams);
cache.base_sha256 = createHash('sha256').update(JSON.stringify(base)).digest('hex');
writeFileSync(p, JSON.stringify(cache, null, 2) + '\n');
console.log(`PASS ${cache.cuts.length} cuts rebased onto ${cache.base_sha256.slice(0, 12)}`);
