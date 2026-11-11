# handoff/: requests across the ownership line

`ownership.json` gives every path one owner (`docs/PLAN.md` section 5). When device A needs something in B's paths, or the other way round,
it does **not** edit. It writes a request here and keeps working on something else until the request is answered.

## Files

- One file per request, named by its id: `A-to-B-0007.md` (from A to B, the seventh request from A to B). Numbers are per direction, four digits, never reused.
- Start from `handoff/TEMPLATE.md`. The id in the file name and the `id:` line must match.
- The requester writes everything above the `## Response` heading. The receiver writes only the response and changes the `status:` line.
- Both devices may write in `handoff/**`; the check `tools/gate/check-ownership.mjs` allows it on both branches.

## Life of a request

1. **Open.** The requester commits the file on its own branch with `status: open` and a `deadline: H<hour>` (hours since H0). One request asks for one thing.
2. **Notice.** Each device reads `handoff/` at the start of every hourly status. A request addressed to you that is still `open` at the next status is a blocker: say so in `status/<device>.md`.
3. **Answer.** The receiver sets `status:` to `accepted` (with the hour it will land), `done` (with the commit hash), or `declined` (with the reason and what to do instead). A decline is a counter-proposal, not a refusal: measurement outranks the brief.
4. **Close.** The requester confirms against the acceptance line and sets `status: closed`. Closed requests stay in the folder: they are dev log material.
5. **Late.** A request past its deadline hour is raised to the Director at the next gate (`docs/GATES.md`, "Always" list).

## What belongs in a request

- **What** is wanted, in one or two sentences, in terms of files or identifiers (not "make it nicer").
- **Why**: what is blocked or degraded without it.
- **Contract names involved**: every identifier on the wire (cartridge keys, enum strings, asset names with their prefix, folders, data asset names). If a contract in `contracts/` does not name them yet, the request says so and proposes the names.
- **Acceptance**: a check the requester can run or look at, for example "`SM_TestAsset` exists under `Game/Plugins/JamAssets/Content/Props/` and has a row in `ASSETS.md`".
- **Deadline hour.**

A request that changes a contract is not a handoff; it is a contract change, which only the Director makes (`contracts/README.md`).

## Dry run (A to B)

1. A copies `handoff/TEMPLATE.md` to `handoff/A-to-B-0001.md`, fills it in, sets `status: open`, commits on `dev-a`.
2. `node tools/gate/check-ownership.mjs --device A` passes: handoff files are writable by both devices.
3. B pulls, sees the request at the next hourly status, appends `## Response` with `status: accepted`, commits on `dev-b`.
4. B delivers the work in B's paths and sets `status: done` with the commit hash.
5. A checks the acceptance line and sets `status: closed`.
