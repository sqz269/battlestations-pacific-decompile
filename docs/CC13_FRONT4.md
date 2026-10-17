# CC13 front4: wiring-frontier callees (segments 28, 60, 82, 53)

Packet `cc13-front4` (2026-10-17). Targets come from `local/output/wire_readiness.json` in the main
checkout: unreconstructed callees that block wiring of already-reconstructed callers. Code is in
`src/cc13_front4.cpp` / `include/bsp/cc13_front4.hpp` (namespace `bsp::cc13_front4`); every call the
image makes goes through a `Front4Host` method named for its target. Descriptive names are hypotheses,
not recovered symbols. Nothing is wired. Status: reconstructed and build-tested only.

## Reconstructed

| Address | C++ | ABI | What it does | Callers it unblocks |
|---|---|---|---|---|
| 0068AA40 | `ingame_interface_request_scene_for_unit_0068aa40` | `__thiscall(ECX = manager, unit)`, RET 4 | Returns when unit == manager+38h, unit is null, byte unit+5Ch is 0, or bytes +5Dh/+60h/+5Eh are set, or manager+20h is 2Bh/2Ch/2Dh/29h; otherwise `004CC460(manager, 20h, unit)`. | 0068AA90, 0068AB80, 0068B3F0, 0068B470, 0068C0B0 |
| 006881F0 | `multi_menu_pop_request_interface_006881f0` | `__thiscall(ECX = menu)`, RET | Checked `*(--end())` of the vector at menu+58h (first +5Ch, last +60h) read twice, each with the three `00BF6713` checks; first read logs `004254B0("GVMultiMenu::PopRequestInterface() req:%s", [00E08CD8 + id*4])`, second goes to `004CC460(menu, id, 0)`; then `last -= 4` when first is non-null and the span is non-empty. | 004D7BA0, 005661C0, 0057B600, 005D3410, 0051C7D0 |
| 00928570 | `game_entity_unlink_from_parent_list_00928570` | `__thiscall(ECX = entity)`, RET | Walks the nodes of [entity+30h]+24h from [parent+28h] (+4 next, +8 value); the first node holding the entity goes to `004837D0(parent+24h, node)`. | 004E79B0, 004E7EB0, 004E9690, 004F1420, 004F1940, 004F2600, 007B33A0 |
| 009258F0 | `scene_node_attach_to_parents_009258f0` | `__thiscall(ECX = instance, parent, world node, const matrix*)`, RET 0Ch | Once (byte +BCh): world node at +30h and append to [world+4] (links +34h/+38h); parent at +3Ch and append to its children +48h/+4Ch/+50h, or with no parent to [[+30h]+8] (links +40h/+44h); flush +B8h/+B4h through `009245A0` (cleared first); `004134F0(+74h, matrix)`; bytes +C8h, +10Ch = 0; `0042ED50` on each child; +BCh = 1. | 00928860 |
| 00B179C0 | `material_texture_count_00b179c0` | `__thiscall(ECX = material)`, RET | `(int16)[+34h]` sign-extended. | 00B850F0 |
| 00B17370 | `object_word_0c_00b17370` | `__thiscall(ECX = object)`, RET | `[+0Ch]`. The word is not named. | 00BA4EB0 |
| 00B0D1C0 | `render_service_word_1cc_00b0d1c0` | `__thiscall(ECX = render service)`, RET | `[+1CCh]`. | 00BA51C0 |
| 00B0D0D0 | `render_service_word_1d4_00b0d0d0` | `__thiscall(ECX = render service)`, RET | `[+1D4h]`. | 0093D250 |
| 0085E4D0 | `matrix_rotate_by_angular_velocity_0085e4d0` | `__fastcall(ECX = out, EDX = in; wx, wy, wz, dt)`, RET 10h | See below. | 007C18B0, 007D9F60 |
| 00860BE0 | `debug_draw_wire_sphere_marker_00860be0` | `__fastcall(ECX = centre, EDX = rgba[4]; radius, u32 arc rgba, u8 detailed)`, RET 0Ch | h = radius + radius; three `0085F990` axis lines from c-h to c+h (both colours the 16 bytes at EDX); three `00860AD0` circles (radius, start 0, sweep 2pi from 00CE3D9C, arc rgba, detailed ? 40h : 4 segments) about X, Y, Z. | 004169A0, 006F3CC0, 007C39B0, 0093FF10 |

Second batch (the next most-blocking callees in the report outside the keep-outs):

| Address | C++ | ABI | What it does | Blocked callers |
|---|---|---|---|---|
| 0042D0D0 | `vector3_transform_direction_0042d0d0` | `__fastcall(ECX = out, EDX = v; matrix*, u8 normalize)`, RET 8, EAX = out | r[i] = (m[i]*vx + m[4+i]*vy) + m[8+i]*vz into a local triple through the existing naked kernel `transform_native_effect_direction_0042d0d0_no_normalize` (same arithmetic and spills); normalize: `00419510(out, &r)`, else copy. | 44 (19 with wired callers) |
| 00784790 | `session_transport_broadcast_except_00784790` | `__thiscall(ECX = transport, message, excluded peer)`, RET 8 | Walks the checked list at transport+8 starting from the SECOND node (begin() is incremented once, failing the check on an empty list); for each peer other than the excluded one: enter [peer+4] (depth +18h), `00783C80(transport, peer, message)`, leave. | 32 |
| 008F0D80 | `scene_enum_decl_find_name_by_value_008f0d80` | `__thiscall(ECX = decl, value)`, RET 4 | Iterates the hash at decl+4 (`008EEB10`/`008EEB60`); returns the entry name (+4) whose value (+8) matches, the global at 00F89450 when that name is null, or null. | 5 |
| 008F0DF0 | `scene_property_get_string_value_008f0df0` | `__thiscall(ECX = property)`, RET | Type 2: +0Ch; type 4: `008F0D80([+28h], [+0Ch])`; else +1Ch or "". | 26 |
| 005CD160 | `movcam_ensure_target_005cd160` | `__thiscall(ECX = screen)`, RET, EAX | When byte +4 is 0: `004CC460([00E198C4], 2Bh, [00E188D8])`, `005CC000(screen)`, `007A6910([screen+30h], unit)`. Returns +30h. | 23 |

Third batch, one-instruction accessors (`__thiscall(ECX = object)`, RET, unless noted):

| Address | C++ | Body | Blocked callers |
|---|---|---|---|
| 006529E0 | `overlay_busy_006529e0` | AL = byte [+4] | 11 |
| 00951F10 | `vehicle_class_equipment_count_00951f10` | EAX = [+128h] | 9 |
| 00952140 | `device_iterator_at_end_00952140` | RET 8; AL = ([second stack arg + 4] == 0); ECX and the first stack arg unread | 9 |
| 006AC220 | `landing_pad_occupant_006ac220` | EAX = [+1F8h] | 7 |
| 00A3E540 | `storage_gate_00a3e540` | AL = byte [+120h] | 5 |
| 007D7A80 | `plane_byte_94_007d7a80` | AL = byte [+94h] | 5 |
| 00939F80 / 00939F70 | `water_amount_00939f80` / `fire_amount_00939f70` | ST0 = float [+34h] / [+38h] | 5 each |
| 0080E490 | `unit_parts_object_0080e490` | EAX = [+1018h] | 13 |
| 00BBDDF0 | `atmosphere_cloud_manager_00bbddf0` | EAX = [+3Ch] | 5 |
| 0099B630 | `pilot_command_pitch_target_or_live_0099b630` | ST0 = [+2D0h] != 0 ? float [+2BCh] : float [[+2F0h]+C64h] | 10 |

Fourth batch:

| Address | C++ | ABI | What it does | Blocked callers |
|---|---|---|---|---|
| 007788D0 | `unit_formation_leader_007788d0` | `__thiscall(ECX = unit)`, RET | [+284h] ? [[+284h]+14h] : 0 | 7 |
| 0080E160 | `unit_weapon_director_0080e160` | `__thiscall(ECX = unit)`, RET | [+738h] | 6 |
| 009A1A20 | `bot_task_commanded_heading_009a1a20` | `__thiscall(ECX = plan)`, RET, ST0 | Mode [+2CCh] == 2: float [+2C0h]. Else offset = 0 (no plane [+2F0h]) or `0099D0A0(plan, plane+C68h)`; returns `00438AA0(plane->vfunc[50h](), offset)`. The virtual takes no stack argument (the 00438AA0 RET 8 balance proves it); a null plane reaches it unguarded. No RNG. | 8 |
| 007D1E50 | `plane_class_construct_base_007d1e50` | `__thiscall(ECX = this)`, RET, EAX = this | `00749050(this)`, vptrs 00D05EAC (+0) and 00D05EA8 (+6Ch), then 32 dword and 4 byte zero stores (list in the ledger record). | 8 (the plane class constructors) |
| 00A1EEF0 | `ai_planner_destruct_base_00a1eef0` | `__thiscall(ECX = planner)`, RET | vptr 00D22D08; drains the group list at +20h: owned groups (`00A1C8B0`) get `006952A0(group, planner)`, STL remove `00A1D1D0`, group+5654h = 0; a group not owned is taken again forever, as in the binary. Then STL clear `00A16B60`, the +18h block back to `00419CC0()->00BD1510(block, [+14h]+1, 1)`, `00695870(planner)`. | 8 |

| 00A10DC0 | `ai_command_follower_pass_00a10dc0` | `__thiscall(ECX = command)`, RET | Group [command+4]; runs when its count +5644h > 1; walks the member list +563Ch from the second node with the container and deref checks; kind 6 (`vfunc[5Ch]`) -> `0077C8D0(member, first member)`; else kind 18h and not `009FFEB0(member)` -> `00A02020(member, 00A10C20(group))`. | 8 |

The existing model of 00A10DC0 in src/ai_command_tick.cpp (fragment record) is not this function: it adds a
null-group guard, counters, position and avoid-zone steps, and reads the leader before the loop. It was
not edited.

"Callers it unblocks" lists the reconstructed callers whose `missing_callees` contained the target;
callers with other missing callees stay blocked on those.

### 0085E4D0 in detail

- len2 = (wx*wx + wy*wy) + wz*wz, stored as a float. When len2 > 1e-10 (double at 00CE3820;
  FCOMI/JBE, so NaN takes the zero arm) len = sqrt(len2) through `00BF7030`, else 0. When
  1e-8 (double at 00D7A350) > len (FCOMIP/JA) the function returns without writing `out`.
- axis = w / len (FDIV, FDIV, FDIVR; written into the argument slots).
- The up vector, passed by value to the look-at: (0,1,0) when |ax| > |ay| and |ax| > |az|; else
  (0,0,1) when |ay| > |ax| and |ay| > |az|; else (1,0,0). |c| is c when c > 0, else
  -0.0f (00D7A208) - c. Every comparison takes the "not greater" arm on NaN.
- `00B63F10(M1, eye = 00F87574, &axis, up)`; `004134F0` copies M1 to M2 and M2 to M3; M3 is
  transposed in place; angle = -(float)(len * dt); `00B64780(M4, &angle)` (EDX is a pointer);
  `004134F0` copies M4 into M1; `in` is block-copied into M4; three `00413920` calls give
  ((in * M2) * M1) * M3; `004134F0` copies that into `out`; out row 3 (+30h..+38h) is taken from the
  copy of `in`; `0085D3D0(out)`.

## Existing implementations recorded ledger-only

| Address | Existing code | Verification |
|---|---|---|
| 00683320 | `bsp::end_thumb_drag` (src/frontend_screen_animation.cpp) | Body is `MOV byte ptr [ECX+48h],0; RET`; the C++ clears `dragging`, documented as +48h. Unblocks 005539E0, 00561320, 005D5D50, 005E6E20, 005EA990, 005FCAE0, 0061D770, 00568A30. |
| 00B179F0 | `native_material_diffuse_00b179f0` (src/gui_material_binding.cpp) | Body is `LEA EAX,[ECX+38h]; RET 4`; the C++ returns `lighting_38`, at +38h in `NativeMaterialStorage`. Unblocks 0053A8E0, 0059A860, 0059A9B0, 00B85200, 00BA9960, 00747560, 0093DE10. |

| 005547D0 | `get_native_input_settings_005547d0` (src/native_input_settings_lifetime.cpp) | Same order as the body: fast return of [00E198E8]; lock of the 00415350 manager's +10h object (depth +18h); re-test; new 540h; 006AB6B0; publish; 00BD0C30 register; unlock. Blocks 20. |

## Not done

- 009277E0 and 009277F0 are thunks (JMP 00923840 / 00923060), and both targets already have
  function records in `src/entity_mission_seg60_argon.cpp`. The current report lists no caller blocked
  on either, so nothing was added.
- 00B107F0 BSP_RenderResources_InitializeMembers: body 00B107F0..00B13028 (about 10 KB), already
  covered by five partial-projection fragments (`src/native_render_resource_init*.cpp`). Skipped:
  too large for this packet. It blocks 0067F080.
- 00441210 BSP_LuaObjectVector_Tidy (blocks 18): `std::vector<LuaObject>::_Tidy`, an STL
  instantiation; out of scope (vendor library). Its ledger text lacks an STL marker, so the report
  still counts it as a blocker.
- 0086A350 BSP_NativeQwordVector_Reserve (7) and 004D0640 BSP_NativeStringSet_Insert (7): STL
  instantiations (vector reserve, set insert); out of scope.

## Adoption note (gameplay hosts, not edited)

`bsp::rotate_about_axis_0085e4d0` (src/plane_advance_pose.cpp), which `game_hosts_units.cpp`
(the plane pose path) and `game_hosts_gunnery.cpp:3215` (00901E7A, GunBot intercept) call, differs from
0085E4D0 in two ways:

1. Up reference: it picks (1,0,0) or (0,0,1) by |az| > |ax| and never (0,1,0). The binary stores 1.0 into
   XMM6 (the y component) at 0085E62F when x dominates, takes (0,0,1) when y dominates, and (1,0,0)
   otherwise. As the existing comment says, the frame choice cancels in L * RotZ * transpose(L) in
   exact arithmetic, so this changes only float rounding.
2. It stops before `0085D3D0` (0085E85F), the re-orthonormalisation of the result. That call does change
   the matrix. LIVE on every path that calls the helper. No shared-RNG (00BD2F10) draw in either.
