# `SpawnNew` is a queue, and `GGame::OnMove` is the consumer

Addresses: 0094c480 00949750 0094c8f0 0094c490 0094a140 00949300 009483d0 009478b0 00943c00
00949530 00948cc0 00948bb0 00f89b3c 00f876a4 0087f800 00ce74f8 00e0cf74

Packet `cc8_spawn_new_route`, worktree `agent/cc8-spawn-new`, on main `2ec3ed6ef`.
`docs/LUA_BINDING_SPAWN.md` (packet `cc_lua_core`) recovered the binding and the record and left
the consumer open: "Nothing in this packet reads the queue back". This packet reads it back.

Every descriptive name here is a hypothesis. Where this document says CONTRACT or DEVIATION, the
statement is not a reading of the executable and says so.

## 1. The route, end to end

| step | address | what it is |
| --- | --- | --- |
| the binding | `0094C480` | `PUSH ECX` / `MOV ECX,[00F89B3C]` / `CALL 00949750` / `RET`. Zero callers is correct: a Lua binding is reached through the 560-row table at `00E0B7B8`. |
| the parse and enqueue | `00949750` | reads twelve named fields out of ONE Lua table, allocates DCh bytes through `00949530`, links the record onto `manager+4h`. **Creates nothing.** |
| the drain's call site | `004E534F` | inside `BSP_Game_OnMove`, `docs/GAME_ON_MOVE_MAP.md` step 20 "World tick": `MOV ECX,[00F89B3C]` / `FSTP dword [ESP]` / `CALL 0094C8F0`. |
| the thunk | `0094C8F0` | eight bytes: `CALL 0094C490` / `RET 4`. The scaled delta is pushed and **never read** - `0094C490` is `__fastcall(ECX = manager)` and its clock is the world time at `DAT_00F876A4`. |
| the drain | `0094C490` | picks one record per interval, runs the solver, and either completes the record or puts it back. |
| the solver | `0094A140` | `__thiscall(record)`. Builds a candidate frame, offers it to `00949300`, and on refusal rebuilds it through `BSP_Matrix_BuildRotationY` with another angle and tries again. |
| the placement gate | `00949300` | `__thiscall(record, frame, block)`. Per member: compose the member frame, ask `00941D30` whether the placement is legal, and `bVar4 &= result`. Only when EVERY member passes does it call the creator, once. |
| the creator | `009483D0` | `__thiscall(record, frame)`. Creates every member, appends each created entity to the vector at `record+CCh`, and at its tail stores `*(record+C0h) = 1`. |

`0094C490`'s own reads confirm the manager layout independently of `docs/LUA_BINDING_SPAWN.md`:
`+4h` the list sentinel (`0094C501`, `0094C56B`), `+8h` the element count (`0094C4AE CMP dword ptr
[EDI + 0x8],EBX`), `+0Ch` a float stamp (`0094C4EA FADD float ptr [EDI + 0xc]`, written back at
`0094C59D MOVSS dword ptr [EDI + 0xc],XMM0`). The object is 10h bytes, so all four words are
accounted for.

## 2. The drain, instruction by instruction

1. `0094C4AE`: an empty queue returns before the clock is read.
2. `0094C4B8`: returns when `DAT_00E188A8` is null or `game+1FE4h == 2`.
3. `0094C4E4..0094C500`: `if (DAT_00F876A4 < *(float*)(globalConfig + 2DCh) + *(float*)(manager + 0Ch)) return;`
   **One request is attempted per interval**, and `manager+0Ch` is the stamp of the last attempt.
4. `0094C4F7 CMP EAX,2` / `JC 0094C56B`: with fewer than two records the head is taken without
   testing anything about it. With two or more, `0094C508..0094C560` walks the list for the first
   record whose party `record+80h` indexes an ACTIVE party - `game+18CCh + party*4` with `+8h != 0`
   and `+9h == 0` - skipping a negative party at `0094C538`, and falls through to the same head arm
   when the walk runs off the end.
5. `0094C58E CALL 009439B0` with `ECX = EDI` (the manager): the record is erased from the list.
6. `0094C593..0094C59D`: the stamp is written.
7. `0094C5A2 CALL 0094A140` with `ECX = ESI` (the record): the solve-and-create attempt.
8. `0094C5A7 CMP byte ptr [ESI + 0xC0],BL` / `JZ 0094C802`.

### The branch that matters

`record+C0h` is the **fulfilled** flag. `docs/LUA_BINDING_SPAWN.md` recorded it as "a byte, always
zero", which is true of the constructor and not of the record's life:

- **was**: `+C0h`, a byte, always zero (`00948E3B`).
- **is**: the constructor clears it and `009483D0` sets it at its tail; `0094C490` reads it back.
- **evidence**: `009487B9 MOV byte ptr [ESI + 0xC0],1`, the last store in `009483D0` before its
  epilogue, reached only after the whole member loop; and `0094C5A7`, which branches on it.

When it is still clear, `0094C802` runs `MOV ECX,EDI` / `CALL 009478B0`, and `009478B0` is the same
`00943C00` node-allocate-and-link the enqueue uses, with the manager as `this`. So an unsatisfiable
request goes **back on the end of the list and is retried next interval**. It is never dropped and
never answered. `EDI` is still the manager at `0094C802` because the jump at `0094C5AD` bypasses
both `LEA EDI,[ESI + 0xCC]` reassignments (`0094C6BF`, `0094C74B`); that was checked by filtering
the whole listing for `EDI`, not by reading one site.

When it is set, `0094C777` tests the record's completion function pointer and, when non-null, walks
the created-entity vector and calls it once per entity:

```
0094c7ca: MOV ECX,dword ptr [ESI + 0xc8]   ; the context word
0094c7d0: MOV EDX,dword ptr [EDI]          ; *iterator, a created entity
0094c7d2: MOV EAX,dword ptr [ESI + 0xc4]   ; the function pointer
0094c7d8: PUSH ECX
0094c7d9: MOV ECX,dword ptr [ESI + 0x80]   ; the party
0094c7df: CALL EAX
0094c7eb: ADD EDI,0x4
```

Then `00948BB0` destructs the record (it calls `BSP_CallbackOwner_Destroy`) and `00BF65AC` frees it.

### Correction: `+C4h` and `+C8h` are not the party and the player

`include/bsp/lua_binding_spawn.hpp` declares `kSpawnRequestPartyOffset = 0xC4` and
`kSpawnRequestPlayerOffset = 0xC8`.

- **was**: `+C4h` the party, `+C8h` the player.
- **is**: `+C4h` is the completion callback's function pointer and `+C8h` its context word. The
  party is `+80h`.
- **evidence**: `0094C532`/`0094C542` use `record+80h` as the index into `game+18CCh + party*4`,
  and `0094C7D9` passes `record+80h` in ECX to the callback. `0094C777 CMP dword ptr [ESI + 0xC4],0`
  skips the entire completion walk when `+C4h` is null, which a raw party index could not do;
  `0094C7D2` loads `+C4h` into EAX and `0094C7DF` calls it.

The header's `kSpawnRequestScalarDOffset = 0x80` is therefore the party, and `include/bsp/lua_spawn_new.hpp`
carries the corrected names beside the old ones.

## 3. The interval is measured, not assumed

```
0087f7d1: PUSH 0xd0e1f8                    ; "SpawnAttemptDelay"
0087f7e2: CALL 0x00b67800                  ; Globals[key]
0087f7e7: FLD float ptr [0x00ce74f8]       ; the default, bytes cd cc 4c 3f = 0.8f
0087f7fb: CALL 0x00b66330                  ; the number-or-default read
0087f800: FSTP float ptr [ESI + 0x2dc]     ; globalConfig+2DCh
```

`FLD float ptr` settles the width: it is a float32 at `00CE74F8`, not a double. This installation's
`scripts/datatables/globals.lua` line 201 sets `Globals["SpawnAttemptDelay"] = 0.5`, so the drain
attempts one request every half second of world time. `src/native_global_config_load.cpp:81` already
carried `f_SpawnAttemptDelay{...,0x2dc,...,0x3f4ccccd}`, which is an independent confirmation of both
the offset and the default.

At 0.05 s per mission frame, a 3000-frame run is 150 s of mission time, so the eight requests USN04
makes at stage init drain in about four seconds.

## 4. Correction to the range defaults in `docs/LUA_BINDING_SPAWN.md`

That document says `angleRange` keeps the defaults 200.0 and 2500.0 from `00D19908`/`00D1990C` when
the field is nil, that both ranges are read at "slot 0 then slot 1", and that `angleRange`'s first
element is clamped to at least 10.0. All three are wrong:

- **the defaults belong to `distRange`**. `00949D69` and `00949D77` load them AFTER the `distRange`
  key push at `00949D50` and inside its absence arm. `00D19908` is `00 00 48 43` = 200.0f and
  `00D1990C` is `00 40 1c 45` = 2500.0f, both float32.
- **the Lua indices are 1 and 2**: `00949CDA PUSH 0x1` / `00949D19 PUSH 0x2` for `angleRange`,
  `00949D9E` / `00949DD4` for `distRange`. Lua arrays are one-based and the executable indexes them
  that way.
- **only `distRange` has an absence arm and only its first element is clamped**. `00949D95 CALL
  00B65FB0` / `00949D9C JNZ 00949E0A` keeps the defaults; `angleRange` has no such test and no
  default at all. The clamp is `00949E0A MOVSS XMM0,[00CE38B8]` (10.0f), `00949E12 COMISS XMM0,
  [ESP+0x10]`, `00949E21 JA` selecting the constant, i.e. `distLow = max(10.0f, distRange[1])`.

## 5. What `009483D0` makes

Per member, in group-member order, with the 10h-byte element at `record+4h + 10h*i`:

| element offset | meaning | evidence |
| --- | --- | --- |
| `+0h` | the vehicle class object | `00948462` calls its `vtable+18h`, `0094847E` its `vtable+28h` |
| `+8h` | a `char*` name, defaulted to the empty NativeString data at `00F89B40` | `00948488`, then a `_memcpy` into the created entity's `+154h`/`+158h` string |
| `+Ch` | one dword, read into the property-bag argument | `0094845B` |

- `vtable+18h(6)` is a kind test. When the class is **not** kind 6, the routine takes
  `operator_new(0x414)` and `BSP_PlaneSquadronTickableEntity_Construct`; when it **is**, it calls
  the class's own `vtable+28h(0)`. So kind 6 is the surface arm and everything else spawns as a
  plane squadron.
- `*(entity + 28Ch) = *(record + 7Ch)`: the created entity carries the request's serial.
- `record+90h`/`+94h` is a `std::vector<float[3]>` of per-member offsets, one per member
  (`00949356` divides the span by 0Ch; `00948440` reads element `i`).
- a kind-6 member after the first calls `BSP_Entity_RequestJoinFormation` on the first created
  entity (`0094875x`, guarded by `0 < local_b0`).
- each created entity is appended to `record+CCh`, a checked `std::vector<Entity*>` whose three
  pointers are `+D0h`/`+D4h`/`+D8h` - the three words `docs/LUA_BINDING_SPAWN.md` records as
  "zeroed", which is simply an empty vector.

## 6. What the script actually asks for, measured

The USN04 menu entry does **not** load `usn_04_defend_guadalcanal.lua`. `order_usn04.log` (run by
`agent/cc8-ai-squadron` on 2026-09-19) says `script=Scripts/missions/usn/usn_19_coralus.lua`, and
the scene line above it derives that path from `universe/Scenes/missions/USN/usn_19_coralus.scn`.
That file in this installation is 99030 bytes, mtime 2024-08-26, and has **fourteen** `SpawnNew(`
call sites. The run logs `MissionLuaNative::SpawnNew 0094c480 UNIMPLEMENTED calls=8`, so eight of
the fourteen are reached inside 3000 mission frames, all at `phase=luaStageInit`, all with `argc=1`.

**The neutral return value breaks nothing at the call site**: not one of the fourteen sites uses the
result. What the mission loses is the `callback`. Every site passes one, and
`luaLexKillersSpawned(unit1,unit2,unit3,unit4)` (line 1166) fills `Mission.LexKillers`, turns each
unit towards `Mission.Lex`, sets `SKILL_ELITE`, calls `Scoring_IgnoreEntityKill` and
`PilotSetTarget`, and then runs `luaAddLexHitListener()` and `luaDelay(luaLexKillersDia, 40)`. None
of that happens today, so the Lex hit listener is never registered and the mission's whole
Lexington-strike branch is inert.

### The callback's arity is a contract read off the consumer

The native's Lua-side dispatch is **not recovered** (section 9). Its arity is, from the shipped
script: the named global takes one argument per group member, in order.

| call site | group members | callback | uses |
| --- | --- | --- | --- |
| line 1054 | 4 (`Lexkiller 1..4`, D4Y/B5N alternating, `WingCount` 5) | `luaLexKillersSpawned` | all four parameters |
| line 2590 (difficulty 0) | 1 (`D3A Val`, `WingCount` 3) | `luaBombersSpawnedLex(unit1,unit2)` | `unit1` only; `unit2` is used on the branch whose request has two members |

All 28 group members in the file are aircraft: `Mission.TypeA6M = 150`, `TypeD3A = 158`,
`TypeD4Y = 159`, `TypeB5N = 162`, assigned as raw numbers at lines 104-108. `Type` is therefore the
same vehicle-class index the air-ops launch seam already takes.

## 7. The reconstruction

`include/bsp/lua_spawn_new.hpp` and `src/lua_spawn_new.cpp` hold the live request, the queue and the
candidate frame. `include/bsp/lua_binding_spawn.hpp` keeps the pure contract from packet
`cc_lua_core` and is not duplicated.

| host method | address | coverage |
| --- | --- | --- |
| `GameMissionLuaHost::run_spawn_new_00949750` | 00949750 | the twelve fields by name, the serial, the enqueue. Partial: the four staged floats are not mapped to record slots (section 9). |
| `GameMissionLuaHost::run_spawn_queue_0094c490` | 0094C490 | the empty test, the interval, the selection, the erase, the stamp, the fulfil test and the requeue. Complete for the Lua arm; the announcement block at `0094C5B3..0094C772` is not reconstructed. |
| `GameMissionLuaHost::fulfil_spawn_request_009483d0` | 0094A140 / 00949300 / 009483D0 | the all-or-nothing group rule and the per-member creation. Partial: no placement test (section 8). |
| `GameMissionLuaHost::complete_spawn_request_0094c777` | 0094C777 | the completion walk, with the Lua call as a contract (section 6). |
| `bsp::SpawnRequestQueue` | 00949530 / 009478B0 / 009439B0 / 00945850 / 00945A20 | complete for the operations the drain and the two id thunks need. |
| `bsp::next_spawn_request_serial_00949f2b` | 00949F2B | complete |

The creation goes through the already-public
`GameScriptOrdersHost::create_unit_from_scene_record_0046db4b`, which per
`docs/LUA_GENERATE_OBJECT_HOST.md`'s Correction already runs `plane_squadron_plan_members_007f4580`
for class 18h and spawns the whole wing. `WingCount` reaches that seam on a `SceneSpawnPoolEntry`,
which is the same carrier a held-back `PlaneSquadronGen` row already uses and for the same reason:
the property bag the key lives in does not exist on this path either.

## 8. What is NOT tested, and why it matters

`00941D30`, the per-member placement test, calls `BSP_Game_PointOutsideMapBounds` and
`BSP_World_GroundHeightAt`, and `0094A140` exists to retry the frame until that test passes. This
process runs **no placement test at all**: the first candidate frame is accepted, so a request is
always fulfilled on its first attempt and the retry path, while implemented, never fires on the
mission this packet measures. The five `excludeRadiusOverride` keys are read and stored and tested
by nothing.

DEVIATIONS, each labelled in the source where it is taken:

1. **Every member takes the plane-squadron arm.** This process cannot ask a class its kind, so
   `009483D0`'s `vtable+18h(6)` branch is not reproduced. Every member is created as
   `PlaneSquadronGen` (class 18h). No call site in `usn_19_coralus.lua` takes the other arm, but a
   mission that spawns a ship group would be created wrongly here.
2. **Every party counts as active.** `0094C508`'s walk prefers a record whose party is active in
   `game+18CCh`; this process has no party table, so the selection always answers the head - which
   is what the native itself does whenever the queue holds fewer than two records.
3. **The member fan-out is this process's rule**, not `0094A140`'s. See section 9.
4. **Member names carry the request serial.** The scripts reuse names across requests
   (`"Lexkiller 1"` at both line 1059 and line 1115) and the squadron registry is keyed by name, so
   `" #<serial>.<index>"` is appended. The script never sees the string; it addresses the unit
   through the entity table the callback hands it.

## 9. Open

- **How the `callback` NativeString at `record+84h` reaches the interpreter.** `00949750` pushes no
  code-address immediate anywhere in its body (the whole listing was filtered for
  `PUSH 0x00......` and `,0x00......`), so no trampoline is installed from an immediate; and no
  callee of `0094A140`, `00949300` or `009483D0` is a Lua helper. The record does hold callback
  OWNER objects - `00949750` destroys two through `BSP_CallbackOwner_Destroy 00695870` at `00949F24`
  and `00949FF4`, and `00948BB0` destroys another - so the dispatch is probably built there. Not
  established. The implemented call is the section 6 contract.
- **The four floats `00949750` stages at `00949F67..00949F8C` are not mapped to record slots.** The
  attempt is recorded so the next reader does not repeat it: the push accounting across
  `00949F64 SUB ESP,0x10` and the five intervening pushes does not close - it makes
  `00949F93 MOV ECX,[ESP+0x68]`, which must be the manager, land on the same steady-frame slot as
  `distRange[2]`. A slot claim that does not close is exactly the trap AGENTS.md names, so nothing
  is claimed. `tools/stack_frame_walk.py` is the tool for it.
- **`0094C5B3..0094C772`**, the block the drain runs before the completion walk when
  `game+1FE4h == 1`, `game+218Ch == 0` and `BSP_Game_GetEffectiveGameMode() <= 3`. It calls
  `00947BC0`, `00913AB0` and `0090EB80` and tests the class kinds 0Fh and 6; it looks like the
  reinforcement announcement. Unread.
- **The other three enqueuers.** `0094B94C` and `0094BF47` in `FUN_0094B600`, and `0094C3C6` in
  `FUN_0094BFF0`, also call `00949530`. They are the non-Lua producers and are presumably where
  `record+C0h != 0` and a non-null `record+C4h` come from. Unread.

## 10. Retractions against the packet brief

- `00A38DA0` is **not** part of this route. It reads `*(00F89B3C)` twice and is in neither the
  drain chain nor the enqueue chain. `docs/HANDOFF_AI_SPAWN_NEW.md` was right to flag it as
  online/session code.
- `00946FC0 BSP_AiParty_AvailableResources` is a planner-side reader of the manager, not the
  consumer. The consumer is `0094C490`.
- "`SpawnNew` registers a reinforcement request against a party's resource budget" is not what the
  binding does. It queues a spawn request that names its own units, its own position and its own
  callback; nothing in `00949750` or `0094C490` reads a resource budget.

## Validation

Build clean, both ctest suites pass. The before/after run pair is section 11 of this document once
the drain has a per-frame call site; until then the binding queues and the drain is never entered,
which the summary line reports as `attempts=0 still_queued=N`.

## no_ghidra_function

None. Every address in the header line has a Ghidra function, confirmed with
`python tools/bsp.py ghidra proto <addr> --brief`:

| address | body |
| --- | --- |
| `0094C490` | `0094C490` - `0094C826` |
| `0094C8F0` | `0094C8F0` - `0094C8F7` |
| `0094A140` | `0094A140` - `0094B5F2` |
| `00949300` | `00949300` - `00949473` |
| `009483D0` | `009483D0` - `009487C5` |
| `009478B0` | `009478B0` - `009478E1` |
| `00943C00` | `00943C00` - `00943C32` |
