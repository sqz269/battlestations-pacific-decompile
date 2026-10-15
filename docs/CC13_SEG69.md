# CC13 segment 69: AI command types, target weights and class readers (packet cc13-seg69)

Segment 69 is 009FE130..00A16030. This pass covered the 58 functions that had no
reconstruction record. Code: `include/bsp/cc13_seg69.hpp`, `src/cc13_seg69.cpp`, namespace
`bsp::cc13_seg69`. Every routine works on the original object as raw bytes (Win32 layout) and
reaches the engine through `Seg69Host`, one method per image routine, global or virtual slot.
These are new C++ interfaces, build-tested only. They are not drop-in binary replacements and
nothing here is wired into bsp_game.

Evidence: the exported pseudocode, checked against the Ghidra listing for every routine with
register inputs, x87 arithmetic or a suspicious decompile. Constants were read from the image:
00CE3958 = 2.0f, 00CE3D34 = 4.0f, 00CE4970 = 1e10f (501502F9h), 00D7A280 = double 0.5,
00D7A260 = -1.0f, and 00F87574 = 12 zero bytes. Strings: 00D21D00 " - ", 00D22A88 "commandType",
00D22C84 "command: ", 00D22C78 "targetPos", 00CFD964 "target", 00CE4390 "\n",
00D228A4 "aiGroupsTargetValue.txt", 00D15B38 "wt", 00CEB484 "%s".

## Reconstructed (46 addresses)

| Address | C++ | ABI | Notes |
|---|---|---|---|
| 009FE130 | `unit_ship_or_squadron_class_value_009fe130` | fastcall(ECX unit), ST0 | Kind 6 (vt+5Ch) answers [[+538h]+500h]. Kind 18h answers [[+35Ch]+188h]. Anything else answers 0. |
| 009FE170 | `class_ship_or_plane_value_009fe170` | fastcall(ECX object), ST0 | The kind test goes through vt+18h, not +5Ch. Type 6 answers +500h, type 0Fh answers +188h, anything else 0. |
| 009FE1B0 | `unit_droppable_ordnance_count_009fe1b0` | fastcall(ECX unit, EDX arg) | A squadron is replaced by its +3D0h plane. A plane stands as itself. The subject must pass 007B9140(ECX subject)(arg); it then answers +C54h. |
| 009FFE00 | `unit_is_available_for_ai_009ffe00` | fastcall(ECX unit), AL | Byte +5Ch must be set and bytes +5Dh, +60h and +5Eh clear. A squadron also needs [+3D0h]+72Ch vt+38h to answer true. |
| 009FFF20 | `unit_class_data_kind5_or_18_009fff20` | fastcall(ECX unit) | Kind 5 answers +538h, kind 18h answers +35Ch, anything else null. |
| 00A00020 | `ai_candidate_record_construct_00a00020` | thiscall(record)(7 args), RET 1Ch | Builds the 20h-byte record. A negative factor draws 00BD2F10(1, tuning+50h, tuning+54h), which is the **shared RNG**. A NaN factor is stored unchanged (COMISS/JBE). |
| 00A02010 | `ai_name_or_empty_00a02010` | fastcall(ECX object) | Answers +5Ch, or 00F8A728 when that is null. |
| 00A038C0 | `ai_forced_target_weight_clear_00a038c0` | cdecl | clear() on the map at 00F8A734: 00A01340(root), then the head links are reset. |
| 00A03A60 | `ai_hint_weight_maps_clear_00a03a60` | cdecl | Clears 00F8A740 the same way through 0096BD60. Then it clears the eight party maps 00F8A750 + 0Ch*i with the erase loop inlined (0096BD60 on the right child, 00BF65AC on the node). |
| 00A04050 | `ai_debug_describe_class_00a04050` | fastcall(ECX out, EDX class)(int), RET 4 | The result is the class name, followed by " - " and the device name when 009552E0 + 00443490(.., DL=0) resolve a device. The temporaries are released in the image's order. |
| 00A07EB0 | `ai_forced_target_weight_set_00a07eb0` | fastcall(ECX, EDX)(attacker, key3, float), RET 0Ch | The 16-byte key is found (00A03B90) and erased (00A04CB0). It is then stored as weight / 009FDF30(attacker vt+1Ch), divided in double and rounded to float (00A079B0). |
| 00A07F60 | `ai_hint_weight_set_global_00a07f60` | fastcall(ECX hint)(float), RET 4 | Stores into 00975C40(00F8A740)[hint]. |
| 00A07F80 | `ai_hint_weight_set_for_party_00a07f80` | fastcall(ECX hint, EDX party)(float), RET 4 | Stores into the map 00F8A750 + party*0Ch. Party is not bound-checked. |
| 00A07FB0 | `ai_group_min_speed_00a07fb0` | fastcall(ECX group), ST0 | Calls 00A07E40(group, &vec)(0), then 00A07C10(&vec), then deletes the vector's first pointer. The delete that Ghidra marked unreachable is reached. |
| 00A08020 | `ai_collect_spawn_point_class_records_00a08020` | fastcall(ECX owner, EDX out)(filter, kind), RET 8 | See below. Each record draws the **shared RNG** 00BD2F10 once. |
| 00A08420 | `ai_collect_candidate_records_00a08420` | fastcall(ECX owner, DL)(byte, out, filter, kind), RET 10h | Calls 00A08020 when DL is set, then 00A07D40(owner, out)(filter, 0) when the stack byte is set. |
| 00A0C1F0 | `ai_debug_target_weight_all_classes_00a0c1f0` | cdecl | Unreferenced. Evaluates 00A08460 over every class pair and level and discards the results. |
| 00A0FA70 | `ai_debug_write_groups_target_value_00a0fa70` | fastcall(ECX text) | Unreferenced developer command. It parses "(a, b)", calls 00A0F970 with [00E0E344] overridden, and writes aiGroupsTargetValue.txt. |
| 00A0FDE0, 00A108F0, 00A10A20, 00A10B40 | `ai_command_destruct_observer_body` | fastcall(ECX command) | Byte-identical except for the EH handler. Each calls 00695870(command ? +8 : 0), then writes vtable 00D22968. |
| 00A0FF60 | `ai_command_describe_type_00a0ff60` | thiscall(command)(lua), RET 4 | Calls 00B66790("commandType", 00E0E308[vt+4h()]). |
| 00A10240 | `ai_command_moveto_construct_00a10240` | thiscall(command)(group, point*), RET 8 | MOVETO: vtable 00D22ABC, +4h is the group, +8h..+10h the point. |
| 00A102D0 | `ai_command_cautious_move_construct_00a102d0` | as above | CAUTIOUSMOVE: +14h holds 00D22A08 then 00D22AE4, +18h = 0, +1Ch = 4, vtable 00D22AEC. |
| 00A10370 | `ai_command_regrouping_move_construct_00a10370` | as above | REGROUPINGMOVE: vtable 00D22B14. |
| 00A10710 | `ai_command_attack_construct_base_00a10710` | thiscall(command)(group, target), RET 8 | Observer base 00CE3CD4, then the ATTACK vtables 00D22B7C/00D22B64, +1Ch = target, then 00694A60(ECX target, EDX command+8). |
| 00A10890 | `ai_command_movetoattack_construct_00a10890` | as above | Base, then 00D22BBC/00D22BA4. |
| 00A109B0 | `ai_command_cautious_attack_construct_00a109b0` | as above | Base, then +20h = 00D22A08, +24h = 0, +28h = 4, 00D22C04/00D22BEC, then +20h = 00D22BE4. |
| 00A10AE0 | `ai_command_close_attack_construct_00a10ae0` | as above | Base, then 00D22C44/00D22C2C. |
| 00A12540, 00A12A50, 00A12F90 | `ai_command_{moveto,regrouping_move,patrol_to}_debug_draw_*` | fastcall(ECX command) | Debug draws with the drawing stripped. Only the owner leader's pose refresh (00414DB0) remains. |
| 00A12580 | `ai_cautious_approach_visit_route_points_00a12580` | thiscall(approach)(group, unused), RET 8 | Calls 00A10C20 after reading the director route (00778860, vt+0Ch, vt+8h, then 0059CAE0), and walks the point indices with checked-iterator tests only. |
| 00A12CD0, 00A12EA0 | `ai_command_{movetoattack,close_attack}_debug_draw_*` | as above | Refreshes the target leader, then the owner leader. |
| 00A12DB0 | `ai_command_cautious_attack_debug_draw_00a12db0` | as above | Calls 00A12580(+20h)(owner, &local), then refreshes target and owner while +24h is clear. |
| 00A12F10 | `ai_command_defend_position_debug_draw_00a12f10` | as above | Refreshes the owner twice, then calls 00A113D0(command)(&local). |
| 00A12FD0 | `ai_command_retreat_debug_draw_00a12fd0` | as above | Refreshes the owner, calls 00A11530(command)(&local), then refreshes the owner again. |
| 00A13080 | `ai_command_find_nearest_enemy_group_00a13080` | thiscall(command)(float* out, point*), RET 8 | See below. |
| 00A13280 | `ai_command_auto_close_attack_nearest_00a13280` | fastcall(ECX command) | When the nearest score is below (tuning+1F4h)^2, it builds an inline CLOSEATTACK (new(20h) + 00A10710 + vtables) and installs it with 00A2BD00. |
| 00A137D0 | `ai_command_describe_string_00a137d0` | thiscall(command)(out), RET 4 | Answers "command: " + type name + "\n". |
| 00A13960 | `ai_command_move_describe_lua_00a13960` | thiscall(command)(lua), RET 4 | Sets "commandType" and a "targetPos" table holding the +8h point (0088BA30). |
| 00A13A60 | `ai_command_attack_describe_lua_00a13a60` | as above | Sets "commandType" and "target" = the target group's first member's Lua self (00927B40). The empty group is not tested. |
| 00A15A70 | `ai_party_brain_construct_00a15a70` | thiscall(brain)(slot), RET 4 | Mode 4..7 builds the single mode planner. Any other mode builds Defend/Attack/Sell (38h each) and Capture (44h). +24h = 009FFD60(slot). |
| 00A15D00 | `ai_party_brain_debug_draw_planners_00a15d00` | fastcall(ECX brain) | Calls vt+28h on the planners 00A15A70 built for the mode. No null tests. |

### 00A08020

A local pair list is bought (006599C0) and filled by 0066E590(ECX owner, EDX &list)(-1). For
each unit in each pair's +10h list:

1. With a filter, the unit's world pose is refreshed inline when +C8h is clear. The parent +3Ch
   goes through 00414DB0, then 00413920 multiplies local*parent, or the +74h local matrix is used
   when there is no parent. The 16 floats go to +CCh, +C8h is set to 1 and +10Ch to 0. The unit
   is skipped when the filter's vt+0h accepts +FCh.
2. A unit whose +340h byte is clear is skipped.
3. 0084D560(ECX unit)(&classes, owner, 0) lists the supply classes. For each class index,
   00964790(index, DL=1) is filtered by kind:
   - below 0, or above 6: every class passes;
   - 0: vt+18h(7);
   - 1 and 2: vt+18h(0Ah), then +808h set requires kind 2 and +808h clear requires kind 1;
   - 3: 0Dh; 4: 0Eh; 5: 8; 6: 0Fh.
4. A class that passes is pushed (00A078F0) as {class, 0, 0, 0, class+134h, 009FDF30 weight,
   00BD2F10(1, +50h, +54h), 0}.
5. The class list is freed inline.

At the end the pair list goes to 006654C0 and its head is deleted. The vacuous checked-iterator
tests on the local list are omitted.

### 00A13080

The team index is (owner +5638h == 0), so team 0 searches list 1 and every other team searches
list 0 (00F8AA48 + 0Ch*index).

The own point is the leader's +FCh x/z, or 00F87574 for an empty group. With the second
argument it becomes the midpoint: the float sum times double 0.5, rounded to float.

A group is a candidate unless its 00A2C3D0 ratio is <= 0. The test is FLDZ/FCOMIP/JNC, so a NaN
ratio is a candidate. The score is (dx^2+dz^2, rounded to float) / (own +5644h + its +5644h),
computed with FIDIV. The best score starts at 1e10f and is replaced only by a strictly lower
score.

## Existing in the cc9 hosts, recorded ledger-only (6)

| Address | Host | Verified |
|---|---|---|
| 009FF3A0 | inline in `src/ai_target_weights.cpp` (level-bomb drops) | `p2 > (float)ammo ? (float)ammo : p2`, truncated. Matches FCOMIP/JBE + CVTTSS2SI; a NaN takes p2. |
| 009FFD60 | `slot_team` in `src/game_hosts_ai.cpp` | **Substitution.** See adoption note 1. |
| 009FFEB0 | `tick_squadron_excluded_009ffeb0` / `rtb_exclusion_arm` in `src/game_hosts_ai.cpp` | Matches the [00E17BF2] gate, 007EDAD0 == 0, class 10h/11h/12h, 007F16D0 + 0077D600(desc, &rec, 1). See adoption note 2. |
| 00A0FC50 | `command_pass_interval` in `src/game_hosts_ai.cpp`, constants in `include/bsp/ai_command_object.hpp` | Exact: it writes 2.0f and 4.0f to the two out pointers, RET 8. |
| 00A0FC90 | `ai_command_notification_is_discarded` (`src/ai_command_object.cpp`) | Exact: a bare `RET 8`. |
| 00A152E0 | `CautiousAttack` arm of `src/ai_command_tick.cpp` | Same calls and the same promotion. See adoption note 3. |

## Adoption notes for the cc9 lead

1. **009FFD60 (slot team).** The image reads `[[00E188A8]+18CCh+slot*4]+28h`.
   The host's `slot_team` reads `g_scene_slot_parties[slot]` and falls back to the slot when
   that table is unpublished. This is LIVE in every mission through the party gate and
   `create_party_brain`. It does not touch the shared RNG.
2. **009FFEB0.** The host additionally runs its 007EDA90 stand-in, `tick_squadron_excluded_007eda90` (a census),
   before the arm. The image has no such call: its arm runs 009FFEC6..009FFF16 directly. This is
   harmless only if that host call has no side effects. The image reads the class id as
   `[[squadron+3D0h]+C4h]`, which the host takes as `unit_class_id(head)`. LIVE through the
   close-attack member gate (00A1441B), and also through the `kCloseAttackSquadronHomeBound`
   switch. It does not touch the RNG.
3. **00A152E0.** The image passes ECX = command to 00A10DC0 and 00A11690, and ECX = command+20h
   to 00A14DD0. The host passes the group, which is a representation difference only.
   - The promotion test computes dx^2+dz^2 in x87 and rounds it to float. It then
     rounds c^2 to float and compares with FCOMIP/JBE. The host computes in float, so a
     knife-edge distance can differ by one ulp.
   - The host guards a null target group; the image does not.
   - It is LIVE whenever a group holds CAUTIOUSATTACK. No RNG use.
4. **00A15A70.** The host's `create_party_brain` fills eight planner records for every game
   mode. The image builds only the mode planner (modes 4..7) or the four base planners (any
   other mode); the other slots stay null. This packet did not trace whether the host's tick
   skips the records the image would leave null. +24h is 009FFD60 (note 1). LIVE: one brain per
   AI party. No RNG use.
5. **00A07F60 / 00A07F80.** `include/bsp/lua_binding_ai.hpp` declares
   `ai_hint_weight_set_global` and `ai_hint_weight_set_for_party`, but no host in `src/`
   overrides them. The reconstructions here are what an override would do. In this
   installation, only the multiplayer competitive scripts call SetHintWeight.

## Skipped (6), with reasons

00A06840, 00A072D0, 00A09810, 00A0A280, 00A0B420 and 00A0DC10 are the developer report writers
and query parsers: aiVStable.txt, the vs-table to/from writers, the class and equipment list
parsers, and the attacker-query evaluator. None has a caller. Ghidra shows none, and an
absolute-pointer scan found none; the scan's positive control (00A0FC90) does find hits.
00A06840 and 00A072D0 are reached only from 00A09810 and 00A0DC10.

They total about 3,700 decompiled lines of inlined NativeString construction, find and substr
work and printf formatting. A faithful reconstruction would need instruction-level reading of
every inlined string temporary. That was not attempted in this packet, so a partial rewrite
would be a stand-in.

## Uncertainty

- The names are hypotheses (the ledger names), not recovered symbols.
- 00A12580's second argument is never read. Its caller passes a 10h-byte local.
- 00BF6713 is modelled as a returning call, because the listing continues after it.
