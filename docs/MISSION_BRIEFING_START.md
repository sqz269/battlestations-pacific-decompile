# From the mission tree through the briefing to the load request

Addresses: 005C5600 0051DCE0 004CC460 005C57D0 00626930 00580940 005806A0 005C27E0 005C3850
005C3BE0 0058C010 005922F0 0058BDF0 0058D9D0 0051B7B0 004E2770

Packet `cc_mission_briefing`, worktree `agent/cc-mission-briefing`. Ghidra was read-only for this
packet; every name below is a hypothesis, not a recovered symbol. `004CC460` was leased to another
owner and is read here without being named or recorded.

`docs/MISSION_LOAD_PATH.md` covers the load from the request pair onward and
`docs/MISSION_TREE_BRIEFING_SCREENS.md` the two screen classes. This document covers the step
between them: what happens when the selected mission has a briefing, and what eventually queues the
requests the load path consumes. Nothing from those two is restated.

## The path, end to end

| # | Routine | Effect |
| --- | --- | --- |
| 1 | `005C57D0` | the mission tree's accept action; a locked mission does nothing |
| 2 | `005C5600` | pending scene, loading text, `00626930`, then the branch |
| 3a | `005C5769` | no briefing key: `game+6ACh` and `00439020` — the load starts here |
| 3b | `005C573F` | a briefing key: `0051DCE0` fills the briefing screen object, `004CC460(1, 0)` raises interface **1**, and **nothing is queued** |
| 4 | `0058C010` | the main-menu screen's page builder mirrors the mission-tree selection into `00E194D8`/`00E194DC` and shows page 9 |
| 5 | `005922F0` | the play action of that page |
| 6 | `004F8A20` / `0058D9D0` | with a `MovieName`, the movie runs first and its completion re-enters the start |
| 7 | `0058BDF0` | pending scene again, `00626930` again, the checkpoint write, then `00439020` |

Step 3b is the whole point: **the briefing arm of `005C5600` queues no state request at all.** It
sets the pending scene, publishes the loading text, resets the statistics block and then hands the
player a screen. The request pair `docs/MISSION_LOAD_PATH.md` starts from is queued one user action
later, by `0058BDF0`, which redoes the pending-scene work from its own copy of the selection.

## The briefing arm of `005C5600`

```
005C573A  CMP dword ptr [EBP + 0x4],EBX   ; EBP = record + side*154h + 0B8h
005C573D  JZ 005C5769                     ; empty briefing key -> launch now
005C573F  MOV EAX,[00E198AC]
005C5744  MOV ECX,dword ptr [EAX + 0x64]  ; the briefing screen object
005C5747  XOR EDX,EDX
005C5749  CMP byte ptr [ESI + 0xB8],BL
005C574F  SETZ DL                         ; the side index, recomputed
005C5752  PUSH EDX
005C5753  PUSH ESI
005C5754  CALL 0051DCE0
005C5759  MOV ECX,dword ptr [00E198AC]
005C575F  PUSH EBX                        ; payload 0
005C5760  PUSH 0x1                        ; interface id 1
005C5762  CALL 004CC460
005C5767  JMP 005C5798                    ; straight to the input reset
```

`0051DCE0` copies `record+08h` into briefing `+0C4h`, the `date` triple into `+0CCh`..`+0D4h`, and
from the side block the key at `+04h` into `+0D8h` plus the two string vectors at `+14h` and `+24h`.
That is the whole of its effect: it is a data copy into an object, not a screen change.

## Where the briefing surface actually is

`docs/MISSION_TREE_BRIEFING_SCREENS.md` left it open that "nothing in the image requests interface
3" and proposed a follow-up `interface_three_raiser`. This packet settles it: **the briefing surface
is a page of the main-menu screen, and the briefing screen object at `[00E198AC]+64h` is a data
holder.** Four independent facts:

- `005C5760` pushes `1`, `INTF_MAINMENU`, and it is the only interface literal on the path.
- `005861B0`, the main-menu screen's bind-layout virtual, binds `FE_briefing_grid` and the five
  `missions_*_Group` widgets. The briefing screen's own bind virtual `0051E280` binds
  `FE_briefing_listbox` and `FE_briefing_grid` too, but nothing raises it.
- `0058C010`, the main-menu screen's page builder, is reached from that screen's enter, update and
  back virtuals, and it sets `00E08874` to 9 (`MissionDetail`) while publishing the mission-tree
  selection into `00E194D8`/`00E194DC`.
- The briefing screen class's own play action, `0051B7B0`, is unreferenced (below), so the class has
  no way to start anything.

```
0058C066  MOV EAX,[00E198AC]
0058C06B  MOV ECX,dword ptr [EAX + 0x5C]   ; the mission-tree screen
0058C06E  MOV EDX,dword ptr [ECX + 0x10]   ; its selected group
0058C071  MOV dword ptr [00E194D8],EDX
0058C077  MOV ECX,dword ptr [EAX + 0x5C]
0058C07A  CALL 005C3870                    ; the selected record
0058C07F  PUSH EAX
0058C080  MOV EAX,[00E198AC]
0058C085  MOV ECX,dword ptr [EAX + 0x5C]
0058C088  CALL 005C3850                    ; its index, looked up by id
0058C08D  MOV dword ptr [00E194DC],EAX
```

`005C3850` is a four-instruction wrapper over `005C3470` that keeps only the mission index of the
pair. From here on the mission-detail page and everything it starts read `00E194D8`/`00E194DC`
through `005806A0`, not the screen's own `+0Ch`/`+10h`.

## `00580940`, the page rule

`__cdecl void()`, 62 instructions. The whole body is inside `if (game+1FE4h == 0)`, so a
non-campaign session leaves every global alone. Otherwise it writes the group index, the mission
index and `-1` into `00E08878`, and then picks the page:

| Condition | `00E08874` |
| --- | --- |
| selected group is 0 (`00580994`) | 8 |
| `005C3BE0` (`005809B6`) — every mission of the group is completed | 2 |
| group is 3 or 4 (`005809D7`) | `7 - (side_index != 0)`, stored at `005809F6` |
| otherwise | `5 - (side_index != 0)`, stored at `00580A12` |

`side_index` is `005C27E0`, whose entire body is `return record[0B8h] == 0`, the same rule
`mission_side_index` already states. `005C3BE0` resolves the record's id through `005C3470`, walks
that group and returns true only when `BSP_MissionProgress_IsCompleted` holds for every mission; an
empty group is vacuously true.

Its single caller is `0062B110`, in the after-action module, so this is the routine that restores
the menu's mission selection and page when a mission ends.

## `005922F0`, the play action

`__thiscall void(MainMenuScreen* this)`, ECX only, `RET`, SEH frame `00C70948`.

```
0059230D  the award tracker singleton, then 00690CD0
0059231B  record = 005806A0()
00592320  EDI = record + 58h              ; the MovieName string
00592325  TEST EAX,EAX                    ; its size dword
00592329  JZ 005923B3                     ; no movie -> start now
0059232F  005830A0(ECX = this)
0059233C  00A85C00(ECX = [00E198AC]+50h)
0059234C  the manager's own vtable +0Ch
0059235B  [movie+4h] = 1 ; [movie+5h] = 1     (movie = [00E18D48])
00592361  004F83B0(ECX = movie)
0059236D  movie vtable +18h
00592383  004F8A20(&record+58h, 1, [00CEFCB8], 0)
0059238E  [movie+30h] = 1
0059239C  004F8970(ECX = movie, 0058D9D0)
005923B3  0058BDF0(), then five empty strings through 0054B530
```

`record+58h` is the Lua key `MovieName` of `kMissionRecordSchema`, so the branch is "this mission
has an intro movie". `0058D9D0` is three instructions — `ECX = [00E198AC]+58h`, then `JMP 0058BDF0`
— and exists only to give `004F8970` a plain function pointer; the ECX it loads is dead in the
callee.

## `0058BDF0`, the start

`__thiscall void(MainMenuScreen* this)`, ECX dead, `RET`, SEH frame `00C700C8`. In native order:

| Site | Effect |
| --- | --- |
| `0058BE0C` | `record = 005806A0()`, `side = 005C27E0(record)` |
| `0058BE40` | `game+2198h` takes `record+00h`, the Lua `id` |
| `0058BE70` | `004E2770(scene, null)`; the scene is `record+24h`, or the empty literal at `00E19504` when null |
| `0058BEDE` | `0057D060` with the side block's `+64h` vector, `record+08h` and the low byte of `record+20Ch` |
| `0058BEFA` | `game+6B8h` takes `record+00h` again |
| `0058BF21` | `00626930(record)` |
| `0058BF26` | the difficulty rule |
| `0058BF67` | the checkpoint rule |
| `0058BF8E` | `00439020` |
| `0058BF99` | `004BEC00` then `00A92C40(0.0f)` |
| `0058BFA5` | the metrics call |
| `0058BFEF` | `004D2A80` |

It is `005C5600`'s twin, with three differences that matter:

- **The difficulty rule has an extra guard.** `005C5600` writes `game+6ACh` unconditionally:
  `record+0B4h`, or `game+6B0h` when that is 3. `0058BDF0` takes the player's value only when the
  byte at main-menu screen `+5Ch` is clear, and when the flag is set and the record asks for the
  player's choice it writes nothing, leaving whatever `game+6ACh` held.
- **It writes the profile.** With the same `+5Ch` byte clear, `007F8D60(ECX = game+650h, record)`
  compares the profile's checkpoint value for this record against the dword at profile `+98h`, and
  on a difference `00425C20` then `00437C70` runs the `BSP_Chk_Save` write.
- **It reports a metric.** When `00E1AED4` is non-zero and `game+1FE4h` is clear,
  `00753810((page == 8) ? 5 : 1, [main-menu screen +5Ch])`.

Because `004E2770` begins with `004BF930`, which destroys every existing scene record, the scene
record `005C5600` built for the briefing is **freed and rebuilt** here. Any pointer into it held
across the briefing is dangling; the load path's own note about `004BF930` applies to this pair of
calls, not only across missions.

## `00626930`

`__fastcall void(MissionRecord* record)`, ECX only, `RET`. It copies `record+08h` (the Lua `name`)
into the string at `00E19798` and `record+38h` (`debriefingText`) into the string at `00E197A0`,
then clears twelve containers in the block at `00E196CC`..`00E19797`:

| Containers | Callee |
| --- | --- |
| `00E196CC`, `00E196DC`, `00E196EC`, `00E19768`, `00E19778`, `00E19788` | `004954F0`, `erase(begin, end)` |
| `00E196FC`, `00E19710`, `00E19724` | `005F6190`, a `14h` object reset in place |
| `00E19738`, `00E19748`, `00E19758` | `006226F0`, `erase(begin, end)` on another element type |

Each erase takes the container's own `_Myfirst`/`_Mylast` pair through the iterator-debug bound
check, so every one of them is a full clear. Four call sites run it — `004DFB70`, `0058BDF0`,
`005C5600` and `005DECA0` — which is the set of places a mission begins.

## `0051B7B0` is unreferenced

`docs/MISSION_TREE_BRIEFING_SCREENS.md` records `0051B7B0` as "the play action" of the briefing
screen. Its body does write the difficulty pair and does call `00439020`, but **nothing in the image
reaches it**:

- No relative `CALL` or `JMP` in `.text` targets any address in `0051B7B0`..`0051B88C`. The check is
  a linear scan of every `E8`/`E9` opcode in the section with the displacement resolved; the same
  scan finds exactly the three known call sites of `00439020` and the three known call sites of
  `0051B450`, so it is not missing edges.
- The four bytes `B0 B7 51 00` do not occur anywhere in the file, so no vtable, dispatch table or
  `MOV r32, imm32` holds its address either.

The briefing screen's translation unit spans `0051A0E0`..`0051E730` and its other members are
referenced, so the linker kept the whole section; this routine is compiled in and uncalled. The
reachable play action is `005922F0`.

Its selector, which the earlier packet could not read, is a **stack argument**:

```
0051B857  CMP byte ptr [ESP + 0xC],BL   ; entry ESP + 4 behind PUSH EBX / PUSH ESI
0051B864  JZ 0051B86D                   ; zero -> 0051B450(0,0), the help line
0051B866  CALL 00439020                 ; non-zero -> the mission-start pair
0051B88A  RET 0x4
```

so it is `__thiscall void(BriefingScreen* this, bool start_now)`. Ghidra's decompiler drops the
`0051B866` block as unreachable because the prototype it stores has no stack parameter; the stored
listing is complete.

## Host methods the executable must implement

`bsp::mission_briefing_host_steps` in `src/mission_briefing_start.cpp` is the machine-readable list:
36 steps in call order with the native call site, the host interface, the method and an owner. The
three interfaces, in the order a briefing start runs them:

| Order | Host | Methods | Owner |
| --- | --- | --- | --- |
| 1 | `MissionBriefingPlayHost` | `flush_award_tracker`, `suspend_page_for_movie`, `stop_front_end_audio`, `apply_pending_interface`, `arm_movie_surface`, `commit_movie_visibility`, `enter_movie_surface`, `play_mission_movie`, `mark_movie_active`, `set_movie_completion`, `start_selected_mission`, `clear_help_line` | gui, audio, movie |
| 2 | `MissionStartHost` | `set_current_mission_key`, `set_pending_scene`, `publish_loading_config`, `set_mission_key_mirror`, `reset_mission_stats`, `main_menu_flag_5c`, `chosen_difficulty`, `set_effective_difficulty`, `checkpoint_differs`, `write_checkpoint`, `request_mission_start`, `reset_front_end_timer`, `metrics_enabled`, `non_campaign_session`, `current_page`, `report_mission_start_metrics`, `finish_start` | game state, gui, profile, metrics |
| 3 | `MissionStatsResetHost` | `set_stats_mission_name`, `set_stats_debriefing_text`, `clear_stats_container` | scoring |

Only three of the 36 steps need a subsystem nobody has reconstructed yet: the movie player
(`004F8A20`, `004F8970`) and the metrics sink (`00753810`). `set_pending_scene` is already
`run_set_pending_scene_004e2770` of `bsp/mission_load_path.hpp`, `reset_mission_stats` is
`run_mission_stats_reset_00626930` in this packet, and `request_mission_start` is the pair
`mission_start_state_requests_00439020` already returns.

## Corrections

- **`include/bsp/mission_load_path.hpp`: the record's title and subtitle were named one slot
  apart.** `kMissionLoadRecordTitleOffset = 0x0` and `kMissionRecordSubtitleOffset = 0x8` are now
  `kMissionLoadRecordIdOffset = 0x0` and `kMissionLoadRecordNameOffset = 0x8`, and
  `MissionTreeSelection::title`/`subtitle` are now `id`/`name`. The producer settles it:
  `kMissionRecordSchema` of `bsp/mission_tree_data.hpp` reads the Lua key `id` into `+0h` and
  `name` into `+8h`. So does every consumer — `005C56C9` copies `+0h` into `game+2198h`, which
  `005C3870`'s non-campaign arm matches against record `+0h` again, and `0051DCE0` copies `+8h`
  into the briefing's title field at `+0C4h`. `include/bsp/mission_tree_screens.hpp`'s
  `kMissionRecordNameOffset = 0x00` / `kMissionRecordTitleOffset = 0x08` were already in the right
  places and are unchanged; only the load-path header was wrong.
- **`include/bsp/mission_tree_screens.hpp`: `set_pending_scene`'s second parameter was `int flags`.**
  It is the weather-descriptor override name forwarded to `0046DF00` argument 5, and `005C5682`
  pushes null. Retyped to `std::string_view weather_override`; `src/mission_tree_screens.cpp` passes
  an empty view. This closes the `mission_tree_host_fix` follow-up of `docs/MISSION_LOAD_PATH.md`.
- **`00626930` has no `side` argument.** `docs/MISSION_TREE_BRIEFING_SCREENS.md` and the ledger
  evidence for `005C5600` both describe it as `00626930(ECX = record, EDX = side)`. `005C5732` does
  load EDX with the side index, but `00626930`'s prologue is `SUB ESP,8 / PUSH EBX / PUSH ESI /
  PUSH EDI / MOV EDI,ECX` and the first write to EDX is at `0062695D`, so the value is dead. The
  same is true at `0058BDF0`'s call site. Recorded in the ledger against `00626930` and `005C5600`.
- **`docs/MISSION_TREE_BRIEFING_SCREENS.md`: `0051B7B0` is not the reachable play action.** Its
  content is described correctly there, but nothing calls it; see the section above. The doc's
  "single largest open question", that nothing requests interface 3, has the same answer: the
  briefing is a main-menu page.
- **`0051B8C0`'s stored body is short.** Ghidra ends it at `0051B90D`, but the routine continues at
  least to `0051B9CA`, where it calls `0051B450`, and the next function starts at `0051B9E0`. The
  decompiled 14 lines are therefore only its first arm. Not repaired here; Ghidra is read-only for
  this packet.
- **`00E194D8`/`00E194DC` versus the mission-tree screen's own indices.** `005C5600` reads the
  record through `005C3870` (screen `+0Ch`/`+10h`); `0058BDF0` reads it through `005806A0` (the two
  globals). They agree only because `0058C010` copies one into the other. Any reconstruction that
  models "the selected mission" as one value has to keep both, because `0058C010` derives the
  mission index by a **name lookup** (`005C3850`), which `005C3470` resolves by keeping the *last*
  match across all groups, while the group index is copied verbatim.

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `main_menu_mission_detail` | 0058C010 0058CAC3 005861B0 | docs/MAIN_MENU_MISSION_DETAIL.md | The 1040-instruction page builder: which widgets the mission-detail page binds, what the four campaign pages differ in, and where the `+5Ch` byte `0058BDF0` reads is written |
| `main_menu_page_names` | 00580940 00597870 00598B60 | include/bsp/main_menu_screens.hpp | `00580940` pairs the pages as `{4,5}` and `{6,7}` with the side index picking the lower one, so an `allied` mission takes 5 or 7 and a `japanese` mission 4 or 6. That is the opposite pairing to the current `CampaignUsn`/`CampaignUsnDlc`/`CampaignIjn`/`CampaignIjnDlc` names, and `Page08` is still unidentified |
| `briefing_second_filler` | 005E49C1 005E4478 | docs/BRIEFING_SECOND_FILLER.md | A routine in the gap `005E4478`..`005E4FC0`, which Ghidra has no function for, calls `0051DCE0` at `005E49C1`. It is the only other filler of the briefing screen object and its screen is unidentified |
| `mission_statistics_block` | 00626930 004954F0 005F6190 006226F0 | docs/MISSION_STATISTICS_BLOCK.md | What the twelve containers at `00E196CC`..`00E19797` hold and who reads the two strings at `00E19798`/`00E197A0`, which no direct reference reads |
| `mission_start_metrics` | 00753810 00E1AED4 | docs/MISSION_START_METRICS.md | The metrics sink and what its two arguments mean |
| `profile_checkpoint_write` | 007F8D60 00437C70 007FA1B0 | docs/PROFILE_CHECKPOINT.md | The map at profile `+94h`, the value at `+98h`, and the `BSP_Chk_Save` write |

## no_ghidra_function

| Start | End | Role |
| --- | --- | --- |
| 0058D9D0 | 0058D9DC | the movie-completion thunk: `ECX = [00E198AC]+58h`, then `JMP 0058BDF0` |

`0051B250`..`0051B44F` (the briefing's `00CEC934` slot `+08h` handler, SEH handler `00C6AA80`) and
`005E4478`..`005E4FBF` are also gaps with no Ghidra function, but this packet names neither: the
first was read only as far as its list walk, and the second only far enough to see its call to
`0051DCE0`.

## Uncertainties

- **The byte at main-menu screen `+5Ch`** gates the difficulty inheritance, the checkpoint write and
  one metrics argument, and is written somewhere in `0058C010` or its neighbours, which this packet
  did not read. It is carried as `main_menu_flag_5c`, deliberately unnamed. It is **not** the
  downloadable-content flag, which `00598BA6` writes at `+55Ch`.
- **`004F8A20`'s third argument** is the float at `00CEFCB8`, read but not identified; the
  reconstruction records the movie name and drops it.
- **The twelve containers `00626930` clears** are distinguished only by their clear routine. Element
  types, and therefore what the block counts, are not recovered.
- **`005C3BE0`'s completion source.** `BSP_MissionProgress_IsCompleted` is taken as a predicate over
  one record; its own body was not read, so the reconstruction takes the flags as an input.
- **Whether `0058BDF0` ever runs without `0058C010` having run first.** All three reachable call
  paths (`005922F0` direct, `005922F0` through the movie, and `0058D9D0`) come off the mission-detail
  page, but `005806A0` reads globals that `00597870` and `00598B60` also write, so a page change
  between the two could change the record under the start.
- **`005DECA0`**, the fourth caller of `00626930`, is in the in-mission module and was not read.

## Reconstruction

`include/bsp/mission_briefing_start.hpp` and `src/mission_briefing_start.cpp`. The page rule, the
selection lookup, the side index and the group-completion walk are pure functions;
`run_mission_stats_reset_00626930`, `run_briefing_play_005922f0`,
`run_start_selected_mission_0058bdf0` and `run_briefing_activate_play_0051b7b0` are sequences over
injected hosts with one pure-virtual method per native call site. The record type is
`MissionRecordData` of `bsp/mission_tree_data.hpp`, reused rather than redefined. Build-tested only:
nothing here is an ABI-compatible replacement, and no fixture exercises it.
