// Scope of the Shipping compile gate (lane A179): which changed paths make the gate build the Shipping game target.
import { test } from 'node:test';
import assert from 'node:assert/strict';
import { needsShipping, main } from './shipping-gate.mjs';

test('code the Shipping target compiles is in scope', () => {
  assert.deepEqual(needsShipping(['Game/Source/JamGame/Private/MenuWidget.cpp', 'Game/Source/JamCore/Public/JamSim.h']).length, 2);
  assert.equal(needsShipping(['Game/Source/JamGame/JamGame.Build.cs']).length, 1);
  assert.equal(needsShipping(['Game/ThirdJam.Target.cs', 'Game/ThirdJam.uproject']).length, 2);
  assert.equal(needsShipping(['Game/Plugins/Foo/Source/Foo/Private/Foo.cpp', 'Game/Plugins/Foo/Foo.uplugin']).length, 2);
});
test('content, docs, tools and status are not', () => {
  assert.equal(needsShipping(['docs/experiments/FUZZ-2.md', 'tools/gate/premerge.ps1', 'status/A179.md', 'Game/Content/Cartridges/hockey.regular.cartridge.json', 'schema/game.schema.json', 'Game/Plugins/JamAssets/Content/x.uasset']).length, 0);
});
test('no scope: skipped without a build, exit 0', async () => {
  const lines = []; const log = console.log; console.log = (s) => lines.push(String(s));
  let code; try { code = await main(['--uproject', 'Game/ThirdJam.uproject', '--paths', 'docs/a.md,tools/b.mjs']); } finally { console.log = log; }
  assert.equal(code, 0);
  assert.ok(lines.some((l) => l.startsWith('SKIP shipping')));
  assert.equal(JSON.parse(lines.at(-1)).verdict, 'skipped');
});
