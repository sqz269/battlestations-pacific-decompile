# cc13-sweepB11: large leftovers of the cc13 sweep

Packet cc13-sweepB11 (2026-10-18). Code: `include/bsp/cc13_sweepB11.hpp`, `src/cc13_sweepB11.cpp`.
`Cc13SweepB11Host` extends `Cc13SweepB10Host`; nothing is wired into a host. Names are hypotheses.
Status: build-tested only (no fixture, not ABI-compatible, not game-validated).

## Packet triage (rule 1)

Eight of the nine listed bodies already had a reconstruction and a ledger record from later cc13 batches,
so no second copy was written:

| Address | Existing reconstruction | Record status |
|---|---|---|
| 0060ABD0 | `cc13_bigB1_hud_plane_effects_update_0060abd0` (src/cc13_bigB1.cpp) | reconstructed_build_tested |
| 007A91A0 | `cc13_bigB1_movcam_step_007a91a0` (src/cc13_bigB1.cpp) | reconstructed_build_tested |
| 00670020 | `cc13_bigB2_hud_support_refresh_view_00670020` (src/cc13_bigB2.cpp) | semantic_build_tested |
| 00627960 | `cc13_bigB2_mission_objectives_rebuild_00627960` (src/cc13_bigB2.cpp) | semantic_build_tested |
| 005DB050 | `cc13_bigB2_lobby_bind_widgets_005db050` (src/cc13_bigB2.cpp) | semantic_build_tested |
| 00822C20 | `cc13_bigB1_unit_sentity_init_00822c20` (src/cc13_bigB1.cpp) | reconstructed_build_tested |
| 005A3210 | `cc13_sweepB10_hud_map_update_pointer_pick_005a3210` (src/cc13_sweepB10.cpp) | reconstructed_build_tested |
| 00617470 | `cc13_sweepB10_powerup_update_bars_00617470` (src/cc13_sweepB10.cpp) | reconstructed_build_tested |

00776930 had only an `analyzed_library_dominated_not_reconstructed` record (docs/CC13_SWEEPB10.md). It is game
code, so it is reconstructed here with the inlined std::list and string work kept at the image's points.

## 00776930 BSP_SessionHostInfo_CheckDlc (reconstructed)

`cc13_sweepB11_session_check_dlc_00776930`, 1125 instructions, vtable 00D039CC slot 4.
ABI: `__thiscall(ECX = host_info; client, unused, WideString* out)`, RET 0Ch, SEH frame (00C89CB0), 598h-byte
frame. The second stack argument is never read.

Behaviour, in the image's order:

1. missing = (host_info +170h / +174h) AND NOT (client +48h / +4Ch), per 32-bit half.
2. A private LuaStateOwner (00B66BD0, 00B6A020 with mask 4) runs `SCRIPTS/datatables/DLC.lua` (00B69D40, 0);
   table = globals (00B67980) field `DLCTable` (00B67800); the globals object is destroyed (00B67700); key and value
   objects are constructed (00B65F50).
3. A group list (0076E970 head, then 00776540 clear) gets, for each missing bit i in 0..63 (__aullshr), a copy of an
   empty record list (0076E990, 007767F0, 007736A0, then 00774050 and operator delete 00BF65AC on the temporary).
   Every DLCTable entry (00B67080 / 00B66420 / 00B67190) whose value string (00B662B0) has '1' at [63 - i] appends a
   zeroed record (007740B0, 00773740) and fills it: mask = the 64-character string read MSB first (__allshl adds), name
   = the key string through the inlined assignment (00419CC0 / 00BD1120 allocate, 00BD1510 release, memcpy).
4. A result list (0076E990) takes one record per group: a group with size <= 1 copies its first record (with size 0
   the image reads the head, after the checked-iterator call). Otherwise the first two records are copied to
   temporaries, diff = |first.mask - second.mask| (unsigned 64-bit), a zeroed record is appended and set to the first
   when (diff & first.mask) == (diff & missing) in both halves, else to the second.
5. 007765A0 (std::list::sort, 26-bin merge sort) and 00774280 (unique by name) run on the result list.
6. Only the first result record is named (there is no loop): key = 0043C130("globals.", name) + "_" (00CE7890) +
   the client's C string at +32h (004261A0 twice). The key is resolved through 00A9FAD0 on [00F8BC4C]; when the
   resolved text starts with the key (0076E7F0 at 0, against the key widened by 004C5E60), the list gets
   "globals." + name + "_english|.,|", else key + "|.,|".
7. out = 00A9FAD0 of the list without its last 4 characters (00469840 count = length - 4, which wraps when the list
   is empty). When out is longer than 100 units it becomes its first 90 units (00770E50) + "..." (005CF570), copied
   back by the inlined wide assignment.
8. Teardown: the resolved wide string, the list string, both lists (clear, operator delete of the head), the three
   LuaObjects (value, key, table) and the LuaStateOwner (00B669A0).

The checked-iterator failure 00BF6713 is called at each of the image's checks (the image continues after it).

Simplifications and uncertainty:
- The frame objects are byte blocks of the sizes the frame gives (LuaStateOwner 4CCh bytes, LuaObject 14h, list 0Ch,
  record value 10h); the list helpers, sort and unique are host hooks, not modelled.
- __aullshr (00BFAED0) and __allshl (00BF9640) are written as C++ 64-bit shifts; every count is 0..63.
- The empty C string 00F1B03C passed to 004C5E60 for a null key is written as "".
- What +32h of the client holds is inferred from its use as the language suffix of a localization key.
- No x87.
