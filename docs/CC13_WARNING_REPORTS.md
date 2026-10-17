# WarningManager report builders, segment 64 (cc13-warnings1)

This continues `docs/CC13_OPTIONS_AND_HARDWARE.md` (WarningManager section, handoff item 7). The
WarningManager is the singleton `[00F8A0C4]` = game+21E0h. These functions turn game events into
the message ids that `009763E0` queues and `0096DBB0` checks against the message table. They only
read unit state.

Status words: **exported** (Ghidra pseudocode in `exports/`), **reconstructed** (C++ in
`src/warning_reports_cc13.cpp`), **build-tested** (compiles in `bsp_core`). Nothing here is wired
into a host, fixture-tested or game-validated. The C++ names are hypotheses. Each function returns
the id it builds, plus the watched-entity slots, so a host can do the allocation and the report
itself.

Evidence: every builder was read from the listing (`bsp.py disasm-raw` / `ghidra disasm`), not just
the pseudocode. Ghidra's pseudocode for `00974C70` merges stack slots and is unusable. Every
literal was read from the PE (`tools/pe_const_read.py`).

## Shared vocabulary

- **Relation** (`00803CE0`, fastcall, ECX = viewer side id, EDX = entity, read whole
  `00803CE0..00803D30`): returns 3 when the viewer side's recon level of the entity is below 2. The
  level is the dword at entity `+1ECh` + side*34h, or `+1F0h` when the byte at `+1F8h` + side*34h
  is set. Otherwise it compares entity `+54h` with the viewer side, where party 2 is neutral:
  0 own, 1 enemy, 2 neutral. The local side id is `[game+18CCh + slot*4]+28h`.
  `00968370(entity)` is the local-side wrapper (null gives 3). `00968400(entity)` tests "nonnull
  and entity `+54h` is the local side".
- **Watched slots.** A Warning has three observer handles. Their names are the archive keys of
  the Warning load `009768F0` and save `0096CF10`: `shortcut` (`+24h`), `ambient` (`+3Ch`) and
  `reporter` (`+54h`). Each handle stores its pointer at `+14h`. `005B7F60` sets a handle from a
  raw pointer and `00523B30` copies one handle into another; both register with `00694A60` and
  unregister with `006952A0`.
- **Kinds** (vtable `+5Ch` IsKindOf): 2 unit, 6 ship, 0Fh plane, 18h squadron, 1Ch command
  building, 2Ah payload weapon, 45h airfield, 0Ch (excluded from the capture-ship scan).
- Entity `+78Ch` != 0 means a **base**; 0 means a naval **supply** point (both the hit and the
  capture builders use it).

## Reconstructed (build-tested, reconstruct-only)

### `00974C70` hit warning constructor: `construct_hit_warning_00974c70`

`__thiscall(ECX = warning (88h), target, hit, unused)`, `RET 0Ch`. The only caller is the hit
channel `00988510` (`0098997C`). `attacker` is `[hit+4]`->vtable `+108h`, or null.

The C++ builds the object in place (`cc13_seg49_64_78::WarningObject`), in listing order:
`00973BA0(target, unused)` (the base stores `+80h` = target and links it with no null test; it
ignores its second argument), vtable `00D1B308` (`00974CB5`), `+6Ch` observer vtable `00D1B2F0`
(`00974CBB`), id `hit`, and byte `+84h` (archive key `areaexclusion`) = 0 (`00974CE2`). Every
slot store below goes through `005B7F60` (or `00523B30` for the shortcut). Each unlinks the old
entity and links the new one when the entity changes.

Flags, in listing order:
- `carrier`: attacker `+ACh` when it is kind 2.
- `attacker_local`: attacker `+1Ch` == game+18ECh (`00974D31`).
- `target_player`: the target, or a kind-0Fh plane's `+9D4h` owner, is `[00E188D8]` (`00974D50`).
- `payload`: attacker `+CCh` is kind 2Ah and the carrier is a local-side (`00968400`) kind-18h
  squadron (`00974D83..00974DC3`).
- The relation of the target (`00974EC3`). A null target counts as 3.

The id starts as `hit` (`00D1B438`). The branches:
1. Own target, attacker not local: `weareunderattack` + `ship` / `planesquadron` / `other`.
   Command buildings get `navalsupply` (not a base), `commandbuildingplayer` (owner slot `+180h` is
   local), or `commandbuildinglast` (exactly one entry of `[game+19CCh]+16Ch` has the local slot;
   otherwise nothing). Then either `player`, or reporter = target (`00975139`) and shortcut =
   reporter (`00975142` `00523B30`). Sets `+84h` = 1 (`00975112`).
2. Attacker local, relation not enemy: `playermistake`. Only an own target goes on to
   `friendlyfire` + `ship` / `plane` / `other`, with reporter = target.
3. Otherwise, only when `payload` holds: `payloadhit`, then `player` when the attacker is local.
   Otherwise it appends `plane` and `ambient` and sets ambient = carrier.

### `009744C0` kill warning constructor: `construct_kill_warning_009744c0`

`__thiscall(ECX = warning (70h), victim)`, `RET 4`. Its caller is the kill channel `009813A0`.
The object is built in place: `00973A90` base (`009744E7`), vtable `00D1B2CC` (`009744F1`), and
`+6Ch` = victim as a plain pointer, not an observer handle (`009744F7`). Then the id is `kill`
(`00D18A84`). The ambient and reporter stores go through `005B7F60` (`009746EB`, `009748B0`,
`00974A41`, `00974BB8`), as in the hit warning. `killer` is victim `+2C4h` when it is
kind 2. The branches depend on the victim's relation:
- **Own**: `own` + `ship` / (`plane` `ambient`, ambient = victim) / `planesquadron` /
  `airfield`.
- **Enemy**: `enemy`. `player_kill` = `00965810(killer)` (also called with a null killer) and
  victim `+2D8h` == local slot.
  - Ship, squadron or airfield victim: append its kind. If the killer is own-side
    (`00968370` == 0), reporter = killer, then the killer's kind (`ship`, or `plane` for kinds 18h
    and 0Fh, else `other`), then `player` on a player kill. Otherwise append `unknown`.
  - Plane victim: `plane` `ambient`, ambient = killer. When there is a killer, append its kind
    and `player`.
- **Neutral**: `neutral` `ambient`, ambient = victim, then `ship` or `plane`.
- **Unknown**: just `kill`.

### `00977DA0` capture-ship report: `report_capture_ship_00977da0`

`__thiscall(ECX = manager, subject, point)`, `RET 8`. Callers: `006F29D0`, `006F6760`,
`BSP_LandingShip_Update` `0074AF50`.

Under the `+24h` lock it checks four guards: a network session (game+1FE4h != 0, so it only
reports in **multiplayer**), `+D0h` clear, slot in `[0,8)`, and clock > 4.0f.
1. With no subject and point `+180h` == local slot, it reports `captureshiprange`. With both
   arguments null the native dereferences a null pointer.
2. With no point and a subject, it builds `captureship` + `base` or `supply`. `range` is subject
   vtable `+218h`, an x87 return stored as a float. Then it walks list 6 (`[game+19CCh]+64h`). The
   first unit with `d2 < range`, not kind 0Ch, and owner slot `+180h` == local slot gets
   `own` (subject `+7B0h` == local side id) or `nmy`. It is reported and the walk stops.

The x87 work was checked against the listing at `00977FB3..00978009`. Each difference is stored as
a float. The stack computes `(dy*dy + dx*dx) + dz*dz` and stores the result as a float. `FLD range;
FCOMIP` with `JBE` skips unless `range > d2`. The C++ evaluates the sum in double before rounding
to float, which can differ from the 80-bit x87 intermediate by at most one float ulp.

### `009781E0` capture-point report: `report_capture_point_009781e0`

`__thiscall(ECX = manager, point, slot_arg, party_arg)`, `RET 0Ch`. Its caller is `006F4D10`. It
has the same guards as `00977DA0`, plus `009761D0(point)`. It reads `current` = point `+54h` and
`previous` = point `+7B0h`. The id starts as `capture`. A kind-1Ch point appends
`commandbuilding`, then one of three branches:
- **Neutralize**: neither `previous` nor `party_arg` is 2, and `current` is 2. It appends
  `neutralize`, then `rel = 00803510(local side, party_arg)`.
  - `rel == 1`: `nmy`, then `supply`, or `base` plus `player` when the controlled unit is a ship
    inside the point's range. That distance comes from `00427EB0`/`00427C90`/`00427E30`, is stored
    as a **double**, and is compared at `0097845A`.
  - `rel == 0`: `own`, and for a base when `slot_arg` == local slot: `oneleft` (one local command
    building left), `last` (none left) or `player`. A supply point gets nothing more.
  - `rel == 2`: nothing more.

  The table at `00E0CFE0` is `own`/`nmy`/`neu`/`unk`; only entries 0 and 1 are reachable here.
- **Capture**: `current` is not 2 and `previous` or `party_arg` is 2. It appends `cap`, then
  `neut` (`previous` is 2) or `nmy`. Then it appends `own`, followed by `supply` or by `base` plus
  `player` when point `+180h` is the local slot, when `current` is the local side. Otherwise it
  appends `nmy` + `base`/`supply`.
- When `current` is not 2 and neither of the others is 2, it appends only `cap`. When `current`
  is 2 and `previous` or `party_arg` is 2, it appends nothing.

Then `0096DBB0` (ECX = `[00F8A0C4]`) validates the id. A known id prints `Capture warning msg`
and reports `00974150(point, id)` (84h). An unknown id prints
`---WarningManager invalid Message--- %d`, passing the text pointer to `%d`, and is dropped.

### `00978A70` command-building report: `report_command_building_00978a70`

`__thiscall(ECX = manager, record, unused)`, `RET 8`. Its caller is `00777850` (session pump). It
has no lock and no guard. The id is `hitweareunderattack` + by record `+18h` (host read
`record_state_18`): 0 `commandbuildingplayer`, 1 `commandbuildinglast`, 2 `navalsupply`,
anything else nothing. It then reports the id as one 6Ch MessageWarning (`00978BC7..00978BF9`:
`operator_new(6Ch)`, `00975AC0(id)`, `009763E0(manager, warning)`) through the host hook
`report_message_warning_00975ac0`. The function returns the reported id.

### Smaller pieces

- `009761D0` `is_entity_reportable_009761d0`: entity `+5Ch` set, `+5Dh`/`+60h`/`+5Eh` clear, and
  the manager `+D4h` map byte clear (`0077EDF0` inserts on a miss). `RET 4`.
- `0096E5E0` `message_from_0096e5e0`: name = argument vtable `+10h` (`0096E63E`), then
  `"Message from |." + name + "|.: "` (`00D1B088`, `00D1B098`, two `004261A0` concatenations). It
  returns `00705E00(text, 1)` with ECX = game+21DCh (`0096E748`). It takes one stack argument and
  returns with `RET 4`. `callsite_census` and Ghidra xrefs find **no reference**, so it is dead code.
- `009766D0` `warning_create_from_archive_009766d0` (`BSP_Warning_CreateFromArchive`):
  `__thiscall(ECX = reader)`, plain `RET`, returns the warning in EAX. It reads `type` through
  reader vtable `+10h` (key `{0, 00CE4AD0}`, field `{1, &local}`, `00976711`), then compares in
  the order 0, 2, 3, 4, 5, 1, 6, 7, 8. Each arm calls `operator_new` (`00BF681B`, host hook
  `operator_new_00bf681b`) and, on a non-null block, the constructor directly:

  | type | size | ctor (called from C++) | vtable | load slot (`vtable+4`) |
  | --- | --- | --- | --- | --- |
  | 0 | 7Ch | `00976230` `construct_string_list_warning_for_archive_00976230` | `00D1B4F8` | `0097B4D0`, host hook |
  | 1 | 84h | `00973C20` `construct_entity_warning_for_archive_00973c20` | `00D1B26C` | `00976C60` `load_entity_warning_00976c60` |
  | 2 | 9Ch | `00973CA0` `construct_recon_warning_for_archive_00973ca0` | `00D1B2A8` | `00976E40` `load_recon_warning_00976e40` |
  | 3 | 70h | `00973A90` `construct_warning_base_00973a90` + vtable store at `0097679B` (00973D30 inlined) | `00D1B2CC` | `009768F0` `load_warning_009768f0` |
  | 4 | 88h | `00973D90` `construct_hit_warning_for_archive_00973d90` | `00D1B308` | `00977000` `load_hit_warning_00977000` |
  | 5 | 84h | `00973E10` `construct_exit_zone_warning_for_archive_00973e10` | `00D1B344` | `00976C60` |
  | 6 | 84h | `00973E90` `construct_confirm_warning_for_archive_00973e90` | `00D1B380` | `00976C60` |
  | 7 | 6Ch | `009756A0` `construct_new_unit_warning_009756a0` | `00D1B460` | `009768F0` |
  | 8 | 6Ch | `009757B0` `construct_powerup_warning_009757b0` | `00D1B4B0` | `009768F0` |

  `009768C7` calls `[[ESI]+4](reader)`; the C++ dispatches on the stored vtable to the four
  reconstructed load bodies. Only `0097B4D0` stays a hook (`string_list_warning_load_0097b4d0`),
  because its reconstruction (`cc13_seg62_seg65.cpp`) works on the raw-object `Seg62Host` model,
  not `WarningObject`. An unknown type, or a null allocation, leaves ESI = 0 and the native
  calls through a null pointer at `009768C7`; the C++ dereferences it the same way (no guard).
  Simplifications: the `type` local is uninitialised in the image and starts at -1 here; the
  EH unwind (free the block if a constructor throws) is not modelled; `009757B0`'s
  `[[00E188A8]+1FE4h]` read is taken as an argument just before the call
  (`network_session_1fe4`). No game host reaches it: 00980380's hook
  `Large62Host::warning_create_from_archive_009766d0` has no implementation.

## Named only (persistence and Warning internals)

| Address | Name | Evidence |
| --- | --- | --- |
| `009768F0` | `BSP_Warning_LoadArchive` | Keys `id` (type 0, `+4h`), `state` (type 1, `+0Ch`), `validityRefTime` (type 2, `+10h`), `message` table, `shortcut`/`ambient`/`reporter` (type 4). Called from `00976C60` and `0097B4D0`. |
| `00976C60` | `BSP_EntityWarning_LoadArchive` | `009768F0` + `entity` at `+80h`. Reached only through a vtable. |
| `0096CF10` | `BSP_Warning_SaveArchive` | Writes `type` (vtable `+10h`), `id`, `state`, `validityRefTime`, `message`, `shortcut`, `ambient`, `reporter`. Called from `0096D330`. |
| `0096D330` | `BSP_StringListWarning_SaveArchive` | `0096CF10` + the `msg` table of the strings at `+70h`. |
| `0096DD30` | `BSP_WarningManager_SaveArchive` | Saves `disableAll` (`+D0h`), `disableEntity`, `waitingQueue` (`+E4h`), `disableList` (`+F0h`), `luaListeners` (`+F8h`), `lastUse`, `shipNoiseInfo` (`+184h`). The load side is `00980380`. |
| `00976B00` | `BSP_Warning_AddMessage` | Pushes {`00CF0DD0`, `00705E00(name, 0)`, `007018A0(local side)`} into the vector at `+14h`. Ten callers in `0097F490..00980070`, plus `00976B60`. |
| `00976CB0` | `BSP_WarningManager_ReportFirstString` | Guards checked before the lock, then a MessageWarning from element 0 of a string vector. Callers `0097B960`, `008D08B0`. |
| `00976DB0` | `BSP_StringListWarning_Activate` | Vtable `00D1B4F8` slot 3. Sets `state = 1`, `validityRefTime = clock`, and resolves each string at `+70h`. |
| `009762A0` | `BSP_StringListWarning_Destroy` | Destroys the string vector at `+70h`, then `0096F170`. |

`00976B60` has no Ghidra function and no reference (`callsite_census`). It calls `00976B00` unless
the entity is non-null and neutral (`00803CE0` == 2). Left unnamed.

Triaged as container plumbing (STL map/list nodes, equal_range, destructors), not reconstructed:
`0096C1D0..0096CDC0`, `0096D5C0..0096DAE0`, `0096DC00`, `0096DCB0`, `00975B40..00975F90`,
`00976190`, `00976320..009763A0`, `00976BA0..00976C20`. `0096D2B0` (listener `callback` save) is
in cc9's listener plumbing and was not touched.

## Where `airRaidSoundExpires` and `collisionSoundDisableTime` land

There is one string reference to each key in each direction (`scan-bytes`: `00D1B000` at
`0096E307` and `00980B4B`, and `00D1AFEC` at `0096E345` and `00980B9B`). The keys belong to the
**per-ship record of the manager map at `+184h`** (`00975D00` = find or insert), which is saved as
the table `shipNoiseInfo`:

| Record | Key | Writer | Meaning |
| --- | --- | --- | --- |
| `+0h` | `collisionSoundDisableTime` | `00977820`: when clock > `+0h`, sets `+0h` = clock + 30.0 (`00CE7630`) and fires a one-shot point effect from manager `+190h` | collision sound re-arm time |
| `+4h` | `airRaidSoundExpires` | `00977990` (cc9's `ScanProximity`): holds 2.0 s (`00D7A308`) | air-raid sound deadline |
| `+8h` | effect handle | `0096C070` / `00867B10` | the live air-raid effect |

The save (`0096E2C0..0096E352`) names each sub-table after the ship's word `+174h` and writes
`+0h`. It writes `+4h` only when `+8h` is set (`NEG/SBB/TEST` at `0096E314`). The load
(`00980ABC..00980BF0`) maps the handle back to the entity through the tables at
`00F89A54`/`00F89AA8`. It reads `+0h`, then `+4h` with a default of -1.0e10 (`00CE4ADC`), and calls
`0096C070(record, value)` when the value is not the default. `include/bsp/warning_reports_cc13.hpp`
has `ShipNoiseInfo`. This corrects the old `+174h..+180h` attribution. Dated notes were appended to
`docs/WARNING_MANAGER.md` and `docs/MISSION_EVENTS_UPDATE.md`.

## RNG

None of these builders draws a random number. `local/wn1_rng.py`, a rel32 E8/E9 plus
absolute-dword census for `00BD2F10` and `00BD2FC0`, finds zero hits over `0096C000..0096F100`,
`00973A90..00979730`, `0097B400..0097B9C0` and `00980380..00980E50`. Its positive control,
`0042C1B0`, gives 4 hits. Callees outside those ranges (`00705E00`, `007018A0`, `00682990`,
`BSP_PointEffect_*`) were not followed. If radio chatter picks among voice variants, the choice is
made after the id is queued, in the message-record or sound layer, not here.

## Handoff

Segment-64 report builders are done; nothing is wired. Open items:

1. Host wiring. A host would call `construct_hit_warning_00974c70` from the hit channel and
   `construct_kill_warning_009744c0` from the kill channel. Both channels are cc9's (`00988510`,
   `009813A0`). It would also call the three capture/command reports from `006F29D0`/`006F6760`/
   `0074AF50`, `006F4D10` and `00777850`. These are all multiplayer or ship-capture paths.
2. `00980380` is still named `BSP_WarningManager_LoadEventTable`, but it is the manager's archive
   load, the mirror of `0096DD30`. It was outside this packet's lease; rename it with its owner.
3. `include/bsp/mission_events.hpp` still names `air_raid_*`/`collision_*` fields after the
   `+174h..+180h` attribution. The owner of that header should move them to a `+184h` per-ship
   record (`ShipNoiseInfo` here).
4. `00976B60` has neither a Ghidra function nor a reference. It filters `00976B00` on a non-neutral
   entity. Leave it undefined unless a caller turns up.
5. The vtable `+108h` of the hit's `+4h` object (the attacker resolver) and the 7Ch string-list
   warning (`009795B0`, built by `00979670`) were not traced further.
