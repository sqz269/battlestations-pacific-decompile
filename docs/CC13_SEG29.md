# Segment 29 (cc13-seg29): Lua machine, result variants, input helpers

Packet cc13-seg29, segment 29 (006964B0..006BE730). Code: `src/settings_seg29_cc13.cpp`,
`include/bsp/settings_seg29_cc13.hpp` (namespace `bsp::cc13_seg29`). Status of every row:
reconstructed and build-tested; not ABI-compatible, not fixture-tested, not game-validated.
Names are hypotheses, not recovered symbols.

## Conventions

- The image's Lua 5.1.1 uses a register convention (lua_State* in ECX, the first integer
  argument in EDX, the rest on the stack). The C++ calls the stock 5.1.1 build (`bsp_lua511`)
  with the same argument values; the C++ interface is not the original ABI.
- `LuaMachine` is a C++ class with one virtual destructor, which gives it the image's layout
  {vftable +0h, lua_State* +4h, owns-state byte +8h}; the image's vtable 00CF8394 has exactly one
  slot (the scalar deleting destructor 006B87C0; 00CF8398 is the "nSlu" string).
- Count/capacity arrays use the raw-storage style of `src/native_input_binding_storage.cpp`
  (operator new 00BF55BE and _free 00BF6989 through `singleton_lifetime_allocate/free`), and call
  its reconstructed reserve/resize functions (00696D80, 00696DE0, 00696E70, 00697220).
- Calls into bodies that are not reconstructed in C++ go through named hook interfaces invoked at
  the same point with the same arguments (`LuaVariantHooks`, `FrontEndActionHost`,
  `InputConfigurationHooks`, `InputBindingNameHooks`, `InputDeviceSettingHooks`, `WordVectorHooks`,
  `AwardTableHooks`, `ObjectArrayHooks`). Reconstructed callees are called directly:
  `reset_singleton_base`, `unregister_pair_006952a0`, `register_observer_pair_00694a60`,
  `erase_native_award_registry_subtree_006b9120`, `compare_insensitive_00438e10`,
  `resize_native_string_header_0041dd40`, `NativeStringStorage::release` (00419CC0 + 00BD1510).
- "placement-new null test dropped": MSVC emits `TEST ECX,ECX / JZ` before constructing into a
  freshly computed slot; the slot is never null, and the C++ omits the test.

## Functions
| Address | Ghidra name | C++ | Original ABI | Behaviour (evidence) | Simplifications |
|---|---|---|---|---|---|
| 006B7E30 | BSP_LuaStateHolder_Construct | `LuaMachine::LuaMachine(lua_State*)` | __thiscall(L) RET 4, EAX this | stores vftable 00CF8394, L at +4h, owns byte +8h = 0 | vftable is the compiler-generated one of the C++ class (one virtual destructor slot, like 00CF8394) |
| 006B7E50 | BSP_LuaStateHolder_Destruct | `LuaMachine::~LuaMachine` | __thiscall RET (tail JMP lua_close) | restores vftable 00CF8394, lua_close(L) when +8h is non-zero | none |
| 006B87C0 | CG_scalar_deleting_dtor_006b87c0 | `LuaMachine::~LuaMachine (deleting form)` | __thiscall(flags) RET 4, EAX this | same body as 006B7E50 inlined, then _free 00BF65AC when flags & 1 | compiler-generated from the virtual destructor; delete uses the host operator delete |
| 006B8740 | BSP_LuaMachine_Construct | `LuaMachine::LuaMachine()` | __thiscall RET, EAX this | luaL_newstate 00A6A260; lua_atpanic(006B8720) 00A67390; lua_gc(6 SETPAUSE, 100) 00A68280; for each {name, open} of 00CF8350 until a null open: lua_pushcclosure(open, 0), lua_pushstring(name), lua_call(1, 0); owns = 1 | library table spelled as luaopen_base/table/io/os/string/math/debug of the stock build (addresses 00A67210..00A61620 in include/bsp/mission_lua_host.hpp) |
| 006B8720 | (no Ghidra function; inside the gap after 006B8700) | `lua_machine_panic_006b8720` | lua_CFunction, L in ECX, RET | lua_tolstring(L, lua_gettop(L), NULL) with the result discarded, return 0 (bytes 006B8720..006B8736) | fragment: Ghidra has no function here |
| 006B7E70 | BSP_LuaMachine_GetTop | `LuaMachine::get_top` | __thiscall RET (tail JMP 00A673D0) | lua_gettop(L) | none |
| 006B7E80 | BSP_LuaMachine_SetTop | `LuaMachine::set_top` | __thiscall(index) RET 4 | lua_settop(L, index) 00A673E0 | none |
| 006B7E90 | BSP_LuaMachine_PushValue | `LuaMachine::push_value` | __thiscall(index) RET 4 | lua_pushvalue 00A67570 | none |
| 006B7EA0 | BSP_LuaMachine_Remove | `LuaMachine::remove` | __thiscall(index) RET 4 | lua_remove 00A67430 | none |
| 006B7EC0 | BSP_LuaStack_Type | `LuaMachine::type` | __thiscall(index) RET 4, EAX type | lua_type 00A675A0 | none |
| 006B7F60 | BSP_LuaStack_IsString | `LuaMachine::is_string_type` | __thiscall(index) RET 4, EAX 0/1 | lua_type 00A675A0 == 4 (CMP EAX,4 / SETZ): a number is not a string here, unlike lua_isstring | none |
| 006B8000 | BSP_LuaStack_ToBoolean | `LuaMachine::to_boolean` | __thiscall(index) RET 4, AL | lua_toboolean 00A677E0 != 0 | none |
| 006B8020 | BSP_LuaStack_ToFloat | `LuaMachine::to_float` | __thiscall(index) RET 4, ST0 float | lua_tonumber 00A67770, then FSTP float / FLD float: rounded to single precision | none |
| 006B8060 | BSP_LuaStack_ToString | `LuaMachine::to_string` | __thiscall(index) RET 4, EAX | lua_tolstring(L, index, NULL) 00A67810 | none |
| 006B80A0 | BSP_LuaStack_ToUserdata | `LuaMachine::to_userdata` | __thiscall(index) RET 4, EAX | lua_touserdata 00A67910 | none |
| 006B80C0 | BSP_LuaMachine_PushBoolean | `LuaMachine::push_boolean` | __thiscall(byte) RET 4 | lua_pushboolean(L, byte != 0) 00A67BC0 | none |
| 006B80E0 | BSP_LuaMachine_PushNumber | `LuaMachine::push_number` | __thiscall(float) RET 4 | FLD float, FSTP double; lua_pushnumber 00A679D0 | none |
| 006B8120 | BSP_LuaMachine_PushString | `LuaMachine::push_string` | __thiscall(text) RET 4 | lua_pushstring 00A67A50 | none |
| 006B8130 | BSP_LuaMachine_PushNil | `LuaMachine::push_nil` | __thiscall RET (tail JMP 00A679C0) | lua_pushnil | none |
| 006B8260 | BSP_LuaTable_SetStringNumber | `LuaMachine::set_string_number` | __thiscall(key, float) RET 8 | lua_pushstring(key); lua_pushnumber((double)value); lua_settable(-3) 00A67E10 | none |
| 006B82A0 | BSP_LuaTable_SetNumberNumber | `LuaMachine::set_number_number` | __thiscall(float key, float value) RET 8 | lua_pushnumber(key); lua_pushnumber(value); lua_settable(-3) | none |
| 006B8320 | BSP_LuaTable_SetNumberString | `LuaMachine::set_number_string` | __thiscall(float key, text) RET 8 | lua_pushnumber(key); lua_pushstring(value); lua_settable(-3) | none |
| 006B8360 | BSP_LuaTable_SetStringString | `LuaMachine::set_string_string` | __thiscall(key, text) RET 8 | lua_pushstring(key); lua_pushstring(value); lua_settable(-3) | none |
| 006B8390 | BSP_LuaInstance_SetFieldNil | `LuaMachine::set_field_nil` | __thiscall(key) RET 4 | lua_pushstring(key); lua_pushnil 00A679C0; lua_settable(-3) | none |
| 006B8460 | BSP_LuaMachine_GetGlobal | `LuaMachine::get_global` | __thiscall(name) RET 4 (tail JMP) | lua_getfield(L, -10002 LUA_GLOBALSINDEX, name) 00A67C40 | none |
| 006B8470 | BSP_LuaMachine_GetTable | `LuaMachine::get_table` | __thiscall(index) RET 4 | lua_gettable 00A67C20 | none |
| 006B84B0 | BSP_LuaMachine_CreateTable | `LuaMachine::create_table` | __thiscall RET | lua_createtable(L, 0, 0) 00A67D10 | none |
| 006B84D0 | BSP_LuaInstance_SetTable | `LuaMachine::set_table` | __thiscall(index) RET 4 | lua_settable 00A67E10 | none |
| 006B8530 | BSP_LuaMachine_ProtectedCall | `LuaMachine::protected_call` | __thiscall(nargs, nresults, errfunc) RET 0Ch, EAX | lua_pcall 00A680E0 (nargs in EDX, nresults and errfunc on the stack) | none |
| 006B85B0 | BSP_LuaStack_Next | `LuaMachine::next` | __thiscall(index) RET 4, EAX | lua_next 00A683A0 | none |
| 006B85F0 | BSP_LuaMachine_Pop | `LuaMachine::pop` | __thiscall(n) RET 4 | lua_settop(L, -1 - n) (OR EDX,-1 / SUB EDX,n) | none |
| 006B8610 | BSP_LuaMachine_RegisterGlobalFunction | `LuaMachine::register_global_function` | __thiscall(name, fn) RET 8 | lua_pushcclosure(fn, 0) 00A67B20; lua_setfield(-10002, name) 00A67E40 | none |
| 006B86B0 | BSP_LuaStack_GetCallerInfo | `LuaMachine::get_caller_info` | __thiscall(level, lua_Debug*) RET 8, EAX | lua_getstack 00A6AB10; zero returns it; else lua_getinfo(L, "nSlu" 00CF8398, ar) 00A6B840 | none |
| 006B87F0 | BSP_LuaVariantVector_Reserve | `lua_variant_vector_reserve_006b87f0` | __thiscall(count) RET 4 | count < 1 -> 1; capacity >= count returns; operator new 00BF55BE count*14h; copy-construct each element 00886DA0; destroy each old element by vtable slot 0 with flags 0; _free 00BF6989 the old data (no null test); data, capacity = new | element bodies are hooks; the SEH unwind that frees the new buffer when a copy throws is not reproduced; the placement-new null test is dropped |
| 006B88E0 | BSP_LuaVariantVector_Resize | `lua_variant_vector_resize_006b88e0` | __thiscall(count) RET 4 | count > capacity -> Reserve; default-construct 008849B0 indices size..count-1; then while count < size: --size and destroy that element by vtable slot 0; size = count | element bodies are hooks; placement-new null test dropped |
| 006B89A0 | BSP_LuaVariantVectorHolder_Clear | `lua_result_holder_clear_006b89a0` | __thiscall RET | when holder+4h is non-zero: Resize(holder+8h, 0), holder+4h = 0 | none |
| 006B89C0 | BSP_LuaVariantVectorHolder_Reset | `lua_result_holder_reset_006b89c0` | __thiscall(owner) RET 4 | when holder+4h is non-zero: Resize(+8h, 0) and +4h = 0; then +4h = owner | none |
| 006B89F0 | BSP_LuaMachine_RunChunk | `lua_machine_run_chunk_006b89f0` | __thiscall(buffer, size, name, holder, error NativeString*, depth) RET 18h, EAX 0/1 | base = lua_gettop; failed = luaL_loadbuffer 00A6A160 != 0, else lua_pcall(0, -1, 0) != 0; if gettop <= base return failed; on failure lua_tolstring(-1, NULL) and, with an error string, 0041E350 assign, then settop(base); on success with a holder 006B89C0(holder, this) and 00887220(holder, top - base, depth), then settop(base) | 00887220 and 0041E350 are hooks |
| 006964B0 | BSP_FrontEnd_ReadActions_4C_4D_4A | `front_end_read_actions_4c_4d_4a_006964b0` | __fastcall(ECX out0, EDX out1, stack out2) RET 4 | ECX = the front-end singleton (global 00E188A8); three action reads through 004D92B0 for actions 4Ch, 4Dh, 4Ah, each AL stored to its byte | 004D92B0 is a hook |
| 006964F0 | BSP_FrontEnd_ReadActions_F3_F4_4A | `front_end_read_actions_f3_f4_4a_006964f0` | __fastcall(ECX out0, EDX out1, stack out2) RET 4 | actions F3h, F4h, 4Ah | 004D92B0 is a hook |
| 00696540 | BSP_FrontEnd_ReadActions_F1_F2_4A | `front_end_read_actions_f1_f2_4a_00696540` | __fastcall(ECX out0, EDX out1, stack out2) RET 4 | actions F1h, F2h, 4Ah | 004D92B0 is a hook |
| 00699B80 | BSP_InputConfiguration_SetDeviceFlags | `input_configuration_set_device_flags_00699b80` | __thiscall(four bytes) RET 10h | when any of +4C9h..+4CCh differs: store all four; then, when +520h is non-zero, 00698730 Clear and 00698A10 Load | Clear and Load are hooks |
| 0069E500 | BSP_InputBinding_GetDisplayName | `input_binding_display_name_0069e500` | __thiscall(ignored ECX, NativeString* out, event*) RET 8 | kind (+0h) must be 2; 004BA6D0([00F8BBF4], 2, +4h) twice, the second result used; null -> 0041E350(out, ""); else device vtable +3Ch(&tmp, +0Ch), 00425F40(out, result), release tmp | device calls and the assigns are hooks |
| 006A9E70 | BSP_InputConfiguration_SetDeviceAxisValue | `input_configuration_set_device_axis_value_006a9e70` | __thiscall(key*, name, float) RET 0Ch | 0055C110(this+8h, key) -> record; 00444BE0(record+24h, name) -> float*; MOVSS store of the value | both map lookups are hooks |
| 00697080 | STL_CopyDwords_00697080 | `copy_dwords_00697080` | __cdecl(first, last, dest), EAX dest + n | dword copy loop; the result is computed before the loop | none |
| 00697170 | BSP_NativeDwordVector_PushBack_00697170 | `push_back_context_word_00697170` | __thiscall(const DWORD*) RET 4 | when count == capacity: 00696D80(doubled capacity, at least 1); store *value at data[count]; ++count | placement-new null test dropped |
| 006972D0 | BSP_NativeDwordVector_PushBackRet_006972d0 | `push_back_context_word_returning_006972d0` | __thiscall(const DWORD*) RET 4, EAX this | identical body to 00697170, returns the header | placement-new null test dropped |
| 006971C0 | BSP_NativeInputModifierVector_PushBack | `push_back_input_modifier_006971c0` | __thiscall(const record*) RET 4 | when count == capacity: 00696DE0(doubled); five dword copy into data + count*14h; ++count | placement-new null test dropped |
| 006972B0 | BSP_NativeDwordVector_Destroy_006972b0 | `destroy_context_words_006972b0` | __thiscall RET | 00696E70(0); _free 00BF6989 of data (header not cleared) | none |
| 006977D0 | BSP_NativeInputModifierVector_Destroy | `destroy_input_modifiers_006977d0` | __thiscall RET | 00697220(0); _free of data (header not cleared) | none |
| 00697D40 | STL_ConstructWordVector_00697d40 | `construct_word_vector_00697d40` | __fastcall(ECX dest, EDX src) RET | when dest is non-null: 00557590(dest, src) copy constructor | 00557590 is a hook |
| 00697EC0 | STL_ConstructWordVector_00697ec0 | `construct_word_vector_00697ec0` | __stdcall(dest, src) RET 8 | when dest is non-null: 00557590(dest, src) | 00557590 is a hook |
| 0069B6F0 | STL_Fill20_0069b6f0 | `fill_records20` | __cdecl(first, last, const value*) | five dword stores per 14h element | EAX residue (last) not modelled |
| 006BBBD0 | STL_Fill20_006bbbd0 | `fill_records20` | __cdecl(first, last, const value*) | identical instantiation of 0069B6F0 | EAX residue not modelled |
| 006BBCA0 | STL_Fill20_006bbca0 | `fill_records20` | __cdecl(first, last, const value*) | identical instantiation of 0069B6F0 | EAX residue not modelled |
| 0069B790 | STL_Copy20_0069b790 | `copy_records20` | __cdecl(first, last, dest), EAX dest end | forward copy of 14h elements | none |
| 006BBDE0 | STL_Copy20_006bbde0 | `copy_records20` | __cdecl(first, last, dest), EAX dest end | identical instantiation of 0069B790 | none |
| 006BBE70 | STL_Copy20_006bbe70 | `copy_records20` | __cdecl(first, last, dest), EAX dest end | identical instantiation of 0069B790 | none |
| 0069B810 | STL_CopyBackward20_0069b810 | `copy_backward_records20` | __cdecl(first, last, dest_end), EAX dest begin | backward copy of 14h elements | none |
| 006BBF80 | STL_CopyBackward20_006bbf80 | `copy_backward_records20` | __cdecl(first, last, dest_end), EAX dest begin | identical instantiation of 0069B810 | none |
| 006BBFF0 | STL_CopyBackward20_006bbff0 | `copy_backward_records20` | __cdecl(first, last, dest_end), EAX dest begin | identical instantiation of 0069B810 | none |
| 0069D610 | STL_UninitFillN20_0069d610 | `uninitialized_fill_n_records20_0069d610` | __fastcall(ECX dest, EDX count, stack value*, three unused) RET 10h | count (unsigned) copies of the 14h value | placement-new null test dropped |
| 0069D650 | STL_EqualDwords_0069d650 | `equal_dwords_0069d650` | __cdecl(first, last, first2), AL | stops at the first unequal dword; AL = reached last (Ghidra listing mis-decodes 0069D65C; read with disasm-raw) | none |
| 006BBB00 | BSP_NativeVector20_Grow_006bbb00 | `grow_records20_006bbb00` | __thiscall RET | capacity = 2*capacity + 2 stored first; operator new 00BF55BE capacity*14h; copy count (unsigned) records; _free old data when non-null; data = new | placement-new null test dropped |
| 0069BA20 | BSP_SingletonBase_Destruct_00e198e8 | `reset_singleton_00e198e8_0069ba20` | __fastcall(ECX) RET | [00E198E8] = 0, then vftable 00CE3818 | uses reset_singleton_base (engine_services_cc13.cpp); the published word is passed in |
| 006AD3F0 | BSP_SingletonBase_Destruct_00e198f0 | `reset_singleton_00e198f0_006ad3f0` | __fastcall(ECX) RET | [00E198F0] = 0, then vftable 00CE3818 | uses reset_singleton_base; the published word is passed in |
| 0069E090 | BSP_NativeString_ReleaseStorage_0069e090 | `release_pooled_string_at0_0069e090` | __thiscall RET | string {len +0, chars +4}: when chars is non-null, 00419CC0 then 00BD1510(chars, len + 1, 1) | through NativeStringStorage::release(chars, len + 1) |
| 0069E0B0 | BSP_NativeString_ReleaseStorage_0069e0b0 | `release_pooled_string_at4_0069e0b0` | __thiscall RET | the same on the string at +4h (len +4h, chars +8h) | through NativeStringStorage::release |
| 006AC330 | BSP_ObserverMember_SetTarget_006ac330 | `observer_member_set_target_006ac330` | __thiscall(target) RET 4, EAX this | when target differs from +14h: old non-null -> 006952A0(old, this); +14h = target; target non-null -> 00694A60(target, this) | calls the reconstructed observer functions directly |
| 006B91C0 | BSP_AwardRegistry_Clear | `award_registry_clear_006b91c0` | __thiscall RET | 006B9120(tree, head->parent); head->parent = head; size = 0; head->left = head; head->right = head (head reloaded each time) | calls the reconstructed 006B9120 with its lifetime context |
| 006B8CC0 | STL_List_Begin_006b8cc0 | `list_begin_at10_006b8cc0` | __thiscall(out) RET 4, EAX out | out+4h = *(this+14h) (head->next), then out+0h = this+10h | none |
| 006BA6A0 | BSP_CString_EqualsInsensitive_006ba6a0 | `is_launch_airstrike_006ba6a0` | __thiscall(ignored ECX, name, unused) RET 8, AL | 00438E10(name, "launchairstrike" 00CF85E0) == 0 | none |
| 006BB4F0 | BSP_ObjectArray58_Destroy_006bb4f0 | `destroy_object_array58_006bb4f0` | __thiscall RET | when data is non-null: vtable slot 0 (flags 0) on each 58h element for i < count (unsigned), then _free data; header left as is | the element destructor is a hook |
| 006AF3D0 | BSP_EntityNetworkId_FromNameStub | `network_id_from_name_stub_006af3d0` | __thiscall(ignored ECX, NativeString by value) RET 8, AX | releases the by-value string (00419CC0 / 00BD1510 when chars non-null), returns AX = 0 | upper EAX not modelled |
| 006B9FD0 | BSP_AwardTable_CopyRecordBuffer00 | `award_table_copy_text_006b9fd0` | __thiscall(out, key) RET 8, EAX out | 0050FC30(this+10h, key) -> record; out = {0,0}; unless out == record: 0041DD40(record len, 1), then when record len != 0 _memcpy(out chars, record chars, out len) | 0050FC30 is a hook; 0041DD40 is the reconstructed resize |
| 006B9F70 | BSP_AwardTable_CopyRecordBuffer14 | `award_table_copy_icon_006b9f70` | __thiscall(out, key) RET 8, EAX out | the same copy from record+14h | 0050FC30 is a hook |

## Not reconstructed here

- Belongs to cc9 (gameplay hosts): the air-operations block 006BA670..006BDBF0
  (BSP_AirOps*, BSP_AirOpsHolder*, BSP_AirOpsSlot*, the AIRBASE session messages, 006BDBF0) and the
  landing-pad / landing-ship helpers 006AC260, 006AC370, 006AC490, 006AC4D0, 006AC550, 006ACB40,
  006ACD80, 006ACDC0. 006AC490 already has a host model in `include/bsp/building_pads.hpp`
  (`set_occupant_006ac490`).
- Skipped, too large or needing un-reconstructed LuaObject plumbing: 006B9450
  BSP_AchievementTable_Load (818 listing lines), 006AC030 BSP_InputSettings_LoadForCurrentProfile
  (LuaObject chain B67980/B67800/B662B0/B68100), 006AF780 BSP_SceneEntity_RestoreFromSaveData
  (property-reader frame; see `include/bsp/save_settings_cc13.hpp`), 006AD600.
- Skipped, STL tree and nested-vector instantiations whose node and element layouts are not
  established (0069C570..006A7960: STL_Tree_*, STL_*StringVector*, STL_DwordVectorVector_*,
  the WordVector16 helpers 00697990..00698400).
