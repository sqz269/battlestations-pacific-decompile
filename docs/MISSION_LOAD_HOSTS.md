# Mission load hosts (packet `cc2_mission_load_hosts`)

Addresses: 004c3840 004cec60 004d87b0 004218e0 004d30f0 00447060 004c9ca0 00424d00 00421500
00b66200 004d32a0

Worktree `agent/cc2-mission-load-hosts`. Ghidra was read-only for this packet; every descriptive
name below is a hypothesis, not a recovered symbol. Report: `reports/mission_load_hosts.json`.
Reconstruction: `include/bsp/mission_load_hosts.hpp`, `src/mission_load_hosts.cpp`.

The driver is `bsp::run_mission_scene_load` over `MissionSceneLoadHost`
(`include/bsp/mission_scene_load.hpp`); this packet covers the seven of its methods that
`docs/GAME_EXECUTABLE.md` still reports unimplemented.

## Headline

Two of the seven already had reconstructions and are reused rather than rewritten. Three of the
remaining five were named after what the call site looked like rather than after the callee, and
all three names are wrong:

| Host method | What the interface says | What the callee does |
| --- | --- | --- |
| `reset_network_slots` | `004CEC60` resets network slots | `004CEC60` is an MSVC `std::_Tree::_Erase`; the reset is a branch of `004DFB70` that a local session never enters |
| `reset_objective_list` | clears the objective list at `game+5CCh/5D0h` | clears an unidentified tree at `game+5C8h` and rebuilds the **avoid-zone** table; the objective sets are at `game+21A4h + slot*4` |
| `rebuild_scripted_name_list` | refills an unidentified container | fills a `set<NativeString>` with the **name of every Lua global whose value is a function**, so teardown can nil the ones the mission scripts added |

## Step coverage

| Step | Native | Coverage | Reconstruction |
| --- | --- | --- | --- |
| `assign_party_player_slots` | `004C3840` | complete, `004C3840..004C3A6B` | `run_assign_party_player_slots_004c3840` |
| `reset_network_slots` | `004CEC60` + `004DFC13..004DFD18` | complete for both; the two cleared trees' producers unread | `run_reset_network_slots_004dfc13` |
| `check_multiplayer_player_count` | `004D87B0` | complete, reused | `run_mission_player_count_check_004d87b0` (existing) |
| `reset_objective_list` | `004218E0` + `004E0754..004E07C2` | complete for the block and the singleton; `00424D00` read for its scan shape and literals only | `run_reset_avoid_zone_state_004e0754` |
| `rebuild_scripted_name_list` | `004D30F0` | complete, `004D30F0..004D3299`; reader `004D32A0` read for its find-and-nil shape | `run_rebuild_scripted_name_list_004d30f0` |
| `release_deferred_dynamics` | `00447060` | complete, reused | `release_all_dynamics_00447060` (existing) |
| `apply_in_game_interface` | `004C9CA0` | partial: the load arm `004C9CCD..004C9D23` only | `run_apply_in_game_interface_load_arm_004c9ccd` |

`004C9CA0`'s entry arm `004C9D6B..004C9EA2` and its network tail `004C9D2F..004C9EA2` are packet
`cc_exe_2j`'s (`docs/GAME_EXECUTABLE.md` milestone 2j, `src/game_hosts_hud.cpp`) and were not
re-read here. `00424D00`'s own body past the name scan is `contract: unread`.

## 1. `004C3840` — which slot gets which party

`__thiscall void(GGame* /*ECX*/, char skipMarkedCandidates)`, `RET 4`, body
`004C3840..004C3A6B`. `004C38ED CMP byte ptr [ESP+0CCh],0` is the one stack argument. The load
path calls it at `004E044D` with **argument 0** and only when `game+1FE4h == 1`.

### Fields

| Field | Read/Written | Site | Meaning |
| --- | --- | --- | --- |
| `[[00E188A8]+5FCh]+988h` | read | `004C3884` | the scene record's side block count, the slot loop bound |
| `game+18CCh + i*4` | read | `004C38A0`, `004C3968` | the eight player slot pointers |
| `slot+8h` | read | `004C38A2`, `004C396F` | slot present |
| `slot+9h` | read | `004C38A8`, `004C3975` | slot flag |
| `slot+0Ah` | read | `004C38AE`, `004C397F` | slot flag |
| `slot+28h` | read | `004C38B4`, `004C3989` | party id, 0..2 |
| `entity+5Ch` | read | `004C3931` | live gate, must be set (`docs/UNIT_INSTANCE_LAYOUT.md`) |
| `entity+5Dh` | read | `004C393B` | must be clear |
| `entity+60h` | read | `004C3945` | must be clear |
| `entity+5Eh` | read | `004C394F` | dead byte, must be clear |
| `entity+188h` | read | `004C3959` | the owning player slot, unsigned, must be below 8 |
| `entity+304h` | read | `004C38FA` | float compared to `[00D7A218]`; only when the argument is set |
| `message+4h` / `+18h` / `+1Ah` / `+1Ch` / `+20h` | written | `004C39F0`..`004C3A04` | the routed message; `+20h` carries the new slot |

### The rule

A slot is **bound** when `slot+8h` is set and either `slot+9h` is clear or `slot+0Ah` is set
(`004C38A2..004C38B2`). `004C396F..004C3983` is the exact negation, so bound and vacant partition
the slots with no third case.

Pass one (`004C3896..004C38D3`) walks the slots in index order and appends each bound slot's index
to a roster keyed by its party: three counts at `ESP+20h`, three cursors at `ESP+2Ch` and a 3x8
table at `ESP+5Ch`. Pass two walks the intrusive list at `00F87198`. For each element that passes
the four byte gates and whose `+188h` owner slot is **vacant**, the vacated slot's own party
selects the roster:

* roster empty (`004C3996` not taken) — the element's vtable `+148h` is called with `(1FFh, 8)`;
* otherwise the element is handed `members[party][cursor]` and the cursor advances
  `(cursor + 1) % count` (`004C3A1D LEA EAX,[EBP+1]` / `CDQ` / `IDIV`, a signed remainder).

So a local single-player session redistributes every unit whose owning player slot is empty
round-robin among the players still bound to that unit's own party, and parks the rest.

`skipMarkedCandidates` is the only behavioural difference between the five callers. Set, it skips
an element whose `+304h` float equals `[00D7A218]` unless `IsKindOf(1Ch)` or `IsKindOf(9)` holds
(`vtable+5Ch`, `docs/LOCAL_PLAYER_UNIT_LISTS.md`). The load path passes 0, so nothing is skipped.

### It does route a message

`004C39E0` builds a message of kind **53h** through `0075B430` in the stack frame at `ESP+38h`,
writes the new slot to `message+20h` (`004C3A04`) and routes it at `004C3A18` through `0077C2A0`
with `ECX = the element` and `(message, 0, 0)`. `docs/SESSION_MESSAGE_DISPATCH.md` establishes that
`0077C2A0` is `__thiscall(entity, message, routeFlagsOverride, out)`, `RET 0Ch`, and that in a local
session the route flags are forced to 1 and the only destination is the local enqueue `0076E520`.
So in process the step is an ordinary local delivery, not a network send.

### Argument counts

`004C39C0..004C39D7` pushes `EBX`, `ECX`, `EAX`, then calls the entity's `vtable+10h` with
`ECX = entity` and **no stack argument**, pushes its result and the format, calls `004254B0` and
cleans up with `ADD ESP,14h`. Five dwords: the format plus name, party, old slot and new slot, the
four specifiers of `%s(party %d player %d)=>player %d` at `00CE75E4`. Ghidra folds the three pushes
into the virtual call and shows a one-argument log; the cleanup is the evidence.

## 2. `004CEC60` — not a network-slot reset

`__thiscall void(Tree* /*ECX*/, Node* node)`, `RET 4`, body `004CEC60..004CECAF`. It is the MSVC
`std::_Tree::_Erase`: return when `node+15h` (`_Isnil`) is set, recurse on `node+8h`, release the
node's value — a native string with its length at `node+0Ch` and its buffer at `node+10h` — through
the sized storage pool, `_free` the node, then continue iteratively on `node+0h`.

Ghidra's listing stops at `004CEC9C` and resumes at `004CECAC`. The eleven bytes between are the
loop's back edge and had to be decoded from the raw bytes
`83 c4 04 80 7e 15 00 8b fe 74 c5` = `ADD ESP,4` / `CMP byte ptr [ESI+15h],0` / `MOV EDI,ESI` /
`JZ 004CEC71`. Without them the pseudocode looks like a single-child recursion.

`ECX` is only propagated to the recursive call and never dereferenced.

### What the load path's network branch does

`004DFC13 CMP dword ptr [EDI+1FE4h],0` / `JZ 004DFD16`. **A local session never enters the reset.**
It takes `004BB160` (and then `004BB440`) instead, which is what
`docs/MISSION_SCENE_LOAD.md` step 6 records. In a networked session `004DFC1F..004DFD11`:

| Site | Effect |
| --- | --- |
| `004DFC2D` | `004CEC60` on `[00E18A64]->+4h`, then `004DFC32..004DFC4C` relinks the header empty and zeroes `00E18A68` |
| `004DFC5E` | the same shape over `00E18A6C` with the eraser `004C1FF0`; element type unread |
| `004DFC80` | `00E188BD = 0`, the same byte `004D87B0`'s side-balance block uses as its one-shot |
| `004DFC91..004DFCA4` | eight headers at `game+758h`, stride 118h: `+0Eh = 0`, word `+10h = FFFDh`, `+18h = 0` |
| `004DFCA6..004DFD0A` | the local slot's party (`game+18ECh` into `game+18CCh`, then `slot+28h`) is handed to `00626930` with the 434h-stride menu record `005D7070` selected, bounds-checked against `([ESI+8]-[ESI+4])/434h` |

`game+758h` is `game+748h + 10h`, so the three fields are `+0Eh`, `+10h` and `+18h` of each 118h
slot record — the same three the load path sets on the local slot at step 23 of
`docs/MISSION_SCENE_LOAD.md`, with `FFFDh` as the unassigned peer id.

**What a local reset leaves: nothing.** The step has no local effect at all.

## 3. `004D87B0` — reused

`docs/MISSION_STATE_FRAME.md` and `include/bsp/mission_state_frame.hpp` carry the whole routine.
The count rule: with `[game+1EF0h+29Ch]` clear, effective game mode 7 (`004D88C6`) wants more than
one participant in total and every other mode wants both sides non-empty; a first failure arms an
eight-second retry and sets `game+1EE5h`, a later failure raises `ingame.multi_notenoughplayer`,
and in mode 1 past the deadline `004D7970(0)` ends the scene. `004D87C9` returns when `game+1FE4h`
is 0, so in a local single-player session the check does not run and the outcome is
`MissionPlayerCountOutcome::kNotRun`.

## 4. `004218E0` — the avoid-zone rebuild, not an objective reset

`004218E0` is `__cdecl AvoidZoneManager*()`: a thread-safe lazy singleton getter that takes the
critical section from `BSP_SingletonLifetime_GetManager`, allocates `operator new(78h)` at
`0042193A`, constructs it with `00421500` (which creates a critical section of its own) and
registers it through `00BD0C30`, caching the pointer in `00E17624`. It resets nothing.

The block that calls it is `004E0754..004E07C2` in `004DFB70`:

| Site | Effect |
| --- | --- |
| `004E075A` | `game+648h = 0` |
| `004E0764` | `game+64Ch = 0.0f` |
| `004E076C..004E07B4` | an inlined `_Tree::_Erase` over `game+5C8h` (`_Myhead` `+5CCh`, `_Mysize` `+5D0h`) whose recursive half is `004C18D0`, then the empty relink. No value destructor runs, so the element is POD |
| `004E07B7` | `004218E0` |
| `004E07BE` | `00424D00` with `ECX` = the singleton |

`00424D00` walks the intrusive list at `[[00E188A8]+19CCh]+370h` through `node+4h` with the element
at `node+8h` and reads each element's name at `+154h` (length) / `+158h` (buffer), falling back to
`"<null name>"`. Its only literals are `AvoidZone`, `AvoidZoneG` and `AvoidZoneG %*s %d #%03d`, and
it reaches `_sscanf` and `_strncmp`, so it rebuilds the avoid-zone table by scanning the freshly
loaded world for `AvoidZone`-named entities. Its own body past that scan is `contract: unread`.

The objective sets are at `game+21A4h + slot*4` (`docs/OBJECTIVE_UNIT_LIST.md`), and nothing in this
block touches them. The tree at `game+5C8h` has no identified producer.

## 5. `004D30F0` — what the scripted-name list holds

`__fastcall void(GGame* /*ECX*/)`, `RET`, body `004D30F0..004D3299`, sole caller `004DFB70` at
`004E08E4`, immediately after `game+5D4h` becomes 0Ch.

`game+1930h` is an MSVC `_Tree` with `_Myhead` at `+1934h` and `_Mysize` at `+1938h`; `004CEC60`'s
node layout (string value at `node+0Ch`/`+10h`) settles the element type. The routine clears it,
then opens the Lua globals table on the state owner at `game+1A0Ch` (`004D3167`, `00B67980`) and
walks it:

| Site | Call | Effect |
| --- | --- | --- |
| `004D317F` | `00B67080` | `IterateFirst(globals, key, value)` |
| `004D318D` | `00B66420` | loop test on the key |
| `004D31A4` | `00B66200` | `IsFunction(value)` |
| `004D31B1` | `00B662B0` | `GetString(key)` |
| `004D320B` | `004D0640` | `BSP_NativeStringSet_Insert(game+1930h, name)` |
| `004D323A` | `00B67190` | `IterateNext` |

`00B66200` is `lua_type(...) == 6`, `LUA_TFUNCTION` (`00B66200..00B6623E`; the unbound arm at
`00B66209` forces -1 and compares it to 6). So **the set holds the name of every Lua global that is
a function at the moment the scene reaches state 0Ch** — the pre-script baseline of the global
namespace.

### Who reads it

`004D32A0`, called only from `BSP_Game_TeardownSessionState` and returning at `004D32D2` when the
set is empty. It walks the globals again, and for every function-valued global whose name the set
does **not** carry (`004D33C5` `BSP_NativeStringSet_Find`, `004D33DB` the iterator compared to the
tree head, i.e. `end()`), it appends the name to a vector, then builds `"<name> = nil"` with
`BSP_NativeString_Concat` and runs each through `BSP_LuaStateOwner_ExecuteString`. The set is the
baseline that lets teardown nil exactly the globals the mission scripts added.

## 6. `00447060` — reused

`docs/GAME_DYNAMICS_LIST.md` and `release_all_dynamics_00447060` in
`include/bsp/mission_state_frame.hpp` already cover it in full. Nothing was added.

## 7. `004C9CA0` load arm and the loading element

`004C9CC0 CMP byte ptr [ESP+1Ch],0` splits the routine. Argument 1 — the scene load's call at
`004E1873` — takes `004C9CCD..004C9D68`:

| Site | Effect |
| --- | --- |
| `004C9CCD` | `[00E198C4]+D4h` non-null: reuse, skip to `004C9D13` |
| `004C9CD7` | `operator new(34h)`; `004C9CE9` a null allocation stores null and skips the init |
| `004C9CED` | `00636D90`, which installs vtable `00CF569C` at `+0h` and `00CF5684` at `+8h` over the `BSP_FrontEndScreen` base `004F7180` |
| `004C9D11` | `CALL [vtable+10h]`, `ECX` = the element, no stack argument |
| `004C9D1F` | `element+4h = 1`, on both the created and the reused path |
| `004C9D23` | `game+1FE4h == 0` jumps to the epilogue: a local session stops here |
| `004C9D2F` | `game+218Ch` set also jumps to the epilogue |

`vtable 00CF569C` is `[00636DC0, 00636DE0, 00636DD0, 00636F10, 00636F30, 004F7590]`, so `+10h` is
`00636F30 BSP_HudSceneInitScreen_Register`, already documented by `docs/HUD_SCREEN_PAGES.md`: it
loads the GUI page `FE_sceneinit` and binds `Message_Text`. The GUI is a contract here.

## Host table

Every native call site the reconstruction routes through a virtual. `contract: unread` marks a
callee whose body was not opened.

| Site | Callee | Host method | this / args / ret | Gate |
| --- | --- | --- | --- | --- |
| `004C3918` | `vtable+5Ch` | `candidate_is_kind` | candidate / `1Ch` / bool | argument set and the `+304h` marker matched |
| `004C3927` | `vtable+5Ch` | `candidate_is_kind` | candidate / `9` / bool | the `1Ch` test failed |
| `004C39A9` | `vtable+148h` | `unbind_candidate_owner` | candidate / `1FFh`, `8` / void | the party roster is empty — `contract: unread` |
| `004C39CA` | `vtable+10h` | `candidate_name` | candidate / none / `const char*` | reassignment — `contract: unread` |
| `004C39D2` | `004254B0` | `log_reassignment` | none / 5 dwords (`ADD ESP,14h`) / void | reassignment |
| `004C39E0` | `0075B430` | folded into `route_owner_change` | stack message / `53h` / void | reassignment |
| `004C3A18` | `0077C2A0` | `route_owner_change` | candidate / message, 0, 0 (`RET 0Ch`) / void | reassignment |
| `004DFC2D` | `004CEC60` | `clear_string_tree_00e18a60` | `00E18A60` / node / void | session mode non-zero |
| `004DFC5E` | `004C1FF0` | `clear_record_tree_00e18a6c` | `00E18A6C` / node / void | session mode non-zero — `contract: unread` |
| `004DFCCF` | `005D7070` | `select_menu_record_005d7070` | `game+60Ch` / none / int | session mode non-zero — `contract: unread` |
| `004DFD0A` | `00626930` | `apply_menu_record_00626930` | the 434h record / `EDX` = party / void | index in range — `contract: unread` |
| `004DFD18` | `004BB160` | `reset_single_player_slots_004bb160` | game / none / void | session mode 0 |
| `004DA755` | `004D87B0` | existing `run_mission_player_count_check_004d87b0` | game / none / void | networked session |
| `004E0789` | `004C18D0` | folded into `clear_scene_tree_5c8h` | `game+5C8h` / node / void | per non-nil node — `contract: unread` |
| `004E07B7` | `004218E0` | folded into `rebuild_avoid_zones_00424d00` | none / none / manager\* | unconditional |
| `004E07BE` | `00424D00` | `rebuild_avoid_zones_00424d00` | the singleton / none / void | unconditional |
| `004D3126` | `004CEC60` | `clear_scripted_name_set` | `game+1930h` / node / void | unconditional |
| `004D3167` | `00B67980` | folded into `lua_global_count` / `lua_global` | `game+1A0Ch` / out / void | unconditional |
| `004D317F` | `00B67080` | folded into `lua_global` | globals / key, value / void | unconditional |
| `004D31A4` | `00B66200` | folded into `LuaGlobalEntry::is_function` | value / none / bool | per entry |
| `004D31B1` | `00B662B0` | folded into `LuaGlobalEntry::name` | key / none / `const char*` | the value is a function |
| `004D320B` | `004D0640` | `insert_scripted_name` | `game+1930h` / string / void | the value is a function |
| `004D33C5` | `004C7F10` | teardown, `scripted_globals_to_nil` | `game+1930h` / key, out / iterator\* | teardown only |
| `004D33E9` | `004CDC20` | teardown, `scripted_globals_to_nil` | vector / string / void | the iterator is `end()` |
| `004C9CD7` | `00BF681B` | `create_loading_element_00636d90` | none / `34h` / `void*` | the element slot is null |
| `004C9CED` | `00636D90` | `create_loading_element_00636d90` | the element / none / `void*` | the allocation succeeded |
| `004C9D11` | `vtable+10h` = `00636F30` | `loading_element_init_00636f30` | the element / none / void | the allocation succeeded |

## Uncertainties

* The list at `00F87198` has **no writer** in its xref set — `004C3840`, `004C3A80`, `004D56E0`,
  `0076F2B0`, `0077C540`, `0077E4C0`, `0077F0E0` and `0077F5E0` all read it. Its element is a
  unit-like object (`+5Ch`/`+5Dh`/`+5Eh` as `docs/UNIT_INSTANCE_LAYOUT.md`, an owner slot at
  `+188h`, a float at `+304h`) but the producer is unread, so the reconstruction takes the elements
  from the host rather than modelling the container.
* The candidate vtable slots `+10h` and `+148h` were not opened. `+10h` returns the `%s` the log
  prints; `+148h` takes two stack arguments and the `(1FFh, 8)` pair is a hypothesis about
  "no owner", not a read contract.
* `00E18A60` and `00E18A6C` are referenced only by `004DFB70`, `004DA780` and their static
  constructors, so nothing in the xref set inserts into either tree.
* The tree at `game+5C8h` has a POD element and no identified producer.
* `004C3840` keeps **three** party rosters, `004D87B0`'s side counters are likewise a three-slot
  array indexed by `slot+28h`, and `docs/VEHICLE_CLASS_DESCRIPTORS.md` reports the party bitmap row
  as stride 3 with the third byte unexplained. The third party is consistent across all three
  readers; nothing was found that fills it.
* No run-time evidence was gathered for this packet. `bsp_game.exe` reaches `004C9CA0`'s load arm
  (milestone 2j already runs it) but not `004C3840`, whose gate is `game+1FE4h == 1`.
