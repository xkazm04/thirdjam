# Status A (or B)

Copy this file to `status/A.md` or `status/B.md` (device A writes A.md, device B writes B.md).
Add one block per hour at the end of the file, keep the old blocks: the dev log reads every `## H<hour>` block and shows it in the timeline,
and the Director reads the last block of both files at each gate. The block heading is `## H<hours since H0>` followed by a short title.
Keep the key names exactly as below: the dev log prints them as written.

## H0 Hourly

- done: what is finished and committed since the last block, as short facts (files, identifiers, counts)
- next: the one or two things this device does in the coming hour
- blocked: what waits on something else, and on whom (name the handoff id, for example A-to-B-0007), or "none"
- lfs size: total size of the LFS objects in the repository, for example 1.4 GB (budget: under 8 GB)
- engine build: the exact editor build id in use, for example 5.9.0 CL 60351404 (must be the same on both devices)
- packaged build age: hours since the last successful packaged build, or "none yet" (packages at H6, H24, H48, H72, then every 6 h)
- packaged size: staged and zip size from the package script, for example 410 MB zip (limit: under 1 GB)
- ledger: rows added to `ASSETS.md` this hour, or "none"
