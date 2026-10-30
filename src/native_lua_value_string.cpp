#include "bsp/native_lua_value_string.hpp"
#include "bsp/native_lua_objects.hpp"
#include "bsp/cc13_seg87.hpp"
#include "bsp/native_render_batch_keys.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_float_format.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <intrin.h>
extern "C" {
#include <lua.h>
}

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Lua value-string construction requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4 &&
    sizeof(Word) == 4 && sizeof(int) == 4 && sizeof(float) == 4 && sizeof(lua_Number) == 8);
static_assert(offsetof(NativeLuaObjectStorage, owner_00) == 0 &&
    offsetof(NativeLuaObjectStorage, kind_04) == 4 &&
    offsetof(NativeLuaObjectStorage, index_08) == 8 &&
    offsetof(NativeLuaStateStorage, state_04) == 4);

struct ExpectedSpan { std::uintptr_t address; Word extent; };
ExpectedSpan expected(NativeLuaValueStringRole role) {
    switch (role) {
    case NativeLuaValueStringRole::true_value: return {0x00ce4378u, 5u};
    case NativeLuaValueStringRole::false_value: return {0x00ce4370u, 6u};
    case NativeLuaValueStringRole::pointer_value: return {0x00d62c34u, 4u};
    case NativeLuaValueStringRole::integer_format: return {0x00ce3a34u, 3u};
    }
    throw std::invalid_argument("Lua value-string context has an unknown RO role");
}
Word word(const void* base, Word offset) noexcept {
    Word value;
    const auto address = reinterpret_cast<std::uintptr_t>(base) + offset;
    _ReadWriteBarrier();
    std::memcpy(&value, reinterpret_cast<const void*>(address), sizeof value);
    _ReadWriteBarrier();
    return value;
}
struct LuaLocation { lua_State* state; std::int32_t index; };
LuaLocation current_location(const NativeLuaObjectStorage& object) noexcept {
    const Word owner = word(&object, 0u);
    const Word state = word(reinterpret_cast<const void*>(static_cast<std::uintptr_t>(owner)), 4u);
    const Word index_bits = word(&object, 8u); // AFTER CURRENT owner state
    std::int32_t index;
    std::memcpy(&index, &index_bits, sizeof index);
    return {reinterpret_cast<lua_State*>(static_cast<std::uintptr_t>(state)), index};
}

// These are call-boundary bridges, not a Lua/CRT implementation. Consume the
// linked cdecl API's REAL x87 return immediately; do not capture a double or
// pre-read the current conversion word. BF7420 receives its address in ECX.
__declspec(noinline) std::int32_t current_number_integer(lua_State* state,
    std::int32_t index, const volatile Word* conversion_word) {
    float narrowed;
    std::int32_t result;
    __asm {
        push index
        push state
        call lua_tonumber
        add esp, 8
        fstp narrowed
        fld narrowed
        mov ecx, conversion_word
        call native_crt_truncate_st0_00bf7420
        mov result, eax
    }
    return result;
}
__declspec(noinline) float current_number_float(lua_State* state, std::int32_t index) {
    float narrowed;
    __asm {
        push index
        push state
        call lua_tonumber
        add esp, 8
        fstp narrowed
    }
    return narrowed; // reload the actual FLOAT32 for the float-valued call
}
} // namespace

NativeLuaValueStringContext::NativeLuaValueStringContext(NativeStringRawPoolContext& strings,
    const volatile std::uint32_t& conversion,
    const std::array<NativeLuaTypeNameSpan, 11>& type_spans,
    const std::array<NativeLuaValueStringSpan, 4>& value_spans)
    : strings_(strings), conversion_(conversion), type_spans_(type_spans),
      value_spans_(value_spans), type_names_(strings, type_spans_),
      integer_{strings, value_spans_[3].data, value_spans_[3].extent_including_nul} {
    for (std::size_t i = 0; i < value_spans_.size(); ++i) {
        const auto& span = value_spans_[i];
        const auto role = static_cast<NativeLuaValueStringRole>(i);
        const auto actual = expected(role);
        if (span.role != role || span.original_address != actual.address ||
            span.extent_including_nul != actual.extent ||
            reinterpret_cast<std::uintptr_t>(span.data) != actual.address)
            throw std::invalid_argument("Lua value-string requires exact actual RO span metadata");
    }
}

void* construct_native_lua_value_string_00b69130(const NativeLuaObjectStorage& object,
    void* fresh, const NativeLuaValueStringContext& context) {
    const Word initial_kind = word(&object, 4u);
    if (initial_kind == 2u) {
        const auto at = current_location(object);
        if (lua_type(at.state, at.index) == LUA_TNIL)
            return construct_native_string_header_0041e870(fresh, context.strings_,
                context.type_spans_[1].data); // actual CE4380
    }
    if (native_lua_is_boolean_00b66000(object)) {
        const auto at = current_location(object); // CURRENT after predicate
        const bool value = lua_toboolean(at.state, at.index) != 0;
        return construct_native_string_header_0041e870(fresh, context.strings_,
            context.value_spans_[value ? 0u : 1u].data);
    }
    if (native_lua_is_integer_number_00b66a60(object)) {
        const auto at = current_location(object); // CURRENT after predicate
        const auto value = current_number_integer(at.state, at.index, &context.conversion_);
        return construct_native_string_from_int_004260b0(fresh, value, context.integer_);
    }
    if (native_lua_is_number_00b66050(object)) {
        const auto at = current_location(object); // no reuse from integer predicate
        const float value = current_number_float(at.state, at.index);
        return construct_native_string_float_header_0043bdf0(fresh, context.strings_, value);
    }
    if (native_lua_is_string_00b660a0(object)) {
        const char* const text = native_lua_string_00b662b0(object);
        return construct_native_string_header_0041e870(fresh, context.strings_, text);
    }
    if (native_lua_is_table_00b661b0(object))
        return construct_native_string_header_0041e870(fresh, context.strings_,
            context.type_spans_[6].data); // actual CF8344
    if (native_lua_is_lightuserdata_00b66160(object))
        return construct_native_string_header_0041e870(fresh, context.strings_,
            context.value_spans_[2].data); // actual D62C34
    if (native_lua_is_function_00b66200(object))
        return construct_native_string_header_0041e870(fresh, context.strings_,
            context.type_spans_[7].data); // actual D04D44
    return construct_native_lua_object_type_name_00b68550(object, fresh, context.type_names_);
}
} // namespace bsp
