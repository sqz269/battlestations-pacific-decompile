# CC13 segment 52, second pass (008288D0-00851CB0): shipyard, stationaries, squadron block

Worker cc13-seg52b, 2026-10-17, after the first pass (docs/CC13_SEG52.md). The struck ranges
00836000-00837FFF and 0084E000-0084EFFF (live cc9 packets) were not leased, recorded or
reconstructed; functions there are only called through hooks (008363E0, 008360C0, 0084EE80,
0084EFE0, 0084E640). 0084DDA0 BeginCommand is left to cc9 as an adoption candidate.

New code: `src/cc13_seg52b.cpp`, `include/bsp/cc13_seg52b.hpp` (namespace `bsp::cc13_seg52b`).
`Seg52bHost` extends `cc13_seg52::Seg52Host`, so the first pass's hooks and functions
(`native_string_release_storage_0084f930`, the Interlocked and vtable hooks) are reused. Every
call the image makes is a named hook at the same point. Status for every row: reconstructed and
build-tested, not ABI-compatible, not fixture-tested, not wired, not game-validated. Names are
hypotheses.

## Reconstructed

| Address | C++ | Notes |
|---|---|---|
| 00843000 | `format_to_buffer16_00843000` | `_vsprintf_s(dest, 10h, format, va)`. |
| 0082A160 | `ref_ptr_assign_0082a160` | Release old (slot = 0), store new; no increment, no self test. |
| 0082A590 | `ship_indexed_point_array_push_back_0082a590` | Grow max(cap*2, 1) via 00829180; 10h element. Also covers the identical 00829CD0. |
| 0084B6B0 | `los_sweep_with_ignore_0084b6b0` | vtable[0B0h] of the ignore object, then 0098B370 on the 0042E630 index. |
| 0082DC30 / 0082DC80 | `vector_holder_construct/destruct` | RefCounted base, vptr 00D099B0, empty vector; destruct 0082CAD0(+8) then 00BD30F0. No references found. |
| 00843460 | `shipyard_accepts_command_token_00843460` | settarget / clearorders / cleartarget, else 00779D50. |
| 00842A80 | `shipyard_weapon_director_00842a80` | Returns +7A0h. |
| 008429E0 | `shipyard_create_weapon_director_008429e0` | 0077E830, new(240h) + 008363E0 into +7A0h, 0095A880(0, 1, 0) when the record kind is 1. |
| 008397A0 | `settings_keyed_string_copy_construct_008397a0` | Key, NativeString, float, byte. |
| 00839800 | `settings_record30_copy_construct_00839800` | String, scalars, three increments. |
| 00839490 | `settings_record30_assign_00839490` | String (not for self), scalars, swap-with-release per pointer. |
| 008486B0 / 00848D00 / 00848D70 | stock record copies | vptr 00CF69A8, +4/+8, 008484A0 name vector, +1Ch; 00848D70 is the checked operator[]. |
| 008485F0 | `shipyard_stock_record_construct_008485f0` | Names split on "," by 0094EDC0 from a stack std::string. |
| 00846970 | `shipyard_slot_copy_construct_00846970` | vptr 00CF6544, string via 00408120, observer 00796BD0 then vptr 00CE3CF0. |
| 0084DA20 | `squadron_block_accept_visitor_0084da20` | No Ghidra function; slot 00D0BDD4 tail-calls visitor vtable[0](block). |
| 0084D980 | `squadron_block_set_fire_and_move_0084d980` | Squadron IsKindOf(17h)/(13h) choose the vtable[40h]/[44h] arguments. |
| 0084DA60 | `squadron_block_target_still_valid_0084da60` | Override of 0071EDD0; flags [[+38h]+3Eh] when the target is unchanged, unless session mode 2. |
| 0084B6F0 | `spawn_oriented_point_effect_0084b6f0` | Look-at unless the direction is exactly zero; 00868420 with a RefPtr copy; byte +9 = 1. |
| 0084C430 | `projectile_sweep_step_segment_0084c430` | One-segment 0C4h list, count 1, forwarded to 0084BF00 with CL/EDX untouched. |
| 00845670 | `shipyard_exit_point_00845670` | Hangar path point via 007AF800, or position + row * 100.0. No references found. |
| 00848080 | `shipyard_construct_00848080` | 0095CC90, eight vptrs, vectors zeroed, class id 46h. |
| 00848380 | `shipyard_allocate_00848380` | new(7A4h), memset, 00848080, 009553D0, +354h. |
| 0084F670 | `stationary_class_destruct_0084f670` | vptr 00D0BE38, two RefPtrs, two NativeStrings. |
| 0084FB80 | `stationary_destruct_0084fb80` | Registry erase (0084FA30), delete +19Ch, tail 00923060. |
| 0084F7F0 | `stationary_apply_fort_class_0084f7f0` | The land-fort class fields; default point 00F87574. |
| 00850930 | `stationary_class_map_destroy_all_00850930` | Delete each class, 008501D0, head reset. |
| 0084FCE0 | `stationary_class_load_explosion_resource_0084fce0` | Resource +18h and the "explosion" point into +2Ch..+34h, once. |
| 00846640 | `shipyard_get_property_stock_00846640` | The "stock" GetProperty table of {classid, count}. |
| 00851580 | `stationary_class_read_lua_fields_00851580` | Mesh, HPBlack, Armor, Height, ExplosionEfx (+28h handle), FakeExplosionGroup. |
| 00851850 | `stationary_on_wrecked_00851850` | Registry push, resource pointer array, session message 41h, wreck effect and announcement. |
| 00845C10 | `shipyard_describe_state_00845c10` | _shipyard / stockVec / hangarVec / slotVec / gameUnit writer dump. |
| 008509F0 | `stationary_sentity_init_008509f0` | Part instance, bag flags, scene node set-up, decal owner, shape descriptors and static body. |
| 00851CB0 | `stationary_class_get_or_create_00851cb0` | Map lookup or Lua-built class (inline ctor, vtable[8] read, vtable[10h]); vtable[0Ch] always. |
| 008437D0 | `shipyard_apply_spawn_message_008437d0` | Slot state 3, hangar bind, inline handle resolve, 00694A60, vtable[0D8h]. |

Simplifications and uncertainties (also in the C++ comments):
- 00843000 forwards a `va_list` where the image passes the address of the first variadic.
- 0084B6F0 and 00851580 initialise their out RefPtr locals to 0; the image passes unwritten
  locals that the callee fills.
- 00845670 and 00851850 use the project's float/double convention for the x87 arithmetic;
  the bytes of each FMUL / FDIVRP were checked (DC C9, DE F1, DE CA, D8 C9).
- Checked-iterator tests (00BF6713) are kept at every point the image makes them.
- The plane squadron command block methods 0084D980 / 0084DA60 / 0084DA20 sit next to cc9's
  0084DDA0 / 0084E010; they are target-validity and fire/move enables, not level-bomb or goaway.

## Existing code, ledger only

- 0082A8F0, 0082B090, 0082A210: byte-identical to 00817BD0 (`ref_ptr_release_00817bd0`,
  src/cc13_seg30_seg50.cpp). 008297F0: identical to 00815060 (`ref_ptr_release_00815060`).
- 0082A550: identical to 00467F60 (`pod_pointer_array_push_back_00467f60`,
  src/engine_seg4_6_9_cc13.cpp).
- 00829CD0: identical to 0082A590 (this file).
- 00846C00 MShipyard IsKindOf: `unit_kind_body_answers` row 46h (src/unit_kind_query.cpp)
  answers the same set (5, 4, 2, 1, 0, +0C4h, 46h).
- 00848410: hosted by cc9 as `shipyard_hangars_usable` (src/game_hosts_gunnery.cpp); filter
  verified (+5Ch set; +5Dh, +60h, +5Eh clear), no mismatch.

## Skipped

- Library-dominated STL members: 0082DF00 / 0082E280 / 0082E5A0 / 0082E1F0 (idle record with
  two std::map members), 0082F8E0 / 0082FD20 (std::vector resize), 008390C0 / 00839160
  (checked table lookups), 00846A80 (three vector tidies), 0084FA30 (vector erase), 0084D480
  (std::list copy), 008482F0 / 00847FE0 (std algorithm over 1Ch-byte elements), 0084FC50
  (tree node construct).
- 0082D040 BuildBuoyancyElements: hosted by cc9 (src/ship_buoyancy_elements.cpp,
  game_hosts_units.cpp); not re-verified here (6C0h of x87 code).
- 0082FE30 BindModelData: 19D1h bytes, too large for this pass; only 0082FEA9..0082FEE8 has
  been read by anyone (ship_buoyancy_elements.hpp).
- 0084DDA0 BeginCommand: left to cc9 (see the packet brief).
- The engine container instantiations of the first pass's list are now covered above.
