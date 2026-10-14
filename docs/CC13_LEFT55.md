# CC13 left55: leftovers of segments 55 and 56 (packet cc13-left55)

The earlier waves left 208 functions of segment 55 (00878530..0088B3D0) and segment 56
(0088B410..008DCD40) without a reconstruction record. This packet covers all 208:
207 are reconstructed in new files, and one (CountdownCancel 008B19A0) was already implemented
in the Lua host. That one gets a ledger-only record, and its gaps are listed below.

Status: reconstructed and build-tested only. Nothing here is fixture-tested or game-validated.
These are new C++ interfaces, not ABI-compatible drop-ins: each comment and ledger record gives
the native ABI. Names are hypotheses, not recovered symbols.

| File | Contents |
|---|---|
| `include/bsp/cc13_left55.hpp`, `src/cc13_left55_seg55.cpp` | Segment 55: the Lua variant record (vtable 00D0E6F4) and its map payload, the map key, NativeString helpers, the VC9 `std::sort` internals over `{dword, float}` pairs, two landscape byte-grid passes, message constructors. |
| `src/cc13_left55_seg56.cpp` (same header) | Segment 56 helpers (0088B410..0088BBC0, 008D3E00) and the first ten bindings (0088BC00..0088DAA0). |
| `include/bsp/cc13_left55_seg56a.hpp`, `src/cc13_left55_seg56a.cpp` | Bindings 00892F70..008A8280. |
| `include/bsp/cc13_left55_seg56b.hpp`, `src/cc13_left55_seg56b.cpp` | Bindings 008A8480..008BCF10 and IsUnitUntouchable 008AC420. |
| `include/bsp/cc13_left55_seg56c.hpp`, `src/cc13_left55_seg56c.cpp` | Bindings 008BDEC0..008CF7E0. |

## Conventions

- **Segment 55 code** works on image memory directly. It uses raw layouts (`NativeStr`,
  `LuaKey`, `LuaVariant`, `VariantMapNode`, `VariantMap`), whose offsets are pinned by
  static_asserts under `_M_IX86`. Every native call goes through `Left55Host`, one method per
  callee, with the address in the method name. The inlined NativeString copy, append and release
  are local helpers that make the same 0041DD40, memcpy and 00419CC0+00BD1510 calls.
- **Checked-iterator asserts.** The `_invalid_parameter_noinfo` calls (00BF6713) of the checked
  iterators are kept at the same points.
- **Segment 56 bindings** follow the earlier packet's shape (docs/CC13_SEG56.md, "Shared
  shape"):
  - The dead `luaMW_<name> failed:` / `luakod` prologue, the state wrap (00B66C00) and the call
    frame (00B679B0) are not reconstructed.
  - The return value is what EAX holds at the RET.
  - Argument readers and pushes are `cc13_seg56::LuaArgs` / `LuaResults`. Field accesses go
    through `ImageAccess` (part files) or raw offsets (this file).
  - Every other callee is a named host method.

## Segment 55

| Address | C++ | ABI | Behaviour and evidence |
|---|---|---|---|
| 0087BB02 | `damageable_section_vector_growth_catch_0087bb02` | catch(...) funclet, frame EBP of the parent | MOV EAX,[EBP+10h]; MOV ESI,[EBP+14h]; MOV ECX,[EBP-14h]; 00878AC0(ECX alloc, ESI first, EAX last) at 0087BB0D; _free(ESI) 0087BB13; _CxxThrowException(0,0) 0087BB1F. Uncertainty: Frame values are passed as parameters; the parent vector reallocation is not this packet's. |
| 0087C518 | `damageable_fake_effect_vector_growth_catch_0087c518` | catch(...) funclet, frame EBP of the parent | Same shape as 0087BB02 with 0087B5B0 (destroy fake-effect rows), _free, rethrow. Uncertainty: Frame values are passed as parameters. |
| 00880080 | `byte_grid_propagate_falloff_00880080` | __stdcall(columns, width, height, bool single_step), RET 10h | Dirty-rectangle relaxation: cell = max(neighbour) - 1 when cell + 1 < max (MOVSX compares 00880190..0088019E); directions step 1 (single_step) or alternate 1/2 (3 - step, 00880129); 00412070(columns, width, height, x, y, dir, 0) at 00880176; window min-1 / max+2 clamped (008800B4..00880103); loop until no change (008801F3). Uncertainty: Ghidra decompiler misplaced the 00412070 register arguments; the listing was used. |
| 00880220 | `forward_handle_00880220` | __thiscall(ECX owner, argument), RET 4 | handle = [owner+42Ch]; null skips; else [[[00E188A8]+19F0h]+A8h] vtable+20h(handle, argument) at 00880248. Uncertainty: Callee meaning not established; named by slot. |
| 00880250 | `transform_unit_length_00880250` | __fastcall(ECX owner), RET, float in ST0 | [owner+3D0h] vtable+1Ch(out, x, y) for (0,0) at 00880272 and (1,0) at 00880292; squared = dx*dx + dy*dy stored float; > double 1e-10 (00CE3820) -> float(sqrt) 00BF7030, else 0. Uncertainty: x87 extended intermediate replaced by float/double C++ arithmetic. |
| 00880420 | `landscape_build_shore_mask_00880420` | __thiscall(ECX owner, columns, step, params), RET 0Ch | Terrain sampled every `step` via [owner+3D0h] vtables +10h/+14h/+20h; level trunc(p3/p1 + 1.5) clamped 1..100; 00880080; ring trunc(p4/(p1*1.7999999523) + 1.5) (00D049A8, FDIVP DE F9); open-water flood forward/backward, ring growth, thinning with run analysis (directions 0..6 filled, 0088097E..008809BB); 0087FA90(columns, w, h, [params+8]) at 00880A4F. Uncertainty: Written from the full listing; the decompiler's output for this body is unreliable (register arguments, clobbered locals). |
| 008840B0 | `native_string_compare_008840b0` | __thiscall(ECX left NativeString, right), RET 4 | Empty left: -(right.size != 0) (NEG/SBB 008840BB); empty right: 1; else inline unsigned strcmp returning -1/0/1. |
| 00884460 | `lua_key_less_00884460` | __thiscall(ECX left key, right key), RET 4, AL | kind compare (JGE/JLE 0088446B/00884473); kind 0 int +0Ch SETL 008844A7; kind 1 008840B0(left+4, right+4) < 0 at 0088448C; other kinds false. |
| 008845A0 | `pending_results_get_field_008845a0` | __thiscall(ECX pending, const char* name), RET 4 | byte +0Ch armed test; machine = [[[00E188A8]+1A08h]+4]; 006B8120(machine, name) 008845BF; [pending+8] -= 1; 006B8470(machine, [pending+8]) 008845CE. |
| 00884660 | `variant_map_lower_bound_00884660` | __thiscall(ECX map, const LuaKey*), RET 4 | Root [head+4]; isnil byte +31h; node.key < key -> right (+8), else result=node, left (+0); key compare inlined as 00884460 with 008840B0 at 00884692. |
| 008849A0 | `clear_four_dwords_008849a0` | __fastcall(ECX), RET | Stores 0 at +4, +8, +0Ch, then +0. Uncertainty: Owner type not identified. |
| 008849D0 | `variant_construct_number_008849d0` | __thiscall(ECX, float), RET 4 | vtable 00D0E6F4, tag 0, +0Ch/+10h = 0, +8 = value. |
| 00884A00 | `variant_construct_int_00884a00` | __thiscall(ECX, int), RET 4 | As 008849D0 with +8 = (float)int (FILD/FSTP). |
| 00884A30 | `variant_construct_bool_00884a30` | __thiscall(ECX, bool), RET 4 | tag 2, byte +8 = value (only the byte is stored). |
| 00884A90 | `variant_construct_entity_00884a90` | __thiscall(ECX, entity), RET 4 | tag 3, word +8 = entity ? [entity+174h] : 0. |
| 00884AC0 | `variant_construct_entity_record_00884ac0` | __thiscall(ECX, record), RET 4 | tag 3, word +8 = [record+18h] (no null test). |
| 00884AF0 | `variant_destruct_00884af0` | __fastcall(ECX), RET | vtable 00D0E6F4 restored, 00886920(this), name NativeString +0Ch/+10h released (00419CC0+00BD1510). |
| 00884B60 | `variant_map_collect_payloads_00884b60` | __thiscall(ECX holder, out), RET 4 | Checked iteration of the map [holder+8] (00BF6713 asserts kept, re-reading [holder+8]); *out++ = [node+24h] (value payload); 006EDA20 increment. |
| 00884CB0 | `list_construct_00884cb0` | __fastcall(ECX), RET, returns this | [+4] = 00884830(); [+8] = 0. |
| 00884D90 | `lua_key_construct_text_00884d90` | __thiscall(ECX key, const char*), RET 4 | text cleared, temporary NativeString assigned (0041E870), copied (resize+memcpy), released; kind = 1. |
| 00884E50 | `lua_key_construct_moved_00884e50` | __thiscall(ECX key, NativeString by value), RET 8 | text cleared, copy of the argument, kind = 1, argument storage released. |
| 00884EF0 | `lua_key_copy_00884ef0` | __thiscall(ECX key, source), RET 4 | text cleared; kind copied; kind 0 copies +0Ch; kind 1 copies the NativeString. |
| 00884FC0 | `variant_construct_string_00884fc0` | __thiscall(ECX, const NativeString*), RET 4 | tag 1; operator new(8); null -> payload 0; else {0,0}, copy, payload = it. |
| 00885070 | `variant_map_find_string_00885070` | ECX map, EDX name, RET 4 (stack dword unread) | Key built by 00884D90, 00884D20(map, &it, &key), key released, checked-iterator asserts; end -> null; else [[node+24h]+4] or 00F87904 ("") when null. Uncertainty: The RET 4 stack argument is never read. |
| 00885400 | `lua_key_to_text_00885400` | __thiscall(ECX key, out), RET 4 | kind 1: out cleared and copied from key text; otherwise 004260B0(out, key+0Ch) at 0088545A. |
| 00885470 | `variant_to_short_text_00885470` | __thiscall(ECX variant, out), RET 4 | tag 0: 0043BDF0(out, float); tag 1: quote = "\"" (0041E870), 0043C130(&head, "\"", payload) then 004261A0(head, out, &quote) at 00885524, both released; tag 5: "{Map}"; else "". |
| 008855B0 | `variant_append_text_008855b0` | __thiscall(ECX variant, NativeString* text), RET 4 | tag 0 00781740; tag 1 quote + text + quote (inline appends); tag 5 "{" via 00425E10, per node key text (00885400) "=" value (recursion) ";", then "}"; tag 6 "<NIL>" via 00425E10; tag 4 " <C Ptr>" via 00425E10 then 0041DD20; tags 2/3 nothing. |
| 00885A70 | `variant_store_in_table_00885a70` | __thiscall(ECX variant, key, instance), RET 8 | tag 0/1 with key kind 0/1: 006B82A0 / 006B8260 / 006B8320 / 006B8360 (key text from 00781540, null data -> 00F87904); tag 5: 00803750 or 008037D0 scope, recursion into every node (ECX node+1Ch, key node+0Ch), then lua_settop([scope.instance+4], -2) via EDX at 00885B59 / 00885C3D. |
| 00886840 | `variant_construct_map_00886840` | __thiscall(ECX, const map*), RET 4 | tag 5; operator new(0Ch); head 006EDEA0(map) with isnil, self links, size 0; when map != source: 006EE960 erase(begin, end) then 00781890 copy (008868D8 / 008868E0). Uncertainty: Null allocation is not guarded before the erase, as in the native. |
| 00886920 | `variant_clear_00886920` | __fastcall(ECX), RET | tag 1: string storage released and the 8-byte block freed; tag 5: erase, free head, clear, free map; tag = -1. |
| 008869D0 | `variant_set_number_008869d0` | __thiscall(ECX, float), RET 4 | 00886920 then tag 0, +8 = value. |
| 008869F0 | `variant_set_int_008869f0` | __thiscall(ECX, int), RET 4 | 00886920 then tag 0, +8 = (float)int. |
| 00886A10 | `variant_set_string_00886a10` | __thiscall(ECX, const NativeString*), RET 4 | 00886920, tag 1, new(8) copy or payload 0. |
| 00886AC0 | `variant_set_bool_00886ac0` | __thiscall(ECX, bool), RET 4 | 00886920, tag 2, byte +8. |
| 00886AE0 | `variant_set_entity_00886ae0` | __thiscall(ECX, entity), RET 4 | 00886920, tag 3, word = entity ? [entity+174h] : 0. |
| 00886B10 | `variant_set_id_00886b10` | __thiscall(ECX, u16), RET 4 | 00886920, tag 3, word +8. |
| 00886B30 | `variant_set_pointer_00886b30` | __thiscall(ECX, pointer), RET 4 | 00886920, tag 4, +8. |
| 00886B50 | `variant_set_map_00886b50` | __thiscall(ECX, const map*), RET 4 | 00886920, tag 5, then as 00886840. |
| 00886C10 | `variant_assign_00886c10` | __thiscall(ECX, source), RET 4 | Self test; 00886920; tag copied; 0/4 dword, 1 new(8) + 00426060(new, payload) at 00886CA7, 2 byte, 3 word, 5 new(0Ch) + 006EEAF0 + 00781F20, 6 0074FB30(this) at 00886C52. |
| 00886DA0 | `variant_copy_construct_00886da0` | __thiscall(ECX, source), RET 4 | vtable 00D0E6F4, tag -1, +0Ch/+10h cleared, then 00886C10. |
| 00887120 | `result_sink_collect_00887120` | __thiscall(ECX sink, count, options), RET 8 | last = 006B7E70([sink+4]) unless count >= 0; for i = 1..last: temp variant, 00886E00(sink, &v, i, 0, options) 0088719E, 006EDF00(sink+8, &v) 008871AA, destroyed. |
| 00887370 | `result_sink_collect_range_00887370` | __thiscall(ECX sink, first, last, options), RET 0Ch | An unused variant is built at entry and destroyed at exit; first < 1 -> 1, last < 1 -> 006B7E70; per index as 00887120. |
| 00887A10 | `deferred_call_copy_00887a10` | __thiscall(ECX, source), RET 4 | Two NativeStrings copied, vector {0,0,0} then 00887560(dst+10h, src+10h) at 00887AB7, +1Ch/+20h copied. |
| 00887E30 | `call_queue_destroy_00887e30` | __fastcall(ECX), RET | 00887D90(queue); _free([queue+4]); [queue+4] = 0. |
| 008882B0 | `call_queue_destroy_008882b0` | __fastcall(ECX), RET | Identical body to 00887E30. |
| 00888960 | `lua_object_ptr_field_00888960` | __thiscall(ECX LuaObject), RET | 00B67800(this, &tmp, "Ptr" 00CFAD08), 00B662D0(tmp), 00B67700(tmp). |
| 00888BE0 | `lua_object_ptr_field_00888be0` | __thiscall(ECX LuaObject), RET | Identical body to 00888960. |
| 00888C40 | `lua_object_is_kind2_entity_00888c40` | __thiscall(ECX LuaObject), RET, AL | 00B661B0 table test; "Ptr" field 00B66160 light-userdata test; second "Ptr" read 00B662D0; non-null and vtable+5Ch(2). |
| 00888F30 | `position_message_construct_00888f30` | __thiscall(ECX msg, const float* pos, float, int), RET 0Ch | 0075B430(97h); +4 = 1, bytes +20h/+21h = 1, +28h = int, vtable 00D02F24, float +24h, word +18h, bytes +1Ah/+1Ch = 0, +2Ch..+34h = *pos. |
| 00888FC0 | `scene_node_set_alpha_recursive_00888fc0` | __thiscall(ECX node, float alpha), RET 4 | vtable+0Ch(00BBE290()) -> 00BBE490(node, alpha); children [node+34h] linked by +3Ch, recursion 00888FFA. |
| 008890B0 | `singleton_destruct_008890b0` | __fastcall(ECX), RET | [00F87908] = 0; vtable = 00CE3818. |
| 00889610 | `owned_block_release_00889610` | __fastcall(ECX slot), RET | Non-null: _free, slot = 0. |
| 008898C0 | `scored_push_heap_008898c0` | __fastcall(ECX first, EDX hole, stack top, key, score), RET 0Ch | VC9 _Push_heap with operator< on the float at +4 (FCOMI 008898E0). |
| 00889ED0 | `list_node_allocate_00889ed0` | __stdcall(value), RET 4 | new(0Ch) -> {0, 0, value}; failure path still stores value at [0+8] (00889EF5). Uncertainty: The null store is reproduced as a write to address 8. |
| 00889F10 | `object_if_kind_2b_00889f10` | __fastcall(ECX), RET | Non-null and vtable+5Ch(2Bh) -> object, else null. |
| 0088A0C0 | `scored_med3_0088a0c0` | __fastcall(ECX a, EDX b, stack c), RET 4 | VC9 _Med3: three conditional swaps by score. |
| 0088A130 | `scored_adjust_heap_0088a130` | __fastcall(ECX first, EDX hole, stack bottom, key, score), RET 0Ch | VC9 _Adjust_heap; the even-length tail sets hole = bottom-1 (LEA EDX,[EDI-1] 0088A17B), then 008898C0(first, hole, top). Uncertainty: The decompiler lost the hole update; the listing was used. |
| 0088A1E0 | `scored_uninitialized_copy_0088a1e0` | __fastcall(ECX first, EDX last, stack dest, 3 unused), RET 10h | Copies pairs to non-null destinations, returns the end. |
| 0088A2A0 | `refcounted_slot_assign_0088a2a0` | __thiscall(ECX slot, value), RET 4 | Old: InterlockedDecrement(+4) via [00CE2220], zero -> vtable+0 (ECX old); slot = 0, then value; no AddRef of the new value. |
| 0088A570 | `scored_fill_0088a570` | __cdecl(first, last, const value*) | std::fill over pairs. |
| 0088A5C0 | `scored_uninitialized_fill_n_0088a5c0` | __fastcall(ECX dest, EDX count, stack value*, 3 unused), RET 10h | Fill n pairs at non-null destinations. |
| 0088A650 | `scored_median_0088a650` | __fastcall(ECX first, EDX mid, stack last), RET 4 | VC9 _Median: n > 40 -> step (n+1)/8 and 0088A0C0 at 0088A68A, 0088A699, 0088A6A9 and 0088A6B5, else one 0088A0C0 at 0088A6C3. |
| 0088A6D0 | `scored_make_heap_0088a6d0` | __fastcall(ECX first, EDX last, 2 unused), RET 8 | VC9 _Make_heap: hole from n/2 down, 0088A130(first, hole, n, first[hole]). |
| 0088A720 | `scored_insertion_sort_0088a720` | __fastcall(ECX first, EDX last), RET | VC9 _Insertion_sort by score. Uncertainty: NaN ordering follows the VC9 source predicate, not re-derived from the FCOMI parity handling. |
| 0088A900 | `lua_table_collect_ptr_fields_0088a900` | ECX PointerArray, EBP Lua table (register input), RET, AL=1 | Two LuaObjects, 00B67080/00B66420/00B67190 iteration; per value 00B67800 "Ptr", 00B662D0; grow by 00529760(cap*2, min 1) when full; null-checked slot store; count++. Uncertainty: EBP register input made an explicit parameter. |
| 0088AAF0 | `tree_construct_0088aaf0` | __fastcall(ECX tree), RET | [+4] = 0088A520(); isnil byte +19h = 1; self links; size 0. |
| 0088ABD0 | `scored_copy_backward_0088abd0` | __cdecl(first, last, dest_end) | Backward pair copy; returns dest_end - n. |
| 0088AC20 | `scored_unguarded_partition_0088ac20` | __fastcall(ECX out pair, EDX first, stack last) | VC9 _Unguarded_partition with operator< on score; the native's four-way unrolled scans visit the same order. |
| 0088B120 | `variant_map_walk_0088b120` | __fastcall(ECX map), RET | Checked iteration; 0088B190(node+1Ch) at 0088B162; 006EDA20; no other effect. |
| 0088B190 | `variant_walk_0088b190` | __fastcall(ECX variant), RET | tag 5 -> tail JMP 0088B120([variant+8]). |
| 0088B360 | `end_scene_choice_0088b360` | __fastcall(ECX choice), RET | 004D7970(game, 0); 3: 00437C70(00425C20()); 3 or 1: 004D7920(game, 0Bh), tail JMP 005FD440([[00E198C4]+D8h]). |

## Segment 56: helpers and the first ten bindings

| Address | C++ | ABI | Behaviour and evidence |
|---|---|---|---|
| 0088B410 | `named_pair_assign_0088b410` | __thiscall(ECX, source), RET 4 | +0/+4 copied, NativeString +8 copied (inline), byte +10h. |
| 0088B4B0 | `entity_start_anim_0088b4b0` | __thiscall(ECX entity, const NativeString*, float), RET 8 | byte +70Ch = 1; NativeString +714h copied; float +710h = [00F876A4] - offset (0088B4EE). |
| 0088B520 | `tree_construct_0088b520` | __fastcall(ECX tree), RET, returns this | Same body as 0088AAF0. |
| 0088B550 | `named_value_copy_0088b550` | __thiscall(ECX, source), RET 4 | NativeString copy then +8. |
| 0088B5A0 | `named_pair_copy_0088b5a0` | __thiscall(ECX, source), RET 4 | Copy constructor of the 0088B410 record. |
| 0088B650 | `scored_sort_heap_0088b650` | __fastcall(ECX first, EDX last), RET | VC9 _Sort_heap: 0088A130(first, 0, n-1, last-1 value) at 0088B684. |
| 0088B6D0 | `named_message_e2_construct_0088b6d0` | __thiscall(ECX msg, const NativeString*, a, b), RET 0Ch | 0075B430(E2h) 0088B6F4; +4 = 1; vtable 00D03644; NativeString +18h cleared and copied; +20h, +24h. |
| 0088B7B0 | `named_message_e7_construct_0088b7b0` | __thiscall(ECX msg, const NativeString*), RET 4 | 0075B430(E7h); +4 = 1; vtable 00D03658; NativeString +18h. |
| 0088B840 | `lua_object_is_vector3_table_0088b840` | __thiscall(ECX LuaObject), RET, AL | Table test; every key a string (00B660A0) and value a number (00B66050); key "x" (_stricmp 00CEB488, skipped when the copy is null), "y"/"z" (00425850 with 00D045F8/00CFD718); exactly three matches. Uncertainty: Repeated keys count, as in the native. |
| 0088BA00 | `lua_object_is_entity_or_vector3_0088ba00` | __thiscall(ECX LuaObject), RET | 008889C0 || 0088B840 on the same object. |
| 0088BBC0 | `variant_array_walk_0088bbc0` | __fastcall(ECX {data, count}), RET | Per 14h-byte variant tag 5: 0088B120([v+8]) at 0088BBE5. |
| 0088BC00 | `lua_log_to_file_0088bc00` | int __fastcall(lua_State*), RET | 006B7E30(holder, L) 0088BCBB, 008875F0(sink, holder), 00887120(sink, -1, 10), 0088BBC0(sink+8), 00887670(sink), 006B7E50(holder); returns literal 0. Uncertainty: Writes nothing: the arguments are collected and discarded. |
| 0088BD20 | `lua_do_file_outside_mpak_0088bd20` | int __fastcall(lua_State*), RET | [00E1AE84]++ ; string arg0 into a NativeString; 00B69D40(owner, &name, 0) at 0088BE42; released; [00E1AE84]--; returns 0. |
| 0088BEC0 | `lua_log_0088bec0` | int (dead prologue only), RET | Builds and frees "luaMW_Log failed:", returns 0; no frame, no argument read. |
| 0088BF20 | `lua_dump_0088bf20` | int (dead prologue only), RET | As Log with "luaMW_Dump failed:". |
| 0088C3E0 | `lua_assert_0088c3e0` | int __fastcall(lua_State*), RET | count==1 and arg0 boolean -> done; else count==2: arg0 boolean then arg1 string test; answers unused; returns 0. |
| 0088D130 | `lua_add_watch_0088d130` | int __fastcall(lua_State*), RET | string arg0 built into a by-value NativeString; 00922240(ECX [[00E188A8]+21CCh], name) at 0088D24F; returns 0. |
| 0088D420 | `lua_log_to_console_0088d420` | int __fastcall(lua_State*), RET | 00B663F0 count read and discarded; returns 0. |
| 0088D560 | `lua_remove_watch_0088d560` | int __fastcall(lua_State*), RET | As AddWatch with 009222E0 at 0088D67F. |
| 0088D6E0 | `lua_network_log_0088d6e0` | int __fastcall(lua_State*), RET | count==1: string arg0 copied into NativeString [00F871B4]+7649ACh; then 00789560(ECX [00F871B4]) at 0088D87B; PushBoolean(AL) 0088D88D; returns 1. |
| 0088DAA0 | `lua_get_gui_name_0088daa0` | int __fastcall(lua_State*), RET | entity arg0 vtable+14h() at 0088DBBD, 00B66710(text) at 0088DBC4; returns 1. |
| 008D3E00 | `multi_lobby_settings_destroy_008d3e00` | __fastcall(ECX), RET | vtable 00CEF164; nested maps +10h and +34h: every inner value pointer (+10h) freed (005672D0/00569B00 increments, checked asserts); maps +34h/+28h/+1Ch/+10h/+04h erased (00576120/00576740/005761F0/00576120/005724D0) and their heads freed; [00F8894C] = 0; vtable 00CE3818. |

## Segment 56 bindings 00892F70..008A8280 (`cc13_left55_seg56a`)

| Address | Lua name | C++ function | Behaviour and evidence (instruction addresses) |
|---|---|---|---|
| 00892F70 | HitTarget | lua_hit_target | arg0 entity, arg1 number truncated by __ftol (008930AA/008930AF), arg2 entity, arg3/arg4 vectors (00893114, 00893147), arg5 number as float (0089317B), arg6 boolean; 0072C170(ECX gun, EDX int, target, &v3, &v4, float, bool) at 008931DE. Pushes nothing. |
| 00893380 | AddWaterLoad | lua_add_water_load | arg0 unit, arg1 side: x sign odd 1/even -1 (008934D2..008934E6), z sign <2 1, <4 0, else -1 (008934E9..00893506); point = ([unit+538h]+A4h * xs * 0.5, 0, +A0h * zs * 0.5) (00893510..00893557, doubles 00D7A280=0.5). arg2 amount; if amount > 0 and !vcall5Ch(8): v = amount/10.0 (00CE3DC0), clamp to [0, 63] (00D099A0/00D099A8), FISTP-truncate (008935ED..00893603); non-zero count builds 0080FF80(count, &point) and routes 0077C2A0(unit, msg, 7, 0) (00893613..00893631). |
| 008945F0 | SetWeaponDirectorTargetEnable | lua_set_weapon_director_target_enable | arg0 through 00888BE0 ("Ptr" 00CFAD08 + 00B662D0, no id lookup), arg1 int column, row = -1 (0089473F) or arg2 when count == 4, then boolean (arg3 or arg2); 00861C20([owner+6DCh], column, row, flag) at 008947D0. |
| 00894C00 | IsAreaEmpty | lua_is_area_empty | arg1 radius first, then arg0 vector with y forced to 0 (00894D58); result starts true (00894D60). False when 0098B450(spatial singleton 0042E630, &min, &max) answers non-zero (00894DAE) or any of three 00903860 ground probes on [game+19CCh] (centre, centre+r, centre-r; 00894DD1, 00894E3F, 00894EA9) hits with height > -10.0f (00CE6848). Pushes one boolean. |
| 00895ED0 | SetAirBaseSlot | lua_set_air_base_slot | block = 006BCD20(arg0, DL=0); slot = arg1-1; count = arg3 (read before arg2); class = 007B8A80(arg2) only when count != 0 else null; equipment = arg4 if count() > 4 else -1 (008960C2); 006CD350(block, slot, class, count, equipment); if slot state ([block+4Ch]+slot*58h+2Ch) == 5, 006CCDA0(slot, 2, 0, 9, 1); 006C4780(slot, class, count); if count() > 4 re-read arg4 and 006BA4B0(slot record, v) (sets +10h); 006BD800(block, slot) message routed on [block+7Ch] with (0, 0) (0089619B..008961B8). |
| 008970B0 | RemoveWrecks | lua_remove_wrecks | walks [[game+19CCh]+43Ch] (next +4h, entity +8h), 00926D90(entity, 1) each; next read after the kill (008971B9). |
| 008973C0 | RemoveAirBasePlanes | lua_remove_air_base_planes | block = 006BCD20(arg0, 0); class = 00964790(arg1, DL=1) (00897506); 006C7B10(block, class, arg2) at 00897543. |
| 008990B0 | FindEntityByID | lua_find_entity_by_id | id = arg0 & FFFFh (008991C6); signed compare with [00F89A10]: table [00F89A54] from base [00F89A0C], else [00F89AA8] from [00F89A60]; entity = row(id-base)*10h + 0Ch. Null pushes nil (008992A5), else the thisTable tail on word +174h (0089920B..0089929A). Pushes one value. |
| 0089A660 | Order | lua_order | arg0 entity, command = 00467170(arg1 string); count == 2: zero target (kind 0, position [00F87574..7C], trailing 0; 0089A7C5..0089A80B), else 0088A810(arg2); 0077D600(entity, command, &target, 1) at 0089A850. |
| 0089B260 | SetCommandBuildingLevel | lua_set_command_building_level | arg0 entity, arg1 level; arg2 read only when count == 3 (else 0); message D4h, vtable 00CFADD0, +4h=1, +18h word 0, +1Ah 0, +1Ch level, +20h arg2; 0077C2A0(entity, msg, 7, 0) (0089B42E / 0089B477). Only the two-argument path restores 00CE4974 (0089B47C). |
| 0089B9D0 | ForceFireDepthCharge | lua_force_fire_depth_charge | children from +48h (next +44h); class 24h and [[child+3F4h]+80h] == 8 -> vtable +1F0h (0089BAF2..0089BB19). |
| 0089BB70 | KillTorpedoesFromUnit | lua_kill_torpedoes_from_unit | list head [[[game+19CCh]+8h]], next +44h read before the test; class 2Bh with +3BCh == unit -> 00926D90(node, 2) (0089BCA0..0089BCBD). |
| 0089BD20 | ForceEquippedOrdnanceType | lua_force_equipped_ordnance_type | arg1 int to +96Ch of each squadron member (+3D0h slots, null past slot 4, count +3CCh re-read; 0089BE68..0089BE95). |
| 0089BEF0 | ReplaceDeviceWithEntity | lua_replace_device_with_entity | device = first class-1Eh child of arg0 with +38Ch == arg1 (else null, 0089C03F..0089C066); replacement = arg2 "Ptr" via inline 00B67800 + 00B662D0; device world matrix cache inlined (00414DB0 on parent, 00413920 local+74h * parent+CCh or local, 004134F0 into +CCh, +C8h=1, +10Ch=0; 0089C0C4..0089C118); 00741E90(replacement, device+CCh); replacement vtable +40h(unit vtable +34h(&tmp)); (replacement+310h) vtable +0Ch; 00926D90(device, 2). |
| 0089C1B0 | ReplaceSceneBorder | lua_replace_scene_border | vector arg1 read first (0089C2D0), then arg0 (0089C2DC); 004CA760(ECX game, &arg0, &arg1) at 0089C2E8. |
| 0089CAA0 | DCEnable | lua_dc_enable | director = vcall +114h(arg0) before reading arg1 boolean; 0071E150(director, flag) only when director != 0 (0089CBF7). |
| 0089D250 | GetPlaneSquadron | lua_get_plane_squadron | [plane+9D4h]: nil when 0 (0089D36C), else thisTable tail on its +174h. |
| 0089D470 | GetSquadronLandedBase | lua_get_squadron_landed_base | [squadron+3B4h]: nil when 0 (0089D58C), else thisTable tail. |
| 0089E050 | PlaneSetGunfire | lua_plane_set_gunfire | null entity -> nothing (0089E166); class 18h -> [+3D0h]; else must be class 0Fh; non-null plane: byte +9B0h = flag ? 1 : -1 (SETNZ/LEA 0089E1B5..0089E1C7). Boolean read only on that path. |
| 0089E850 | SquadronForceRelease | lua_squadron_force_release | each member whose byte +9C0h + 8*word[00F876B8] is set -> 007BBBA0(member) (0089E977..0089E9A0). |
| 0089ED20 | IsInPosition | lua_is_in_position | false when [unit+284h] == 0 (0089EE36); else 0070EB00(group, float arg1) pushed (0089EE71). |
| 0089F360 | PlaneSetMovieCollision | lua_plane_set_movie_collision | boolean arg1; class 0Fh -> byte +9E2h; class 18h -> each member's +9E2h (0089F4A9..0089F4FD). |
| 0089FAF0 | SquadronSetBaseUnsupported | lua_squadron_set_base_unsupported | block = class 45h ? base+72Ch : base+1188h (inline, no null case; 0089FC3D..0089FC4E); 006CA730(block, squadron). |
| 008A0630 | SquadronSetTravelSpeed | lua_squadron_set_travel_speed | speed float arg1; flag = arg2 when count == 3 else false; +388h = 0.5f (00CE3800), byte +38Fh = flag, +3A0h = speed, byte +3ACh = 1 (008A07D6..008A0802). |
| 008A0F10 | EntityTurnToPosition | lua_entity_turn_to_position | delta = arg1 vector - world position (+FCh..104h after 00414DB0); y zeroed unless count == 3 and arg2 true; rows (1,0,0,0)(0,1,0,0)(delta,0)(0,0,0,1) through 0085DC80 (008A112E..008A11AC). Host session (game+1FE4h != 0) and class 2: unless 007788B0, heading = -007B4E90(delta), 007EDEB0 message routed (1, 0). Else class 18h: each member gets its own position in row 3 and vtable +88h; class 6: vtable +88h, 00C336C0 4x3 copy, 00C56CF0 on [0080E490(entity)+2Ch]; otherwise vtable +88h with own position. |
| 008A1B70 | SquadronSetWandererMul | lua_squadron_set_wanderer_mul | float arg1; class 0Fh -> +840h; anything else -> each member's +840h (008A1CB9..008A1D0F). |
| 008A1D60 | SquadronSetWandererEnabled | lua_squadron_set_wanderer_enabled | as above with byte +844h (008A1EA9..008A1EED). |
| 008A54C0 | PlaneBombFallingTime | lua_plane_bomb_falling_time | 00414DB0 when +C8h clear; 007BCC80(plane, float +100h) pushed as float (008A55E6..008A55FF). |
| 008A5870 | PilotStartAttackRun | lua_pilot_start_attack_run | message BCh, vtable 00CF3BB8, +4h=1, +18h word 0, +1Ah 0, +1Ch 0, +20h 1; 0077C2A0(entity, msg, 0, 0) (008A5987..008A59C5). |
| 008A5A20 | SetAirbaseSmallPlanes | lua_set_airbase_small_planes | boolean read before 006BCD20(arg0, 0); non-null block: +20h = flag ? 1 : 2 (008A5B75..008A5B7C). |
| 008A5D60 | GetSquadFlyTime | lua_get_squad_fly_time | arg0 through 00888D20 ("Ptr"); 007EE150(ptr if non-null and class 18h, else null) pushed as float (008A5E76..008A5E9C). |
| 008A5EF0 | IsPlayerControlled | lua_is_player_controlled | arg0 through 00888D20; 00779B60 pushed as boolean. |
| 008A62A0 | SetCheatTurbo | lua_set_cheat_turbo | float arg1; class 18h -> +340h of members 0..4, stopping at the first null slot, count not read (008A6406..008A641F); else class 5 -> own +340h. |
| 008A6640 | UnitGetFireStance | lua_unit_get_fire_stance | director = vcall +114h; +3Ch set -> 1 + (+3Dh != 0); clear -> +3Dh ? 3 : 0 (008A6762..008A677D); push_number. |
| 008A6C30 | UnitSetPlayerCommandsEnabled | lua_unit_set_player_commands_enabled | director = vcall +114h before reading arg1 int; 0071DD30(director, int) at 008A6D76. |
| 008A7780 | PilotStop | lua_pilot_stop | 0077D600(entity, 00E08F88, zero target, 1) (008A7896..008A78E9). |
| 008A7940 | PilotClearOrders | lua_pilot_clear_orders | 0077D600(entity, 00E08F08, zero target, 1) (008A7A8F..008A7AA9). |
| 008A8280 | GetRelativeSpeed | lua_get_relative_speed | both entity reads take argument 0 (PUSH EBP 008A835E and 008A8397); if +10Ch clear: 00414DB0, +10Ch = 1, 00B63D50(+110h, +CCh); v = vcall +34h(other); local = 0042D0D0(v, +110h, 0); push float(vcall +38h(observer) + local.x) (008A8410..008A842D). |

## Segment 56 bindings 008A8480..008BCF10, 008AC420 (`cc13_left55_seg56b`)

| Address | Lua name | C++ function | Behaviour and evidence (instruction addresses) |
|---|---|---|---|
| 008A8480 | GetETA | lua_get_eta | Both entities read from argument 0 (PUSH EBP, EBP=0 at 008A855E and 008A8597). Lazy inverse of A (byte +10Ch, 00414DB0, 00B63D50 into +110h); B vtable+34h vector turned into A's frame by 0042D0D0 (normalize 0); closing = (float)(A vtable+38h + local.z) (008A8619); nil unless closing > double 0.001 (00CF3F30, JBE at 008A862F); else push length(B.pos - A.pos)/closing (0042B2F0, FDIV at 008A86B3). Returns 1. |
| 008A8B40 | SetPartyID | lua_set_party_id | Argument 0 is a net id (low word) resolved through 00F89A10/0C/54 or 60/A8 (008A8C48..008A8C7A); argument 1 the party. vtable+5Ch(2) true: message 7Fh (vtable 00D032E8, +1Ch party) routed 0077C2A0(entity, 7, 0); false: vtable+2Ch(party, [entity+58h], &0) (008A8D16). Pushes the party (008A8D2D). Returns 1. |
| 008A8D90 | SetCommandBuildingOwnerPlayer | lua_set_command_building_owner_player | vtable+5Ch(1Ch) true: message D3h (vtable 00CFADBC, +1Ch player, byte +20h = 1) routed 0077C2A0(entity, 7, 0) (008A8EE7..008A8F2A). Returns 0. |
| 008A91B0 | SetAiControl | lua_set_ai_control | byte entity+290h = 0; no other argument read. Returns 0. |
| 008A9460 | GetPrimaryTarget | lua_get_primary_target | Entity null or not class 2 -> null (008A9578), then vtable+178h(&target) on it. Object target: 00521EA0 twice, id word +174h -> 004260B0 text -> thisTable[text] pushed (00B67910, 00B678E0, 00B663D0). Position target: new table with x/y/z of target+8h (0088BA30). Else nil. Returns 00B66400. |
| 008AA560 | PutRelTo | lua_put_rel_to | A, B, offset(arg 2). Copy of B's world matrix (REP MOVSD 008AA716) with translation = 004142E0(offset, B+CCh); y zeroed when A is class 6 but not 8 (008AA784). A vtable+114h, 007788B0 when class 2 (result unused), vtable+120h(&matrix), vtable+11Ch((float)(deg*pi/180)) when count == 4, vtable+D8h. If class 2 and 00778890: every other member of [A+284h] (count +4F8h, 0070D060) gets vtable+118h(00F87574) and vtable+D8h. Returns 0. |
| 008AAB90 | ClearForcedReconLevel | lua_clear_forced_recon_level | 00803B50 on entity+1E8h+34h*level; class 18h: each plane slot (<= +3CCh, null past slot 4); class 1Ah: checked walk of the vector at +394h with the 00BF6713 asserts (008AAD83..008AADE4). Returns 0. |
| 008ABAC0 | GetRoleAvailable | lua_get_role_available | New table on the frame, [1..9] = dwords entity+188h.. via 00B665D0 (008ABC01). Returns 00B66400. |
| 008ABC70 | GetPlayerIndexBySlot | lua_get_player_index_by_slot | 0, or [[game+18CCh+4*slot]+1Ch] when game is non-null; pushed as integer. Returns 1. |
| 008ABDF0 | GetValidPlayerInSlot | lua_get_valid_player_in_slot | game && record+8h && !record+9h (008ABF0D..008ABF2C). Returns 1. |
| 008ABF90 | GetRoleOwner | lua_get_role_owner | As GetRoleAvailable from entity+1ACh. Returns 00B66400. |
| 008AC9D0 | Save | lua_save | Only the dead prologue string; no frame; XOR EAX,EAX at 008ACA1B. |
| 008ACD10 | AddFire | lua_add_fire | Reads entity(0), integer(1), integer(2), number(3); nothing else. Returns 0. |
| 008ACF00 | GetFire | lua_get_fire | Walks the vector at entity+344h (size 0043EC50, checked index 00BF6713); pushes 0.0 for the first record with +4h == arg1 and +8h == arg2 (008AD0E7). Returns 0 or 1. |
| 008AD150 | SetRepairLevel | lua_set_repair_level | entity(0), integer(2) read before string(1) (PUSH EBX=2 at 008AD269); 00827A40(entity, 0081AD40(name), level). Returns 0. |
| 008AD540 | FailureRepairEnable | lua_failure_repair_enable | vtable+5Ch(6) result discarded (008AD65F); 00939FE0(entity+A20h, boolean(1)). Returns 0. |
| 008ADC40 | GetRepairLevel | lua_get_repair_level | Pushes 0080E130(entity, 0081AD40(string(1))). Returns 1. |
| 008AE180 | KillStationaries | lua_kill_stationaries | World list [[[game+19CCh]+4]], next +38h read after the kill; class 36h -> 00926D90(entity, 0). Returns 0. |
| 008AE2F0 | KillBullets | lua_kill_bullets | Same walk; class 29h or 2Ah or 2Bh -> 00926D90(entity, 0). Returns 0. |
| 008AE760 | GetAirbaseOrders | lua_get_airbase_orders | 006BCD20(entity, DL=0); pushes bool +E0h, bool +E1h, integer +E4h. Returns 3. |
| 008AE900 | SetAirbaseOrders | lua_set_airbase_orders | Readers run integer(3), boolean(2), boolean(1) (008AEA6C..008AEA86); 006BF650(block, arg1, arg2, arg3). Returns 0. |
| 008AEB20 | SetBorderZoneParty | lua_set_border_zone_party | vector(0), integer(1); 00489C40(game, &vector) record +4h = party (008AEC7D). Returns 0. |
| 008AF260 | GetBorderZone | lua_get_border_zone | Two tables {x,y,z} of floats game+711Ch..7124h and +7128h..7130h (00B67930, 00B67400). Returns 00B66400. |
| 008AF700 | GetBorderCross | lua_get_border_cross | Start = entity position (refresh) or vector(0). bearing = (float)(deg*pi/180); with an entity += atan2([+F4h],[+ECh]) - pi/2 (00BF701A, 008AF979). End = start + 50000*(-sin, 0, cos) unless 0071C4F0 says outside (then (0, y, 0)). 004BC120 crossed -> table {x,y,z}, else nil. Returns 00B66400. |
| 008AFFC0 | EndSCore | lua_end_score | No frame. Host and latch game+1EF4h clear: set it, message E4h (004C9780 with [game+18C8h], 1) sent by 0076A9F0(game+1EF0h); exactly one live record of 8 at game+750h (stride 118h) -> [00F8A2FC] vtable+198h, then +184h. Returns 0. |
| 008B0470 | TraingingGroundEndScene | lua_training_ground_end_scene | No frame. 004D7970(game, 1). Returns 0. |
| 008B4DA0 | MovCam_PathWalkPathPointPropertyPotlekokkal | lua_movcam_path_walk_path_point_property_potlekokkal | value(0), float(1), MovCam tail from 2 (bool, number, string, variants via 00887370(6,-1,4) when count >= 6); 005CD160([[00E198C4]+98h]); 007A74F0(target, copy of arg 0, point, flag, number, &name, variants); host session (mode == 1) -> 0076D5B0(game+1EF0h, &arg0, point, flag, number). Returns 00B66400 (nothing pushed: 0). |
| 008B8970 | Scoring_GetRealPlayTime | lua_scoring_get_real_play_time | Pushes float [[game+21A0h]+14A0h]. Returns 1. |
| 008B8CA0 | Scoring_SetRanking | lua_scoring_set_ranking | Slot = game+18ECh, or integer(0) in a session (then ranking is arg 1); 00905350(scoring, slot, ranking). Returns 0. |
| 008B8E60 | Scoring_ForceRefreshScoringTable | lua_scoring_force_refresh_scoring_table | Slot as above; 009150A0(scoring, slot). Returns 0. |
| 008B9E90 | Scoring_AddActionScore | lua_scoring_add_action_score | Slot from arg 0 in a session; offline with count == 3 arg 0 is read and discarded (008B9FD5); key string, integer; 0090F140. Returns 0. |
| 008BA550 | Scoring_AddShipScore | lua_scoring_add_ship_score | Same shape, 0090F1A0. Returns 0. |
| 008BAC10 | Scoring_AddPlaneScore | lua_scoring_add_plane_score | Same shape, 0090F200. Returns 0. |
| 008BB2D0 | Scoring_AddCommandScore | lua_scoring_add_command_score | Same shape, 0090F260. Returns 0. |
| 008BBC40 | ScoringIsUnlocked | lua_scoring_is_unlocked | Pushes 007FC4C0(game+650h, string(0)). Returns 1. |
| 008BC740 | SetAchievements | lua_set_achievements | Slot from arg 0 when is_integer(0); token string; value integer when count > index+1 else 1; 0090EDE0(scoring, slot, token, value, 0). Returns 0. |
| 008BCD80 | Multi_GetPlayers | lua_multi_get_players | Table {[slot 0..7] = record+28h} for records with +9h clear (0-based keys, EDI). Returns 00B66400. |
| 008BCF10 | GetPlayerDetails | lua_get_player_details | Table {[slot] = {party +28h, ai +9h, playerindex +1Ch, playerslot, playerName char[] +58h, ailevel +2Ch}} for records with +8h and (!+9h or +0Ah). Returns 00B66400. |
| 008AC420 | IsUnitUntouchable | lua_is_unit_untouchable | vtable+140h non-null (called twice) and its byte +1D4h. Returns 1. |

## Segment 56 bindings 008BDEC0..008CF7E0 (`cc13_left55_seg56c`)

| Address | Lua name | C++ function | Behaviour and evidence (instruction addresses) |
|---|---|---|---|
| 008BDEC0 | Objectives_ClientRefresh | lua_objectives_client_refresh | byte [game+218Ch]==0 and [game+1FE4h]==1 (EBX=1) -> 00627960(), 00626C50() (008BDFBC/008BDFC1); returns 0 |
| 008BE010 | Hack_DisableBoatFormations | lua_hack_disable_boat_formations | byte [game+19D0h] = get_boolean(0); returns 0 |
| 008BE180 | Hack_ForceColumnFormationNearIslands | lua_hack_force_column_formation_near_islands | byte [game+19D1h] = get_boolean(0); returns 0 |
| 008BE2F0 | GetPlayerActWeapon | lua_get_player_act_weapon | pushes C string [00E0CFC4 + [[interface+50h]+44h]*4] via 00B66710 (008BE3D4..008BE3EE); returns 1 |
| 008BEA50 | GetPlaneEquipment | lua_get_plane_equipment | plane = entity if vt+5Ch(0Fh) else [entity+3D0h] (null -> no push, 008BEB82); pushes int [plane+C54h]; returns 1/0 |
| 008BEBE0 | GetPlaneIsReloading | lua_get_plane_is_reloading | result 0, else 007B92E0 on entity (kind 0Fh) or non-null [entity+3D0h] (008BED18); push_boolean; returns 1 |
| 008BEF80 | GetSquadTorpedoes | lua_get_squad_torpedoes | flag = get_boolean(1) iff count()==2; NewTable; list [game+19CCh] count +21Ch node +224h; e=[node+8], byte +5Eh==0 and ((e+310h)->vt+38h() or flag) and a squadron slot (<=4) == [e+3BCh] -> table[n++] = self(e) (008BF192/008BF1A5); returns 00B66400 |
| 008C09A0 | SetRoleHack | lua_set_role_hack | get_integer(2) then get_integer(1) (008C0AEF/008C0AF7); 0077C470(entity, arg1, arg2); returns 0 |
| 008C0B70 | SetMouseExclusiveRepairCategory | lua_set_mouse_exclusive_repair_category | empty body: frame opened, 00B66400 at 008C0C5A; returns 0 |
| 008C0E10 | ActivateObjectives | lua_activate_objectives | 005ED5D0([interface+60h], get_boolean(0)) (008C0F26); returns 0 |
| 008C1620 | SetRocketAirGroundTypeDifferent | lua_set_rocket_air_ground_type_different | byte 00F874FD = !get_boolean(0) (SETZ); returns 0 |
| 008C1790 | SetHP | lua_set_hp | 00877B90(entity_at(0), (float)get_number(1)) (008C18CB); returns 0 |
| 008C2AC0 | WipeEntityType | lua_wipe_entity_type | kind 0, 3Dh when _stricmp(name,"cloud")==0 (008C2B96/008C2BA7); list [[[game+19CCh]+4]] next +38h; byte +5Eh==0 and [+C4h]==kind -> 00926D90(e,2); returns 0 |
| 008C2C50 | GetTargetInfoTarget | lua_get_target_info_target | t = [[interface+B4h]+8]; null -> push_nil (008C2DA4); else thisTable[u16 t+174h] (004260B0..00B663D0); returns 1 |
| 008C3170 | SetVisibilityTerrainNode | lua_set_visibility_terrain_node | get_boolean(1), get_string(0); node = 00B72300([[interface+54h]+0Ch], name, "Terrain Visibility Group"); factor (float)flag; 00B6DA70(node,f,0); parent [node+30h]; c=00B6E750(parent, 00B6D800(parent), 1); c -> 00B6DA70(00B6E750(c,name,1), f, 0) (008C3311..008C3393); returns 0 |
| 008C3610 | GetDevice | lua_get_device | list [entity+48h] next +44h; vt+5Ch(1Eh) and [+38Ch]==get_integer(1) -> thisTable entity; end -> push_nil (008C3782); returns 1 |
| 008C3880 | GetKamikazeByDummy | lua_get_kamikaze_by_dummy | list [[game+19CCh]+130h] next +4; [[node+8]+6B8h]==get_integer(0) -> thisTable entity of [node+8]; no match pushes nothing; returns 1/0 |
| 008C3AB0 | GetFormationPositions | lua_get_formation_positions | group [entity+284h]; NewTable; i<[group+4F8h]: 00B66670/00B67720, rec=0070D070(group,i), k=[group+500h], 0088BA30(sub,{-0.0f-rec[10h+4k], 0, -0.0f-rec[20h+4k]}) (00D7A208 = 80000000h, 008C3C32..008C3C5D); returns 00B66400 |
| 008C3CE0 | ReloadTorpedoes | lua_reload_torpedoes | list [entity+48h] next +44h; vt+5Ch(20h) and [[+3F4h]+80h]==7 -> barrels 0..[+448h): 0072CF00(gun, b, 0.0f, 0) (008C3E32); returns 0 |
| 008C3EA0 | Scoring_IgnorePlayersPartyLosses | lua_scoring_ignore_players_party_losses | byte [[game+21A0h]+1484h] = 1; returns 0 |
| 008C3FE0 | Scoring_AddPartyLoss | lua_scoring_add_party_loss | amount = 1 (008C40A8), get_integer(2) iff count()>=3; get_string(1); get_integer(0); 0090ED90([game+21A0h], party, &name, amount) (008C4169); returns 0 |
| 008C4200 | SetCounterTimer | lua_set_counter_timer | 00528BE0([interface+C8h], (float)get_number(0)) (008C4314); returns 0 |
| 008C4B50 | SetShallowWater | lua_set_shallow_water | o = [[[game+19CCh]+34Ch]+8]; non-null -> get_string(0), 00880220(o, &name) (008C4C86); returns 0 |
| 008C5FD0 | RemoveTrigger | lua_remove_trigger | list = entity_at(0)->vt+60h() (008C60ED); get_string(1); push_boolean(0074FBB0(ECX list, EDX &name)); returns 1 |
| 008C7010 | CheatRemoveWater | lua_cheat_remove_water | vt+5Ch(6) -> vt+23Ch(); returns 0 |
| 008C74B0 | GameOver | lua_game_over | 00A933F0(004BEC00(), 1Eh, 5); byte [[00F8A0C4]+D0h]=1; 0096BF70; lists recon(008053C0(0))+DDCh/+DE8h/+DF4h next +4: o=[[node+8]+4], byte o+5Dh==0 -> o->vt+F4h(1.0f) (008C75DB FLD1); returns 0 |
| 008C8560 | class_enumeration | lua_class_enumeration | 004CAC30/004D2B10(game); VehicleClass global; count()==0: iterate keys k>=0 (15Eh -> 004254B0("kukac!")), skip Type LandVehicle/LandFort/CommandBuilding, else record+28h=0, 00964790(k,1)->vt+18h(0Fh), second pass with +28h=1; else get_integer(0) build once; final 004CAC30; literal return 0 |
| 008C9460 | AddLineOfSightTrigger | lua_add_line_of_sight_trigger | 00750550(entity_at(3), (float)get_number(4)); optional string arg 1 (008C9350); get_string(2); 00750790; 0074FAB0(entity_at(0)->vt+60h(), trigger); returns 0 |
| 008C9730 | AddProximityTrigger | lua_add_proximity_trigger | 0088A810 target from arg 3; 007505C0(&target, (float)get_number(4)); then as AddLineOfSightTrigger; returns 0 |
| 008C9A10 | AddSquadOrdnanceDroppedTrigger | lua_add_squad_ordnance_dropped_trigger | 00750660() first (008C9AF6); then optional arg 1, get_string(2), 00750790, 0074FAB0; returns 0 |
| 008CBEB0 | HackSensitiveUnit | lua_hack_sensitive_unit | ptr = 00888D20 "Ptr" of arg 0; 008CAEE0([00F8A0C4]+1A4h, &ptr) (008CBFE0); returns 0 |
| 008CC330 | Scoring_IgnoreEntityKill | lua_scoring_ignore_entity_kill | 0063F1E0([game+21A0h]+1488h, &entity_at(0)) (008CC454); returns 0 |
| 008CC850 | Loading_Start | lua_loading_start | [game+644h]<=0: 0096BF70, 004CD0F0(game,1,0,1), 0068A670(interface,&m,&v), 00867EE0(004D1100(),0.0f,[game+19FCh]), [00F8BBD8]->vt+4(&m, v), 0060D6E0([00E19698],1) (008CC954..008CC9DD); returns 0 |
| 008CCEB0 | SortByDistance | lua_sort_by_distance | origin pose of entity_at(0) (00414DB0 when +C8h==0); iterate arg 1: copy value (00442220), score vector3 (0088B840/00888760) or Ptr entity pose; x87 squared distance (008CD0FB..008CD1FE); pairs sorted by 008C9E10; NewTable; table[i+1]=objects[pair.index] with checked index (00BF6713); returns 00B66400 |
| 008CECC0 | GetPayloads | lua_get_payloads | NewTable; work vector seeded with entity or squadron slots (vt+5Ch(18h)); BFS over [e+48h]/+44h; vt+5Ch(2Ah) -> table[n++]=self(e) (008CEF08/008CEF1F); returns 00B66400 |
| 008CEFC0 | GetPayload | lua_get_payload | same seed; then get_integer(1); BFS; each vt+5Ch(2Ah) decrements n, n<1 -> thisTable entity (u16 +174h, 008CF23E); exhausted -> nothing; returns 1/0 |
| 008CF7E0 | GetGuns | lua_get_guns | GetPayloads with vt+5Ch(20h) (008CFA17); returns 00B66400 |

## Existing implementation, recorded only

CountdownCancel 008B19A0 is implemented by name in `GameScriptOrdersHost`
(src/game_hosts_script_orders.cpp, the "CountdownCancel" branch). It gets a ledger record
only (status `host_implementation_partial`); that file was not edited.

### Defects in cc9 files

- `src/game_hosts_script_orders.cpp`, CountdownCancel: the native calls 0052AB90 twice on
  `[[00E198C4]+C8h]` (008B1A92, 008B1AA2). The host logs those calls as unimplemented and
  does not perform them.
- Same branch: when `[game+1FE4h] == 1` (session host), the native calls
  00770940(game+1EF0h) at 008B1AE8 to broadcast the countdown stop. The host has no such
  call.
