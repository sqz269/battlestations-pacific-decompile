#include "bsp/native_enum_dictionary_word_getters.hpp"
#include "bsp/native_enum_dictionary_lookup.hpp"
#include <cstring>

namespace bsp {
namespace {
using Lookup = const void* (*)(const void*, const NativeString&, std::uint32_t&,
    const char*, const char*);

std::uint32_t read_word(const void* owner, const char* key,
    NativeStringRawPoolContext& strings, const char* node_empty,
    const char* key_empty, Lookup lookup) {
    NativeString temporary;
    construct_native_string_header_0041e870(&temporary, strings, key);
    std::uint32_t bucket;
    const void* node = lookup(static_cast<const char*>(owner) + 4,
        temporary, bucket, node_empty, key_empty);
    std::uint32_t word;
    std::memcpy(&word, static_cast<const char*>(node) + 8, sizeof word);
    destroy_native_string_header_0041dd20(&temporary, strings);
    return word;
}
} // namespace

std::uint32_t read_native_enum_table_word_0048e960(const void* owner,
    const char* key, NativeStringRawPoolContext& strings,
    const char* node_empty, const char* key_empty) {
    return read_word(owner, key, strings, node_empty, key_empty,
        find_native_enum_table_node_0048d4e0);
}
std::uint32_t read_native_enum_symbol_word_0048e840(const void* owner,
    const char* key, NativeStringRawPoolContext& strings,
    const char* node_empty, const char* key_empty) {
    return read_word(owner, key, strings, node_empty, key_empty,
        find_native_enum_symbol_node_0048d480);
}
} // namespace bsp
