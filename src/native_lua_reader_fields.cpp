#include "bsp/native_lua_reader_fields.hpp"
#include "bsp/native_lua_field_defaults.hpp"
#include "bsp/native_lua_reader_support.hpp"

namespace bsp {
void read_native_lua_field_00bd6830(NativeLuaReaderStorage& reader,
    std::uint32_t key_kind, std::uint32_t key_bits, void* actual_field_pair,
    NativeLuaReaderFieldScratch& scratch, NativeLuaFieldValueBindings& bindings) {
    auto& parent = reader.current_raw();
    lookup_native_lua_reader_child_00bd5790(&scratch.lookup, parent, key_kind, key_bits);
    store_native_lua_field_value_00bd63b0(scratch.lookup, actual_field_pair,
        scratch.value, bindings);
    destroy_native_lua_object_00b67700(scratch.lookup);
}

void read_native_lua_field_or_default_00bd68d0(NativeLuaReaderStorage& reader,
    std::uint32_t key_kind, std::uint32_t key_bits, void* actual_field_pair,
    const void* actual_fallback_pair, NativeLuaReaderFieldScratch& scratch,
    NativeLuaFieldValueBindings& bindings) {
    auto& parent = reader.current_raw();
    lookup_native_lua_reader_child_00bd5790(&scratch.lookup, parent, key_kind, key_bits);
    if (native_lua_is_nil_00b65fb0(scratch.lookup)) {
        store_native_lua_field_default_00bd61c0(actual_field_pair,
            actual_fallback_pair, bindings.strings);
    } else {
        store_native_lua_field_value_00bd63b0(scratch.lookup, actual_field_pair,
            scratch.value, bindings);
    }
    destroy_native_lua_object_00b67700(scratch.lookup);
}
} // namespace bsp
