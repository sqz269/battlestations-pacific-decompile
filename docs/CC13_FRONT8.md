# cc13-front8: wiring frontier, round 3

Worker cc13-front8, branch `agent/cc13-front8`, base main 1f810c9aa. All bodies live in
`src/cc13_front8.cpp` / `include/bsp/cc13_front8.hpp` (`bsp::cc13_front8`), registered on
`bsp_core` through `cmake/startup.cmake`. Each one is reconstructed from assembly with one host
hook per call target. They are build-tested but not wired into any host, and none is game-validated.
Descriptive names are hypotheses.

## Review minors (front7)

| Item | Change |
|---|---|
| 0061B0F0 | X (`[EBX+20h]`, the party slot +2Ch) is now re-read at 0061B35D (the 00AB64C0 `[X+D4h]` argument) and at 0061B3A8 (X+70h for 006171B0). The re-reads come after the 00614060/00AB64C0 calls, as in the image. |
| 004D0640 | Ledger and doc wording: the comparator is an empty-string test followed by `_stricmp`, with no length compare (004D0661-004D0690). |
| 00AB5C60 | Renamed to `gui_icon_construct_00ab5c60` to match main's `gui_icon.hpp` / `GuiWidgetType::Icon`. No caller used the old name. The front7 record was renamed in place (one record line replaced by one), so the address still carries two records, as before: this one and `GuiIconRuntime_construct_00ab5c60_fragment`. |
| 005C6A70 | Status downgraded to `partial_semantic_projection`, matching the older records. Only the key order was checked. |

## Reconstructed bodies

| Address | Function | ABI | Evidence | Uncertainty | Rows it unblocks (callers) |
|---|---|---|---|---|---|
| 0077B280 | `replication_schedule_sync_0077b280` | `__cdecl(entity, record {object, last tick}*, byte flag, n {float distance, int interval} pairs)`, returns a bool in AL; 3 + 2n stack dwords with n varying by caller: 007BD338 `ADD ESP,44h` (7 pairs), the `ADD ESP,3Ch` callers (6), 00859A13 `ADD ESP,34h` (5) | Whole body 0077B280-0077B6DD from assembly, with the x87 bytes read: D8 m32 forms, `DE C1` FADDP, `DE FA` FDIVP ST(2),ST(0) = elapsed / interval, DC m64 FDIV/FMUL by 00D03DD0 (float pi/3 widened), 00D7A280 0.5 and 00CE3DE8 -8.0. It has party/alive gates (vtable +5Ch(6), +15Ch, byte 00F87150). Time comes from vslot +14h of `[game+2078h]` or `[game+207Ch]`. Unless `flag` is set, it computes the distance to object+ACh and applies view-cone scaling when the distance is > 150.0 and 007F7CD0 holds. It walks the bands, capped at 3 when `[entity+2ECh] >= tick`, then does the interval remap. The level is `clamp(-7 - ftol(s * -8.0), 0, 40h)` and goes into entity+288h. The queue slot is `(level-1)*300h + count[level-1]`, written into 00F57150/00F1B150/00F27150, with counts at 00F1B050. | The band walk reads the caller's pairs without a bound, as the image does. The int/float quotient is modelled at 24-bit precision control (main thread). 007789D0 is game code, the existing `safe_acos_007789d0` in `src/ship_ai_station_keeping.cpp` (a clamped acos); the hook is named `safe_acos_007789d0`. The NaN arms follow the FCOMIP/JBE jumps. | 0074D030, 007BD240, 008138A0 (now done-unwired) |
| 00B68D70 | `lua_object_resolve_dot_path_00b68d70` | `__thiscall(ECX = LuaObject; LuaObject* out sret, const NativeString* path, bool* ok)` RET 0Ch, returns out in EAX | Whole body 00B68D70-00B6911E from assembly. It copies out from this (00B66FA0), copies the path and sets `*ok = 1`. Then, for each "." segment (`_strcspn` plus two 00469840 substrings, with the inlined assigns and pool releases): out must be bound and, for kind 2, must be a table (lua_type 5). Otherwise `*ok = 0` and the walk continues. Next `tmp = out[segment]` (00B68100), with the atol / "0" integer fallback through 00B67720 when tmp is a kind-2 nil. Then `out = tmp` (00B67690) and the tracked releases (00B66DE0). An empty segment sets `*ok = 0` and stops. | The no-dot arm copies 0 bytes of "" into rest.data (kept). There is an existing globals-root projection in `src/locale_lua_context.cpp`; this is the raw whole body beside it. | 004EA080, 005072C0, 008C8F70 (done-unwired); wired callers 004E96D0, 0095C640 |
| 00438FE0 | `transfer_cstring_key_00438fe0` (raw) | `__thiscall(ECX = destination; source*)` RET 4, returns destination in EAX | 00438FE0-00438FF4: byte +4 moves from the source (the source byte is cleared), then dword +0 is copied. | None | 009013D0 (still blocked on 00438F80) |
| 006B8B10 | `achievement_count_006b8b10` | `__fastcall(ECX = registry)`, returns EAX | `MOV EAX,[ECX+1Ch]; RET` | The name comes from the callers. | 005E7B10 |
| 00BDB2E0 | `vfs_search_group_select_value_list_00bdb2e0` | `__stdcall(const iterator {container, node}*, int selector)` RET 8, returns EAX | 00BDB2E0-00BDB340: selector 0 gives node+10h, 1 gives node+1Ch, anything else 0. Both arms run the checked dereference first. | 00BF6713 is modelled as returning. | 00BE2310; wired 00BE1B90 |
| 0053D230 | `hud_free_camera_forward_to_unit_0053d230` | three stack dwords (a float[3] by value) RET 0Ch; ECX unused | 0053D230-0053D265: the unit is `[00E188D8]`. If vslot +5Ch(6) holds, y is set to 0.0. The unit is then re-read and vslot +118h is called with the argument block as the float[3]. | The name is a hypothesis. | 00540390 |
| 00B68550 | `lua_object_get_type_name_00b68550` | `__thiscall(ECX = LuaObject; NativeString* out sret)` RET 4, returns out in EAX | 00B68550-00B685B7: the type is -1 for kind 0, lua_type(L, index) for kind 2, and 5 otherwise; then 00B68460(ECX = out, EDX = type). | None | 0043C1C0 |
| 00A96F40 | `text_owner_dispatch_queued_events_00a96f40` | `__thiscall(ECX = owner; unused dword)` RET 4 | 00A96F40-00A96F9F: the platform `[0109CF04]` is read once. While byte owner+4 is set and `[platform+17Ch]` is non-zero, it calls 00BECE90, then owner vslot 0, and falls back to 00A96750 when vslot 0 declines. | The image pushes the two byte locals as dwords whose upper bytes are never written; the hooks receive only the bytes. | 005D18A0 |
| 0050A950 | `tactical_library_preview_clear_rotors_0050a950` | `__fastcall(ECX = preview)` RET | 0050A950-0050AA0B: two first > last checks, then 004FCBE0 erases [first, last) of the dword vector at +424h. It logs "Rotor clear!" (00CEB9F8), then for each remaining element does a checked index and logs "Rotor %d=0x%x" (00CEB598). Then 005092E0(004FDE20()), 00504960(+5C4h; 0), and `+48Ch = 0`. | The names come from the strings. 00BF6713 is modelled as returning. | 0050B2D0; wired 0050AA10 |

## Existing implementations, recorded ledger-only

| Address | Existing body | Spot check |
|---|---|---|
| 004DC410 | `resize_native_particle_record_array_004dc410`, `src/native_particle_record_resize.cpp` (full 004DC410..004DC4DB body) | Agrees with the assembly 004DC410-004DC4D9: signed grow test into 004DA180, non-null construction of the new 2Ch records with the 004C3020 sentinel at +0Ch, a shrink loop that decrements `[header+4]` in memory and destroys through 004D45A0, then the final count store. The front8 duplicate was dropped; the verdict is appended to the ledger name record. |

## Existing fragments beside the raw bodies

- 00438FE0: `transfer_cstring_key_00438fe0` in `src/robot_config.cpp`, a borrowed-key fragment record in the ledger (the source does not cite the address).
- 00A96F40: `dispatch_text_input_00a96f40_fragment`, `src/text_input.cpp:159`.
- 00BDB2E0: the select_values helper, `src/native_vfs_search_groups.cpp:138`.
- 00B68550: the type-name path in `src/locale_lua_context.cpp:150` (lua_type, then `locale_lua_type_name_00b68460`).

## Skipped

| Address | Reason |
|---|---|
| 005F98F0 | Gameplay unit-proximity x87, referenced from `game_hosts_ai.cpp` (cc9). |
| 00A2D8E0, 0072D950, 0077D270, 00748A40, 00A07E40, 00735100, 00956600, 008255B0 | Gameplay / cc9-hosted lanes. |
| 00BE9760 | Already exists as `FingerprintPayload::load_00be9760`. |
| 004FF580, 00785170, 00787850, 007858C0, 00698730, 0064AE20 | Ranked but not examined in this packet. |

## Readiness (`tools/wire_readiness.py`)

| State | Before (1f810c9aa) | After review fix (004DC410 duplicate dropped) |
|---|---|---|
| ready | 1625 | 1628 |
| blocked_on_callees | 253 | 240 |
| callees_done_no_wired_caller | 9053 | 9067 |
| wired | 7478 | 7483 |
| unknown_no_identifier | 1251 | 1251 |

wire_readiness counts only `kind=function` records, so it still reports 004DC410 as missing even
though the existing full body is recorded as a fragment. 004DCAA0 is back to blocked, and 004DDA40 and
004DE290 list it as missing again. That accounts for the one-row difference from 7d576ed5a (blocked 239,
wired 7484).
