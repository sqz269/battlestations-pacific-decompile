# The ten mission Lua bindings the installed scripts reach most

Addresses: 008c8f70 008c4d10 008ae030 0088c620 008a8930 008b87f0 008c61c0 008b8640 00897fb0 008cfe40 0088a330 00905340 0090bfc0 00a788a0 0077edf0 00b664b0 00b66450 00b677e0 004263b0 0076d280 00706760 0096bf70 00964790 0095ba60

Packet `cc_lua_core`, worktree `agent/cc-lua-core`. Ghidra was read-only. Every name here is a
hypothesis, not a recovered symbol.

`docs/LUA_BINDING_TABLE.md` ranked the 560 rows of `00E0B7B8` by how many of the 299 installed
mission scripts call them during the chunk and the four entry points. `CreateScript` (299) and
`FindEntity` (258) were already read by `docs/LUA_BINDING_ENTITY.md`. The next ten are this
document. `debugtrap` (160) is excluded because its count is the probe's own error-handler
pushes, which that document established.

The reconstruction is `include/bsp/lua_binding_core.hpp` and `src/lua_binding_core.cpp`; the
spawn block is `docs/LUA_BINDING_SPAWN.md` and the three doubly-named handlers are
`docs/LUA_BINDING_ALIASES.md`.

## The shape they share

Every one of the ten is `__fastcall(lua_State* in ECX)` and returns its result count in EAX.
Between the machine's prologue (`00B66C00` borrowed owner, `00B679B0` call frame) and its
epilogue (`00B66400` result count, `00B669A0` close), each one reads arguments through
`00B677E0` and pushes through `00B664B0` or `00B66450`. The result count is
`lua_gettop - frame.base`, never a literal, so a binding returns whatever it left on the stack.

Each also carries a one-time initialisation of a per-binding static log category named
`luakod`, guarded by bit 0 of its own flag byte. It is the compiler's static-local pattern and
has no per-call effect; it is not modelled.

## The routines

| Address | Global | Scripts | Reads | Pushes | Coverage |
| --- | --- | --- | --- | --- | --- |
| `008C8F70` | `PrepareClass` | 258 | every argument, as an integer, three times each | nothing | complete |
| `008C4D10` | `Music_Control_SetLevel` | 237 | argument 0 as an integer, twice | nothing | complete |
| `008AE030` | `GetDifficulty` | 206 | nothing | one number | complete |
| `0088C620` | `SETLOG` | 203 | nothing | nothing | complete |
| `008A8930` | `SetParty` | 202 | entity at 0, integer at 1 | one number | complete |
| `008B87F0` | `Scoring_RealPlayTimeRunning` | 196 | boolean at 0 | one boolean | complete |
| `008C61C0` | `LoadMessageMap` | 194 | string at 0, integer at 1, twice each | nothing | complete |
| `008B8640` | `Scoring_SetFinalScoringFunctionName` | 192 | string at 0 | nothing | complete |
| `00897FB0` | `SetThink` | 191 | entity at 0, string at 1 | nothing | complete |
| `008CFE40` | `EnableMessages` | 170 | one of three shapes | nothing | complete |

`SETLOG` is the surprise. 203 scripts call it and the listing from `0088C620` to `0088C74B` is
25 instructions with no branch into game code: open the owner, open the frame, take the count,
close. It reads no argument and writes no game state. The name suggests a logging switch; the
body does not implement one on this build.

`GetDifficulty` is equally short. It pushes the literal `2` when the game's non-campaign flag
is non-zero and the dword at game+6ACh otherwise. `include/bsp/mission_tree_screens.hpp`
already names both fields (`kGameNonCampaignFlagOffset` 1FE4h, `kGameEffectiveDifficultyOffset`
6ACh) from the launch path, so this document reuses those names rather than declaring new ones.

`PrepareClass` is where the work is. Per argument it logs `Prepare class %d`, builds the
globals path `VehicleClass.<id>.Race` out of three pieces (`00CE9248` + the decimal id through
`004263B0` + `00D153E0`), resolves it with `00B68D70`, and when the result is an integer calls
`0095BA60` with 1 for race 1 or 4 and 0 for anything else. It then calls the vehicle-class
factory `00964790` with `DL = 1`. Both callees were already reconstructed
(`docs/SCENE_UNIT_CREATORS.md`, `docs/VEHICLE_CLASS_DESCRIPTORS.md`).

## The host table

Every native step, as the call site that reaches it. `native` is the callee, `address` the call
site, and the containing function is the packet routine unless the row says otherwise.

| Host method | address | native | Contract |
| --- | --- | --- | --- |
| `log_prepare_class` | `008C9099` | `004254B0` | variadic log sink; `ADD ESP,8` says format plus one int |
| `resolve_global_integer` | `008C91BA` | `00B68D70` | dot path over the globals; folds the is-integer test at `008C926C` |
| `vehicle_class_mark_party_required` | `008C9297` | `0095BA60` | body read; per-party required-class bitmap |
| `vehicle_class_get_or_create` | `008C92BE` | `00964790` | body read; vehicle-class descriptor factory |
| `music_director_set_level` | `008C4E1D` | `00A788A0` | body read; partial, the track switch was not read |
| `session_send_music_level` | `008C4E74` | `0076D280` | body read; session message 2Ah, vtable `00D0319C` |
| `entity_vcall_5c` | `008A8A83` | vtable+5Ch | unread: a virtual with no resolved concrete vtable |
| `entity_vcall_2c` | `008A8AE3` | vtable+2Ch | unread, same reason |
| `session_route_party_message` | `008A8AC4` | `0077C2A0` | unread; the ledger name predates this packet |
| `scoring_set_real_play_time_running` | `008B8901` | `00905340` | body read in full, three instructions |
| `scoring_set_final_scoring_function_name` | `008B8766` | `0090BFC0` | body read; NativeString assign at scoring+147Ch |
| `message_map_load` | `008C6308` | `00706760` | partial: language and content-ownership resolution, then a file block |
| `call_0088b6d0_0076a9f0_00765590` | `008C63C3` | `0088B6D0` | unread, named by address |
| `entity_set_think_script_name` | `008980E8` | `0088A330` | body read including the flow gap; see Corrections |
| `set_entity_message_suppression` | `008CFFE4` | `0077EDF0` | body read; a map find-or-insert keyed on the entity pointer |
| `message_system_drain_queue` | `008D000A` | `0096BF70` | partial: takes the critical section at +24h and walks the list at +E4h |

Two more rows have no callee at all: `game_non_campaign_flag` is the load at `008AE113`,
`008C4E38` and `008C6355`, and `set_global_message_suppression` is the store at `008D0000`.

## Game fields

| Field | Offset | Established by |
| --- | --- | --- |
| non-campaign flag | game+1FE4h | already `kGameNonCampaignFlagOffset`; read by three of the ten |
| effective difficulty | game+6ACh | already `kGameEffectiveDifficultyOffset`; read by `GetDifficulty` |
| mission scoring object | game+21A0h | `008B88FA`, `008B8753` |
| message-map manager | game+21DCh | `008C62FC`; producer already in `world_construct.hpp` |
| session sender | game+1EF0h | `008C63D0`, `008C4E66`; an interior pointer, not a load |
| real-play-time-running | scoring+14A4h | `00905344` |
| final scoring function name | scoring+147Ch | `0090BFC0` |
| think script name | entity+1D8h | `0088A333` |
| think state byte | entity+1DCh | `0088A37F`, always cleared |
| global message suppression | `*(00F8A0C4)`+D0h | `008D0000` |
| per-entity suppression map | `*(00F8A0C4)`+D4h | `008CFFDA` |

The non-campaign branches of `Music_Control_SetLevel` and `LoadMessageMap` both test the flag
against the literal 1, not against zero (`008C4E42`, `008C635B`). `GetDifficulty` tests it
against zero. So a value other than 0 or 1 would give the fixed difficulty 2 while taking
neither session path; nothing in this packet establishes whether such a value occurs.

## Run-time evidence

`bsp_mission_script_probe --sweep --recon-tables` now takes `--core-bindings`, which routes
these ten through `src/lua_binding_core.cpp` instead of the stub. The probe's host performs the
one step it honestly can (`resolve_global_integer`, against the real datatable globals) and
records the rest.

| Measure | stubbed | reconstructed |
| --- | --- | --- |
| scripts whose chunk and four entry points run clean | 299 / 299 | 299 / 299 |
| `CreateScript` bodies that raise when called with `this` | 160 | 156 |
| distinct rows reached at load time | 140 | 140 |

Four scripts stop raising: `COTP-IJN/PRCPIJN/jm10`, `chg/chg_8_periscope`, `chg/chg_9_coup` and
`ijn/ESMP/04_solomon_sea`. Those bodies were failing on a nil where `GetDifficulty` or
`SetParty` should have returned a value. The load path itself was already clean under stubs, so
the reconstruction cannot improve it; what it improves is how far the `CreateScript` bodies get,
and those run from the frame loop.

The same run makes the per-script counts rise, because a body that gets further calls more
bindings: `SetThink` 191 to 195, `Scoring_SetFinalScoringFunctionName` 192 to 198,
`Music_Control_SetLevel` 237 to 239, `SetParty` 202 to 203, `EnableMessages` 170 to 171 and
`SETLOG` 203 to 204.

`PrepareClass` was called 2616 times over the sweep, and because it loops over its arguments
that is 2623 path resolutions. 2571 of them found a number and reached `0095BA60`; the other 52
are class ids the installed `vehicleclasses.lua` has no `Race` row for.

That figure only appears once the probe reproduces `00B68D70`'s numeric fallback. A first run
resolved nothing at all, because `vehicleclasses.lua` builds its rows under integer keys and
`VehicleClass["5"]` is not `VehicleClass[5]`. The resolver's rule, already reconstructed by
`docs/LOCALE_TEXT_LOOKUP.md` and implemented for the locale root in
`src/locale_lua_context.cpp`, is a string lookup first and then a base-10 `_atol` retry taken
only when the conversion is non-zero or the segment is the single character `0`. With the
fallback the count goes from 0 to 2571, which is run-time evidence for that rule as much as for
this one.

## Corrections

**`0088A330`'s decompilation drops a reachable block.** `python tools/bsp.py ghidra flow
0088a330` reports one gap: after the `_free` call at `0088A35B`, thirteen bytes at
`0088A360..0088A36C` are undisassembled, and the decompiler shows an early `return` there.

- was: `if (entity->think_name != nullptr) { free(entity->think_name); return; }`
- is: `if (entity->think_name != nullptr) { free(entity->think_name); entity->think_name = nullptr; }` and the routine falls through to duplicate the new name and clear entity+1DCh
- evidence: the disk bytes at `0088A360` are `83 C4 04 C7 86 D8 01 00 00 00 00 00 00`, which is
  `ADD ESP,4` then `MOV dword ptr [ESI+1D8h],0`, and the listing resumes at `0088A36D`.

The integrator can clear it with `python tools/ghidra_flow_repair.py 0088a330 --apply`. Until
then a reader of the pseudocode would conclude that `SetThink("x")` on an entity that already
has a think name leaves the old name freed and dangling; it does not.

**The same class of gap is in `00945A20`**, and there it changes the contract rather than only
the tidiness. See `docs/LUA_BINDING_SPAWN.md`.

**`Spawn` 00944680 belongs to the entity-returning set.** `docs/LUA_BINDING_TABLE.md` counted
19 handlers that end in the `thisTable` tail, derived mechanically from the call graph. `Spawn`
had no Ghidra function when that export ran, so the graph did not carry it. It does reach
`00B67910` with the `thisTable` literal and then `00B663D0`, so the set is 20. Re-checking all
24 newly defined handlers the same way, `Spawn` is the only one; the three spawn thunks and the
fourteen `AI*` handlers do not, and neither do the thunks' targets.

- was: nineteen entity-returning bindings
- is: twenty, with `Spawn` `00944680` added
- evidence: `python tools/bsp.py callees 00944680` lists both `00B67910` and `00B663D0`; the
  same query over the other 23 newly defined handlers lists neither.

## Uncertainties

- `[00F8BBCC]` is the music director's base and `[00E188A8]` the game's. Whether they are the
  same object was not established; the packet cites each literally.
- `00706760` was characterised from its callee list and two literals, not read. Its contract
  row says `partial`.
- `0096BF70` was read as far as the critical section and the list walk; what it does to each
  queued message was not read.
- The two `SetParty` virtuals have no resolved concrete vtable, so their contracts are unread
  and the probe takes the no-session-message branch by choice, recorded rather than assumed.
- `00905340` leaves only `AL` defined on return, and `008B890B` pushes the whole of `EAX`. The
  upper 24 bits are whatever `00B66250` left. `lua_pushboolean` treats any non-zero as true, so
  the round trip is correct for both values, but it is correct by accident of the ABI.

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `lua_binding_message_map` | 00706760 00707480 0088b6d0 0076a9f0 00765590 | docs/LUA_BINDING_MESSAGE_MAP.md | The message-map manager at game+21DCh and the three-step session path `LoadMessageMap` takes outside a campaign |
| `lua_binding_message_system` | 0096bf70 0077edf0 00f8a0c4 | docs/MESSAGE_SYSTEM.md | The message singleton: the queue at +E4h, its critical section, and what the two suppression bytes gate |
| `lua_binding_music_director` | 00a788a0 00f8bbcc 0076d280 | docs/MUSIC_DIRECTOR.md | The 18h-byte track rows at +10h, the set selector at +210h, and whether `[00F8BBCC]` is the game object |
| `lua_binding_entity_party_vtable` | 008a8930 | docs/LUA_BINDING_CORE.md | Resolve the concrete vtable behind entity+5Ch and +2Ch so the two `SetParty` rows stop saying `unread` |
| `lua_binding_frame_reach` | 00e0b7b8 | docs/LUA_BINDING_TABLE.md | Which rows the frame loop reaches, which the load-time sweep cannot see (still open) |

## no_ghidra_function

None. Every address this packet named already had a Ghidra function: the ten core handlers and
the three aliased handlers were defined before it started, and the four spawn handlers were
defined and named by the integrator at 39F9528F, which `python tools/bsp.py ghidra proto <addr>
--brief` confirms for all four.
