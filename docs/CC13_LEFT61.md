# CC13 leftovers of segments 61 and 1 (cc13-left61)

Packet: the functions of segment 61 (0092E0B0..00951F20) and segment 1 (00413D10..0042A830)
that the first cc13 wave left without a reconstruction record. Code:
`src/cc13_left61_seg61.cpp`, `src/cc13_left61_seg1.cpp`, `include/bsp/cc13_left61.hpp`
(namespace `bsp::cc13_left61`).

Objects are reached through `Raw` byte offsets in their Win32 layout. Every call the image makes
goes through a `Left61Host` method named for its target, except pure CRT routines (memcpy,
strlen, memmove_s, _stricmp), which are called directly, and calls between functions of this
packet, which call the C++ directly. The MSVC9 container helpers that exist in several identical
instantiations share one body and take the image's callee addresses as data, so each
instantiation still calls exactly its own targets. Names are hypotheses, not recovered symbols.

Status of every row: reconstructed and Win32 build-tested; no fixture, no original-ABI or game
validation. x87 float chains are written as float arithmetic in the image's operation order: the
main thread runs at 24-bit precision control (docs/CC13_FIXTURE_PASS_1.md). Unwind actions of EH
states that only run when a callee throws are not modelled unless the row says otherwise; catch
blocks (destroy the built range, then rethrow) are modelled.

Most FUN_ bodies here had no export; they were read from the live Ghidra decompiler and listing.


## Segment 61 reconstructed (92)

| Address | C++ | ABI | Notes |
|---|---|---|---|
| 0092E250 | `transform_box_0092e250` | __thiscall(ECX = const float box[6]; float out[6], const float matrix[12]), RET 8 | Box by affine matrix as centre/extent: centre and half extent stored to float (x0.5 double 00D7A280), centre through 004142E0, |rotation| (COMISS > 0 keeps, else -0.0 (00D7A208) - x, NaN included) and 0042D0D0(out, extent, |m|, 0); out = centre -/+ radius. The image leaves the |m| fourth column unwritten (0042D0D0 does not read it); zero here. |
| 0092F2E0 | `unit_controller_apply_network_state_0092f2e0` | __thiscall(ECX = controller; const dword state[8]), RET 4, tail JMP 0092E5B0 | Copies eight dwords into controller+40h..+5Ch, byte +60h = 1, then the tail jump to 0092E5B0 with its stack argument replaced by 1 (host hook). |
| 0092FD50 | `two_vector_element_destroy_0092fd50` | __fastcall(ECX = element), RET | Two vectors at +0 and +10h: operator delete (00BF65AC) of +14h then +4 when set; first/last/end zeroed after each. |
| 00930520 | `vector20_buy_00930520` | __thiscall(ECX = vector; count), RET 4, AL | vector<14h-byte>::_Buy: zero first/last/end, false for 0, 009302A0 when count > 0CCCCCCCh, 0092C810(ECX = count, EDX = 0). |
| 00930570 | `vector20_buy_00930570` | __thiscall(ECX = vector; count), RET 4, AL | Same body as 00930520 with 00930310 / 0092C870. |
| 00930F20 | `vector20_assign_00930f20` | __thiscall(ECX = this; const vector* right), RET 4, EAX = this | MSVC9 vector::operator= for a POD 14h-byte element: clear 0092FF80; copy 0092F010 (cdecl); size 0092C060; _Buy 00930520; _Ucopy 0092F8D0 (thiscall, RET 0Ch); old storage through operator delete. Branch order and the recomputed counts follow the listing 00930F20..0093105E. |
| 00931070 | `vector20_assign_00931070` | __thiscall(ECX = this; const vector* right), RET 4, EAX = this | Same body as 00930F20 with 0092FFC0, 0092F080, 0092C0C0, 00930570, 0092F900. |
| 009311C0 | `construct_vector_009311c0` | __fastcall(ECX = memory, EDX = const vector*), RET | _Construct: 00930CE0(ECX = memory, source) when memory is not null; the EH frame has no unwind action. |
| 00931200 | `construct_vector_00931200` | __fastcall(ECX = memory, EDX = const vector*), RET | Same as 009311C0 with 00930DD0. |
| 009317D0 | `allocator_construct_009317d0` | __thiscall(ECX = allocator (unused); memory, const vector*), RET 8 | allocator::construct: 00930CE0 when memory is not null. |
| 00931820 | `allocator_construct_00931820` | __thiscall(ECX = allocator (unused); memory, const vector*), RET 8 | Same as 009317D0 with 00930DD0. |
| 009319D0 | `debris_entry_vector_resize_009319d0` | __thiscall(ECX = vector; count), RET 4 | resize(count) as 00931530(count, 14h-byte value); value dwords +0 and +10h are zero, +4..+0Ch are copied from uninitialised stack slots in the image (passed as zero here: indeterminate in the image). |
| 00931B70 | `uninit_fill_n_vectors_00931b70` | __fastcall(ECX = first, EDX = count; const vector*, allocator, 2 tags), RET 10h | _Uninit_fill_n of 10h-byte vectors through 00930CE0 (null slots skipped, still advanced); catch 00931BCB destroys [first, current) through 00930EC0(allocator, p) and rethrows (00BF6885(0,0)). |
| 00931C10 | `uninit_fill_n_vectors_00931c10` | __fastcall(ECX = first, EDX = count; const vector*, allocator, 2 tags), RET 10h | Same as 00931B70 with 00930DD0 / 00930EF0. |
| 00931CB0 | `uninit_swap_move_vectors_00931cb0` | __fastcall(ECX = first, EDX = last; dest, allocator, 2 tags), RET 10h, EAX = dest end | _Swap_move: copy-construct an empty local vector into each non-null slot (00930CE0), then swap +4/+8/+0Ch with the source (unguarded); local freed through operator delete when set; catch 00931D3E destroys [dest, current) and rethrows. The local's own unwind action is not modelled (it never owns storage). |
| 00931DE0 | `uninit_swap_move_vectors_00931de0` | __fastcall(ECX = first, EDX = last; dest, allocator, 2 tags), RET 10h, EAX = dest end | Same as 00931CB0 with 00930DD0 / 00930EF0. |
| 009322F0 | `insert_n_nested_009322f0` | __thiscall(ECX = vector; where {container, ptr}, count, const vector* value), RET 10h | MSVC9 vector<vector<T>>::_Insert_n, read from the listing 009322F0..00932561: _Tmp copy 00930CE0; max 0FFFFFFFh (009301C0); growth cap + cap/2 (0 on overflow) raised to the need; 0092C750; 00931CB0, 009320D0, 00931CB0; old range 009316F0 + operator delete; catch 0093245E (00931F10, delete, rethrow); spill branch 00932190, 009320D0 (catch 009324E7), 00931AF0; in-place branch 00932190, 009321C0, 00931AF0; _Tmp freed. _Tmp's unwind action not modelled. |
| 00932570 | `insert_n_nested_00932570` | __thiscall(ECX = vector; where {container, ptr}, count, const vector* value), RET 10h | Same body as 009322F0 (call lists compared) with 00930DD0, 00930230, 0092C7B0, 00931DE0, 00932150, 00931760, 00931F30, 00932200, 00931B40, 00932230. |
| 0093A8B0 | `named_row_find_0093a8b0` | __fastcall(ECX = first, EDX = last; const native string* name), RET 4, EAX | Linear find over 10h-byte rows: lengths compared first, equal zero lengths match, else _stricmp (00BF7FBF) of the data pointers; the unreachable 'right empty' arm returns 1 as inlined. |
| 0093A9E0 | `named_row_find_iterator_0093a9e0` | __fastcall(ECX = out iterator, EDX = name; first {cont, ptr}, last {cont, ptr}), RET 10h, EAX = out | out = {first.container, 0093A8B0(first.ptr, last.ptr, name)}. |
| 0093AC30 | `named_row_release_name_0093ac30` | __fastcall(ECX = row), RET | When +8 is set: 00419CC0() then 00BD1510(pool; data, length + 1, 1). |
| 0093AD30 | `named_row_copy_construct_0093ad30` | __thiscall(ECX = row; const row*), RET 4, EAX = row | +0 copied, string zeroed, 0041DD40(length, 1) + memcpy when the source is not empty, float +0Ch by FLD/FSTP. |
| 0093AD90 | `named_row_fill_0093ad90` | __cdecl(first, last, const row*) | Fill by the inlined row assignment (+0, name through 0041DD40/memcpy unless the same string, float +0Ch). |
| 0093ADF0 | `named_row_copy_0093adf0` | __cdecl(first, last, dest), EAX = dest end | Copy by the inlined row assignment. |
| 0093AE60 | `named_row_construct_0093ae60` | __fastcall(ECX = memory, EDX = const row*), RET | _Construct through 0093AD30 when memory is not null. |
| 0093AEC0 | `named_row_copy_backward_0093aec0` | __cdecl(first, last, dest_end), EAX = dest begin | copy_backward by the inlined row assignment. |
| 0093AFF0 | `named_row_destroy_0093aff0` | __thiscall(ECX = allocator (unused); row), RET 4 | Same body as 0093AC30 on the argument. |
| 0093B070 | `named_row_uninit_copy_0093b070` | __fastcall(ECX = first, EDX = last; dest, allocator, 2 tags), RET 10h, EAX = dest end | _Uninit_copy with the inlined row copy construction; catch 0093B115 destroys [dest, current) through 0093AFF0 and rethrows. |
| 0093B210 | `named_row_uninit_fill_n_0093b210` | __fastcall(ECX = first, EDX = count; const row*, allocator, 2 tags), RET 10h | _Uninit_fill_n; catch 0093B2B9 destroys through 0093AFF0 and rethrows. |
| 0093B690 | `insert_n_named_row_0093b690` | __thiscall(ECX = vector; where {container, ptr}, count, const row* value), RET 10h | Same _Insert_n body as 009322F0 (growth arithmetic and call list checked) with 0093AD30, 0093B150, 0093A1C0, 0093B070, 0093B4F0, 0093B300, 0093B530, 0093B620, 0093AD90, 0093B460; _Tmp's name returns to the string pool (0093B8E9). |
| 0093CB10 | `simple_3d_scene_construct_0093cb10` | __fastcall(ECX = scene), RET | vtable 00D19724, +4..+10h and +24h zero. |
| 0093CC20 | `simple_3d_scene_release_0093cc20` | __fastcall(ECX = scene), RET | +0Ch: InterlockedDecrement (import 00CE2220) of +4, vtable slot 0 at zero, cleared; +10h and +8h through 00B6DFA0; +4h through 00B72250; each cleared only when set. |
| 0093D250 | `simple_3d_scene_submit_render_0093d250` | __fastcall(ECX = scene), RET | Label "X" (00CE9A38) built by 0041E870 in the outgoing argument area; [00F8D39C] read, 004C11F0, 00B0D0D0(ECX = that object), then 00B1F4D0(queue; +4, +8, targets, label by value). The label's unwind action is not modelled. |
| 0093E510 | `map_overlay_draw_world_line_0093e510` | __thiscall(ECX = overlay; x0, y0, x1, y1), RET 10h | Screen points ox + sx*(x - wx), oy - (y - wy)*sy; 00860030(ECX = overlay+8 rgba). Float arithmetic in the image's order (main thread 24-bit precision control). |
| 0093EAE0 | `map_overlay_draw_arrow_head_0093eae0` | __thiscall(ECX = overlay; const float point[2], const float dir[2], size), RET 0Ch | Scaled direction (dx*sx, -dy*sy) normalised by 00419260; two 00860030 strokes from the point to base +/- side with side = (-uy, ux) * size * 0.5. |
| 0093EC80 | `map_overlay_draw_arrow_mid_head_0093ec80` | __thiscall(ECX = overlay; const float from[2], const float to[2], size), RET 0Ch | size limited to (b*b + a*a) * 0.5 (a = sx*dx, b = sy*-dy; FCOMIP JBE keeps size on NaN); head at the midpoint ((from+to)*0.5), then 0093E510. |
| 0093EDC0 | `map_overlay_draw_arrow_0093edc0` | __thiscall(ECX = overlay; const float from[2], const float to[2], size), RET 0Ch | As 0093EC80 with the head at `to`. |
| 0093EEB0 | `map_overlay_draw_world_circle_0093eeb0` | __thiscall(ECX = overlay; const float centre[2], radius), RET 8 | radius / sx, radius / sy stored to float, then 0093E9A0(centre, &r). |
| 0093F280 | `sound_overlay_axis_step_0093f280` | CL = char round_up; stack float value, RET 4, ST0 | Read from the listing (the decompiler mixes the branch constants): |value| (<= 0 -> -0.0 - v); below 100/200/500 step*ftol((v +/- step)/step + 0.5) with step 5/10/50; below 1000/4000/else 0093EEE0(v +/- step, step) with 100/200/500 (0093EEE0 is __stdcall RET 8, not cdecl as its ledger name says); clamp to [1, 1e6]. |
| 0093F4B0 | `sound_overlay_compare_0093f4b0` | __cdecl(context, object** a, object** b), EAX | d = b->vfunc1C() - a->vfunc1C() (a's value stored to float first): -1 if 0 > d, 1 if d > 0, else 0. |
| 0093FA70 | `sound_overlay_place_label_0093fa70` | __thiscall(ECX = overlay, 0Ch-element array at +4; float out[2], x, y, sound*), RET 10h, EAX = out | y += 12 (double 00CE42D0) until no entry equals (x, y) (the image scans four per step); grow through 0093F590 (ECX = overlay+4) to cap*2 (1 below 2) when full; append {x, y, sound} when the slot is not null (the sound pointer is stored as a dword); out = (x, y). Fixed 2026-10-14 (cc13-fix4): the fourth argument was typed as a float `z`, and 0093F590 was passed the overlay instead of overlay+4. |
| 0093FE90 | `sound_overlay_apply_sample_settings_0093fe90` | __thiscall(ECX = overlay; 48h-byte options by value), RET 48h | 0093FDF0 copy into the argument area, 00A83120(ECX = overlay+4Ch), vtable +3Ch, then the by-value options destroyed inline: 0093F950(options+3Ch, 0) and _free (00BF6989) of options+3Ch. Unwind action not modelled. |
| 009420A0 | `spawn_member_seed_property_bag_009420a0` | __thiscall(ECX = member; party), RET 4 | 008F3710(ECX = member+0Ch) for Party = party, Race = 2 - (party != 0), Skill = 2*(game+1FE4h != 0) + 1, OwnerPlayer = 9, Behaviour = 1, and Type = class+70h when member+0 is set (keys 00CE5804, 00CE8EE0, 00CF8838, 00CF882C, 00CFACBC, 00CE4780). |
| 00942130 | `team_support_points_cap_00942130` | __fastcall(ECX = player), ST0 | Counts the eight player records game+18CCh.. with +8 set, (+9 clear or +0Ah set) and +28h equal to the player's team; float[00E0CFB4] * 0.5 / count (FIDIV), 0 when none. |
| 00943C00 | `spawn_list_buy_node_00943c00` | (ECX unused; next, prev, const dword* record), RET 0Ch, EAX = node | operator new(0Ch) (00BF681B); the node+4 and node+8 null tests never fail (written even when new returned null), as in the image. |
| 00944130 | `spawn_member_construct_00944130` | __thiscall(ECX = member; party, class, name by value {length, data}), RET 10h, EAX | member+0 = class, name copied (0041DD40 + memcpy), bag = new(114h) + 008F41A0(0) or null, 009420A0(party), the by-value name returned to the string pool. Unwind states 1/2 not modelled. |
| 00944210 | `spawn_member_read_lua_table_00944210` | __thiscall(ECX = member; Lua table), RET 4 | 0043D8F0(ECX = table, EDX = bag); member+0 = 00964790(ECX = Find(bag, "Type")+0Ch, DL = 1); name = 008F0DF0(Find(bag, "Name")), strlen (0 for null), 0041DD40(length, 0), memcpy when data is set. Finds are not null-checked. |
| 00944400 | `pointer_range_copy_00944400` | __cdecl(first, last, dest), EAX = dest end | memmove_s of the signed dword count (SAR 2) when not zero. |
| 009444C0 | `spawn_member_copy_backward_009444c0` | __cdecl(first, last, dest_end), EAX = dest begin | copy_backward with the inlined member assignment (dwords +0/+0Ch, name through 0041DD40/memcpy). |
| 00944540 | `spawn_member_push_heap_00944540` | __fastcall(ECX = first, EDX = hole; top, member by value {class, length, data, bag}, pred), RET 14h | _Push_heap: parent (hole-1)/2 (signed); moves while parent key (class+0B0h) > value key (FCOMIP JBE stops on NaN); value placed; by-value name to the pool. Unwind action not modelled. |
| 00945820 | `spawn_manager_construct_00945820` | __fastcall(ECX = manager), EAX | +4 = 00943B90(), +8 = 0, [00F89B3C] = manager, +0Ch = float bits D01502F9h (00CE4ADC). |
| 00945C30 | `spawn_member_construct_from_table_00945c30` | __thiscall(ECX = member; party, Lua table), RET 8 | +0..+8 zero, bag new(114h) + 008F41A0(0), 009420A0(party), 00944210(table). Unwind state 1 not modelled. |
| 00945CF0 | `record_fill_00945cf0` | __cdecl(first, last, const record*) | Fill by the inlined member/record assignment. |
| 00945DA0 | `record_construct_00945da0` | __fastcall(ECX = memory, EDX = const record*), RET | _Construct through 00660540 when memory is not null. |
| 00945E00 | `record_swap_00945e00` | __fastcall(ECX = a, EDX = b), RET | tmp = 00660540(a); a = b; b = tmp; tmp's name to the string pool. Unwind action not modelled. |
| 00945EE0 | `spawn_member_adjust_heap_00945ee0` | __fastcall(ECX = first, EDX = hole; bottom, record by value, pred), RET 18h | _Adjust_heap: right child unless its key > the left key (FCOMIP JBE keeps right on NaN); last-child case at bottom; value copied into 00944540(first, hole, original hole); own by-value name to the pool. |
| 009460B0 | `record_rotate_009460b0` | __fastcall(ECX = first, EDX = middle; last, 2 tags), RET 0Ch | MSVC9 random-access _Rotate (gcd cycles), hole value held in a local copy released to the pool; EAX (leftover) not modelled; unwind not modelled. |
| 009462B0 | `spawn_member_pop_heap_009462b0` | __fastcall(ECX = first, EDX = last; dest, record by value, pred, tag), RET 1Ch | *dest = *first; 00945EE0(first, 0, last - first, 00660540 copy of the value); by-value name to the pool. |
| 00946450 | `record_destroy_00946450` | __thiscall(ECX = allocator (unused); record), RET 4 | Name to the string pool when set. |
| 00946480 | `record_uninit_copy_00946480` | __fastcall(ECX = first, EDX = last; dest, allocator, 2 tags), RET 10h, EAX | _Uninit_copy; catch destroys through 00946450 and rethrows. |
| 00946570 | `spawn_member_make_heap_00946570` | __fastcall(ECX = first, EDX = last; pred, 2 tags), RET 0Ch | hole = count/2 down to 1: copy element hole-1 and 00945EE0(first, hole-1, count, copy). |
| 00946610 | `spawn_member_median3_00946610` | __fastcall(ECX = first, EDX = mid; last, pred), RET 8 | _Med3: swap(mid, first) when key(mid) > key(first); swap(last, mid) when key(last) > key(mid); swap(mid, first) again (FCOMIP JBE). |
| 009466A0 | `record_uninit_copy_009466a0` | __fastcall(ECX = first, EDX = last; dest, allocator, 2 tags), RET 10h, EAX | Same body as 00946480 (call list compared). |
| 009467B0 | `members_resource_usage_009467b0` | __fastcall(ECX = member vector), ST0 | Checked-iterator walk (00BF6713 reports in the listing's places); sum of ResourceUsage (00CFACAC; float when type 1 else CVTSI2SS of the int; 0 when absent via 0048E9F0). |
| 00946870 | `members_resource_usage_for_player_00946870` | __thiscall(ECX = member vector; player), RET 4, ST0 | As 009467B0 for members whose OwnerPlayer (00CF882C, -1 when absent) equals player. |
| 009469F0 | `queued_resource_usage_for_team_009469f0` | __stdcall(team), RET 4, ST0 | Walks [00F89B3C]'s list; 009467B0(ECX = request) summed for requests with +78h == team; checked-iterator reports kept (manager re-read each pass). |
| 00946C80 | `record_uninit_fill_n_00946c80` | __fastcall(ECX = first, EDX = count; const record*, allocator, 2 tags), RET 10h | _Uninit_fill_n; catch destroys through 00946450 and rethrows. |
| 00946DD0 | `spawn_member_median_00946dd0` | __fastcall(ECX = first, EDX = mid; last, pred), RET 8 | _Median: above 40 elements step = (count+1)/8 and four 00946610 calls (ninther), else one. |
| 00946EC0 | `spawn_member_pop_heap_00946ec0` | __fastcall(ECX = first, EDX = last; pred), RET 4 | When ((last-first) & ~0Fh) > 10h: 00660540 copy of last[-1], 009462B0(first, last-1, last-1, copy). |
| 00946F00 | `player_support_points_00946f00` | __fastcall(ECX = player), ST0 | 00942130(player) minus +304h (when > 0) of live units (+5Ch set, +5Dh/+60h/+5Eh clear; kind 0Fh only as flight leader via +9D4h) whose 009FFD20 slot is the player; minus 00946970([00F89B3C], player). |
| 00946FC0 | `team_available_resources_00946fc0` | __fastcall(ECX = team), ST0 | float[00E0CFB4]*0.5 minus +304h of the same live units with +54h == team (UCOMISS/LAHF/TEST 44h/JNP skips only exact 0), minus 009469F0(team). |
| 009473B0 | `spawn_member_unguarded_partition_009473b0` | __fastcall(ECX = pair out, EDX = first; last, pred), RET 8 | MSVC9 _Unguarded_partition written from the library source, checked against the decompiler and the nine 00945E00 call sites (argument order); pred = key(a) > key(b); 00946DD0 for the median. |
| 00947680 | `spawn_member_insertion_sort_00947680` | __fastcall(ECX = first, EDX = last; pred), RET 4 | Insertion through 009460B0: key stored to float, FCOMI against first, unguarded walk back while greater; rotate only when the point differs. |
| 00947750 | `spawn_member_sort_heap_00947750` | __fastcall(ECX = first, EDX = last; pred), RET 4 | _Sort_heap: copy last[-1], 009462B0(first, last-1, last-1, copy) while more than one element. |
| 009480B0 | `spawn_member_sort_009480b0` | __fastcall(ECX = first, EDX = last; ideal, pred), RET 8 | MSVC9 _Sort (32-element threshold, ideal halved plus half, smaller half recursed, heap sort through 00946570/00947750 when ideal runs out, 00947680 below). |
| 009481A0 | `spawn_request_read_exclude_radius_009481a0` | __fastcall(ECX = float block[7]; LuaObject by value), RET 14h | Unless 00B65FB0 nil: copy (00B66FA0) into 004425C0's reader, five 00BD68D0 reads ({0, key}, {2, &block[i]}, {2, block[i]}) for ownHorizontal..formationHorizontal, +14h/+18h squares, 00441A20; then 00B67700 on the argument. |
| 009488F0 | `insert_n_record_009488f0` | __thiscall(ECX = vector; where {container, ptr}, count, const record*), RET 10h | Same _Insert_n body as 009322F0 (call list compared) with 00660540, 00946A80, 009416B0, 009466A0, 00947900, 00946D70, 009479D0, 00948080, 00945CF0, 00946C00; _Tmp's name to the pool. |
| 00948BB0 | `spawn_request_destroy_00948bb0` | __fastcall(ECX = request), RET | operator delete of +0D0h (zeroed), +0B8h name to pool, +90h (zeroed), +84h name, 00695870(ECX = +10h), 00946D70 over the members and delete of +4 (zeroed). |
| 009496A0 | `spawn_manager_destroy_009496a0` | __fastcall(ECX = manager), RET | [00F89B3C] = 0; per node 009442A0(request) then 00948BB0 + operator delete when set; checked-iterator report kept; list nodes and head freed, +4 = 0. |
| 0094C830 | `spawn_manager_send_spawn_request_0094c830` | __thiscall(ECX = manager; a0..a7), RET 20h | 00947BC0(&msg; a0, a1, a3, a2, a4, a5, a6); game+1FE4h 0 or 1: 0094B600(&msg, a7), else 00770AF0(ECX = game+1EF0h; &msg); 00766DA0(&msg). |
| 0094C900 | `spawn_manager_send_single_0094c900` | __thiscall(ECX = manager; a0..a7), RET 20h | One-element vectors for a1 (00947FD0), a4 and a5 (00442190); 0094C830(a0, &v1, a2, a3, &v4, &v5, a6, a7); vectors freed in the order v5, v4, v1. |
| 0094CA10 | `ordered_map_delete_subtree_0094ca10` | __thiscall(ECX = tree; node), RET 4 | Post-order: left (+4), right (+8), then the node's vtable slot 0 with 1. |
| 0094CA40 | `ordered_map_destruct_0094ca40` | __fastcall(ECX = tree), RET | vtable 00D2505C; root subtree deleted inline; vtable 00D24F84 (ZMemoryPooledObject). |
| 0094EEF0 | `append_entry_texts_0094eef0` | __fastcall(ECX = list, EDX = vector<string>*), RET | Per 8-byte entry: text (+4, or "" at 00F89B74) into a temporary std::string (00408720), 00849170 push_back, heap buffer freed when capacity >= 10h. |
| 0094F830 | `effect_entity_destruct_0094f830` | __fastcall(ECX = entity), RET | Vtables 00CE8BD0/+10h 00CE8BB4/+24h 00CE8BAC/+170h 00CE8BA8; live effect +1E8h: 00867B10 and byte +9 = 1 when set; +1E8h and +1E4h released (00CE2220, slot 0); 009287B0. Unwind states not modelled. |
| 0094F910 | `effect_entity_activate_0094f910` | __fastcall(ECX = entity), RET | 009277E0; when (+0C0h)+4 == 1: Find(bag, "Effect" 00D09B84), 00870CD0(&tmp, +0Ch, 0), counted assignment into +1E4h (00CE221C / 00CE2220), tmp released. |
| 0094FB50 | `periodic_effect_update_0094fb50` | __thiscall(ECX = updater (entity+1E4h); float seconds), RET 4 | +38h -= seconds (stored); returns while 0 < it (JC, NaN too); +38h = +34h; with a definition (+3Ch): pose refresh 00414DB0 when entity+0C8h is clear, 00868420(&tmp, game+19ECh, definition with an added reference, entity+0CCh, 0, 0), 004845D0(+40h, &tmp), tmp released, live effect byte +9 = 1. |
| 0094FC60 | `periodic_effect_entity_activate_0094fc60` | __fastcall(ECX = entity), RET | 009277E0; type 1: Period (00D19B68, default 1.0 00D7A24C) -> +218h, Delay (00D0D404, default 0) -> +21Ch (float when property type 1, else CVTSI2SS), Effect handle -> +220h as in 0094F910. |
| 009510F0 | `traffic_record_construct_009510f0` | __thiscall(ECX = object; unused, value), RET 8, EAX | 0049F9B0(ECX = object) then +0 = value, +4/+8 = -1, +1Ch 10.0, +18h 40.0, +10h 0, byte +0Ch 0, +14h 1, +2Ch/+30h/+28h 0.0, +24h 5.0. |
| 00951450 | `traffic_record_vector_destroy_00951450` | __fastcall(ECX = vector), RET | 00951160(ECX = vector), operator delete of +4 when set, +4..+0Ch zeroed. |
| 00951820 | `entity_id_table_load_used_00951820` | __thiscall(ECX = table; archive), RET 4 | Archive vtable +4 open {0, "used"}, +14h has {1, i}, +10h read(1, i, {1, &slot}) into the argument slot (starts as the archive pointer); per id: 00951720 when +50h is 0, entry (id 0: +10h, else +4Ch + (id - +4)*10h) tagged DEEEDEEEh to the +3Ch list, +50h-1; computed entry tagged 0 to the +1Ch list, +50h+1; +8 close. |
| 00951940 | `entity_id_table_save_used_00951940` | __thiscall(ECX = table; archive), RET 4 | Skips the leading consecutive-id run from +10h exactly as 00951949..0095199B; writes (vtable +0Ch) {1, id} for each entry with a next; then units of game+19CCh with +5Eh set and vtable +5Ch(1) true whose word +174h lies in [+4, +4 + +8). |

## Segment 1 reconstructed (13)

| Address | C++ | ABI | Notes |
|---|---|---|---|
| 0041E490 | `counted_pointer_array_reserve_0041e490` | __thiscall(ECX = array {data, count, capacity}; capacity), RET 4 | At least 1; only a larger capacity reallocates: operator new 00BF55BE(capacity*4), each old slot copied with InterlockedIncrement (00CE221C) into non-null new slots (the running slot pointer is tested), old slots released (InterlockedDecrement 00CE2220, slot 0 at zero), _free (00BF6989) of the old storage, capacity stored. |
| 0041E780 | `native_string_at4_destroy_0041e780` | __fastcall(ECX = owner), RET | String {+4 length, +8 data}: 00419CC0 + 00BD1510(data, length + 1, 1) when data is set. |
| 0041E7A0 | `counted_record_destroy_0041e7a0` | __fastcall(ECX = record), RET | Counted pointers +2Ch, +28h, +24h released (decrement, slot 0 at zero, cleared), then the string {+0 length, +4 data} to the pool. |
| 0041F370 | `counted_pointer_array_resize_0041f370` | __thiscall(ECX = array; count), RET 4 | 0041E490 when capacity < count; new slots nulled; trailing slots released from the back; count stored. |
| 004204A0 | `native_string_at4_destroy_004204a0` | __thiscall(ECX = allocator (unused); owner), RET 4 | Same body as 0041E780 on the argument. |
| 00420660 | `counted_pointer_array_destroy_at30_00420660` | __fastcall(ECX = owner), RET | 0041F370(ECX = owner+30h, 0) then _free of the array storage. |
| 00424C40 | `game_settings_get_singleton_00424c40` | __cdecl(), EAX | Double-checked lazy singleton at 00F8753C under the lifetime manager's critical section (00415350 +10h; EnterCriticalSection 00CE2218, depth +18h, LeaveCriticalSection 00CE2210): operator new(76Ch), 00424A10, 00BD0C30 registration on a fresh 00415350 result. The stack lock guard (vtable 00CE37FC) unwind is not modelled. |
| 00427AC0 | `native_string_vector_size_00427ac0` | __fastcall(ECX = vector), EAX | 0 when first is null, else (last - first) SAR 3. |
| 00427C90 | `vector3_subtract_to_00427c90` | __thiscall(ECX = a; float out[3], const float b[3]), RET 8 | out = a - b component-wise (FLD/FSUB/FSTP). |
| 00427E30 | `vector3_length_squared_00427e30` | __fastcall(ECX = v), ST0 | Association from the bytes: (x*x + y*y) + z*z, stored to float (the decompiler prints z*z + x*x + y*y). |
| 00427EB0 | `entity_world_position_refreshed_00427eb0` | __fastcall(ECX = entity), EAX | 00414DB0 when byte +0C8h is clear; returns entity+0FCh. |
| 00428090 | `segment_intersects_sphere_00428090` | __fastcall(ECX = centre, EDX = start; radius, end), RET 8, AL | Traced from the bytes: d = end - start, f = start - centre; a = (dxx+dyy)+dzz; b = 2*((dy*fy + dx*fx) + dz*fz); c = ((fxx+fyy)+fzz) - r*r; disc = b*b - (a*4.0)*c; false when 0 > disc; t1 = (-b - sqrt)/(a+a): false when > 1, true unless < 0; t2 = (sqrt - b)/(a+a): true unless < 0. NaN takes the JBE side. |
| 0042A7E0 | `plane_is_on_ground_0042a7e0` | __fastcall(ECX = plane), EAX | 1 when +900h is 4 or 5, else 0. |

## Existing implementations recorded (4)

| Address | Existing C++ | Status | Evidence |
|---|---|---|---|
| 00944680 | `lua_binding_spawn` (src/lua_binding_spawn.cpp) | existing_partial_model_not_reverified_left61 | The Lua Spawn binding is modelled in lua_binding_spawn.hpp:288 (coverage stated there as partial: placement helpers 00941FA0, 009426D0, 004BC120, 00942CB0 not read). cc13-left61 did not re-verify the body. |
| 00946380 | `lua_binding_spawn_new_id_is_requested` (src/lua_binding_spawn.cpp) | existing_model_thunk_checked_left61 | Thunk checked in the listing: PUSH ECX (Lua state); ECX = [00F89B3C]; CALL 00945850; the C++ models 00946380 -> 00945850 together (00945850 has its own verified record). |
| 00946390 | `lua_binding_spawn_new_id_remove` (src/lua_binding_spawn.cpp) | existing_model_thunk_checked_left61 | Thunk checked in the listing: PUSH ECX; ECX = [00F89B3C]; CALL 00945A20; the C++ models the pair together. |
| 0094C480 | `lua_binding_spawn_new` (src/lua_binding_spawn.cpp) | existing_model_thunk_checked_left61 | Thunk checked in the listing: PUSH ECX; ECX = [00F89B3C]; CALL 00949750 (the SpawnNew body); body not re-verified by cc13-left61. |

## Skipped, segment 61 (17)

cc9 lane (ship motion, gunnery and damage; cc9 hosts already stand in for them):

- 0092E5B0 BSP_UnitController_ApplyNetworkCorrection: the ship's network dead-reckoning
  correction (cc9_ship_post_motion_read; game_hosts_units.cpp hosts it).
- 0092F930: the unit controller pose update (unit_rudder.cpp host hook controller_update_0092f930).
- 00933BB0, 00933DA0: the ship-part effect spawns of BSP_UnitInstance_UpdateShipMotion and the
  submarine update; they draw on the shared RNG stream cc9 manages.
- 00935540 BSP_UnitInstance_SpawnSinkingWreckDebris (cc9-gunnery23) and 00935D30 (the breakup
  pieces of the death sink, unit_death_sink.hpp).
- 0093A2C0 (the failure section message 9Bh), 0093AA00 (the failure-row lookup), 0093B920 (the
  repair task destructor) and 0093BD80 BSP_LuaBinding_AddEngineJamFailure: the ShipSystems
  failure model (unit_fire_flooding.hpp, game_hosts_gunnery.cpp).

Not attempted in this packet (large bodies, context budget):

- 0093D2D0 BSP_MapScene_SetupOceanMaterials (773 listing lines) and 0093FF10
  BSP_SoundOverlay_Draw (924 lines, unreferenced debug overlay).
- 00945450 (the SpawnLight Lua binding), 00947A30 (the spawn request completion: script callback
  or AI group), 00947BC0 (session message 42h constructor), 0094B600 (its handler), 0094BFF0.

## Skipped, segment 1 (55)

Unreferenced: no rel32 CALL/JMP and no absolute dword anywhere in the image
(`python tools/callsite_census.py <addresses>` reports 0 for each). The template ones are dead
COMDAT instantiations; the game cannot run them:

- TMPL_ helpers 00413D10, 00413E70, 00413F20, 00414010, 00414070, 00418C50, 00418CD0, 00418D60,
  00418DE0, 00418EA0, 00418F30, 00418FB0, 004199D0.
- 004171F0, 004174A0, 0041A030, 0041A1E0, 0041A4E0, 0041B4B0, 0041B580, 0041B7E0, 0041BD10,
  0041C490, 0041CB50, 0041F820, 00423640.

cc9 lane, the avoid-zone navigation system the ship AI lanes bind (game_avoid_zone_runtime,
avoid_zone_draft_layers, game_hosts_avoid_zones, ship_ai_*): 00416270, 004166E0, 00416840,
004169A0, 004178B0, 00417990, 00417E60, 0041AC60, 0041B120, 0041B290, 0041B4E0, 0041B510,
0041B680, 0041B700, 0041C0F0, 0041D0C0, 0041D150, 0041DE70, 0041E000, 0041EA30, 0041F0D0
(the registry singleton 00E17620 destructor), 0041F980 (the manager destructor, 00E17624),
00421500, 00421CE0, 00423520, 004235A0, 00424730 (the registry constructor). This is a
judgement call: the avoid zones are mission geometry, but every live caller is ship AI, spawn
placement or the registry itself.

cc9 lane, gunnery: 00428420 (the projectile segment trace's landing-troop hit test, caller
BSP_Projectile_TraceSegmentAndImpact) and 00428800 BSP_Explosion_NotifyBlastSites
(blast_damage.hpp hosts it).
