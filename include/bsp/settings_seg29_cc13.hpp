#pragma once
// Segment 29 (006964B0..006BE730), packet cc13-seg29: the mission Lua machine wrapper, its
// result-variant vector, the input-configuration vector helpers, the front-end action readers,
// and small record helpers. Evidence and per-function ABI: docs/CC13_SEG29.md. Names are
// hypotheses, not recovered symbols.
//
// The image's Lua 5.1.1 is built with a register convention (lua_State* in ECX, first integer
// argument in EDX, the rest on the stack). This file calls the stock 5.1.1 library linked as
// bsp_lua511 with its own C convention; the argument values are the image's.

#include <cstddef>
#include <cstdint>

struct lua_State;
struct lua_Debug;

namespace bsp {
class NativeStringStorage;
struct NativeObserverOwnerStorage;
class NativeObserverLifetime;
struct NativeGameProfileLifetimeContext;
struct NativeAwardRegistryLifetimeProgress;
} // namespace bsp

namespace bsp::cc13_seg29 {

// ---------------------------------------------------------------------------------------------
// LuaMachine: 0Ch bytes {vftable 00CF8394, lua_State* +4h, owns-state byte +8h}. The vtable has
// one slot, the scalar deleting destructor 006B87C0 (the next dword 00CF8398 is the "nSlu" string).
// Every method is __thiscall on the machine and reads L from +4h.
// ---------------------------------------------------------------------------------------------
class LuaMachine {
public:
    // 006B7E30, __thiscall(L) RET 4: vftable, L, owns = 0. Wraps a state it does not own.
    explicit LuaMachine(lua_State* state) noexcept;
    // 006B8740, __thiscall() RET: luaL_newstate, lua_atpanic(006B8720), lua_gc(SETPAUSE, 100),
    // the seven standard libraries from the pair table 00CF8350, then owns = 1.
    LuaMachine();
    // 006B7E50 (non-deleting) and 006B87C0 (scalar deleting, frees with _free 00BF65AC when
    // flags & 1): vftable, then lua_close when owns-state is set.
    virtual ~LuaMachine();
    LuaMachine(const LuaMachine&) = delete;
    LuaMachine& operator=(const LuaMachine&) = delete;

    lua_State* state() const noexcept { return state_; }

    int get_top();                                            // 006B7E70
    void set_top(int index);                                  // 006B7E80
    void push_value(int index);                               // 006B7E90
    void remove(int index);                                   // 006B7EA0
    int type(int index);                                      // 006B7EC0
    bool is_string_type(int index);                           // 006B7F60 (lua_type == 4 only)
    bool to_boolean(int index);                               // 006B8000
    float to_float(int index);                                // 006B8020
    const char* to_string(int index);                         // 006B8060
    void* to_userdata(int index);                             // 006B80A0
    void push_boolean(bool value);                            // 006B80C0
    void push_number(float value);                            // 006B80E0
    void push_string(const char* text);                       // 006B8120
    void push_nil();                                          // 006B8130
    void set_string_number(const char* key, float value);     // 006B8260
    void set_number_number(float key, float value);           // 006B82A0
    void set_number_string(float key, const char* value);     // 006B8320
    void set_string_string(const char* key, const char* value); // 006B8360
    void set_field_nil(const char* key);                      // 006B8390
    void get_global(const char* name);                        // 006B8460
    void get_table(int index);                                // 006B8470
    void create_table();                                      // 006B84B0
    void set_table(int index);                                // 006B84D0
    int protected_call(int nargs, int nresults, int errfunc); // 006B8530
    int next(int index);                                      // 006B85B0
    void pop(int count);                                      // 006B85F0
    void register_global_function(const char* name, int (*function)(lua_State*)); // 006B8610
    int get_caller_info(int level, lua_Debug* record);        // 006B86B0

private:
    lua_State* state_;
    bool owns_state_;
};
#if defined(_M_IX86)
static_assert(sizeof(LuaMachine) == 0x0C, "LuaMachine is 0Ch bytes in the image");
#endif

// 006B8720, the panic function 006B8740 installs: lua_tolstring(L, lua_gettop(L), NULL), the
// result discarded, then return 0.
int lua_machine_panic_006b8720(lua_State* state);

// ---------------------------------------------------------------------------------------------
// The result-variant vector {data +0, size +4, capacity +8} of 14h-byte variant records, and the
// holder {+0 not touched here, owner LuaMachine* +4, vector +8} that 006B89F0 fills.
// The element bodies live outside this segment: they are host hooks, called where the image
// calls them.
// ---------------------------------------------------------------------------------------------
inline constexpr std::uint32_t kLuaVariantStride = 0x14;
struct LuaVariantVector {
    std::uint8_t* data;
    std::int32_t size;
    std::int32_t capacity;
};
struct LuaResultHolder {
    std::uint32_t word_00; // not read or written by the functions here
    LuaMachine* owner;
    LuaVariantVector results;
};
#if defined(_M_IX86)
static_assert(sizeof(LuaResultHolder) == 0x14, "holder layout");
#endif

struct LuaVariantHooks {
    virtual ~LuaVariantHooks() = default;
    // 00886DA0 __thiscall(dst, src) RET 4: the variant copy constructor (vftable 00D0E6F4, tag -1,
    // +0Ch/+10h cleared, then the assign 00886C10).
    virtual void copy_construct_variant_00886da0(void* destination, const void* source) = 0;
    // 008849B0 __thiscall(dst): the default constructor (reconstructed as
    // initialize_native_mission_lua_variant_008849b0; the element's vtable is image code, so
    // the host owns the element representation).
    virtual void construct_variant_008849b0(void* destination) = 0;
    // CALL [[element]+0] with flags 0: the element's virtual destructor, no free.
    virtual void destroy_variant_virtual(void* element) = 0;
    // 00887220 __thiscall(holder, count, depth) RET 8: BSP_MissionLua_CollectStackResults.
    virtual void collect_stack_results_00887220(LuaResultHolder* holder, int count, int depth) = 0;
    // 0041E350 __thiscall(string, text) RET 4: BSP_NativeString_AssignCString.
    virtual void native_string_assign_0041e350(void* native_string, const char* text) = 0;
};

// 006B87F0, __thiscall(vector, count) RET 4.
void lua_variant_vector_reserve_006b87f0(LuaVariantVector& vector, std::int32_t count,
                                         LuaVariantHooks& hooks);
// 006B88E0, __thiscall(vector, count) RET 4.
void lua_variant_vector_resize_006b88e0(LuaVariantVector& vector, std::int32_t count,
                                        LuaVariantHooks& hooks);
// 006B89A0, __thiscall(holder) RET.
void lua_result_holder_clear_006b89a0(LuaResultHolder& holder, LuaVariantHooks& hooks);
// 006B89C0, __thiscall(holder, owner) RET 4.
void lua_result_holder_reset_006b89c0(LuaResultHolder& holder, LuaMachine* owner,
                                      LuaVariantHooks& hooks);
// 006B89F0, __thiscall(machine, buffer, size, chunk name, holder or null, error NativeString* or
// null, depth) RET 18h. Returns 1 when the load or the call failed, else 0.
int lua_machine_run_chunk_006b89f0(LuaMachine& machine, const char* buffer, std::size_t size,
                                   const char* chunk_name, LuaResultHolder* holder,
                                   void* error_string, int depth, LuaVariantHooks& hooks);

// ---------------------------------------------------------------------------------------------
// Input configuration and front end.
// ---------------------------------------------------------------------------------------------
struct FrontEndActionHost {
    virtual ~FrontEndActionHost() = default;
    // 004D92B0 BSP_FrontEnd_ActionEdgeOrRepeat, ECX = [00E188A8], RET 4, AL result.
    virtual bool action_edge_or_repeat_004d92b0(std::int32_t action) = 0;
};
// __fastcall(ECX out0, EDX out1, stack out2) RET 4: three action reads into three bytes.
void front_end_read_actions_4c_4d_4a_006964b0(std::uint8_t* out0, std::uint8_t* out1,
                                             std::uint8_t* out2, FrontEndActionHost& host);
void front_end_read_actions_f3_f4_4a_006964f0(std::uint8_t* out0, std::uint8_t* out1,
                                             std::uint8_t* out2, FrontEndActionHost& host);
void front_end_read_actions_f1_f2_4a_00696540(std::uint8_t* out0, std::uint8_t* out1,
                                             std::uint8_t* out2, FrontEndActionHost& host);

struct InputConfigurationHooks {
    virtual ~InputConfigurationHooks() = default;
    virtual void clear_00698730(void* configuration) = 0; // BSP_InputConfiguration_Clear
    virtual void load_00698a10(void* configuration) = 0;  // BSP_InputConfiguration_Load
};
// 00699B80, __thiscall(configuration, four bytes) RET 10h.
void input_configuration_set_device_flags_00699b80(void* configuration, std::uint8_t flag_4c9,
    std::uint8_t flag_4ca, std::uint8_t flag_4cb, std::uint8_t flag_4cc,
    InputConfigurationHooks& hooks);

// The input event the binding screens pass around: kind +0h, device index +4h, code +0Ch.
struct InputBindingNameHooks {
    virtual ~InputBindingNameHooks() = default;
    // 004BA6D0 BSP_InputBackend_GetDeviceByClassAndIndex, ECX = [00F8BBF4], (class, index) RET 8.
    virtual void* device_by_class_and_index_004ba6d0(std::int32_t device_class,
                                                     std::uint32_t index) = 0;
    // CALL [[device]+3Ch](&result, code): the device's control name, written into the hidden
    // result NativeString; returns the result pointer.
    virtual void* device_control_name_virtual_3c(void* device, void* result_string,
                                                 std::uint32_t code) = 0;
    // 00425F40 BSP_NativeString_AssignHeader, __thiscall(dst, const NativeString* src) RET 4.
    virtual void native_string_assign_header_00425f40(void* destination, const void* source) = 0;
    // 0041E350 BSP_NativeString_AssignCString, __thiscall(dst, text) RET 4.
    virtual void native_string_assign_0041e350(void* destination, const char* text) = 0;
};
// 0069E500, __thiscall(ignored ECX, NativeString* out, const event*) RET 8. Only kind 2 events
// are named; out is left untouched for other kinds or when the first device lookup fails.
void input_binding_display_name_0069e500(void* out, const void* event, InputBindingNameHooks& hooks,
                                         NativeStringStorage& strings);

struct InputDeviceSettingHooks {
    virtual ~InputDeviceSettingHooks() = default;
    // 0055C110 STL_InputDeviceMap_Index, __thiscall(map, const key*) RET 4: map[key].
    virtual void* input_device_map_index_0055c110(void* map, const void* key) = 0;
    // 00444BE0 BSP_WeatherFloatMap_LookupOrInsert, __thiscall(map, name) RET 4: float slot.
    virtual float* float_map_lookup_or_insert_00444be0(void* map, const void* name) = 0;
};
// 006A9E70, __thiscall(settings, const key*, name, float value) RET 0Ch:
// settings+8h [key] -> record, record+24h [name] = value (MOVSS).
void input_configuration_set_device_axis_value_006a9e70(void* settings, const void* key,
    const void* name, float value, InputDeviceSettingHooks& hooks);

// Count/capacity arrays {data +0, count +4, capacity +8} (native_input_binding_storage.cpp).
// 00697080, __cdecl(first, last, dest): copy dwords, returns dest + (last - first).
std::uint32_t* copy_dwords_00697080(const std::uint32_t* first, const std::uint32_t* last,
                                    std::uint32_t* destination) noexcept;
// 00697170, __thiscall(header, const DWORD*) RET 4.
void push_back_context_word_00697170(void* header, const std::uint32_t* value);
// 006972D0, the same body returning the header in EAX.
void* push_back_context_word_returning_006972d0(void* header, const std::uint32_t* value);
// 006971C0, __thiscall(header, const 14h-byte modifier*) RET 4.
void push_back_input_modifier_006971c0(void* header, const void* modifier);
// 006972B0 / 006977D0, __thiscall(header) RET: resize to 0, then _free the data pointer.
void destroy_context_words_006972b0(void* header) noexcept;
void destroy_input_modifiers_006977d0(void* header) noexcept;

struct WordVectorHooks {
    virtual ~WordVectorHooks() = default;
    // 00557590 STL_CheckedWordVector_CopyConstruct, __thiscall(dst, src) RET 4.
    virtual void copy_construct_word_vector_00557590(void* destination, const void* source) = 0;
};
// 00697D40, __fastcall(ECX destination, EDX source) RET and 00697EC0, __stdcall(destination,
// source) RET 8: placement copy-construct when the destination is non-null.
void construct_word_vector_00697d40(void* destination, const void* source, WordVectorHooks& hooks);
void construct_word_vector_00697ec0(void* destination, const void* source, WordVectorHooks& hooks);

// ---------------------------------------------------------------------------------------------
// 14h-byte trivially copied records: fill, copy and copy_backward, three identical
// instantiations each, all __cdecl(first, last, value-or-dest) RET.
// ---------------------------------------------------------------------------------------------
// 0069B6F0, 006BBBD0, 006BBCA0.
void fill_records20(void* first, void* last, const void* value) noexcept;
// 0069B790, 006BBDE0, 006BBE70: returns dest + (last - first).
void* copy_records20(const void* first, const void* last, void* destination) noexcept;
// 0069B810, 006BBF80, 006BBFF0: returns dest_end - (last - first).
void* copy_backward_records20(const void* first, const void* last, void* destination_end) noexcept;
// 0069D610, __fastcall(ECX dest, EDX count, stack value, three unused) RET 10h.
void uninitialized_fill_n_records20_0069d610(void* destination, std::uint32_t count,
                                             const void* value) noexcept;
// 0069D650, __cdecl(first, last, first2): AL = every dword equal.
bool equal_dwords_0069d650(const std::uint32_t* first, const std::uint32_t* last,
                           const std::uint32_t* first2) noexcept;
// 006BBB00, __thiscall(vector {data, size, capacity}) RET: capacity = 2 * capacity + 2,
// reallocate, copy the 14h-byte records, _free the old data when non-null.
void grow_records20_006bbb00(void* header);

// ---------------------------------------------------------------------------------------------
// Small owners.
// ---------------------------------------------------------------------------------------------
// 0069BA20 / 006AD3F0, __fastcall(ECX object) RET: clear the published singleton word
// (00E198E8 / 00E198F0), then store the singleton-base vftable 00CE3818.
void reset_singleton_00e198e8_0069ba20(void* object, std::uint32_t& published_00e198e8) noexcept;
void reset_singleton_00e198f0_006ad3f0(void* object, std::uint32_t& published_00e198f0) noexcept;

// 0069E090 / 0069E0B0, __thiscall(object) RET: release a pooled string {length, chars} at
// +0h / +4h when the chars pointer is non-null; the header is left as it was.
void release_pooled_string_at0_0069e090(void* object, NativeStringStorage& strings) noexcept;
void release_pooled_string_at4_0069e0b0(void* object, NativeStringStorage& strings) noexcept;

// 006AC330, __thiscall(member, target) RET 4, EAX member: retarget an observer member whose
// target owner lives at +14h.
void* observer_member_set_target_006ac330(void* member, NativeObserverOwnerStorage* target,
                                          NativeObserverLifetime& lifetime);

// 006B91C0, __thiscall(tree) RET: erase every node of the award registry tree, then reset the
// head links and the size.
void award_registry_clear_006b91c0(void* tree, NativeGameProfileLifetimeContext& context,
                                   NativeAwardRegistryLifetimeProgress& progress);

// 006B8CC0, __thiscall(list, out) RET 4, EAX out: out = {list + 10h, head->next} where the head
// pointer is at list + 14h.
void* list_begin_at10_006b8cc0(void* owner, void* out) noexcept;

// 006BA6A0, __thiscall(ignored ECX, name, unused) RET 8: case-insensitive "launchairstrike".
bool is_launch_airstrike_006ba6a0(const char* name);

// 006AF3D0, __thiscall(ignored ECX, NativeString by value {length, chars}) RET 8: releases the
// argument string and returns network id 0 in AX (upper EAX is not part of the result).
std::uint16_t network_id_from_name_stub_006af3d0(std::uint32_t length, char* chars,
                                                 NativeStringStorage& strings) noexcept;

// Award table text and icon: __thiscall(table, NativeString* out, key) RET 8, EAX out.
// record = 0050FC30(table + 10h, key); out is zeroed, then copied from record+0h (006B9FD0)
// or record+14h (006B9F70) unless out aliases the source.
struct AwardTableHooks {
    virtual ~AwardTableHooks() = default;
    // 0050FC30 BSP_AwardTable_FindRecord, __thiscall(map, key) RET 4.
    virtual void* find_award_record_0050fc30(void* map, const void* key) = 0;
};
void* award_table_copy_text_006b9fd0(void* table, void* out, const void* key, AwardTableHooks& hooks,
                                     NativeStringStorage& strings);
void* award_table_copy_icon_006b9f70(void* table, void* out, const void* key, AwardTableHooks& hooks,
                                     NativeStringStorage& strings);

struct ObjectArrayHooks {
    virtual ~ObjectArrayHooks() = default;
    // CALL [[element]+0] with flags 0 on a 58h-byte element.
    virtual void destroy_element_virtual(void* element) = 0;
};
// 006BB4F0, __thiscall(array {data +0, count +4}) RET: when data is non-null, destroy each of
// the count elements (unsigned compare), then _free the data. The header is not cleared.
void destroy_object_array58_006bb4f0(void* array, ObjectArrayHooks& hooks);

} // namespace bsp::cc13_seg29
