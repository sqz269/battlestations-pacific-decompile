#include "bsp/settings_seg29_cc13.hpp"

#include "bsp/engine_services_cc13.hpp"
#include "bsp/entity_identity.hpp"
#include "bsp/native_award_registry_lifetime.hpp"
#include "bsp/native_input_binding_storage.hpp"
#include "bsp/native_string.hpp"
#include "bsp/observer_edges.hpp"
#include "bsp/observer_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

// Segment 29, packet cc13-seg29. Evidence, ABI and simplifications per function:
// docs/CC13_SEG29.md.
namespace bsp::cc13_seg29 {
namespace {

template <class T> T get(const void* p, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const unsigned char*>(p) + offset, sizeof value);
    return value;
}
template <class T> void put(void* p, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<unsigned char*>(p) + offset, &value, sizeof value);
}
unsigned char* at(void* p, std::size_t offset) noexcept {
    return static_cast<unsigned char*>(p) + offset;
}
void* allocate(std::uint32_t bytes) {
    // operator new 00BF55BE, the same boundary native_input_binding_storage.cpp uses.
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}
// ADD EAX,EAX / CMP EAX,1 / JG: the doubled capacity, or 1 when it is not above 1.
std::int32_t doubled(std::int32_t capacity) noexcept {
    const auto result = static_cast<std::int32_t>(static_cast<std::uint32_t>(capacity) * 2u);
    return result > 1 ? result : 1;
}
void copy_record20(void* destination, const void* source) noexcept {
    for (std::size_t word = 0; word < 0x14; word += 4)
        put<std::uint32_t>(destination, word, get<std::uint32_t>(source, word));
}
// Inlined NativeString release: when the chars pointer (+4h) is non-null, 00419CC0 then
// 00BD1510(chars, length + 1, 1); the third argument is unused. The header is not cleared.
void release_pooled_string(void* string, NativeStringStorage& strings) noexcept {
    char* const chars = get<char*>(string, 4);
    if (chars == nullptr) return;
    strings.release(chars, get<std::uint32_t>(string, 0) + 1);
}
// Inlined NativeString copy construction: zero the header, then unless it aliases the source,
// 0041DD40(source length, preserve 1) and, when the source length is non-zero, _memcpy of the
// destination's current length from the source data.
void copy_construct_string(void* out, const void* source, NativeStringStorage& strings) {
    put<std::uint32_t>(out, 0, 0);
    put<std::uint32_t>(out, 4, 0);
    if (out == source) return;
    resize_native_string_header_0041dd40(out, strings, get<std::uint32_t>(source, 0), true);
    if (get<std::uint32_t>(source, 0) == 0) return;
    std::memcpy(get<char*>(out, 4), get<const char*>(source, 4), get<std::uint32_t>(out, 0));
}

// The pair table at 00CF8350: {name, luaopen_*}, terminated by a null function. Names at
// 00CE3A0C (empty), then the table/io/os/string/math/debug strings.
struct LibraryPair {
    const char* name;
    lua_CFunction open;
};
const LibraryPair kLibraries[] = {
    {"", luaopen_base},     {"table", luaopen_table}, {"io", luaopen_io},
    {"os", luaopen_os},     {"string", luaopen_string}, {"math", luaopen_math},
    {"debug", luaopen_debug}, {nullptr, nullptr},
};
constexpr const char* kCallerInfoWhat = "nSlu"; // 00CF8398
constexpr int kGcPause = 100;                   // PUSH 64h at 006B8761
constexpr const char* kLaunchAirstrike = "launchairstrike"; // 00CF85E0

} // namespace

// ---------------------------------------------------------------------------------------------
// LuaMachine
// ---------------------------------------------------------------------------------------------
LuaMachine::LuaMachine(lua_State* state) noexcept : state_(state), owns_state_(false) {}

LuaMachine::LuaMachine() {
    state_ = luaL_newstate();
    lua_atpanic(state_, lua_machine_panic_006b8720);
    lua_gc(state_, LUA_GCSETPAUSE, kGcPause);
    for (const LibraryPair* pair = kLibraries; pair->open != nullptr; ++pair) {
        lua_pushcclosure(state_, pair->open, 0);
        lua_pushstring(state_, pair->name);
        lua_call(state_, 1, 0);
    }
    owns_state_ = true;
}

LuaMachine::~LuaMachine() {
    if (owns_state_) lua_close(state_);
}

int lua_machine_panic_006b8720(lua_State* state) {
    lua_tolstring(state, lua_gettop(state), nullptr);
    return 0;
}

int LuaMachine::get_top() { return lua_gettop(state_); }
void LuaMachine::set_top(int index) { lua_settop(state_, index); }
void LuaMachine::push_value(int index) { lua_pushvalue(state_, index); }
void LuaMachine::remove(int index) { lua_remove(state_, index); }
int LuaMachine::type(int index) { return lua_type(state_, index); }
bool LuaMachine::is_string_type(int index) { return lua_type(state_, index) == LUA_TSTRING; }
bool LuaMachine::to_boolean(int index) { return lua_toboolean(state_, index) != 0; }
float LuaMachine::to_float(int index) {
    // FSTP float / FLD float: the double result is rounded to single precision.
    return static_cast<float>(lua_tonumber(state_, index));
}
const char* LuaMachine::to_string(int index) { return lua_tolstring(state_, index, nullptr); }
void* LuaMachine::to_userdata(int index) { return lua_touserdata(state_, index); }
void LuaMachine::push_boolean(bool value) { lua_pushboolean(state_, value ? 1 : 0); }
void LuaMachine::push_number(float value) { lua_pushnumber(state_, static_cast<double>(value)); }
void LuaMachine::push_string(const char* text) { lua_pushstring(state_, text); }
void LuaMachine::push_nil() { lua_pushnil(state_); }

void LuaMachine::set_string_number(const char* key, float value) {
    lua_pushstring(state_, key);
    lua_pushnumber(state_, static_cast<double>(value));
    lua_settable(state_, -3);
}
void LuaMachine::set_number_number(float key, float value) {
    lua_pushnumber(state_, static_cast<double>(key));
    lua_pushnumber(state_, static_cast<double>(value));
    lua_settable(state_, -3);
}
void LuaMachine::set_number_string(float key, const char* value) {
    lua_pushnumber(state_, static_cast<double>(key));
    lua_pushstring(state_, value);
    lua_settable(state_, -3);
}
void LuaMachine::set_string_string(const char* key, const char* value) {
    lua_pushstring(state_, key);
    lua_pushstring(state_, value);
    lua_settable(state_, -3);
}
void LuaMachine::set_field_nil(const char* key) {
    lua_pushstring(state_, key);
    lua_pushnil(state_);
    lua_settable(state_, -3);
}
void LuaMachine::get_global(const char* name) { lua_getfield(state_, LUA_GLOBALSINDEX, name); }
void LuaMachine::get_table(int index) { lua_gettable(state_, index); }
void LuaMachine::create_table() { lua_createtable(state_, 0, 0); }
void LuaMachine::set_table(int index) { lua_settable(state_, index); }
int LuaMachine::protected_call(int nargs, int nresults, int errfunc) {
    return lua_pcall(state_, nargs, nresults, errfunc);
}
int LuaMachine::next(int index) { return lua_next(state_, index); }
void LuaMachine::pop(int count) { lua_settop(state_, -1 - count); }
void LuaMachine::register_global_function(const char* name, int (*function)(lua_State*)) {
    lua_pushcclosure(state_, function, 0);
    lua_setfield(state_, LUA_GLOBALSINDEX, name);
}
int LuaMachine::get_caller_info(int level, lua_Debug* record) {
    const int found = lua_getstack(state_, level, record);
    if (found == 0) return found;
    return lua_getinfo(state_, kCallerInfoWhat, record);
}

// ---------------------------------------------------------------------------------------------
// Result variants
// ---------------------------------------------------------------------------------------------
void lua_variant_vector_reserve_006b87f0(LuaVariantVector& vector, std::int32_t count,
                                         LuaVariantHooks& hooks) {
    if (count < 1) count = 1;
    if (vector.capacity >= count) return;
    auto* const replacement =
        static_cast<std::uint8_t*>(allocate(static_cast<std::uint32_t>(count) * kLuaVariantStride));
    for (std::int32_t i = 0; i < vector.size; ++i)
        hooks.copy_construct_variant_00886da0(replacement + i * kLuaVariantStride,
                                              vector.data + i * kLuaVariantStride);
    for (std::int32_t i = 0; i < vector.size; ++i)
        hooks.destroy_variant_virtual(vector.data + i * kLuaVariantStride);
    singleton_lifetime_free(vector.data); // _free 00BF6989, no null test
    vector.data = replacement;
    vector.capacity = count;
}

void lua_variant_vector_resize_006b88e0(LuaVariantVector& vector, std::int32_t count,
                                        LuaVariantHooks& hooks) {
    if (count > vector.capacity) lua_variant_vector_reserve_006b87f0(vector, count, hooks);
    for (std::int32_t i = vector.size; i < count; ++i)
        hooks.construct_variant_008849b0(vector.data + i * kLuaVariantStride);
    while (count < vector.size) {
        --vector.size;
        hooks.destroy_variant_virtual(vector.data + vector.size * kLuaVariantStride);
    }
    vector.size = count;
}

void lua_result_holder_clear_006b89a0(LuaResultHolder& holder, LuaVariantHooks& hooks) {
    if (holder.owner == nullptr) return;
    lua_variant_vector_resize_006b88e0(holder.results, 0, hooks);
    holder.owner = nullptr;
}

void lua_result_holder_reset_006b89c0(LuaResultHolder& holder, LuaMachine* owner,
                                      LuaVariantHooks& hooks) {
    if (holder.owner != nullptr) {
        lua_variant_vector_resize_006b88e0(holder.results, 0, hooks);
        holder.owner = nullptr; // 006B89D7, overwritten by the next store
    }
    holder.owner = owner;
}

int lua_machine_run_chunk_006b89f0(LuaMachine& machine, const char* buffer, std::size_t size,
                                   const char* chunk_name, LuaResultHolder* holder,
                                   void* error_string, int depth, LuaVariantHooks& hooks) {
    lua_State* const state = machine.state();
    const int base = lua_gettop(state);
    int failed = 1;
    if (luaL_loadbuffer(state, buffer, size, chunk_name) == 0)
        failed = lua_pcall(state, 0, LUA_MULTRET, 0) != 0 ? 1 : 0;
    const int top = lua_gettop(state);
    if (top <= base) return failed;
    if (failed != 0) {
        const char* const message = lua_tolstring(state, -1, nullptr);
        if (error_string != nullptr) hooks.native_string_assign_0041e350(error_string, message);
        lua_settop(state, base);
        return failed;
    }
    if (holder != nullptr) {
        lua_result_holder_reset_006b89c0(*holder, &machine, hooks);
        hooks.collect_stack_results_00887220(holder, top - base, depth);
    }
    lua_settop(state, base);
    return failed;
}

// ---------------------------------------------------------------------------------------------
// Front end and input configuration
// ---------------------------------------------------------------------------------------------
namespace {
void read_three_actions(std::uint8_t* out0, std::uint8_t* out1, std::uint8_t* out2,
                        std::int32_t first, std::int32_t second, FrontEndActionHost& host) {
    *out0 = host.action_edge_or_repeat_004d92b0(first) ? 1 : 0;
    *out1 = host.action_edge_or_repeat_004d92b0(second) ? 1 : 0;
    *out2 = host.action_edge_or_repeat_004d92b0(0x4a) ? 1 : 0;
}
} // namespace

void front_end_read_actions_4c_4d_4a_006964b0(std::uint8_t* out0, std::uint8_t* out1,
                                             std::uint8_t* out2, FrontEndActionHost& host) {
    read_three_actions(out0, out1, out2, 0x4c, 0x4d, host);
}
void front_end_read_actions_f3_f4_4a_006964f0(std::uint8_t* out0, std::uint8_t* out1,
                                             std::uint8_t* out2, FrontEndActionHost& host) {
    read_three_actions(out0, out1, out2, 0xf3, 0xf4, host);
}
void front_end_read_actions_f1_f2_4a_00696540(std::uint8_t* out0, std::uint8_t* out1,
                                             std::uint8_t* out2, FrontEndActionHost& host) {
    read_three_actions(out0, out1, out2, 0xf1, 0xf2, host);
}

void input_configuration_set_device_flags_00699b80(void* configuration, std::uint8_t flag_4c9,
    std::uint8_t flag_4ca, std::uint8_t flag_4cb, std::uint8_t flag_4cc,
    InputConfigurationHooks& hooks) {
    if (get<std::uint8_t>(configuration, 0x4c9) == flag_4c9 &&
        get<std::uint8_t>(configuration, 0x4ca) == flag_4ca &&
        get<std::uint8_t>(configuration, 0x4cb) == flag_4cb &&
        get<std::uint8_t>(configuration, 0x4cc) == flag_4cc)
        return;
    const bool loaded = get<std::uint8_t>(configuration, 0x520) != 0; // read before the stores
    put<std::uint8_t>(configuration, 0x4c9, flag_4c9);
    put<std::uint8_t>(configuration, 0x4ca, flag_4ca);
    put<std::uint8_t>(configuration, 0x4cb, flag_4cb);
    put<std::uint8_t>(configuration, 0x4cc, flag_4cc);
    if (!loaded) return;
    hooks.clear_00698730(configuration);
    hooks.load_00698a10(configuration);
}

void input_binding_display_name_0069e500(void* out, const void* event, InputBindingNameHooks& hooks,
                                         NativeStringStorage& strings) {
    if (get<std::int32_t>(event, 0) != 2) return;
    const auto index = get<std::uint32_t>(event, 4);
    if (hooks.device_by_class_and_index_004ba6d0(2, index) == nullptr) return;
    // The image repeats the lookup and uses the second result.
    void* const device = hooks.device_by_class_and_index_004ba6d0(2, index);
    if (device == nullptr) {
        hooks.native_string_assign_0041e350(out, ""); // 00CE3A0C
        return;
    }
    unsigned char name[8]; // the hidden result NativeString, filled by the callee
    void* const result = hooks.device_control_name_virtual_3c(device, name, get<std::uint32_t>(event, 0xc));
    hooks.native_string_assign_header_00425f40(out, result);
    release_pooled_string(name, strings);
}

void input_configuration_set_device_axis_value_006a9e70(void* settings, const void* key,
    const void* name, float value, InputDeviceSettingHooks& hooks) {
    void* const record = hooks.input_device_map_index_0055c110(at(settings, 8), key);
    *hooks.float_map_lookup_or_insert_00444be0(at(record, 0x24), name) = value;
}

std::uint32_t* copy_dwords_00697080(const std::uint32_t* first, const std::uint32_t* last,
                                    std::uint32_t* destination) noexcept {
    std::uint32_t* const end = destination + (last - first);
    for (; first != last; ++first, ++destination) *destination = *first;
    return end;
}

void push_back_context_word_00697170(void* header, const std::uint32_t* value) {
    if (get<std::int32_t>(header, 4) == get<std::int32_t>(header, 8))
        reserve_native_input_context_words_00696d80(header, doubled(get<std::int32_t>(header, 8)));
    // The value is read after a possible reallocation, as the image does.
    auto* const slot = get<std::uint32_t*>(header, 0) + get<std::int32_t>(header, 4);
    *slot = *value;
    put<std::int32_t>(header, 4, get<std::int32_t>(header, 4) + 1);
}

void* push_back_context_word_returning_006972d0(void* header, const std::uint32_t* value) {
    push_back_context_word_00697170(header, value);
    return header;
}

void push_back_input_modifier_006971c0(void* header, const void* modifier) {
    if (get<std::int32_t>(header, 4) == get<std::int32_t>(header, 8))
        reserve_native_input_modifiers_00696de0(header, doubled(get<std::int32_t>(header, 8)));
    void* const slot = at(get<void*>(header, 0), static_cast<std::size_t>(get<std::int32_t>(header, 4)) * 0x14);
    copy_record20(slot, modifier);
    put<std::int32_t>(header, 4, get<std::int32_t>(header, 4) + 1);
}

void destroy_context_words_006972b0(void* header) noexcept {
    resize_native_input_context_words_00696e70(header, 0);
    singleton_lifetime_free(get<void*>(header, 0));
}

void destroy_input_modifiers_006977d0(void* header) noexcept {
    resize_native_input_modifiers_00697220(header, 0);
    singleton_lifetime_free(get<void*>(header, 0));
}

void construct_word_vector_00697d40(void* destination, const void* source, WordVectorHooks& hooks) {
    if (destination != nullptr) hooks.copy_construct_word_vector_00557590(destination, source);
}
void construct_word_vector_00697ec0(void* destination, const void* source, WordVectorHooks& hooks) {
    if (destination != nullptr) hooks.copy_construct_word_vector_00557590(destination, source);
}

// ---------------------------------------------------------------------------------------------
// 14h-byte records
// ---------------------------------------------------------------------------------------------
void fill_records20(void* first, void* last, const void* value) noexcept {
    for (auto* p = static_cast<unsigned char*>(first); p != last; p += 0x14) copy_record20(p, value);
}

void* copy_records20(const void* first, const void* last, void* destination) noexcept {
    auto* in = static_cast<const unsigned char*>(first);
    auto* out = static_cast<unsigned char*>(destination);
    for (; in != last; in += 0x14, out += 0x14) copy_record20(out, in);
    return out;
}

void* copy_backward_records20(const void* first, const void* last, void* destination_end) noexcept {
    auto* in = static_cast<const unsigned char*>(last);
    auto* out = static_cast<unsigned char*>(destination_end);
    while (in != first) {
        in -= 0x14;
        out -= 0x14;
        copy_record20(out, in);
    }
    return out;
}

void uninitialized_fill_n_records20_0069d610(void* destination, std::uint32_t count,
                                             const void* value) noexcept {
    auto* out = static_cast<unsigned char*>(destination);
    for (; count != 0; --count, out += 0x14) copy_record20(out, value);
}

bool equal_dwords_0069d650(const std::uint32_t* first, const std::uint32_t* last,
                           const std::uint32_t* first2) noexcept {
    for (; first != last; ++first, ++first2)
        if (*first != *first2) break;
    return first == last;
}

void grow_records20_006bbb00(void* header) {
    const std::int32_t capacity = get<std::int32_t>(header, 8) * 2 + 2;
    put<std::int32_t>(header, 8, capacity);
    void* const replacement = allocate(static_cast<std::uint32_t>(capacity) * 0x14u);
    for (std::uint32_t i = 0; i < get<std::uint32_t>(header, 4); ++i)
        copy_record20(at(replacement, i * 0x14u), at(get<void*>(header, 0), i * 0x14u));
    if (get<void*>(header, 0) != nullptr) singleton_lifetime_free(get<void*>(header, 0));
    put<void*>(header, 0, replacement);
}

// ---------------------------------------------------------------------------------------------
// Small owners
// ---------------------------------------------------------------------------------------------
void reset_singleton_00e198e8_0069ba20(void* object, std::uint32_t& published_00e198e8) noexcept {
    engine_services_cc13::reset_singleton_base(object, published_00e198e8);
}
void reset_singleton_00e198f0_006ad3f0(void* object, std::uint32_t& published_00e198f0) noexcept {
    engine_services_cc13::reset_singleton_base(object, published_00e198f0);
}

void release_pooled_string_at0_0069e090(void* object, NativeStringStorage& strings) noexcept {
    release_pooled_string(object, strings);
}
void release_pooled_string_at4_0069e0b0(void* object, NativeStringStorage& strings) noexcept {
    release_pooled_string(at(object, 4), strings);
}

void* observer_member_set_target_006ac330(void* member, NativeObserverOwnerStorage* target,
                                          NativeObserverLifetime& lifetime) {
    auto* const current = get<NativeObserverOwnerStorage*>(member, 0x14);
    if (current == target) return member;
    auto& self = *static_cast<NativeObserverOwnerStorage*>(member);
    if (current != nullptr) lifetime.unregister_pair_006952a0(*current, self);
    put<NativeObserverOwnerStorage*>(member, 0x14, target);
    if (target != nullptr) register_observer_pair_00694a60(*target, self, lifetime);
    return member;
}

void award_registry_clear_006b91c0(void* tree, NativeGameProfileLifetimeContext& context,
                                   NativeAwardRegistryLifetimeProgress& progress) {
    erase_native_award_registry_subtree_006b9120(tree, get<void*>(get<void*>(tree, 4), 4), context,
                                                 progress);
    void* head = get<void*>(tree, 4);
    put<void*>(head, 4, head);
    head = get<void*>(tree, 4);
    put<std::uint32_t>(tree, 8, 0);
    put<void*>(head, 0, head);
    head = get<void*>(tree, 4);
    put<void*>(head, 8, head);
}

void* list_begin_at10_006b8cc0(void* owner, void* out) noexcept {
    put<void*>(out, 4, get<void*>(get<void*>(owner, 0x14), 0));
    put<void*>(out, 0, at(owner, 0x10));
    return out;
}

bool is_launch_airstrike_006ba6a0(const char* name) {
    return compare_insensitive_00438e10(name, kLaunchAirstrike) == 0;
}

std::uint16_t network_id_from_name_stub_006af3d0(std::uint32_t length, char* chars,
                                                 NativeStringStorage& strings) noexcept {
    if (chars != nullptr) strings.release(chars, length + 1);
    return 0;
}

void* award_table_copy_text_006b9fd0(void* table, void* out, const void* key, AwardTableHooks& hooks,
                                     NativeStringStorage& strings) {
    void* const record = hooks.find_award_record_0050fc30(at(table, 0x10), key);
    copy_construct_string(out, record, strings);
    return out;
}

void* award_table_copy_icon_006b9f70(void* table, void* out, const void* key, AwardTableHooks& hooks,
                                     NativeStringStorage& strings) {
    void* const record = hooks.find_award_record_0050fc30(at(table, 0x10), key);
    copy_construct_string(out, at(record, 0x14), strings);
    return out;
}

void destroy_object_array58_006bb4f0(void* array, ObjectArrayHooks& hooks) {
    if (get<void*>(array, 0) == nullptr) return;
    for (std::uint32_t i = 0; i < get<std::uint32_t>(array, 4); ++i)
        hooks.destroy_element_virtual(at(get<void*>(array, 0), i * 0x58u));
    singleton_lifetime_free(get<void*>(array, 0));
}

} // namespace bsp::cc13_seg29
