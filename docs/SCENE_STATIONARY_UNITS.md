# The stationary unit path, 004F0BE0 and 00748C40

Addresses: 004F0BE0, 00851CB0, 00851B40, 004F04C0, 004F0FB0, 00748C40, 00748A40, 00745940,
008F2260, 008F0DF0, 00964790.

Evidence: the image read with `disasm-raw` for 004F0BE0, the decompilations of 00851CB0, 004F0FB0
and 00748C40, and the listings of 00748A40 and 00745940. The observer-pair rule in section 5 is
proven against a class that already has a row before it is applied to one that does not.

## 1. Ghidra does not bound 004F0BE0 either

`lookup` answers "no function starts here", with `BSP_SceneUnit_CreatePlaneSquadronGen`
(`004F0AD0`) named as the enclosing candidate. That is the same answer `004E5B00` gave in the
previous packet, and the two cases are opposite:

```
004f0bdd: ret 0x10                      the previous function ends
004f0be0: mov eax, dword ptr fs:[0]     a full SEH prologue starts here
```

No `INT3` padding, a real prologue: `004F0BE0` is a genuine function that Ghidra has merged into
its neighbour. `004E5B00` had padding on both sides and one byte of body. The padding is what
separates the two readings, which is why the check is worth making every time.

## 2. What 004F0BE0 does

```
004F0BFE  PUSH 00CE4780 "Type" / CALL 008F2260      the bag lookup
004F0C0E  JE  004F0CAB                              absent: nothing is created
004F0C17  CMP [EAX+4],4 / 004F0C1C CMP [EAX+4],2    the property must be type 4 or 2
004F0C28  CALL 008F0DF0                             the type's text
004F0C43  CALL 00851CB0                             the stationary class, by name
004F0C4F  CALL [vtable+4]                           that class allocates the instance
004F0C88  CALL 004F04C0                             the shared scene-unit setup
```

It resolves its type **by text**, not by a numeric class id, and it hands that text to a factory
that is not `00964790`.

## 3. 00851CB0 is the stationary class factory

```
piVar2 = FUN_00851B40(name)          a cache; a hit returns the existing class
BSP_LuaStateOwner_GetGlobals(...)
BSP_LuaObject_GetByName("StationaryClass")
BSP_LuaObject_GetByNativeString(name)
operator new(0x3C)                    a 3Ch-byte class built from that row
```

So a stationary class is a row of the **`StationaryClass`** Lua global, cached and wrapped in a
3Ch-byte descriptor. `VehicleClass` is a different table with a different factory
(`00964790`, 22 `Type` literals, 5Ch-byte kinds). This is the parallel the previous packet said
must exist, and it is why this installation's `stationaryclasses.lua` defines 220 rows under
`StationaryClass` without a single `Type` field: the field would have no reader.

## 4. But Ormoc Bay's units are LandFort rows, not Stationary rows

The USN22 scene types are `LandFort` with the type token naming a `StationaryTypes` symbol:

```
scene type LandFort StationaryTypes:Stat_kate=595 party=Japanese(1) x3
scene type LandFort StationaryTypes:Stat_zero=602 party=Japanese(1) x7
```

Those go to `004F0FB0 BSP_SceneUnit_CreateLandFort`, which has its own split, already recorded in
`docs/SCENE_UNIT_CREATORS.md`: it tests the scene property `Stationary` and, when it is set and its
value byte is non-zero, takes the instance from **`00748C40`** instead of the descriptor's
vtable+28h. `00964790` is called on both arms, and on the stationary arm its answer is not used to
allocate.

`00748C40` is `operator new(1ACh)` then `00748A40`, a separate 1ACh-byte class. A fort is 758h
bytes; the prop is a third of that, which is how 72,615 authored LandFort entities stay affordable.

## 5. The prop's observer tables, and the rule that names them

`00748A40` stores three vptrs:

```
00748A64  MOV [ESI],0xcff678
00748A6A  MOV [ESI+0x10],0xcff65c
00748A71  MOV [ESI+0x24],0xcff654
```

The rule for which of those are the observer pair is not assumed here; it is checked against a
class that already has a row. `00745940`, the LandFort constructor, stores:

```
0074597D  MOV [ESI],0xcff3f8
00745983  MOV [ESI+0x10],0xcff3e0
...
007459A5  MOV [ESI+0x310],0xcff3b4
```

and LandFort's row in `src/native_unit_observer_endpoint.cpp` is
`{0x00747000, 0x00cff3f8, 0x00cff3e0}`, with `0x00cff3b4` its tick vtable in the motion-dispatch
table. So the observed table is the vptr at `this` and the callback table the one at `this+10h`,
and the tick vtable is the one at `this+310h`.

Applied to the prop: **observed `00CFF678`, callback `00CFF65C`**. The prop is 1ACh bytes and has
no slot at 310h, so it carries no tick vtable and takes no motion dispatch, which is right for a
static prop.

## 6. What this process was doing wrong

The units host picks both the motion dispatch and the observer tables from
`VehicleClassDescriptorRow::allocate_instance`. A stationary unit has no such descriptor by
construction, so the lookup returned a null descriptor, the creator came out zero, and the observer
publish refused. That is the throw the previous packet made legible.

The fix follows the native's own shape rather than a flag: when the vehicle-class chain answers
nothing and the type's text answers in `StationaryClass`, the unit is a stationary prop and takes
the prop's own pair. `GameMissionLuaHost::stationary_class_exists` is `00851CB0`'s lookup, and
`bsp::publish_stationary_prop_observer_tables_00748a40` is the pair from section 5.

## 7. The DummyTargetVehicle gap

Separately from the stationary path, `src/vehicle_class.cpp` holds 22 `Type` literals and
`src/native_unit_observer_endpoint.cpp` 21 observer rows. The literal with no row, and no
motion-dispatch row either, is `DummyTargetVehicle` (`MDummyTarget`). It is a real gap of the same
shape as the one this packet closes, it would raise the same throw, and no mission examined in this
thread's survey exercises it. It is left open deliberately: its leaf constructor was not read, and
inventing its pair from the pattern is exactly what section 5 exists to avoid.

## Uncertainty

* **Not run.** Section 6's fix is reasoned from section 5's rule, not observed. USN22 should load
  further; whether it loads completely is unknown, because the rest of the prop's construction
  (`0084F7F0`, and `004F04C0`'s shared setup) was not read.
* Whether the type id a `StationaryTypes` token resolves to (595 for `Stat_kate`) collides with an
  unrelated `VehicleClass` row of the same number. The host indexes `VehicleClass` by that id, and
  the two enums are different id spaces. The `StationaryClass` test is by text and so is immune,
  but a false positive on the `VehicleClass` side would take the descriptor arm first.
* What `00851B40`'s cache is keyed on, and what the 3Ch-byte stationary class holds.
* `DummyTargetVehicle`, as section 7 says.

## Host methods

`bsp::publish_stationary_prop_observer_tables_00748a40` and
`GameMissionLuaHost::stationary_class_exists`, with the stationary arm in the units host that uses
them.

## Corrections

Two, applied after this packet's first commit and neither of them a claim about the binary.

* `publish_stationary_prop_observer_tables_00748a40` was added to
  `src/native_unit_observer_endpoint.cpp` and its header. Those are Codex-lineage files and we do
  not edit them. Both are restored to their previous text and the function now lives in
  `src/game_hosts_units.cpp`, beside its only caller, with a comment saying why it is there rather
  than beside the keyed rows it belongs with.
* `004F0BE0` is now defined in Ghidra rather than left for the next reader to rediscover. See
  no_ghidra_function.

Otherwise none: this packet completes the reading the previous one left as its contract, and the
previous packet's retraction stands as written.

## no_ghidra_function

None now. `004F0BE0` had no Ghidra function and was merged into the range of `FUN_004F0AD0`; it was
read from the image, and section 1 gives the check that separates it from a one-byte `RET`. Since
it genuinely starts a function, one is now defined over `004F0BE0`..`004F0CC0`, 224 bytes, ending
at the `RET 10h` at `004F0CBD` with the next SEH prologue at `004F0CC0`. The event is recorded in
`reports/scene_stationary_units_function_definitions.json`.

That also fixes a reporting gap this packet first worked around: `tools/verify_report_calls.py`
requires a call site to lie in some Ghidra function, so before the definition none of section 2's
sites could be checked. All four are ordinary verified rows now.

## Validation

**Not run.** The build is clean and both ctest suites pass. The run is one call:

```
./tools/run_game.ps1 -Log local\usn22_stationary.log -WaitSeconds 2400 -- --frames 3200 `
    --press-start-frame 30 --menu-select USN22 --mission-frames 3000 --mission-frame-seconds 0.05
```

Previously this stopped after four unit registrations with `unit observer creator projection is
unavailable`. It should now pass the unit after `Hangar1` and register the stationary props. What
to read: how many `unit world registration` lines appear before any refusal, and, if one still
comes, whether its message now names a non-zero creator, which would be a different gap from this
one.
