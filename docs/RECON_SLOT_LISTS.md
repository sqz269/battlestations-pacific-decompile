# The per-party recon slots and how their unit lists are rebuilt

Packet `cc2_recon_slot_lists`. Addresses: `008073C0`, `008042B0`, `00804150`, `00804E10`,
`00804C30`, `008040E0`, `00804D20`, `00804E70`, `00805430`, `00805490`, `00805680`, `008048A0`,
`00805AF0`, `00805BE0`, `00805D90`, `00805F60`, `00806480`, `00806840`, `008065B0`, `008069A0`,
`00806A60`, `00806B10`, `006B8390`, `008053C0`, `00803BA0`, `00805240`.

This packet takes the three follow-ups `docs/FIXED_STEP_COUNTDOWN.md` left open
(`recon_slot_rebuild`, `recon_report_values`, `recon_slot_indices`). It answers what fills the
class arrays, what the five triples separate, how a unit gets into a party's recon list, what the
Lua table contains, and what the three indices are.

## 1. The membership rule

A unit appears in party `P`'s recon list when all of the following hold. There is no line-of-sight
trace and no ray cast anywhere in the path; "visibility" is a per-observer table lookup on a
horizontal distance and an optional bearing window.

**(a) The unit's class is scanned.** `00806480` returns a lazily built singleton (`00F874C8`,
constructed by `00805F60`) whose first vector holds twenty-two class ids. The scan visits the world
registry's per-class list for each, and only for each: a class absent from this vector never enters
any slot.

| id | class | id | class | id | class |
| --- | --- | --- | --- | --- | --- |
| `35h` | unnamed, parent `05` | `0Eh` | `MTorpedoBoat` | `16h` | `MLargeReconPlane` |
| `07h` | `MDestroyer` | `13h` | `MPlaneFighter` | `17h` | `MPlaneKamikaze` |
| `0Ah` | `MCruiser` | `10h` | `MPlaneBomber` | `19h` | `MLandVehicle` |
| `0Bh` | `MCargo` | `12h` | `MPlaneDiveBomber` | `1Bh` | `MLandFort` |
| `0Ch` | `MLandingShip` | `11h` | `MPlaneTorpedoBomber` | `45h` | `MAirfield` |
| `0Dh` | `MBattleship` | `15h` | `MSmallReconPlane` | `46h` | `MShipyard` |
| `08h` | `MSubmarine` | `09h` | `MMothership` | `2Bh` | `MTorpedo` |
| | | | | `34h` | `MWaterMine` |

The singleton's **second** vector holds `18h` `PlaneSquadronGen` and `1Ah` `LandConvoy`. Those two
are never scanned; the grouping passes create their entries. `008073C0` retires them with two
literal `00805430` calls (`00807921`, `0080792A`) rather than reading that vector.

**(b) The unit passes the four gate bytes and one class test** (`008074D5`..`008074FA`):
`+5Ch` set (the live gate), `+5Dh`, `+60h` and `+5Eh` all clear, then `vtable[5Ch](2)` true, so the
unit must be `IsKindOf(02h)`, the base of every scene entity including `PlaneSquadronGen` and
`LandConvoy`.

**(c) Its relation to the slot decides which of three arrays it lands in.** `008065B0` computes the
relation twice with identical code (`008065FF`..`0080672B` for a new unit, `00806636`..`00806662`
for one carried over) from the slot's own index at `+28h` and the unit's party at `+54h`:

```
relation(slotIndex, unitParty):
    if slotIndex == 2:                       # 00806721, its own branch
        return unitParty == 2 ? 0 : 2
    if slotIndex == unitParty: return 0      # own      (array +34h)
    return unitParty == 2 ? 2 : 1            # neutral / enemy
```

Slot 2 is not the general rule applied to index 2: for that slot **nothing is ever enemy**, every
non-party-2 unit is neutral. The party value 2 is the neutral side, and `+54h`'s construction
default is 2 (`docs/UNIT_INSTANCE_LAYOUT.md`).

Membership in the three class arrays therefore depends on nothing but party. **Detection decides
what is published, not what is a member.**

**(d) Detection raises the entry's level, and the level decides the published list.** Each entry is
a `1Ch`-byte record whose `+0Ch` carries a level of 0, 1 or 2. The record is created with level 2
for the own relation and level 0 otherwise (`00804E70`'s second argument, `2` at `008067DB` and `0`
at `0080679D`/`0080675C`). `00806840` then runs a sensor pass over the enemy and neutral arrays and
writes each record's `+0Ch` from the target's own detection record (`0080695C`). The drain at
`00807644` and `008077C2` keeps only level 2 in the enemy and neutral triples, copies level 1 into
the "unknown" triple, and drops level 0 entirely. The own triple is never drained.

**(e) The detection level itself** comes from a float accumulator on the target, per party:
`target + 1E8h + party * 34h`, `+0Ch` the value, `+4h` the level. `00805AF0` clamps the value to
`[0, 1.0]` (`00D7A24C`) and thresholds it: below `0.25` (`00CE3868`) level 0, below `0.5`
(`00CE3800`) level 1, otherwise level 2. An own-relation unit is forced to the top every pass with
`00805AF0(1.0f, 1, nullptr)` (`008067FA`, `008066E1`). A target no observer saw this pass is reset
to zero by `00805BE0`: detection does **not** decay, it snaps off.

**(f) The sensor evaluation, `008048A0`**, `__fastcall bool(ECX = observer, EDX = target, int mask,
float dt, float* outGain)`, `RET 0Ch`. The rebuild calls it with `mask = 0FFh` and
`dt = slot->+30h`, the seconds since the slot's last rebuild:

1. the observer must have a sensor block: `[[observer+538h]+B4h]` non-null, else false;
2. `signature` = `[[target+538h]+B8h]` when the target is `IsKindOf(05h)`, else `1.0f`
   (`00D7A24C`);
3. `env` = gameplay-modifier category `0Ch` for the observer (`008E6430
   BSP_GameplayModifiers_ProductForUnit`) when `[00E0C978]` and `[[00F88C30]+118h]` are both set,
   else `1.0f`;
4. `rangeScale = env * env * signature * tuning74 * tuning74`, with `tuning74 = [[game+21C4h]+74h]`;
5. a target that is `IsKindOf(08h)` (`MSubmarine`) and for which `00922DC0
   BSP_Entity_IsSurfaceTarget` is false multiplies `rangeScale` by `[[game+21C4h]+78h]` squared:
   the submerged-submarine range penalty;
6. `distSq` is **horizontal only**, `(dx² + dz²)` from the two world frames' `+FCh` and `+104h`;
   height is never read;
7. `normalized = distSq / rangeScale`;
8. the sensor row is `[[observer+538h]+B4h] + 8 + (observerCategory * 8 + targetCategory) * 0Ch`,
   a `{begin, count}` pair of `1Ch`-byte entries. Both categories come from the units' `+1E4h`
   sub-object, `vtable[1]()`;
9. each entry applies when `normalized <= entry+4h`, when `mask` carries bit `entry+10h`, and, if
   `entry+18h` is set, when `|SubtractWrappedAngle(atan2(dx, dz), observer->vtable[50h]())| <=
   entry+14h` (`00438B10 BSP_Math_SubtractWrappedAngle`, `RET 8`);
10. an applying entry contributes `gain = entry+8h * dt`, reduced so that the target's current
    accumulated value plus the gain does not pass `entry+0Ch`. The routine returns whether any
    entry applied and writes the **largest** gain.

`00806840` keeps the best gain over the whole own triple and the observer that produced it, then
calls `00805AF0(bestGain, 1, bestObserver)` on the target, or `00805BE0` when nothing applied.
Only own-triple entries that are `IsKindOf(05h)` act as observers (`008068C2`).

Two global gates skip the sensor pass for a target and leave its level alone: the session-kind word
`[00E188A8]+1FE4h == 2` (`00806871`) and the target's own force byte
`det[slot]+10h` (`00806883`).

The accumulator the gain is compared against is indexed by the **observer's** `+54h`
(`00804B2F`), not by the slot index. Inside the rebuild the two coincide, because the observers
come from the slot's own-relation triple.

## 2. The slot layout

`0x12A0` bytes. Everything below has a producer in this packet's addresses. The first three
arrays, the size, the index and the `+25h` byte are already in `docs/FIXED_STEP_COUNTDOWN.md`;
`0xE14 + 61h * 0Ch == 0x12A0`, so the object ends exactly at the fourth array.

| offset | size | what | producer |
| --- | --- | --- | --- |
| `+0h` | 4 | vptr `00D08E94` | `008050E0`, `00805265` |
| `+14h` | 4 | vptr `00D08E88`, a second base; `00695760` detaches it | `0080526B`, `0080538F` |
| `+24h` | 1 | contents-changed flag | set `00806818`, cleared `008073E6` and `0080799C` |
| `+25h` | 1 | publish-dirty flag | set `00803BAB`, read/cleared by `008079B0` |
| `+28h` | 4 | the slot's index | `008053C0` through `008050E0`; read `008065FF`, `00806B3C`, `00805272` |
| `+2Ch` | 4 | float, time of this rebuild | `008073E1` from `00F876A4` |
| `+30h` | 4 | float, seconds since the last rebuild | `008073D6`; read `008068CD` as the sensor `dt` |
| `+34h` | `61h * 0Ch` | array A, per-class lists, relation **own** | `008067EA` appends, `00805240` destroys |
| `+4C0h` | `61h * 0Ch` | array B, relation **enemy** | `008067AC` |
| `+94Ch` | `61h * 0Ch` | array C, relation **neutral** | `0080676B` |
| `+DD8h` | `0Ch` | triple 0, published as `own` | `00807537`, `008078F4`, `00807919` |
| `+DE4h` | `0Ch` | triple 1, published as `enemy`; head at `+DE8h` | `00807620`, drained `00807634` |
| `+DF0h` | `0Ch` | triple 2, published as `neutral` | `008077A0`, drained `008077B4` |
| `+DFCh` | `0Ch` | triple 3, published as `unknown` | `0080769B`, `00807819` |
| `+E08h` | `0Ch` | triple 4, the union of 0..3, **not published to Lua** | `00807933`..`00807966` |
| `+E14h` | `61h * 0Ch` | array D, the per-class **carry-over** lists | `00804150` fills, `008040E0` drains, `00805430` retires |

Every list is `{count, head, tail}` and every node is `{prev, next, payload}` (`00804C30`,
`008042B0`, `00804E10`). A payload is the `1Ch`-byte entry record:

| offset | what | producer |
| --- | --- | --- |
| `+0h` | vptr `00D08E78` | `00804E8B` |
| `+4h` | the unit | `00804EB3` |
| `+8h` | the unit's category, `unit->[+1E4h]->vtable[1]()`, refreshed each pass | `00804EC1`, `00806633` |
| `+0Ch` | the detection level, 0/1/2 | `00804EAA` at creation, `00806981` each pass |
| `+10h`/`+14h`/`+18h` | a `{count, head, tail}` member list, used only by group records | `00804E95`, `008055E4` |

The per-unit detection record, `unit + 1E8h + party * 34h`, three of them (`00779B4E` iterates
three), `34h` each:

| offset | what | producer |
| --- | --- | --- |
| `+4h` | level 0/1/2 | `00805B73`, `00805BFE`, `00806A01` |
| `+8h` | forced level | read `00805AFA`, `008069F6` |
| `+0Ch` | accumulated float, clamped to `[0, 1.0]` | `00805B10`, `00805B48`, `00805BF9` |
| `+10h` | force byte; when set, `+8h` replaces `+4h` everywhere | read `00805AF4`, `00806883` |
| `+14h`..`+2Bh` | an observer endpoint; `+28h` holds the detecting unit | `00805B76`..`00805B93` |
| `+2Ch` | change-delegate object, `vtable[0](context, oldLevel, newLevel)` | `00805BB4` |
| `+30h` | the delegate's context | `00805BBC` |

`00694A60 BSP_Observer_RegisterPair` and `006952A0 BSP_Observer_UnregisterPair`
(`docs/OBSERVER_LIFETIME.md`) are the registration; they are not re-derived here.

## 3. The rebuild, `008073C0`, in order

`__thiscall void(ReconSlot*)`, body `008073C0`-`008079A5`, `RET`.

1. `008073C1`..`008073E6`: `+30h = [00F876A4] - +2Ch`; `+2Ch = [00F876A4]`; `+24h = 0`.
2. `008073D0`..`00807416`: clear all five triples through `008042B0`.
3. `0080741B`..`00807460`: for each of the `61h` class buckets, splice A, then B, then C into
   D through `00804150`. The `1Ch` records survive; the three relation arrays are now empty and D
   holds last pass's population.
4. `00807462`..`0080749A`: reset this slot's detection record on every unit of the world list at
   `[[game+19CCh]+13Ch]` through `00805BE0`.
5. `0080749C`..`00807527`: the scan. For each of the twenty-two class ids, walk the world
   registry's list at `[game+19CCh] + 18h + id * 0Ch`, apply the gate of §1(b), and call
   `008065B0(id, unit)`. After each class, `00805430(id)` retires whatever is still in `D[id]`.
6. `00807529`..`00807545`: triple 0 takes a copy of every own bucket. This happens **before** the
   own grouping, so triple 0 carries the individual planes and land vehicles too.
7. `00807547`..`0080757F`: `00806840` over every enemy bucket, then every neutral bucket, with
   triple 0 as the observer list.
8. `00807581`..`00807610`: enemy grouping. Seven `00805490` calls fold `10h`, `13h`, `12h`, `11h`,
   `16h`, `15h`, `17h` into `18h`; `008069A0(B[18h])` publishes the squadron's level;
   `00805680(B[19h], B[1Ah])` then `00806A60(B[1Ah])` do the same for convoys.
9. `00807615`..`008076F3`: triple 1 takes every enemy bucket, then is drained by level.
10. `008076F9`..`00807871`: the same three steps for neutral, into triple 2.
11. `00807877`..`0080791C`: own grouping, then triple 0 takes `A[18h]` and `A[1Ah]`.
12. `00807921`..`0080792E`: `00805430(18h)` and `00805430(1Ah)`.
13. `00807933`..`00807966`: triple 4 takes copies of triples 0, 1, 2, 3.
14. `0080796B`..`0080799C`: when `+24h` is set, and `[game+18ECh]` is in `[0, 7]`, and
    `[game+18CCh + idx*4]->+28h` equals this slot's index, clear `[game+193Ch]`; then clear `+24h`.

### `008065B0`, the add or refresh

`__thiscall void(int classId, Unit* unit)`, `RET 8`, body `008065B0`-`00806835`.

Search `D[classId]` for a record whose `+4h` is the unit.

* **found**: refresh `+8h` from `unit->[+1E4h]->vtable[1]()`, compute the relation, and splice the
  node out of `D[classId]` into `A|B|C[classId]` with `008040E0`. The level at `+0Ch` is left alone.
* **not found**: `operator new(1Ch)`, construct with `00804E70(unit, relation == own ? 2 : 0)`,
  append to `A|B|C[classId]` with `00804C30`, set the slot's `+24h`, and register the pair with
  `00694A60(unit, slot)`.
* the **own** relation additionally calls `00805AF0(1.0f, 1, nullptr)` on the unit's detection
  record for this slot, on both paths.

### `00805430`, the retire

`__thiscall void(int classId)`, `RET 4`. Everything still in `D[classId]` was not seen this pass.
For each, `00694AF0(unit, slot)` gates `006952A0(unit, slot)`; then `00804D20` destroys the list,
which destroys the `1Ch` records through their vtable.

### `00805490` and `00805680`, the grouping

`__thiscall void(List* memberClassList, List* groupClassList)`, `RET 8`. A member with level `>= 1`
and a non-null group back pointer (`+9D4h` for a plane's squadron, `+738h` for a land vehicle's
convoy) is folded into the group's record: the group record is taken out of `D[18h]`/`D[1Ah]` if it
is there (and its level reset to 0 and its member list cleared), found in the destination list, or
created; the member record is appended to the group record's `+10h` member list; and the group's
level becomes the maximum of its members'. The member records stay where they are.

`008069A0` recomputes that maximum from the member list and writes it to the group unit's own
detection record; `00806A60` writes the group record's stored level instead. Both fire the
record's change delegate when the effective level moved.

## 4. What `00806B10` publishes

`__thiscall void(LuaInstance*)`, `RET 4`, body `00806B10`-`00806CCE`. Two passes over
`recon[slot->+28h]`.

The first pass (`00806B30`..`00806B9D`) sets `enemy`, `own`, `neutral`, `unknown` to nil through
`006B8390` (`lua_pushstring`, `lua_pushnil`, `lua_settable` at `-3`). The second
(`00806BA2`..`00806CB7`) refills them:

| Lua key | string | triple | `00805D90` call |
| --- | --- | --- | --- |
| `enemy` | `00D08E64` | 1, `+DE4h` | `00806BE4` |
| `own` | `00D08E60` | 0, `+DD8h` | `00806C1A` |
| `neutral` | `00CE5604` | 2, `+DF0h` | `00806C50` |
| `unknown` | `00CEB808` | 3, `+DFCh` | `00806C86` |

Triple 4 is not published.

`00805D90`, `__thiscall void(LuaInstance*, List* triple, int)`, `RET 0Ch`, body
`00805D90`-`00805F29`, does the work:

1. constructs nineteen local `{count, head, tail}` lists (`00BF7CD1` with `0Ch`, `13h`);
2. walks the triple. A unit whose `+C4h` class id is one of `0Fh`, `10h`, `11h`, `12h`, `14h`,
   `15h`, `16h`, `17h`, `13h`, `19h` is **skipped**: individual planes and land vehicles never
   reach the script, only their squadron and convoy aggregates stand for them. Otherwise the
   category is `unit->[+170h]->vtable[0]()`; `13h` means "no category" and also skips; anything
   else buckets the record;
3. for each of the nineteen categories in the order of the pointer table `00E0B590`..`00E0B5DB`
   (`docs/RECON_VALUES.md` names them), descends into that named table and, for every record,
   writes `table[itoa(unit->+174h, 10)] = <the unit's Lua value>` through `006B8120`, `00927BF0`
   and `006B84D0(-3)`.

So the nineteen category tables are **neither counts nor arrays**. Each is a map from the unit's
decimal entity id, as a string key, to the unit's Lua object:

```
recon[index][relation][category][tostring(entityId)] = entity
```

The third argument to `00805D90` (`0` for `own`, `1` for the other three) is **never read**; the
body has no access to `[ESP+0x138]` other than the exception-state write at `00805EFF`.

## 5. The three indices

The index is the **party (side) index**, and there are exactly three parties: 0 and 1, the two
belligerents, and 2, the neutral side.

* `004E0535`..`004E0544` in `BSP_Game_LoadMissionScene` creates all three with
  `for (i = 0; i < 3; ++i) 008053C0(i)`, then arms an immediate refresh with `00807A50`.
* `009F5D3A` calls `008053C0` with `[[state+14h]+54h]`, the owning unit's party
  (`docs/BOT_FIRE_TARGET.md`).
* `008065FF` compares the slot's `+28h` against the unit's `+54h` to derive the relation, and
  `00806840`/`008048A0` index the unit's three-entry detection array with the **observer's** `+54h`
  while `008065B0` indexes the same array with the **slot's** `+28h`. The two are the same
  quantity.
* `00779B26`..`00779B51` and `0077D29C`..`0077D2BB` walk a unit's three detection records
  (`+1F0h`, stride `34h`, `i < 3`) and call `00803BA0(i)` for each whose effective level is above
  zero: a unit that dies marks every context that could see it.

A local player's slot record caches its context: `004E0575`..`004E0588` and `004C9E5E`..`004C9E6F`
call `008053C0([localPlayerSlot+28h])` and store the result in `localPlayerSlot+30h`. The active
player is `[game+18ECh]` into `[game+18CCh]`.

`008053C0` is `__fastcall void*(int index)`: it returns the slot in `EAX` on both paths
(`00805414` reaches the epilogue with either the existing pointer or the one just stored).

`00803BA0`, `__fastcall void(int index)`, sets `slot->+25h` and, when the index is the active
local player's context, clears `[game+193Ch]` — the same tail the rebuild ends with.

## 6. Cadence

| trigger | site | what |
| --- | --- | --- |
| the 3-second countdown | `00807A08` in `008079B0` | rebuild every present slot; publish only the ones whose `+25h` is set |
| mission scene load | `004E059B` in `BSP_Game_LoadMissionScene` | create all three slots, `00807A50` to arm, then rebuild the active player's slot at once |
| in-game interface applied | `004C9E82` in `BSP_Game_ApplyInGameInterface` | resolve the local player's slot, rebuild it |
| `007C631C` in `FUN_007C5F60` | | rebuild the active player's slot |
| unit death | `00779B43`, `0077D2B6` | `00803BA0(i)` per context that saw it: publish-dirty only, no rebuild |

The rebuild runs on every period for every present slot; the `+25h` byte gates only the Lua
publish. `docs/MISSION_SCENE_LOAD.md` line 185 and `docs/MISSION_LOAD_PATH.md` line 153 already name
the load-time pair; both describe the `004E0583`/`004E059B` sites above, and
`docs/MISSION_LOAD_PATH.md` calls it "view priming", which this packet narrows to "resolve the local
player's recon context and rebuild it".

## 7. Coverage

| routine | reconstruction | coverage |
| --- | --- | --- |
| `008073C0` | `rebuild_recon_slot_lists_008073c0` | complete |
| `008065B0` | `recon_relation_for_008065ff`, host method | complete |
| `008048A0` | `recon_evaluate_sensor_row_008048a0` and the four scalar rules | complete for the arithmetic; the two vtable category calls and `008E6430` are host calls |
| `00806840` | host method | complete as a reading; not projected |
| `00805AF0`, `00805BE0` | `recon_detection_accumulate_00805af0`, `recon_detection_level_00805b3d` | complete |
| `00805490`, `00805680` | `recon_member_groups_008054d7`, host methods | complete as a reading; the list surgery is host-side |
| `00805D90` | `recon_publish_excludes_class_00805de6` | complete as a reading; the Lua calls are not projected |
| `00805F60` | `kReconScannedClassIds`, `kReconAggregateClassIds` | complete for both vectors |
| `00806B10` | none, table above | complete |
| `00805240` | none, layout table above | complete |
| `008042B0`, `00804150`, `00804E10`, `00804C30`, `008040E0`, `00804D20`, `00804E70` | none | complete as readings |
| `00885A70` | none | **not on this path**: its only callers are `00885DA0` and itself, and `00806B10` never reaches it |

## 8. Corrections

| document | was | is | evidence |
| --- | --- | --- | --- |
| `docs/FIXED_STEP_COUNTDOWN.md` | "three arrays of `61h` elements of `0Ch` bytes at `+34h`, `+4C0h` and `+94Ch`" | **four**. The fourth is at `+E14h`, the carry-over array | `00805300`..`0080530C` destroys it with the same `{ptr, 0Ch, 61h, 00804E00}` array helper call as the other three, and `0xE14 + 61h * 0Ch == 0x12A0`, the whole object |
| `docs/FIXED_STEP_COUNTDOWN.md` | "`008073C0` clears all five at `008073D0..00807416`", with `008042B0` unread | correct, but for a reason the listing tool hid: `008042B0` is a **loop**, not a single pop | `bsp.py ghidra disasm` omits `008042EC: ADD ESP,4 / 008042EF: CMP [ESI],0 / 008042F2: JNZ 008042B8`; the disk bytes `83 C4 04 83 3E 00 75 C4` at `008042EC` carry them |
| `docs/FIXED_STEP_COUNTDOWN.md` | "the arrays are plausibly per-class unit lists and the three of them plausibly the relations the report names. **Uncertain** ... the report has four relations against three arrays" | settled. The arrays are indexed by class id and are exactly the three relations; the fourth Lua relation, `unknown`, is not an array but triple 3, drained out of the enemy and neutral triples by detection level | `008067EA`/`008067AC`/`0080676B` select the arrays from the relation; `00806B10` maps `unknown` to `+DFCh`, which only `0080769B` and `00807819` fill |
| `docs/FIXED_STEP_COUNTDOWN.md` | the five triples are "the same five lists `004C3CB0` walks" | the five triples are `own`, `enemy`, `neutral`, `unknown` and their union; the fifth is native-only | `00806B10`'s four `00805D90` calls; `00807933`..`00807966` builds the fifth from the other four and nothing publishes it |
| `docs/LOCAL_PLAYER_UNIT_LISTS.md` line 267 and the `unit_registry_five_lists` follow-up at line 289 | `008073C0` coverage "partial: only the five `008042B0` clears and the list-1-to-list-3 move were read"; what the five triples separate was the open question | answered and complete. The triples are the four Lua relations and their union; the 1-to-3 move is the detection drain of §1(d) | the whole body is read above; §3 lists every step and §4 the Lua mapping. That doc's reading that `004C3CB0` walks triples 0, 1 and 3 and the HUD triple 4 is consistent and not superseded |
| `docs/FIXED_STEP_COUNTDOWN.md` | `008053C0` row read `__fastcall void(int index)`, corrected in that doc's tail to return the slot | confirmed from the body: both paths leave the slot pointer in `EAX` | `008053D9` and `0080540D`, then the shared epilogue at `00805414` |
| packet brief | `00885A70` belongs with `00806B10` in `recon_report_values` | it does not. `00806B10` reaches `006B8190`, `00803750`, `006B8390`, `008037D0`, `00805D90`, `00A673E0`, `006B8210` and nothing else | `bsp.py ghidra callers 00885A70` returns `00885DA0 BSP_MissionLua_PushArgumentRecord` and itself |

## 9. `no_ghidra_function`

None. Every routine named in this document has a Ghidra function.

## 10. Open questions

* `[[game+19CCh]+13Ch]`, the world list step 4 resets, is read but not identified. Every unit on it
  loses its detection for the slot before the scan; which units are on it is unread.
* `[game+21C4h]`, the tuning object supplying `+74h` and `+78h`, is unread beyond those two floats.
* The observer and target **categories** (`unit->[+1E4h]->vtable[1]()`, 0..7) that index the sensor
  table are unread; only their use is established. They are not the `+170h` category the publish
  uses, which runs 0..18.
* `00694AF0`, the predicate gating the retire notification, is unread here
  (`docs/OBJECTIVE_UNIT_LIST.md` touches it).
* No run-time evidence: `bsp_game.exe` fakes this list (`docs/GAME_EXECUTABLE.md` line ~4806), so
  no path it runs reaches `008073C0`.

## Correction from docs/SENSOR_TABLES.md (packet cc2_sensor_tables)

- **Was:** the categories run 0..7 and the table is observerCategory * 8 + targetCategory
  **Is:** the stride is 8 but the record holds seven groups; the categories run 0..6 and column 7 is allocated and unreachable
  **Evidence:** 00808CE4 PUSH 7 with element size 0x60 into a 0x2a8 allocation (8 + 7 * 0x60 = 0x2a8); the six Lua key names map to 0..5 and 004F1740 returns 6
- **Was:** entry +0Ch is a clamp the gain is reduced against
  **Is:** it is one of exactly three values derived from the script's MaxLevel: 0 -> 0.0f, 1 -> 0.25f, 2 (the default) -> 1.0f, which are the published-level thresholds
  **Evidence:** 0080865A PUSH 2 into 00B66380, then 0080866B-00808685 with 00CE3868 and 00D7A24C

## Correction from docs/GAMEPLAY_LOOSE_ENDS_1.md (packet cc2_gameplay_loose_ends_1)

- **Was:** The world list step 4 resets is read but not identified; which units are on it is unread.
  **Is:** It is the class-id-24 list and its members are plane squadrons, not units. Step 4 therefore resets one detection record per squadron, and 00807480's LEA ECX,[ECX + EAX*0x34 + 0x1E8] indexes a squadron's record block.
  **Evidence:** The same arithmetic and insert as the docs/SENSOR_TABLES.md correction above. 006FE620 BSP_UnitInstance_RegisterInWorldLists, the unit's implementation of the same +130h virtual at 00CFC3D0+130h, joins ids 2, 4, 5, 6 and 7 only (ADD ECX,0x30/0x48/0x54/0x60/0x6C), never id 24.
