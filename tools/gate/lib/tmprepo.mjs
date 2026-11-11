// Test helper: a throwaway git repository under the OS temp dir. This is the ONLY place the test
// suites run git with write effects, and only inside the temp directory they created.
// GIT_DIR and the other repository-pointing variables are removed from the environment so a test
// can never touch the real repository, whatever shell started it.
import { execFileSync } from 'node:child_process';
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';

export function cleanGitEnv(extra = {}) {
  const env = { ...process.env, ...extra };
  for (const k of ['GIT_DIR', 'GIT_WORK_TREE', 'GIT_INDEX_FILE', 'GIT_COMMON_DIR', 'GIT_NAMESPACE', 'GIT_OBJECT_DIRECTORY', 'GIT_PREFIX']) delete env[k];
  // the null device is not accepted as a config path by git on Windows; use a real empty file instead
  const empty = path.join(os.tmpdir(), 'jam-empty-gitconfig');
  if (!fs.existsSync(empty)) fs.writeFileSync(empty, '');
  env.GIT_CONFIG_GLOBAL = empty;
  env.GIT_CONFIG_NOSYSTEM = '1';
  env.GIT_TERMINAL_PROMPT = '0';
  return env;
}

export class TmpRepo {
  constructor(prefix = 'jam-test-repo-') {
    this.dir = fs.mkdtempSync(path.join(os.tmpdir(), prefix));
    // refuse to operate anywhere that is not a fresh directory under the temp dir
    if (!path.resolve(this.dir).startsWith(path.resolve(os.tmpdir()))) throw new Error('temp repo outside the temp dir');
    this.git('init', '-q', '-b', 'main');
    this.git('config', 'user.name', 'Test Person');
    this.git('config', 'user.email', 'test.person@example.com');
    this.git('config', 'commit.gpgsign', 'false');
    this.git('config', 'core.autocrlf', 'false');
  }

  git(...args) {
    return execFileSync('git', ['-C', this.dir, ...args], { env: cleanGitEnv(), encoding: 'utf8', stdio: ['ignore', 'pipe', 'pipe'] });
  }

  write(rel, content = 'x\n') {
    const p = path.join(this.dir, rel);
    fs.mkdirSync(path.dirname(p), { recursive: true });
    fs.writeFileSync(p, content);
  }

  /** Commit everything staged by `add`, at a given ISO date. */
  commit(message, iso = '2026-11-12T08:00:00+01:00') {
    this.git('add', '-A');
    execFileSync('git', ['-C', this.dir, 'commit', '-q', '--allow-empty', '-m', message], {
      env: cleanGitEnv({ GIT_AUTHOR_DATE: iso, GIT_COMMITTER_DATE: iso }),
      encoding: 'utf8',
      stdio: ['ignore', 'pipe', 'pipe'],
    });
  }

  cleanup() {
    fs.rmSync(this.dir, { recursive: true, force: true });
  }
}
