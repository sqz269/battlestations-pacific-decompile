#pragma once
// Packet cc13-pool12 (segment 29): the mission save-data Lua loader's four
// reference resolvers (function and entity references, keyed by string or by
// number), the float NativeString concatenation they share with the encoders,
// and the two small registries beside them (the entity name map built from
// BSP_Game_OnInit and the 00E198F0 Lua function name singleton). Names are
// hypotheses, not recovered symbols. Objects are raw image storage; every
// routine the image calls is a host method invoked at the same point. CRT
// (stricmp, atol, memcpy, operator new, free), kernel32 critical sections and
// the STL tree instantiations are bound with no credit. Build-only: nothing
// here is wired into a live host path.
#include <cstddef>
#include <cstdint>

namespace bsp::pool_pool12_cc13 {

// The engine's {length, data} string.
struct NativeStringImage {
    std::uint32_t length;
    char* data;
};
// A Lua object wrapper as the image passes it by value: 14h opaque bytes
// (BSP_LuaObject_*; 00B67700 destroys one).
struct LuaObjectImage {
    std::uint32_t words[5];
};
static_assert(sizeof(void*) != 4 || (sizeof(NativeStringImage) == 8 && sizeof(LuaObjectImage) == 0x14),
              "Win32 layout");

// A checked STL iterator as the image passes it: {container*, node*}.
struct CheckedIterImage {
    void* container;
    void* node;
};

struct Pool12Host {
    virtual ~Pool12Host() = default;

    // ---- image globals ----
    virtual std::uint32_t global_u32(std::uint32_t address) = 0;
    virtual void global_store_u32(std::uint32_t address, std::uint32_t value) = 0;

    // ---- library (bound) ----
    // 00BF7FBF _stricmp, cdecl -> EAX. 00BF8417 atol, cdecl -> EAX.
    virtual std::int32_t stricmp_00bf7fbf(const char* a, const char* b) = 0;
    virtual std::int32_t atol_00bf8417(const char* text) = 0;
    // 00BF7680 memcpy, cdecl(dst, src, count). 00BF681B operator new, cdecl(bytes).
    // 00BF65AC free, cdecl(block).
    virtual void crt_memcpy_00bf7680(void* destination, const void* source, std::uint32_t count) = 0;
    virtual void* operator_new_00bf681b(std::uint32_t bytes) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    // [00CE2218] EnterCriticalSection, [00CE2210] LeaveCriticalSection (__stdcall).
    virtual void enter_critical_section_00ce2218(void* section) = 0;
    virtual void leave_critical_section_00ce2210(void* section) = 0;
    // STL tree head-node buyers, __thiscall(ECX = tree), RET; EAX = node:
    // 005826B0 (the name map), 005FD1B0 (the function name map).
    virtual void* stl_tree_buy_head_node(std::uint32_t callee, void* tree) = 0;
    // STL tree erase-range, __thiscall(ECX = tree; iterator* out, CheckedIter first,
    // CheckedIter last), RET 14h: 0058D860, 005FEDC0.
    virtual void stl_tree_erase_range(std::uint32_t callee, void* tree, CheckedIterImage* out,
                                      CheckedIterImage first, CheckedIterImage last) = 0;

    // ---- engine strings ----
    // 00419CC0 BSP_SizedStoragePool_GetSingleton: no arguments; EAX.
    // 00BD1510 BSP_SizedStoragePool_ReturnBlock: __thiscall(ECX = pool; data, bytes, dword), RET 0Ch.
    virtual void* sized_pool_get_00419cc0() = 0;
    virtual void sized_pool_return_block_00bd1510(void* pool, void* data, std::uint32_t bytes,
                                                  std::int32_t flag) = 0;
    // 0041DD40 BSP_NativeString_Resize: __thiscall(ECX = string; length, dword), RET 8.
    virtual void native_string_resize_0041dd40(NativeStringImage* string, std::uint32_t length,
                                               std::int32_t arg) = 0;
    // 0041E870 BSP_NativeString_Assign: __thiscall(ECX = string; const char*), RET 4; EAX = string.
    virtual NativeStringImage* native_string_assign_0041e870(NativeStringImage* string, const char* text) = 0;
    // 00469840 BSP_NativeString_Substring: __thiscall(ECX = source; out*, position, count),
    // RET 0Ch; EAX = out.
    virtual NativeStringImage* native_string_substring_00469840(const NativeStringImage* source,
                                                                NativeStringImage* out, std::uint32_t position,
                                                                std::uint32_t count) = 0;
    // 0043BDF0 BSP_NativeString_ConstructFromFloat: __thiscall(ECX = out; float), RET 4; EAX = out.
    virtual NativeStringImage* native_string_from_float_0043bdf0(NativeStringImage* out, float value) = 0;

    // ---- Lua object wrappers (__thiscall, ECX = the object) ----
    // 00B68100 GetByNativeString(out*, const NativeString* key) RET 8; 00B67720
    // GetByIndex(out*, int) RET 8; 00B662B0 GetString() -> EAX; 00B67700 Destruct();
    // 00B67980 BSP_LuaStateOwner_GetGlobals(out*) RET 4 -> EAX = out (ECX = owner);
    // 00B675D0 SetObject(const NativeString* key, const LuaObject*) RET 8;
    // 00B666C0 SetObjectAtIndex(int, const LuaObject*) RET 8;
    // 00B67530 SetLightUserdata(const NativeString* key, void*) RET 8;
    // 00B66620 SetLightUserdataAtIndex(int, void*) RET 8.
    virtual LuaObjectImage* lua_get_by_string_00b68100(LuaObjectImage* table, LuaObjectImage* out,
                                                       const NativeStringImage* key) = 0;
    virtual LuaObjectImage* lua_get_by_index_00b67720(LuaObjectImage* table, LuaObjectImage* out,
                                                      std::int32_t index) = 0;
    virtual const char* lua_get_string_00b662b0(LuaObjectImage* object) = 0;
    virtual void lua_destruct_00b67700(LuaObjectImage* object) = 0;
    virtual LuaObjectImage* lua_state_owner_get_globals_00b67980(void* owner, LuaObjectImage* out) = 0;
    virtual void lua_set_object_00b675d0(LuaObjectImage* table, const NativeStringImage* key,
                                         const LuaObjectImage* value) = 0;
    virtual void lua_set_object_at_index_00b666c0(LuaObjectImage* table, std::int32_t index,
                                                  const LuaObjectImage* value) = 0;
    virtual void lua_set_light_userdata_00b67530(LuaObjectImage* table, const NativeStringImage* key,
                                                 void* pointer) = 0;
    virtual void lua_set_light_userdata_at_index_00b66620(LuaObjectImage* table, std::int32_t index,
                                                          void* pointer) = 0;

    // ---- singletons ----
    // 00415350 BSP_SingletonLifetime_GetManager: no arguments; EAX.
    // 00BD0C30: __thiscall(ECX = manager; instance), RET 4 (register for teardown).
    virtual void* singleton_lifetime_manager_00415350() = 0;
    virtual void singleton_lifetime_register_00bd0c30(void* manager, void* instance) = 0;
};

// 006AE120 __stdcall(LuaObject parent, NativeString key) RET 1Ch, callee-destroyed arguments.
void mission_load_resolve_function_ref_by_string_key_006ae120(LuaObjectImage* parent, NativeStringImage* key,
                                                              Pool12Host& host);
// 006AE280 __stdcall(LuaObject parent, NativeString key) RET 1Ch.
void mission_load_resolve_entity_ref_by_string_key_006ae280(LuaObjectImage* parent, NativeStringImage* key,
                                                            Pool12Host& host);
// 006AE730 __stdcall(LuaObject parent, float key) RET 18h.
void mission_load_resolve_function_ref_by_number_key_006ae730(LuaObjectImage* parent, float key,
                                                              Pool12Host& host);
// 006AE870 __stdcall(LuaObject parent, float key) RET 18h.
void mission_load_resolve_entity_ref_by_number_key_006ae870(LuaObjectImage* parent, float key,
                                                            Pool12Host& host);
// 006AF2B0 __thiscall(ECX = left; NativeString* out, float) RET 8; EAX = out.
NativeStringImage* native_string_concat_float_006af2b0(const NativeStringImage* left, NativeStringImage* out,
                                                       float value, Pool12Host& host);
// 006AFE80 __thiscall(ECX = map), RET; EAX = map.
void* mission_entity_name_map_construct_006afe80(void* map, Pool12Host& host);
// 006AFEC0 __thiscall(ECX = map; flags), RET 4; EAX = map.
void* mission_entity_name_map_scalar_deleting_dtor_006afec0(void* map, std::uint32_t flags, Pool12Host& host);
// 006AFF20 __thiscall(ECX = names), RET; EAX = names.
void* mission_lua_function_names_construct_006aff20(void* names, Pool12Host& host);
// 006AFF90 __thiscall(ECX = names), RET.
void mission_lua_function_names_destruct_006aff90(void* names, Pool12Host& host);
// 006B17B0 no arguments, RET; EAX = [00E198F0].
void* mission_lua_function_names_get_006b17b0(Pool12Host& host);

}  // namespace bsp::pool_pool12_cc13
