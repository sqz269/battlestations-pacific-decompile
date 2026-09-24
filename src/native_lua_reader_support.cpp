#include "bsp/native_lua_reader_support.hpp"
#include "bsp/lua_numeric.hpp"
#include <cstring>
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Lua reader support requires MSVC Win32.
#endif

namespace bsp {
namespace {
std::int32_t signed_word(std::uint32_t bits) noexcept {
    std::int32_t value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}
} // namespace

NativeLuaObjectStorage* copy_construct_native_lua_object_00b66fa0(
    void* fresh, const NativeLuaObjectStorage& source) noexcept {
    auto* const result = ::new (fresh) NativeLuaObjectStorage;
    volatile auto& destination = *result;
    const volatile auto& current_source = source;
    destination.owner_00 = current_source.owner_00;
    destination.kind_04 = current_source.kind_04;
    destination.index_08 = current_source.index_08;
    destination.opaque_0c = current_source.opaque_0c;
    const auto tracked = current_source.tracked_10;
    auto* const owner = destination.owner_00; // B66FC1 precedes tracked store.
    destination.tracked_10 = tracked;
    if (tracked != 0) {
        volatile auto& current_owner = *owner;
        const auto index = signed_word(static_cast<std::uint32_t>(current_owner.stack_offset_0c) +
            static_cast<std::uint32_t>(destination.index_08));
        if (index >= current_owner.high_water_4c4)
            current_owner.high_water_4c4 = signed_word(static_cast<std::uint32_t>(index) + 1u);
        volatile auto& slot = current_owner.slots_14[index];
        const auto count = slot.count_14;
        slot.references_00[count] = result;
        slot.count_14 = signed_word(static_cast<std::uint32_t>(slot.count_14) + 1u);
    }
    return result;
}

NativeLuaObjectStorage* lookup_native_lua_reader_child_00bd5790(
    void* fresh, NativeLuaObjectStorage& parent,
    std::uint32_t key_kind, std::uint32_t key_bits) {
    auto* const output = construct_native_lua_object_00b65f50(fresh);
    NativeLuaObjectStorage temporary;
    bool temporary_constructed = false;
    try {
        NativeLuaObjectStorage* returned;
        if (key_kind == 0) {
            returned = native_lua_get_by_name_00b67800(parent, &temporary,
                reinterpret_cast<const char*>(key_bits));
        } else if (key_kind == 1) {
            returned = native_lua_get_by_index_00b67720(parent, &temporary, signed_word(key_bits));
        } else if (key_kind == 2) {
            float number;
            std::memcpy(&number, &key_bits, sizeof number);
            returned = native_lua_get_by_index_00b67720(parent, &temporary,
                lua_float_index_00bd5790(number));
        } else {
            return output;
        }
        temporary_constructed = true; // Native states1/2/3 after getter return.
        assign_native_lua_object_00b67690(*output, *returned);
        temporary_constructed = false; // Native state0 BEFORE temporary destroy.
        destroy_native_lua_object_00b67700(temporary);
    } catch (...) {
        // DFFBBC map: states1/2/3 destroy temporary, then state0 destroys
        // the guarded output. Lua longjmp remains the raw provider boundary.
        if (temporary_constructed) destroy_native_lua_object_00b67700(temporary);
        destroy_native_lua_object_00b67700(*output);
        throw;
    }
    return output;
}
} // namespace bsp
