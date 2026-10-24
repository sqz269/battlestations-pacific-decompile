// Packet cc13-pool12, segment 29 (see include/bsp/pool_pool12_cc13.hpp). Build-only.
#include "bsp/pool_pool12_cc13.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error cc13-pool12 reconstruction reads Win32 image storage.
#endif

namespace bsp::pool_pool12_cc13 {

namespace {

template <class T>
T& at(void* object, std::size_t off) noexcept {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(object) + off);
}

// The inlined NativeString release: data non-null -> the pool takes back
// length + 1 bytes with flag 1 (00419CC0 then 00BD1510).
void release_native_string(NativeStringImage& string, Pool12Host& host) {
    if (string.data != nullptr) {
        void* pool = host.sized_pool_get_00419cc0();
        host.sized_pool_return_block_00bd1510(pool, string.data, string.length + 1, 1);
    }
}

// The shared front of the four resolvers: the stored text is "<3-char tag><payload>"; the
// value's string is copied into a NativeString (0041E870), the payload taken by
// 00469840(3, 7FFFFFFFh) and the copy released. `value` stays live for the caller.
NativeStringImage payload_of(LuaObjectImage& value, Pool12Host& host) {
    NativeStringImage text{0, nullptr};
    NativeStringImage payload{0, nullptr};
    const char* source = host.lua_get_string_00b662b0(&value);
    NativeStringImage* const copy = host.native_string_assign_0041e870(&text, source);
    host.native_string_substring_00469840(copy, &payload, 3, 0x7FFFFFFFu);
    release_native_string(text, host);
    return payload;
}

// globals[payload] (the Lua state owner at [00E188A8]+1A0Ch), the globals
// object destroyed before the result is used.
LuaObjectImage global_named(const NativeStringImage& name, Pool12Host& host) {
    LuaObjectImage globals{};
    LuaObjectImage function{};
    void* const owner = reinterpret_cast<void*>(host.global_u32(0x00E188A8u) + 0x1A0Cu);
    LuaObjectImage* const g = host.lua_state_owner_get_globals_00b67980(owner, &globals);
    host.lua_get_by_string_00b68100(g, &function, &name);
    host.lua_destruct_00b67700(&globals);
    return function;
}

// The entity a payload names: "x" (case-insensitive) is null; otherwise atol of
// the payload ("" at 00E198FC when the payload has no storage), its low word an
// entity handle id resolved through the two banks of 16-byte slots (entity at
// +0Ch): ids below [00F89A10] index [00F89A54] from [00F89A0C], the rest index
// [00F89AA8] from [00F89A60].
void* entity_of(const NativeStringImage& payload, Pool12Host& host) {
    const char* text = payload.data;
    if (text != nullptr && host.stricmp_00bf7fbf(text, "x") == 0) return nullptr;  // 00CEB488
    if (text == nullptr) text = "";                                                  // 00E198FC
    const std::int32_t id = static_cast<std::uint16_t>(host.atol_00bf8417(text));
    std::uint32_t slot;
    if (id < static_cast<std::int32_t>(host.global_u32(0x00F89A10u))) {
        slot = host.global_u32(0x00F89A54u) + (static_cast<std::uint32_t>(id) - host.global_u32(0x00F89A0Cu)) * 16u;
    } else {
        slot = host.global_u32(0x00F89AA8u) + (static_cast<std::uint32_t>(id) - host.global_u32(0x00F89A60u)) * 16u;
    }
    return *reinterpret_cast<void**>(slot + 0x0Cu);
}

}  // namespace

// ---------------------------------------------------------------------------
// 006AE120 (96 insns), BSP_MissionLoad_ResolveFunctionRefByStringKey. Only
// caller 006AEA00 BSP_MissionLoad_DecodeSlotByStringKey. __stdcall(LuaObject
// parent (14h bytes by value), NativeString key (by value)) RET 1Ch; ECX/EDX
// are not read; both arguments are destroyed here (callee-destroyed by-value
// arguments). Evidence: assembly 006AE120..006AE275.
// parent[key] = globals[parent[key]:GetString().substr(3)]: value V =
// 00B68100(parent, key); its string -> copy (0041E870) -> payload (00469840(3,
// 7FFFFFFFh)), copy released; globals via 00B67980 on [00E188A8]+1A0Ch; F =
// globals[payload] (00B68100), globals destroyed; 00B675D0(parent, key, F); F
// destroyed; payload released; V destroyed; parent destroyed; key released.
// EH states: 1 (entry, both arguments live), 2, 3, 5, 6, 8, 5, 2, 1, 0, -1.
void mission_load_resolve_function_ref_by_string_key_006ae120(LuaObjectImage* parent, NativeStringImage* key,
                                                              Pool12Host& host) {
    // EH state 1
    LuaObjectImage value{};
    host.lua_get_by_string_00b68100(parent, &value, key);             // 006AE14E
    // EH state 2 (3 while the copy is live, then 5)
    NativeStringImage payload = payload_of(value, host);             // 006AE15C..006AE1A2
    // EH state 6, then 8
    LuaObjectImage function = global_named(payload, host);           // 006AE1A7..006AE1DB
    host.lua_set_object_00b675d0(parent, key, &function);            // 006AE1EE
    // EH state 5
    host.lua_destruct_00b67700(&function);                           // 006AE1FC
    // EH state 2
    release_native_string(payload, host);                            // 006AE201
    // EH state 1
    host.lua_destruct_00b67700(&value);                              // 006AE22D
    // EH state 0
    host.lua_destruct_00b67700(parent);                              // 006AE23B
    // EH state -1
    release_native_string(*key, host);                               // 006AE240
}

// ---------------------------------------------------------------------------
// 006AE280 (109 insns), BSP_MissionLoad_ResolveEntityRefByStringKey. Only caller
// 006AEA00. __stdcall(LuaObject parent, NativeString key) RET 1Ch, both
// destroyed here. Evidence: assembly 006AE280..006AE3F6.
// value V = parent[key]; payload as in 006AE120; pointer = null for "x"
// (_stricmp with 00CEB488), else the entity of handle (u16)atol(payload)
// (bank split [00F89A10]); 00B67530(parent, key, pointer) stores it as light
// userdata; payload released; V, parent destroyed; key released.
// EH states: 1, 2, 3, 5, 2, 1, 0, -1. Uncertain: atol overflow and handles
// outside both banks are not checked by the image either.
void mission_load_resolve_entity_ref_by_string_key_006ae280(LuaObjectImage* parent, NativeStringImage* key,
                                                            Pool12Host& host) {
    // EH state 1
    LuaObjectImage value{};
    host.lua_get_by_string_00b68100(parent, &value, key);             // 006AE2AE
    // EH state 2 (3, then 5)
    NativeStringImage payload = payload_of(value, host);             // 006AE2BC..006AE302
    void* const entity = entity_of(payload, host);                   // 006AE307..006AE36F
    host.lua_set_light_userdata_00b67530(parent, key, entity);       // 006AE37D
    // EH state 2
    release_native_string(payload, host);                            // 006AE382
    // EH state 1
    host.lua_destruct_00b67700(&value);                              // 006AE3AE
    // EH state 0
    host.lua_destruct_00b67700(parent);                              // 006AE3BC
    // EH state -1
    release_native_string(*key, host);                               // 006AE3C1
}

// ---------------------------------------------------------------------------
// 006AE730 (85 insns), BSP_MissionLoad_ResolveFunctionRefByNumberKey. Only
// caller 006AF120 BSP_MissionLoad_DecodeSlotByNumberKey. __stdcall(LuaObject
// parent, float key) RET 18h; the parent is destroyed here. Evidence: assembly
// 006AE730..006AE862. The index is CVTTSS2SI (truncation) of the key; then as
// 006AE120 with 00B67720 GetByIndex and 00B666C0 SetObjectAtIndex.
// EH states: 0 (entry, the parent live), 1, 2, 4, 5, 7, 4, 1, 0, -1.
void mission_load_resolve_function_ref_by_number_key_006ae730(LuaObjectImage* parent, float key,
                                                              Pool12Host& host) {
    const std::int32_t index = static_cast<std::int32_t>(key);       // 006AE749 CVTTSS2SI
    // EH state 0
    LuaObjectImage value{};
    host.lua_get_by_index_00b67720(parent, &value, index);            // 006AE761
    // EH state 1 (2, then 4)
    NativeStringImage payload = payload_of(value, host);             // 006AE76F..006AE7B5
    // EH state 5, then 7
    LuaObjectImage function = global_named(payload, host);           // 006AE7BA..006AE7EF
    host.lua_set_object_at_index_00b666c0(parent, index, &function); // 006AE7FE
    // EH state 4
    host.lua_destruct_00b67700(&function);                           // 006AE80C
    // EH state 1
    release_native_string(payload, host);                            // 006AE811
    // EH state 0
    host.lua_destruct_00b67700(&value);                              // 006AE83E
    // EH state -1
    host.lua_destruct_00b67700(parent);                              // 006AE84F
}

// ---------------------------------------------------------------------------
// 006AE870 (98 insns), BSP_MissionLoad_ResolveEntityRefByNumberKey. Only caller
// 006AF120. __stdcall(LuaObject parent, float key) RET 18h; the parent is
// destroyed here. Evidence: assembly 006AE870..006AE9C2. As 006AE280 with the
// truncated index, 00B67720 and 00B66620 SetLightUserdataAtIndex.
// EH states: 0, 1, 2, 4, 1, 0, -1.
void mission_load_resolve_entity_ref_by_number_key_006ae870(LuaObjectImage* parent, float key,
                                                            Pool12Host& host) {
    const std::int32_t index = static_cast<std::int32_t>(key);       // 006AE889 CVTTSS2SI
    // EH state 0
    LuaObjectImage value{};
    host.lua_get_by_index_00b67720(parent, &value, index);            // 006AE8A1
    // EH state 1 (2, then 4)
    NativeStringImage payload = payload_of(value, host);             // 006AE8AF..006AE8F5
    void* const entity = entity_of(payload, host);                   // 006AE8FA..006AE962
    host.lua_set_light_userdata_at_index_00b66620(parent, index, entity);  // 006AE96C
    // EH state 1
    release_native_string(payload, host);                            // 006AE971
    // EH state 0
    host.lua_destruct_00b67700(&value);                              // 006AE99E
    // EH state -1
    host.lua_destruct_00b67700(parent);                              // 006AE9AF
}

// ---------------------------------------------------------------------------
// 006AF2B0 (82 insns), BSP_NativeString_ConcatFloat (operator+(string, float)).
// __thiscall(ECX = left; NativeString* out, float value) RET 8; EAX = out.
// Callers include the mission save encoders (006B1E40, 006B4CB0). Evidence:
// assembly 006AF2B0..006AF391.
// out = {0, 0}; unless out is left: resize out to left.length and, when left
// is non-empty, copy out.length bytes of left's data (no terminator byte);
// the float is passed through FLD/FSTP (an exact float copy) to 0043BDF0 into
// a temporary; when the temporary is non-empty, out grows by its length and
// its bytes are copied after the old length; the temporary is released.
// EH states: 0 (out live; the frame also sets a "returned object" flag to 1),
// 1 around the temporary, 0. Uncertain: the "%f" formatting lives in 0043BDF0.
NativeStringImage* native_string_concat_float_006af2b0(const NativeStringImage* left, NativeStringImage* out,
                                                       float value, Pool12Host& host) {
    out->length = 0;                                                  // 006AF2DE
    out->data = nullptr;
    // EH state 0
    if (out != left) {                                                // 006AF2D8
        host.native_string_resize_0041dd40(out, left->length, 1);    // 006AF2EC
        if (left->length != 0) host.crt_memcpy_00bf7680(out->data, left->data, out->length);  // 006AF300
    }
    NativeStringImage temporary;
    const NativeStringImage* const formatted = host.native_string_from_float_0043bdf0(&temporary, value);  // 006AF320
    // EH state 1
    const std::uint32_t extra = formatted->length;                   // 006AF327
    if (extra != 0) {
        const std::uint32_t old_length = out->length;                // 006AF335
        host.native_string_resize_0041dd40(out, extra + old_length, 1);
        host.crt_memcpy_00bf7680(out->data + old_length, formatted->data, extra);  // 006AF34F
    }
    // EH state 0
    release_native_string(temporary, host);                          // 006AF359
    return out;
}

// ---------------------------------------------------------------------------
// 006AFE80 (20 insns), BSP_MissionEntityNameMap_Construct. Only caller
// 004E3AA0 BSP_Game_OnInit. __thiscall(ECX = map), RET; EAX = map. No EH frame.
// Evidence: assembly 006AFE80..006AFEB7: vtable 00CF82BC at +0; the tree at
// +4h gets its head from 005826B0 (+8h), the head's isnil byte (+19h) = 1 and
// its left/parent/right (+0/+4/+8) = itself (the inlined STL tree init), size
// +0Ch = 0. Uncertain: the key/value types (save_settings_cc13.hpp reads it
// as a NativeString -> int map, consistent with the 0058D860 eraser).
void* mission_entity_name_map_construct_006afe80(void* map, Pool12Host& host) {
    void* const tree = static_cast<std::uint8_t*>(map) + 4;
    at<std::uint32_t>(map, 0) = 0x00CF82BCu;                         // 006AFE89
    void* const head = host.stl_tree_buy_head_node(0x005826B0u, tree);  // 006AFE8F
    at<void*>(tree, 4) = head;
    at<std::uint8_t>(head, 0x19) = 1;
    at<void*>(at<void*>(tree, 4), 4) = at<void*>(tree, 4);
    at<void*>(at<void*>(tree, 4), 0) = at<void*>(tree, 4);
    at<void*>(at<void*>(tree, 4), 8) = at<void*>(tree, 4);
    at<std::uint32_t>(tree, 8) = 0;                                  // 006AFEAF
    return map;
}

// ---------------------------------------------------------------------------
// 006AFEC0 (33 insns), BSP_MissionEntityNameMap_ScalarDeletingDestructor
// (vtable 00CF82BC slot 0). __thiscall(ECX = map; flags) RET 4; EAX = map. No
// EH frame. Evidence: assembly 006AFEC0..006AFF10: vtable re-stored; erase
// [begin, head) of the tree +4h with 0058D860; free the head; head and size
// zeroed; free(map) when bit 0 of flags is set.
void* mission_entity_name_map_scalar_deleting_dtor_006afec0(void* map, std::uint32_t flags, Pool12Host& host) {
    void* const tree = static_cast<std::uint8_t*>(map) + 4;
    at<std::uint32_t>(map, 0) = 0x00CF82BCu;                         // 006AFECA
    void* const head = at<void*>(tree, 4);
    CheckedIterImage out{};
    host.stl_tree_erase_range(0x0058D860u, tree, &out, CheckedIterImage{tree, at<void*>(head, 0)},
                              CheckedIterImage{tree, head});         // 006AFEE0
    host.free_00bf65ac(at<void*>(tree, 4));                          // 006AFEE9
    at<std::uint32_t>(tree, 4) = 0;
    at<std::uint32_t>(tree, 8) = 0;
    if ((flags & 1) != 0) host.free_00bf65ac(map);                   // 006AFF01
    return map;
}

// ---------------------------------------------------------------------------
// 006AFF20 (31 insns), BSP_MissionLuaFunctionNames_Construct. Only caller
// 006B17B0. __thiscall(ECX = names), RET; EAX = names. Evidence: assembly
// 006AFF20..006AFF87: EH state 0 (the singleton base, whose unwind is
// 006AD3F0) before vtable 00CF82E8; the tree +4h initialised as in 006AFE80
// with head buyer 005FD1B0; size +0Ch = 0. 10h bytes.
void* mission_lua_function_names_construct_006aff20(void* names, Pool12Host& host) {
    void* const tree = static_cast<std::uint8_t*>(names) + 4;
    // EH state 0
    at<std::uint32_t>(names, 0) = 0x00CF82E8u;                       // 006AFF4B
    void* const head = host.stl_tree_buy_head_node(0x005FD1B0u, tree);  // 006AFF51
    at<void*>(tree, 4) = head;
    at<std::uint8_t>(head, 0x19) = 1;
    at<void*>(at<void*>(tree, 4), 4) = at<void*>(tree, 4);
    at<void*>(at<void*>(tree, 4), 0) = at<void*>(tree, 4);
    at<void*>(at<void*>(tree, 4), 8) = at<void*>(tree, 4);
    at<std::uint32_t>(tree, 8) = 0;                                  // 006AFF74
    return names;
}

// ---------------------------------------------------------------------------
// 006AFF90 (39 insns), BSP_MissionLuaFunctionNames_Destruct. Only caller
// 006B0000 (its scalar deleting wrapper). __thiscall(ECX = names), RET.
// Evidence: assembly 006AFF90..006AFFFE: EH state 0; erase the tree +4h with
// 005FEDC0, free the head, zero head and size; [00E198F0] = 0 (the singleton
// slot); then the base's vtable 00CE3818 (the inlined base destructor).
void mission_lua_function_names_destruct_006aff90(void* names, Pool12Host& host) {
    void* const tree = static_cast<std::uint8_t*>(names) + 4;
    // EH state 0
    void* const head = at<void*>(names, 8);                          // 006AFFB1
    CheckedIterImage out{};
    host.stl_tree_erase_range(0x005FEDC0u, tree, &out, CheckedIterImage{tree, at<void*>(head, 0)},
                              CheckedIterImage{tree, head});         // 006AFFCA
    host.free_00bf65ac(at<void*>(tree, 4));                          // 006AFFD3
    at<std::uint32_t>(tree, 4) = 0;
    at<std::uint32_t>(tree, 8) = 0;
    host.global_store_u32(0x00E198F0u, 0);                           // 006AFFE5
    at<std::uint32_t>(names, 0) = 0x00CE3818u;                       // 006AFFEB
}

// ---------------------------------------------------------------------------
// 006B17B0 (51 insns), BSP_MissionLuaFunctionNames_Get. No arguments, RET; EAX
// = [00E198F0]. Evidence: assembly 006B17B0..006B186C. Double-checked
// singleton: when [00E198F0] is set it is returned at once. Otherwise a guard
// {vtable 00CE37FC, lock} is built on the stack with lock = [00415350()+10h]:
// when set, EnterCriticalSection(lock) and ++[lock+18h]. EH state 0. If
// [00E198F0] is still null: operator new(10h) (state 1), 006AFF20 on it or
// null, state 0, [00E198F0] = it, and 00BD0C30(00415350(), it) registers it.
// The guard's inlined destructor: when the lock is set, --[lock+18h] then
// LeaveCriticalSection. Returns [00E198F0] (re-read).
void* mission_lua_function_names_get_006b17b0(Pool12Host& host) {
    if (host.global_u32(0x00E198F0u) != 0) {                          // 006B17C5
        return reinterpret_cast<void*>(host.global_u32(0x00E198F0u));
    }
    void* const lock = at<void*>(host.singleton_lifetime_manager_00415350(), 0x10);  // 006B17D6
    if (lock != nullptr) {
        host.enter_critical_section_00ce2218(lock);                  // 006B17EF
        at<std::int32_t>(lock, 0x18) += 1;
    }
    // EH state 0
    if (host.global_u32(0x00E198F0u) == 0) {                          // 006B1801
        void* const block = host.operator_new_00bf681b(0x10);        // 006B180C
        // EH state 1
        void* const names = block != nullptr ? mission_lua_function_names_construct_006aff20(block, host) : nullptr;
        // EH state 0
        host.global_store_u32(0x00E198F0u, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(names)));
        void* const manager = host.singleton_lifetime_manager_00415350();  // 006B1836, before the re-read
        host.singleton_lifetime_register_00bd0c30(manager, reinterpret_cast<void*>(host.global_u32(0x00E198F0u)));  // 006B1844
    }
    if (lock != nullptr) {                                            // 006B1849
        at<std::int32_t>(lock, 0x18) += -1;
        host.leave_critical_section_00ce2210(lock);
    }
    return reinterpret_cast<void*>(host.global_u32(0x00E198F0u));    // 006B1858
}

}  // namespace bsp::pool_pool12_cc13
