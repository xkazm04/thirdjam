# tools/cleanup: the reviewed list of unused files (lane A236, Sun 15 Nov 2026)

`remove-list.txt` holds one repository-relative path a line (a file, or a folder ending in `/`), grouped under `# reason` lines with each
group's size. Lines starting with `#` are comments and are never applied; the lines starting with `#?` at the end are **candidates
the lane did not judge unused** (a tool, a doc or a status file names them): they stay until someone removes the `#? ` in front.

How each line was judged: the git index of the tree on main `62e52912` was read for the tracked files and their sizes; every tracked
text file outside `tools/**/out/` and `tools/**/raw/` was searched for the exact path (forward or back slashes, absolute, relative to
the reader's folder), then for the file or folder name alone. A line is in the list only when both searches found nothing, or (the
stills) when the files that matter (`submission/page.md`, `form.md`, `video/*.md`, `devlog/curated/shots.txt`) do not name it.

## Apply (the Director, from the repository root, in Git Bash)

Dry run first (prints what would go, changes nothing):

```bash
grep -v -e '^#' -e '^$' tools/cleanup/remove-list.txt | tr -d '\r' | while IFS= read -r p; do git rm -r -n -q --ignore-unmatch -- "$p"; done
```

Then the removal (tracked paths through `git rm`; the one untracked line, `du.exe.stackdump`, is deleted as a file):

```bash
grep -v -e '^#' -e '^$' tools/cleanup/remove-list.txt | tr -d '\r' | while IFS= read -r p; do git rm -r -q --ignore-unmatch -- "$p"; [ -f "$p" ] && rm -f -- "$p"; done
git status --short | head
```

Then commit as usual. The list removes files from the tree only: the history keeps every byte, so a clone does not get smaller.

## Undo

- Before the commit: `git restore --staged --worktree --source=HEAD -- <path>` for one path, or the same loop with
  `git restore --staged --worktree --source=HEAD -- "$p"` in place of `git rm`.
- After the commit: `git revert <the cleanup commit>`, or for one path `git checkout <the cleanup commit>^ -- <path>`.
- `du.exe.stackdump` was never tracked: a crash dump of a Git Bash tool; nothing of it is needed.

## The .gitignore lines that go with it

`tools/**/out/`, `tools/**/raw/`, `tools/**/*.pid` and `*.stackdump`: lanes' bulk output never travels in a landing
(`briefs/EVOLUTION-COMMON.md`, "Disk is the memory"). Files already tracked there stay tracked; a new sheet a doc must link is added with
`git add -f`. No test reads under these folders.
