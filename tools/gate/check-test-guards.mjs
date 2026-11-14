#!/usr/bin/env node
// Test-guard gate (lane A159, R-0057 point 3): test code must close inside `#if WITH_DEV_AUTOMATION_TESTS`.
//
//   node tools/gate/check-test-guards.mjs [--root <repo>] [--json]
//
// Why: a test defined below the `#endif` of its guard compiles in the Development editor and passes every test; only the
// Shipping game target (no automation) fails, an hour after landing. This reads the C++ text, no Unreal, seconds.
//
// Faults (each line: FAIL <rule> <file>:<line> <text>):
//   unguarded-test     an IMPLEMENT_*_AUTOMATION_TEST, *DEFINE_SPEC or FAutomationTestBase/FAutomationSpecBase/latent command outside the guard
//                      (a guard is `#if WITH_DEV_AUTOMATION_TESTS` or `WITH_AUTOMATION_TESTS`, alone or in a `&&` conjunction; `||`, `!` and
//                      the `#else` of a positive guard do not count)
//   no-guard-in-tests  a file under a `Tests/` folder with no guard at all
//   code-outside-guard in a file under `Tests/`: anything but comments, includes, pragmas and blank lines outside the guard (a helper that
//                      only the tests call breaks the Shipping compile or fills it with unused-symbol warnings)
//   guard-only-symbol  non-test code, outside the guard, names a function / type / constant that is defined only inside a guard
//   unbalanced         `#if` without `#endif`, or `#endif` without `#if`
// Scans Game/Source (*.cpp, *.h, *.inl). Game/Plugins is device B's assets and vendor plugins: not scanned.
// Exit 0 clean, 1 faults, 2 could not run.
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const GUARD_NAMES = ['WITH_DEV_AUTOMATION_TESTS', 'WITH_AUTOMATION_TESTS'];
const TEST_MARKER = /\b(IMPLEMENT_(?:SIMPLE|COMPLEX|CUSTOM_SIMPLE|NETWORKED|CUSTOM_COMPLEX)_AUTOMATION_TEST|IMPLEMENT_[A-Z_]*AUTOMATION_TEST|BEGIN_DEFINE_SPEC|END_DEFINE_SPEC|DEFINE_SPEC|DEFINE_LATENT_AUTOMATION_COMMAND[A-Z_]*|IMPLEMENT_[A-Z_]*LATENT_AUTOMATION_COMMAND|FAutomationTestBase|FAutomationSpecBase)\b/;
const SKIP_VERBS = /^\s*(return|else|new|delete|throw|case|goto|using|typedef|template|friend|if|for|while|switch|do)\b/;
const KEYWORDS = new Set(['if', 'for', 'while', 'switch', 'return', 'sizeof', 'static_cast', 'const_cast', 'reinterpret_cast', 'dynamic_cast', 'defined', 'operator', 'catch', 'alignof', 'decltype', 'TEXT', 'check', 'ensure', 'verify']);

// ---- text preparation ---------------------------------------------------------------------------------------------------------------

/** Replace comments and string/char literals with spaces, keeping line structure. Returns an array of code lines (same count as the file). */
export function stripCode(text) {
  const out = [];
  let inBlock = false;
  for (const raw of text.split(/\r?\n/)) {
    let line = '';
    let i = 0;
    while (i < raw.length) {
      const c = raw[i];
      const n = raw[i + 1];
      if (inBlock) {
        if (c === '*' && n === '/') { inBlock = false; i += 2; line += '  '; } else { i++; line += ' '; }
        continue;
      }
      if (c === '/' && n === '*') { inBlock = true; i += 2; line += '  '; continue; }
      if (c === '/' && n === '/') { line += ' '.repeat(raw.length - i); break; }
      if (c === '"' || c === "'") {
        // raw string literal R"x( ... )x" is rare here; a plain scan to the closing quote is enough for the preprocessor and braces
        const q = c; line += q; i++;
        while (i < raw.length && raw[i] !== q) { if (raw[i] === '\\') { line += ' '; i++; } line += ' '; i++; }
        if (i < raw.length) { line += q; i++; }
        continue;
      }
      line += c; i++;
    }
    out.push(line);
  }
  return out;
}

/** Is this `#if` condition a positive guard (true only when automation tests are compiled in)? */
export function isPositiveGuard(cond) {
  const c = cond.replace(/\s+/g, ' ').trim();
  if (/\|\|/.test(c)) return false;
  const terms = c.split('&&').map((t) => t.trim().replace(/^\(+|\)+$/g, '').trim());
  return terms.some((t) => {
    const m = /^(?:defined\s*\(?\s*)?([A-Z_]+)\s*\)?(?:\s*(?:==\s*1|!=\s*0))?$/.exec(t);
    return m && GUARD_NAMES.includes(m[1]);
  });
}
/** `#if !WITH_DEV_AUTOMATION_TESTS`: its #else branch is guarded. */
function isNegativeGuard(cond) {
  const c = cond.replace(/\s+/g, ' ').trim();
  return /^!\s*\(?\s*(?:defined\s*\(?\s*)?([A-Z_]+)\s*\)?\s*\)?$/.test(c) && GUARD_NAMES.includes(/([A-Z_]{6,})/.exec(c)[1]);
}

/** Per line: { guarded, faultless structure info }. Also returns structure faults. */
export function analyseFile(text) {
  const code = stripCode(text);
  const stack = []; // {kind:'pos'|'neg'|'other', branch:'if'|'else', line}
  const lines = [];
  const faults = [];
  let cont = false;
  for (let i = 0; i < code.length; i++) {
    const l = code[i];
    const guardedNow = stack.some((f) => (f.kind === 'pos' && f.branch === 'if') || (f.kind === 'neg' && f.branch === 'else'));
    const pre = !cont && /^\s*#\s*(\w+)\s*(.*)$/.exec(l);
    const wasCont = cont;
    cont = /\\\s*$/.test(l);
    let guarded = guardedNow;
    if (pre) {
      const [, dir, rest] = pre;
      if (dir === 'if' || dir === 'ifdef' || dir === 'ifndef') {
        let cond = rest;
        if (dir === 'ifdef') cond = `defined(${rest.trim()})`;
        if (dir === 'ifndef') cond = `!defined(${rest.trim()})`;
        const kind = isPositiveGuard(cond) ? 'pos' : isNegativeGuard(cond) ? 'neg' : 'other';
        stack.push({ kind, branch: 'if', line: i + 1 });
        guarded = stack.some((f) => (f.kind === 'pos' && f.branch === 'if') || (f.kind === 'neg' && f.branch === 'else'));
      } else if (dir === 'elif') {
        const top = stack[stack.length - 1];
        if (!top) faults.push({ rule: 'unbalanced', line: i + 1, text: '#elif without #if' });
        else { top.branch = 'else'; if (top.kind === 'pos') top.kind = 'posElif'; }
        guarded = stack.some((f) => (f.kind === 'pos' && f.branch === 'if') || (f.kind === 'neg' && f.branch === 'else'));
      } else if (dir === 'else') {
        const top = stack[stack.length - 1];
        if (!top) faults.push({ rule: 'unbalanced', line: i + 1, text: '#else without #if' });
        else top.branch = 'else';
        guarded = stack.some((f) => (f.kind === 'pos' && f.branch === 'if') || (f.kind === 'neg' && f.branch === 'else'));
      } else if (dir === 'endif') {
        if (!stack.length) faults.push({ rule: 'unbalanced', line: i + 1, text: '#endif without #if' });
        else stack.pop();
        guarded = stack.some((f) => (f.kind === 'pos' && f.branch === 'if') || (f.kind === 'neg' && f.branch === 'else'));
      }
    }
    lines.push({ code: l, guarded, pre: pre ? pre[1] : null, cont: wasCont });
  }
  for (const f of stack) faults.push({ rule: 'unbalanced', line: f.line, text: '#if without #endif' });
  const hasGuard = lines.some((x, idx) => /^\s*#\s*(if|elif)\b/.test(x.code) && isPositiveGuard(x.code.replace(/^\s*#\s*(if|elif)\s*/, '')))
    || lines.some((x) => /^\s*#\s*if(n?def)?\b/.test(x.code) && GUARD_NAMES.some((g) => x.code.includes(g)));
  return { lines, faults, hasGuard };
}

// ---- symbols ------------------------------------------------------------------------------------------------------------------------

/** Names declared at file scope or class scope (never inside a function body) per line. Returns Map<lineIndex, string[]>. */
export function declaredNames(lines) {
  const kinds = []; // brace stack: 'ns' | 'class' | 'fn' | 'enum'
  const result = new Map();
  let header = ''; // text since the last ; { }
  for (let i = 0; i < lines.length; i++) {
    const { code, pre, cont } = lines[i];
    if (pre || cont) continue;
    const inFn = () => kinds.includes('fn');
    // names on this line (only at declaration scope)
    if (!inFn()) {
      const names = [];
      const t = code;
      if (!SKIP_VERBS.test(t)) {
        let m;
        const cls = /\b(?:class|struct|union|enum(?:\s+class)?)\s+(?:[A-Z_]+_API\s+)?([A-Za-z_]\w*)\b(?!\s*;)/g;
        while ((m = cls.exec(t))) names.push({ name: m[1], type: true });
        const ns = /\bnamespace\s+([A-Za-z_]\w*)/.exec(t);
        if (ns) names.push({ name: ns[1], type: true });
        const fn = /^\s*(?:(?:virtual|static|inline|FORCEINLINE|constexpr|explicit|extern)\s+)*(?:[\w:<>,*&]+\s+)+?[*&]*\s*((?:[A-Za-z_]\w*::)*[A-Za-z_~]\w*)\s*\(/.exec(t);
        if (fn) { const nm = fn[1].split('::').pop(); if (!KEYWORDS.has(nm) && !/^[A-Z_]{4,}$/.test(nm)) names.push({ name: nm, type: false }); }
      }
      if (names.length) result.set(i, names.map((n) => ({ ...n, member: kinds.includes('class') })));
    }
    // brace tracking
    for (const ch of code) {
      if (ch === '{') {
        let k = 'fn';
        const h = header + ' ' + code.slice(0, code.indexOf('{'));
        if (/\bnamespace\b/.test(h) && !h.includes('(')) k = 'ns';
        else if (/\bextern\s+"C"/.test(h)) k = 'ns';
        else if (/\benum\b/.test(h) && !h.includes('(')) k = 'enum';
        else if (/\b(class|struct|union)\b/.test(h) && !/\)\s*(const)?\s*$/.test(h.trim()) && !h.includes('(')) k = 'class';
        kinds.push(k); header = '';
      } else if (ch === '}') { kinds.pop(); header = ''; }
      else if (ch === ';') header = '';
    }
    header += ' ' + code;
    if (header.length > 600) header = header.slice(-600);
  }
  return result;
}

// ---- the scan -----------------------------------------------------------------------------------------------------------------------

function walk(dir, out) {
  if (!fs.existsSync(dir)) return;
  for (const e of fs.readdirSync(dir, { withFileTypes: true })) {
    const p = path.join(dir, e.name);
    if (e.isDirectory()) {
      if (/^(Intermediate|Binaries|Saved|Content|ThirdParty)$/i.test(e.name) || e.name === 'JamAssets') continue;
      walk(p, out);
    } else if (/\.(cpp|h|inl)$/i.test(e.name)) out.push(p);
  }
}

export function scan(root) {
  const files = [];
  walk(path.join(root, 'Game', 'Source'), files);
  // Director, 14 Nov: Game/Plugins holds only device B's assets and vendor plugins (DLSS, Streamline: gitignored, present in the main checkout
  // alone, their tests guarded the vendor's own way): not ours to judge, and they failed the gate with four false faults. Game/Source only.
  const faults = [];
  const infos = [];
  const add = (rel, line, rule, text) => faults.push({ rule, file: rel, line, text: String(text).trim().slice(0, 120) });
  const analysed = [];
  for (const f of files) {
    const rel = path.relative(root, f).replace(/\\/g, '/');
    const text = fs.readFileSync(f, 'utf8');
    const a = analyseFile(text);
    const isTests = /(^|\/)Tests\//.test(rel);
    const raw = text.split(/\r?\n/);
    analysed.push({ rel, a, isTests, raw });
    for (const x of a.faults) add(rel, x.line, x.rule, x.text);
    // unguarded-test
    a.lines.forEach((l, i) => {
      if (!l.guarded && !l.pre && TEST_MARKER.test(l.code)) add(rel, i + 1, 'unguarded-test', raw[i]);
    });
    // a Tests/ file
    if (isTests) {
      if (!a.hasGuard) add(rel, 1, 'no-guard-in-tests', 'file under Tests/ carries no WITH_DEV_AUTOMATION_TESTS guard');
      else {
        a.lines.forEach((l, i) => {
          if (l.guarded || l.cont) return;
          if (l.pre) { if (/^(if|ifdef|ifndef|elif|else|endif|include|pragma|define|undef)$/.test(l.pre) && !(l.pre === 'define')) return; if (l.pre === 'define' || l.pre === 'undef') add(rel, i + 1, 'code-outside-guard', raw[i]); return; }
          if (l.code.trim() === '') return;
          add(rel, i + 1, 'code-outside-guard', raw[i]);
        });
      }
    }
  }
  // guard-only symbols
  const guardedDefs = new Map(); // name -> {rel, line, type}
  const openDefs = new Set();
  for (const f of analysed) {
    const names = declaredNames(f.a.lines);
    for (const [i, ns] of names) {
      for (const n of ns) {
        if (f.a.lines[i].guarded) {
          if (n.member && f.isTests) continue; // a member of a test class is the test's own business
          if (!guardedDefs.has(n.name)) guardedDefs.set(n.name, { rel: f.rel, line: i + 1, type: n.type });
        } else openDefs.add(n.name);
      }
    }
  }
  // anything declared anywhere outside a guard (variables, locals, parameters, members) with the same name: the name is not guard-only
  const DECL = /(?:^|[^.>\w])((?:[A-Za-z_][\w:]*)(?:<[^;{}()]*>)?[&*\s]+)([A-Za-z_]\w*)\s*(?=[=;,)\[:{(]|$)/g;
  const NOT_TYPE = /^(return|else|new|delete|throw|case|goto|co_return|co_yield|typename|using|sizeof|do)$/;
  for (const f of analysed) {
    for (const l of f.a.lines) {
      if (l.guarded || l.pre) continue;
      let m;
      DECL.lastIndex = 0;
      while ((m = DECL.exec(l.code))) {
        if (!NOT_TYPE.test(m[1].trim().split(/[\s&*]+/).pop())) openDefs.add(m[2]);
        DECL.lastIndex = m.index + m[0].length - 1;
      }
    }
  }
  const only = [...guardedDefs.keys()].filter((n) => !openDefs.has(n) && n.length >= 5 && !KEYWORDS.has(n));
  if (only.length) {
    const alt = (t) => only.filter((n) => guardedDefs.get(n).type === t).join('|');
    const parts = [];
    if (alt(true)) parts.push(`(?<![.>\\w])(?:${alt(true)})\\b`);
    if (alt(false)) parts.push(`(?<![.>\\w])(?:${alt(false)})\\s*\\(`);
    const re = new RegExp(parts.join('|'));
    for (const f of analysed) {
      if (f.isTests) continue;
      f.a.lines.forEach((l, i) => {
        if (l.guarded || l.pre) return;
        const m = re.exec(l.code);
        if (m) {
          const nm = /\w+/.exec(m[0])[0];
          const d = guardedDefs.get(nm);
          add(f.rel, i + 1, 'guard-only-symbol', `${nm} is defined only inside the test guard (${d.rel}:${d.line}): ${f.raw[i]}`);
        }
      });
    }
  }
  infos.push(`${files.length} files scanned, ${analysed.filter((x) => x.isTests).length} under Tests/, ${guardedDefs.size} guarded names, ${only.length} guard-only`);
  return { faults, infos, files: files.length };
}

function main() {
  const argv = process.argv.slice(2);
  const ri = argv.indexOf('--root');
  const root = ri >= 0 ? path.resolve(argv[ri + 1]) : path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..', '..');
  const json = argv.includes('--json');
  let r;
  try { r = scan(root); } catch (e) { console.log(`FAIL test-guards could not run: ${e.message}`); process.exit(2); }
  for (const f of r.faults) console.log(`FAIL ${f.rule} ${f.file}:${f.line} ${f.text}`);
  for (const i of r.infos) console.log(`INFO ${i}`);
  if (r.files === 0) { console.log('FAIL test-guards found no source under Game/Source'); process.exit(2); }
  console.log(`${r.faults.length ? 'FAIL' : 'PASS'} test-guards ${r.faults.length} fault(s) in ${r.files} files`);
  if (json) console.log(JSON.stringify({ schema: 'jam.gate/1', step: 'test-guards', faults: r.faults.length, files: r.files }));
  process.exit(r.faults.length ? 1 : 0);
}
if (process.argv[1] && path.resolve(process.argv[1]) === fileURLToPath(import.meta.url)) main();
