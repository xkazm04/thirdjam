# Lane A230, track L2 (the look), Sunday evening: the state of the neon arena, and more of the existing modes played on it (device A, Fable: judgement by looking)

You are lane A230 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md`, then `briefs/NIGHT-COMMON.md`: their rules are yours in full.**
Then R-0064 (his order of one neon mode, "Tron like": a mood, no mark copied), **R-0069 to R-0071** at the end of `contracts/RULINGS.md`,
`docs/experiments/LOOKS.md` and `status/A223.md` (today's look lane: the neon arena's boxes, the fog, its film tools `tools/qa/eye3/`),
`Game/Source/JamGame/Private/NeonArena.cpp` / `LookTierSwitcher.cpp` (how the tier `l3_neon` is built: A's grid arena over device B's rig
`LR_L3_Neon`), `contracts/look-tiers.md`, the twelve mode cartridges (`Game/Content/Cartridges/modes/*.cartridge.json`: `look.tier`,
the rink's size), device B's `status/B.md` H59 to H60.5 (the neon rig's newest parts: the goal's burst of light, the lilac outline; crowd
actors exist only for rink lengths 56, 60, 64, 70, 80, 90, 120 and 140 m).

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A230.md`. Two heavy
processes at most; ONE film at a time. Off screen only. Nothing of yours runs from the main checkout.

## His words (15 Nov 17:16, verbatim)
"Check state of Neon/Tron arenas to combine in some of the existing modes"

## What is true now
Of the twelve modes only Glow Ice (`hockey.mode.glass-house`, 90 m) plays on `l3_neon`; eight play on clay (`p3_clay`: Arsenal 90 m,
Bruisers 56, Cage 60, Marathon 140, Power Play 60, Shotgun 56, Sniper 90, Sudden Death 120), Fog of War on night ice, Pond and Outnumbered
in daylight. The neon arena is the look he asked for by name and the trailer's finale, and it shows in one mode.

## Do
1. **The state, by looking (the first 25 minutes):** the neon arena from the game camera at 1920 x 1080 and 1280 x 720 at every rink size
   a clay mode uses (56, 60, 90, 120, 140 m), with that mode's own rules and bodies (ten in a 56 m box; three a side on 140 m): the grid,
   the boards' lines, the nets, the boxes, the stands or their absence, the puck and its trail, the rings, the pickups and boosts, the
   goal's burst, the HUD and the controls panel on the dark sheet. One `SEEN` line a size: what holds, what breaks (a floor that ends,
   boards that do not meet, a bright daylight mesh left in the dark, an unreadable puck, a ring lost on the grid).
2. **Choose two or three of the eight clay modes for the neon arena**, as the look's director: where the mode's idea and the arena fit
   (the Director's first thoughts, yours to overrule by what you see: **Arsenal**, boosts and tricks, is an arcade idea and stands on the
   90 m sheet Glow Ice already proves; **Marathon**, three a side on endless ice, wants a grid to the horizon if the arena holds at
   140 m; **Cage**, a tight box, could be a cage of light). Keep clay where the timber hall is the point (Shotgun, Bruisers) and keep PLAY
   NOW on clay. Each mode still reads as itself: if two neon modes look the same in a still, give each its own accent within the tier's
   keys (line colours, the grid's density) and say which.
3. **Make it so:** the chosen cartridges' `look.tier` (and any look key the tier needs) changed; the mode's facts line on MODES if it
   names a look; nothing of the sim changes (the same rules, sizes, bots: prove the sim's hash equal per changed mode on a 60-second
   scripted match, or say exactly which pinned hash moves and why, and re-pin it). What only device B's rig can cure at a size goes under
   `FOR B` with a still; a mode whose size does not hold is not moved.
4. Tell lane A231 (the modes' illustrations) at once which modes you move: a line `NEON MODES: <ids>` in your status file as soon as you
   have decided (by 17:55 at the latest, even if the cure for a size is not finished).

## Prove
Sheets before / after per moved mode at both sizes (`docs/experiments/neon/`): a faceoff, open play, a goal; the unmoved modes unchanged
(one still each); `Jam.Look.*`, `Jam.Rink.*`, `Jam.Menu.*` and the tests that pin a mode's cartridge updated and said; the full `Jam`
filter in a cleared environment; Shipping compiles.

`SEEN` lines as in 1; `NEON MODES:` by 17:55; `NEON READY` with the files by name, the sheets and the gate **by 19:15 by the machine
clock**. Keep the files closed after the line.
