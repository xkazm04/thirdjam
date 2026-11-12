# Lane A19: review pages for the operator's verdicts (device A, from H8.4)

You are lane A19 of the Third Jam (team Hokej). The operator said: "For any list of verdicts use html page with radio buttons for comfort and quick read or listen and pass or
reject/comment." Ruling R-0020 point 3 in `contracts/RULINGS.md` defines the tool. Read first: that ruling, `prep-reference/HOCKEY-LADDER.html` and `STYLE-PATHS.html` (the house style of
earlier review pages: tokens on `:root`, dark-mode guards, phone width, decision form that builds a paste-back line), `Game/Content/Barks/barks.json` and `REVIEW.md` (the first list to
review: about 1,340 lines grouped by situation, speaker and tone, mild lines tagged), `docs/experiments/FEEL.md` (the proposal table: the second list), `devlog/README.md`.

You own: `tools/review/**`, `docs/reviews/**` (generated pages and the exported decisions). Run no git command; no editor needed. Node 24, zero dependencies. Before writing the page
template, invoke the `artifact-design` skill (Skill tool): the Director publishes each page as a private claude.ai artifact.

## Deliverables
1. **`tools/review/build.mjs --in <items.json> --out <page.html> --title "<2-4 words>"`**: items are `[{ id, group, subgroup?, text, note?, tags?: [] }]`; the page is one self-contained
   HTML file (inline CSS and JS, no external requests, under 3 MB): a sticky header with the title, counts (reviewed / passed / rejected / commented), a group filter and a search box;
   items grouped with collapsible groups; per item: the text (large, readable), tags as chips (e.g. `mild`), radios **pass / reject**, a one-line comment field, and for text lines a
   **listen** button that reads the line aloud with `window.speechSynthesis` (pick an English voice, rate 1.0; a "listen group" button plays a whole group in sequence; stop on click);
   keyboard: J/K move, P pass, R reject, L listen, C focus comment. Decisions persist in `localStorage` (wrapped in try/catch) keyed by the page title. Footer: **export** buttons that
   produce (a) JSON `{ title, decided_at, decisions: [{ id, verdict, comment }] }` copied to the clipboard and shown in a textarea, and (b) a markdown summary (counts, rejected ids,
   comments) for pasting into a chat. Works at phone width; light and dark; the `artifact-design` contract (title 2-4 words, tokens on `:root`, dark-mode guards, body background).
2. **`tools/review/apply-barks.mjs --decisions <json>`**: removes rejected lines from `barks.json`, appends comments to `REVIEW.md`, re-runs `tools/barks/filter.mjs` checks and
   `tools/barks/embed.mjs`, prints what changed. Tests for build and apply (`node --test tools/review`).
3. **First page: the bark bank** `docs/reviews/barks-2026-11-12.html` built from `barks.json` (group = situation, subgroup = speaker + tone, tags = mild), with the register note at the top
   (what the operator asked for, what the filter forbids) so a reviewer knows the bar.
4. **Second page: feel and balance proposals** `docs/reviews/feel-2026-11-12.html` from `docs/experiments/FEEL.md`'s proposal table and A3's sweep (`docs/experiments/BALANCING.md`):
   each proposal is an item with its numbers in `note`.
5. `tools/review/README.md`: how any lane turns a list into a page and how the Director applies decisions.

Report: the two pages' paths and sizes, how the listen button behaves without a voice, the keyboard map, test results. Time box 90 minutes; report at 45 minutes with the bark page.
