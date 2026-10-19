# CC13 front7: wiring-frontier callees, round 2

Packet cc13-front7 continues docs/CC13_FRONT6.md. It covers the large bodies that front6 skipped and then the next
blockers in the `local/output/wire_readiness.json` ranking. Code is in `src/cc13_front7.cpp` and
`include/bsp/cc13_front7.hpp`, namespace `bsp::cc13_front7`. That file is registered in `bsp_core` through
`cmake/startup.cmake`. Nothing is wired into a host.

Objects are reached through `cc13_sweepA::Raw`, which gives byte offsets into the Win32 layout. Every call the image
makes goes through a host method named for its target address. Image globals go through `global_pointer` /
`global_address`. CRT bodies (memcpy, memset, sprintf, atol, _stricmp, __allshl, the _ftol2 family, operator delete,
_invalid_parameter_noinfo) are bound through hooks, not reconstructed. 00BF6713 is modelled as returning, as in
front6. Names are descriptive hypotheses, not recovered symbols.

Status: reconstructed and build-tested (MSVC Win32 `./scripts/build.ps1`, the existing tests pass). Nothing here is
fixture-tested, ABI-compatible or game-validated.

## Whole bodies

| Address | C++ | ABI | Evidence | Uncertainty | Rows it unblocks |
|---|---|---|---|---|---|
| 0078CFF0 | `environment_apply_to_camera_0078cff0` | `__thiscall(ECX = environment; camera, float clock)` RET 8 | Assembly 0078CFF0-0078D1A6, covering the 00BA40F0/00BA2FF0 ocean effects, 004FB850 + 00B6FE50 clear colour, the [camera+184h] fog setters (re-read per call, same mapping as `src/environment_fog_apply.cpp`), and 00BA0090 with float(clock * double 00D7A3A0). | The two colour arms of the camera+198h/+174h test are identical in the image. FLD/FSTP m32 NaN quieting is not modelled. | 00503510 (now ready) |
| 0061B0F0 | `powerup_screen_rebuild_lists_0061b0f0` | `__fastcall(ECX = screen)` RET, EH 00C77F58 | Assembly 0061B0F0-0061B839, covering the 008E9AF0 collect, the V1/V3 loops with checked indexes, the inlined push_back (00441F30 slow path), the int-map arm (00614060/00AB64C0/006171B0), the CI string-map find (00613900 + inlined compare), the four 00AB64C0 icons, the 00617870/006170F0 subscripts, and the 004F6210 copies into 0061A400. | The slot re-reads are kept at the image points. | 0061B840, 0061B850, 008EE020 (now done, no wired caller) |
| 0087CA80 | `damageable_class_read_lua_fields_0087ca80` | `__thiscall(ECX = descriptor; LuaObject* row)` RET 4, EH 00C96A7D | Assembly 0087CA80-0087D723 (858 instructions), cross-checked with docs/NATIVE_DAMAGEABLE_CLASS_LUA_ORCH4.md. Fields read: Name, Unique, Mesh, Comment, HP (default 100.0), Armour, ExplosionType. Damage.Sections records are appended by 0087C870 to +18h, each with MshCategory, Index, the FireEfx handle (else BigFire), FailureChance (-100.0 / 100.0) and FailureDamageThreshold (-1.0). FakeExplosionEffects is read through "%d" keys and the 0087C920 resize, with elements 1..4. Emberkek is read through 0087B750 [+40h] = 004B1400. | In the 0087C920 by-value record the image never writes dword +8; the hook receives only +0, +4 and +0Ch. The FailureChance quotient assumes the main-thread 24-bit precision control (float division). | 00440AC0, 00960230 (now done, no wired caller) |
| 005DF510 | `lobby_dlc_unlock_mask_005df510` | `__stdcall(int mode)` RET 4, EDX:EAX; ECX not an input | Assembly 005DF510-005E08DF (1394 instructions). The jump table at 005E08E4 maps each mode to its dword-pair vectors at entry +18Ch+10h*k on both 154h sides. Those names go through atol, then VehicleClass[n].Type, onto list A. List A is sorted and uniqued (005DF1A0/005DA670). Matching rows with a string UnlockDLCMinimum go onto list B. The result is mask \|= __allshl(1, atol(suffix after the last "_") - 1). | 005DF1A0 and 005DA670 are read as list sort and unique from a skim only. | 005E51D0 (now done, no wired caller) |
| 00531130 | `text_set_glyph_reference_00531130` | `__thiscall(ECX = text; const WideString*, listener, reference)` RET 0Ch | Assembly 00531130-005311BC: self-assignment test, 004C53E0 resize, inlined word copy through the terminator, then +1ACh/+1B0h. | This raw-offset body sits beside the typed projection in `src/gui_text_glyph_reference.cpp`. | 00531380, 0054C050, 005D26D0, 0061FBE0 |
| 00AB5C60 | `gui_icon_construct_00ab5c60` | `__thiscall(ECX = widget)` RET, EAX = widget | Assembly 00AB5C60-00AB5D29: 00AA9390(widget; 6), then the derived stores in image order (vtable 00D5C4C0, 1.0 from 00D7A24C). | +F0h is not written by this constructor. | 00AC77C0, 00AD2D50 (now ready) |
| 00798080 | `movie_camera_track_pair_keyframe_00798080` | `__thiscall(ECX = track; keyframe)` RET 4 | Assembly 00798080-0079812C: the checked list walk over track+68h. The comparison is FUCOMIP/LAHF/TEST AH,44h/JNP, which is ordered float equality on +F0h. The pair id is taken from word [[track+74h]+3C0h] and the dword there is incremented. | None beyond the hooks. | 007A2CD0 (now ready), 007A2D30 |
| 005B83E0 | `voice_owner_stop_and_clear_005b83e0` | `__fastcall(ECX = owner)` RET, EH 00C72110 | Assembly 005B83E0-005B8506: the voice-slot stop over the [owner+58h] chain, 005B7840(owner+54h), and the per-object teardown and delete over the list at +64h, followed by an inlined list clear. | 007026F0 is called before the image's null test on the object; this is kept. | 0096BF70 (now ready) |
| 00435CC0 | `profile_manager_construct_00435cc0` | `__thiscall(ECX = manager)` RET, EAX = manager | Assembly 00435CC0-00435D04: vtable 00CE411C, then 00B65F50(this+4). | None. | 00425C20 (now ready) |

## Existing implementations recorded (ledger only)

| Address | Existing C++ | Verification |
|---|---|---|
| 00698A10 | `load_native_input_configuration_00698a10`, src/native_input_configuration_runtime.cpp | The tail 00699AD8-00699B7D was checked against the assembly: 004BEC00, the 00A922A0 rebind, FLDZ, 00A92C40, the [00F88A30] test, 005547D0/006AB820, +520h = 1, and four LuaObject releases. The prefix comes from earlier packets. The record was a fragment until now; its callers 0055FA10, 0055FCA0, 00560430, 00560BF0 and 005F65C0 now have all callees done. Its own callee 00698730 (the Clear, still a fragment) now blocks 00699B80. |
| 005C6A70 | `read_mission_record_005c6a70`, src/mission_tree_data.cpp | This is a typed model, recorded as a partial semantic projection (status `partial_semantic_projection`): only its key order was checked against the image's string-reference order. 005C9F70 is now ready. |
| 004D0640 | none (library) | Classified as the VC9 `std::set<NativeString, case-insensitive less>::insert` instantiation: _Tree descent with _Isnil at +15h, an empty-string test, then _stricmp comparator (no length compare), 004CE270 _Insert on a miss, and a pair sret. It is renamed `STL_NativeStringSetCI_Insert_004D0640` and is bind-only. 004D0FC0 is now ready. |

## Skipped

| Address | Reason |
|---|---|
| 00565D80, 005CD0F0 | Leased by cc13-small1 (packet cc13-small1b). Bodies were drafted, then removed before commit. |
| 00A2D8E0, 00A07E40, 0072D950, 0077D270, 00748A40, 00956600, 00735100 | Gameplay segments (40, 41, 43, 62, 69, 70) or cc9-hosted (00748A40 in game_hosts_units.cpp, 00A07E40 in game_hosts_ai.cpp). |
| 0077B280 | Gameplay replication delta, 294 instructions of pose/tolerance x87. Left for a max-effort packet. |
| 00B68D70 | 307 instructions; a fragment already exists in src/locale_lua_context.cpp. Not reached in this packet. |

## Also in this packet

The 007F7D90 row in docs/CC13_FRONT6.md and its ledger evidence now describe the out-of-line base destructor body.
Its callers are 0075C6E3 (the scalar-deleting destructor) and the EH funclets 00C89AA5 and 00C8F773.

## Readiness

`python tools/wire_readiness.py` on this tree:

| State | Before | After |
|---|---|---|
| ready | 1600 | 1613 |
| blocked_on_callees | 293 | 266 |
| callees_done_no_wired_caller | 9022 | 9043 |
| wired | 7437 | 7441 |
