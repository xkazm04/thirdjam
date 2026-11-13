# The evolution tracks (Director; live from Fri 13 Nov 18:50; R-0047)

Ten standing tracks. Each lane does one evolution, lands through the gate (and the acceptance run when a player's verb changes), and proposes the next three. The Director
keeps every track busy until the freeze. Rules: `briefs/EVOLUTION-COMMON.md`. The game's laws: `docs/VISION.md`.

**Freezes (R-0047):** rule changes in the sim land until **Saturday 19:00**, each with a neutral value and a green gate; content freezes **Saturday 22:00**; Sunday is the
submission; submit Sunday 22:00. **Cursor owner now: track S2** (after the Director's H26 release run).

## Fable: mechanics and design, and Blender
| Track | Goal | File family | First evolution |
|---|---|---|---|
| **F0 Design eye** | Play the build as a critic and keep the other tracks' queues honest | `docs/design/**` only; no source | Review H26 against VISION: the ten things a first-time player feels in five minutes, ranked, each with the track that owns it |
| **F1 Contact feel** | A hit that reads and feels right at every strength | the contact functions in `JamSim.cpp`, `JamPawns.cpp` (hit-stop), `CameraFollow.*` (kick), the HUD's contact texts | The stumble and the refused press read: HUD line for "not with the puck", the stumble's camera and hit-stop scaled by the ratio, a board check that uses the boards as the second body |
| **F2 Carrier skill** | Beating a defender by stopping, cutting and the stick feels great | the movement and stick functions in `JamSim.cpp`, drills in `HokejGameMode.cpp`, `HokejAcceptDrill.cpp` | Drill 21 (stop-and-go past a standing pro defender) and the tuning it shows: the burst's window, the brake's grip, puck protection with the body between |
| **F3 Arena in Blender** | The rink dressed for 5 v 5 in the clay voice | `Game/Content/Arena/**` (A's own meshes), `tools/blender/arena/**`, `RinkActor.*` placement only | Benches and penalty boxes with doors, built in Blender to the rink's dimensions, placed by `RinkActor`, using B's clay materials by reference |

## Opus: the codebase and the quality of the implementation
| Track | Goal | File family | First evolution |
|---|---|---|---|
| **Q1 Determinism and lockstep** | Two peers stay bit-identical with everything added today | `JamSim.cpp` hash and state only, `Game/Source/JamNet/**` | Audit every field added since protocol 5 (abilities, contact, stumble, burst, sprint, brake, lift cone) for the hash and the wire; re-run A56's soak on protocol 7 |
| **Q2 Review and repair** | Today's fast landings read by an adversary | the ability rule, `JamAICore.*` (the brain), `CommentarySystem.*`, `MenuFlow.*`, cartridge loader | Review A84, A94, A90, A81 for edge cases (an ability during a knockdown, a hip check under the new contact, the brain's reads with nine skaters down, a rejected mode file) and fix what is real |
| **Q3 Performance at five a side** | No hitch, and headroom, with ten figures and the new brain | `EffectsBinder.*`, `HUDWidget.*` tick, the brain's cost, `DefaultEngine.ini` | Profile H26 at 5 v 5 on both tiers with A58's tools; the brain's space read per frame; fix the top offender |

## Sonnet: tests and hardening
| Track | Goal | File family | First evolution |
|---|---|---|---|
| **S1 Soak** | The whole game, tens of thousands of matches, every cut and mode | `tools/soak/**`, reports | The full soak on the new main (nine cells, all twelve modes, the ladder's takes): crashes, matches that never end, idle stretches, modes whose character number broke under the vision |
| **S2 Acceptance matrix** | The acceptance run on everything a player can pick | `tools/qa/accept/**` drivers and cuts only, reports | The acceptance run on each mode and each look tier; a Shipping smoke and clean-profile test of H26; which rows a mode's rules legitimately change |
| **S3 Fuzz and loader** | No cartridge can crash or hang the game | `tools/fuzz/**`, tests under `Game/Source/JamCore/Private/Tests/JamFuzzTests.cpp` | Thousands of schema-valid random cartridges at the limits through the loader and 600 frames of sim, headless: crashes, non-determinism, never-ending matches |

## Device B: the same loop (handoff A-to-B-0022, 19:20; B owns its board)
| Track | Model | Goal |
|---|---|---|
| **BF1 The figure plays the vision** | Fable + Blender | The stumble by direction and strength, the side takes lowered, a hard-stop pose and spray |
| **BF2 The arena and the clay look** | Fable + Blender | Takes over the arena track from lane A103's report: the crowd band, the scoreboard, the stands |
| **BF3 Hits you hear and see** | Fable | The ability effect and sound, a bump and a stumble sound, the brake's spray, the crowd by strength, 72 commentator lines |
| **BQ1 The quality of B's implementation** | Opus | Blueprints against the hit-stop, every take plays when its event fires, cook warnings, ledger sizes |
| **BS1 Tests only a second machine can run** | Sonnet | The clean-machine test of every package, the video with sound, joined play A to B, B's half of the dev log |

A merges `dev-b` at every half-hour check when it is ahead; B merges main before each lane.

## Queue (the Director appends each lane's "next three" here; the top line of a track is what runs next)
(empty until the first reports)
