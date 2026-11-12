// Read-only `git log` against a repository. Nothing here writes to the repo; GIT_DIR and friends
// are removed from the child environment so a hook or a parent shell cannot point git elsewhere.
import { execFileSync } from 'node:child_process';

const F = '\x1f';
const R = '\x1e';

function cleanEnv() {
  const env = { ...process.env, GIT_OPTIONAL_LOCKS: '0', GIT_TERMINAL_PROMPT: '0' };
  for (const k of ['GIT_DIR', 'GIT_WORK_TREE', 'GIT_INDEX_FILE', 'GIT_COMMON_DIR', 'GIT_NAMESPACE', 'GIT_OBJECT_DIRECTORY']) delete env[k];
  return env;
}

/** Heuristic lane for an unlabelled repo: by the branch a commit was reached from. */
export function deviceForRef(ref) {
  if (!ref) return 'A';
  const r = ref.replace(/^refs\/(heads|remotes\/[^/]+)\//, '');
  if (/(^|[\/_-])dev[-_]?b$|^b$|^device[-_]?b/i.test(r)) return 'B';
  return 'A';
}

/**
 * @param {string} repo path
 * @param {string|undefined} device 'A' | 'B' | undefined (infer from branch name)
 * @returns {{hash:string, ms:number, subject:string, device:string, repo:string}[]}
 */
/**
 * Hashes reachable from refs that do not look like device B's. `--source` alone names whichever ref git met first,
 * so a commit that main and dev-b share could be called B; a commit is B only when no other ref reaches it.
 * Returns undefined when there is nothing to partition (then the `--source` name decides).
 */
function hashesOfNonBRefs(repo) {
  try {
    const run = (a) => execFileSync('git', ['-C', repo, '--no-pager', ...a], { env: cleanEnv(), encoding: 'utf8', maxBuffer: 256 * 1024 * 1024, stdio: ['ignore', 'pipe', 'pipe'] });
    const refs = run(['for-each-ref', '--format=%(refname)', 'refs/heads', 'refs/remotes']).split(/\r?\n/).filter((r) => r && !/\/HEAD$/.test(r));
    const a = refs.filter((r) => deviceForRef(r) === 'A');
    if (!a.length || a.length === refs.length) return undefined;
    return new Set(run(['log', '--format=%H', ...a]).split(/\r?\n/).filter(Boolean));
  } catch {
    return undefined;
  }
}

export function readGitLog(repo, device, redact = (s) => s) {
  const fmt = ['%H', '%aI', '%S', '%s'].join(F) + R;
  let out;
  try {
    out = execFileSync(
      'git',
      ['-C', repo, '--no-pager', 'log', '--branches', '--remotes', '--source', '--no-color', `--format=${fmt}`],
      { env: cleanEnv(), encoding: 'utf8', maxBuffer: 256 * 1024 * 1024, stdio: ['ignore', 'pipe', 'pipe'] },
    );
  } catch (e) {
    const msg = (e.stderr || e.message || '').toString().trim().split('\n')[0];
    // an empty repository has no commits: not an error
    if (/does not have any commits yet|bad default revision|unknown revision/i.test(msg)) return [];
    throw new Error(`git log failed for ${redact(repo)}: ${redact(msg)}`);
  }
  const nonB = device ? undefined : hashesOfNonBRefs(repo);
  const commits = [];
  for (const rec of out.split(R)) {
    const r = rec.replace(/^\n+/, '');
    if (!r) continue;
    const [hash, iso, ref, ...rest] = r.split(F);
    const ms = Date.parse(iso);
    if (!hash || Number.isNaN(ms)) continue;
    commits.push({
      hash: hash.slice(0, 7),
      full: hash,
      ms,
      subject: redact(rest.join(F).trim()),
      device: device || (nonB ? (nonB.has(hash) ? 'A' : 'B') : deviceForRef(ref)),
    });
  }
  return commits;
}

export function loadCommits(repos, redact) {
  const seen = new Map();
  for (const { device, dir } of repos) {
    for (const c of readGitLog(dir, device, redact)) if (!seen.has(c.full)) seen.set(c.full, c);
  }
  return [...seen.values()].sort((a, b) => a.ms - b.ms);
}
