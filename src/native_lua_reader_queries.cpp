#include "bsp/native_lua_reader_queries.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Lua reader queries require MSVC Win32.
#endif

namespace bsp {

bool native_lua_reader_has_key_00bd5eb0(NativeLuaReaderStorage& reader,
    NativeLuaObjectStorage& scratch, std::uint32_t tag, std::uint32_t bits) {
    lookup_native_lua_reader_child_00bd5790(&scratch, reader.current_raw(), tag, bits);
    const bool present = !native_lua_is_nil_00b65fb0(scratch);
    destroy_native_lua_object_00b67700(scratch);
    return present;
}

void native_lua_reader_enumerate_keys_00bd5f50(NativeLuaReaderStorage& reader,
    NativeLuaReaderKeys& output, NativeLuaReaderQueryScratch& scratch,
    const bool& crt_sse2_conversion) {
    output.count_7d0 = 0;
    construct_native_lua_object_00b65f50(&scratch.table);
    construct_native_lua_object_00b65f50(&scratch.key);
    construct_native_lua_object_00b65f50(&scratch.value);
    assign_native_lua_object_00b67690(scratch.table, reader.current_raw());
    native_lua_iterate_first_00b67080(scratch.table, scratch.key, scratch.value);
    while (!native_lua_is_unbound_00b66420(scratch.key)) {
        if (native_lua_is_string_00b660a0(scratch.key)) {
            const char* const text = native_lua_string_00b662b0(scratch.key);
            const auto index = output.count_7d0;
            output.keys[index].tag = 0;
            output.keys[index].bits = reinterpret_cast<std::uint32_t>(text);
            output.count_7d0 = output.count_7d0 + 1;
        } else if (native_lua_is_integer_number_00b66a60(scratch.key)) {
            const auto integer = native_lua_integer_00b66290(scratch.key, crt_sse2_conversion);
            const auto index = output.count_7d0;
            output.keys[index].tag = 1;
            output.keys[index].bits = static_cast<std::uint32_t>(integer);
            output.count_7d0 = output.count_7d0 + 1;
        } else if (native_lua_is_number_00b66050(scratch.key)) {
            // Native606C spills ST0; 6076 reloads it; 607F spills again before
            // the tag/payload stores. Keep the float transfers on x87.
            const float number = native_lua_number_00b66270(scratch.key);
            const auto index = output.count_7d0;
            std::uint32_t bits;
            __asm {
                fld number
                fstp bits
            }
            output.keys[index].tag = 2;
            output.keys[index].bits = bits;
            output.count_7d0 = output.count_7d0 + 1;
        }
        native_lua_iterate_next_00b67190(scratch.table, scratch.key, scratch.value);
    }
    destroy_native_lua_object_00b67700(scratch.value);
    destroy_native_lua_object_00b67700(scratch.key);
    destroy_native_lua_object_00b67700(scratch.table);
}

} // namespace bsp
