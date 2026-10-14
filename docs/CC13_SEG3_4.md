# Segments 3 and 4 (0043E8F0-004486A0, 004486C0-00453C30), packet cc13-seg3

Reconstructions in `src/seg3_4_cc13.cpp` / `include/bsp/seg3_4_cc13.hpp`,
namespace `bsp::seg3_4_cc13`. Names are hypotheses, not recovered symbols.
Status: reconstructed and build-tested (MSVC Win32, bsp_core); not
ABI-compatible, not fixture-tested, not game-validated.

## Conventions

- Objects are the image's raw layouts at byte offsets; every call is a named
  host method at the same point with the same arguments.
- The dialog commands and the device save state talk to a property stream
  through tagged 8-byte values passed by value, `{kind 0 string, 1 int,
  2 float; payload}`: reader vtable +0Ch (key, destination, default) and +10h
  (key, destination), writer +0Ch (key, value), +04h open table, +08h close.
  `PropertyStreamHost::literal` / `address_of` give the image addresses the
  payloads carry.
- NativeString blocks go back to the sized pool through 00419CC0 + 00BD1510
  (`pool_return_block`). The type-name temporary in 0044C060 is host-built.
- `docs/CC13_SYS_2.md` had left the dialog command classes to a cc9 dialog
  sequencer packet; no cc9 branch has code for them, and this packet's lease
  covers them.

## Functions

| Address | C++ | Behaviour, ABI and uncertainty |
|---|---|---|
| `00449A30` | `dialog_command_scalar_delete_00449a30` | 00449A30 / 00449A50 __thiscall(command, flags) RET 4 -> EAX: the hidepanel (8-byte) and pause (0Ch-byte) scalar deletes: vtable = 00CE4940, _free when flags bit 0. |
| `00449A50` | `dialog_command_scalar_delete_00449a50` | 00449A30 / 00449A50 __thiscall(command, flags) RET 4 -> EAX: the hidepanel (8-byte) and pause (0Ch-byte) scalar deletes: vtable = 00CE4940, _free when flags bit 0. |
| `0044A3F0` | `dialog_command_read_base_0044a3f0` | 0044A3F0 __thiscall(command, reader) RET 4: reader +0Ch("panel", int -> +4h, default 1), then +4h = 0 (the value read is always overwritten). |
| `0044A440` | `dialog_command_msg_read_0044a440` | 0044A440 __thiscall(command, reader) RET 4: the base read inline, then reader +10h("message", string -> +8h). |
| `0044A4C0` | `dialog_command_setpanel_read_0044a4c0` | 0044A4C0 __thiscall(command, reader) RET 4: the base read inline, then +10h("name" -> +8h) and +10h("character" -> +10h). |
| `0044A570` | `dialog_command_pause_read_0044a570` | 0044A570 __thiscall(command, reader) RET 4: +4h = 0, then +10h("time", float -> +8h). |
| `0044A5B0` | `dialog_command_callback_read_0044a5b0` | 0044A5B0 __thiscall(command, reader) RET 4: +4h = 0, then +10h("callback", string -> +8h). |
| `0044B380` | `dialog_command_msg_type_0044b380` | 0044B380 / 0044B3B0 / 0044B3E0 __fastcall -> EAX: the msg, setpanel and callback type ids 0, 1, 4. |
| `0044B3B0` | `dialog_command_setpanel_type_0044b3b0` | 0044B380 / 0044B3B0 / 0044B3E0 __fastcall -> EAX: the msg, setpanel and callback type ids 0, 1, 4. |
| `0044B3E0` | `dialog_command_callback_type_0044b3e0` | 0044B380 / 0044B3B0 / 0044B3E0 __fastcall -> EAX: the msg, setpanel and callback type ids 0, 1, 4. |
| `0044B3F0` | `dialog_command_msg_destruct_0044b3f0` | 0044B3F0 / 0044B4D0 __fastcall(command): the msg / callback destructors: the NativeString +8h's block (when +Ch is set, length +8h + 1) back to the pool, then vtable 00CE4940. 0044B450: setpanel, the character string (+10h/+14h) first, then the name (+8h/+Ch). |
| `0044B450` | `dialog_command_setpanel_destruct_0044b450` | 0044B3F0 / 0044B4D0 __fastcall(command): the msg / callback destructors: the NativeString +8h's block (when +Ch is set, length +8h + 1) back to the pool, then vtable 00CE4940. 0044B450: setpanel, the character string (+10h/+14h) first, then the name (+8h/+Ch). |
| `0044B4D0` | `dialog_command_callback_destruct_0044b4d0` | 0044B3F0 / 0044B4D0 __fastcall(command): the msg / callback destructors: the NativeString +8h's block (when +Ch is set, length +8h + 1) back to the pool, then vtable 00CE4940. 0044B450: setpanel, the character string (+10h/+14h) first, then the name (+8h/+Ch). |
| `0044BE50` | `dialog_command_create_from_archive_0044be50` | 0044BE50 __fastcall(ECX = reader) -> EAX: the dialog command factory. A local NativeString gets reader +0Ch("type", default "msg"); "msg" (non-null text and __stricmp) makes a 10h-byte msg (vtable 00CE49CC, +8h/+Ch = 0), then through 00425850: "setpanel" 18h bytes (00CE49DC, +8h..+14h = 0), "hidepanel" 8 bytes (00CE4950), "pause" 0Ch bytes (00CE4960), "callback" 10h bytes (00CE49EC, +8h/+Ch = 0); +4h is left for the read. The command's vtable +08h then reads the reader. An unknown type (or a failed operator new) leaves the command null and the image calls through it anyway; the host receives the null. The local string is returned to the pool. |
| `0044C060` | `dialog_command_write_base_0044c060` | 0044C060 __thiscall(command, writer) RET 4: writer +0Ch("panel", int +4h + 1), then the type name by vtable +04h (0 "msg", 1 "setpanel", 2 "hidepanel", 3 "pause", 4 "callback", other: an empty NativeString, written as 00E17BFB) as +0Ch("type", string); the temporary is released. |
| `0044C220` | `dialog_command_msg_write_0044c220` | 0044C220 / 0044C340 __thiscall(command, writer) RET 4: the base write, then "message" / "callback" = the text +Ch (00E17BFB when null). |
| `0044C340` | `dialog_command_callback_write_0044c340` | 0044C220 / 0044C340 __thiscall(command, writer) RET 4: the base write, then "message" / "callback" = the text +Ch (00E17BFB when null). |
| `0044C270` | `dialog_command_setpanel_write_0044c270` | 0044C270 __thiscall(command, writer) RET 4: the base write, "name" = +Ch, "character" = +14h (00E17BFB for null). |
| `0044C2F0` | `dialog_command_pause_write_0044c2f0` | 0044C2F0 __thiscall(command, writer) RET 4: the base write, "time" = float +8h. |
| `0043F120` | `device_die_if_health_depleted_0043f120` | 0043F120 / 0043F7D0 __fastcall(entity), two identical vtable slots: when the health +370h is <= 0.0f (COMISS; NaN keeps it alive) vtable +70h(1). |
| `0043F7D0` | `device_die_if_health_depleted_0043f7d0` | 0043F120 / 0043F7D0 __fastcall(entity), two identical vtable slots: when the health +370h is <= 0.0f (COMISS; NaN keeps it alive) vtable +70h(1). |
| `0043F7B0` | `device_init_health_and_parts_0043f7b0` | 0043F7B0 __fastcall(entity): 0087BCC0, then +168h = 00740FE0([00E1AEA0]). |
| `0043F860` | `device_handle_messages_4b4c_0043f860` | 0043F860 __thiscall(entity, message) RET 4 -> AL: message types 4Bh / 4Ch (byte +10h) answer 1; anything else goes to 00878350. |
| `0043FEA0` | `device_write_state_0043fea0` | 0043FEA0 __thiscall(entity, writer) RET 4: 008776E0, "deviceclass" = int [[+354h]+6Ch], then sub-table "_device" with "platform" = int +38Ch. |
| `00440770` | `device_class_construct_00440770` | 00440770 __fastcall(class) -> EAX: 0087C640, vtable 00CE4480, +70h = 0. |
| `00440A30` | `ref_slot_release_00440a30` | 00440A30 __fastcall(slot): a counted reference release: when [slot] is set, InterlockedDecrement(+4h) reaching 0 destroys it (vtable +0h), then [slot] = 0. |
| `004420D0` | `device_class_destruct_004420d0` | 004420D0 __fastcall(class): vtable 00CE4480; the registry slot +6Ch is cleared (the registry grows to +6Ch + 1 first when +6Ch >= its count); the platform reference +70h is released as in 00440A30; then 0087C700. |
| `0044C620` | `panel_sequence_slot_copy_construct_0044c620` | 0044C620 __thiscall(dst, const src*) RET 4 -> EAX = dst: the sequence slot copy-constructor: byte +0h, then the NativeStrings +4h and +Ch, each zeroed first and, unless it is the source object, resized to the source length and filled (memcpy of the destination length) when the source is not empty. |
| `0044C7C0` | `panel_sequence_slot_assign_0044c7c0` | 0044C7C0 __thiscall(dst, const src*) RET 4 -> EAX = dst: the same as an assignment (no zeroing first). |
| `0044C8D0` | `panel_sequence_slot_construct_at_0044c8d0` | 0044C8D0 __fastcall(ECX = place, EDX = src): 0044C620 into place when place is not null (an allocator construct). |
| `0044F9D0` | `record40_construct_0044f9d0` | 0044F9D0 __fastcall(ECX = record) -> EAX: vtable 00CE4B40, +4h/+8h/+Ch = 0.0f, byte +10h = 0, +18h..+24h and +2Ch..+34h = 0, byte +38h = 0, +3Ch = 0. |
| `0044F380` | `record40_copy_construct_0044f380` | 0044F380 __thiscall(dst, const src*) RET 4 -> EAX = dst: vtable 00CE4B40, floats +4h..+Ch (FLD/FSTP), byte +10h, 0044DC40 on +14h, dword +24h, 0044EF30 on +28h, byte +38h, dword +3Ch. |
| `00450450` | `panel_sequence_pair_copy_construct_00450450` | 00450450 __thiscall(dst, const src*) RET 4 -> EAX = dst: the NativeString key +0h (zeroed, then copied unless self) and the 0044F380 record at +8h. |
| `00443A30` | `time_of_day_split_clock_00443a30` | 00443A30 __thiscall(clock, int* hours, int* minutes, int* seconds, int* ms) RET 10h (caller BSP_LuaBinding_GetDayTime): the float hour +4h split by truncation (CVTTSS2SI): hours = (int)t; then, each only when its pointer is set, m = (float)((t - hours) * 60.0), minutes = (int)m; s = (float)((m - minutes) * 60.0), seconds = (int)s; ms = _ftol((float)((s - seconds) * 1000.0)). The two 60.0 products use the DC-form FMUL (ST1 *= ST0; the listing prints it as FMUL ST1); the image keeps the 80-bit intermediates, the C++ uses double (MINOR). |
| `0043EE60` | `string_format_buffer_2a_0043ee60` | 0043EE60 cdecl(buffer, format, ...): _vsprintf_s(buffer, 2Ah, format, args). |
| `00449310` | `owned_ptr_delete_reset_00449310` | 00449310 __fastcall(slot): when [slot] is set, its vtable +00h(1), then [slot] = 0. |
| `00445940` | `weather_config_destruct_00445940` | 00445940 __fastcall(config): vtable 00CE4934; the float maps +68h, +5Ch and +38h are emptied (00444B10 over [begin, head)), their head nodes freed and +4h/+8h zeroed; then the NativeString +2Ch/+30h block goes back to the pool. |

## Implemented on main before this packet

cc13-backfill (152a7e698) checked this list against the ledger: 43 of the 45 addresses already had
reconstruction records (`python tools/bsp.py lookup <address>`). The two that did not now have
records: `0043F080` (`script_entity_live_0043f080`, src/script_orders_ver_cc13.cpp, verified
against the listing) and `00442220` (`NativeLuaReaderStorage::push_copy_00442220`,
src/native_lua_reader.cpp, minor deviation: host vector growth in place of the VC9 internals).
`0044E150`, `0044F0A0` and `004517E0` keep fragment records (full-range clear only). The list:

`0043EBD0`, `0043EBF0`, `0043F080`, `0043FA60`, `00440180`, `004412B0`, `00441360`, `00441760`,
`00441780`, `00441A20`, `00441A70`, `00442220`, `004425C0`, `00442B90`, `00442C50`, `00442CE0`,
`00442D80`, `00442E20`, `00442EC0`, `00442F50`, `00442FF0`, `004436E0`, `004437A0`, `004437C0`,
`004437F0`, `00443880`, `00443BA0`, `00443D00`, `00443D60`, `00443FB0`, `004447C0`, `00444BE0`,
`004462D0`, `00449910`, `00449940`, `00449960`, `00449980`, `004499A0`, `004499C0`, `004499E0`,
`00449AF0`, `0044BCB0`, `0044E150`, `0044F0A0`, `004517E0` (found by `git grep` of a definition).

## Not reconstructed here

| Address | Reason |
|---|---|
| `0043FF40` | x87 animation phase (fmod of the fixed clock); deferred to an x87 pass. |
| `00440AC0`, `00440E10`, `00440F50`, `004423A0`, `004426C0`, `00443090`, `00443490`, `00447510` | Lua-reading device-class and entity setup bodies (refcounted platform pointers, Lua object temporaries); not attempted. |
| `0044B5B0`, `0044BD30`, `00450210`, `00450750`, `004507D0`, `004514A0`, `004518B0`, `00451A90`, `00451B90`, `00451E50`, `00452130` | Panel owner / sequence property readers and save state; not attempted. |
| `0043EA00`, `0043FA00`, `00440600`, the `*_Construct`/`*_Clear`/`*_EraseRange`/`*_BuyNode`/`*_DestroySubtree` map helpers, iterator steps and `TMPL_RefPtr_*` | STL or template instantiations. |
