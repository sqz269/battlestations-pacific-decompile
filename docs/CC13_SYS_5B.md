# CC13 pass over segment 56, second packet (cc13-sys5b)

Packet cc13-sys5b, lineage cc13, continuing `docs/CC13_SYS_5.md` (packet cc13-sys5) on the same
files: `include/bsp/engine_seg56_cc13.hpp`, `src/engine_seg56_cc13.cpp`. Lease
`00890000-008DCD40`. Every name is a descriptive hypothesis, not a recovered symbol. Nothing is
ABI compatible, game-validated or game-run.

## Records

| Address | Native | C++ |
| --- | --- | --- |
| 008B0C10 / 008B0E10 / 008B1010 | MissionNarrative / Urgent / Override | `lua_mission_narrative_post` |
| 00898C60 | Message | `lua_message` |
| 008C8390 | DebugTrap | `lua_debug_trap` |
| 008D0EA0 | OverrideMessage | `lua_override_message` |
| 008A13D0 | SetVisibility | `lua_set_visibility` |
| 008C0F90 | FakePing | `lua_fake_ping` |
| 008D0430 / 008D0060 / 008C89D0 | HackPressInput chain | `lua_hack_press_input`, `hack_input_queue_press`, `make_hack_input_press` |
| 008D9750 | SceneTokenizer_ConvertByUnitToken | `scene_convert_by_unit_token` |
| 008DC210 | MapPingEntry_DescribeProperties | `map_ping_entry_describe_properties` |
| 008DA100 | NavPoint_SEntityInit | `nav_point_sentity_init` |

All are build-tested and recorded, and their naming-ledger evidence points at the reconstruction.

## The variant-argument path

The narrative bindings and Message do not open a LuaStateOwner frame. They build a
LuaStateHolder (006B7E30) and a script override list (008875F0), then collect the frame as
14h-byte variant records: 00887120(-1, 4) takes every argument and 00887370(first, -1, 4) takes
stack slots `first..top` (1-based). The 4 is the table-expansion depth of 00886E00
(`include/bsp/mission_named_call_args.hpp`). A string variant keeps a pointer to its native
string at record +8h. The narrative listing (008B0D93..008B0DAC) passes variant 0's +8h dword as
the text pointer, copies variant 1's string as the callback, and gives variants 2..n to a new
0Ch-byte vector (006EDF00 per record). It does that only when there are at least three
arguments. The cc9 host in `src/game_hosts_script_orders.cpp` reached the same reading for
MissionNarrative and Countdown. The models pass the extras to the host as
`(LuaArgs&, first index)`, and so does `read_movcam_tail` from cc13-sys5.

Message passes arguments 1..n (00887370(2, -1, 4)) to 009291D0 on the entity in argument 0.
BSP_MissionLua_PublishInputEvent and BSP_MissionEntity_PublishLuaDead use the same publisher.

## Notes from the listings

- DebugTrap calls `debug.traceback` (00D153C8) through 00887750 on the mission Lua host
  game+1A08h. It then hands the script's own string (when given) and the traceback text to
  004254B0 as format strings, not as `%s` arguments.
- OverrideMessage iterates argument 0 whenever it is not a string. There is no table test.
- HackInputPress: +0 is the duration float (`MOVSS [ESI]` at 008C89DF) and +4 is the float
  clock 00F876A4 plus the delay. The record list sits under game+5BCh, keyed by the input id.
- ConvertByUnitToken uses the rows 008D9150 appends (`include/bsp/native_game_tables.hpp`). A
  source-then-target match does not consume the token: it copies it to +405h and clears +805h.
  A target-then-source match consumes it. The reciprocal branch computes `(1 / value) / factor`
  in x87 extended precision; the model uses double, which can differ in the last float bit.
- MapPingEntry_DescribeProperties writes 8-byte `(tag, payload)` pairs by value. Key tag 0 is a
  name and 1 an index. Value tags: 0 string, 1 int, 3 bool, 4 entity, 5 vector.

## Correction to cc13-sys5

`SetTipEffect` 0089E670 walks +3CCh / +3D0h of its object. cc13-sys5 named these a plane's
engine count and engine array. SetVisibility walks the same pair only after the object's
vtable+5Ch(18h) answers true, and treats a vtable+5Ch(5) object as a single plane. So the pair is
a squadron's member count and its five member slots. The host methods were renamed
(`squadron_member_count_3cc`, `squadron_member_3d0`, `kSquadronMemberSlots`) and the 0089E670
record evidence was replaced. The ledger name of 00951F40, `BSP_Plane_SetElevatorAttachment`, does
not fit its use here (a per-plane visibility flag); it was left unchanged for its owner to review.

## Left out

- `class_enumeration` 008C8560: it writes the local side's party row index
  (*(game+18CCh+side*4)+28h) and destroys the world containers while loading every vehicle
  class. That is gameplay state, so it goes to cc9.
- `Pause` 008AFD50 and `Loading_Start` 008CC850: they reach the simulation gate 004CD0F0.
- `MovCam_PathWalkPathPointPropertyPotlekokkal` 008B4DA0: the setter 007A74F0 and the broadcast
  0076D5B0 take stack slots that were not traced to arguments.
- `SetVisibilityTerrainNode` 008C3170: the second lookup goes through 00B6D800 /
  00B6E750 on the found node's +30h, and the meaning of the name header was not established.
- `MultiLobbySettings_Destroy` 008D3E00 (nested tree walks freeing per-node buffers).
- `thunk_008DB510` 008DBAA0: it jumps into 008DB510, which the index puts inside
  `STL_PointerList_AllocateSentinel_008db4f0`. The containment needs checking first.

The read-only gameplay queries and the gameplay-writing bindings listed in
`docs/CC13_SYS_5.md` are unchanged.

## Handoff

Branch `agent/cc13-sys5b`. Every non-gameplay item of the cc13-sys5 open list is done except
the six above.
