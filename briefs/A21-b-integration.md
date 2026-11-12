# Lane A21: device B's content wired into the game (device A, from H9.1)

You are lane A21 of the Third Jam (team Hokej). Device B has delivered looks, effects, sounds and a clay rig that A's code does not yet call. Read first:
`contracts/README.md`, `contracts/events.md` (C-0003), `contracts/look-tiers.md` (C-0002), `contracts/depth.md` (C-0005), `contracts/RULINGS.md` R-0016 to R-0021, and these
handoffs **on the branch `origin/dev-b`** (read them with `git show origin/dev-b:handoff/B-to-A-00NN.md`; that is the only git command you run, read-only):
B-to-A-0012 (Variant parameter for the new events), 0013 (dev log reads B's notes), 0015 (cook list, now fixed on main at 5760168: verify only), 0016 (zone strips, pickup look,
obstacle materials: what A's code assigns), 0018 (tier value `p3_clay`). B's assets are on `origin/dev-b` under `Game/Plugins/JamAssets/Content/`; `main` carries the H8 merge of
dev-b, newer assets come with the next merge (the Director merges at a window; code against the names in the handoffs and guard every load with a log-once fallback).

You own: `Game/Source/JamGame/Private/EffectsBinder.cpp` (+ its header), `Game/Source/JamGame/Private/RinkActor.cpp` zone and obstacle visuals (+ header, visual members only),
`Game/Source/JamGame/Private/LookTierSwitcher.*`, the pickup actor visual (`JamPickupActor.*` in JamCore/Effects or JamGame: visual members only), `schema/` entries for the tier
value, `Game/Config/DefaultGame.ini` cook lines, `devlog/build.mjs` (notes-b), `status/A21.md`. Run no git command but the read-only `git show` above; no editor and no build of
the shared tree outside the Director's windows (:00 and :30; the Director links then and tells you). Compile-verify your units with UBT single-file as other lanes do
(`status/A18.md` shows the command). Node 24.

## Job, in this order
1. **Binder `Variant`** (0012): set Niagara user parameter `User.Variant` and MetaSound input `Variant` per the table (poke clean 0 / miss 1; faceoff drop 0 / won 1 / lost 2;
   foul from_behind 0 / no_puck 1; signal defend 0 / space 1 / pass 2 / praise 3, confirm the order against `JamSignals` on A's side and write the real order in your status);
   `bark`, `signal`, `announce` as 2D sounds; `zone_enter/exit` pass the zone kind's colour as `User.Colour` (colours in 0012); `pass` sends `Strength` = speed / `game.puck.max_speed`;
   `shot_blocked` reuses `NS_foul`/`MS_foul` Variant 0 until B makes one; `period` stays silent. Append the Variant table to `contracts/events.md` under a "Proposed by B, accepted" line
   (the Director signs it in a ruling).
2. **Zones, obstacles, pickups** (0016): one quad per visible zone with a dynamic `MI_Zone_<kind>` (scalars `Circle`, `Strength`), 0.2 cm above the ice, no shadow, no collision,
   stencil 0; obstacles through the boards' `LoadLook` path with `MI_Toon_Bumper` / `MI_Toon_GoalShield`, stencil 1; pickups as `SM_Pickup_Base` + `SM_Pickup_Token` at socket
   `PickupTop` with `MI_Pickup_<kind>` (`custom` for any other id), optional `NS_pickup_idle` with `User.Colour`, `Deactivate()` on take. Every load by path with a log-once fallback to
   the current placeholder so a missing asset never blocks play. `NS_blade_spray`: the toy figure sets `User.Strength` from the lean (0..1).
3. **Tier `p3_clay`** (0018): schema and limits value, `ULookTierSwitcher::GetKnownTiers`, `GetRigMapPath` -> `/JamAssets/Rigs/LR_P3_Clay.LR_P3_Clay`, `+MapsToCook` line,
   `SetTier` unloads the previous rig (B's `BP_LookParams` restores `MPC_Look` on EndPlay), trim `Emissive` per tier: add `look.trim_emissive` with defaults night 30, clay 12,
   day 30, neon 30 (the rink reads it from the switcher); the depth-of-field focus distance is never scaled with the rink.
4. **Dev log** (0013): `devlog/build.mjs` reads every `devlog/notes*.md` (A's `notes.md` first, then `notes-b.md`, each block keeps its device lane); test in `devlog/*.test.mjs`.
5. **Cook verification** (0015): after the Director's next Shipping package, read its cook log for `/JamAssets/VFX`, `/JamAssets/Audio`, `/JamAssets/Skaters`, `/JamAssets/Props`
   and write the counts in your status; the Director tells you the log path.

Report: what each handoff gets as its answer (the Director writes it into the handoff files at the merge), the file list, compile results, and open questions. Status blocks in
`status/A21.md` with real `date` timestamps. Time box 90 minutes; interim report at 45.
