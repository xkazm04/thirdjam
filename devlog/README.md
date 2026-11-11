# devlog/: the build as a timeline

`devlog/build.mjs` turns agent transcripts, git history, screenshots, notes and status files into **one self-contained HTML page**: an hour ruler
H0 to H93 with the gates marked, a lane per device (agent turns per hour as bars, commits per hour as pills), then an hour-by-hour journal
with notes, screenshots, status reports and commits, a summary strip, and the list of agent sessions. Inline CSS, no web fonts, no external
requests, readable at laptop and phone width, light and dark. Zero dependencies (Node 24).

H0 is Thu 2026-11-12 06:00 CET (`devlog/lib/constants.mjs`, the one place that says so). Times are CET.

## Build it

```
node devlog/build.mjs --out devlog/out/index.html \
  --transcripts A=C:/Users/<you>/.claude/projects/<project-slug-of-device-A> \
  --transcripts B=<path to device B's transcripts> \
  --repo <path to the repository> \
  --embed
```

All inputs are optional; with none, you get the empty page (gates only). Flags:

| Flag | Meaning |
|---|---|
| `--out <file>` | the page to write (required) |
| `--transcripts [A=\|B=]<dir>` | Claude Code transcripts (`*.jsonl`, searched recursively, subagents included). Repeatable. Without a label: device A |
| `--repo [A=\|B=]<path>` | a git repository; read-only `git log --branches --remotes`. Repeatable, commits are de-duplicated by hash. Without a label a commit counts for B when it was reached from a branch named `dev-b`, otherwise A |
| `--shots <dir>` | screenshots (default `devlog/shots`) |
| `--status <dir>` | `A.md` and `B.md` (default `status`) |
| `--notes <file>` | notes (default `devlog/notes.md`) |
| `--gates <file>` | gate list (default `docs/GATES.md`, headings of the form `## Gate 1 (H6): ...`) |
| `--title <text>` | page title (default `Third Jam dev log`) |
| `--embed` | put screenshots inside the page as base64, so the file stands alone. Without it, images are relative links and must be published next to the page |
| `--fixtures` | build from the fake data in `devlog/fixtures` (smoke test) |

Exit 0 built, 1 refused (a leak survived redaction), 2 could not run. The last line printed is `PASS wrote ...` with the counts, including what the redaction pass removed.

Transcripts live in `C:/Users/<you>/.claude/projects/<project-slug>/`; each session is a `<session-id>.jsonl`, and the subagents of a session are
in `<session-id>/subagents/agent-*.jsonl`. Use `--transcripts` on the project folder. The page counts one turn per model response, takes tokens from the `usage`
fields, and never prints prompt text. Session titles are shown (redacted).

## Screenshots

File name: `YYYYMMDD-HHMMSS-<A|B>-<label>.png`, where the time is the **CET wall-clock time** the picture was taken and `<label>` uses letters, digits, `-` and `_`.

```
20261112-190500-A-greybox-arena.png
20261113-063000-B-look-study-night.png
```

- To get the name right, use the helper: `node devlog/shot.mjs A greybox-arena path/to/capture.png "Greybox arena, first playable"` copies the file into `devlog/shots/` stamped with the current time and writes the caption.
- A caption comes from the label (`greybox-arena` becomes "Greybox arena") unless a sidecar file with the same name and `.txt` exists.
- Files that do not match the pattern are skipped with a `WARN` line. `.png`, `.jpg`, `.jpeg` and `.webp` are accepted.
- Keep screenshots modest (a 1600 px wide PNG or a JPEG); a page with `--embed` carries them all.

## Notes

`devlog/notes.md` holds one block per entry, `## H12 title`; decimals place a note within an hour (`## H12.5`). Markdown subset: paragraphs, bulleted and
numbered lists, fenced code, `**bold**`, `*italic*`, `` `code` ``, `[links](https://...)`. Raw HTML is shown as text. Quotes are curled and `...` becomes an ellipsis.

## Status

`status/A.md`, `status/B.md`: append `## H<hour> ...` blocks with `- key: value` lines (`status/TEMPLATE.md`). Each block appears in its hour.

## Redaction

Every string that reaches the page is redacted, and the finished page is redacted once more and checked to be stable. Removed: API keys and tokens
(`sk-ant-...`, `sk-...`, GitHub, Slack, AWS, Google, JWT, private key blocks, `Bearer ...`, `KEY=value` assignments with telling names), email addresses, and
absolute paths under any home directory (`C:\Users\<name>`, `/c/Users/<name>`, `/home/<name>`, `/Users/<name>` and Claude project slugs), which become `~`.
The footer of the page reports how many of each were removed. Still read the page once before you publish it: redaction is a net, not a promise.

## Tests

```
node --test "devlog/**/*.test.mjs" "tools/gate/**/*.test.mjs"
```

(`node --test devlog` does not work on Node 24: a bare directory name is treated as a file.) The tests build pages from `devlog/fixtures` (fake transcripts with
planted keys, emails and home paths, fake notes, status, and PNGs written by the test) and from temporary git repositories, and assert the structure of the page
and that nothing planted survives. `node devlog/fixtures/make-fixtures.mjs` regenerates the fixture transcripts.
