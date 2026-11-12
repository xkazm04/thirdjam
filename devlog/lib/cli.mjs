// Argument parsing for build.mjs. Repeatable flags: --transcripts, --repo. Each may carry a device
// label as "A=<path>" or "B=<path>"; without a label a transcript directory counts as device A and a
// repo infers the device from the branch a commit was reached from (see gitlog.mjs).

const VALUE_FLAGS = new Set(['--out', '--transcripts', '--repo', '--shots', '--status', '--notes', '--gates', '--title', '--since']);
const BOOL_FLAGS = new Set(['--embed', '--fixtures', '--help', '-h']);

export function labelled(v) {
  const m = String(v).match(/^([AaBb])=(.+)$/);
  return m ? { device: m[1].toUpperCase(), dir: m[2] } : { device: undefined, dir: String(v) };
}

export function parseArgs(argv) {
  const o = { transcripts: [], repos: [], embed: false, fixtures: false, help: false };
  for (let i = 0; i < argv.length; i++) {
    const a = argv[i];
    let flag = a;
    let val;
    const eq = a.startsWith('--') ? a.indexOf('=') : -1;
    if (eq > 0) {
      flag = a.slice(0, eq);
      val = a.slice(eq + 1);
    }
    if (BOOL_FLAGS.has(flag)) {
      if (flag === '--embed') o.embed = true;
      else if (flag === '--fixtures') o.fixtures = true;
      else o.help = true;
      continue;
    }
    if (!VALUE_FLAGS.has(flag)) throw new Error(`unknown argument: ${a}`);
    if (val === undefined) {
      val = argv[++i];
      if (val === undefined || val.startsWith('--')) throw new Error(`${flag} needs a value`);
    }
    if (flag === '--transcripts') o.transcripts.push(labelled(val));
    else if (flag === '--repo') o.repos.push(labelled(val));
    else o[flag.slice(2)] = val;
  }
  return o;
}

export const HELP = `usage: node devlog/build.mjs --out <file.html> [options]

  --out <file>             page to write (required)
  --transcripts [A=|B=]<dir>   Claude Code transcripts (JSONL, searched recursively); repeatable
  --repo [A=|B=]<path>     git repository to read commits from (read-only); repeatable
  --shots <dir>            screenshots named YYYYMMDD-HHMMSS-<A|B>-<label>.png (default devlog/shots)
  --status <dir>           status/A.md and status/B.md (default status)
  --notes <file>           notes with "## H12 title" blocks (default devlog/notes.md)
  --gates <file>           gate list (default docs/GATES.md)
  --since <ISO time>       ignore transcript turns and prompts before this instant (e.g. 2026-11-12T05:00:00Z); a project folder may hold older sessions
  --title <text>           page title (default "Third Jam dev log")
  --embed                  put screenshots inside the page as base64 (otherwise relative links)
  --fixtures               build from devlog/fixtures (fake data) for a smoke test
`;
