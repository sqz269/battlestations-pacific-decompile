# cc13-seg87: segments 87, 89 and 75 leftovers

Packet cc13-seg87 (2026-10-14). The remaining functions without a reconstruction record in
segment 87 (Lua script runtime, 00B65BA0..00B6D1B0), segment 89 (scene resources and
refcounting, 00B80FD0..00B86E10) and segment 75 (GUI list controls, 00A9AC70..00AA0F50).

Code: `include/bsp/cc13_seg87.hpp`, `src/cc13_seg87.cpp`, plus one instantiation line for the
Listbox pool in `src/native_material_pools.cpp`. Status of everything below: reconstructed and
build-tested (compiled at /W4 /WX /fp:strict); no fixture, not wired into a host, not
game-validated, not ABI-compatible. Names are hypotheses, not recovered symbols.

## Approach

- Segment 87 is the game's LuaPlus-style object layer over the linked stock Lua 5.1.1 (the
  repository builds the unmodified 5.1.1 sources, `third_party/README.md`). None of these
  functions is a stock Lua API routine; they are the game's wrappers. They are written over the
  existing actual-storage model `NativeLuaObjectStorage` (14h) / `NativeLuaStateStorage` (4C8h)
  in `include/bsp/native_lua_objects.hpp` and call the stock API at the same points. Every Lua
  call goes through `object.owner_00->state_04`, reloaded per call as the listings do.
- Segments 89 and 75 work on actual native objects through raw offsets. Calls the binary makes
  to code outside this packet, and virtual calls, go through named hook interfaces
  (`NativeSeg89Calls`, `NativeModelWalkCalls`, `NativeGuiListboxCalls`,
  `NativeGuiAnimatedValueCalls`, `NativeLuaScriptCacheCalls`), invoked at the same point with
  the same arguments.
- Ten pointer-vector members (00B85750, 00B857A0, 00B85A30, 00B85AC0, 00A9AEB0, 00A9AF00,
  00A9B250, 00A9B2D0, 00A9EA60, 00A9EE40) are instruction-for-instruction the Widget pool's
  00AA6FA0 / 00AA6FF0 / 00AA76C0 / 00AA7740 apart from jump targets (normalised listing
  comparison, local script `cc13-seg87_cmp.py`), so they delegate to the existing
  `native_gui_pool_table_*` bodies instead of re-implementing them.
- The Listbox pool (global 00F8BC10, vtable D5BBA4, slot 180h x 16, slab 1824h) is one more
  instance of the shared slab engine. Construct, Destroy, Trim, Push, Reserve and InitBlock match
  the Widget/Model pool listings modulo constants; Allocate matches the Model pool 00ACC350 and
  FrameBox 00AD0050 shape; Free matches Model 00ACC050. `GuiListboxPoolTraits` carries the
  constants; the allocate stub is 00A9B7E0 (mislabelled `CG_static_dtor_stub`) and the atexit
  destroy stub 00CE0A00.

## Segment 87

| Address | C++ | Original ABI | Behaviour |
| --- | --- | --- | --- |
| 00B65C30 | `transform_plane_by_matrix4x3_00b65c30` | thiscall(plane), stack matrix, RET 4 | n' = n*R per component `(y*m[4+j] + x*m[j]) + z*m[8+j]`; the translation goes through 00B65AC0 with the transposed rotation (the decompiler drops that call's output); d' = d - dot, dot stored as float32 first. |
| 00B65E20 | `construct_native_lua_owner_with_dofile_00b65e20` | thiscall(owner), stack L, RET 4 | owns = 0, state = L, `DoFile` (00B69E00, zero upvalues) registered. No tracking-slot init. |
| 00B65EC0 | `native_lua_do_buffer_00b65ec0` | thiscall, stack (buf, size), RET 8 | `luaL_loadbuffer(.., "DoBuffer")` status ignored, `lua_call(0, -1)`. |
| 00B65EF0 | `native_lua_do_buffer_protected_00b65ef0` | same, EAX 0/1 | load + `lua_pcall(0, -1, 0)`; results/error stay on the stack. |
| 00B65F70 | `native_lua_is_bound_00b65f70` | RET, AL | kind != 0. |
| 00B65F80 | `native_lua_type_00b65f80` | RET, EAX | kind 0 -> -1, kind 2 -> `lua_type`, other kinds -> 5. |
| 00B660F0 | `native_lua_equals_cstring_00b660f0` | stack text, RET 4 | kind 2 STRING and `strcmp == 0`. |
| 00B66160 / 00B66200 | `native_lua_is_lightuserdata_00b66160` / `native_lua_is_function_00b66200` | RET, AL | 00B65F80 logic == 2 / == 6. |
| 00B662E0 | `native_lua_to_pointer_00b662e0` | RET, EAX | `lua_topointer`, no kind gate. |
| 00B663D0, 00B66430, 00B66450, 00B664B0, 00B66710 | `native_lua_push_*` | RET / RET 4 | `lua_checkstack(1)` then pushvalue / pushnil / pushboolean(byte != 0) / pushnumber(int) / pushstring. |
| 00B664E0, 00B66530, 00B665D0, 00B66620, 00B66670, 00B666C0, 00B66740, 00B672F0 | `native_lua_set_*_at_*` | RET 4 / RET 8 | `checkstack(2)`, pushnumber(int key), push value (nil, raw byte boolean, number, lightuserdata, new table, VALUE's index on THIS state, C string, NativeString), `lua_settable` at the object's current index. |
| 00B66960 | `destroy_native_lua_fundamentals_00b66960` | thiscall, RET | vtable D62C18, `_free` bytes when set, clear, global 0108FF1C = 0, vtable CE3818. |
| 00B67050 | `native_lua_release_00b67050` | stack byte, RET 4 | bound: 00B66DE0(owner, object, index, byte), kind = 0. |
| 00B67350 / 00B67630 | `native_lua_set_nil_00b67350` / `native_lua_set_string_00b67630` | RET 4 / RET 8 | NativeString key = nil; C-string key = NativeString value. Null NativeString data reads as the empty byte 0108FF2C. |
| 00B678E0 / 00B67910 | `native_lua_get_by_native_string_00b678e0` / `native_lua_get_by_literal_00b67910` | stack (fresh, key), RET 8 | 00B67800 with the key text. |
| 00B67930 | `native_lua_new_table_00b67930` | stack fresh, RET 4 | `lua_createtable(0, 0)`; untracked kind-2 object at the new top. |
| 00B679F0, 00B67A80, 00B67B40, 00B67C40 | `native_lua_get_vector2/3/4_*`, `native_lua_get_float4_00b67c40` | stack out, RET 4 | elements 1..N via 00B67720, `lua_tonumber` stored as float32, temp released (remove flag 1) when bound. 00B67C40 is a twin of 00B67B40. |
| 00B680A0 | `native_lua_get_vector3_or_00b680a0` | stack (out, fallback), RET 8 | table-like -> 00B67A80, else copy three floats. |
| 00B68630 | `native_lua_to_display_string_00b68630` | stack fresh, RET 4 | jump table 00B6879C: "none", "nil", "true"/"false", "lightuserdata", number via 00B66270 + 0043BDF0 (hook), the string, "table", "function", "userdata", "thread", "Unknown" (strings read from the PE). |
| 00B687D0 / 00B68AD0 | `native_lua_deep_copy_to_index_00b687d0` / `native_lua_deep_copy_to_field_00b68ad0` | stack (key, source), RET 8, AL | see the header comment; the iteration ends on the KEY's kind; NUMBER keys truncate the float32 (_ftol2), STRING keys build a pooled NativeString released inline, other key types return 0. |
| 00B69F20 | `register_native_lua_dofile_00b69f20` | thiscall, RET | `DoFile` registration only. |
| 00B6BD60, 00B6C0E0, 00B6D0A0 | `close_/open_and_run_/clear_native_lua_script_cache_*` | thiscall, RET | the unreferenced script-cache instance methods over `NativeLuaScriptCacheStorage` (names vector +0, records +10h, open byte +20h, owner +24h). 00B65E80 is byte-identical to 00B669A0 and is reproduced by `close_native_lua_state_00b669a0`. |

Existing implementations recorded ledger-only: 00B6BD80 `run_record_00b6bd80` and 00B6D1B0
`build_compressed_00b6d1b0` (`src/lua_script_loader_cc13.cpp`), checked against the listings.

## Segment 89

| Address | C++ | Original ABI | Behaviour |
| --- | --- | --- | --- |
| 00B80FD0 / 00B81000 | `construct_native_parser_map_00b80fd0` / `construct_native_cache_map_00b81000` | thiscall, RET | std::map constructors over 00B7E000 / 00B7E050 (isnil at +19h). |
| 00B81170 / 00B811A0 | `destroy_native_string_table_rows_*` | thiscall, RET | 00B80E10(0) then `_free(rows+0)`. Identical bodies. |
| 00B81A30, 00B81A70, 00B86C80, 00B86CC0 | `deallocate_native_block_*` | stack (block, n), RET 8 | operator delete 00BF65AC(block). |
| 00B81C10 | `allocate_native_registry_elements_00b81c10` | fastcall(count) | overflow -> std::bad_alloc; else operator new 00BF681B(count*12). |
| 00B82350 | `clear_native_registry_list_00b82350` | thiscall, RET | list clear, head relinked before the nodes are deleted. |
| 00B82B50 | `construct_native_registry_list_00b82b50` | thiscall, stack (unused), RET 4 | head = 00B82390, size 0. |
| 00B82CC0 | `free_native_registry_storage_00b82cc0` | thiscall, RET | vector storage release. |
| 00B84FE0, 00B850F0, 00B85200, 00B85310 | `set_native_model_parameter_owner_by_effect_00b84fe0`, `replace_native_model_texture_00b850f0`, `set_native_model_diffuse_by_effect_00b85200`, `set_native_model_texture_by_effect_00b85310` | fastcall(model, key), stack args | recursive model walkers (type test, geometries, elements, material [element+20h], effect name, children +34h/+3Ch). 00B85200 writes the diffuse colour (the ledger name says Get). |
| 00B854D0, 00B85510, 00B85540 | `construct_/delete_native_debug_render_flags_*` | thiscall | vtable D63190 constructor; two identical deleting destructors (global 010901CC = 0, base vtable CE3818). |
| 00B85750, 00B857A0, 00B85A30, 00B85AC0 | `*_native_pointer_vector_*` | thiscall | delegate to the identical 00AA6FA0 family. |
| 00B85980 / 00B859A0 | `fill_native_slot_table_00b85980` / `native_slot_table_is_full_00b859a0` | thiscall | 64 dwords at +60h stride 64h; word +1980h == 40h. |
| 00B85B30 | `unlink_native_ref_chain_00b85b30` | thiscall, stack node, RET 4 | unlink from the +38h chain (no end test), drop one reference. |
| 00B860E0 / 00B861D0 | `construct_native_ref_record_00b860e0` / `assign_native_ref_record_00b861d0` | RET 24h / RET 4 | constructor and assignment of a 60h refcounted record (the ledger's FixedSlotTable names are wrong). |
| 00B866B0, 00B86800, 00B86810, 00B86860, 00B86870, 00B86880, 00B868A0 | trivial returns | | 0, globals 01090210/01090218/0109021C/01090228, two no-ops (RET 10h / RET 4). |
| 00B86820 | `dispatch_native_fallback_resource_00b86820` | RET 10h | `[[visitor]+8](resource, arg3)`. |
| 00B868D0 | `native_fallback_resource_is_type_00b868d0` | RET 4 | token in 01090210..01090217. |
| 00B86AC0, 00B86AE0, 00B86B90 | `initialize_native_fallback_type_00b86ac0`, `delete_native_resource_interface_00b86ae0`, `replace_native_ref_slot_00b86b90` | | 00B86A00 thunk; vtable D63208 deleting destructor; +0Ch = [[+0Ch]+10h](6, 0). |
| 00B86D00 / 00B86DF0 | `rotate_native_type_tree_00b86d00` / `native_type_tree_maximum_00b86df0` | | `_Rrotate` (the ledger's RotateLeft is a misnomer) and `_Max` over nodes with isnil at +21h. |

## Segment 75

| Address | C++ | Original ABI | Behaviour |
| --- | --- | --- | --- |
| 00A9AC70 | `set_native_listbox_input_filter_00a9ac70` | RET 4 | +144h = routine. |
| 00A9B7F0 | `describe_native_listbox_properties_00a9b7f0` | stack writer, RET 4 | base 00AAAED0, then DontMoveTheItems / AutoControl / CenterVertical (bool) and LineDistance / HeightPlus (float) written as ({0, name}, value) only when 00BD5680 says they differ from false / 0.0f; a non-empty +16Ch vector becomes an "Items" table of ({1, i+1}, {0, text}). Tagged values are 8-byte {type, value}: 0 string, 1 int, 2 float, 3 bool. |
| 00A9E400 | `read_native_listbox_properties_00a9e400` | stack reader, RET 4 | base 00AAA710; reads the three bools and NewHighlightIndex (+118h, default 1); erases +16Ch; LineDistance / HeightPlus only when present; "Items" {1, i} while present into pooled strings (00450540 append, inline release); each name resolved by 00AA7E00(name, 1) and appended by 00A9D750(row, null, 0); then +80h(1). |
| 00A9ACD0, 00A9AFD0, 00A9B0B0, 00A9B150, 00A9B330, 00A9B4A0, 00A9B580 | Listbox pool entries | per `GuiListboxPoolTraits` | shared slab engine instance. |
| 00A9AEB0, 00A9AF00, 00A9B250, 00A9B2D0, 00A9EA60, 00A9EE40 | slab-table members | thiscall | delegate to the identical 00AA6FA0 family. |
| 00A9BB30 | `close_native_listbox_paging_00a9bb30` | thiscall, RET | paging off: list clear 00A9B3F0, both arrows hidden. |
| 00A9BB70 | `layout_native_listbox_item_00a9bb70` | stack (row, apply), RET 8 | kind-3 target row or child; position (width, size.y); +30h = -0.009375f (00CF3F64). |
| 00A9BF90 | `clear_native_listbox_items_00a9bf90` | thiscall, RET | hide, unlink and delete every row node with the listener suppressed; then +80h(0). |
| 00A9C860 | `select_native_listbox_row_by_data_00a9c860` | RET 4 | select the first row whose +D8h matches unless hidden. |
| 00A9C8F0 | `select_native_listbox_index_silently_00a9c8f0` | RET 4 | 00A9C7C0 with the listener cleared. |
| 00A9CC50 | `scroll_native_listbox_00a9cc50` | RET 4 | last `count` rows moved by half their height. |
| 00A9CDC0, 00A9CDF0, 00A9CE60 | `clear_native_listbox_selection_00a9cdc0`, `select_native_listbox_exclusive_*` | | deselect other lists, activate this one (+60h(1)). |
| 00A9CF30 | `reset_native_listbox_highlight_00a9cf30` | RET 8 | fit rows to the highlight widget, track the widest, 00A9CEB0. |
| 00A9D010 | `construct_native_gui_list_00a9d010` | thiscall, RET | head 00A9B720, size 0. |
| 00A9D360 | `scroll_native_listbox_to_item_00a9d360` | RET 0Ch | page step 0.001f / 0.25 constants from the PE. |
| 00A9E130 / 00A9E210 | `destroy_native_listbox_00a9e130` / `delete_native_listbox_00a9e210` | | destructor body (vtable D5BBF8) and the pool-freeing deleting destructor. |
| 00A9EC30 | `uppercase_native_wide_string_00a9ec30` | RET 4 | each code unit through the existing `locale_uppercase_00a9eba0`. |
| 00A9EF60 | `find_first_of_native_wide_string_00a9ef60` | RET 8 | `_wcscspn` find-first-of with the native start clamp. |
| 00A9F460 | `copy_construct_native_string_record_00a9f460` | RET 4 | NativeString copy into a 10h record, +8/+0Ch = 0. |
| 00AA0E20 / 00AA0E60 | `restart_native_gui_animated_value_00aa0e20` / `set_native_gui_animated_value_target_00aa0e60` | RET 4 | current animation +50h: stop and pick +54h/+58h; set target (target, 0, 1.0f) then the +4 routine. |

## Skipped

- 00B6A390 `BSP_LuaObject_WriteLuaSourceText`: about 1830 instructions with a std::sort over keys,
  string concatenation helpers and a recursive writer. Not reconstructable faithfully in this
  packet's budget.
- 00B83F30 `BSP_Camera_RenderToFrameTargets_Provisional`, 00B848C0 and 00B84A90 (screenshot
  BMP): renderer vtable composition with by-value NativeString temporaries under an EH frame; the
  decompiler output is stack-confused and the renderer/surface objects are not modelled here.
- 00A9E970 `BSP_StringMapNodePool_InitBlock` and 00A9F0B0 `BSP_StringMapNodePool_Allocate`: the
  StringMap node pool (00E1768C, slot 1Ch x 400h) is another slab-engine instance, but its
  construct (00436230), free (00736090) and trim live outside this packet (partly inside
  agent/argon-v2's segment-41 lease) and the engine templates are file-private, so a complete
  traits row was not established.

## Uncertainty

- x87 arithmetic in 00B65C30, 00A9CC50 and 00A9D360 is modelled with float32/double C++
  arithmetic; extended-precision intermediates can round differently in rare cases.
- 00B860E0/00B861D0 and the ref-chain owner are named by shape only; the owning class was not
  identified.
- Hook interfaces stand for real callees; a host must bind them to the actual implementations.
