# Mission Lua event channels, segment 65 (cc13-events1)

Requested by cc9 (the gameplay lead), to diff its Lua host against the image. Worker cc13-events1,
2026-10-12. This is a read: nothing here is wired into a host. Status of every statement below is
**exported/read from the disk listing** (`bsp.py disasm-raw`, Capstone over the on-disk image) unless
it says otherwise. Ghidra decompiles were used to navigate; every argument order, box type and
flag claim was checked against the listing. Names are hypotheses. Ledger names were added for the
seventeen unnamed routines in the segment-65 lease (`config/names/00980000.jsonl`). Ghidra was not
written; renames are left to the lead.

Scripts, all in `local/` of this worktree (ignored): `ev1_scan.py` (rel32 E8/E9/0F 8x plus
absolute-dword census over every section), `ev1_disp.py` (condensed listing), `ev1_105.py` (which
register/stack slot is the base of every `+104h`/`+105h` access), `ev1_subs.py` (subscription
constructor -> vtable -> slots), `ev1_match.py` (matcher field order). Call-site rows are in
`reports/cc13_event_channels_calls.json`; `tools/verify_report_calls.py` checked all 42 rows against
live Ghidra with 0 failures.

**Positive control for the fire-site census.** The same scan finds 23 calls to `0097B8C0`
`BSP_WarningChannel_Evaluate`: one in each of 19 dispatchers and two each in the kill (`009813A0`) and
hit (`00988510`) dispatchers, which is exactly the shape read from the listings. `ghidra callers` was
not used for any census.

## 1. The shared shape of a dispatcher

Every dispatcher runs this sequence except `input` (`00982540`: no lock, no flags, it reads the subscriptions' input sets directly) and `hpEvent` (`00986B00`: locks, but evaluates inline and never touches the flags):

1. `__thiscall`, ECX = the WarningManager (the global at `00F8A0C4`). Take the lock at `+24h`
   (`EnterCriticalSection`, re-entrancy count at `[lock+18h]`).
2. Clear `+104h` and `+105h` (the two bytes of section 4).
3. `00980150 BSP_WarningManager_ChannelIndex(manager+F8h, name)`: a case-insensitive
   `std::map<NativeString, list>`. If the channel's list size (`channel+8h`) is 0, skip to step 7.
4. Box the event parameters into a `vector<Box*>`. Box kinds, from the vtables' `kind()` slots
   (`00968090`/`009680F0`/`00968130`): `00D1AF24` = entity id (16-bit, entity `+174h`, kind 0),
   `00D1AF2C` = int (kind 1), `00D1AF34` = bool (byte zero-extended, kind 2), `00D1AF3C` = float.
   Strings are boxed through `0096DC00`.
5. `0097B8C0(channel, params, out)` (cdecl): for each subscription in the channel's list, call
   `subscription->vtable[0Ch](params)`; on true append the callback name (`subscription+4h`).
6. If at least one callback matched: build a Lua argument vector (`008849D0` float, `00884A00`
   int-as-number, `00884A30` boolean, `00884A90` entity from an entity pointer, `00884AC0` entity
   from a handle's `+18h` id, `00884FC0` string; variant tag 0 number, 2 boolean, 3 entity) and call
   `00887E50 BSP_MissionLuaHost_CallNamedThreadSafe(host = [game]+1A08h, 0, &callbackName, args,
   0, -1)` once per matched callback, in list order. **Then set `+105h = 1`.** With no match the
   store is jumped over (for example `009818BD je 009819E8` skips the store at `009819DB`).
7. Optional default warning (section 3, column "default"), then unlock.

`00887E50` (docs/MISSION_LUA_HOST.md) runs the callback inline, **or queues it** on the host's
deferred queue when `BSP_FrameJobPool` says so (drained at fixed-step row 7,
`BSP_MissionLuaHost_DrainQueuedCalls`), **or drops it** when `game+1FE4h == 2`. `+105h` is set in all
three cases: it means "a subscription matched", not "a callback ran".

The base register of every `+105h = 1` store was resolved to the manager: `ev1_105.py` follows the
ECX copy through the stack slot it is spilled to, with EH-state anchors (`[esp+N]` resolving to the
`push -1` slot) to detect ESP drift. For the five functions where the linear sweep drifted
(`009813A0`, `00980E50`, `00986480`, `00986820`, `00988510`) the slot was checked by hand: each
reloads the manager from the slot written in the prologue (for example `009813A0` stores EBP = ECX at
`009813CC` and every later `[esp+10h]` read is that slot).

## 2. Subscription path (segment 64 loaders)

**Registration.** Lua `AddListener(channel, id, params)` is `008C6760`. It calls `00980C10`
`BSP_WarningManager_AddListener` (only caller), which under the lock:

- looks up the channel (`00980150`);
- finds or inserts the listener slot keyed by `id` (`00978D60`, called at `00980CC8`);
- builds the subscription with `0097E360 BSP_WarningManager_ParseEventBlock(name, params)` and
  stores it into the slot (`00980CDA mov [ebx], eax`). **The previous subscription for the same
  (channel, id) is overwritten without being deleted**: re-adding an id replaces it and leaks the
  old object; behaviourally the new block wins.

`0097E360` compares the channel name against 23 names (`__stricmp` for `recon`, then
`00425850` for the rest), `operator new`s the matching subscription, runs its constructor and then
calls **`subscription->vtable[1](reader)`**, the loader (`0097E8B9..0097E8C1`). ESI starts at 0
(`0097E37E xor esi, esi`). **An unknown channel name falls out of the chain at `0097E888 je
0097E8B9` with ESI = 0, so the native reads `[0]` and faults**: AddListener with a misspelled
channel crashes the original. A host should not silently ignore it if it wants parity, but it also
must not crash; record it.

**Removal.** `RemoveListener` (`008C6990`) calls `00980D30` `BSP_WarningManager_RemoveListener`:
lock, channel lookup, `0096D6F0` find by id, `subscription->vtable[0](1)` (scalar deleting
destructor), erase. A missing id dereferences the end iterator; the checked iterator calls
`00BF6713` (the CRT invalid-parameter path). `IsListenerActive` (`008C6BB0`) goes through
`00980E00` (its only caller, `008C6D07`), the lookup without the erase per docs/LUA_BINDING_MISSION.md; not re-read here.

**The four vtable slots** (every subscription vtable is four slots: `ev1_subs.py`):

| slot | role | evidence |
| --- | --- | --- |
| 0 | scalar deleting destructor | `CG_scalar_deleting_dtor_*`; called with 1 by `00980D30` |
| 1 | loader: reads `callback` into `+4h`, then one filter per key into `+0Ch`, `+1Ch`, `+2Ch`, ... | called by `0097E360` (both AddListener and the archive load) |
| 2 | save: writes the block back | `0096DD30` (manager archive save, block `luaListeners`) calls `[sub]->vtable[8](writer)` at `0096E07A` after writing the id |
| 3 | matcher: `params[k]` against the k-th filter, all must pass | called by `0097B8C0` |

Filter readers used by the loaders: `009721C0` entity-id set, `009722D0` int set, `00970FF0`
string set (case-insensitive), `00972390` byte set (`exited`, `enter`), `0096AAD0` a float range:
it opens the sub-table and reads `min` (default `-1e10`, `00CE4ADC`) and `max` (default `+1e10`,
`00CE4970`). An empty set matches anything (`00979140`, docs/LUA_BINDING_MISSION.md). The three
loaders the lead named are `00972FE0` (hpEvent loader), `0096E850` (kill save) and `0096AAD0`
(the range reader).

**Save/load loses most filters.** Only five subscriptions have a full save slot: `recon`
(`0096E790`), `kill` (`0096E850`), `exitzone` (`0096E9F0`), `input` (`0096EA70`) and `surrender`
(`0096EAD0`). The other eighteen use `0096D2B0`, which writes `callback` and nothing else. The load
(`00980380`, calls `0097E360` at `0098079B`) rebuilds each listener through the loader, so after a
save and load those eighteen listeners come back with empty filters, matching every event of their
channel. Inferred from the two listings; not run.

**Matcher order** (`ev1_match.py`): every matcher tests the boxed parameters in the same order as
its loader's keys, field `+0Ch` against `params[0]`, `+1Ch` against `params[1]`, and so on, with a
bounds check before each index. `hit`'s matcher (`009685B0`) adds four `009658B0` range/int tests
for the last four keys. `hpEvent`'s matcher tests only (entity, reason); the `hp` threshold at
`+2Ch` is tested inline by the dispatcher.

## 3. The channel table

"Fire sites" are every rel32 call (or jump) to the dispatcher in the image, plus absolute
references, from `ev1_scan.py`. "Lua args" are in push order to the callback. "Keys" are the
subscription's filter keys in loader order (= box order = matcher order). ABI is from the `RET`
immediate.

| channel | dispatcher, ABI | fire sites (call site in caller) | Lua args (order, type) | keys | default |
| --- | --- | --- | --- | --- | --- |
| `recon` | `00980E50`, (party, unit, old, new), RET 10h | `0077B16B` in `0077B0C0` (entity virtual: the reconlevel writer; 51 vtable references) | unit (entity), old (number), new (number), party (number) | entity, oldLevel, newLevel, party | `BSP_ReconWarning_Construct` (9Ch) |
| `kill` | `009813A0`, (unit), RET 4 | `007F3B56` in `007F3B10` BSP_Aircraft_OnDestroyed; `00959507` in `00959450` BSP_Unit_OnDestroyed | victim (entity), lastAttacker (entity, from the handle copied from victim `+2B0h`), lastAttackerPlayerIndex (number, victim `+2D8h`) | entity, lastAttacker, lastAttackerPlayerIndex | `009744C0` kill warning (70h) |
| `hit` | `00988510`, (hitEntity, hitRecord*), RET 8 | `0077D149` in `0077CE60` BSP_Unit_RecordDamageAttribution (two absolute "hits" at `007A5C6F`/`00B070E6` are the bytes of `[ebp+98h]` operands, not references) | target, targetDevice, attacker (entities), attackType (string), attackerPlayerIndex (number), damage, fire, leak (numbers, hit `+48h`/`+4Ch`/`+50h`) | target, targetDevice, attacker, attackType, attackerPlayerIndex, damageCaused, fireCaused, leakCaused | `00974C70` hit warning (88h) |
| `exitzone` | `00982120`, (entity, exited byte), RET 8 | `007C6D65` in `007C6C30` BSP_Plane_OutOfActionCountdown; `007F31E6` in `007F31A0` BSP_PlaneSquadron_OnPlaneLeftMap; `0081BF0C` in `0081BEB0`; `00826E8F` in `00826D70` (both virtual, 9 vtables each) | entity (entity = arg->vtable[140h]()), exited (boolean) | entity, exited | `BSP_ExitZoneWarning_Construct` when `exited == 0` |
| `input` | `00982540`, (), RET 0, **no lock, no `+104h`/`+105h`** | `009875A2` in `00987590` BSP_WarningManager_Update (its first call) | inputID (number), value (number, input record `+24h`) | inputID | none |
| `surrender` | `00982990`, (party), RET 4 | `004D8979` in `004D87B0` BSP_Game_CheckMultiplayerPlayerCount | party (number) | party | none |
| `failure` | `00982C50`, (entity, failureType string, duration float), RET 0Ch | `0081B77D` in `0081B630` (virtual); `0093C0C2` in `0093BED0` BSP_ShipSystems_RollComponentFailure; `0093C4D1` in `0093C300` BSP_ShipSystems_PickRandomFailure; `0095AD52` in `0095ABE0` BSP_Unit_HandleMessage | entity, failureType (string), duration (number) | entity, failureType, duration (range) | EntityWarning `failure`+type, or ids `failureshipyard` / `failureairfield` |
| `leak` | `009832F0`, (entity, seconds float), RET 8 | `0093A513` in `0093A4F0` BSP_RepairTask_AddWaterSeconds | entity, seconds (number) | entity, duration (range) | EntityWarning `leak` |
| `fire` | `00983780`, (entity, seconds float), RET 8 | `0093A484` in `0093A470` BSP_RepairTask_AddFireSeconds | entity, seconds (number) | entity, duration (range) | EntityWarning `fire` |
| `repair` | `00983C10`, (entity, setting int), RET 8 | `0064A281` in `0064A270` BSP_RepairTask_SetRepairPriority; `0064A33B` in `0064A2F0` | entity, repairSetting (number) | entity, repairSetting | none |
| `zone` | `00983F50`, (entity, zoneName, enter byte), RET 0Ch | **none**: no rel32, no absolute reference | entity, zoneName (string), enter (boolean) | entity, zoneName, enter | none |
| `command` | `00984300`, (entity, target, cls, status), RET 10h | `0071F79B`, `0071F8C0` in `0071F600` BSP_WeaponDirector_BeginCommandBase; `00836D51` in `00836920` BSP_WeaponDirector_Step; `0084E431` in `0084E010` BSP_PlaneSquadronCommandBlock_Step; `009E595C` in `009E5770` BSP_ShipAi_MoveToPosStateStep; `009E5C35` in `009E59C0` BSP_ShipAi_MoveOnPathStateStep | entity, target (entity), command (string, `cls->vtable[4]()`), status (string) | entity, target, command, status | none |
| `target` | `00984800`, (entity, target), RET 8 | `0071F7E8`, `0071F909` in `0071F600` | entity, target (entities) | entity, target | none |
| `ammoType` | `00984BA0`, (squadron), RET 4 | `006E5633` in `006E4D50` BSP_MBombPlatform_Drop | squadron (entity), ammoType (number, `007EDAD0` BSP_PlaneSquadron_AmmoType) | entity, ammoType | none |
| `stock` | `00984EB0`, (entity, vehicleClass, stock), RET 0Ch | `006C7CF1` in `006C7B10`; `006C7DF7` in `006C7D10`; `006CA8C0` in `006CA770` BSP_AirOps_AddStock; `00846FAB` in `00846D90` | entity, vehicleClass, stock (numbers) | entity, vehicleClass, stock | none |
| `gui` | `00985250`, (guiName, enter byte), RET 8 | `00987098` in `00987090` BSP_WarningHook_ForwardEcxEdx, a hook stored at `[00F8BF4C]` by Init (`009874F6`) and called at `00AC44BE` in `00AC4450` BSP_GuiLayer_SetVisible | guiName (string, `layer->vtable[7Ch]()`), enter (boolean) | guiName, enter | none |
| `generate` | `00985590`, (entity), RET 4 | `0077FE50` in `0077FAD0` (5 direct callers, 12 vtable references: entity init paths) | entityName (string, `entity->vtable[10h]()`), entityType (number, entity `+C4h`) | entityName, entityType | none |
| `player` | `00985920`, (playerIndex, event string), RET 8 | `004D100C` in `004D0FC0` (callers `007702E0`, `00773420` BSP_Session_ReceiveRemovePlayer); `007707C3` in `00770750` BSP_Session_SetPlayerSurrender | playerIndex (number), event (string) | playerIndex, event | none |
| `musicOver` | `00985C50`, (music string), RET 4 | `00987087` in `00987080` BSP_WarningHook_ForwardEcx, stored at `[00F8BBCC]+230h` by Init (`009874EC`), called at `00A78AC2` in `00A789C0` BSP_StreamedDialogManager_Update | music (string) | music | none |
| `chat` | `00985F30`, (playerName, messageType, message), RET 0Ch | `005D009A` in `005D0050` BSP_MultiChatScreen_AddMessage | playerIndex (number: the `game+18CCh` slot whose active player name `+58h` matches, else -1), message (string), messageType (number) | playerIndex, message, messageType | none |
| `entityKilled` | `00986480`, (entity), RET 4 | `0077D253` in `0077D1A0` BSP_UnitInstance_DestroyAndBroadcast | entity, killReason (string, table `00E0CF04[entity+70h]`) | entity, killReason | none |
| `shipLanded` | `00986820`, (ship), RET 4 | `0074AE79` in `0074AD90` BSP_LandingShip_FinishLanding | ship (entity) | entity | none |
| `hpEvent` | `00986B00`, (entity, reason, oldHp, newHp), RET 10h, **no `+104h`/`+105h`** | `008791EB` in `00879070` BSP_UnitInstance_ApplyDamage | entity, reason (string) | entity, reason, hp | none |

Per-channel gates read from the listings:

- **recon**: fires only for a live unit (`+5Ch` set, `+5Dh`, `+5Eh`, `+60h` clear).
- **exitzone**: the entity is replaced by `entity->vtable[140h]()` (the controlling unit; a ship
  returns itself, docs/CONTROLLED_UNIT.md) before boxing.
- **entityKilled, shipLanded, hpEvent**: if the entity has a scene parent (`00923810(entity, 1)`,
  the `+3Ch` chain), the event fires only when that parent answers `IsKindOf(44h)`
  (`009864EB`, `0098688B`, `00986B67`). Kind 44h is not named in the repository.
- **hpEvent** fires a subscription only on a threshold crossing against the subscription's `hp`
  (`+2Ch`), checked in x87 at `00986D0B..00986D75`: reason `damage` needs `old > hp` and
  `hp > new`; reason `repair` needs `old < hp` and `hp < new`; both comparisons strict. The reasons
  are compared with `__stricmp`. Other reasons never fire. It boxes only (entity, reason).
- **kill** first builds and posts the loss text (`globals.warn_uslost`/`warn_japanlost`/
  `warn_<class>_lost`, `005CF3D0`) before the lock, only for party < 2 and kind 18h or 6. Under the
  lock it cancels the unit's warnings (`00976F10`) and, for a ship, runs `00975D00`, `0096AE90`,
  `00975E30` (the ShipNoiseInfo record, docs/CC13_WARNING_REPORTS.md). These run whatever
  `+104h`/`+105h` say.
- **hit**: target = `hitEntity->vtable[140h]()`, kept only if it is kind 2 (`0098858A..00988592`);
  null returns at once. targetDevice = `hitRecord+0` when it is non-null, not the target and kind
  1Eh. attacker: `hitRecord+4` itself when it is a ship whose `[+538h]+510h` or `+514h` is positive,
  or kind 17h (attackType index 11h); otherwise the resolver `hitRecord+4 -> vtable[108h]()` gives
  the attackType index (`[[resolver+4]+8]`, table `00E08E58`) and the attacker `resolver+ACh` when
  it is kind 2. attackerPlayerIndex = `resolver+1Ch`, or -1. Before the channel the dispatcher runs
  the feedback reports and the per-target rate limit cc9 already documented.

## 4. When each channel fires

The dispatch is synchronous inside its producer; nothing is queued on the warning side. Where the
producer sits in the frame:

| channel | producer context | frame position |
| --- | --- | --- |
| `input` | `00987590` BSP_WarningManager_Update, first call | per frame: `00737A50` BSP_Application_RunFrame -> `004E4A40` BSP_Game_OnMove -> Update. Before the warning queue work in the same Update |
| `hit` | `0077CE60` <- `009239A0` BSP_Entity_DispatchQueuedHit <- `00926700` deferred entity event queue drain | fixed step rows 6 and 14 (docs/FIXED_STEP_FANOUT.md), LIFO (docs/ENTITY_EVENT_QUEUES.md) |
| `kill` | unit/plane death: `00959450` (callers `00747170`, `0074DB70`, `007BCAA0` BSP_Plane_OnDestroyed, `00824B60` BSP_UnitInstance_OnWrecked) and `007F3B10` (virtual) | inside death processing; not traced to a fixed-step row |
| `entityKilled` | `0077D1A0`, the unit vtable `+70h` destroy path (docs/ENTITY_EVENT_QUEUES.md) | inside destruction; not traced to a row |
| `hpEvent` | `00879070` <- `00879810` ApplyHealthDelta, `0087D730` | inside damage application |
| `failure`, `leak`, `fire`, `repair` | ship-system rolls (`00826F10` BSP_ShipEntity_ApplyHitRecord -> `0093BED0`) and unit message handling (`00821E80` BSP_UnitInstance_HandleMessage); `repair` also from the HUD (`00603120` HudPauseScreen_Enter, `0064DD30` HudShipScreen45_Update) | inside hit/message handling; the HUD path is player input |
| `recon` | `0077B0C0`, the reconlevel writer (docs/RECON_SENSOR_PASS_BINDING.md) | inside the recon pass |
| `exitzone` | plane out-of-action countdown, squadron left-map, two unnamed virtuals | inside plane/unit update |
| `command`, `target` | weapon director command begin/step, squadron command block step, two ship-AI state steps | inside AI/director think |
| `ammoType` | bomb platform drop | inside weapon release |
| `stock` | air-ops stock changes (scene load `006CADD0`, Lua `AddAirBaseStock` `00896A90`, shipyard launch) | load time or inside the caller |
| `generate` | `0077FAD0`, entity initialisation (5 callers, 12 vtables) | not traced; likely `SEntity::InitAll` (row 12) or spawn |
| `shipLanded` | `0074AD90` <- `0074AF50` BSP_LandingShip_Update | inside the landing ship's update |
| `surrender` | `004D87B0` <- `004DA6C0` EnterMissionState, `0076FFC0` Session_FlushOutboundStep (row 13) | multiplayer only |
| `player`, `chat` | session receive (`00773420`, `00770850`), multiplayer chat screen | multiplayer only |
| `gui` | `00AC4450` GuiLayer_SetVisible, when a layer's notified visibility byte `+F5h` changes (`00AC44A9`) | GUI, any time a layer shows or hides |
| `musicOver` | `00A789C0` BSP_StreamedDialogManager_Update, sound system | sound update |
| `zone` | none | never fired natively |

## 5. The `+104h` and `+105h` bytes (item c)

**What sets them.**
- `+104h` is set to 1 by exactly one routine: `00979670` (`009796BF`), whose only caller is the Lua
  native `OverrideMessage` (`008D0EA0`, binding row `00E0C7B0`, error string `luaMW_OverrideMessage
  failed:`). It sets the byte when `+D0h` is clear, the local slot is in [0, 7] and the clock
  `00F876A4` is past `00CE3D34`, and raises the script's own 7Ch warning. Scan:
  `C6 ?? 04 01 00 00 01` finds only it and an unrelated HUD screen (`006082D0`).
  So `+104h` means "a callback replaced the default message", and only when the callback ran inline.
- `+105h` is set to 1 by **every** locking dispatcher (21 stores, section 1 step 6), not only the six
  the request listed: `00980E50`, `009813A0` (twice), `00982120`, `00982990`, `00982C50`,
  `009832F0`, `00983780`, `00983C10`, `00983F50`, `00984300`, `00984800`, `00984BA0`, `00984EB0`,
  `00985250`, `00985590`, `00985920`, `00985C50`, `00985F30`, `00986480`, `00986820`, `00988510`
  (twice). `input` and `hpEvent` never touch either byte.

**What reads them.** An image-wide census of byte reads at `+105h` (`80 ?? 05 01 00 00`,
`38 ?? 05 01 00 00`, `3A`, `8A`, `0F B6`, `F6` forms) finds the two known sites plus
`BSP_HudShipScreen45_*` and `BSP_GuiMovie_Start`, whose `this` is a HUD object, not the manager.
`+104h` is read at the end of recon, kill (twice), exitzone, surrender, failure, leak, fire, repair,
target, ammoType, stock, musicOver, chat and hit (twice). In the one channel without a default
warning that was checked (`ammoType`, `00984E6E..00984E92`) the read only chooses between two
identical unlock tails.

**Neither `+105h` read skips a default warning block.** Both skip a **second listener pass**:

`009813A0` (kill), after the first pass:

```
00981A5B  cmp byte [edi+104h], 0 ; je 00981AA4        ; +104h: OverrideMessage ran
00981A64  ...destroy attacker handle ; jmp 009820E3    ;   -> unlock and return, no warning
00981AA4  cmp byte [edi+105h], 0 ; jne 00981E25        ; a kill subscription matched
00981AB1  attacker = [esp+88h] (handle+14h) ; null -> 00981E37
00981AC2  attacker->vtable[5Ch](0Fh) ; not a plane -> 00981E25
00981AD3  attacker->vtable[140h]() -> 006E1B90 re-points the attacker handle
00981AEE  "kill": clear +104h/+105h, ChannelIndex, box (victim, new attacker, +2D8h),
          Evaluate (00981C31), call the matched callbacks, +105h = 1 (00981D9C)
00981E07  cmp byte [edi+104h], 0 ; jne 00981A74 (no warning)
00981E25/00981E37 -> 00981E3E  the default kill warning block
```

`00988510` (hit) has the same structure: `0098900B` `+104h` -> `0098896D` (no warning, return);
`0098901A` `+105h` -> `00989604`; otherwise, if the attacker (`[esp+B0h]`) is a plane,
`vtable[140h]()` re-points it (`00989047..00989060`) and the `hit` channel is evaluated again
(`009892E4`) with the replaced attacker; `009895DC` `+104h` -> exit; else `00989604`, the default hit
warning block.

So the skipped block is "evaluate the channel a second time with the plane attacker replaced by
`attacker->vtable[140h]()`, and call every subscription that now matches". `vtable[140h]` is the
controlling-unit accessor (a ship returns itself, docs/CONTROLLED_UNIT.md); for a plane the
returned object was not resolved here, but the shape (a listener keyed on the attacker that did
not match the plane) makes the squadron the likely answer. **Unverified.**

**Side effects of the skipped block.** Natively: the attacker handle in the dispatcher's frame
(`006E1B90`), the manager's `+104h`/`+105h`, the channel map lookup, box allocation, and the
callback log line in the kill matcher (`004254B0`). No RNG call, no unit order, no scoring, no
objective state in the native part. **But it calls mission Lua callbacks**, and those are gameplay:
cc9's own measurements show a `recon` callback issuing three `NavigatorAttackMove` orders (JM06) and
a `hit` callback spawning six planes (USN01 prediction). So `+105h` gates gameplay: if any
subscription matched the plane attacker in pass 1, no subscription keyed on the plane's controller
gets a call in pass 2, even if it would match. A host that fires `kill`/`hit` once, with the plane as
attacker, misses every listener keyed on the squadron; a host that fires both unconditionally
double-fires when both match.

**The default warning blocks themselves** (gated by `+104h`, `+D0h`, slot and clock, never by
`+105h`):
- kill, `00981E3E..009820AC`: `00803CE0` relation and recon-level reads, a plane-kill throttle in the
  statics `00F8A0CC`/`00F8A0D0` (a function-local static: init bit and last time, compared with
  `game+64Ch`), `00922A90` query, `009744C0` + `009763E0` report. The only writes outside the warning
  object are those two statics, which nothing outside this block reads (not censused).
- hit, `00989604..`: the same guard set, then `00974C70` + report.
- Both builders draw no RNG (docs/CC13_WARNING_REPORTS.md, "RNG").
These blocks are presentation (voice/text queue), with the caveat that their warnings are
**observable from Lua**: the queue is what `OverrideMessage` competes with, not script state.

## 6. Corrections to existing documents

To `docs/MISSION_EVENTS_UPDATE.md`, "Channel producers" (dated note appended there is left to the
doc's owner; this list is the evidence):

1. `recon` has a producer: `0077B0C0` (virtual, reconlevel writer). cc9's
   docs/LUA_BINDING_MISSION.md already says so.
2. `ammoType` has a producer: `006E4D50` BSP_MBombPlatform_Drop.
3. `gui` and `musicOver` are fired, through function-pointer hooks that Init installs
   (`009874EC`, `009874F6`): GUI layer visibility and the streamed-dialog update.
4. `zone` is the only channel with no fire site.
5. `exitzone` has four producers, `failure` four (adds `0081B630`), `command` six (adds the two
   ship-AI state steps).
6. `entityKilled`'s producer is `0077D1A0`, not `0077CE60` (`0077CE60` produces `hit`).
7. `failureshipyard`/`failureairfield` are warning ids built after the `failure` channel, and
   `damage`/`repair` are `hpEvent` reasons; neither are channels. `globals.warn_uslost` is loss
   text, not a channel.
8. The correction under "Correction from docs/COMMAND_COMPLETION.md", "the `+105h` store at `0098473F` is on the channel object" is wrong: EBP is
   reloaded with the manager at `009845B2` (`mov ebp, [esp+20h]` after three pushes, the prologue
   slot written at `00984329`; EH-state anchors at `[esp+70h..7Ch]` resolve to the `push -1` slot
   throughout) and is not written again before `0098473F`.

To cc9's `kLuaListenersBound`/`kLuaHitListenersBound` substitutions (for cc9 to judge): callbacks in
the image do receive arguments (section 3); `kill` and `hit` have the second pass above.

## 7. Uncertainties

- `vtable[140h]` for a plane (section 5) and kind 44h (section 3) are unresolved.
- The frame position of the producers marked "not traced" was not walked to a fixed-step row.
- The save/load filter loss is inferred from the save slots and the load path; not run.
- `00887E50`'s queue decision depends on the job pool; which callers run on a worker thread was not
  read, so "inline" is the expectation, not a measurement.

## Handoff

Done: the full channel table, the subscription path, and item (c). Nothing is wired; no C++ was
written (the dispatchers are cc9's host territory). Open:

1. Resolve the plane's `vtable[140h]` (find the plane vtable through its constructor) to confirm
   the kill/hit second pass keys on the squadron.
2. Name kind 44h (the scene-parent gate of `entityKilled`, `shipLanded`, `hpEvent`).
3. Ghidra renames for the seventeen ledger names added here, through `ghidra_annotate.py --apply`.
