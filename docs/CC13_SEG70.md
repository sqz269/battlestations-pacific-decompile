# CC13 SEG70: the AI coordinator's leaves (packet cc13-seg70)

Segment 70 spans 00A16050-00A371A0 and covers the high-level AI coordinator: the party brains, the planners (Defend, Attack, Sell, Capture, Duel, Escort, Siege, Competitive), the AI group's commands and membership, the Lua group info, and the forced-target-weight rules. The pool held 54 functions with no reconstruction record. The results:

- 43 are reconstructed in `src/cc13_seg70.cpp` / `include/bsp/cc13_seg70.hpp`.
- 7 already had an implementation. Each was verified against the listing and given a record.
- 4 were skipped.

Every name here is a hypothesis, not a recovered symbol. The code is reconstructed and build-tested only. It provides new C++ interfaces, is not ABI-compatible, and is not wired into `bsp_game`.

## 1. USNOS check: capture and objective members (done first)

The cc9 lead's USNOS stall prompted this check. The question was whether seg 70's capture and objective-member logic could explain HQs that will not capture or Betty members that keep the mission alive.

- **objectiveMembers / autoGrouping.** These are only fields of the Lua table that `00A2EEE0` fills for `AIGetGroupInfo`, whose single caller is 00A3799F. No script in this installation calls `AIGetGroupInfo` or reads either field. Search: every `*.lua` under `scripts`.
- **The USNOS end test.** `us_osumi.lua:1181` calls `luaRemoveDeadsFromTable(Mission.FinalTrgs)`. The six Betty squadrons are appended at :1813. That test is `unit.Dead`, which belongs to the plane lane, not seg 70.
- **Capture progress is not here.** `00A335D0` loads the per-mode tuning block, which includes `Capture_InRangeCaptureMul`, `Capture_LandedCaptureMul` and `Capture_SmallLandingShipSurviveMul` at +6Ch..+78h. All 33 callers of the block accessor `00A371A0` are AI planner or command code in segments 68-70. No CommandBuilding unit code reads them. The capture rate and radius live in the CommandBuilding code.
- **The hosted Capture planner.** `capture_think_00a29fd0`, `capture_plan_00a29fd0` and `defend_score_00a243d0` in `src/game_hosts_ai.cpp` match the listing. Only the residuals that docs/CC13_GPR_69.md 7.3 and 7.4 already list remain, and none is live on USNOS:
  - The activity filter on `have_target`. With `kCaptureNoTargetImageBound` ON both branches run the same path.
  - The assignment score is formed in double.
  - `00A1E250`'s 35-for-0 substitute. +19Ch is authored 35.
  - `StrategicGain` is fixed at 0. It is authored only in `multi/islandcapture*.lua`.

  None of these draws on 00BD2F10. The team lead was told before the reconstruction began.

## 2. Model

Each routine works on the original object as raw bytes, with offsets as in the image. It reaches the engine only through `Seg70Host`. Every method of that interface names the image routine, virtual slot or global it stands for, and every image call happens at the same point with the same arguments.

The `_SECURE_SCL` checked-iterator reports (CALL 00BF6713) are not modelled. They fire only on a corrupted list.

## 3. Reconstructed (43)

| Address | Model | ABI | Notes |
| --- | --- | --- | --- |
| 00A16490 | `party_brain_destruct_body_00a16490` | fastcall(brain) | eight slots, vtable[0](1), zeroed |
| 00A16990 | `party_brain_delete_00a16990` | fastcall(slot) | 00A16490, _free 00BF65AC; unreferenced |
| 00A16B20 | `destroy_party_brains_00a16b20` | void() | 00F8A89C..<00F8A8BC |
| 00A16CC0 | `debug_capture_scores_stripped_00a16cc0` | void() | scores each CommandBuilding (00A243D0 own team [00E0E354], else 00A1E250), colour guard 00F8A9B8, inline world-matrix refresh; nothing drawn; unreferenced |
| 00A184F0 | `party_aggressive_ratio_00a184f0` | fastcall(party) -> ST0 | float at 00F8A8D0 + party*1Ch |
| 00A185E0 | `group_order_selling_00a185e0` | (group) RET 4 | SELLING vtable 00D229B8 through 00A2BD00 unless 00A2BE10; a failed allocation installs 0 |
| 00A1EE50 | `planner_construct_base_00a1ee50` | thiscall(brain) RET 4 | list head 00A168A0, +30h side, +34h enemy (SETZ) |
| 00A1EFC0, 00A1F0F0, 00A1F220, 00A1F390, 00A1F500, 00A1F630, 00A1F760, 00A1F890 | `planner_construct_00a1efc0_family` | thiscall(brain) RET 4 | vtable and literal name per kind (table in the source); Capture adds +38h/+3Ch = 0, +40h = -1.0f |
| 00A1F310 | `planner_sell_release_group_00a1f310` | thiscall(group) RET 4 | same body as the hosted 00A1E210 |
| 00A1FC30 | `planner_capture_debug_draw_stripped_00a1fc30` | fastcall(planner) | Capture slot +28h; 00A1E250 per enemy CommandBuilding, 00414DB0 on the entity |
| 00A22920 | `planner_split_groups_by_class_00a22920` | fastcall(planner) | unreferenced; see 5.1 |
| 00A25B90 | `planner_quick_spawn_00a25b90` | thiscall(name, flag, a, b) RET 10h | 00F8AB6A gate, 00946970 == 0.0, 00A08420, 00A25A30(point or 0, 1.0f, ...) |
| 00A25CB0 | `planner_defend_debug_draw_stripped_00a25cb0` | fastcall(planner) | 00A2DEF0 per owned group, 00A243D0 per item, inline FLD/FSTP matrix store |
| 00A2BD00 | `group_set_command_00a2bd00` | thiscall(command) RET 4 | deletes the old command first |
| 00A2BD50 | `group_copy_command_from_00a2bd50` | thiscall(other) RET 4 | other's command vtable+20h(group), no null test |
| 00A2BD90 | `group_forward_command_00a2bd90` | thiscall(a, b) RET 8 | tail jump to vtable+24h |
| 00A2BDB0 | `group_is_attacking_target_00a2bdb0` | thiscall(target) RET 4 | IsType(6) and +1Ch |
| 00A2BE10 | `group_is_selling_00a2be10` | thiscall | IsType(0Eh) |
| 00A2BE70 | `group_order_retreat_00a2be70` | fastcall(group) | RETREAT vtable 00D22A60 unless IsType(0Dh); unreferenced |
| 00A2C050 | `debug_default_text_style_00a2c050` | void() | {0, 0.5, 1, 1} at 00F8AAE8 |
| 00A2C2A0 | `group_order_regrouping_move_00a2c2a0` | thiscall(point, unused) RET 8 | vtables 00D22A94 then 00D22B14; unreferenced |
| 00A2D9D0 | `group_remove_entity_00a2d9d0` | thiscall(entity, queue) RET 8 | 0077BEA0, +16Ch, 006956A0, the emptied list 00F8AA78 |
| 00A2EEE0 | `group_fill_lua_info_00a2eee0` | thiscall(table) RET 4 | see 5.2 |
| 00A317C0 | `controller_tick_element_00a317c0` | fastcall | this+170h or 0 |
| 00A31800 | `coordinator_reset_singleton_00a31800` | fastcall | [00F8AAFC] = 0, vtable 00CE3818 |
| 00A31A40 | `array32_grow_00a31a40` | fastcall(array) | 2n+2, 20h elements, null-skipped copies |
| 00A31B10 | `array_dword_reserve_00a31b10` | thiscall(n) RET 4 | MUL/SETO size |
| 00A31C20 | `controller_message_construct_00a31c20` | thiscall(value) RET 4 | class 4Bh, vtable 00D02C90 |
| 00A31C90 | `controller_send_party_message_00a31c90` | thiscall(unit, value) RET 8 | D3h for 1Ch, 4Bh for 9; 0077C2A0(msg, 7, 0); +528h = -1 |
| 00A32120 | `array32_push_back_00a32120` | thiscall(element) RET 4 | |
| 00A32200 | `array_dword_grow_00a32200` | fastcall(array) | grows only when 2n+2 > n |
| 00A32460 | `controller_destruct_body_00a32460` | fastcall | 00A16B20, 00A038C0, [00F8AB60] = 0, 00875490, 00925780 |
| 00A32500 | `forced_target_weight_insert_00a32500` | fastcall(rule, DL)(mode) RET 4 | see 5.3 |
| 00A32880 | `array_dword_push_back_00a32880` | thiscall(value) RET 4 | |
| 00A329F0 | `controller_broadcast_party_state_00a329f0` | thiscall | see 5.4; unreferenced |
| 00A371A0 | `active_mode_tuning_00a371a0` | void() | coordinator + 4 + mode*23Ch |

Constants were read from the image:

| Address | Bytes | Value |
| --- | --- | --- |
| 00D7A24C | 00 00 80 3F | 1.0f |
| 00D7A260 | 00 00 80 BF | -1.0f |
| 00CE3800 | 00 00 00 3F | 0.5f |

## 4. Existing implementations, verified and recorded (7)

| Address | Where | Verdict |
| --- | --- | --- |
| 00A1E210 | `capture_hand_off`, src/game_hosts_ai.cpp | Release and claim match. The host has no observer objects. Partial. |
| 00A2C8D0 | `can_auto_merge`, src/game_hosts_ai.cpp | Matches. The host assumes +5648h = 1, and no producer in this process clears it. |
| 00A2D440 | `destroy_group`, src/game_hosts_ai.cpp | Partial, as the host says. The notify and the list removals are modelled. Not modelled: the observer lock 00694280, the clear of [00F8A9DC], and the sub-object teardown (004C4900, 00695870, 00695760). |
| 00A2DB50 | `apply_party_change_removals_00a2db50` | The removal matches. The host polls where the image reacts to event 6. |
| 00A2DDE0 | `evict_invalid_members` | The same members leave. The host makes one pass where the image restarts from the head after each removal. |
| 00A2DFA0 | `create_group` | Matches. Not modelled: the group id counter 00E0E348 (+5658h) and +5624h..+562Ch. The host clamps a team outside 0..2 to 0, which the image does not. |
| 00A31C60 | src/unit_kind_query.cpp, row 4Bh | Matches: accepts 0 and 4Bh, plus the dynamic +C4h test. |

## 5. Notes on the larger routines

### 5.1 `00A22920` splits groups by class

The listing does not match the earlier naming note, which said "each class gets its own group". For each owned group with 2 or more members, the routine removes every member in turn with `00A2D9D0(member, 0)` and then sorts it:

- A member that does not answer IsKindOf(5) is put back into the group.
- The first member of each `+538h` class is also put back.
- For a later member of a known class, the class counter is read and then incremented. If the counter equals the number of groups made so far, the member founds a new group: new 5660h, `00A2DFA0`, then `00A2BD50` copies the old group's command.
- Otherwise the member joins the group made at the counter's index, whatever that group's class is.

The new groups are then claimed by the planner: `+20h` append, `+5654h`, `00694A60`, `+2Ch = 1`. `00A2C600`'s answer is computed and discarded. The image's three 128-entry stack arrays have no bound check, and neither does the model.

### 5.2 `00A2EEE0`, the AIGetGroupInfo table

The routine writes these keys:

| Key | Value |
| --- | --- |
| `leader` | The first member's Lua self object. |
| `members` | 1-based. |
| `autoGrouping` | Byte +5648h. |
| `party` | The team, +5638h. |
| `playerIndex` | The party slot, +5634h. |
| `reconRatio` | `00A2C3D0`'s float truncated by `_ftol2` (00BF7420). Lua sees an integer, so a ratio below 1 reads 0. |
| `objectiveMembers` | `00A2DEF0` over the team's world set. |
| `unitTypes` | Name to count, with names from 00E0B590. |
| `command` | Filled by the command's vtable+18h. |

The teardown keeps the image's order.

### 5.3 `00A32500`, the forced-rule insert

DL selects the table:

- DL set: the global table at 00F8AB5C.
- DL clear: the per-mode table at 00F8AB08 + mode*0Ch. A negative mode is replaced by `009FFC80`'s.

The routine removes the first identical rule, comparing [0], [1], the low byte of [2], the low byte of [4], +1Dh, +1Eh, and, when [4] is set, [5], [6] and the low byte of [7]. The weight is not compared. It then grows the table to 2n+2 when full, appends the rule, and calls `00A038C0`.

### 5.4 `00A329F0`, the party broadcast

The routine runs only when `004BCA50` <= 3. It records two things:

- The present players: `[game+18CCh+i*4]` +8 set and +9 clear.
- Whether an AI party is enabled in slots 0..3 and in slots 4..7 (`009FFE50`).

It then walks world list +58h. A unit counts when it answers IsKindOf 9 or 1Ch, has side <= 1 and has player slot < 8; both compares are signed.

- **The unit's side has an AI party.** The unit is sent to slot 0 when slots 0..3 hold an AI party, else to slot 4, unless it is already there.
- **No AI party on its side.** A unit whose player is absent is collected. The collected units are then dealt round-robin to the present players of base 4 or 0, with the cursor starting at 3.

Labelled:

- A negative side or slot indexes the image's stack below its arrays. The model reads those bytes as 0, which is exact only for slot -1..-3.
- When no player of the target bucket is present, the image loops forever, and so does the model.

## 6. Skipped (4)

| Address | Reason |
| --- | --- |
| 00A31000 BSP_AiDebug_GroupOverlay | Has no caller: no rel32 and no absolute reference. It is a debug overlay of about 300 decompiled lines, gated on `[[00E198C4]+54h]+5`. |
| 00A2F6F0 BSP_AiGroup_DescribeDebug | About 1000 decompiled lines of debug string building. Its only caller is 00A31000. |
| 00A2D580 BSP_AiDebug_GroupMembersOverlay | Its only caller is 00A31000. |
| 00A2CDD0 BSP_AiDebug_GroupTargetText | Its only caller is 00A2D580. |

These four make up the unreached debug-text chain. A faithful model is mostly native-string formatting and does not change behaviour, so the effort went to the reached routines instead.

## 7. Uncertainty

- The message objects in `00A31C90` are modelled as zeroed stack buffers of 24h and 34h bytes. What `0075B430` writes beyond the fields named here is the host's.
- The field roles are hypotheses. Examples are `+5634h` as the party slot and `+5638h` as the team; the Lua keys support them.
- Nothing here was run. The Ghidra function bodies were not annotated by this packet.
