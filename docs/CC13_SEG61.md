# CC13 segment 61 leaves (cc13-seg61)

Packet: segment 61, 0092E250..00951940. The partition keywords are owner player, resource
usage, cSmoothMapZoomLevel and support manager. This pass took the self-contained utility
classes at the segment's end and the engine's own dynamic-array helpers around 0093F590. It
also carries the three cc13-seg61 fixes.

Code: `src/support_seg61_cc13.cpp`, `include/bsp/support_seg61_cc13.hpp`
(namespace `bsp::support_seg61_cc13`). Objects are reached through `Raw` byte offsets in their
Win32 layout. Every call the image makes goes through a `Seg61Host` method named for its
target. The exceptions are the pure CRT routines memcpy, memset, strlen, strcmp and qsort,
which are called directly, plus two direct calls into existing C++: 00438E10
`compare_insensitive_00438e10` and 0093F6E0 `reserve_sound_sample_parameters_0093f6e0`.
Names are hypotheses, not recovered symbols. Status of every row: reconstructed and Win32
build-tested; no fixture, no original-ABI or game validation.

Most of the FUN_ bodies here had no export (they were created after the last snapshot). They
were read from the live Ghidra decompiler and listing (`bsp.py ghidra decompile|disasm`).

## Fixes (one commit each)

| Address | File | Change |
|---|---|---|
| 00AC3660 | src/engine_seg49_64_78_cc13.cpp | the constructor now calls 00AC3400 itself (`list_buy_sentinel_00aac180`, instruction-identical) and stores the sentinel at +4h; it used to take the sentinel from the caller (no callers existed) |
| 00439950 | src/plane_wanderer.cpp | rounding cos and sin to float (FSTP at 004399BF / 004399C9) before the products with r, behind `kPlaneWandererFloatTrigBound` (default false keeps the old double path) |
| 00432480 | src/ship_sub_entity_list.cpp | `sub_entity_list_self_00432480` appends `this` through `PointerVectorHost::pointer_vector_push_back_004323d0`; `sub_entity_list_slot0fc` appends through the same hook for all three providers instead of into a caller array |

## Reconstructed (45)

### Scene text writer (0094FDC0..00950350)

Layout: +0 vtable (slot 0 = write(data, length)), byte +4 = the line is already indented,
+8 depth, +0Ch indent width, char +10h fill. The chunk-list writer keeps head +14h, tail +18h
and total +1Ch, with 80Ch-byte chunks: vtable 00D19B74, data +4..+803h, used +804h, next +808h.
The file writer keeps its HANDLE at +14h. The scratch buffer is 00F89F80.

| Address | C++ | ABI | Body |
|---|---|---|---|
| 0094FDC0 | scene_text_writer_write_indent_0094fdc0 | thiscall, RET | depth * width fill chars once per line; byte +4 = 1 |
| 0094FF10 | scene_text_writer_write_string_0094ff10 | thiscall, RET 4 | indent, then the text when non-empty |
| 0094FF50 | scene_text_writer_write_line_0094ff50 | thiscall, RET 4 | as 0094FF10, then "\r\n" and byte +4 = 0 |
| 0094FFA0 | scene_text_writer_manipulator_0094ffa0 | thiscall(int), RET 4 | 0: newline; 1: one indent unit |
| 00950000 | scene_text_writer_write_int_00950000 | thiscall(int), RET 4 | `_itoa` (00BF9010) into the scratch buffer |
| 00950050 | scene_text_writer_write_unsigned_00950050 | thiscall(unsigned), RET 4 | 00439AC0("%u") |
| 00950130 | scene_text_writer_write_float_00950130 | thiscall(float), RET 4 | 004B8490(scratch, "%.4f", double) |
| 009500A0 | scene_text_chunk_list_join_009500a0 | thiscall, RET | concatenates the chunks into a new buffer |
| 00950190 | win32_file_writer_write_00950190 | thiscall(data, len), RET 8 | WriteFile on +14h |
| 009501B0 | win32_file_writer_open_009501b0 | thiscall(name), RET 4 | CreateFileA(CREATE_ALWAYS); 004531F0 on failure |
| 009501F0 | scene_text_chunk_list_append_chunk_009501f0 | thiscall, RET | tail->next = new chunk, tail = it |
| 00950240 | scene_text_chunk_list_write_00950240 | thiscall(data, len), RET 8 | chunk writer slot 0 |
| 009502E0 | scene_text_chunk_list_reset_009502e0 | thiscall, RET | head = tail = new chunk, total 0 |
| 00950320 | scene_text_chunk_list_clear_00950320 | thiscall, RET | delete the chain, clear head/tail/total |
| 00950350 | scene_text_chunk_destruct_00950350 | thiscall, RET | delete next |

### Pooled string list (0094EFB0..0094F540)

Layout: +0 array of pointers to 8-byte {length, data} strings, +4 count, +8 capacity
(grows by 40h).

| Address | C++ | ABI | Body |
|---|---|---|---|
| 0094EFB0 | string_list_construct_0094efb0 | thiscall, RET | capacity 40h, array 00BF55BE |
| 0094F010 | string_list_grow_0094f010 | thiscall, RET | capacity += 40h, reallocate |
| 0094F090 | string_list_find_0094f090 | thiscall(needle, bool), RET 8 | index or -1; flag = case-insensitive |
| 0094F1B0 | string_list_sort_0094f1b0 | thiscall(bool), RET 4 | qsort with 0094F130 (flag set) or 0094F180 |
| 0094F1E0 | string_list_join_lines_0094f1e0 | thiscall, RET | strings joined with '\n' |
| 0094F270 | string_list_append_0094f270 | thiscall(string, bool adopt), RET 8 | copy (00426060) unless adopted; returns count |
| 0094F2F0 | string_list_insert_0094f2f0 | thiscall(string, index, adopt), RET 0Ch | clamped insert |
| 0094F3A0 | string_list_delete_string_0094f3a0 | thiscall(slot), RET | pool release, operator delete, clear |
| 0094F3E0 | string_list_clear_0094f3e0 | thiscall, RET | delete all, count 0 |
| 0094F540 | string_list_destroy_0094f540 | thiscall, RET | clear, _free the array |

The comparators 0094F130 and 0094F180 are labels in the gap after 0094F090 (no Ghidra
function). They are reconstructed as `string_list_compare_0094f130` and
`string_list_compare_insensitive_0094f180` but not counted or recorded.

### Block buffer (0094F560..0094F770)

Layout: +0 vtable 00D19B64, +4 block size, +8 block array, +0Ch index, +10h last allocated,
+14h used, +18h flattened copy, +1Ch compared but never written by these bodies.

| Address | C++ | ABI | Body |
|---|---|---|---|
| 0094F560 | block_buffer_construct_0094f560 | thiscall(size), RET 4 | one block allocated |
| 0094F5E0 | block_buffer_destruct_0094f5e0 | thiscall, RET | free blocks, array, copy |
| 0094F650 | block_buffer_reset_0094f650 | thiscall, RET | rewind, drop the copy |
| 0094F680 | block_buffer_next_block_0094f680 | thiscall, RET | advance, growing the array |
| 0094F770 | block_buffer_flatten_0094f770 | thiscall, RET | contiguous NUL-terminated copy |

### Dynamic arrays and singletons

{data +0, count +4, capacity +8}; the 12-byte element is {float, float, int key}.

| Address | C++ | ABI | Body |
|---|---|---|---|
| 0093F590 | array12_reserve_0093f590 | thiscall(int), RET 4 | reserve, n >= 1 |
| 0093F610 | array4_reserve_0093f610 | thiscall(int), RET 4 | reserve, n >= 1 |
| 0093F670 | array4_resize_0093f670 | thiscall(int), RET 4 | resize, new slots zeroed |
| 0093F840 | array12_resize_0093f840 | thiscall(int), RET 4 | resize, new keys zeroed |
| 0093F7F0 | array12_push_0093f7f0 | thiscall(element*), RET 4 | doubling push |
| 0093F8A0 | array4_push_0093f8a0 | thiscall(dword*), RET 4 | doubling push |
| 0093F900 | array16_push_0093f900 | thiscall(element*), RET 4 | doubling push through 0093F6E0 |
| 0093F790 | array12_find_pair_0093f790 | thiscall(out, key), RET 8 | the pair for a key, default {100, 100} |
| 0093FC10 | vector_destroy_0093fc10 | thiscall, RET | 0093F840(0), _free |
| 0093FC30 | vector_destroy_0093fc30 | thiscall, RET | 0093F670(0), _free |
| 0093FC80 | singleton_owner_destruct_0093fc80 | thiscall, RET | array at +4 freed, [00F89B24] = 0, base vtable |
| 0093F520 | singleton_base_destruct_0093f520 | thiscall, RET | [00F89B24] = 0, vtable 00CE3818 |
| 00940D80 | singleton_base_destruct_00940d80 | thiscall, RET | [00F89B34] = 0, vtable 00CE3818 |
| 009318D0 | two_vectors_clear_009318d0 | thiscall, RET | zero +4..+0Ch and +14h..+1Ch |
| 0092E580 | dyn_world_apply_entity_transform_0092e580 | thiscall, RET | 00414DB0 when needed, tail jump 006FA6D0 |

Where the listing and the decompiler disagree (the listing was followed):

- 0093F790: the decompile drops the early return, so a match would always be overwritten by
  the default. The listing returns at 0093F7E4.
- 0094F3E0: the listing guards the loop with count > 0; the decompile shows a do-while.
- 009501F0 / 009502E0: on a failed allocation the listing stores null. The decompile shows the
  stores after the `if` as unconditional.

Behaviour kept as found:

- 0094F770 leaks the previous flattened copy and never updates +1Ch, so it rebuilds on every
  call once +1Ch is stale.
- The indent memset in 0094FDC0 and 0094FFA0 is not bounded to the 256-byte scratch buffer.
- The null tests on placement targets in the array helpers are kept.

Simplifications:

- 00F89B75, the zero byte that stands in for null string data, is a local empty string, since
  only its contents are read.
- SEH frames that only matter during unwinding (0094F270, 0094F2F0, 0093FC80, 009318D0) are
  not modelled.

## Skipped (125)

- Existing C++ that mentions these addresses, not verified in this packet, so no record added:
  00941D30 (src/lua_spawn_cc13.cpp), 00945850, 00945A20, 009478B0, 00948CC0, 00949300,
  00949530, 0094A140 (src/lua_spawn_new.cpp; 0094A140's C++ is an accessor and probably not
  the same function).
- Larger bodies not attempted: the unit controller network state and correction (0092E5B0,
  0092F2E0), the wreck debris spawn 00935540, the map scene and overlays 0093CB10..0093FF10,
  the spawn manager and Lua spawn bindings 00941D30..0094C900, the effect entities
  0094F830/0094F910/0094FB50/0094FC60, and 00951820.
- Not examined in this pass. The ones looked at (0093A8B0, 0093AD30, 0093AD90, 0093AE60,
  00945DA0, 00946EC0, 0094CA10) are container/template code that should be tagged: 0092E250, 0092F930, 0092FD50, 00930520,
  00930570, 00930F20, 00931070, 009311C0, 00931200, 009317D0, 00931820, 009319D0,
  00931B70..00933DA0, 0093A2C0..0093B920, 00943F70..00944540, 00945450, 00945820,
  00945CF0..00946F00, 00947680..00948BB0, 0094BFF0, 0094C830, 0094CA10, 0094CA40, 0094EEF0,
  009510F0, 00951450.
