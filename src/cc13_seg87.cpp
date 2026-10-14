// cc13_seg87: segment 87 LuaObject helpers, owner DoBuffer/DoFile entries and
// the script-cache instance methods. Evidence: docs/CC13_SEG87.md.
// Reconstructed and build-tested only; not fixture- or game-validated.

#include "bsp/cc13_seg87.hpp"
#include "bsp/native_lua_field_setters.hpp"
#include "bsp/native_lua_class_setters.hpp"
#include "bsp/native_game_lua_globals_lifetime.hpp"

#include <cstring>
#include <intrin.h>
#include <new>
#include <string.h>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
}
#include <zlib.h>

namespace bsp {
namespace {

// Every native Lua call reloads [object] then [owner+4].
lua_State* state_of(const NativeLuaObjectStorage& object) noexcept
{
    return object.owner_00->state_04;
}

// The NativeString {length, data} read order used by 00B672F0/00B67350/00B67630:
// length first, then data, with a null data pointer replaced by the empty byte
// at 0108FF2C.
void push_native_string(lua_State* state, const NativeString& text)
{
    const std::uint32_t length = text.length();
    const char* data = text.data();
    if (!data) data = "";
    lua_pushlstring(state, data, length);
}

// Head shared by the integer-key setters 00B664E0..00B66740 and 00B672F0.
void push_integer_key(const NativeLuaObjectStorage& object, std::int32_t key)
{
    (void)lua_checkstack(state_of(object), 2);
    // FILD dword / FSTP double: every int32 is exact.
    lua_pushnumber(state_of(object), static_cast<lua_Number>(key));
}

void settable_current(const NativeLuaObjectStorage& object)
{
    lua_settable(state_of(object), object.index_08);
}

// 00B65AC0 (segment 86, outside this packet): v = v * M for a row-major 3x3
// M, each component summed as (x*M0j + y*M1j) + z*M2j and stored as float32.
void transform_vector3_by_matrix3_00b65ac0(float v[3], const float m[9])
{
    const float x = v[0];
    const float y = v[1];
    const float z = v[2];
    const float rx = x * m[0] + y * m[3] + z * m[6];
    const float ry = x * m[1] + y * m[4] + z * m[7];
    const float rz = x * m[2] + y * m[5] + z * m[8];
    v[0] = rx;
    v[1] = ry;
    v[2] = rz;
}

// The inline release at the end of every GetVector element: test the temp's
// kind, then 00B66DE0(owner, temp, index, 1). The temp's kind is not cleared.
void release_temporary(NativeLuaObjectStorage& temp)
{
    if (temp.kind_04)
        release_native_lua_tracked_object_00b66de0(temp.owner_00, temp, temp.index_08, 1);
}

float* get_elements(NativeLuaObjectStorage& object, float* out, std::int32_t count)
{
    for (std::int32_t i = 0; i < count; ++i) {
        alignas(NativeLuaObjectStorage) unsigned char storage[sizeof(NativeLuaObjectStorage)];
        NativeLuaObjectStorage* temp = native_lua_get_by_index_00b67720(object, storage, i + 1);
        // lua_tonumber 00A67770 returns ST0; FSTP float rounds to float32.
        out[i] = static_cast<float>(lua_tonumber(state_of(*temp), temp->index_08));
        release_temporary(*temp);
    }
    return out;
}

// The kind/type test inlined by 00B680A0, 00B687D0 and 00B68AD0: unbound is
// not a table, kind 2 asks lua_type, every other kind counts as a table.
bool table_like(const NativeLuaObjectStorage& object)
{
    if (object.kind_04 == 0) return false;
    if (object.kind_04 != 2) return true;
    return lua_type(state_of(object), object.index_08) == LUA_TTABLE;
}

// The key/value pair the deep copies build inline: owner 0, kind 0, index -1,
// tracked 0; word 0C is left unwritten.
void construct_iteration_object(NativeLuaObjectStorage& object) noexcept
{
    object.owner_00 = nullptr;
    object.kind_04 = 0;
    object.index_08 = -1;
    object.tracked_10 = 0;
}

// The inline key/value destructor: release when bound, then kind = 0.
void destroy_iteration_object(NativeLuaObjectStorage& object)
{
    if (object.kind_04) {
        release_native_lua_tracked_object_00b66de0(object.owner_00, object, object.index_08, 1);
        object.kind_04 = 0;
    }
}

// The walk shared by 00B687D0 (from 00B68870) and 00B68AD0 (from 00B68B36):
// `destination` is the freshly fetched sub-table temp.
bool copy_table_entries(NativeLuaObjectStorage& destination, NativeLuaObjectStorage& source,
    NativeStringStorage& strings)
{
    NativeLuaObjectStorage key;
    NativeLuaObjectStorage value;
    construct_iteration_object(key);
    construct_iteration_object(value);
    bool result = true;
    native_lua_iterate_first_00b67080(source, key, value);
    for (;;) {
        if (key.kind_04 == 0) {
            destroy_iteration_object(value);
            destroy_iteration_object(key);
            release_temporary(destination);
            return result;
        }
        const std::int32_t type = key.kind_04 == 2 ? lua_type(state_of(key), key.index_08) : LUA_TTABLE;
        if (type == LUA_TNUMBER) {
            // FSTP float, FLD float, _ftol2 (00BF7420): truncation toward zero.
            const float number = static_cast<float>(lua_tonumber(state_of(key), key.index_08));
            const auto index = static_cast<std::int32_t>(number);
            result = native_lua_deep_copy_to_index_00b687d0(destination, index, value, strings) && result;
        } else if (type == LUA_TSTRING) {
            const char* text = lua_tolstring(state_of(key), key.index_08, nullptr);
            NativeString name;
            name.assign_0041e870(strings, text);
            result = native_lua_deep_copy_to_field_00b68ad0(destination, name, value, strings) && result;
            // Inline release: 00419CC0 pool, 00BD1510(data, length + 1, 1) when data is non-null.
            destroy_native_string_header_0041dd20(&name, strings);
        } else {
            destroy_iteration_object(value);
            destroy_iteration_object(key);
            release_temporary(destination);
            return false;
        }
        native_lua_iterate_next_00b67190(source, key, value);
    }
}

// 00B65E80, byte-identical to 00B669A0: lua_close when state and owns are set,
// then clear the state word.
void close_owner_00b65e80(NativeLuaStateStorage& owner)
{
    close_native_lua_state_00b669a0(owner);
}

// The record runner inlined in 00B6C0E0 (and 00B6BD80): operator new(bound),
// uncompress (00BC94D0) with destLen = bound, 00B65EC0 with the inflated length,
// _free. Neither status is tested.
void run_record(NativeLuaStateStorage& owner, const NativeLuaScriptRecord& record,
    NativeLuaScriptCacheCalls& calls)
{
    const std::uint32_t bound = record.bound_08;
    auto* text = static_cast<Bytef*>(calls.operator_new_00bf55be(bound));
    uLongf length = bound;
    (void)uncompress(text, &length, record.data_00, record.compressed_size_04);
    native_lua_do_buffer_00b65ec0(owner, reinterpret_cast<const char*>(text),
        static_cast<std::uint32_t>(length));
    calls.free_00bf6989(text);
}

std::uint32_t record_count(const NativeLuaScriptRecord* begin, const NativeLuaScriptRecord* end) noexcept
{
    // SUB, IMUL 2AAAAAABh, SAR 1, sign fix: signed division by 12.
    const auto bytes = static_cast<std::int32_t>(
        reinterpret_cast<std::uintptr_t>(end) - reinterpret_cast<std::uintptr_t>(begin));
    // The index compare that follows is unsigned (JNC / JC).
    return static_cast<std::uint32_t>(bytes / 12);
}

} // namespace

void transform_plane_by_matrix4x3_00b65c30(float plane[4], const float matrix[16])
{
    const float x = plane[0];
    const float y = plane[1];
    const float z = plane[2];
    // 00B65C86..00B65D64: each component (y*m[4+j] + x*m[j]) + z*m[8+j].
    const float nx = y * matrix[4] + x * matrix[0] + z * matrix[8];
    const float ny = y * matrix[5] + x * matrix[1] + z * matrix[9];
    const float nz = y * matrix[6] + x * matrix[2] + z * matrix[10];
    // Locals at ESP+30h (translation) and ESP+48h (the transposed 3x3).
    float translation[3] = {matrix[12], matrix[13], matrix[14]};
    const float transposed[9] = {
        matrix[0], matrix[4], matrix[8],
        matrix[1], matrix[5], matrix[9],
        matrix[2], matrix[6], matrix[10],
    };
    transform_vector3_by_matrix3_00b65ac0(translation, transposed);
    // 00B65D6D..00B65DB4: (y*t.y + x*t.x) + z*t.z, FSTP float, then w - dot.
    const float dot = y * translation[1] + x * translation[0] + z * translation[2];
    plane[0] = nx;
    plane[1] = ny;
    plane[2] = nz;
    plane[3] = plane[3] - dot;
}

NativeLuaStateStorage& construct_native_lua_owner_with_dofile_00b65e20(
    NativeLuaStateStorage& owner, lua_State* state, int (*do_file_00b69e00)(lua_State*))
{
    owner.owns_00 = 0;
    owner.state_04 = state;
    lua_pushcclosure(state, do_file_00b69e00, 0);
    lua_setfield(owner.state_04, LUA_GLOBALSINDEX, "DoFile");
    return owner;
}

void register_native_lua_dofile_00b69f20(NativeLuaStateStorage& owner,
    int (*do_file_00b69e00)(lua_State*))
{
    lua_pushcclosure(owner.state_04, do_file_00b69e00, 0);
    lua_setfield(owner.state_04, LUA_GLOBALSINDEX, "DoFile");
}

void native_lua_do_buffer_00b65ec0(NativeLuaStateStorage& owner, const char* buffer,
    std::uint32_t size)
{
    (void)luaL_loadbuffer(owner.state_04, buffer, size, "DoBuffer");
    lua_call(owner.state_04, 0, LUA_MULTRET);
}

std::int32_t native_lua_do_buffer_protected_00b65ef0(NativeLuaStateStorage& owner,
    const char* buffer, std::uint32_t size)
{
    (void)lua_gettop(owner.state_04);
    if (luaL_loadbuffer(owner.state_04, buffer, size, "DoBuffer") == 0
        && lua_pcall(owner.state_04, 0, LUA_MULTRET, 0) == 0) {
        (void)lua_gettop(owner.state_04);
        return 0;
    }
    (void)lua_gettop(owner.state_04);
    return 1;
}

bool native_lua_is_bound_00b65f70(const NativeLuaObjectStorage& object) noexcept
{
    return object.kind_04 != 0;
}

std::int32_t native_lua_type_00b65f80(const NativeLuaObjectStorage& object)
{
    if (object.kind_04 == 0) return -1;
    if (object.kind_04 == 2) return lua_type(state_of(object), object.index_08);
    return LUA_TTABLE;
}

bool native_lua_equals_cstring_00b660f0(const NativeLuaObjectStorage& object, const char* text)
{
    if (object.kind_04 == 0 || object.kind_04 != 2) return false;
    if (lua_type(state_of(object), object.index_08) != LUA_TSTRING) return false;
    return std::strcmp(lua_tolstring(state_of(object), object.index_08, nullptr), text) == 0;
}

bool native_lua_is_lightuserdata_00b66160(const NativeLuaObjectStorage& object)
{
    return native_lua_type_00b65f80(object) == LUA_TLIGHTUSERDATA;
}

bool native_lua_is_function_00b66200(const NativeLuaObjectStorage& object)
{
    return native_lua_type_00b65f80(object) == LUA_TFUNCTION;
}

const void* native_lua_to_pointer_00b662e0(const NativeLuaObjectStorage& object)
{
    return lua_topointer(state_of(object), object.index_08);
}

void native_lua_push_value_00b663d0(const NativeLuaObjectStorage& object)
{
    (void)lua_checkstack(state_of(object), 1);
    lua_pushvalue(state_of(object), object.index_08);
}

void native_lua_push_nil_00b66430(const NativeLuaObjectStorage& object)
{
    (void)lua_checkstack(state_of(object), 1);
    lua_pushnil(state_of(object));
}

void native_lua_push_boolean_00b66450(const NativeLuaObjectStorage& object, std::uint8_t value)
{
    (void)lua_checkstack(state_of(object), 1);
    lua_pushboolean(state_of(object), value != 0);  // CMP byte / SETNZ
}

void native_lua_push_integer_00b664b0(const NativeLuaObjectStorage& object, std::int32_t value)
{
    (void)lua_checkstack(state_of(object), 1);
    lua_pushnumber(state_of(object), static_cast<lua_Number>(value));
}

void native_lua_push_cstring_00b66710(const NativeLuaObjectStorage& object, const char* text)
{
    (void)lua_checkstack(state_of(object), 1);
    lua_pushstring(state_of(object), text);
}

void native_lua_set_nil_at_00b664e0(NativeLuaObjectStorage& object, std::int32_t key)
{
    push_integer_key(object, key);
    lua_pushnil(state_of(object));
    settable_current(object);
}

void native_lua_set_boolean_at_00b66530(NativeLuaObjectStorage& object, std::int32_t key,
    std::uint8_t value)
{
    push_integer_key(object, key);
    lua_pushboolean(state_of(object), value);  // MOVZX: the raw byte, not normalised here
    settable_current(object);
}

void native_lua_set_integer_at_00b665d0(NativeLuaObjectStorage& object, std::int32_t key,
    std::int32_t value)
{
    push_integer_key(object, key);
    lua_pushnumber(state_of(object), static_cast<lua_Number>(value));
    settable_current(object);
}

void native_lua_set_lightuserdata_at_00b66620(NativeLuaObjectStorage& object, std::int32_t key,
    void* value)
{
    push_integer_key(object, key);
    lua_pushlightuserdata(state_of(object), value);
    settable_current(object);
}

void native_lua_set_new_table_at_00b66670(NativeLuaObjectStorage& object, std::int32_t key)
{
    push_integer_key(object, key);
    lua_createtable(state_of(object), 0, 0);
    settable_current(object);
}

void native_lua_set_object_at_00b666c0(NativeLuaObjectStorage& object, std::int32_t key,
    const NativeLuaObjectStorage& value)
{
    push_integer_key(object, key);
    lua_pushvalue(state_of(object), value.index_08);
    settable_current(object);
}

void native_lua_set_string_at_00b66740(NativeLuaObjectStorage& object, std::int32_t key,
    const char* value)
{
    push_integer_key(object, key);
    lua_pushstring(state_of(object), value);
    settable_current(object);
}

void native_lua_set_lstring_at_00b672f0(NativeLuaObjectStorage& object, std::int32_t key,
    const NativeString& value)
{
    push_integer_key(object, key);
    push_native_string(state_of(object), value);
    settable_current(object);
}

void native_lua_set_nil_00b67350(NativeLuaObjectStorage& object, const NativeString& key)
{
    (void)lua_checkstack(state_of(object), 2);
    push_native_string(state_of(object), key);
    lua_pushnil(state_of(object));
    settable_current(object);
}

void native_lua_set_string_00b67630(NativeLuaObjectStorage& object, const char* key,
    const NativeString& value)
{
    (void)lua_checkstack(state_of(object), 2);
    lua_pushstring(state_of(object), key);
    push_native_string(state_of(object), value);
    settable_current(object);
}

void native_lua_release_00b67050(NativeLuaObjectStorage& object, std::uint8_t remove_stack)
{
    if (object.kind_04 == 0) return;
    release_native_lua_tracked_object_00b66de0(object.owner_00, object, object.index_08, remove_stack);
    object.kind_04 = 0;
}

NativeLuaObjectStorage* native_lua_get_by_native_string_00b678e0(NativeLuaObjectStorage& object,
    void* fresh, const NativeString& key)
{
    const char* text = key.data();
    if (!text) text = "";
    native_lua_get_by_name_00b67800(object, fresh, text);
    return static_cast<NativeLuaObjectStorage*>(fresh);
}

NativeLuaObjectStorage* native_lua_get_by_literal_00b67910(NativeLuaObjectStorage& object,
    void* fresh, const char* key)
{
    native_lua_get_by_name_00b67800(object, fresh, key);
    return static_cast<NativeLuaObjectStorage*>(fresh);
}

NativeLuaObjectStorage* native_lua_new_table_00b67930(NativeLuaObjectStorage& object, void* fresh)
{
    lua_createtable(state_of(object), 0, 0);
    const int top = lua_gettop(state_of(object));
    auto* result = static_cast<NativeLuaObjectStorage*>(fresh);
    result->owner_00 = object.owner_00;
    result->kind_04 = 2;
    result->index_08 = top;
    result->opaque_0c = 0;
    result->tracked_10 = 0;
    return result;
}

float* native_lua_get_vector2_00b679f0(NativeLuaObjectStorage& object, float* out)
{
    return get_elements(object, out, 2);
}

float* native_lua_get_vector3_00b67a80(NativeLuaObjectStorage& object, float* out)
{
    return get_elements(object, out, 3);
}

float* native_lua_get_vector4_00b67b40(NativeLuaObjectStorage& object, float* out)
{
    return get_elements(object, out, 4);
}

float* native_lua_get_float4_00b67c40(NativeLuaObjectStorage& object, float* out)
{
    return get_elements(object, out, 4);
}

float* native_lua_get_vector3_or_00b680a0(NativeLuaObjectStorage& object, float* out,
    const float* fallback)
{
    if (table_like(object))
        return native_lua_get_vector3_00b67a80(object, out);
    out[0] = fallback[0];
    out[1] = fallback[1];
    out[2] = fallback[2];
    return out;
}

NativeString* native_lua_to_display_string_00b68630(const NativeLuaObjectStorage& object,
    void* fresh, const NativeLuaDisplayStringCalls& calls)
{
    auto* out = ::new (fresh) NativeString;
    const char* text;
    // Jump table 00B6879C indexed by type + 1.
    switch (native_lua_type_00b65f80(object)) {
    case LUA_TNONE: text = "none"; break;                      // 00CE43EC
    case LUA_TNIL: text = "nil"; break;                        // 00CE4380
    case LUA_TBOOLEAN:                                         // 00CE4378 / 00CE4370
        text = lua_toboolean(state_of(object), object.index_08) ? "true" : "false";
        break;
    case LUA_TLIGHTUSERDATA: text = "lightuserdata"; break;    // 00D62C1C
    case LUA_TNUMBER:
        calls.construct_from_float_0043bdf0(calls.context, *out, native_lua_number_00b66270(object));
        return out;
    case LUA_TSTRING: text = lua_tolstring(state_of(object), object.index_08, nullptr); break;
    case LUA_TTABLE: text = "table"; break;                    // 00CF8344
    case LUA_TFUNCTION: text = "function"; break;              // 00D04D44
    case LUA_TUSERDATA: text = "userdata"; break;              // 00D58568
    case LUA_TTHREAD: text = "thread"; break;                  // 00D58558
    default: text = "Unknown"; break;                          // 00D1CEE8
    }
    out->assign_0041e870(calls.strings, text);
    return out;
}

bool native_lua_deep_copy_to_index_00b687d0(NativeLuaObjectStorage& destination,
    std::int32_t key, NativeLuaObjectStorage& source, NativeStringStorage& strings)
{
    if (!table_like(source)) {
        native_lua_set_object_at_00b666c0(destination, key, source);
        return true;
    }
    // 00B68821..00B68858: the 00B66670 body inline.
    push_integer_key(destination, key);
    lua_createtable(state_of(destination), 0, 0);
    settable_current(destination);
    alignas(NativeLuaObjectStorage) unsigned char storage[sizeof(NativeLuaObjectStorage)];
    native_lua_argument_at_00b677e0(destination, storage, key);
    return copy_table_entries(*reinterpret_cast<NativeLuaObjectStorage*>(storage), source, strings);
}

bool native_lua_deep_copy_to_field_00b68ad0(NativeLuaObjectStorage& destination,
    const NativeString& key, NativeLuaObjectStorage& source, NativeStringStorage& strings)
{
    if (!table_like(source)) {
        native_lua_set_object_00b675d0(destination, key, source);
        return true;
    }
    native_lua_set_new_table_00b67580(destination, key);
    alignas(NativeLuaObjectStorage) unsigned char storage[sizeof(NativeLuaObjectStorage)];
    native_lua_get_by_string_00b68100(destination, storage, key);
    return copy_table_entries(*reinterpret_cast<NativeLuaObjectStorage*>(storage), source, strings);
}

void destroy_native_lua_fundamentals_00b66960(NativeLuaFundamentalsStorage& cache,
    volatile std::uint32_t& global_0108ff1c, void (*free_00bf6989)(void*))
{
    cache.vtable_00 = 0x00D62C18u;
    if (cache.bytes_04) {
        free_00bf6989(cache.bytes_04);
        cache.bytes_04 = nullptr;
    }
    global_0108ff1c = 0;
    cache.vtable_00 = 0x00CE3818u;
}

void close_native_lua_script_cache_00b6bd60(NativeLuaScriptCacheStorage& cache)
{
    if (!cache.open_20) return;
    close_owner_00b65e80(cache.owner_24);
    cache.open_20 = 0;
}

void open_and_run_native_lua_script_cache_00b6c0e0(NativeLuaScriptCacheStorage& cache,
    const NativeLuaScriptRecordView& globals, NativeStringStorage& strings,
    const NativeLuaBootstrapInputs& inputs, NativeLuaScriptCacheCalls& calls)
{
    if (cache.open_20) return;
    open_native_lua_state_00b6a020(cache.owner_24, 0x65, strings, inputs);
    // 00B6C103: begin and end are reloaded every iteration; no checked access.
    for (std::uint32_t i = 0;; ++i) {
        NativeLuaScriptRecord* begin = globals.begin_0108ff44;
        if (!begin || i >= record_count(begin, globals.end_0108ff48)) break;
        run_record(cache.owner_24, begin[i], calls);
    }
    // 00B6C180: the instance vector through the checked operator[].
    for (std::uint32_t i = 0;; ++i) {
        if (!cache.records_begin_14 || i >= record_count(cache.records_begin_14, cache.records_end_18))
            break;
        if (!cache.records_begin_14 || i >= record_count(cache.records_begin_14, cache.records_end_18))
            calls.invalid_parameter_00bf6713();
        run_record(cache.owner_24, cache.records_begin_14[i], calls);
    }
    cache.open_20 = 1;
}

void clear_native_lua_script_cache_00b6d0a0(NativeLuaScriptCacheStorage& cache,
    NativeLuaScriptCacheCalls& calls)
{
    if (!cache.names_begin_04) return;
    const auto name_bytes = static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(cache.names_end_08)
        - reinterpret_cast<std::uintptr_t>(cache.names_begin_04));
    if ((name_bytes >> 3) == 0) return;
    if (cache.open_20) {
        close_owner_00b65e80(cache.owner_24);
        cache.open_20 = 0;
    }
    NativeString* const names_end = cache.names_end_08;
    if (cache.names_begin_04 > names_end) calls.invalid_parameter_00bf6713();
    NativeString* const names_begin = cache.names_begin_04;
    if (names_begin > cache.names_end_08) calls.invalid_parameter_00bf6713();
    std::uint32_t output[2];
    calls.erase_strings_004954f0(&cache, output, names_begin, names_end);
    for (std::uint32_t i = 0;; ++i) {
        if (!cache.records_begin_14 || i >= record_count(cache.records_begin_14, cache.records_end_18))
            break;
        if (!cache.records_begin_14 || i >= record_count(cache.records_begin_14, cache.records_end_18))
            calls.invalid_parameter_00bf6713();
        calls.free_00bf6989(cache.records_begin_14[i].data_00);
    }
    NativeLuaScriptRecord* const records_end = cache.records_end_18;
    if (cache.records_begin_14 > records_end) calls.invalid_parameter_00bf6713();
    NativeLuaScriptRecord* const records_begin = cache.records_begin_14;
    if (records_begin > cache.records_end_18) calls.invalid_parameter_00bf6713();
    if (records_begin == records_end) return;
    cache.records_end_18 = static_cast<NativeLuaScriptRecord*>(
        copy_native_game_lua_records_00b6c360(records_end, cache.records_end_18, records_begin));
}

// ===========================================================================
// Segment 89
// ===========================================================================

namespace {

using U32 = std::uint32_t;

unsigned char* at(void* object, U32 offset) noexcept
{
    return static_cast<unsigned char*>(object) + offset;
}
const unsigned char* at(const void* object, U32 offset) noexcept
{
    return static_cast<const unsigned char*>(object) + offset;
}
U32 dword(const void* object, U32 offset) noexcept
{
    U32 value;
    std::memcpy(&value, at(object, offset), 4);
    return value;
}
void set_dword(void* object, U32 offset, U32 value) noexcept
{
    std::memcpy(at(object, offset), &value, 4);
}
void* pointer(const void* object, U32 offset) noexcept
{
    void* value;
    std::memcpy(&value, at(object, offset), 4);
    return value;
}
void set_pointer(void* object, U32 offset, const void* value) noexcept
{
    std::memcpy(at(object, offset), &value, 4);
}
float single(const void* object, U32 offset) noexcept
{
    float value;
    std::memcpy(&value, at(object, offset), 4);
    return value;
}
void set_single(void* object, U32 offset, float value) noexcept
{
    std::memcpy(at(object, offset), &value, 4);
}

// kernel32 InterlockedIncrement / InterlockedDecrement through the import
// cells 00CE221C / 00CE2220 on the reference count at object+4.
void add_reference(void* object) noexcept
{
    _InterlockedIncrement(reinterpret_cast<volatile long*>(at(object, 4)));
}
void drop_reference(void* object, NativeSeg89Calls& calls)
{
    if (_InterlockedDecrement(reinterpret_cast<volatile long*>(at(object, 4))) == 0)
        calls.release_vcall_00(object);
}

// The inline reference-pointer assignment used by 00B860E0 and 00B861D0:
// skip when equal, store, add to the new one, then drop the old one.
void assign_reference(void* object, U32 offset, void* value, NativeSeg89Calls& calls)
{
    void* const old = pointer(object, offset);
    if (old == value) return;
    set_pointer(object, offset, value);
    if (value) add_reference(value);
    if (old) drop_reference(old, calls);
}

void* construct_map_head(void* map, void* head)
{
    set_pointer(map, 4, head);
    *at(head, 0x19) = 1;
    set_pointer(pointer(map, 4), 4, pointer(map, 4));
    set_pointer(pointer(map, 4), 0, pointer(map, 4));
    set_pointer(pointer(map, 4), 8, pointer(map, 4));
    set_dword(map, 8, 0);
    return map;
}

// The geometry/element walk shared by the four model walkers.
template <typename Visit>
void walk_model_materials(void* model, NativeModelWalkCalls& calls, Visit&& visit)
{
    if (!calls.is_type_vcall_0c(model, calls.current_model_type_00b74310())) return;
    for (U32 i = 0; i < calls.geometry_count_00b74650(model); ++i) {
        void* const geometry = calls.geometry_00b74640(model, i);
        for (U32 j = 0; j < calls.element_count_00b72b40(geometry); ++j)
            visit(pointer(calls.element_00b732c0(geometry, j), 0x20));
    }
}

const NativeString* material_effect_name(void* material, NativeModelWalkCalls& calls)
{
    return calls.effect_name_00b172d0(pointer(material, 0x7c));
}

// The strstr match of 00B85200/00B85310: both data pointers non-null, a hit,
// and hit - name != -1.
bool effect_contains(const NativeString& name, const NativeString& key)
{
    const char* const text = name.data();
    const char* const part = key.data();
    if (!text || !part) return false;
    const char* const hit = std::strstr(text, part);
    return hit && hit - text != -1;
}

} // namespace

void* construct_native_parser_map_00b80fd0(void* map, NativeSeg89Calls& calls)
{
    return construct_map_head(map, calls.buy_parser_map_head_00b7e000(map));
}

void* construct_native_cache_map_00b81000(void* map, NativeSeg89Calls& calls)
{
    return construct_map_head(map, calls.buy_cache_map_head_00b7e050(map));
}

void destroy_native_string_table_rows_00b81170(void* rows, NativeSeg89Calls& calls)
{
    calls.string_table_rows_tidy_00b80e10(rows, 0);
    calls.free_00bf6989(pointer(rows, 0));
}

void destroy_native_string_table_rows_00b811a0(void* rows, NativeSeg89Calls& calls)
{
    calls.string_table_rows_tidy_00b80e10(rows, 0);
    calls.free_00bf6989(pointer(rows, 0));
}

void deallocate_native_block_00b81a30(void* block, std::uint32_t, NativeSeg89Calls& calls)
{
    calls.delete_00bf65ac(block);
}

void deallocate_native_block_00b81a70(void* block, std::uint32_t, NativeSeg89Calls& calls)
{
    calls.delete_00bf65ac(block);
}

void deallocate_native_block_00b86c80(void* block, std::uint32_t, NativeSeg89Calls& calls)
{
    calls.delete_00bf65ac(block);
}

void deallocate_native_block_00b86cc0(void* block, std::uint32_t, NativeSeg89Calls& calls)
{
    calls.delete_00bf65ac(block);
}

void* allocate_native_registry_elements_00b81c10(std::uint32_t count, NativeSeg89Calls& calls)
{
    if (count != 0 && 0xFFFFFFFFu / count < 12u) throw std::bad_alloc();
    return calls.operator_new_00bf681b(count * 12u);
}

void clear_native_registry_list_00b82350(void* list, NativeSeg89Calls& calls)
{
    void* node = pointer(pointer(list, 4), 0);
    set_pointer(pointer(list, 4), 0, pointer(list, 4));
    set_pointer(pointer(list, 4), 4, pointer(list, 4));
    set_dword(list, 8, 0);
    while (node != pointer(list, 4)) {
        void* const next = pointer(node, 0);
        calls.delete_00bf65ac(node);
        node = next;
    }
}

void* construct_native_registry_list_00b82b50(void* list, NativeSeg89Calls& calls)
{
    set_pointer(list, 4, calls.buy_registry_list_head_00b82390(list));
    set_dword(list, 8, 0);
    return list;
}

void free_native_registry_storage_00b82cc0(void* vector, NativeSeg89Calls& calls)
{
    if (void* const begin = pointer(vector, 4)) calls.delete_00bf65ac(begin);
    set_dword(vector, 4, 0);
    set_dword(vector, 8, 0);
    set_dword(vector, 0xc, 0);
}

void set_native_model_parameter_owner_by_effect_00b84fe0(void* model, const NativeString& effect,
    void* owner, NativeModelWalkCalls& calls)
{
    walk_model_materials(model, calls, [&](void* material) {
        const NativeString& name = *material_effect_name(material, calls);
        if (name.length() != effect.length()) return;
        bool equal;
        if (name.length() == 0) equal = effect.length() == 0;
        else if (effect.length() == 0) equal = false;
        else equal = _stricmp(name.data(), effect.data()) == 0;  // 00BF7FBF
        if (equal) calls.set_parameter_owner_00b18a40(material, owner, 1);
    });
    for (void* child = pointer(model, 0x34); child; child = pointer(child, 0x3c))
        set_native_model_parameter_owner_by_effect_00b84fe0(child, effect, owner, calls);
}

void replace_native_model_texture_00b850f0(void* model, void* current, void* replacement,
    NativeModelWalkCalls& calls)
{
    walk_model_materials(model, calls, [&](void* material) {
        for (std::int32_t k = 0; k < calls.texture_slot_count_00b179c0(material); ++k) {
            const auto bound = static_cast<std::int32_t>(
                static_cast<std::int16_t>(dword(material, 0x34) & 0xffffu));
            void* const texture = (k >= 0 && k < bound) ? pointer(material, 0x10 + 4u * static_cast<U32>(k)) : nullptr;
            if (texture == current) calls.set_texture_slot_00b189f0(material, k, replacement);
        }
    });
    for (void* child = pointer(model, 0x34); child; child = pointer(child, 0x3c))
        replace_native_model_texture_00b850f0(child, current, replacement, calls);
}

void set_native_model_diffuse_by_effect_00b85200(void* model, const NativeString& effect,
    const std::uint32_t color[4], NativeModelWalkCalls& calls)
{
    walk_model_materials(model, calls, [&](void* material) {
        if (!effect_contains(*material_effect_name(material, calls), effect)) return;
        float* const diffuse = calls.diffuse_color_00b179f0(material, 0);
        std::memcpy(diffuse, color, 16);  // four MOV dword pairs
    });
    for (void* child = pointer(model, 0x34); child; child = pointer(child, 0x3c))
        set_native_model_diffuse_by_effect_00b85200(child, effect, color, calls);
}

bool set_native_model_texture_by_effect_00b85310(void* model, const NativeString& effect,
    void* texture, std::int32_t slot, NativeModelWalkCalls& calls)
{
    bool result = false;
    walk_model_materials(model, calls, [&](void* material) {
        if (!effect_contains(*material_effect_name(material, calls), effect)) return;
        calls.set_texture_slot_00b189f0(material, slot, texture);
        result = true;
    });
    for (void* child = pointer(model, 0x34); child; child = pointer(child, 0x3c))
        result = set_native_model_texture_by_effect_00b85310(child, effect, texture, slot, calls) || result;
    return result;
}

void* construct_native_debug_render_flags_00b854d0(void* flags) noexcept
{
    set_dword(flags, 0, 0x00D63190u);
    *at(flags, 4) = 0;
    *at(flags, 5) = 1;
    *at(flags, 6) = 0;
    return flags;
}

void* delete_native_debug_render_flags_00b85510(void* object, std::uint8_t flags,
    volatile std::uint32_t& global_010901cc, NativeSeg89Calls& calls)
{
    global_010901cc = 0;
    set_dword(object, 0, 0x00CE3818u);
    if (flags & 1) calls.delete_00bf65ac(object);
    return object;
}

void* delete_native_debug_render_flags_00b85540(void* object, std::uint8_t flags,
    volatile std::uint32_t& global_010901cc, NativeSeg89Calls& calls)
{
    global_010901cc = 0;
    set_dword(object, 0, 0x00CE3818u);
    if (flags & 1) calls.delete_00bf65ac(object);
    return object;
}

void set_native_pointer_vector_capacity_00b85750(NativeGuiPoolSlabTable& vector, std::uint32_t capacity)
{
    native_gui_pool_table_set_capacity(vector, capacity);  // body of 00AA6FA0
}

void grow_native_pointer_vector_00b857a0(NativeGuiPoolSlabTable& vector)
{
    native_gui_pool_table_grow(vector);  // body of 00AA6FF0
}

void push_native_pointer_vector_00b85a30(NativeGuiPoolSlabTable& vector, std::byte* const& value)
{
    native_gui_pool_table_push_back(vector, value);  // body of 00AA76C0
}

void reserve_native_pointer_vector_00b85ac0(NativeGuiPoolSlabTable& vector, std::uint32_t capacity)
{
    native_gui_pool_table_reserve(vector, capacity);  // body of 00AA7740
}

void fill_native_slot_table_00b85980(void* table, std::uint32_t value) noexcept
{
    for (U32 i = 0; i < 0x40; ++i) set_dword(table, 0x60 + i * 0x64, value);
}

bool native_slot_table_is_full_00b859a0(const void* table) noexcept
{
    std::uint16_t count;
    std::memcpy(&count, at(table, 0x1980), 2);
    return count == 0x40;
}

void unlink_native_ref_chain_00b85b30(void* owner, void* node, NativeSeg89Calls& calls)
{
    void* previous = owner;
    while (pointer(previous, 0x38) != node) previous = pointer(previous, 0x38);
    set_pointer(previous, 0x38, pointer(node, 0x38));
    set_pointer(node, 0x38, nullptr);
    drop_reference(node, calls);
}

void* construct_native_ref_record_00b860e0(void* record, void* ref_20, std::uint32_t d08,
    std::uint32_t d0c, std::uint32_t d10, std::uint32_t d14, std::uint32_t d18,
    std::uint32_t d1c, const float color[4], std::uint32_t, NativeSeg89Calls& calls)
{
    set_dword(record, 0, 0x00CEB130u);  // base vtable, then refcount 1
    set_dword(record, 4, 1);
    set_dword(record, 0, 0x00D63194u);
    set_dword(record, 0x20, 0);
    set_dword(record, 0x38, 0);
    set_dword(record, 0x4c, 0);
    *at(record, 0x58) = 1;
    set_dword(record, 0x08, d08);
    set_dword(record, 0x0c, d0c);
    set_dword(record, 0x10, d10);
    set_dword(record, 0x14, d14);
    set_dword(record, 0x50, 0);
    set_dword(record, 0x5c, 0);
    set_dword(record, 0x18, d18);
    set_dword(record, 0x1c, d1c);
    assign_reference(record, 0x20, ref_20, calls);
    set_single(record, 0x24, color[0]);
    set_single(record, 0x28, color[1]);
    set_single(record, 0x2c, color[2]);
    set_single(record, 0x30, color[3]);
    set_single(record, 0x34, 0.0f);
    return record;
}

void* assign_native_ref_record_00b861d0(void* destination, const void* source, NativeSeg89Calls& calls)
{
    *at(destination, 0x58) = *at(source, 0x58);
    for (U32 offset = 0x08; offset <= 0x1c; offset += 4) set_dword(destination, offset, dword(source, offset));
    assign_reference(destination, 0x20, pointer(source, 0x20), calls);
    for (U32 offset = 0x24; offset <= 0x34; offset += 4) set_single(destination, offset, single(source, offset));
    assign_reference(destination, 0x38, pointer(source, 0x38), calls);
    set_dword(destination, 0x54, dword(source, 0x54));
    for (std::int32_t i = 0; i < static_cast<std::int32_t>(dword(destination, 0x4c)); ++i) {
        const U32 offset = 0x3c + 4u * static_cast<U32>(i);
        if (void* const reference = pointer(destination, offset)) {
            drop_reference(reference, calls);
            set_pointer(destination, offset, nullptr);
        }
    }
    set_dword(destination, 0x4c, 0);
    for (std::int32_t i = 0; i < static_cast<std::int32_t>(dword(source, 0x4c)); ++i) {
        void* const reference = pointer(source, 0x3c + 4u * static_cast<U32>(i));
        add_reference(reference);  // no null test here
        const U32 count = dword(destination, 0x4c);
        set_pointer(destination, 0x3c + 4u * count, reference);
        set_dword(destination, 0x4c, count + 1);
    }
    assign_reference(destination, 0x50, pointer(source, 0x50), calls);
    assign_reference(destination, 0x5c, pointer(source, 0x5c), calls);
    return destination;
}

std::uint32_t native_return_zero_00b866b0(std::uint32_t) noexcept
{
    return 0;
}

std::uint32_t native_fallback_runtime_type_00b86800(const volatile std::uint32_t& global_01090210) noexcept
{
    return global_01090210;
}

std::uint32_t native_fallback_type_descriptor_00b86810(const volatile std::uint32_t& global_01090218) noexcept
{
    return global_01090218;
}

std::uint32_t native_unknown_item_type_token_00b86860(const volatile std::uint32_t& global_0109021c) noexcept
{
    return global_0109021c;
}

std::uint32_t native_unknown_item_aux_token_00b86870(const volatile std::uint32_t& global_01090228) noexcept
{
    return global_01090228;
}

void dispatch_native_fallback_resource_00b86820(void* resource, void* visitor,
    std::uint32_t, std::uint32_t argument, std::uint32_t, NativeSeg89Calls& calls)
{
    calls.visit_vcall_08(visitor, resource, argument);
}

void native_resource_item_noop_00b86880() noexcept {}

void native_resource_item_noop_00b868a0() noexcept {}

bool native_fallback_resource_is_type_00b868d0(const volatile std::uint32_t (&types_01090210)[2],
    std::uint32_t token) noexcept
{
    for (const volatile std::uint32_t& type : types_01090210)
        if (type == token) return true;
    return false;
}

void* initialize_native_fallback_type_00b86ac0(void* type, NativeSeg89Calls& calls)
{
    calls.initialize_fallback_resource_type_00b86a00(type);
    return type;
}

void* delete_native_resource_interface_00b86ae0(void* object, std::uint8_t flags, NativeSeg89Calls& calls)
{
    set_dword(object, 0, 0x00D63208u);
    if (flags & 1) calls.delete_00bf65ac(object);
    return object;
}

void replace_native_ref_slot_00b86b90(void* owner, NativeSeg89Calls& calls)
{
    set_pointer(owner, 0xc, calls.replace_vcall_10(pointer(owner, 0xc), 6, 0));
}

void rotate_native_type_tree_00b86d00(void* tree, void* node) noexcept
{
    void* const lifted = pointer(node, 0);
    set_pointer(node, 0, pointer(lifted, 8));
    if (*at(pointer(lifted, 8), 0x21) == 0) set_pointer(pointer(lifted, 8), 4, node);
    set_pointer(lifted, 4, pointer(node, 4));
    void* const head = pointer(tree, 4);
    if (node == pointer(head, 4)) {
        set_pointer(head, 4, lifted);
    } else {
        void* const parent = pointer(node, 4);
        if (node == pointer(parent, 8)) set_pointer(parent, 8, lifted);
        else set_pointer(parent, 0, lifted);
    }
    set_pointer(lifted, 8, node);
    set_pointer(node, 4, lifted);
}

void* native_type_tree_maximum_00b86df0(void* node) noexcept
{
    while (*at(pointer(node, 8), 0x21) == 0) node = pointer(node, 8);
    return node;
}

} // namespace bsp


// ===========================================================================
// Segment 75
// ===========================================================================

namespace bsp {

void* initialize_native_listbox_pool_slab_00a9acd0(void* raw, std::uint32_t index) noexcept
{
    return NativeGuiListboxPool::initialize_slab(raw, index);
}

void destroy_native_listbox_pool_00a9afd0(NativeGuiListboxPool& pool)
{
    pool.destroy();
}

void trim_native_listbox_pool_00a9b0b0(NativeGuiListboxPool& pool)
{
    pool.trim();
}

void free_native_listbox_pool_slot_00a9b150(NativeGuiListboxPool& pool, void* slot) noexcept
{
    pool.free(slot);
}

void free_native_listbox_slot_thunk_00a9b330(NativeGuiListboxPool& pool_00f8bc10, void* slot) noexcept
{
    free_native_listbox_pool_slot_00a9b150(pool_00f8bc10, slot);
}

void construct_native_listbox_pool_00a9b4a0(NativeGuiListboxPool& pool)
{
    pool.initialize();
}

void* allocate_native_listbox_pool_slot_00a9b580(NativeGuiListboxPool& pool)
{
    return pool.allocate();
}

void set_native_listbox_pool_table_capacity_00a9aeb0(NativeGuiPoolSlabTable& table, std::uint32_t capacity)
{
    native_gui_pool_table_set_capacity(table, capacity);
}

void grow_native_listbox_pool_table_00a9af00(NativeGuiPoolSlabTable& table)
{
    native_gui_pool_table_grow(table);
}

void push_native_listbox_pool_slab_00a9b250(NativeGuiPoolSlabTable& table, std::byte* const& slab)
{
    native_gui_pool_table_push_back(table, slab);
}

void reserve_native_listbox_pool_table_00a9b2d0(NativeGuiPoolSlabTable& table, std::uint32_t capacity)
{
    native_gui_pool_table_reserve(table, capacity);
}

void grow_native_string_map_pool_table_00a9ea60(NativeGuiPoolSlabTable& table)
{
    native_gui_pool_table_grow(table);
}

void push_native_string_map_pool_slab_00a9ee40(NativeGuiPoolSlabTable& table, std::byte* const& slab)
{
    native_gui_pool_table_push_back(table, slab);
}

namespace {

constexpr U32 kRowsFc = 0xfc;
constexpr U32 kRowsHead100 = 0x100;
constexpr U32 kRowsSize104 = 0x104;
constexpr U32 kSelectedContainer108 = 0x108;
constexpr U32 kSelectedNode10c = 0x10c;
constexpr U32 kListener114 = 0x114;
constexpr U32 kHighlightWidth124 = 0x124;

void* row_of(void* node) noexcept
{
    return pointer(node, 8);
}

// The inline 00A9CDC0 body: selection = end, then +60h(0).
void clear_selection(void* listbox, NativeGuiListboxCalls& calls)
{
    set_pointer(listbox, kSelectedContainer108, at(listbox, kRowsFc));
    set_pointer(listbox, kSelectedNode10c, pointer(listbox, kRowsHead100));
    calls.activate_vcall_60(listbox, 0);
}

float float_bits(U32 bits) noexcept
{
    float value;
    std::memcpy(&value, &bits, 4);
    return value;
}

// 00A9BB70's per-target tail.
void layout_target(void* listbox, void* target, std::uint8_t apply, NativeGuiListboxCalls& calls)
{
    if (!apply) return;
    *at(target, 0x1f0) = 0;
    const float width = single(listbox, kHighlightWidth124);
    // UCOMISS with 00D7A218 (0.0f), LAHF / TEST AH,44h / JNP: only an
    // ordered equal compare skips.
    if (width == 0.0f) return;
    const float* const size = calls.size_00aa6740(target);
    const float xy[2] = {width, size[1]};
    calls.set_position_vcall_58(target, xy);
    if (dword(target, 0x100) == 0) {
        set_single(target, 0x30, float_bits(0xBC19999Au));  // 00CF3F64, -0.009375f
        set_single(target, 0x34, 0.0f);
    }
}

} // namespace

void set_native_listbox_input_filter_00a9ac70(void* listbox, std::uint32_t routine) noexcept
{
    set_dword(listbox, 0x144, routine);
}

void close_native_listbox_paging_00a9bb30(void* listbox, NativeGuiListboxCalls& calls)
{
    if (*at(listbox, 0x148) == 0) return;
    *at(listbox, 0x148) = 0;
    calls.clear_list_00a9b3f0(at(listbox, 0x14c));
    calls.show_vcall_34(pointer(listbox, 0x15c), 0);
    calls.show_vcall_34(pointer(listbox, 0x160), 0);
}

bool layout_native_listbox_item_00a9bb70(void* listbox, void* row, std::uint8_t apply,
    NativeGuiListboxCalls& calls)
{
    if (calls.kind_vcall_5c(row) == 3) {
        layout_target(listbox, row, apply, calls);
        return true;
    }
    // The row's own child list at +64h {alloc, head +68h, size +6Ch}; the
    // checked-iterator reports cannot fire on this walk.
    void* const head = pointer(row, 0x68);
    for (void* node = pointer(head, 0); node != pointer(row, 0x68); node = pointer(node, 0)) {
        void* const child = row_of(node);
        if (calls.kind_vcall_5c(child) == 3) {
            layout_target(listbox, child, apply, calls);
            return true;
        }
    }
    return false;
}

void clear_native_listbox_items_00a9bf90(void* listbox, NativeGuiListboxCalls& calls)
{
    void* const listener = pointer(listbox, kListener114);
    set_pointer(listbox, kListener114, nullptr);
    while (dword(listbox, kRowsSize104) != 0) {
        void* node = pointer(pointer(listbox, kRowsHead100), 0);
        if (node == pointer(listbox, kRowsHead100)) calls.invalid_parameter_00bf6713();
        calls.show_vcall_34(row_of(node), 0);
        void* const head = pointer(listbox, kRowsHead100);
        node = pointer(head, 0);
        if (node == head) calls.invalid_parameter_00bf6713();
        if (node == pointer(listbox, kRowsHead100)) continue;
        set_pointer(pointer(node, 4), 0, pointer(node, 0));
        set_pointer(pointer(node, 0), 4, pointer(node, 4));
        calls.delete_00bf65ac(node);
        set_dword(listbox, kRowsSize104, dword(listbox, kRowsSize104) - 1);
    }
    void* const head = pointer(listbox, kRowsHead100);
    set_pointer(listbox, kSelectedContainer108, at(listbox, kRowsFc));
    set_pointer(listbox, kSelectedNode10c, head);
    set_pointer(listbox, 0x110, nullptr);
    set_pointer(listbox, kListener114, listener);
    calls.refresh_vcall_80(listbox, 0);
}

void select_native_listbox_row_by_data_00a9c860(void* listbox, std::uint32_t data,
    NativeGuiListboxCalls& calls)
{
    void* const end = pointer(listbox, kRowsHead100);
    set_pointer(listbox, kSelectedContainer108, at(listbox, kRowsFc));
    set_pointer(listbox, kSelectedNode10c, end);
    for (void* node = pointer(end, 0); node != end; node = pointer(node, 0)) {
        if (dword(row_of(node), 0xd8) != data) continue;
        if (*at(row_of(node), 0x77) == 0) {
            set_pointer(listbox, kSelectedContainer108, at(listbox, kRowsFc));
            set_pointer(listbox, kSelectedNode10c, node);
        }
        break;
    }
    calls.refresh_vcall_80(listbox, 0);
}

void select_native_listbox_index_silently_00a9c8f0(void* listbox, std::int32_t index,
    NativeGuiListboxCalls& calls)
{
    void* const listener = pointer(listbox, kListener114);
    set_pointer(listbox, kListener114, nullptr);
    calls.select_index_00a9c7c0(listbox, index);
    set_pointer(listbox, kListener114, listener);
}

void scroll_native_listbox_00a9cc50(void* listbox, std::int32_t count, NativeGuiListboxCalls& calls)
{
    void* node = pointer(listbox, kRowsHead100);
    if (count <= 0) return;
    for (std::int32_t remaining = count; remaining != 0; --remaining) {
        node = pointer(node, 4);
        if (node == pointer(listbox, kRowsHead100)) {
            calls.invalid_parameter_00bf6713();
            if (node == pointer(listbox, kRowsHead100)) calls.invalid_parameter_00bf6713();
        }
        void* const row = row_of(node);
        // FLD float, FMUL double 00D7A280 (0.5), FSTP double; FLD float, FADD
        // double, FSTP float.
        const double half = static_cast<double>(calls.size_00aa6740(row)[1]) * 0.5;
        const float y = static_cast<float>(static_cast<double>(calls.local_position_00aa67f0(row)[1]) + half);
        calls.set_local_y_00aa78f0(row, y);
    }
}

void clear_native_listbox_selection_00a9cdc0(void* listbox, NativeGuiListboxCalls& calls)
{
    clear_selection(listbox, calls);
}

void select_native_listbox_exclusive_00a9cdf0(void* listbox, void* other, void* third,
    NativeGuiListboxCalls& calls)
{
    if (other != listbox) clear_selection(other, calls);
    if (third != listbox) clear_selection(third, calls);
    calls.activate_vcall_60(listbox, 1);
}

void select_native_listbox_exclusive_00a9ce60(void* listbox, void* other, void* third,
    void* fourth, NativeGuiListboxCalls& calls)
{
    if (fourth != listbox) clear_selection(fourth, calls);
    select_native_listbox_exclusive_00a9cdf0(listbox, other, third, calls);
}

void reset_native_listbox_highlight_00a9cf30(void* listbox, std::uint8_t apply,
    std::uint32_t argument, NativeGuiListboxCalls& calls)
{
    set_single(listbox, kHighlightWidth124, 0.0f);
    if (!apply) return;
    void* const manager = calls.gui_manager_004c12b0();
    void* const highlight = calls.highlight_widget_00aa0f50(manager,
        static_cast<std::int32_t>(dword(listbox, 0x118)));
    void* const end = pointer(listbox, kRowsHead100);
    for (void* node = pointer(end, 0); node != end; node = pointer(node, 0)) {
        calls.fit_relative_00ac0820(row_of(node), highlight, 0.0f);
        // FLD +124h, FLD size.x, FCOMIP, JBE: raise only when strictly larger.
        if (calls.size_00aa6740(highlight)[0] > single(listbox, kHighlightWidth124))
            set_single(listbox, kHighlightWidth124, calls.size_00aa6740(highlight)[0]);
    }
    calls.set_highlight_width_00a9ceb0(listbox, single(listbox, kHighlightWidth124), argument);
}

void* construct_native_gui_list_00a9d010(void* list, NativeGuiListboxCalls& calls)
{
    set_pointer(list, 4, calls.buy_list_head_00a9b720(list));
    set_dword(list, 8, 0);
    return list;
}

void scroll_native_listbox_to_item_00a9d360(void* listbox, std::int32_t index, float step,
    std::uint8_t force, NativeGuiListboxCalls& calls)
{
    if (*at(listbox, 0x11d) == 0 && force == 0) {
        scroll_native_listbox_00a9cc50(listbox,
            static_cast<std::int32_t>(dword(listbox, kRowsSize104)) - index, calls);
        return;
    }
    float amount;
    if (step > float_bits(0x3A83126Fu)) {  // 00D7A23C, 0.001f; COMISS / JBE
        amount = step;
    } else {
        void* const first = pointer(pointer(listbox, kRowsHead100), 0);
        if (first == pointer(listbox, kRowsHead100)) calls.invalid_parameter_00bf6713();
        // FLD float, FMUL double 00D7A348 (0.25), FSTP float.
        amount = static_cast<float>(static_cast<double>(calls.size_00aa6740(row_of(first))[1]) * 0.25);
    }
    void* const end = pointer(listbox, kRowsHead100);
    std::int32_t position = 0;
    for (void* node = pointer(end, 0); node != end; node = pointer(node, 0), ++position) {
        void* const row = row_of(node);
        // 00D7A208 is -0.0f: rows before `index` move by -amount.
        const float delta = position < index ? float_bits(0x80000000u) - amount : amount;
        calls.set_local_y_00aa78f0(row, calls.local_position_00aa67f0(row)[1] + delta);
    }
}

void destroy_native_listbox_00a9e130(void* listbox, NativeGuiListboxCalls& calls)
{
    set_dword(listbox, 0, 0x00D5BBF8u);
    void* const strings = at(listbox, 0x16c);
    if (pointer(strings, 4)) {
        calls.destroy_string_range_00432050(pointer(strings, 4), pointer(strings, 8), strings, listbox);
        calls.delete_00bf65ac(pointer(strings, 4));
    }
    set_dword(strings, 4, 0);
    set_dword(strings, 8, 0);
    set_dword(strings, 0xc, 0);
    calls.destroy_list_storage_00a9b740(at(listbox, 0x14c));
    calls.destroy_list_storage_00a9b740(at(listbox, kRowsFc));
    calls.destroy_widget_base_00aa9730(listbox);
}

void* delete_native_listbox_00a9e210(void* listbox, std::uint8_t flags,
    NativeGuiListboxPool& pool_00f8bc10, NativeGuiListboxCalls& calls)
{
    destroy_native_listbox_00a9e130(listbox, calls);
    if (flags & 1) free_native_listbox_pool_slot_00a9b150(pool_00f8bc10, listbox);
    return listbox;
}

void uppercase_native_wide_string_00a9ec30(const LocaleTables& tables, LocaleTextRuntimeHost& host,
    NativeWideStringHeader& text)
{
    for (std::uint32_t i = 0; i < text.length_00; ++i) {
        char16_t* const unit = text.data_04 + i;
        *unit = locale_uppercase_00a9eba0(tables, host, *unit);
    }
}

std::int32_t find_first_of_native_wide_string_00a9ef60(const NativeWideStringHeader& text,
    const NativeWideStringHeader& set, std::int32_t start)
{
    const char16_t* const characters = set.data_04;
    if (!text.data_04 || text.length_00 == 0 || !characters) return -1;
    if (start < 0) start = 0;
    else if (static_cast<std::uint32_t>(start) > text.length_00) return -1;
    const auto span = std::wcscspn(reinterpret_cast<const wchar_t*>(text.data_04 + start),
        reinterpret_cast<const wchar_t*>(characters));
    const std::uint32_t index = static_cast<std::uint32_t>(span) + static_cast<std::uint32_t>(start);
    return index == text.length_00 ? -1 : static_cast<std::int32_t>(index);
}

void* copy_construct_native_string_record_00a9f460(void* fresh, const NativeString& source,
    NativeStringStorage& strings)
{
    auto* const text = static_cast<NativeString*>(fresh);
    set_dword(fresh, 0, 0);
    set_dword(fresh, 4, 0);
    if (static_cast<const void*>(text) != static_cast<const void*>(&source)) {
        resize_native_string_header_0041dd40(fresh, strings, source.length(), true);
        if (source.length() != 0)
            std::memcpy(pointer(fresh, 4), source.data(), dword(fresh, 0));  // _memcpy 00BF7680
    }
    set_dword(fresh, 8, 0);
    set_dword(fresh, 0xc, 0);
    return fresh;
}

void restart_native_gui_animated_value_00aa0e20(void* value, std::uint8_t forward,
    NativeGuiAnimatedValueCalls& calls)
{
    calls.stop_vcall_34(pointer(value, 0x50), 0);
    set_dword(value, 0x50, forward ? dword(value, 0x54) : dword(value, 0x58));
}

void set_native_gui_animated_value_target_00aa0e60(void* value, std::uint32_t target,
    NativeGuiAnimatedValueCalls& calls)
{
    calls.set_target_vcall_88(pointer(value, 0x50), target, 0, 1.0f);  // FLD1
    calls.notify_04(dword(value, 4), pointer(value, 0x50), 0);
}

} // namespace bsp

namespace bsp {
namespace {

NativeTaggedValue tagged(U32 type, const void* value) noexcept
{
    return {type, static_cast<U32>(reinterpret_cast<std::uintptr_t>(value))};
}
NativeTaggedValue tagged_bits(U32 type, U32 value) noexcept
{
    return {type, value};
}
NativeTaggedValue tagged_float(float value) noexcept
{
    U32 bits;
    std::memcpy(&bits, &value, 4);
    return {2, bits};
}

// One describer block: build the value and the default, compare with
// 00BD5680, write ({0, name}, value) when they differ.
void describe_value(void* writer, const char* name, NativeTaggedValue value, NativeTaggedValue fallback,
    NativeGuiListboxPropertyCalls& calls)
{
    if (!calls.tagged_value_equals_00bd5680(value, fallback))
        calls.write_vcall_0c(writer, tagged(0, name), value);
}

// The checked element access the Items loops repeat: stop at a null begin
// or i >= (signed (end - begin) >> 3) compared unsigned; report through
// 00BF6713 on the second (checked operator[]) test.
NativeString* checked_string(void* header, U32 index, NativeGuiListboxPropertyCalls& calls)
{
    auto* const begin = static_cast<NativeString*>(pointer(header, 4));
    if (!begin) return nullptr;
    const auto count = static_cast<U32>(static_cast<std::int32_t>(dword(header, 8) - dword(header, 4)) >> 3);
    if (index >= count) return nullptr;
    auto* const again = static_cast<NativeString*>(pointer(header, 4));
    if (!again || index >= static_cast<U32>(static_cast<std::int32_t>(dword(header, 8) - dword(header, 4)) >> 3))
        calls.invalid_parameter_00bf6713();
    return static_cast<NativeString*>(pointer(header, 4)) + index;
}

bool strings_non_empty(const void* header) noexcept
{
    if (!pointer(header, 4)) return false;
    return (static_cast<std::int32_t>(dword(header, 8) - dword(header, 4)) >> 3) != 0;
}

} // namespace

void describe_native_listbox_properties_00a9b7f0(void* listbox, void* writer,
    const char* empty_00f8bc48, NativeGuiListboxPropertyCalls& calls)
{
    calls.describe_widget_base_00aaaed0(listbox, writer);
    describe_value(writer, "DontMoveTheItems", tagged_bits(3, *at(listbox, 0x11c)), tagged_bits(3, 0), calls);
    describe_value(writer, "AutoControl", tagged_bits(3, *at(listbox, 0x11e)), tagged_bits(3, 0), calls);
    describe_value(writer, "CenterVertical", tagged_bits(3, *at(listbox, 0x11d)), tagged_bits(3, 0), calls);
    describe_value(writer, "LineDistance", tagged_float(single(listbox, 0x13c)), tagged_float(0.0f), calls);
    describe_value(writer, "HeightPlus", tagged_float(single(listbox, 0x128)), tagged_float(0.0f), calls);
    void* const strings = at(listbox, 0x16c);
    if (!strings_non_empty(strings)) return;
    calls.begin_table_vcall_04(writer, tagged(0, "Items"));
    for (U32 i = 0;; ++i) {
        NativeString* const text = checked_string(strings, i, calls);
        if (!text) break;
        const char* data = text->data();
        if (!data) data = empty_00f8bc48;
        calls.write_vcall_0c(writer, tagged_bits(1, i + 1), tagged(0, data));
    }
    calls.end_table_vcall_08(writer);
}

void read_native_listbox_properties_00a9e400(void* listbox, void* reader,
    NativeStringStorage& strings, NativeGuiListboxPropertyCalls& calls)
{
    calls.read_widget_base_00aaa710(listbox, reader);
    calls.read_vcall_0c(reader, tagged(0, "DontMoveTheItems"), tagged(3, at(listbox, 0x11c)), tagged_bits(3, 0));
    calls.read_vcall_0c(reader, tagged(0, "AutoControl"), tagged(3, at(listbox, 0x11e)), tagged_bits(3, 0));
    calls.read_vcall_0c(reader, tagged(0, "NewHighlightIndex"), tagged(1, at(listbox, 0x118)), tagged_bits(1, 1));
    calls.read_vcall_0c(reader, tagged(0, "CenterVertical"), tagged(3, at(listbox, 0x11d)), tagged_bits(3, 0));
    void* const header = at(listbox, 0x16c);
    auto* const end = static_cast<NativeString*>(pointer(header, 8));
    if (pointer(header, 4) > static_cast<void*>(end)) calls.invalid_parameter_00bf6713();
    auto* const begin = static_cast<NativeString*>(pointer(header, 4));
    if (static_cast<void*>(begin) > pointer(header, 8)) calls.invalid_parameter_00bf6713();
    std::uint32_t output[2];
    calls.erase_strings_004954f0(header, output, begin, end);
    if (calls.has_vcall_14(reader, tagged(0, "LineDistance")))
        calls.read_vcall_0c(reader, tagged(0, "LineDistance"), tagged(2, at(listbox, 0x13c)), tagged_float(0.0f));
    if (calls.has_vcall_14(reader, tagged(0, "HeightPlus")))
        calls.read_vcall_0c(reader, tagged(0, "HeightPlus"), tagged(2, at(listbox, 0x128)), tagged_float(0.0f));
    if (calls.has_vcall_14(reader, tagged(0, "Items"))) {
        calls.begin_table_vcall_04(reader, tagged(0, "Items"));
        for (U32 i = 1; calls.has_vcall_14(reader, tagged_bits(1, i)); ++i) {
            NativeString item;  // zeroed header at ESP+10h
            calls.read_vcall_10(reader, tagged_bits(1, i), tagged(0, &item));
            calls.append_string_00450540(header, item);
            // Inline release: 00419CC0 pool, 00BD1510(data, length + 1, 1).
            destroy_native_string_header_0041dd20(&item, strings);
        }
        calls.end_table_vcall_08(reader);
    }
    if (strings_non_empty(header)) {
        for (U32 i = 0;; ++i) {
            NativeString* const name = checked_string(header, i, calls);
            if (!name) break;
            void* const row = calls.find_child_by_name_00aa7e00(listbox, *name, 1);
            calls.append_row_00a9d750(listbox, row, nullptr, 0);
        }
    }
    calls.refresh_vcall_80(listbox, 1);
}

} // namespace bsp
