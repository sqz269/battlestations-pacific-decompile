#include "bsp/native_input_configuration_load.hpp"
#include <cstdlib>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native input configuration loading requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
Word address(const void* p) noexcept { return reinterpret_cast<Word>(p); }
template<class T = Word> volatile T& field(Word p, Word offset = 0) noexcept {
    return *reinterpret_cast<volatile T*>(p + offset);
}
Word word_count(Word begin, Word end) noexcept {
    return static_cast<Word>(static_cast<std::int32_t>(end - begin) >> 2);
}
template<class Query>
auto query_index(NativeLuaObjectStorage& table, std::int32_t index, Query query) {
    NativeLuaObjectStorage temporary;
    auto* const returned = native_lua_get_by_index_00b67720(table, &temporary, index);
    decltype(query(*returned)) value;
    try {
        value = query(*returned);
    } catch (...) {
        destroy_native_lua_object_00b67700(temporary);
        throw;
    }
    // Native marks the temporary unowned before calling its destructor. A
    // normal destruction exception must not cause the same object to retry.
    destroy_native_lua_object_00b67700(temporary);
    return value;
}
} // namespace

void* decode_native_input_descriptor_006974f0(void* configuration, void* output,
    NativeLuaObjectStorage& table, std::uint8_t swap,
    const volatile std::uint8_t& x360comp, const bool& conversion) {
    const auto integer = [&conversion](const NativeLuaObjectStorage& object) {
        return native_lua_integer_00b66290(object, conversion);
    };
    const auto first = query_index(table, 1, integer);
    auto code = query_index(table, 2, integer);
    const auto third_is_nil = query_index(table, 3,
        [](const NativeLuaObjectStorage& object) { return native_lua_is_nil_00b65fb0(object); });
    const auto third = third_is_nil ? 0 : query_index(table, 3, integer);
    const auto owner = address(configuration);
    if (x360comp != 0 &&
        (field<std::uint8_t>(owner, 0x4c9) != 0 || field<std::uint8_t>(owner, 0x4cc) != 0) &&
        swap != 0) {
        auto row = field(owner, 0x4d4);
        const auto outer = owner + 0x4d0u;
        if (row > field(outer, 8)) _invalid_parameter_noinfo();
        while (true) {
            const auto end = field(outer, 8);
            if (field(outer, 4) > end) _invalid_parameter_noinfo();
            // Native CMP EBP,EBP makes697631's owner-mismatch call unreachable.
            if (row == end) break;
            if (row >= field(outer, 8)) _invalid_parameter_noinfo();
            const auto initial = field(row, 4);
            if (!initial || word_count(initial, field(row, 8)) == 0)
                _invalid_parameter_noinfo();
            const auto begin = field(row, 4);
            if (static_cast<Word>(code) == field(begin)) {
                if (!begin || word_count(begin, field(row, 8)) <= 1)
                    _invalid_parameter_noinfo();
                code = field<std::int32_t>(field(row, 4), 4);
                break;
            }
            if (!begin || word_count(begin, field(row, 8)) <= 1)
                _invalid_parameter_noinfo();
            const auto current = field(row, 4);
            if (static_cast<Word>(code) == field(current, 4)) {
                if (!current || word_count(current, field(row, 8)) == 0)
                    _invalid_parameter_noinfo();
                code = field<std::int32_t>(field(row, 4));
                break;
            }
            if (row >= field(outer, 8)) _invalid_parameter_noinfo();
            row += 0x10u;
        }
    }
    const auto result = address(output);
    field<std::int32_t>(result, 0xc) = code;
    field<std::int32_t>(result, 4) = third;
    field<std::int32_t>(result) = first;
    field(result, 8) = 0;
    field<std::uint8_t>(result, 0x10) = 0;
    return output;
}

void load_native_input_configuration_script_prefix_00698a10(
    void* configuration, NativeInputConfigurationLoadServices& services) {
    const auto owner = address(configuration);
    clear_native_input_configuration_00698730(configuration, services.cleanup);
    auto& lua = *static_cast<NativeLuaStateStorage*>(configuration);
    if (field<std::uint8_t>(owner, 0x4c8) == 0)
        open_native_lua_state_00b6a020(lua, 1, services.strings, services.bootstrap);

    NativeString command;
    command.assign_0041e870(services.strings,
        services.x360comp_00f88a30 != 0 ? "X360COMP=true" : "X360COMP=false");
    try {
        (void)execute_native_lua_string_00b66c60(lua, command);
    } catch (...) {
        destroy_native_string_header_0041dd20(&command, services.strings);
        throw;
    }
    destroy_native_string_header_0041dd20(&command, services.strings);

    NativeString path;
    path.resize_0041dd40(services.strings, 0x1d, true);
    auto* const destination = field<char*>(address(&path), 4);
    if (destination) {
        const auto bytes = field(address(&path)) + 1u;
        if (bytes) std::memcpy(destination, "Scripts\\datatables\\Inputs.lua", bytes);
    }
    try {
        run_native_lua_file_00b69d40(lua, path, 0, services.strings, services.files);
    } catch (...) {
        destroy_native_string_header_0041dd20(&path, services.strings);
        throw;
    }
    destroy_native_string_header_0041dd20(&path, services.strings);
    field<std::uint8_t>(owner, 0x4c8) = 1;
}
} // namespace bsp
