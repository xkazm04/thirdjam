# Director rulings on open contract questions (append-only, newest last)

## R-0001 (H5, 2026-11-12 10:50): answers to lane A0's open questions
1. **Skater visual swap key:** `game.skater.visual` enum `toy | humanoid_b1 | generated_b2`, default `toy` (absent = toy). Lane A4 adds it to the schema; the game mode
   picks the visual component class by it; device B's test cartridges set `generated_b2` / `humanoid_b1`.
2. **`SetTeam` takes `FLinearColor`.** The cartridge carries `colour_hex`; the loader converts once. Nobody else parses hex.
3. **Event payload gains `Kind` (FName, `NAME_None` when not applicable)** for `pickup_spawn` / `pickup_take` (the pickup kind) and `knockdown` (the cause: `check` | `boost`).
   Payload is now `{ Who, Where, Strength, Kind }`; lane A1 extends the struct, the bus and the log line (`kind=` appended).
4. **Effects binder lives in `JamGame` (lane A5 owns `UJamEffectsBinder`)**: a component on the game mode that listens to the bus and spawns, by soft object path, the assets
   named by convention in `contracts/events.md` (`/JamAssets/FX/NS_<event>`, `/JamAssets/Audio/MS_<event>`); a missing asset logs once and is skipped. A content-only plugin cannot hold code.
5. **`SetTier` is called by the game mode** after the cartridge loads (`game.look.tier`), and again on a Director's Call that changes the tier.
6. **Extra prefixes accepted:** `SK_ SKEL_ AS_ ABP_ PM_ DA_` join `contracts/assets-naming.md`.
7. **`tools/gate/check-assets-ledger.mjs` exists** (lane B2 built it, 45 tests); A0's note was wrong. It runs at every merge window.
8. **Ownership reconciliation:** the seam stubs under `Game/Plugins/JamAssets/Content/Rigs/` and the four contracts were created by A0 on the Director's behalf; from now on
   `ownership.json` stands: B owns the rig maps, the Director owns `contracts/**`.
9. **Packaging:** Development packages are measured but never shipped; the size gate applies to Shipping only. `package.ps1` keeps failing Development on size; that is noise, not a gate.
