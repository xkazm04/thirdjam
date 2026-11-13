import { test } from 'node:test';
import assert from 'node:assert/strict';
import { scopeOf, main } from './drift-gate.mjs';

test('scopeOf: sim and brain run everything, cartridge folders pick their half, the rest nothing', () => {
  assert.equal(scopeOf(['docs/x.md', 'tools/a.mjs', 'Game/Source/JamCore/Tests/T.cpp', 'Game/Source/JamCore/Tools/x.cpp']), 'none');
  assert.equal(scopeOf(['Game/Source/JamCore/Private/JamSim.cpp']), 'all');
  assert.equal(scopeOf(['Game/Source/JamCore/AI/JamBrain.cpp']), 'all');
  assert.equal(scopeOf(['Game/Content/Cartridges/modes/hockey.mode.cage.cartridge.json']), 'modes');
  assert.equal(scopeOf(['Game/Content/Cartridges/ladder/take-3.cartridge.json']), 'takes');
  assert.equal(scopeOf(['Game/Content/Cartridges/ladder/take-3.cartridge.json', 'Game/Content/Cartridges/modes/a.cartridge.json']), 'all');
  assert.equal(scopeOf(['Game/Content/Cartridges/hockey.regular.cartridge.json']), 'all');
  assert.equal(scopeOf(['Game' + String.fromCharCode(92) + 'Source' + String.fromCharCode(92) + 'JamCore' + String.fromCharCode(92) + 'Rules' + String.fromCharCode(92) + 'x.cpp']), 'all');
});
test('main: no relevant path is a skip (exit 0), no uproject is exit 2', async () => {
  const log = console.log; const out = []; console.log = (...x) => out.push(x.join(' '));
  try {
    assert.equal(await main(['--uproject', 'x.uproject', '--paths', 'docs/a.md,tools/b.mjs']), 0);
    assert.match(out.join('\n'), /SKIP drift/);
    assert.equal(await main([]), 2);
  } finally { console.log = log; }
});
