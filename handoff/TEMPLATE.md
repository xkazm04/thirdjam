# A-to-B-0000: one-line title of the request

id: A-to-B-0000
from: A
to: B
status: open
opened: H0
deadline: H0

## What

One or two sentences, in terms of files or identifiers. One request asks for one thing.

## Why

What is blocked or degraded without it, and since when.

## Contract names involved

List every wire-level identifier this request touches. If `contracts/` does not name it yet, say "proposed" and give the name.

- contract: `contracts/<file>.md` (section or id)
- cartridge keys: `...`
- enum strings: `...`
- assets (prefix `SM_` `M_` `MI_` `T_` `SFX_` `NS_` `BP_`): `...`
- folders: `...`
- data assets: `...`

## Acceptance

A check the requester can run or look at. Prefer a command, a file that must exist, or a row in `ASSETS.md`.

- [ ] ...

## Response

Written by the receiver only. Set `status:` above to `accepted`, `done`, or `declined`.

- answered: H0
- status: (accepted | done | declined)
- lands at: H0 (accepted) or commit `abc1234` (done)
- notes or counter-proposal: ...
