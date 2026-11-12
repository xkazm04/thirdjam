# Review pages (lane A19, ruling R-0020 point 3)

Any list that needs a human verdict (bark lines, feel proposals, generated art captions, cut sentences, cut lines) goes to the operator as one self-contained
HTML page: grouped items, pass / reject / comment per item, a Listen button for text, filters, running counts, and an export the operator pastes back.

| File | Does |
|---|---|
| `build.mjs` | `--in items.json --out page.html --title "<2-4 words>"` builds the page. Also `--intro note.txt`, `--description "..."`, `--no-listen`, `--listen-note` |
| `template.html` | the page (inline CSS and JS, no external request, tokens on `:root`, dark-mode guards, phone width) |
| `barks-items.mjs` | bark bank to items plus the register note (group = situation, subgroup = speaker + tone, tag `mild`) |
| `feel-items.mjs` | the FEEL.md proposal table plus A3's sweep numbers to items |
| `apply-barks.mjs` | `--decisions decisions.json` applies the exported decisions to the bark bank |
| `review.test.mjs` | `node --test "tools/review/*.test.mjs"` (Node 24 needs the glob; a bare directory is read as a module) |

## Turn a list into a page (any lane)

1. Write `items.json`: `[{ "id", "group", "subgroup"?, "text", "note"?, "tags"?: [] }]`. `id` is unique and stable (decisions come back keyed by it). Unknown keys are errors.
   Put numbers a reviewer needs in `note` (line breaks kept). `text` is read aloud by Listen; use `--no-listen` for lists that are not prose, `--listen-note` to read the note too.
2. Write an optional `intro.txt` (what the list is, the bar to judge it by): blank-line separated paragraphs, `- ` lines make a list, `**bold**` works, everything else is escaped.
3. `node tools/review/build.mjs --in items.json --out docs/reviews/<topic>-<date>.html --title "Topic Review" --intro intro.txt`. The page must stay under 3 MB; the build refuses otherwise.
4. Hand the page to the Director to publish as a private artifact. Keep the title stable: decisions are remembered in the browser under it.
   For a one-off list, a small script in the lane's own folder that writes `items.json` is enough (see `barks-items.mjs`).

## What the operator sees

Sticky header with the title, counts (reviewed / passed / rejected / commented), a group filter, a status filter, a search box and Listen group. Groups collapse. Each item shows the
text large, its id, tag chips (for example `mild`), an optional note, Listen, Pass / Reject radios (click a checked radio again to clear it) and a one-line comment.
"Pass the rest" in a group marks every undecided item still shown there as passed.

Keyboard: `J` next, `K` previous, `P` pass, `R` reject (both move on to the next item; the same key on an already-decided item clears it), `L` listen (again to stop), `C` focus the comment, `Esc` leave the comment.

Listen uses `window.speechSynthesis`: an English voice (en-US, then en-GB, then any en), rate 1.0. A group plays in sequence, the playing item is highlighted, any click on Stop ends it. With no speech
support the buttons are disabled and a line says so; with no voice installed a line says Listen may stay silent. Decisions persist in `localStorage` under `review:<title>` (all access in try/catch; the page works without it).

Footer: **Copy JSON** (`{ title, decided_at, decisions: [{ id, verdict?, comment? }] }`, also shown in the textarea, selected for Ctrl+C when the clipboard is blocked), **Copy markdown summary**
(counts, rejected ids with their text, comments), and a two-click **Clear all decisions**.

## Director: apply the decisions

The operator pastes the JSON back. Save it to a file, then for the bark bank:

```
node tools/review/apply-barks.mjs --decisions decisions.json --dry-run     # shows what would change, writes nothing
node tools/review/apply-barks.mjs --decisions decisions.json               # applies
node --test tools/barks/*.test.mjs
```

Apply removes rejected lines from `Game/Content/Barks/barks.json` (counts follow), appends a dated section (removed lines, comments) to `Game/Content/Barks/REVIEW.md`, runs the content filter on every
remaining line, checks every situation x speaker x tone still has 12 lines and the mild words stay under the quota, then rebuilds `BarkData.inl` through `tools/barks/embed.mjs`. Exit 1 means the
bank needs attention (the FAIL lines name it; a short combination is topped up with `node tools/barks/generate.mjs --keep --only <situation>`). Exit 2 means it could not run. `--no-embed`, `--bank`, `--review`
point it elsewhere. Comments and passes carry no change to the bank; they are logged for the next pass. Other lists (feel proposals and so on) are applied by hand from the markdown summary.
