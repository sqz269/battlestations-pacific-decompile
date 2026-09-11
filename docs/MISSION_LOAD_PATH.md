# Mission load path (request to populated scene)

Addresses: 005c5600 004e2770 004bf930 004e1d70 004c6890 004c2c80 00439020 004d7920 004bb160 004bb440 004e1ca0 004bc890 004e4430 004dfb70 004db920 004da6c0 0046df00 004d4df0

Packet `cc_mission_load`, worktree `agent/cc-mission-load`. Ghidra was read-only for this packet;
every name below is a hypothesis, not a recovered symbol.

This document covers the part of the load that sits **in front of** `004DFB70`: what selects the
mission, what queues the request, and what the record it selects hands to the eight scene slot
records. `docs/MISSION_SCENE_LOAD.md` covers `004DFB70` itself, `docs/MISSION_STATE_ENTRY.md` the
`0Ch`→`0Dh` transition, `docs/SCENE_FILE_READER.md` the `.scn` reader and
`docs/SCENE_ENTITY_FACTORY.md` the class table. Nothing from those is restated here.

## The path, end to end

| # | Frame | Routine | Effect |
| --- | --- | --- | --- |
| 1 | the click | `005C5600` `BSP_MissionTree_StartSelectedMission` | reads the selected record, sets the pending scene, then either opens the briefing or queues the start |
| 2 | same frame | `004E2770` `BSP_Game_SetPendingScene` | destroys the old records, builds one for this scene, selects it |
| 3 | same frame | `004E1D70` `BSP_Game_BuildSceneRecord` | allocates the `0x109C` record and runs the `.scn` **header** pass into it |
| 4 | same frame | `004C6890` `BSP_Game_SelectSceneRecord` | `game+5FCh`, `game+60Ch`, `game+2015h`, `game+2028h`, the eight slot records |
| 5 | same frame | `00439020` `BSP_Game_RequestMissionStart` | queues request `6` then request `0Ah` |
| 6 | next frame | `004E4430` the drain | dispatches `6`, then `0Ah` in the same pass |
| 7 | next frame | `004DFB70` | the whole load, synchronously; leaves `game+5D4h = 0Ch` |
| 8 | frames after | `004DB920` → `004DA6C0` | `game+5D4h = 0Dh`, the mission runs |

Steps 2 to 5 all happen inside the one call at step 1, so the scene record and the eight slot
records exist **before** the request is queued. `004DFB70` only reads `game+5FCh`; it never
selects.

## What the briefing branch does

`005C5600` computes `side = (record+0B8h == 0)` (`005C5633`, a `SETZ` on the byte) and takes the
per-side sub-record at `record + 0B8h + side*154h` (`005C563C`, `IMUL EAX,EAX,0x154`). The branch
at `005C573A` tests the first dword of that sub-record:

- **non-zero** → `0051DCE0` fills the briefing screen from `[00E198AC]+64h` and `004CC460(1, 0)`
  pushes the interface request. **No state request is queued**; the mission starts later from the
  briefing screen.
- **zero** → `game+6ACh` takes `record+0B4h`, except that the value 3 means "keep what is already
  in `game+6B0h`" (`005C5772`), and `00439020` queues the pair.

Either way the routine first copies `record+0h` (a native `{int length; char* data}` pair) into
`game+2198h` and `game+6B8h`, hands the loading screen its text through `0057D060` (the string
vector at side-block `+64h`, the pair at `record+8h`, and the byte at `record+20Ch`), runs
`00626930(record, side)`, and ends with an input update at 0.0f.

The scene path it passes is the **data pointer** of the native string at `record+20h`, loaded from
`record+24h` at `005C5670`; a null pointer is replaced by the empty literal at `00E1952F`.

## `004E2770` and the scene record list

`004E2770` is `__thiscall void(GGame* this, const char* scenePath, const char* weatherOverride)`,
`ECX = game`, two stack arguments, `RET 8` at `004E27D2`. The second argument is a **string
pointer**, not a flag word: it reaches `0046DF00` as that function's argument 5, the override name.
`005C5682` pushes null for it, so a campaign mission lets the reader pick its weather descriptor
out of `SCRIPTS\datatables\Weather.lua`.

Order: `004BF930(game+5F0h)`, then `game+5FCh = 0`, then the override string at `game+600h/604h` is
emptied, then `004E1D70(game, scenePath, 0, -1, override)`, then `004C6890(game, 0)`.

`game+5F0h` is a three-word intrusive list header `{int count; Node* head; Node* tail}` over
`0xC`-byte nodes `{Node* prev; Node* next; SceneRecord* payload}`. `004BF930` pops every node,
calls the payload's virtual `+0h` with 1 and frees the node — so **the previous mission's scene
record is destroyed before the new one is built**, and any pointer into it held across the call is
dangling. `004C2C80` is the matching push_back and has exactly one caller.

## `004E1D70`, the scene record

`__thiscall int(GGame* this, const char* scenePath, char arm, int unused, const char* override)`,
`ECX = game`, four stack arguments, `RET 10h`, SEH handler `00C676C6`. The record is
`operator new(0x109C)` at `004E1F30` and `004E213D`, which is exactly the mission id at `+1098h`
plus four bytes, so the record has no tail beyond it. The whole body runs inside a VFS file block
named by the prefix at `00CE7F0C` plus `derive_scene_short_name` of the path.

The third stack argument is pushed as `-1` by `004E2770` and is **never read**: after the prologue
its slot is `[ESP+0x68]`, and nothing in the 383-instruction listing touches it.

Argument 2 picks the arm at `004E1E99`:

| Arm | Duplicate test | `record+914h` | Header pass | Append | Tail |
| --- | --- | --- | --- | --- | --- |
| `0` (what `004E2770` passes) | none | not written | unconditional, `004E21B1` | `004C2C80`, `004E21C1` | award tracker `004E1CA0` then `0068EA00` |
| non-zero | byte compare against every `record+910h`; a match returns 0 and appends nothing | VFS provider `0109CEEC` virtual `+8h`, `004E1FBD` | only when `+914h` is set, `004E20DA` | hand-built `0xC` node, `004E20DF` | none |

The header pass is `0046DF00(path, 0, 0, record, override, 0)` — `SceneFilePass::Header` of
`include/bsp/scene_file.hpp`. That is the **first** of four reads of the same `.scn` file in a
mission start: this one, then `004DFB70` step 11 (header again, against the live database), then
the two passes inside `004D4DF0`.

### Record fields this path uses

| Offset | Use |
| --- | --- |
| `+0h` | side-block array, stride `0x120`, count at `+988h`, capped at 8 by the layout (8 × 0x120 = 0x900, and `+90Ch` follows) |
| `+90Ch/+910h` | the scene path |
| `+914h` | byte, VFS presence; written only by the reuse arm |
| `+928h` | script-name table, stride 8 |
| `+980h/+984h` | comma-separated localisation table names |
| `+988h` | side-block count |
| `+1098h` | mission id |

## `004C6890` and the eight slot records

`__thiscall void(GGame* this, int index)`, `ECX = game`, one stack argument, `RET 4` at `004C6AFA`.

The clamp at `004C68A7` compares `count < index`, so `index == count` is **accepted**; the walk
then runs off the end of the list, and because the walk also stops early when a node has no
successor, the tail record is taken instead. Only an empty list yields a null record, and then the
routine returns at `004C6911` after emptying the override string.

The slot array is based at **`game+1008h`** with stride `0x118`: `004BB160` spells the eight
addresses out literally (`1008h, 1120h, 1238h, 1350h, 1468h, 1580h, 1698h, 17B0h`) when it points
`game+18CCh..18E8h` at them. The live participant array `004BB440` claims from is a second array of
the same record at `game+748h`, and it writes the same offsets, so the two share one layout:

| Offset | `004C6890` writes | `004BB440` writes |
| --- | --- | --- |
| `+8h` | 1 for a supplied side block, 0 otherwise | 1 when the record is claimed |
| `+24h` | side block `+8h` | copied from `game+102Ch` |
| `+28h` | side block `+4h` | copied from `game+1030h` |
| `+50h` | `strcpy` of the empty literal `00CE3A0C` | the first name argument |
| `+70h`/`+78h` | `strcpy` of the empty literal | the second name argument |

`game+1030h` is slot 0's `+28h`, so **the side selector `004DFB70` reads as
`[game+18CCh + local*4]+28h` originates in side block 0's `+4h` dword of the scene record** — that
is, in the mission's own authored party table, reached through `004BB440`. Slots past the record's
side-block count have only their in-use byte cleared; the rest of the `0x118` record keeps whatever
the previous mission left in it.

The routine also writes the low byte of the mission id to `game+2015h`, the script slot to
`game+2028h`, and in session mode 1 publishes the mission id through the `00F8A2FC` vtable at
`+178h`/`+17Ch`. In any networked mode it ends with `004BC890(game, game+614h, 0)`.

## `00439020`

Seven instructions, `__cdecl void()`: `004D7920(game, 6)` then `004D7920(game, 0Ah)`, `RET` at
`0043903A`. The drain re-reads the queue count after each dispatch, so both land in one pass —
interface first, then the load.

## Host methods the rebuilt executable must implement

`bsp::mission_load_host_steps` in `src/mission_load_path.cpp` is the machine-readable list: 68
steps in call order, each with the native address, the host interface, the method and an owner.
Fifty-seven need a subsystem owner; eleven the reconstruction already decides.

| Owner | Steps | What is blocking |
| --- | --- | --- |
| pure | 11 | nothing; the reconstruction supplies the answer and the method records it |
| gui | 14 | loading screen (`0057CB60`, `0057C250`, `0057D060`), the in-mission HUD manager (`0068A990`, `004C9680`), `004C9CA0` |
| session | 11 | participants, the network slot table, the tag `0Ch` event, `0077F5E0`, the award tracker |
| renderer | 7 | `00874640`, `006AD600`, `00951560`, the view priming at `008053C0`/`008073C0`, `[00F8D394]` vtable `+E4h` |
| scene graph | 5 | `0046DF00` (three of the four passes), `004D4DF0`, `004F2800`, the eleven precache calls at `004E0A9B` |
| lua | 5 | `005E2F00`, `008860B0`, `0045F440`, `0045F520`, `004D30F0` |
| vfs | 5 | `00BE0A30`/`00BDCB30` file blocks, the provider probe at `0109CEEC`, `00AA06D0` |
| audio | 4 | the time scale pair, `00A7A440`, `007065E0` |
| world | 3 | `004DC6A0`, `004DE610`, `00447060` |
| input | 3 | `00A91020`, `00A92C40` |

The renderer, scene-graph and world rows are the ones another owner (Codex holds the scene graph,
terrain, resource manager, VFS and render leases) has to supply; everything in the gui, session,
lua, audio and input rows is reconstructed elsewhere in `src/` or is a thin call.

**The narrowest path to a populated scene** is the scene-graph row: `0046DF00` and `004D4DF0` are
what create entities, and `004F2800` is what lets a class token resolve. Without them the load
reaches `game+5D4h = 0Ch` with an empty world; everything else on the list only decorates it.

## Proof against an installed mission

`bsp_mission_scene_probe` (`src/mission_scene_probe.cpp`) opens one installed `.scn` through the
reconstructed reader and prints the entity count per class name, the class id each token resolves
to, whether the registration pass keeps it, the derived short name, the five numbered block names
and the mission script path.

Default mission: **`universe/Scenes/missions/USN/usn_2_java.scn`**, the second USN campaign mission
("Battle of the Java Sea"). It is the smallest single-player campaign scene at 48680 bytes, and its
mission-tree entry is `scripts/datatables/missiontree.lua` line 3404, whose `sceneFile` key is
`sceneFilePath .. "USN/usn_2_java.scn"` with `sceneFilePath = "universe/Scenes/missions/"`.

```
scene: universe/Scenes/missions/USN/usn_2_java.scn (48680 bytes)
header uniqueID=1 (mission id, record+1098h) nextUID=2 properties=yes precache=no
entities per class (top-level blocks 34, nesting depth 2):
  DestroyerGen x32 id=0x7 instantiate-only template=32 uid=0 multitype=32
  NavPoint x2 id=0x41 instantiate-only template=2 uid=0 multitype=0
entities total=34 distinct classes=2 registration-pass classes kept=0 unregistered=0
short name (004cd7f0) = "usn_2"
vfs blocks: "1_usn_2" "3_usn_2" "4_usn_2" "5_usn_2" "6_usn_2"
```

A `grep` of the installed file reports the same 34 entity blocks and the same two class tokens, so
the reader neither drops nor invents one.

Across all 259 installed `.scn` files the probe exits 0 on every file:

| Measure | Value |
| --- | --- |
| files | 259 |
| entities | 133655 |
| distinct class tokens | 22 of the 26 registered classes |
| tokens the class table does not cover | 0 |
| files with a recovered error | 9 (21 errors) |
| files with no entities | 1 (`chg/usn_finale.scn`, header + traffic + groups only) |

The recovered errors are authoring slips the native reader is built to survive. The one at
`multi/scene12.scn` line 46101 is `template "Entities\SpawnPoint"` with no terminating `;`; the
reconstruction records the error, does not consume the token and resumes, which is what `008D9930`
does. Zero unregistered tokens matters because `0046CF40` dereferences the class descriptor at
`node+8` with no null check — an unknown token would fault the native reader, so a shipped file
producing one would mean the 26-row table is wrong.

`docs/SCENE_ENTITY_FACTORY.md` quotes 133664 entities over the same 259 files; this sweep counts
133655. The nine-entity difference is not explained here and both counts are recorded as they were
measured.

## Corrections

- **`docs/MISSION_SCENE_LOAD.md`: the two script-slot rules are not the same.** The doc says
  `004C6890` "picks the script slot with the same 8-or-9 rule `004DFB70` uses". They differ on slot
  9. `004C6958` tests `slot == 9` and **jumps to the store**, keeping it; `004E0896` tests the same
  value and falls into `MOV EAX,8`. So `game+2028h` can hold 9 while the load's own local copy of
  the slot is 8. `normalise_mission_script_slot` in `bsp/mission_scene_load.hpp` remains correct for
  `004DFB70`; `src/mission_load_path.cpp` carries the separate rule for `004C6890`.
- **`docs/MISSION_SCENE_LOAD.md`: the slot-record base is `game+1008h`, not `game+1010h`.**
  `004BB160` writes the eight literal addresses into `game+18CCh..18E8h`, and the first is
  `game+1008h`. The byte `004C6A44` sets at `game+1010h` is therefore slot `+8h`, and the two dwords
  land at slot `+28h` and `+24h`, not `+20h` and `+1Ch`. Read that way they coincide exactly with
  the offsets the same doc quotes from `004BB440`, which is the check that settles it.
- **`include/bsp/mission_tree_screens.hpp`: `set_pending_scene(scene, int flags)` has the wrong
  second parameter.** It is a `const char*` override name forwarded to `0046DF00` argument 5, and
  `005C5682` pushes null. That header is not owned by this packet; the change is listed as a
  follow-up.
- **`004E2770`'s ledger evidence said `004E1D70(scene, 0, FFFFFFFFh, flags)`.** The last argument
  is the override name, and the `FFFFFFFFh` is dead. Both recorded under the appended evidence.

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `mission_tree_host_fix` | 005c5600 004e2770 | include/bsp/mission_tree_screens.hpp, src/mission_tree_screens.cpp | Retype `set_pending_scene`'s second parameter as the override name; the header is owned elsewhere |
| `scene_record_storage` | 004e1d70 00419cc0 0041dd40 | docs/SCENE_RECORD_STORAGE.md | The sized-storage-pool string plumbing `004E1D70` uses for `record+90Ch/910h`, which this packet modelled as `std::string` |
| `mission_briefing_start` | 0051dce0 004cc460 005c57d0 00626930 | docs/MISSION_BRIEFING_START.md | The briefing arm of `005C5600` and what starts the mission from the briefing screen |
| `scene_record_side_blocks` | 004e1d70 0046df00 | docs/SCENE_RECORD_SIDE_BLOCKS.md | What fills the `0x120` side blocks at `record+0h` and the count at `+988h`; the `.scn` reader is the only candidate producer and this packet did not find the write |
| `leaderboard_publish` | 004c6890 00f8a2fc | docs/LEADERBOARD_PUBLISH.md | The `00F8A2FC` vtable `+178h`/`+17Ch` pair `004C6890` runs in session mode 1 |
| `scene_class_count_delta` | 0046cf40 | - | The nine-entity difference between this sweep (133655) and `docs/SCENE_ENTITY_FACTORY.md` (133664) |

## no_ghidra_function

none — every address in the Addresses line resolves to an existing Ghidra function.

## Uncertainties

- **The third argument of `004E1D70` (`-1`).** Never read by the body. It is either dead in this
  build or consumed through a path the decompiler and the stack-slot scan both missed. The
  reconstruction names it `unused` and passes it through.
- **Side-block contents.** `004C6890` copies two dwords out of each `0x120` block and nothing
  reads them further in this path; the meaning of the `+8h` dword is not established. The `+4h`
  dword is the value that becomes the side selector, but only because `004BB440` forwards it and
  `004DFB70` tests it for zero — the reconstruction models it as an opaque selector, matching the
  existing `MissionSceneLoadState::side_selector`.
- **Who fills the side blocks.** The header pass is the only thing that touches the record between
  `operator new` and `004C6890`, so the `.scn` reader must write them, but no write to
  `record+0h..8FFh` was found in the reader's own reconstruction. Listed as a follow-up.
- **`004C6890` reads some fields through the global `[00E188A8]` and others through its `this`
  pointer** (`004C6933` versus `004C696D`). They are the same object in every observed call, and
  the reconstruction assumes so.
- **The `00F8A2FC` pair in session mode 1** is recorded as "ran" rather than modelled; the object
  behind the vtable was not read.
- **Frame boundaries.** `advance_mission_load_path` assumes the state `0Ch` handler runs on a later
  frame than the load, which is what `004E4A4D` dispatching `004DB920` only while `game+5D4h` is
  `0Ch` implies, but the exact ordering inside one `004E4430` pass was not traced — `004E4430` is
  leased to another owner and was read only through `docs/GAME_FRAME_CONTROL.md`.
- The mission Lua host is started inside `004DFB70` (steps 25 to 27 of `docs/MISSION_SCENE_LOAD.md`)
  and this packet did not re-read it; `docs/MISSION_LUA_HOST.md` is the authority.

## Reconstruction

`include/bsp/mission_load_path.hpp` and `src/mission_load_path.cpp`. The record layout, the list
shape, the slot arithmetic, the two normalisation rules and the briefing decision are pure
functions with explicit inputs. `run_set_pending_scene_004e2770` and
`run_build_scene_record_004e1d70` are sequences over `SetPendingSceneHost`, one method per native
call site. `advance_mission_load_path` sequences the existing drivers — `run_mission_scene_load`,
`run_mission_device_wait` and `run_mission_state_entry` — and owns no behaviour of its own. This is
a semantic reconstruction, not an ABI-compatible replacement: `bsp_mission_scene_probe` checks the
scene-side half of it against installed data, and the rest is build-tested only.
