import { test } from 'node:test';
import assert from 'node:assert/strict';
import os from 'node:os';
import { createRedactor, redactHtml } from './lib/redact.mjs';

const r = () => createRedactor();

test('anthropic style keys are removed', () => {
  const x = r();
  const out = x.redact('key sk-ant-api03-ABCDEFGHIJKLMNOP-1234_abcd end');
  assert.ok(!out.includes('sk-ant'), out);
  assert.match(out, /\[redacted-key\]/);
  assert.equal(x.stats.key, 1);
});

test('other common secret shapes are removed', () => {
  const samples = [
    'sk-proj-abcdefghijklmnopqrstuvwx',
    'sk-abcdefghijklmnopqrstuvwxyz012345',
    'ghp_abcdefghijklmnopqrstuvwxyz0123456789',
    'github_pat_11ABCDEFG0abcdefghijklmnopqrstuvwxyz',
    'xoxb-1234567890-abcdefghij',
    'AKIAABCDEFGHIJKLMNOP',
    'AIzaSyA-abcdefghijklmnopqrstuvwxyz01234',
    'eyJhbGciOiJIUzI1NiJ9.eyJzdWIiOiIxMjM0NTY3ODkwIn0.abcdefghijklmnop',
    'Authorization: Bearer abcdef0123456789abcdef.0123',
    'ANTHROPIC_API_KEY=abcdef123456',
    '"api_key": "abcdef123456"',
    'password = hunter2hunter2',
    '-----BEGIN PRIVATE KEY-----\nMIIEvQIBADANBg\n-----END PRIVATE KEY-----',
  ];
  for (const s of samples) {
    const out = r().redact(`before ${s} after`);
    for (const secret of ['abcdefghijklmnopqrstuvwx', 'abcdef123456', 'abcdef0123456789', 'hunter2', 'MIIEvQ', 'AKIAABC', 'eyJhbGci', 'xoxb-1234', 'github_pat_11', 'ghp_abc']) {
      assert.ok(!out.includes(secret), `${s} -> ${out}`);
    }
    assert.match(out, /redacted-key/, s);
  }
});

test('ordinary prose and numbers are left alone', () => {
  const s = 'Gate 1 passed at H6 with 3 commits; token budget is fine, key idea: ship it. sk-learn is not a key.';
  assert.equal(r().redact(s), s);
});

test('email addresses are removed', () => {
  const x = r();
  const out = x.redact('write to Some.Person+tag@Example.co.uk or a@b.io now');
  assert.ok(!out.includes('@'), out);
  assert.equal(x.stats.email, 2);
});

test('home paths become ~ in every spelling', () => {
  const cases = [
    'C:\\Users\\someone\\kiro\\proj\\a.txt',
    'c:/Users/someone/kiro/proj/a.txt',
    'C:\\\\Users\\\\someone\\\\kiro',
    '/c/Users/someone/kiro/proj',
    '/Users/someone/proj',
    '/home/someone/proj',
    'C--Users-someone-kiro-proj',
  ];
  for (const c of cases) {
    const out = r().redact(`see ${c} ok`);
    assert.ok(!/someone/.test(out), `${c} -> ${out}`);
    assert.match(out, /~/);
  }
  assert.equal(r().redact('C:\\Users\\someone\\kiro\\a.txt'), '~\\kiro\\a.txt');
});

test('this machine\'s real home directory never survives', () => {
  const home = os.homedir();
  const x = r();
  for (const s of [home, home.replace(/\\/g, '/'), home + '\\sub\\file.txt', JSON.stringify(home)]) {
    const out = x.redact(`path: ${s}`);
    assert.ok(!out.toLowerCase().includes(home.toLowerCase().replace(/\\/g, '/')) && !out.includes(home), out);
  }
});

test('redaction is idempotent', () => {
  const s = 'a sk-ant-api03-ABCDEFGHIJKLMNOP b x@y.com c C:\\Users\\q\\z d password=abcdefghi Bearer abcdefghijklmnopqrstuvwxyz';
  const once = r().redact(s);
  assert.equal(r().redact(once), once);
});

test('redactHtml leaves embedded images alone but cleans the rest', () => {
  const img = 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==';
  const html = `<img src="${img}"><p>mail a@b.com</p>`;
  const out = redactHtml(html, r());
  assert.ok(out.includes(img));
  assert.ok(!out.includes('a@b.com'));
});

test('non-strings pass through', () => {
  const x = r();
  assert.equal(x.redact(5), 5);
  assert.equal(x.redact(undefined), undefined);
  assert.equal(x.redact(''), '');
});
