// Builds the Messages API request body for one Designer attempt. Pure: no network, no environment, no clock.
// The body never contains credentials; the live runner adds headers. See prompts/README.md for the design.
//
//   assembleRequest({ cartridge, sentence, model, previous })
//     previous omitted      first attempt: system prompt + <player_sentence> + <current_cartridge>
//     previous = {output, errors[]}   repair attempt: the repair template with the errors and the previous reply
//
// Wire format: text JSON, validated locally (schema/validate.mjs), not a forced tool call. Reasons in prompts/README.md.

import { readFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { DESIGNER_GAME_EDITABLE } from '../../schema/validate.mjs';

const HERE = dirname(fileURLToPath(import.meta.url));
export const ROOT = resolve(HERE, '..', '..');

// Attempt chain: haiku first, one repair on haiku, then one retry on sonnet.
export const MODELS = Object.freeze({
  first: 'claude-haiku-5-5',
  repair: 'claude-haiku-5-5',
  retry: 'claude-sonnet-5-5',
});

// Haiku 5.5 defaults to effort "medium" and Sonnet 5.5 to "high"; the Designer is latency-sensitive, so pin both low.
// Sampling parameters are omitted on purpose: both models reject non-default values.
const EFFORT = { 'claude-haiku-5-5': 'low', 'claude-sonnet-5-5': 'low' };
export const MAX_TOKENS = 8000; // adaptive thinking tokens count against this too

const read = (...p) => readFileSync(resolve(ROOT, ...p), 'utf8');

export function loadPrompts() {
  return {
    system: read('prompts', 'designer.system.md'),
    repair: read('prompts', 'designer.repair.md'),
  };
}

// Replace {{NAME}} markers with a function replacer so "$" in the value is never special.
function fill(template, vars) {
  return template.replace(/\{\{([A-Z_]+)\}\}/g, (m, name) => {
    if (!(name in vars)) throw new Error(`prompt placeholder ${m} has no value`);
    return vars[name];
  });
}

// Keep text from closing our own tags: escape angle brackets.
const escapeText = (s) => s.replace(/</g, '&lt;').replace(/>/g, '&gt;');
const escapeJson = (s) => s.replace(/</g, '\\u003c').replace(/>/g, '\\u003e');

// Lane A44: the game schema excerpt the Designer sees is cut down to the parts of `game` it may change (DESIGNER_GAME_EDITABLE), so the
// system prompt stays under 70 KB as keys are added. A top-level key named whole in the list (pickups, effects, barks, commentary) stays whole;
// one named only through sub-keys (rink.length_m, shot.one_timer, camera.end_height_m) keeps those sub-keys and loses the others, its
// `additionalProperties: false` and the `required` entries that went (the real `game` keeps those keys: the prompt says copy them). A key the
// list never names (teams, puck) goes. The C++ twin is TrimGameSchema in JamDesignerPrompt.cpp; Jam.Designer.PromptParity keeps the two equal.
export function trimGameSchema(schema, paths = DESIGNER_GAME_EDITABLE) {
  const out = JSON.parse(JSON.stringify(schema));
  const props = out.properties ?? {};
  for (const key of Object.keys(props)) {
    const mine = paths.filter((p) => p === key || p.startsWith(`${key}.`));
    if (mine.length === 0) { delete props[key]; continue; }
    if (mine.includes(key)) continue;
    const node = props[key];
    if (!node || typeof node !== 'object' || !node.properties) continue;
    const keep = new Set(mine.map((p) => p.split('.')[1]));
    for (const k of Object.keys(node.properties)) if (!keep.has(k)) delete node.properties[k];
    delete node.additionalProperties;
    if (Array.isArray(node.required)) {
      node.required = node.required.filter((r) => keep.has(r));
      if (node.required.length === 0) delete node.required;
    }
  }
  delete out.additionalProperties;
  if (Array.isArray(out.required)) {
    out.required = out.required.filter((r) => r in props);
    if (out.required.length === 0) delete out.required;
  }
  return out;
}

export function buildSystemText({ limits, envelopeSchema, gameSchema, prompts = loadPrompts() } = {}) {
  const lim = limits ?? JSON.parse(read('schema', 'limits.json'));
  // blocked_terms stay out of the prompt on purpose: they are a program-side net, not instructions to the model.
  const { blocked_terms, ...visible } = lim;
  return fill(prompts.system, {
    LIMITS: JSON.stringify(visible, null, 2),
    ENVELOPE_SCHEMA: JSON.stringify(envelopeSchema ?? JSON.parse(read('schema', 'cartridge.schema.json')), null, 2),
    GAME_SCHEMA: JSON.stringify(trimGameSchema(gameSchema ?? JSON.parse(read('schema', 'game.schema.json'))), null, 2),
  });
}

export function assembleRequest({ cartridge, sentence, model = MODELS.first, previous, limits, prompts = loadPrompts() }) {
  if (typeof sentence !== 'string' || sentence.trim() === '') throw new RangeError('the player sentence is empty');
  const lim = limits ?? JSON.parse(read('schema', 'limits.json'));
  if (sentence.length > lim.max_sentence_chars) {
    throw new RangeError(`the player sentence is ${sentence.length} characters; the limit is ${lim.max_sentence_chars}`);
  }
  const safeSentence = escapeText(sentence.trim());
  const safeCartridge = escapeJson(JSON.stringify(cartridge));

  let userText;
  if (previous) {
    userText = fill(prompts.repair, {
      ERRORS: previous.errors.map((e) => `- ${e}`).join('\n'),
      SENTENCE: safeSentence,
      CARTRIDGE: safeCartridge,
      PREVIOUS: escapeText(String(previous.output)),
    });
  } else {
    userText = `<player_sentence>\n${safeSentence}\n</player_sentence>\n\n<current_cartridge>\n${safeCartridge}\n</current_cartridge>`;
  }

  return {
    model,
    max_tokens: MAX_TOKENS,
    system: [
      // Stable across every request, so it is the cacheable prefix.
      { type: 'text', text: buildSystemText({ limits: lim, prompts }), cache_control: { type: 'ephemeral' } },
    ],
    messages: [{ role: 'user', content: userText }],
    output_config: { effort: EFFORT[model] ?? 'low' },
  };
}
