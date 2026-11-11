#!/usr/bin/env node
// Ownership gate: does this device's branch change only paths it may change?
//
//   node tools/gate/check-ownership.mjs --device A|B|director [--base main] [--repo <path>] [--ownership <file>]
//
// Changed paths = `git diff --name-only <base>...HEAD` (what the branch added since it left <base>)
//               + working-tree changes (staged, unstaged, untracked). Renames count as old path and new path.
// Exit 0 every changed path is allowed, 1 at least one is not (listed), 2 could not run.
// A device may change: its own paths, and shared.any_device. The Director may change shared.* only.
// The ownership table is read from <repo>/ownership.json (so a device cannot loosen it unnoticed:
// ownership.json is itself director_only).
import { execFileSync } from 'node:child_process';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const KEYS = {
  top: ['schema', 'note', 'owners', 'shared'],
  owners: ['A', 'B'],
  shared: ['note', 'any_device', 'director_only'],
};

export function globToRegExp(glob) {
  let re = '';
  const g = glob.replace(/\\/g, '/');
  for (let i = 0; i < g.length; i++) {
    const c = g[i];
    if (c === '*') {
      if (g[i + 1] === '*') {
        i++;
        if (g[i + 1] === '/') {
          i++;
          re += '(?:.*/)?'; // "**/" crosses zero or more folders
        } else re += '.*';
      } else re += '[^/]*';
    } else if (c === '?') re += '[^/]';
    else re += c.replace(/[.+^${}()|[\]\\]/g, '\\$&');
  }
  // "dir/**" also matches "dir" itself
  const base = g.endsWith('/**') ? `(?:${re}|${re.slice(0, -2).replace(/\/$/, '')})` : re;
  return new RegExp(`^${base}$`, 'i');
}

export function loadOwnership(file) {
  let data;
  try {
    data = JSON.parse(fs.readFileSync(file, 'utf8'));
  } catch (e) {
    throw new Error(`cannot read ownership table ${path.basename(file)}: ${e.message}`);
  }
  const unknown = (obj, allowed, where) => {
    for (const k of Object.keys(obj)) if (!allowed.includes(k)) throw new Error(`ownership.json: unknown key "${k}" in ${where}`);
  };
  unknown(data, KEYS.top, 'top level');
  if (data.schema !== 'thirdjam.ownership/1') throw new Error('ownership.json: schema must be "thirdjam.ownership/1"');
  if (!data.owners || !data.shared) throw new Error('ownership.json: owners and shared are required');
  unknown(data.owners, KEYS.owners, 'owners');
  unknown(data.shared, KEYS.shared, 'shared');
  const lists = {
    A: data.owners.A,
    B: data.owners.B,
    any_device: data.shared.any_device,
    director_only: data.shared.director_only,
  };
  for (const [k, v] of Object.entries(lists)) {
    if (!Array.isArray(v) || !v.every((s) => typeof s === 'string' && s.length)) throw new Error(`ownership.json: ${k} must be a list of patterns`);
  }
  return Object.fromEntries(Object.entries(lists).map(([k, v]) => [k, v.map((p) => ({ pattern: p, re: globToRegExp(p) }))]));
}

/** Which role(s) own a path? */
export function ownersOf(rel, table) {
  const hit = (list) => table[list].some((x) => x.re.test(rel));
  return ['A', 'B', 'any_device', 'director_only'].filter(hit);
}

/** Decide one path for a device. Returns {ok, owners, reason}. */
export function decide(rel, device, table) {
  const owners = ownersOf(rel, table);
  const dev = owners.filter((o) => o === 'A' || o === 'B');
  if (dev.length > 1) return { ok: false, owners, reason: 'ownership table is ambiguous: claimed by both devices', broken: true };
  if (!owners.length) return { ok: false, owners, reason: 'owned by nobody (not in ownership.json)' };
  if (device === 'director') {
    return owners.includes('A') || owners.includes('B')
      ? { ok: false, owners, reason: `owned by device ${dev[0]}` }
      : { ok: true, owners };
  }
  if (owners.includes(device) || owners.includes('any_device')) return { ok: true, owners };
  if (dev.length) return { ok: false, owners, reason: `owned by device ${dev[0]}; file a handoff request` };
  return { ok: false, owners, reason: 'director only; file a handoff request' };
}

function gitEnv() {
  const env = { ...process.env, GIT_OPTIONAL_LOCKS: '0', GIT_TERMINAL_PROMPT: '0' };
  for (const k of ['GIT_DIR', 'GIT_WORK_TREE', 'GIT_INDEX_FILE']) delete env[k];
  return env;
}

function git(repo, args) {
  return execFileSync('git', ['-C', repo, '--no-pager', '-c', 'core.quotepath=false', ...args], {
    env: gitEnv(),
    encoding: 'utf8',
    maxBuffer: 256 * 1024 * 1024,
    stdio: ['ignore', 'pipe', 'pipe'],
  });
}

const splitZ = (s) => s.split('\0').filter(Boolean);

/** Changed paths: branch since base, plus working tree. Returns {paths:Set, branchPaths, treePaths}. */
export function changedPaths(repo, base) {
  const branch = new Set(splitZ(git(repo, ['diff', '--name-only', '--no-renames', '-z', `${base}...HEAD`])));
  const tree = new Set();
  let hasHead = true;
  try {
    git(repo, ['rev-parse', '--verify', '-q', 'HEAD']);
  } catch {
    hasHead = false;
  }
  if (hasHead) for (const p of splitZ(git(repo, ['diff', '--name-only', '--no-renames', '-z', 'HEAD']))) tree.add(p);
  for (const p of splitZ(git(repo, ['ls-files', '--others', '--exclude-standard', '-z']))) tree.add(p);
  return { paths: new Set([...branch, ...tree]), branch, tree };
}

export function parseArgs(argv) {
  const o = { base: 'main' };
  for (let i = 0; i < argv.length; i++) {
    const a = argv[i];
    const take = () => {
      const v = argv[++i];
      if (v === undefined || v.startsWith('--')) throw new Error(`${a} needs a value`);
      return v;
    };
    if (a === '--device') o.device = take();
    else if (a === '--base') o.base = take();
    else if (a === '--repo') o.repo = take();
    else if (a === '--ownership') o.ownership = take();
    else if (a === '--json') o.json = true;
    else throw new Error(`unknown argument: ${a}`);
  }
  if (!o.device || !['A', 'B', 'director'].includes(o.device)) throw new Error('--device A, B or director is required');
  return o;
}

export function run(argv, out = console) {
  let o;
  try {
    o = parseArgs(argv);
  } catch (e) {
    out.error(`FAIL ${e.message}\nusage: node tools/gate/check-ownership.mjs --device A|B|director [--base main] [--repo <path>] [--ownership <file>]`);
    return 2;
  }
  let repo;
  try {
    repo = path.resolve(o.repo || git(process.cwd(), ['rev-parse', '--show-toplevel']).trim());
    repo = git(repo, ['rev-parse', '--show-toplevel']).trim();
  } catch (e) {
    out.error(`FAIL not inside a git repository (${String(e.stderr || e.message).trim().split('\n')[0]})`);
    return 2;
  }
  let table;
  try {
    table = loadOwnership(o.ownership ? path.resolve(o.ownership) : path.join(repo, 'ownership.json'));
  } catch (e) {
    out.error(`FAIL ${e.message}`);
    return 2;
  }
  try {
    git(repo, ['rev-parse', '--verify', '-q', `${o.base}^{commit}`]);
  } catch {
    out.error(`FAIL base ref "${o.base}" does not exist in this repository (use --base <ref>)`);
    return 2;
  }
  let changed;
  try {
    changed = changedPaths(repo, o.base);
  } catch (e) {
    out.error(`FAIL could not read changes: ${String(e.stderr || e.message).trim().split('\n')[0]}`);
    return 2;
  }
  const bad = [];
  let broken = false;
  for (const p of [...changed.paths].sort()) {
    const d = decide(p, o.device, table);
    if (d.broken) broken = true;
    if (!d.ok) bad.push({ path: p, reason: d.reason });
  }
  const total = changed.paths.size;
  if (broken) {
    for (const b of bad) out.error(`FAIL ${b.path}: ${b.reason}`);
    out.log(JSON.stringify({ schema: 'thirdjam.gate/1', step: 'ownership', device: o.device, ok: false, changed: total, violations: bad }));
    return 2;
  }
  for (const b of bad) out.log(`FAIL ${b.path}: ${b.reason}`);
  if (bad.length) out.log(`FAIL ownership: ${bad.length} of ${total} changed path(s) are not device ${o.device}'s to change`);
  else out.log(`PASS ownership: ${total} changed path(s), all allowed for device ${o.device} (base ${o.base})`);
  out.log(JSON.stringify({ schema: 'thirdjam.gate/1', step: 'ownership', device: o.device, base: o.base, ok: !bad.length, changed: total, violations: bad }));
  return bad.length ? 1 : 0;
}

if (process.argv[1] && path.resolve(process.argv[1]) === fileURLToPath(import.meta.url)) process.exitCode = run(process.argv.slice(2));
