// Fixture generators. Everything here is obviously fake ("test-asset", "fakeuser", example.com).
//  - writeFixtureShots(dir): tiny PNGs, written by code so no image file lives in the repo
//  - writeFixtureTranscripts(dir): fake Claude Code transcripts in the real shape, with planted
//    secrets, an email and a home path that the redaction pass must remove
// Run `node devlog/fixtures/make-fixtures.mjs` to regenerate the checked-in transcripts.
import fs from 'node:fs';
import path from 'node:path';
import zlib from 'node:zlib';
import { fileURLToPath } from 'node:url';

export const FAKE_KEY = 'sk-ant-api03-TESTFIXTURE0000000000000000000000';
export const FAKE_EMAIL = 'test.person@example.com';
export const FAKE_HOME_PATH = 'C:\\Users\\fakeuser\\kiro\\test-project\\Game\\Source\\test-asset.cpp';
export const FAKE_GH_TOKEN = 'ghp_TESTFIXTURE00000000000000000000000000';

function crc32(buf) {
  let c;
  const table = crc32.t || (crc32.t = Array.from({ length: 256 }, (_, n) => {
    c = n;
    for (let k = 0; k < 8; k++) c = c & 1 ? 0xedb88320 ^ (c >>> 1) : c >>> 1;
    return c >>> 0;
  }));
  let crc = 0xffffffff;
  for (const b of buf) crc = table[(crc ^ b) & 0xff] ^ (crc >>> 8);
  return (crc ^ 0xffffffff) >>> 0;
}

function chunk(type, data) {
  const len = Buffer.alloc(4);
  len.writeUInt32BE(data.length);
  const td = Buffer.concat([Buffer.from(type, 'ascii'), data]);
  const crc = Buffer.alloc(4);
  crc.writeUInt32BE(crc32(td));
  return Buffer.concat([len, td, crc]);
}

/** A small valid RGB PNG with a diagonal gradient. */
export function makePng(w = 96, h = 54, tint = [60, 110, 200]) {
  const raw = Buffer.alloc((w * 3 + 1) * h);
  for (let y = 0; y < h; y++) {
    raw[y * (w * 3 + 1)] = 0;
    for (let x = 0; x < w; x++) {
      const t = (x + y) / (w + h);
      const o = y * (w * 3 + 1) + 1 + x * 3;
      raw[o] = Math.round(tint[0] * (1 - t) + 235 * t);
      raw[o + 1] = Math.round(tint[1] * (1 - t) + 225 * t);
      raw[o + 2] = Math.round(tint[2] * (1 - t) + 205 * t);
    }
  }
  const ihdr = Buffer.alloc(13);
  ihdr.writeUInt32BE(w, 0);
  ihdr.writeUInt32BE(h, 4);
  ihdr[8] = 8; // bit depth
  ihdr[9] = 2; // RGB
  return Buffer.concat([
    Buffer.from([0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a]),
    chunk('IHDR', ihdr),
    chunk('IDAT', zlib.deflateSync(raw)),
    chunk('IEND', Buffer.alloc(0)),
  ]);
}

export function writeFixtureShots(dir) {
  fs.mkdirSync(dir, { recursive: true });
  const files = [
    ['20261112-190500-A-test-asset-greybox', [60, 110, 200], 'Greybox of the test-asset arena. Mail test.person@example.com about it.'],
    ['20261112-191000-B-look-study-test-asset', [200, 110, 60], null],
    ['20261113-063000-A-second-package-test', [90, 140, 90], null],
  ];
  const written = [];
  for (const [name, tint, cap] of files) {
    const p = path.join(dir, `${name}.png`);
    fs.writeFileSync(p, makePng(96, 54, tint));
    if (cap) fs.writeFileSync(path.join(dir, `${name}.txt`), cap + '\n');
    written.push(p);
  }
  // a file with a bad name, which must be skipped with a warning
  fs.writeFileSync(path.join(dir, 'not-a-screenshot-name.png'), makePng(8, 8));
  return written;
}

// ---------- transcripts ----------
const H0 = Date.parse('2026-11-12T06:00:00+01:00');
const at = (h) => new Date(H0 + h * 3600000).toISOString();

function turn(sessionId, msgId, h, usage, opts = {}) {
  // one API response is written as several records sharing message.id
  const base = (content, stop) => ({
    parentUuid: null,
    isSidechain: !!opts.sidechain,
    type: 'assistant',
    message: { model: opts.model || 'claude-test-model', id: msgId, type: 'message', role: 'assistant', content, stop_reason: stop, usage },
    sessionId,
    timestamp: at(h),
    cwd: FAKE_HOME_PATH.replace(/\\[^\\]+$/, ''),
  });
  const recs = [base([{ type: 'text', text: 'working on test-asset' }], null)];
  if (!opts.single) recs.push(base([{ type: 'tool_use', id: 'toolu_test', name: 'Bash', input: { command: 'echo test' } }], 'tool_use'));
  return recs;
}

export function transcriptLines() {
  const L = [];
  const sid = 'session-test-0001';
  L.push({ type: 'mode', mode: 'normal', sessionId: sid });
  L.push({ type: 'ai-title', aiTitle: `Fix the cartridge loader for ${FAKE_EMAIL} (key ${FAKE_KEY}) in ${FAKE_HOME_PATH}`, sessionId: sid });
  L.push({ type: 'user', isMeta: true, message: { role: 'user', content: '<local-command-caveat>fake</local-command-caveat>' }, sessionId: sid, timestamp: at(0.9) });
  L.push({ type: 'user', message: { role: 'user', content: 'Please set up the test-asset arena' }, sessionId: sid, timestamp: at(1.0) });
  const mk = (n, i, o, cr, cw) => ({ input_tokens: i, output_tokens: o, cache_read_input_tokens: cr, cache_creation_input_tokens: cw });
  const hours = [1.1, 1.3, 1.6, 2.2, 2.4, 6.1, 6.2, 6.5, 13.0, 13.2, 13.4, 13.6, 14.2, 25.0];
  hours.forEach((h, i) => L.push(...turn(sid, `msg_test_${i}`, h, mk(i, 3, 400 + i * 10, 20000 + i * 100, 1500))));
  L.push({ type: 'user', message: { role: 'user', content: 'Now the second package test' }, sessionId: sid, timestamp: at(13.1) });
  L.push({ type: 'user', message: { role: 'user', content: [{ type: 'tool_result', tool_use_id: 'toolu_test', content: 'ok' }] }, sessionId: sid, timestamp: at(13.15) });
  // robustness: assistant record with no usage, none timestamp, and a broken line
  L.push({ type: 'assistant', message: { id: 'msg_test_nousage', role: 'assistant', content: [] }, sessionId: sid, timestamp: at(14.5) });
  L.push({ type: 'assistant', message: { id: 'msg_test_nots', role: 'assistant', content: [], usage: mk(0, 1, 1, 0, 0) }, sessionId: sid });
  L.push({ type: 'last-prompt', lastPrompt: `token=${FAKE_GH_TOKEN}`, sessionId: sid });
  return L;
}

export function subagentLines() {
  const sid = 'session-test-0001';
  const L = [];
  [13.3, 13.5, 13.9].forEach((h, i) => L.push(...turn(sid, `msg_sub_${i}`, h, { input_tokens: 2, output_tokens: 150, cache_read_input_tokens: 9000, cache_creation_input_tokens: 700 }, { sidechain: true, single: true })));
  return L;
}

export function deviceBLines() {
  const sid = 'session-test-0002';
  const L = [{ type: 'ai-title', aiTitle: 'Look study for the test-asset set', sessionId: sid }];
  [6.6, 6.9, 7.2, 13.1, 13.3, 26.0, 26.4].forEach((h, i) => L.push(...turn(sid, `msg_b_${i}`, h, { input_tokens: 4, output_tokens: 800, cache_read_input_tokens: 30000, cache_creation_input_tokens: 2000 })));
  return L;
}

export function writeFixtureTranscripts(dir) {
  const w = (rel, lines, extra = '') => {
    const p = path.join(dir, rel);
    fs.mkdirSync(path.dirname(p), { recursive: true });
    fs.writeFileSync(p, lines.map((l) => JSON.stringify(l)).join('\n') + '\n' + extra);
  };
  w('A/session-test-0001.jsonl', transcriptLines(), 'this line is not json\n');
  w('A/session-test-0001/subagents/agent-testsub1.jsonl', subagentLines());
  w('B/session-test-0002.jsonl', deviceBLines());
}

if (process.argv[1] && path.resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  writeFixtureTranscripts(path.join(path.dirname(fileURLToPath(import.meta.url)), 'transcripts'));
  console.log('fixture transcripts written');
}
