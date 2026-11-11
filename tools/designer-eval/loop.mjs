// The Designer attempt chain, shared by the mock and the live runner and, later, the model of what JamDesigner does in C++.
//
//   attempt 1  haiku, fresh request
//   attempt 2  haiku, repair request (validator errors + previous reply)   only if attempt 1 was invalid
//   attempt 3  sonnet, repair request (errors of attempt 2 + its reply)    only if attempt 2 was invalid
//
// A valid refusal ends the chain at any attempt. Outcomes: first_try | repaired | refused | failed.
//
// callModel(body) must resolve to { text, stop_reason?, usage?: {input_tokens, output_tokens}, ms? }.
// It may throw an error with `fatal = true` (bad key, malformed request): the chain stops, nothing more is spent.

import { assembleRequest, MODELS } from './assemble.mjs';
import { parseDesignerReply } from '../../schema/validate.mjs';

export async function runDesigner({ cartridge, sentence, callModel, limits, models = MODELS }) {
  const steps = [models.first, models.repair, models.retry];
  const attempts = [];
  let tokensIn = 0;
  let tokensOut = 0;
  let ms = 0;
  let previous;

  for (let i = 0; i < steps.length; i += 1) {
    const model = steps[i];
    const body = assembleRequest({ cartridge, sentence, model, previous, limits });
    let reply;
    try {
      reply = await callModel(body);
    } catch (e) {
      attempts.push({ model, kind: 'error', errors: [`the model call failed: ${e.message}`] });
      if (e.fatal) return finish('failed');
      previous = { output: '(no reply)', errors: ['at /: the model call produced no reply. Reply with the JSON object.'] };
      continue;
    }
    tokensIn += reply.usage?.input_tokens ?? 0;
    tokensOut += reply.usage?.output_tokens ?? 0;
    ms += reply.ms ?? 0;

    if (reply.stop_reason === 'refusal') {
      // The platform's own safety classifier declined. There is no fallback model for it; treat it as a refusal.
      attempts.push({ model, kind: 'refusal', platform: true });
      return finish('refused', { refusal: 'The request was declined.' });
    }

    const parsed = parseDesignerReply(reply.text, cartridge, { limits });
    attempts.push({ model, kind: parsed.kind, errors: parsed.errors });
    if (parsed.kind === 'cartridge') return finish(i === 0 ? 'first_try' : 'repaired', { cartridge: parsed.cartridge });
    if (parsed.kind === 'refusal') return finish('refused', { refusal: parsed.refusal });
    previous = { output: reply.text ?? '', errors: parsed.errors };
  }
  return finish('failed');

  function finish(outcome, extra = {}) {
    return { outcome, attempts, tokens_in: tokensIn, tokens_out: tokensOut, ms, ...extra };
  }
}
