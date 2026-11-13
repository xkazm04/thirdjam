# The plan reconciled (Director, Fri 13 Nov 14:20 CET; R-0043)

The operator after the plan and model switch: "please reconcile the plan as previous model may not lead into best possible directions we can achieve". This page is the
Director's honest read of where the last twelve hours went right and wrong, and the order of work to the deadline. It replaces `docs/experiments/NIGHT-PLAN.md` as the
live plan. The jam's rules (digest in `docs/PLAN.md` section 2): a 30 to 60 s video at 1080p or more, a Windows zip under 1 GB named TeamName_GameName, the Glass Box dev
log with the entry, sourced assets noted, no trademarks. Submit Sunday 22:00; the real deadline is Monday 02:59.

## 1. What went wrong (and what it changes)

1. **Six packages shipped with a core fault no gate could see.** H19 to H24 could not shoot or pass while the player aimed (the carried puck sat past the shot reach,
   R-0042). Every gate measures bots and playtester classes that write `FJamInput` directly and never hold the blade where a mouse holds it; every "proof in a package"
   proved a verb's read-out, never a goal by a human hand. **Change:** an acceptance run through the real input path (cursor, buttons) that skates, aims, shoots at three
   charges, scores, passes, receives, checks and is checked, pass or fail per verb: lane A87. No H package from H25 on without it green.
2. **The curve was tuned four times on a player that is not the player.** The playtester classes do not carry the puck on an extended blade, so the win rates describe a
   game the human does not play. **Change:** one last curve pass (A86) after the reach fix and 5 v 5 land, and its first job is to make the classes hold the stick the way
   the mouse does. No other lane edits a difficulty number.
3. **Mechanics were added before the earlier ones met their bars.** The sweep keeps the puck 4 of 10 times, the spin 2 of 10; the saucer, the abilities and twelve modes
   were stacked on top. **Change:** no new mechanic after A84's abilities. A85 re-proves the five bars; a bar that still fails is cut down to what passes, not re-tuned
   past Saturday noon.
4. **Nobody was working on the submission.** The video, the page text, the dev log build, the clean-machine test and the ledger check had no lane; the highlight tool was
   dropped at noon. **Change:** lane A88 starts now and owns the deliverables end to end.
5. **The Director's session is a single point of failure** (stalled 02:00 to 09:28, restarted 13:23, lost its model at 14:05). **Change already made:** lane state on disk,
   chain scripts in files, lanes resumable by a new agent in minutes.

## 2. What went right (keep)
The controls fix (A49), the v4 figures and the tackling chain (A45, A73, A80, B's takes), the soak as the curve's instrument (A53, A70), the modes as cartridges (A67, A72,
A81), the match-start and memory work (A65), the lockstep soak (A56), the operator's reframing (R-0041: aggression, 5 v 5, abilities, a raw brand).

## 3. The order of work

| When | What | Lanes |
|---|---|---|
| Fri 14:20 to 17:00 | The player can shoot and pass while aiming; the acceptance run exists; H25 | A85 (Fable), A87 (Opus) |
| Fri 14:20 to 18:00 | Abilities on F, the pass on Q (off by default; two abilities ship in modes) | A84 (Opus) |
| Fri 14:20 to Sun | The submission: video, page text, dev log, ledger, clean-profile test; refreshed per package | A88 (Sonnet) |
| Fri 17:00 to 20:00 | 5 v 5 lands with the reach fix; one curve pass on the real player; H26 | A83 (done, held), A86 (Opus) |
| Fri evening | Copy and tone in the raw voice (menu, mode pitches, commentator lines for knockdowns), teen-rated | A89 (Sonnet) |
| Sat to 12:00 | **Sim freeze**: after this only fixes the acceptance run or the gate demand | all |
| Sat 12:00 to 22:00 | Look and feel polish with B (logo, banner, crowd, bursts, clay), HUD, the modes' numbers, a joined-play test with B on the final protocol | A + B |
| Sat 22:00 | **Content freeze**; the submission candidate H-final | Director |
| Sun 06:00 to 18:00 | Packaging, clean-machine test, the video, the dev log, the ledger, the submission text (PLAN.md's schedule) | A88 + Director |
| Sun 22:00 | Submit (five hours of buffer) | operator |

## 4. Rules for the lanes from here
- A lane that changes the sim says the hash before and after and runs the full `Jam` filter; a lane that changes a verb the player uses runs A87's acceptance run.
- One lane per file family at a time: the sim (A85, then frozen), the profiles (A86), the input mapping (A84), the HUD (nobody until asked).
- A bar not met is reported with its count; the Director cuts scope, the lane does not tune past a ruling.
- Models: Fable for feel in the sim and the figures' seam, Opus for systems and measurement, Sonnet for harnesses, captures and text.

## 5. Open with the operator
- The eyes tool is blind: the Gemini key's prepaid credits are used up.
- The tie rule at a match's hard stop (last touch wins) stands unless the operator wants a draw.
- B's logo and banner by Saturday midday (A-to-B-0019).
