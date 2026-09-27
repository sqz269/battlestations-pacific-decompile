# The loss warning 009813A0 (packet `cc9_loss_warning_read`)

Address: 009813A0, ledger name `BSP_WarningManager_ReportLoss` (a hypothesis). It is
`__thiscall(manager = [00F8A0C4], unit)`, and the body is 009813A0..00982110 (defined).

Callers:
- 00959450 `BSP_Unit_OnDestroyed` (the unit death route, `docs/UNIT_DEATH_ROUTE.md`);
- 007F3B10 `BSP_Aircraft_OnDestroyed`.

This packet is docs only, with Ghidra read only. The body was read from the pseudocode. The
middle of it (its stack after the lock at about 00981700) is poorly recovered by the decompiler,
so the structure below is certain, and the argument lists of the internal helpers are not.

## 1. Structure

1. **The guard.**
   - `unit+54h` (the side) must be below 2;
   - and `unit->vtable[5Ch](18h)` (a squadron) or `vtable[5Ch](6)` (a ship) must be true.
   - Otherwise it returns at once. An aircraft (class 0Fh and below it) fails the guard.
2. **The text** (the HUD line). `009FFD20 BSP_Unit_LossCountingSlot` gives the unit's
   player-slot index.
   - **Slot with a name:** the text is `"[" + [slot+78h] + "] " + [slot+58h]`, where the slot
     record is `[game+18CCh + slot*4]`.
   - **Otherwise:** the key `"globals.warn_uslost"` for side 0, and `"globals.warn_japanlost"` for
     side 1.
   - **Either way**, a second key follows: `globals.warn_<class>_lost`, from the pointer table
     00E0B630 indexed by a virtual class-type query on the unit. The first entries read as
     carrier, destroyer, torpedo boat, battleship, cruiser, cargo, landing ship and level bomber;
     the strings are at 00D0CD9C onward.
   - The pair is posted through 005CF3D0 (not read: the message-line post).
3. **Under the manager's lock** (`[manager+14h]`, `EnterCriticalSection`, with a recursion count):
   - 00976F10 `BSP_WarningManager_CancelByTarget(unit)` drops every pending warning on this unit;
   - for a ship (IsKindOf 6), 00975D00 -> 0096AE90 -> 00975E30 remove it from the manager's
     per-ship lists (not read further);
   - 006E6670 on `unit+2A0h` and the word at `unit+2B8h` (not read: a projectile or effect
     handle release).
4. **The `kill` channel, twice.**
   - Its name is the 4-byte string `"kill"` at 00D18A84 in the first run, and a literal `"kill"`
     in the second (a conditional branch that was not separated).
   - Each run takes the channel index (00980150) and evaluates it (0097B8C0
     `BSP_WarningChannel_Evaluate`), with three pushed variants: the unit, a class word
     (`unit+2B8h`), and `[unit+2D4h]`.
   - For every listener it calls `00887E50 BSP_MissionLuaHost_CallNamedThreadSafe`: **the
     mission script's registered `kill` callbacks run here**.
5. **Teardown.** 0077EDF0 `BSP_MessageSystem_EntitySuppressSlot`, 006952A0
   `BSP_Observer_UnregisterPair`, 00695870 `BSP_CallbackOwner_Destroy`, and 006E0860 (not
   read).

## 2. Who reads its effects

- **The HUD:** the message line posted in step 2.
- **Audio:** the voice line keyed by the same `globals.warn_*` string, through the message
  post. Not traced to the sound bank.
- **The warning manager:** the unit's pending warnings are cancelled, so a ship that dies with a
  torpedo warning queued never raises it.
- **The mission scripts, which is not audio or HUD.** The `kill` channel's Lua listeners.
  - `include/bsp/mission_result.hpp` already notes that "defeat is a mission-script reaction to
    the kill warning". USN02's failure (`usn_2_java.lua` line 521: Houston or Exeter dead) is
    such a reaction.
  - **This is the step that matters for gameplay.** The host fails USN02 today through its own
    script-order bindings, so a binding here must not raise the callback a second time.

## 3. Binding plan (for cc9-world-init, the owner of `src/game_hosts_mission_frame.cpp`)

The entry is already the contract in `include/bsp/game_hosts_mission_frame.hpp`:
`void game_warning_report_loss_009813a0(std::size_t unit)`.

1. **The guard, bound concretely:** side < 2 and (ship kind or squadron kind). An aircraft
   returns.
2. **The text.** Record `WarningManager::loss_text_005cf3d0`, keeping the chosen key
   (`warn_uslost` / `warn_japanlost` / the slot form, then `warn_<class>_lost`) in a counter per
   key.
3. **The cancel.** Bind `CancelByTarget` over the manager's pending list: it is the one effect
   on other warning rows. Keep 00975D00 / 0096AE90 / 00975E30 and 006E6670 as records.
4. **The `kill` channel.**
   - Record `WarningManager::kill_channel_0097b8c0` with a listener count.
   - **Do not call the Lua listeners** until the script host's own `kill` event path is shown to
     be the same one, or USN02 would fail twice.
   - A census of which mission scripts register `kill` listeners is the first read of that
     follow-up.
5. **Wiring.** The gunnery death route (`kUnitDeathRouteBound`, ON) replaces its
   `WarningManager::report_loss_009813a0` record with the entry, under a new switch in the frame
   file. Planes also reach it through 007F3B10 for squadrons, which the host does not route yet.

**Pairs and predictions.** USN02 9200/9000 and E2, streams on:
- **USN02:** entries 22, guard passes 22 (12 `warn_uslost`, 10 `warn_japanlost`, if no named
  slot), cancels 0..22.
- **E2:** entries 51 and guard passes 0, since every loss is an aircraft.
- **Both:** the whole native table and every death row identical, as long as step 4 stays a
  record.

## 4. no_ghidra_function and names

- No body without a Ghidra function. 009813A0, 009FFD20, 00976F10, 00980150, 0097B8C0 and
  00887E50 are all defined.
- **Name added:** 009813A0 `BSP_WarningManager_ReportLoss` (a hypothesis).
