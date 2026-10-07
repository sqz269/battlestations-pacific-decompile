#include "bsp/native_enum_dictionary_membership.hpp"
#include "bsp/native_enum_dictionary_lookup.hpp"

namespace bsp {

bool contains_native_enum_symbol_0048e8d0(const void* owner, const char* key,
    NativeStringRawPoolContext& strings, const char* node_empty,
    const char* key_empty) {
    NativeString temporary;
    construct_native_string_header_0041e870(&temporary, strings, key);
    std::uint32_t bucket;
    const void* node = find_native_enum_symbol_node_0048d480(
        static_cast<const char*>(owner) + 4, temporary, bucket,
        node_empty, key_empty);
    const bool found = node != nullptr;
    destroy_native_string_header_0041dd20(&temporary, strings);
    return found;
}

} // namespace bsp
