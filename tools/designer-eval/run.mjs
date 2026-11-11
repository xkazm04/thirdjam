// Designer eval. Runs each sentence through the attempt chain and prints PASS/FAIL lines and one JSON line last.
//
//   node tools/designer-eval/run.mjs                 mock model (default): deterministic, no network, no key
//   node tools/designer-eval/run.mjs --mock
//   node tools/designer-eval/run.mjs --live          real API, only if the key is in the environment, else exit 2
//   options: --sentences file.json  --cartridge file.json  --only <id>  --verbose
//
// Exit codes: 0 every sentence passed, 1 at least one FAIL, 2 could not run (bad arguments, no key for --live).
// Time here is wall-clock only for API latency; tokens and latency are 0 in mock mode.

import { readFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { runDesigner } from './loop.mjs';
import { createMockModel } from './mock-model.mjs';
import { createLiveModel, hasKey, MissingKeyError } from './live-model.mjs';

const HERE = dirname(fileURLToPath(import.meta.url));
const short = (m) => m.replace(/^claude-/, '').replace(/-\d+-\d+$/, '');

export function parseArgs(argv) {
  const o = { mode: 'mock', sentences: resolve(HERE, 'sentences.json'), cartridge: resolve(HERE, '..', '..', 'schema', 'fixtures', 'test-arena.cartridge.json'), verbose: false };
  for (let i = 0; i < argv.length; i += 1) {
    const a = argv[i];
    if (a === '--mock') o.mode = 'mock';
    else if (a === '--live') o.mode = 'live';
    else if (a === '--verbose') o.verbose = true;
    else if (a === '--sentences') o.sentences = resolve(argv[++i] ?? '');
    else if (a === '--cartridge') o.cartridge = resolve(argv[++i] ?? '');
    else if (a === '--only') o.only = argv[++i];
    else throw new Error(`unknown argument ${a}`);
  }
  return o;
}

export async function evaluate({ sentences, cartridge, callModel, limits, log = () => {}, verbose = false }) {
  const totals = { valid_first_try: 0, repaired: 0, failed: 0, refused: 0, tokens_in: 0, tokens_out: 0, ms_total: 0 };
  let anyFail = false;
  for (const s of sentences) {
    const r = await runDesigner({ cartridge, sentence: s.text, callModel, limits });
    totals.tokens_in += r.tokens_in;
    totals.tokens_out += r.tokens_out;
    totals.ms_total += r.ms;
    if (r.outcome === 'first_try') totals.valid_first_try += 1;
    else if (r.outcome === 'repaired') totals.repaired += 1;
    else if (r.outcome === 'refused') totals.refused += 1;
    else totals.failed += 1;

    const gotCartridge = r.outcome === 'first_try' || r.outcome === 'repaired';
    const pass = (s.expect === 'cartridge' && gotCartridge) || (s.expect === 'refusal' && r.outcome === 'refused');
    if (!pass) anyFail = true;
    const via = r.attempts.map((a) => `${short(a.model)}:${a.kind}`).join(' > ');
    log(`${pass ? 'PASS' : 'FAIL'} ${s.id} [${r.outcome}] ${via}${pass ? '' : ` (expected ${s.expect})`}`);
    if (verbose || !pass) {
      for (const a of r.attempts) for (const e of a.errors ?? []) log(`     ${short(a.model)}: ${e}`);
      if (r.refusal) log(`     refusal: ${r.refusal}`);
    }
  }
  return { totals, anyFail };
}

async function main() {
  let opts;
  try {
    opts = parseArgs(process.argv.slice(2));
  } catch (e) {
    console.error(e.message);
    return 2;
  }
  let sentences;
  let cartridge;
  try {
    sentences = JSON.parse(readFileSync(opts.sentences, 'utf8'));
    cartridge = JSON.parse(readFileSync(opts.cartridge, 'utf8'));
  } catch (e) {
    console.error(`cannot read input: ${e.message}`);
    return 2;
  }
  if (opts.only) sentences = sentences.filter((s) => s.id === opts.only);
  if (!sentences.length) { console.error('no sentences to run'); return 2; }

  let callModel;
  if (opts.mode === 'live') {
    if (!hasKey()) { console.error(new MissingKeyError().message); return 2; }
    callModel = createLiveModel();
  } else {
    callModel = createMockModel();
  }

  console.log(`designer-eval mode=${opts.mode}${opts.mode === 'mock' ? ' (mock: tokens and latency are 0)' : ''} sentences=${sentences.length}`);
  const { totals, anyFail } = await evaluate({ sentences, cartridge, callModel, log: (l) => console.log(l), verbose: opts.verbose });
  console.log(JSON.stringify({ tool: 'designer-eval', mode: opts.mode, ...totals }));
  return anyFail ? 1 : 0;
}

if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  main().then((code) => { process.exitCode = code; });
}
