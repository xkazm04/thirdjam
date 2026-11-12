// node --test tools/eyes : prompt builder, JSON validation, redaction, model picking, PNG thumbnail. No network: eyes.mjs is never imported.
import { test } from 'node:test';
import assert from 'node:assert/strict';
import { mkdtempSync, writeFileSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import {
  readEnvKey, redact, makeLogger, listPresets, loadPreset, parsePreset, buildPrompt, retryPrompt, parseReply, validateShape,
  pickModels, estimateCost, encodePng, decodePng, downscale, pngThumbnail, renderMarkdown, headerLine, PREAMBLE,
} from './lib.mjs';

const FAKE = 'AIzaSyFAKEFAKEFAKEFAKEFAKEFAKEFAKEFAKE123';

test('readEnvKey reads only the named variable, with or without quotes, and ignores comments', () => {
  const dir = mkdtempSync(join(tmpdir(), 'eyes-test-'));
  const env = join(dir, '.env');
  writeFileSync(env, `# comment\nOTHER=abc\nexport GOOGLE_AI_API_KEY="${FAKE}"\nTHIRD='x'\n`);
  assert.equal(readEnvKey(env), FAKE);
  assert.equal(readEnvKey(env, 'THIRD'), 'x');
  assert.equal(readEnvKey(env, 'MISSING'), undefined);
  assert.equal(readEnvKey(join(dir, 'nope.env')), undefined);
});

test('redact removes the key wherever it appears: plain, in a URL, in a header, in JSON, and key-shaped tokens', () => {
  const s = `GET https://x/models?key=${FAKE} {"x-goog-api-key":"${FAKE}"} plain ${FAKE} other AIzaSyZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ`;
  const r = redact(s, FAKE);
  assert.equal(r.includes(FAKE), false);
  assert.equal(r.includes('AIzaSyZZZ'), false);
  assert.match(r, /key=\[REDACTED\]/);
  assert.match(r, /"x-goog-api-key":"\[REDACTED\]"/);
  assert.equal(redact('no key here', FAKE), 'no key here');
  assert.equal(redact('short', 'abc'), 'short'); // a key under 8 characters is never used as a pattern
});

test('makeLogger passes every line through redact, objects included', () => {
  const lines = [];
  const log = makeLogger(FAKE, (l) => lines.push(l));
  log('err', { body: `key=${FAKE}` }, FAKE);
  assert.equal(lines.length, 1);
  assert.equal(lines[0].includes(FAKE), false);
  assert.match(lines[0], /\[REDACTED\]/);
});

test('every shipped preset parses, has a role, a prompt with the stills placeholder, and a JSON shape', () => {
  const names = listPresets();
  assert.deepEqual(names, ['ab-pair', 'bug-hunt', 'figure', 'hud', 'p2-bar', 'readability']);
  for (const n of names) {
    const p = loadPreset(n);
    assert.ok(['pro', 'flash'].includes(p.role), `${n} role`);
    assert.match(p.prompt, /\{\{stills\}\}/, `${n} stills placeholder`);
    assert.equal(typeof p.shape, 'object', `${n} shape`);
    assert.ok(p.minStills >= 1 && p.maxStills >= p.minStills, `${n} still range`);
  }
  assert.equal(loadPreset('ab-pair').maxStills, 2);
  assert.equal(loadPreset('figure').thumbnail, true);
  assert.throws(() => loadPreset('nope'), /unknown preset/);
});

test('parsePreset rejects a file without a Shape section or with a non-JSON shape', () => {
  assert.throws(() => parsePreset('role: pro\n\n## Prompt\nx {{stills}}\n'), /missing "## Shape"/);
  assert.throws(() => parsePreset('## Prompt\nx\n## Shape\n```json\n{oops}\n```'), /shape is not JSON/);
});

test('buildPrompt fills distance, bar text and still labels, attaches images in order, and ends with the shape', () => {
  const preset = parsePreset('role: pro\nstills: 2\ndistance: true\n\n## Prompt\nAt {{distance}} m. {{bar}}\nStills:\n{{stills}}\n\n## Shape\n```json\n{"verdict":"enum a|b|tie"}\n```');
  const stills = [{ label: 'A', path: 'C:/x/one.jpg', mime: 'image/jpeg', data: 'AAA', width: 10, height: 5 }, { label: 'B', path: 'C:/x/two.png', mime: 'image/png', data: 'BBB' }];
  const bar = { path: 'ref.png', mime: 'image/png', data: 'RRR' };
  const { text, parts } = buildPrompt(preset, { stills, distance: 40, bar, barText: 'THE BAR TEXT', thumbnails: [{ label: 'B', mime: 'image/png', data: 'TTT', width: 128, height: 64 }] });
  assert.ok(text.startsWith(PREAMBLE));
  assert.match(text, /At 40 m\. THE BAR TEXT/);
  assert.match(text, /Still A: one\.jpg\nStill B: two\.png/);
  assert.match(text, /"verdict": "enum a\|b\|tie"/);
  assert.equal(parts[0].text, text);
  assert.match(parts[1].text, /REFERENCE/);
  assert.equal(parts[2].inline_data.data, 'RRR');
  assert.match(parts[3].text, /Still A \(one\.jpg, 10 x 5 px\)/);
  assert.equal(parts[4].inline_data.mime_type, 'image/jpeg');
  assert.match(parts[5].text, /Still B \(two\.png\)/);
  assert.equal(parts[6].inline_data.data, 'BBB');
  assert.match(parts[7].text, /downscaled to 128 x 64/);
  assert.equal(parts[8].inline_data.data, 'TTT');
  assert.equal(parts.length, 9);
  const nobar = buildPrompt(preset, { stills: [stills[0]], distance: 10 });
  assert.equal(nobar.parts.length, 3);
  assert.equal(nobar.text.includes('{{'), false);
});

test('retryPrompt quotes the errors and the previous answer', () => {
  const r = retryPrompt(['$.score: missing', '$.x: unknown key "x"'], '{"x":1}');
  assert.match(r, /\$\.score: missing/);
  assert.match(r, /unknown key "x"/);
  assert.match(r, /\{"x":1\}/);
});

test('parseReply tolerates fences and prose around the object', () => {
  assert.deepEqual(parseReply('```json\n{"a":1}\n```'), { a: 1 });
  assert.deepEqual(parseReply('Here you go: {"a":[1,2]} thanks'), { a: [1, 2] });
  assert.throws(() => parseReply('no json at all'), /no JSON object/);
  assert.throws(() => parseReply('{"a":'), /JSON/);
});

test('validateShape: scalars, ranges, enums, string arrays, optional strings, nested objects, counted arrays, unknown keys', () => {
  const shape = {
    score: 'int 1-5', conf: 'number 0-1', verdict: 'enum a|b|tie', notes: 'string[] 1-3', ok: 'bool', text: 'string?', why: 'string',
    el: { sky: { closer: 'enum a|b|tie', why: 'string' } },
    gaps: [{ rank: 'int 1-3', gap: 'string' }, '2-2'],
  };
  const good = { score: 3, conf: 0.5, verdict: 'a', notes: ['x'], ok: true, why: 'because', el: { sky: { closer: 'b', why: 'y' } }, gaps: [{ rank: 1, gap: 'g' }, { rank: 2, gap: 'h' }] };
  assert.deepEqual(validateShape(good, shape), []);
  assert.deepEqual(validateShape({ ...good, text: '' }, shape), []); // optional string may be empty
  assert.deepEqual(validateShape({ ...good, text: 'ok' }, shape), []);
  const bad = { score: 7, conf: '0.5', verdict: 'c', notes: [], ok: 'yes', why: '', el: { sky: { closer: 'a', why: 'y', extra: 1 } }, gaps: [{ rank: 1.5, gap: 'g' }], stray: 1 };
  const errs = validateShape(bad, shape);
  const has = (re) => assert.ok(errs.some((e) => re.test(e)), `expected ${re} in ${errs.join(' | ')}`);
  has(/\$\.score: must be between 1 and 5/);
  has(/\$\.conf: must be a number/);
  has(/\$\.verdict: must be one of a \| b \| tie/);
  has(/\$\.notes: must have 1 to 3 items/);
  has(/\$\.ok: must be true or false/);
  has(/\$\.why: must be a non-empty string/);
  has(/\$\.el\.sky: unknown key "extra"/);
  has(/\$\.gaps: must have 2 to 2 items/);
  has(/\$\.gaps\[0\]\.rank: must be an integer/);
  has(/\$: unknown key "stray"/);
  assert.ok(validateShape({}, shape).some((e) => /\$\.score: missing/.test(e)));
  assert.deepEqual(validateShape('x', 'any'), []);
  assert.ok(validateShape([], { a: 'string' })[0].includes('must be an object'));
  assert.ok(validateShape({ a: 1 }, { a: 'wat' })[0].includes('preset bug'));
});

test('every shipped shape accepts a minimal well-formed answer and rejects an extra key', () => {
  const samples = {
    'ab-pair': { elements: Object.fromEntries(['sky', 'ice', 'pools', 'figures', 'construction', 'outline'].map((k) => [k, { closer: 'a', why: 'w' }])), verdict: 'a', confidence: 0.7, reason: 'r', loser_keeps: 'l' },
    'p2-bar': { score: 4, gaps: [1, 2, 3].map((r) => ({ rank: r, element: 'sky', gap: 'g', change: 'c' })), works: ['w'], lead_sentence: 's' },
    readability: { stills: [{ label: 'A', elements: Object.fromEntries(['puck', 'carrier', 'team_colours', 'far_net', 'zone_strips'].map((k) => [k, { score: 3, evidence: 'e' }])), first_look: 'f', noise: [], overall: 3 }] },
    figure: { hockey_player: { full: 4, small: 3, parts: { stick: true, blade: true, hands: true, skates: false, helmet: true }, evidence: 'e' }, toy: { full: 4, small: 4, evidence: 'e' }, best_row: 'top', proportions: { head_of_height: 0.3, shoulder_of_height: 0.4, stick_of_height: 0.9 }, changes: ['c'], worst: ['w'], works: ['k'] },
    hud: { stills: [{ label: 'A', elements: [{ name: 'score', position: 'top', legible: true, text: '00' }, { name: 'radar', position: 'br', legible: true }], collisions: [], missing: ['m'], smallest_text_px: 12, legibility: 4 }] },
    'bug-hunt': { stills: [{ label: 'A', findings: [{ where: 'w', what: 'x', severity: 3, confidence: 0.8 }], clean: false }] },
  };
  for (const [name, sample] of Object.entries(samples)) {
    const p = loadPreset(name);
    assert.deepEqual(validateShape(sample, p.shape), [], name);
    assert.ok(validateShape({ ...sample, extra: 1 }, p.shape).some((e) => /unknown key "extra"/.test(e)), `${name} extra key`);
  }
});

test('pickModels prefers the newest pro and flash that generate content, stable before preview, and skips generators, tts, lite', () => {
  const m = (name, methods = ['generateContent']) => ({ name: `models/${name}`, supportedGenerationMethods: methods });
  const picks = pickModels([
    m('gemini-2.5-pro'), m('gemini-2.5-flash'), m('gemini-3.1-pro-preview'), m('gemini-3.1-pro-preview-customtools'), m('gemini-3.8-flash'), m('gemini-3.8-flash-lite'),
    m('gemini-3.8-flash-tts'), m('gemini-3.1-flash-image'), m('gemini-nano-banana-2.1'), m('gemini-embedding-001', ['embedContent']), m('gemini-pro-latest'), m('gemini-3.5-flash-001'),
  ]);
  assert.equal(picks.pro, 'gemini-3.1-pro-preview');
  assert.equal(picks.flash, 'gemini-3.8-flash');
  assert.equal(picks.usable.includes('gemini-3.8-flash-tts'), false);
  assert.equal(picks.usable.includes('gemini-3.1-flash-image'), false);
  assert.equal(picks.usable.includes('gemini-3.8-flash-lite'), false);
  assert.equal(picks.usable.includes('gemini-3.1-pro-preview-customtools'), false);
  assert.deepEqual(pickModels([]), { pro: undefined, flash: undefined, usable: [] });
  // a stable 3.1 pro beats its preview; a 3.0 stable loses to a 3.1 preview (newer version first)
  assert.equal(pickModels([m('gemini-3.1-pro-preview'), m('gemini-3.1-pro')]).pro, 'gemini-3.1-pro');
  assert.equal(pickModels([m('gemini-3-pro'), m('gemini-3.1-pro-preview')]).pro, 'gemini-3.1-pro-preview');
});

test('estimateCost bills thoughts as output and returns undefined for an unpriced model', () => {
  assert.equal(estimateCost('gemini-2.5-flash', { prompt: 1_000_000, output: 100_000, thoughts: 100_000 }), 0.3 + 0.5);
  assert.equal(estimateCost('gemini-3.8-flash', { prompt: 1_000_000, output: 0 }), 0.75);
  assert.equal(estimateCost('gemini-9-unknown', { prompt: 10, output: 10 }), undefined);
});

test('PNG round trip and box downscale to 128 px keep size and average colour', () => {
  const w = 512, h = 256, ch = 3;
  const px = Buffer.alloc(w * h * ch);
  for (let y = 0; y < h; y++) for (let x = 0; x < w; x++) { const o = (y * w + x) * ch; px[o] = x < w / 2 ? 200 : 20; px[o + 1] = 100; px[o + 2] = y < h / 2 ? 0 : 255; }
  const png = encodePng({ width: w, height: h, channels: ch, pixels: px });
  const back = decodePng(png);
  assert.equal(back.width, w); assert.equal(back.height, h); assert.equal(back.channels, 3);
  assert.equal(Buffer.compare(back.pixels, px), 0);
  const t = pngThumbnail(png, 128);
  assert.equal(t.width, 128); assert.equal(t.height, 64);
  const small = decodePng(t.buffer);
  assert.equal(small.pixels[0], 200); // top-left stays red-ish
  assert.equal(small.pixels[(63 * 128 + 127) * 3 + 2], 255); // bottom-right stays blue
  const edge = small.pixels[(0 * 128 + 63) * 3]; // the column on the seam is the mean of the two halves, or one side of it
  assert.ok(edge === 200 || edge === 20 || (edge > 20 && edge < 200));
  const same = downscale({ width: 100, height: 50, channels: 3, pixels: Buffer.alloc(100 * 50 * 3) }, 128);
  assert.equal(same.width, 100);
  assert.throws(() => decodePng(Buffer.from('not a png')), /not a PNG/);
});

test('renderMarkdown and headerLine give a paste-able block that never spreads strings into characters', () => {
  const md = renderMarkdown({ verdict: 'b', confidence: 0.9, elements: { sky: { closer: 'b', why: 'dome' } }, gaps: [{ rank: 1, gap: 'g', change: 'c' }], works: ['x', 'y'], empty: [] });
  assert.equal(md.split('\n').length, 8);
  assert.match(md, /^- verdict: b\n- confidence: 0\.9\n- elements:\n  - sky: closer b, why dome\n- gaps:\n  - rank: 1; gap: g; change: c\n- works: x; y\n- empty: \(none\)$/);
  const h = headerLine({ preset: 'ab-pair', model: 'm', stills: [{ label: 'A', path: 'C:/p/a.jpg' }], distance: 24, usage: { prompt: 10, output: 5, thoughts: 2 }, cost: 0.0123, ms: 1234, retried: true });
  assert.equal(h, '**eyes/ab-pair** A=a.jpg at 24 m (m, in 10 out 5 think 2 tokens, ~$0.0123, 1.2 s, 1 retry)');
  assert.match(headerLine({ preset: 'x', model: 'm', stills: [], usage: { prompt: 0, output: 0 }, ms: 0 }), /cost n\/a/);
});
