# CC13 seg59c: segment 59 leftovers (packet `cc13-seg59b`)

Segment 59 (`008EE670..00922240`; gun bots, the torpedo / depth-charge / tail-gunner / pilot
bot rows, bullet throw and angle-error terms). Code: `include/bsp/cc13_seg59c.hpp`,
`src/cc13_seg59c.cpp` (marker `cc13_seg59c`). Nothing is bound into `bsp_game`.

## Census (index of 2026-10-19, main 32b1ed132)

- 571 candidates; 232 carry a reconstruction record (cc13-seg59A / cc13-seg59b, cc9 gunnery and
  robot-config hosts, earlier cc13 files); 339 carry none.
- Of the 339, two are thunks and 329 are tagged library (`STL_*`, `CG_*` compiler thunks and
  deleting destructors, `Catch_All`, xlen throw sites). They stay bound, not rebuilt.
- The remaining eight are this packet: 0090E6C0, 008FB520, 00905370, 00920EC0, 008FACC0,
  009080A0, 00908120 here, and 00910F10 (`std::map<uint32, vector<float>>::operator[]`, already an
  interface method in `cc13_seg59b.hpp`; library, left bound).
- The bot gameplay of the segment (008FFF20 torpedo tick, 00901BA0 attack range, 00903420 target
  size, 00902290 turn lead, 008FFA20 aim error, 009030C0 ballistic correction, 008FEF40 trigger,
  00902920 AA fire byte, 008FDBE0 flak roll error, 00901C20 intercept time) is hosted in
  `game_hosts_gunnery.cpp` and audited in docs/CC13_GPR_40.md, docs/CC13_VER_23.md,
  docs/GUN_BOT_TICKS.md and docs/GUN_BOT_REMAINDER.md. Nothing new to adopt; no URGENT item.
- The four scoring getters 0090C800 / 0090C870 / 0090C8E0 / 0090C950
  (`existing_host_partial_lookup_unimplemented`) are covered by `scoring_get_slot_keyed_score`
  in `src/scoring_bindings.cpp` over the host's map lookup; no second copy.

## Routines

| Address | C++ | ABI | Evidence |
|---|---|---|---|
| `0090E6C0` | `damage_control_note_unit_in_slot_list_0090e6c0` | `__thiscall(ECX = manager, unit)`, RET 4 | Word id at unit+174h, slot at unit+180h (unsigned, `CMP EAX,7 / JA` 0090E6D2). List = manager + slot*284h + 140h, a `vector<uint16>` (+4h first, +8h last). Linear search; on a match return; else `007265E0` push_back of the argument slot holding the id (0090E724). Reached only by tail jumps at 0093A4DD and 0093A55F (rel32 scan), the repair task's add-fire / add-water seconds, with ECX = game+21A0h (docs/SHIP_FIRE_FLOODING.md). Tagged `stl_probable`, but it is game code. |
| `008FB520` | `zero_float_008fb520` | `__thiscall`, RET, returns ST0 | `FLDZ / RET`. One caller, 00959F12 in BSP_Unit_ApplyGunAimMessage, ECX = [esi+394h]; the result is stored with FSTP. |
| `00905370` | `scoring_noop_00905370` | `__thiscall`, RET | `RET`. Caller 0076D1CC (ServerSendSceneScoring), ECX = [eax+21A0h]. |
| `00920EC0` | `construct_world_noop_00920ec0` | `__thiscall`, RET | `RET`. Caller 004DF9D9 (BSP_Game_ConstructWorld) with ECX = the new game+21CCh object: 004DF9C1 constructs it with 009221E0, `MOV ECX,EAX` at 004DF9CA, stored at game+21CCh by 004DF9D3. ECX is 0 only when the allocation fails (004DF9BD `JE 004DF9C8`, `XOR EAX,EAX`). |
| `008FACC0` | `clear_three_vectors_008facc0` | `__fastcall(ECX = object)`, RET | erase(first, last) on the vectors at +14h (008FA1A0), +24h (008FA220), +04h (008F9FD0), in that order, each after a doubled first <= last check (00BF6713). |
| `009080A0` | `map_all_node_floats_zero_009080a0` | `__fastcall(ECX = map)`, returns AL | In-order walk (00581290); `UCOMISS [node+20h], [00D7A218]` (0.0f); `LAHF / TEST AH,44h / JP 00908104` sends both "not equal" and unordered (NaN) to return 0; the end returns 1. |
| `00908120` | `map_sum_node_int_1f4_00908120` | `__fastcall(ECX = map)`, returns EAX | In-order walk; sums the dword at node+1F4h (00908167). |

008FACC0, 009080A0 and 00908120 have no reference in the image: Ghidra xrefs are empty, and a
rel32 (E8/E9) plus absolute-dword scan of `.text` found none. The same scan found the known
callers of 0090E6C0 and 00920EC0, which controls for a vacuous negative. They are dead code, kept
for completeness.

## Uncertainty

- The element types of the three vectors in 008FACC0 and the map value types in 009080A0 /
  00908120 are not established; the erase and iterator routines are host methods.
- 0090E6C0's list meaning (which slot list the damage-control warning feeds) follows the callers;
  it is not cross-checked against a reader of manager+140h.
