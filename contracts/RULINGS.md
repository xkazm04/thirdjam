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

## R-0002 (H5.5, 2026-11-12 11:20): answers to device B's handoffs B-to-A-0001..0005
1. **B-to-A-0001 accepted.** `tools/run/common.ps1` gains `Set-EngineDotnet` (forces the engine's bundled x64 .NET on an ARM64 host; no-op on AMD64), called by `build.ps1` and `package.ps1`.
2. **B-to-A-0002 accepted, lands later.** `register-h0.ps1` will register through `Register-ScheduledTask` with a `[datetime]`, `StartWhenAvailable`, and battery settings off; the date bug is real
   (A's task was registered for 11 December as well; it would not have fired even without the reboot). Not urgent while the jam runs.
3. **B-to-A-0003 accepted.** `ownership.json`: `briefs/**` moves to `shared.any_device`; `GAME.md` is `director_only`.
4. **B-to-A-0004: the folder is `VFX/`** (the contracts win; R-0001 point 4 is corrected to `/JamAssets/VFX/NS_<event>` and `/JamAssets/Audio/MS_<event>`). `BP_EventFx` is struck from C-0003.
   The binder (lane A5) sets Niagara user parameters **`User.Strength`** (float) and **`User.Colour`** (linear colour: the team colour of `Who`, white when `Who` is -1), spawns at `Where` in
   world space, auto-destroy; MetaSounds get a float input **`Strength`** and play as a one-shot at `Where`. B delivers only `NS_<event>` and `MS_<event>`.
5. **B-to-A-0005 accepted in full.** Lanes A1 (rink, nets, puck, ice) and A5 (toy skater) set `render_custom_depth` and the stencil values from B's table (ice 2, boards/nets/figures 1, puck 3,
   trim strips and beacon 0 with no shadow) and assign B's material instances by the object paths `/JamAssets/Look/<name>.<name>`, falling back to the engine default when absent, never failing.
   A1 drives `RinkHalfLength` and `RinkHalfWidth` (cm) on the dynamic `MI_Ice_Night` instance from `game.rink`, and spawns the 10 cm emissive trim strips (`MI_Trim_Blue` x < 0, `MI_Trim_Orange`
   x > 0) and the two far-net beacon posts (`MI_Trim_Beacon`, d 12 cm, 8 m, behind each net) from the rink size. The table is an addition to C-0002.
6. **Device B's machine is not the plan's machine**: Windows 11 ARM64, 64 GB unified memory shared with the GPU, the editor running x64 under emulation. TRACKS.md section 5 caps for B are
   therefore **4 builders and at most 2 editors**, and B's lead measures before each new editor as it already does. B's H6 row is expected at about H9; accepted.
7. **New event name** `directors_call` (lane A7, payload `Kind` = the tunable key, `Strength` = the new value as a float where meaningful) joins C-0003.
