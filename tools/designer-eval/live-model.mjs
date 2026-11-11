// The live model: one POST to the Messages API per attempt, plain fetch, no SDK, no dependencies.
// The key is read from the environment at call time and used only as a request header. It is never stored,
// logged or included in an error message. This module makes no network call until callModel() is invoked.

const KEY_ENV = 'ANTHROPIC_API_KEY';
const DEFAULT_URL = 'https://api.anthropic.com/v1/messages';
const API_VERSION = '2023-06-01';
const RETRYABLE = new Set([429, 500, 502, 503, 504, 529]);

export class MissingKeyError extends Error {
  constructor() {
    super(`no API key found: set the ${KEY_ENV} environment variable (it is read from the environment only and never printed)`);
    this.name = 'MissingKeyError';
  }
}

export const hasKey = (env = process.env) => typeof env[KEY_ENV] === 'string' && env[KEY_ENV].trim() !== '';

const sleepMs = (ms) => new Promise((r) => setTimeout(r, ms));

export function createLiveModel({ env = process.env, fetchImpl = globalThis.fetch, url = DEFAULT_URL, sleep = sleepMs, retryDelayMs = 1500, timeoutMs = 60000 } = {}) {
  async function once(body) {
    if (!hasKey(env)) throw new MissingKeyError();
    const started = performance.now();
    const res = await fetchImpl(url, {
      method: 'POST',
      headers: {
        'content-type': 'application/json',
        'x-api-key': env[KEY_ENV].trim(),
        'anthropic-version': API_VERSION,
      },
      body: JSON.stringify(body),
      signal: AbortSignal.timeout(timeoutMs),
    });
    const ms = Math.round(performance.now() - started);
    let data;
    try { data = await res.json(); } catch { data = undefined; }
    return { res, data, ms };
  }

  return async function callModel(body) {
    let r;
    try {
      r = await once(body);
      if (!r.res.ok && RETRYABLE.has(r.res.status)) {
        await sleep(retryDelayMs);
        const again = await once(body);
        again.ms += r.ms;
        r = again;
      }
    } catch (e) {
      if (e instanceof MissingKeyError) { e.fatal = true; throw e; }
      throw new Error(`network error: ${e.name}`);
    }
    if (!r.res.ok) {
      const msg = r.data?.error?.message ?? 'no error message';
      const err = new Error(`HTTP ${r.res.status}: ${msg}`);
      err.fatal = r.res.status === 400 || r.res.status === 401 || r.res.status === 403 || r.res.status === 404;
      throw err;
    }
    const content = Array.isArray(r.data?.content) ? r.data.content : [];
    const text = content.filter((b) => b.type === 'text').map((b) => b.text).join('');
    const u = r.data?.usage ?? {};
    return {
      text,
      stop_reason: r.data?.stop_reason,
      usage: {
        input_tokens: (u.input_tokens ?? 0) + (u.cache_creation_input_tokens ?? 0) + (u.cache_read_input_tokens ?? 0),
        output_tokens: u.output_tokens ?? 0,
      },
      ms: r.ms,
    };
  };
}
