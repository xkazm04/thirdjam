// Seeded-fault proof for check-test-guards.mjs (lane A159): three planted faults are caught, the tree as it is is clean.
import { test } from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { scan, isPositiveGuard } from './check-test-guards.mjs';

const HERE = path.dirname(fileURLToPath(import.meta.url));
const REPO = path.join(HERE, '..', '..');

function tmpTree(files) {
  const root = fs.mkdtempSync(path.join(os.tmpdir(), 'jam-guards-'));
  for (const [rel, text] of Object.entries(files)) {
    const p = path.join(root, 'Game', 'Source', rel);
    fs.mkdirSync(path.dirname(p), { recursive: true });
    fs.writeFileSync(p, text);
  }
  return root;
}
const rules = (r) => r.faults.map((f) => f.rule).sort();

const GOOD_TEST = `#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamA, "Jam.A", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FJamA::RunTest(const FString&) { return true; }
#endif
`;

test('a clean synthetic tree passes', () => {
  const root = tmpTree({ 'JamX/Private/Tests/JamATests.cpp': GOOD_TEST, 'JamX/Private/Code.cpp': 'void Run()\n{\n}\n' });
  try { assert.deepEqual(scan(root).faults, []); } finally { fs.rmSync(root, { recursive: true, force: true }); }
});

test('planted 1: a test defined below the #endif of the guard (the Shipping-only failure of 13 Nov)', () => {
  const bad = `#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamA, "Jam.A", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FJamA::RunTest(const FString&) { return true; }
#endif

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamB, "Jam.B", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FJamB::RunTest(const FString&) { return true; }
`;
  const root = tmpTree({ 'JamX/Private/Tests/JamSoakTests.cpp': bad });
  try {
    const r = scan(root);
    assert.ok(rules(r).includes('unguarded-test'), JSON.stringify(r.faults));
    assert.equal(r.faults.find((f) => f.rule === 'unguarded-test').line, 7);
    assert.ok(rules(r).includes('code-outside-guard'));
  } finally { fs.rmSync(root, { recursive: true, force: true }); }
});

test('planted 2: a file under Tests/ with no guard at all', () => {
  const root = tmpTree({ 'JamX/Private/Tests/JamUnguardedTests.cpp': '#include "CoreMinimal.h"\nstatic int32 Helper() { return 1; }\n' });
  try { assert.ok(rules(scan(root)).includes('no-guard-in-tests')); } finally { fs.rmSync(root, { recursive: true, force: true }); }
});

test('planted 3: non-test code calls a symbol defined only under the guard', () => {
  const root = tmpTree({
    'JamX/Public/JamThing.h': 'class FJamThing\n{\npublic:\n\tvoid Tick();\n#if WITH_DEV_AUTOMATION_TESTS\n\tvoid ForceTestOnlyReset();\n#endif\n};\n',
    'JamX/Private/JamThing.cpp': 'void FJamThing::Tick()\n{\n\tForceTestOnlyReset();\n}\n',
  });
  try {
    const r = scan(root);
    assert.deepEqual(rules(r), ['guard-only-symbol'], JSON.stringify(r.faults));
    assert.equal(r.faults[0].line, 3);
  } finally { fs.rmSync(root, { recursive: true, force: true }); }
});

test('planted 4: a guard that is never closed, and a spec outside the guard', () => {
  const root = tmpTree({
    'JamX/Private/Tests/JamOpen.cpp': '#if WITH_DEV_AUTOMATION_TESTS\nBEGIN_DEFINE_SPEC(FJamSpec, "Jam.Spec", EAutomationTestFlags::ProductFilter)\nEND_DEFINE_SPEC(FJamSpec)\n',
    'JamX/Private/Spec.cpp': 'BEGIN_DEFINE_SPEC(FJamSpec2, "Jam.Spec2", EAutomationTestFlags::ProductFilter)\nEND_DEFINE_SPEC(FJamSpec2)\n',
  });
  try {
    const r = scan(root);
    assert.ok(rules(r).includes('unbalanced'));
    assert.ok(r.faults.some((f) => f.rule === 'unguarded-test' && f.file.endsWith('Spec.cpp')));
  } finally { fs.rmSync(root, { recursive: true, force: true }); }
});

test('guard forms: positive guards count, negated, or-ed and #else branches do not', () => {
  assert.ok(isPositiveGuard('WITH_DEV_AUTOMATION_TESTS'));
  assert.ok(isPositiveGuard('WITH_AUTOMATION_TESTS'));
  assert.ok(isPositiveGuard('defined(WITH_DEV_AUTOMATION_TESTS) && WITH_EDITOR'));
  assert.ok(!isPositiveGuard('!WITH_DEV_AUTOMATION_TESTS'));
  assert.ok(!isPositiveGuard('WITH_EDITOR || WITH_DEV_AUTOMATION_TESTS'));
  assert.ok(!isPositiveGuard('WITH_EDITOR'));
  const root = tmpTree({ 'JamX/Private/Tests/JamElse.cpp': '#if WITH_DEV_AUTOMATION_TESTS\n#else\nIMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamC, "Jam.C", 0)\n#endif\n' });
  try { assert.ok(rules(scan(root)).includes('unguarded-test')); } finally { fs.rmSync(root, { recursive: true, force: true }); }
});

test('the tree as it is: clean', () => {
  const r = scan(REPO);
  assert.ok(r.files > 50, `only ${r.files} files scanned`);
  assert.deepEqual(r.faults, [], r.faults.map((f) => `${f.rule} ${f.file}:${f.line}`).join('\n'));
});

test('a real test file with its last test moved below the #endif is caught', () => {
  const src = path.join(REPO, 'Game', 'Source', 'JamNet', 'Private', 'Tests', 'JamNetGuestTests.cpp');
  const lines = fs.readFileSync(src, 'utf8').split(/\r?\n/);
  const lastImpl = lines.map((l, i) => (/^\s*IMPLEMENT_/.test(l) ? i : -1)).filter((i) => i >= 0).pop();
  const lastEnd = lines.map((l, i) => (/^\s*#endif/.test(l) ? i : -1)).filter((i) => i >= 0).pop();
  assert.ok(lastImpl !== undefined && lastEnd > lastImpl);
  lines.splice(lastEnd, 1);
  lines.splice(lastImpl, 0, '#endif');
  lines.push('#if WITH_DEV_AUTOMATION_TESTS', '#endif');
  const root = tmpTree({ 'JamNet/Private/Tests/JamNetGuestTests.cpp': lines.join('\n') });
  try { assert.ok(rules(scan(root)).includes('unguarded-test')); } finally { fs.rmSync(root, { recursive: true, force: true }); }
});
