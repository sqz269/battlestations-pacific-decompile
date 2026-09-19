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

This process runs the drain at the tail of `GameWorldHost::run_world_entity_update_00904bf0`, the
per-frame world walk, on the same scaled delta. That site needs no re-entrancy guard and the run
says so rather than an inspection: `summary world walk ... walks=3000` against
`summary mission frames ... simulated=3000`, so it is called exactly once per simulated frame and
never on a paused one. The mission Lua host publishes itself to it through a process pointer that
`attach_script_orders` sets and clears, which mirrors `00F89B3C`'s own lifetime (`004DFAC3` writes
it in `BSP_Game_ConstructWorld`, `004D2D7E` clears it in `BSP_Game_DestroyWorld`); the queue is
cleared with it, so a request cannot outlive its mission.
| the solver | `0094A140` | `__thiscall(record)`. Builds a candidate frame, offers it to `00949300`, and on refusal rebuilds it through `BSP_Matrix_BuildRotationY` with another angle and tries again. |
| the placement gate | `00949300` | `__thiscall(record, frame, block)`. Per member: compose the member frame, ask `00941D30` whether the placement is legal, and `bVar4 &= result`. Only when EVERY member passes does it call the creator, once. |
| the creator | `009483D0` | `__thiscall(record, frame)`. Creates every member, appends each created entity to the vector at `record+CCh`, and at its tail stores `*(record+C0h) = 1`. |

`0094C490`'s own reads confirm the manager layout independently of `docs/LUA_BINDING_SPAWN.md`:
`+4h` the list sentinel (`0094C501`, `0094C56B`), `+8h` the element count (`0094C4AE CMP dword ptr
[EDI + 0x8],EBX`), `+0Ch` a float stamp (`0094C4EA FADD float ptr [EDI + 0xc]`, written back at
`0094C59D MOVSS dword ptr [EDI + 0xc],XMM0`). The object is 10h bytes, so all four words are
accounted for.

## 2. The drain, instruction by instruction

1. `0094C4AE CMP dword ptr [EDI + 0x8],EBX` / `JZ 0094C810`: an empty queue returns before the
   clock is read.
2. `0094C4BC` and `0094C4C4`: returns when `DAT_00E188A8` is null or `game+1FE4h == 2`.
3. the rate limit, with the x87 stack written out because the comparison's sense is the whole rule:

   ```
   0094c4d1: FLD float ptr [0x00f876a4]   ; ST0 = now
   0094c4d7: FSTP float ptr [ESP + 0x8]
   0094c4db: CALL 0x00432650              ; EAX = the global config
   0094c4e0: FLD float ptr [ESP + 0x8]    ; ST0 = now
   0094c4e4: FLD float ptr [EAX + 0x2dc]  ; ST0 = interval, ST1 = now
   0094c4ea: FADD float ptr [EDI + 0xc]   ; ST0 = interval + lastAttempt
   0094c4ed: FCOMIP ST0,ST1
   0094c4ef: FSTP ST0
   0094c4f1: JA 0x0094c810                ; return when interval + lastAttempt > now
   ```

   **One request is attempted per interval**, and `manager+0Ch` is the stamp of the last attempt.
4. `0094C4FA CMP EAX,0x1` / `0094C4FF JBE 0x0094C56B` (the decompiler renders this as `< 2`): with
   one record or none the head is taken without
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
- **evidence**: `009487AD MOV byte ptr [EBX + 0xc0],0x1` with `EBX` the record, the last store in
  `009483D0` before its epilogue (`009487C3 RET 4`), reached only after the whole member loop and
  after `0094879A CALL 00925F20 BSP_SEntity_InitAll`; and `0094C5A7`, which branches on it.

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
| `+0h` | the vehicle class object | `00948510`/`00948519` call its `vtable+18h`, `00948522`/`00948529` its `vtable+28h` |
| `+8h` | a `char*` name, defaulted to the empty NativeString data at `00F89B40` | a `_memcpy` into the created entity's `+154h`/`+158h` string |
| `+Ch` | one dword, read into the property-bag argument | `009486B6 CALL 00922E20` |

- `vtable+18h(6)` is a kind test, and the branch sense is from the bytes: `0094851B TEST AL,AL` /
  `0094851D JZ 0x00948533`. `AL == 0`, i.e. **not** kind 6, jumps to `PUSH 0x414` and
  `00948563 CALL 007F2C60 BSP_PlaneSquadronTickableEntity_Construct`; `AL != 0`, kind 6, falls
  through to the class's own `vtable+28h(0)` at `00948529`. So kind 6 is the surface arm and
  everything else spawns as a plane squadron.
- `009485A4 MOV dword ptr [ESI + 0x28c],EAX`: the created entity carries the request's serial from
  `record+7Ch`.
- `record+90h`/`+94h` is a `std::vector<float[3]>` of per-member offsets, one per member
  (`00949356` divides the span by 0Ch).
- a kind-6 member after the first calls `BSP_Entity_RequestJoinFormation` at `0094870E`, guarded by
  the same kind test repeated at `009486C8`/`009486DA` and by a non-zero member index.
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

### Which road the callback takes

There are two existing ways into a named Lua global in this process, and the SpawnNew callback fits
neither exactly, so it is worth saying which it uses and why.

`GameScriptOrdersHost`'s script-call road (`src/game_hosts_script_orders.cpp:1560`) is the one the
think and `luaDelay` timers take. It always pushes the entity's own self-table as argument 1 and
then forwards stack slots, with `debug` as the `lua_pcall` error handler. That first argument is
wrong here: `luaBombersSpawnedLex(unit1, unit2)` takes the created units and nothing else, with no
subject in front of them.

So the callback uses the mission Lua host's own road, the same `lua_getfield(LUA_GLOBALSINDEX, name)`
plus `lua_pcall` that `call_entry_point` uses, with `push_resolved_entity_by_id` supplying one
entity table per group member and `lua_pushnil` for one that will not resolve. The only difference
from `call_entry_point` is the argument count. It is not a second road; it is the same primitive at
a different arity. The `errfunc` is 0 rather than `debug`, which in Lua 5.1 keeps the error object,
so a raising callback is reported with its message. The native hands `debugtrap` to `lua_pcall`
instead and `008C8390` returns no results, so Lua 5.1 replaces the error object with nil and the
caller learns only that the call failed; `docs/MISSION_LUA_HOST.md` line 138 records that the
`debugtrap` handler is used only by the named-call path. Losing the message is not worth
reproducing in a diagnostic host, and this is the same choice `call_entry_point` already makes.

### Bound from the listing, versus substituted through the scene-record path

`009483D0` builds a unit **from a class type**; `create_unit_from_scene_record_0046db4b` builds one
from an authored scene record. They are not the same creator, and this table says which side each
part of the route came from, because the difference is the packet's largest labelled substitution.

| part | bound from the listing | substituted |
| --- | --- | --- |
| the binding's twelve fields, defaults, clamp and serial | yes, `00949750` | - |
| the queue, the interval, the pick rule, the retry, the fulfil flag | yes, `0094C490` / `009478B0` / `009483D0` | - |
| the all-or-nothing group rule | yes, `00949300`'s `bVar4 &=` | - |
| one entity per group member, in order, appended in order | yes, `009483D0`'s loop and the `record+CCh` append | - |
| the member's `Name` becoming the entity's name | yes, the `_memcpy` into `entity+154h`/`+158h` | - |
| the member's `Type` choosing the class | no | the record's `type_id`, resolved by the scene-record creator instead of by the class object at member+0h |
| the plane-vs-surface arm | no | every member takes `PlaneSquadronGen` (class 18h); `009483D0` chooses by `vtable+18h(6)` |
| `WingCount` reaching `007F4580` | no | carried on a `SceneSpawnPoolEntry`, the same carrier a held-back row uses |
| the placement, exclusion radii and the retry's sampling | no | first candidate accepted; see "What is NOT tested" |
| `entity+28Ch = record+7Ch`, the serial on the entity | no | not written; the serial is kept host-side on the request |
| `BSP_Entity_RequestJoinFormation` for a kind-6 member after the first | no | not reached, because no member takes the surface arm |

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

- **How the `callback` NativeString at `record+84h` reaches the interpreter.** `00949750` installs
  no trampoline from an immediate. The check is worth writing out because the first attempt at it
  was vacuous: Ghidra prints an address immediate without leading zeros (`PUSH 0xd0e1f8`), so a
  filter for `PUSH 0x00......` matches nothing anywhere and proves nothing. Filtering the whole
  526-instruction listing (which reaches `0094A13D RET 4`, so the body is covered) for
  `PUSH 0x[0-9a-f]{5,8}` gives sixteen pushes: fifteen are `.rdata` string literals between
  `00CE3A0C` and `00D199D0` - the twelve field names, the two empty-string defaults and one more -
  and the sixteenth is `00949752 PUSH 0xca8625`, the MSVC exception handler pushed in the prologue
  two instructions into the function. None of them is a body in this routine's own segment.
  No callee of `0094A140`, `00949300` or `009483D0` is a Lua helper either.

  **`record+10h` is an object with a callback-owner destructor, and it is probably `refPos`, not the
  callback.** The record destructor hands exactly that field over: `00948C6D LEA ECX,[ESI + 0x10]` /
  `00948C74 CALL 00695870 BSP_CallbackOwner_Destroy`. Its producer is the constructor at
  `00948CF3 LEA ECX,[EBP + 0x10]` / `00948CFA CALL 008F8610`, taking one caller argument. `008F8610`
  belongs to the same family `00949750` uses to resolve `area` and `refPos` - `008F8530` at
  `00949B7D` on the entity arm of the `refPos` read, with `008F84D0`, `008F8680` and `008F84A0`
  nearby - so `+10h` reads as the resolved entity-or-position reference, which would own a callback
  because it subscribes to the entity it names. HYPOTHESIS: none of those bodies was read. It does
  mean the callback-owner objects in this record are not evidence for a Lua dispatch, and the two
  `00695870` calls in the binding itself (`00949F24`, `00949FF4`) are epilogue destructors for stack
  temporaries, with the EH state bytes around them to match. So the question is more open than the
  callback owners first suggested, not less.

  Nothing recovered so far reaches the interpreter from `record+84h`. The implemented call is the
  section 6 contract, taken from the script, and it is labelled as such in the source.
- **The four floats `00949750` stages at `00949F67..00949F8C` are not mapped to record slots by the
  producer.** The attempt is recorded so the next reader does not repeat it: the push accounting
  across `00949F64 SUB ESP,0x10` and the five intervening pushes does not close - it makes
  `00949F93 MOV ECX,[ESP+0x68]`, which must be the manager, land on the same steady-frame slot as
  `distRange[2]`. A slot claim that does not close is exactly the trap AGENTS.md names, so nothing
  is claimed from that side. `tools/stack_frame_walk.py` is the tool for it.

  **The consumer settles which pair is which, though**, and that is enough for this route.
  `0094A140` reads `record+68h` and `record+6Ch` as floats, forms
  `(record+6Ch + record+68h) * 0.5` - the multiplier is `0094A30A FLD double ptr [00D7A280]`, whose
  eight bytes are `00 00 00 00 00 00 e0 3f` = 0.5, a **double** by the load instruction - and
  `record+6Ch - record+68h`, negates through `00D7A208` (a float `00 00 00 80`, -0.0, loaded with
  `MOVSS`), and feeds the result to `BSP_Matrix_BuildRotationY`. A midpoint and a half-width turned
  into a yaw is an angle range, so `+68h`/`+6Ch` are `angleRange`. `record+70h` and `+74h` are
  compared low-against-high in the same block and `+70h` then divides a lateral offset to produce
  an angle, which makes it a radius, so `+70h`/`+74h` are `distRange`. **`record+78h` is compared
  as an `int` against an entity's `+54h`, the party field**, which corrects
  `lua_binding_spawn.hpp`'s "+74h, +78h two floats": `+78h` is not a float.

  What remains undecoded is only the SAMPLING - how the twelve-iteration retry
  (`local_350 = 0xc`) walks the angle and the distance between those bounds.
- **`0094C5B3..0094C772`**, the block the drain runs before the completion walk when
  `game+1FE4h == 1`, `game+218Ch == 0` and `BSP_Game_GetEffectiveGameMode() <= 3`. Its bodies were
  not read, but its callee sets place it: `00947BC0` calls `BSP_SessionMessage_ConstructBase` and
  the `BSP_BitCursor_Read*` family, so it serialises a session message, and `00913AB0` calls
  `BSP_MissionScoring_GrantAward` and carries the literals `Counter_RUA_MU`, `GA_SL` and `RUA_MU`,
  so it bumps a scoring counter. **The block announces and scores the spawn; it does not create
  anything**, which is why leaving it out costs this route no units. The kind tests it makes on
  0Fh and 6 choose which counter. `0090EB80` (sole callee `00625900`) is unread.
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

## 11. Correction found by the run: `00888760` reads `x`, `y`, `z`, not 1, 2, 3

`read_vector3_00888760` in `src/game_hosts_lua.cpp` read the position table with `rawgeti` 1..3 and
required exactly three array entries. `00888760 BSP_LuaObject_ReadVector3` has no index path at
all: `00888767..008887A9` is the prologue and two Lua temporaries, `008887A9 CALL 00B67080` opens an
iteration and `0088882F CALL 00B67190` steps it, and between them it compares each key against
three string literals and stores the number it finds:

```
008887c2: PUSH 0xceb488   ; "x", bytes 78 00
008887dd: FSTP float ptr [ESP + 0x8]
008887e3: PUSH 0xd045f8   ; "y", bytes 79 00
008887fe: FSTP float ptr [ESP + 0xc]
00888804: PUSH 0xcfd718   ; "z", bytes 7a 00
0088881f: FSTP float ptr [ESP + 0x10]
```

- **was**: a position argument is a table of exactly three numbers at indices 1, 2 and 3.
- **is**: a position argument is a table with the keys `x`, `y` and `z`; one keyed 1, 2, 3 matches
  nothing and reads (0, 0, 0).
- **evidence**: the three literals above, and the producer side this repository already had right -
  `GetPosition` pushes the same three named keys through `0088BA30`
  (`GameScriptOrdersHost::push_vector3_table_0088ba30`), and `usn_19_coralus.lua` writes
  `spawnpos1.x`, `.y` and `.z` on that table before handing it to `SpawnNew` as `area.refPos`.

The helper is shared with `run_generate_object_00944fd0`, so GenerateObject's position argument was
mis-read the same way. `docs/LUA_GENERATE_OBJECT_HOST.md` records that GenerateObject was not
called in either of its runs, so no measured column depends on the old behaviour.

### The native requires none of the three, and reports nothing

Asked whether "all three required" was the native's rule, it is not, and the first version of the
fix in this packet had it wrong too. Filtering the whole 85-instruction listing for the three
staging slots gives six accesses and no initialisation: the three `FSTP`s above, and the three
reads at the tail that copy them out **unconditionally**, whatever the walk found.

```
00888848: MOVSS XMM0,dword ptr [ESP + 0x8]
0088884e: MOVSS dword ptr [EDI],XMM0
00888852: MOVSS XMM0,dword ptr [ESP + 0xc]
00888858: MOVSS dword ptr [EDI + 0x4],XMM0
0088885d: MOVSS XMM0,dword ptr [ESP + 0x10]
00888867: MOVSS dword ptr [EDI + 0x8],XMM0
...
0088888b: MOV EAX,EDI
00888899: RET
```

`EDI` is `ECX`, the out vector (`0088877A MOV EDI,ECX`), and `0088888B MOV EAX,EDI` returns **that
pointer, not a success flag**. So a table carrying only `x` and `z` leaves the y component at
whatever that stack slot held, and the caller cannot tell.

The caller does not ask, either. `00949750`'s own site is `00949B9B LEA ECX,[ESP+0x80]` - a bare
stack local with no pre-fill adjacent to the call - then `00949BA2 CALL 00888760` and
`00949BA7`/`00949BAB`/`00949BB0` reading all three floats straight back with no test in between.
The shape question the native does ask is asked *before*, by `008889C0` at `00949B60`: is this an
entity handle? Everything that is not goes to `00888760` regardless.

So the host reader returns true when the table carried **at least one** of `x`, `y`, `z`, and
fills an absent component with **zero**. The zero is a DEVIATION, labelled: an uninitialised stack
value is not reproducible and zero is the only defensible substitute. The `bool` is this process's
own signal, not a recovered one; it exists so a caller can still ask "position or not?", and it is
deliberately not `found == 3`, because that would refuse a `{x=..., z=...}` sea-level point the
image accepts.

## Validation

Build clean, both ctest suites pass. `python tools/verify_report_calls.py
reports/cc8_spawn_new_route.json` reports 17 call rows checked, 0 failed.
`python tools/const_width_sweep.py --all --load-sites` checks 727 constants with this packet's four
floats included (`kSpawnAttemptDelayDefault` 00CE74F8, `kSpawnNewDistRangeDefaultLow` 00D19908,
`...High` 00D1990C, `kSpawnNewDistRangeLowMinimum` 00CE38B8) and none of them is in any mismatch
category; the one `B` row it reports is `kPilotPitchHalfRange`, which predates this packet.

| column | commit | what it is |
| --- | --- | --- |
| baseline | main `2ec3ed6ef` | `MissionLuaNative::SpawnNew 0094c480 UNIMPLEMENTED calls=8` |
| run 1 | `c2a776849` (main `d9c6fb08a` merged) | `local/spawn_after_usn04.log` |
| run 2 | run 1 plus the section 11 fix | `local/spawn_refpos_usn04.log` |
| run 3 | run 2 plus the relaxed reader ("at least one key") | `local/spawn_final_usn04.log` |

Run 3 confirms the relaxed reader changes nothing measurable, which is the prediction: every
`refPos` in this mission comes from `GetPosition`, which writes all three keys, so `found == 3` and
`found > 0` agree on every table the run sees. Its `summary SpawnNew` line, its
`summary mission world units=57`, its `summary mission commands` line and its 57
`unit world registration` lines are identical to run 2's, and the two runs' **whole native-call
tables agree row for row across all 72 rows**. Run 3 is the column that sits on the committed code.

Both runs: `--frames 3200 --press-start-frame 30 --menu-select USN04 --mission-frames 3000
--mission-frame-seconds 0.05`.

**Run 1.**

```
summary SpawnNew 0094c480 calls=8 rejected=0 queued=8 attempts=243 fulfilled=0
  requeued=243 units=0 callbacks=0 callback_missing=0 still_queued=8
  interval=0.500 clock=150.0
```

`calls=8` reproduces the baseline exactly, and the row is now `concrete` rather than
`UNIMPLEMENTED`. The queue and the drain are proved: 243 attempts over a 150.0 s mission clock at a
0.500 s interval, and 0.500 is the value the host read out of this installation's `globals.lua` and
logged, not a compiled-in constant. The requeue is proved too - `requeued=243`, `still_queued=8`,
nothing dropped. Four requests queue at stage init (serials 1-4) and four later (serials 5-8), so
the pacing is the script's.

Nothing was created, and the log says why on every request line: `refPos ABSENT (0.0 0.0 0.0)`.
That is section 11.

**Run 2**, the same build with only section 11's fix on top.

```
summary SpawnNew 0094c480 calls=8 rejected=0 queued=8 attempts=8 fulfilled=8
  requeued=0 units=8 callbacks=8 callback_missing=0 still_queued=0
  interval=0.500 clock=150.0
```

Every request now reads its position, is satisfied on its FIRST attempt, and answers its script:

```
SpawnNew 0094c480: serial 1 party 1, 1 group member(s), callback "luaBombersSpawnedLex",
  angleRange given, refPos (-12914.8 1500.0 -4946.7)
spawn queue 0094c490: serial 1 member 1 "D3A Val" type 158 WingCount 3 party 1
  -> entity 22 at (-12914.8 1500.0 -4746.7)
spawn queue 0094c490: serial 1 fulfilled, "luaBombersSpawnedLex"(1 unit table(s)) ran
```

The +200.0 on z is section 7's fan-out: one member puts `t` at the middle of `angleRange`, which is
0 for the script's symmetric `{DEG(-10), DEG(10)}`, and the distance is the clamped low end of the
absent `distRange`, 200.0.

| measure | run 1 | run 2 |
| --- | --- | --- |
| `SpawnNew` row | `concrete calls=8` | `concrete calls=8` |
| fulfilled / requeued | 0 / 243 | 8 / 0 |
| units created | 0 | 8 group members |
| callbacks run | 0 | 8, none raising |
| `summary mission world units` | 33 | 57 |
| `summary mission world lists registrations` | - | 57 |
| `summary mission commands` | `units=33 resolved=50 issued=94` | `units=57 resolved=80 issued=365` |
| `MissionLuaNative::PilotSetTarget` | `calls=1` | `calls=9` |
| `MissionLuaNative::EntityTurnToEntity` | `calls=1` | `calls=9` |

The world's unit count rises by **24**, not by 8, and that is the right number: each group member is
a `PlaneSquadronGen` whose `WingCount` is 3, so the creation seam spawns three planes for it. The
log says so per squadron - `WingCount=3 -> 3 member plane(s) (007F4580 mode 1 on the held-back row)`
- and the created entity ids step by three (22, 25, 28, 31, ... 55). That line calls itself
"GenerateObject squadron" because it is emitted inside
`create_unit_from_scene_record_0046db4b`, which this packet reuses rather than copies.

The `PilotSetTarget` and `EntityTurnToEntity` counts are the point of the packet: those calls are
made by `luaBombersSpawnedLex` and `luaBombersSpawnedTown` on the units the callback receives, and
they went from one to nine. The spawned aircraft reach the strike census by name with their wings:
`B5N Kate #8.1`, `B5N Kate #8.1|.-2`, `B5N Kate #8.1|.-3`, role `torpedo`, `script:PilotSetTarget`.
No callback raised; the run logs no Lua error.

Each of the 24 units is registered in the world by name:
`unit world registration: unit=D3A Val #1.1 creator=00956390 primary=00d19d28 entry=00956300
lists=6`, with `|.-2` and `|.-3` for its wings, and `unit world registration` lines go 33 in run 1
to 57 in run 2, of which 24 name a spawned aircraft. Each also gets a `plane spawn:` line with its
heading, so they enter as flying aircraft rather than as sea-level objects.

### The result that matters most: an objective stops completing itself

The native-call diff between the two runs is not only additions. These counts went DOWN:

| binding | run 1 | run 2 |
| --- | --- | --- |
| `HideUnitHP` | 41 | 0 |
| `Blackout` | 47 | 6 |
| `Objectives_Completed` | 1 | 0 |
| `CreateScript` / `SetThink` / `SETLOG` | 10 | 9 |
| `SetWait` | 9 | 8 |
| `DeleteScript` | 8 | 7 |
| `Music_Control_SetLevel` | 1 | 0 |

That is the fix, not a regression, and `usn_19_coralus.lua:569` says why:

```lua
if Mission.Difficulty == 0 then
    if Mission.BomberWave == 4 or table.getn(luaRemoveDeadsFromTable(Mission.IJNBombersLex)) == 0 then
        HideUnitHP()
        luaObj_Completed("primary",1,true)
        Blackout(true, "luaMoveToPh2", 3)
    end
end
```

`Mission.IJNBombersLex` is filled by `luaBombersSpawnedLex` from the units `SpawnNew` hands it.
With the binding unimplemented that table stayed empty, so `table.getn(...) == 0` was TRUE and the
mission's **primary objective completed itself on a mission whose bombers had never been created**.

Why it then repeated, which is what makes the counts so large, is in the shipped global helper:
`scripts/global/commandhelpers.lua:5898 luaObj_Completed` sets `obj.Success = true` at line 5923 but
its `obj.Active = false` is **commented out** at 5922. So `luaObj_IsActive` at 5989 keeps answering
true for ever, the guard at 5901 (`luaObj_GetSuccess(...) ~= nil`) stops only the native
`Objectives_Completed` call, and the mission script's own enclosing `if luaObj_IsActive("primary",1)`
at `usn_19_coralus.lua:567` re-entered on every think pass. That is the shape of the run-1 column
exactly: `Objectives_Completed` **once**, `HideUnitHP` and `Blackout` **41 and 47 times**, and
`Music_Control_SetLevel` once from `luaObj_Completed`'s own `setMusic` arm at 5925.

With the spawns landing, the test at 570 is false and none of it fires; run 2's residual six
`Blackout` calls come from the other sites in the file, such as `Blackout(false, "", 1)` at 1034.
The one fewer `CreateScript`/`SetThink`/`SETLOG`/`SetWait`/`DeleteScript` is the `luaMoveToPh2`
script object that is no longer spuriously created.

This installation is modded, so the commented-out `obj.Active = false` is a fact about **this
installation's** helper and nothing is claimed about a retail one. `scripts/global/` is not uniform:
`luamw_init.lua`, `messagesender.lua` and `timetable.lua` all carry the bulk mtime 2024-07-13
08:26:50, while `commandhelpers.lua` is 690958 bytes at **2024-10-29 12:54:22**, three and a half
months later than its three siblings. It is therefore a file that was replaced after the bulk
install, and the line numbers above are that file's.

(An earlier draft of this paragraph said 2024-10-29 was "the same bulk date as the rest of
`scripts/global/`". That was wrong and is retracted; the four mtimes above are the measurement.)

**Not reached in either run, and not tested by this packet:** `luaSpawnLexKillers`, and therefore
`luaLexKillersSpawned`, its four-member callback, and the `LexHitListener` it registers.
`MissionLuaNative::AddListener` is `calls=2` in both columns, unchanged. The reason is the script,
not the route: `luaSpawnLexKillers` is called only from `luaEndZuikakuDeadMovie`
(`usn_19_coralus.lua:1036`), the tail of the Zuikaku-sinking cinematic, which a 150-second run never
reaches. The four-argument arity in section 6 therefore remains a contract read off the script and
is **not** exercised by a run; only the one-argument shape is.

**Also not measured.** Whether the spawned bombers then fly an attack and hit the Lexington; and
the retry path, because with no placement test every request is satisfied on its first attempt (run
1 exercised the retry 243 times, but only through the `refPos` failure, not through `00941D30`).

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
