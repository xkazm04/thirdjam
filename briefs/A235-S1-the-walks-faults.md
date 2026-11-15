# Lane A235, track S1 (stabilizing), Sunday night: the walk's faults, cured and shown on the same frames (device A, Opus)

You are lane A235 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md`, then `briefs/NIGHT-COMMON.md`: their rules are yours in full.**
Then **R-0069, R-0070 and R-0073** at the end of `contracts/RULINGS.md` (nothing technical is drawn on or in front of a skater; at a hit
the figure's animation is the whole message; from now only faults seen in the candidate are fixed, each proven on the same
reproduction; nothing is improved), lane A234's walk of the candidate H45 (`tools/qa/a235-in/A234-status.md` and the fault stills in
`tools/qa/a235-in/walk/`: copies the Director put in your worktree), `status/A227.md` (`jam.fx.HitEffects`: which events it already
silences, in `UJamEffectsBinder::HandleEvent`; the edge arrow's clamp), `status/A229.md` (the HUD's one look: `HUDOneLook.cpp`, its
`HideLabelsUnderCards`), `status/A222.md` (the controls panel, the teaching hint).

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A235.md`. Two heavy
processes at most; ONE film at a time. Off screen only. Nothing of yours runs from the main checkout. **The landing is at 21:20 and
the final package at 22:00: `FAULTS READY` by 21:10 by the machine clock, not later.** Do the jobs in this order; each is small: if
one is not, leave it and say so at once.

## A judge will see these (the walk's verdict): cure all five
1. **W13: coloured balls burst at the skates at every body collision that is not a landed check** and float up for half a second; in the
   first two seconds of every match, at the opening faceoff (PLAY NOW rookie f110 to f130). Lane A227 silenced the bursts of a check, a
   cross-check, a knockdown, a foul and a skater's board hit; the collision's (the bump's) burst is the same family and was missed. By
   default no visual effect is spawned at any contact between skaters; the sounds stay. Under the same `jam.fx.HitEffects` (0).
2. **W10: a knocked-down man's slide leaves a streak on the ice** (grey on the four neon modes: Marathon f8040, Glow Ice f471; soot-dark
   on the day look: Outnumbered f270; not seen on clay or night). Find what draws it (an effect spawned for the slide or the fall, a trail
   on the figure, a decal): it is a mark at a hit: off by default on every look, behind a variable; if it is drawn inside device B's
   figure and A cannot stop the call that starts it, say so under `FOR B` at once.
3. **W15: the orange man-down triangle stands in the picture, beside the man just sent flying** (Sniper f849) or alone on the ice (Power
   Play f800), not at the frame's edge. It is the downed man's off-screen arrow (`UJamHitEdgeWidget`, `HitRead.cpp`; lane A227 put a
   clamp above the controls panel into its tick today: check whether that clamp moved it into the picture). The rule: this arrow is not
   drawn at all by default (`jam.hit.OffScreen` or its variable to 0: a mark at a hit, and the camera shows the man anyway); say what
   the variable is.
4. **W6: the yellow teaching hint and the MATCH card share one place** for about 2 seconds at a later faceoff (PLAY NOW rookie f800 to
   f920; also Fog of War; also at 1280 x 720): the card cuts through the hint's letters. While a card is up (the same test your
   `HideLabelsUnderCards` uses) the teaching hint is not drawn; it comes back when the card leaves.
5. **W7: on Glow Ice the faceoff's dial and prompt ("1 HOLD / PRESS SHOOT AT THE DROP") are drawn for him while he sits in the box**
   (f1920, under `IN THE BOX 8`). While the human is boxed the faceoff's dial and prompt are not drawn for him.

## Then, only what is a line or two each (stop at 20:50 and prove what you have)
6. **MAKE YOUR OWN and a CLI on PATH:** the walk found that the game, started from a shell where an assistant's command-line tool is on
   PATH, labels MAKE YOUR OWN "Live" and would send the player's sentence out through that tool. The page and the CREDITS say: a key the
   player types himself, otherwise answers from games made earlier. Read the Designer's provider choice: in a COOKED game (any
   configuration) the command-line provider is never used: only a typed key, else the saved games. Behind a variable for development
   (default: off in cooked builds). A test. Nothing is sent anywhere by you while testing: no key, no call.
7. W2: SETTINGS: a stray lower-case "off" beside `VOICE: OFF`. W4: MODES: the ALL RINKS facts line leaves one word alone on a second
   line (shorten the line). W11: the label "own 24 m" in thin lower case: the HUD's label type and capitals (`OWN  24 M` as the far
   net's label is set). W16: the "F: ... READY" plate is paler than the look's plate and touches the faceoff prompt: the look's plate
   (`OLPlate`) and clear of the prompt. W3: MAKE YOUR OWN: the fifth sentence of the list is cut through: fit or drop the fifth.
   W9: in clock modes the card "PERIOD 2 - FACEOFF" shows while play runs on: the card shows only at a real faceoff.
Not yours: W8 (the camera at the boards: known, not a small change), W12, W14 (device B's dust on the day look), W1, W5.

## Prove
For each of 1 to 5: the walk's own frames again before and after (the same mode, level, seed and frames; the editor binary of your tree
with the variable at its neutral value against the default; say that it is the editor binary), a pair under `docs/experiments/faults/`.
Tests in `Jam.Hit.*`, `Jam.Hud.*`, `Jam.Menu.*`, `Jam.Designer.*` for each cure; the full `Jam` filter in a cleared environment;
Shipping compiles; the sim's hash equal on a 60-second scripted match on clay and on Glow Ice (presentation only).

`SEEN` as each of 1 to 5 is filmed; `FAULTS READY` with the files and functions by name, the pairs and the gate by 21:10.
