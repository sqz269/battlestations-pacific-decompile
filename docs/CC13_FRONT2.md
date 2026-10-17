# CC13 front2: callees that block wiring (packet cc13-front2)

Targets came from `local/output/wire_readiness.json` (main checkout): unreconstructed functions
that block already-reconstructed callers in state `blocked_on_callees`. Code: `src/cc13_front2.cpp`,
`include/bsp/cc13_front2.hpp` (host interface `Cc13Front2Host`). Names are hypotheses, not
recovered symbols. Every function is a new C++ interface over raw native blocks: reconstructed and
build-tested only, not ABI-compatible, not fixture-tested, not wired, not game-validated.

## Reconstructed

| Address | Function | Original ABI | Notes |
|---|---|---|---|
| 0042D0D0 | `vector3f_transform_direction_optional_normalize_0042d0d0` | __fastcall(ECX dest, EDX src; matrix, normalize byte), RET 8, EAX dest | The arithmetic is the existing x87 kernel `transform_native_effect_direction_0042d0d0_no_normalize` (material_effect_plane.cpp), run into a local that stands for the ESP+10h spill; normalize != 0 hands that spill to 00419510 (`normalize_00419510`, world_ocean.cpp) with the destination as output (0042D150), else the spill is copied (0042D162..0042D17D). All three components are spilled before any destination write, so source/destination aliasing behaves as in the image. |
| 0042DCD0 | `camera_mover_update_focus_0042dcd0` | __thiscall(mover, float dt, byte add_parent_yaw), RET 8, AL | Gate: parent +3ACh nonzero and `COMISS 0,[+3D4h]`/JNC (runs when +3D4h > 0 or unordered). With +3D8h set and no entity at +3C4h: +3D4h = -1.0f (00D7A260), +3D8h = 0. Failing paths zero +3E0h/+3E4h and return 0. Focus: point +3C8h or the entity's +FCh (00414DB0 first when its +C8h is clear), minus the mover's +FCh (00414DB0 likewise); yaw = 00438B10(_CIatan2(z, x), pi/2 [00CE3C64]); with add_parent_yaw and a parent: 00414DB0 when needed, 0042D2E0(parent+CCh) and 00438AA0(yaw, output Y); 00419510 on the difference, yaw = 00438AA0(yaw, +3E0h); pitch = 00438AA0(0042CF10(n.y), +3E4h); step = +3DCh * dt; 0042C750(mover; yaw, step, pitch, step), its AL ignored; returns 1. Atan2 operand order from the loads at 0042DDE1/0042DDE5 (ST1 = z, ST0 = x). |
| 00A91020 | `input_device_table_any_dynamic_device_button_down_00a91020` | __fastcall(ECX table), RET, AL | Three checked vectors at table+6Ch + class*24h (+4 first, +8 last). Non-null devices are asked through vtable +2Ch; the first nonzero answer returns 1. The checked-iterator tests (first > last, dereference and increment at last) call 00BF6713 and continue as the image does. |
| 00A95CB0 | `gamepad_force_clear_all_channels_00a95cb0` | no arguments, RET | Count from [00F8BBF4]+B8h..+BCh taken once (SAR 2, 0 for a null begin); for each index the global is reloaded after the first and 00A95890 is called with the element, or with ECX = 0 when the begin is null or the index is past the current size (00A95CFF). The checked operator[] repeats the test and calls 00BF6713. |
| 00784790 | `session_transport_broadcast_except_00784790` | __thiscall(transport, message, exclude_peer), RET 8 | List at transport+8h (head at +0Ch). An empty list hits the checked increment (00BF6713) because the walk starts at the second node. Each peer other than exclude_peer: EnterCriticalSection([peer+4]) via 00CE2218, depth +18h += 1, 00783C80(transport, peer, message), reload [peer+4], depth -= 1, LeaveCriticalSection via 00CE2210. |

| 008D5B50 | `settings_apply_all_008d5b50` | __thiscall(ECX settings at 00F88980), RET | 008D5430; 00B1FFB0(renderer, 2 - +68h); renderer vtable +F0h(+64h); 004B46E0(game; +41h..+44h); 004DCDF0. With no [00F8A304], a [00F8ABE8] and 00A3E510: new(844h), 009955F0 on success. Game: foliage +4Ch = {0.5, 0.7, 1.0}[+54h] (no bounds test, as in the image), ocean byte +F8h = +6Ch. 004C1710()+14h = +9. Font path (entry +18h; "" when empty, the 00F88A3C text when data is null) and a "Fonts\" string (0041DD40(6, 1) + memcpy of length+1) to 00AC3610(007371D0(), root, path); lanfile (entry +4 or 00F88A3C) to 00AA09D0. 00A94C50(game+634h == 0 && +40h == 0). Download manager: with +1Ch, vtable +24h, 0043EA80(+98h), vtable +14h(+98h), vtable +20h; then "globals" to 00AA0D30 and 00AA06D0(1). First-call latches (flags 00F88A48 bits 0..2 for 00F88A44 = +8Dh, 00F88A40 = +70h, 00F88A3D = +84h). Scene block (game+19ECh): "FoliageGroup" via 00B72270(name, 0), 00B6DA70(+86h ? 1.0 : 0.0, 1); 00BBCFE0(00BBDDF0(game+19E8h), +74h); ocean byte, vtable +10h when +70h moved; [00F8BBF0] vtable +8 when +84h moved; +8Dh latch; each move sets the changed byte. Tail: 00B0D580(+8Dh, +58h); renderer vtable +80h size against +14h/+18h, 00B0FC00 on a mismatch; 00B29E60(+14h, +18h, +1Eh, +58h, +60h, changed); [0109CF04] vtable +0Ch(+1Eh, +14h, +18h); XLiveOnResetDevice(renderer+1A28h) and later 00AA4EF0(004C12B0()) each when [00E198C4] is null; 00B107F0(+8Ch, +8Dh, +58h); 00B0D080(+90h); the 00439100 region text into the NativeString at 0108FF24 (0041DD40(len, 0), memcpy of length). Each temporary NativeString is destroyed inline (00419CC0 / 00BD1510(data, length + 1, 1)) at the image's point. The changed flag is passed as its byte (the image pushes the whole dword, upper bytes stale; 00B29E60 reads the low byte). Supersedes the projection `apply_all_settings_008d5b50` in game_settings.cpp, which leaves the download-manager construction out. |
| 0077FAD0 | `net_entity_read_replication_state_0077fad0` | __thiscall(ECX entity), RET | 009295B0. Init record +C0h kind 2, data +8 with +C5h: leader (+C6h): 0070DB20 when +284h is null, group +4FCh = +CAh, 0070EFD0(+CBh); member: 00521E30(word +C8h), 0077F940(leader), 0070D080(group, entity), then the record gets +CCh..+D4h at +4..+C when group +4FCh == 18h, else the eight floats +D8h..+F8h in the image's interleaved store order. Kind 3: 009238A0 into the 14h-byte by-value area, 004425C0, keys "_netEntity", "_reconData", 00804F10(entity+1E4h), leave, read "hasPlrCmd" (type 3, +290h), "formation" (00BD5EB0, enter, 0070ECF0 stored to +284h before the leave), reader vtable +4("lastBanto"), 006E6B10(entity+2B0h), vtable +8, read "lastBantoPlayerIndex" (type 1, +2D8h), leave, 00441A20. Then "reconlevel" (0041DD40(10, 1) + memcpy) to 00B67580 on the 00927B40 self object, 00B67700, string destroyed; 00985590([00F8A0C4], entity) for kind 5 or 18h (vtable +5Ch); vtable +0Ch(+15Ch, 0) when +15Ch is nonzero. Float copies are x87 FLD/FSTP in the image (an SNaN would be quieted there, not here). |

Host calls: 00BF6713, 00414DB0, _CIatan2 00BF701A, the [00F8BBF4] global, device vtable +2Ch,
00A95890, the two critical-section imports, 00783C80, and for 008D5B50 / 0077FAD0 each callee
listed in their rows (the existing reconstructions of those callees have unrelated typed
interfaces, so they are reached through named hooks). Direct calls of existing
reconstructions: 00419510 (world_ocean.cpp), 00438AA0 and 00438B10 (unit_rudder.cpp), 0042CF10
(camera_position_modes.cpp), 0042D2E0 (camera_decomposition.cpp), 0042C750
(segment2_camera_cc13.cpp) and the 0042D0D0 kernel (material_effect_plane.cpp). No shared-RNG
(00BD2F10) draws in any of them.

0077FAD0 was skipped by docs/CC13_SWEEPB2.md as a cc9-lane name (docs/CC13_GP_0071_0078.md
"Names for cc9"); this packet's brief limited the cc9 exclusion to 00778820..0077D270 and
0077A080, so it is reconstructed here in a new file. No host file was edited.

## Existing implementations, recorded ledger-only

| Address | Existing function | Verified |
|---|---|---|
| 00A92C40 | `update_input_manager_00a92c40` (src/input_tick.cpp, used by game_hosts_mission_frame.cpp) | backend vtable +4(seconds); [backend+D4h] read-and-clear then 00A922A0; per enabled record 00A92370 and, with a listener at +2Ch, 00A91A50(seconds, prev = +20h && +1Ch > 0, cur = +28h && +24h > 0) with JBE (NaN false); then the [00F8BBFC] hook. The listener is a typed index rather than the +2Ch pointer (bounds test is a projection artefact). |
| 00A92D40 | `activate_native_input_action_context_00a92d40` (src/native_input_action_configuration.cpp) | raw-memory transcription; walk, clears and FLD1 listener update match 00A92D40..00A92E68. |
| 00A900F0 | `reset_native_input_backend_00a900f0` (src/native_input_backend_startup.cpp) | 3x8 slots from +4, vtable +14h on each non-null slot. |
| 00A966E0 | `set_text_editor_active_00a966e0` (src/text_editor_activation.cpp) | +4 byte, +24h, 00A965A0(enabled), 004BEC00 then 00A92C40(owner, 0.0); when enabled 0041DD40(0, 0) on +18h, memcpy(data, "", [+18h]) when data is non-null, +20h = 0. |
| 008D4820 | `reset_native_profile_control_defaults_008d4820` (src/native_profile_settings.cpp) | stores +42h, +43h, +41h, +40h, +44h in image order; tail 008D45D0 when [00F8ABE8] is non-null, its +28h == 2 (00A3E500) and +119h != 0 (00A3E510). The older `reset_control_defaults_008d4820` in game_settings.cpp leaves the tail out. |

## Skipped

- 0077EDF0 BSP_MessageSystem_EntitySuppressSlot: a `std::map<unsigned, unsigned char>::operator[]`
  instantiation (lower-bound walk on the tree at ECX, insert through STL_Tree_InsertHint_0077e950 on
  a miss, checked-iterator tests, returns &node+10h). STL instantiations are out of scope (vendor
  rule 2b); its eight blocked callers already reach it through named hooks.
