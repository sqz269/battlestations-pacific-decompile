#include "bsp/native_enum_dictionary_lookup.hpp"

#include "bsp/entity_identity.hpp"
#include "bsp/native_lua_script_overrides.hpp"
#include <cctype>
#include <cstring>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4, "These readers borrow actual Win32 storage");
static_assert(sizeof(NativeString) == 8, "The query uses an actual eight-byte header");

template<class T> T load(const void* owner, std::uint32_t offset) {
    T value;
    std::memcpy(&value, static_cast<const char*>(owner) + offset, sizeof value);
    return value;
}

std::uint32_t bucket_for(const NativeString& key, const char* empty) {
    const char* data = key.data();
    std::uint32_t hash = key.length();
    const std::uint32_t step = (hash | 0x20u) >> 5;
    std::uint32_t remaining = hash;
    if (!data) data = empty;
    while (remaining >= step) {
        const auto character = static_cast<unsigned char>(
            std::toupper(static_cast<signed char>(*data)));
        hash ^= std::uint32_t(character) + (hash << 5) + (hash >> 2);
        remaining -= step;
        ++data; // Native advances one byte even when step is greater than one.
    }
    return hash & 0x3fu;
}

const void* find_node(const void* receiver, const NativeString& key,
    std::uint32_t& bucket, const char* node_empty, const char* key_empty,
    std::uint32_t (*hash)(const NativeString&, const char*)) {
    bucket = hash(key, node_empty);
    const void* node = load<const void*>(receiver, 8u + bucket * 4u);
    if (!node) return nullptr;
    const auto query_length = key.length();
    do {
        if (load<std::uint32_t>(node, 0) == query_length) {
            const char* data = load<const char*>(node, 4);
            if (!data) data = node_empty;
            const char* query = native_string_data_or_00419ca0(key, key_empty);
            if (compare_insensitive_00438e10(query, data) == 0) return node;
        }
        node = load<const void*>(node, 0x0c);
    } while (node);
    return nullptr;
}
} // namespace

std::uint32_t native_property_key_bucket_0043b760(const void* actual_key_header,
    const char* actual_empty_00e177e4) {
    const char* data = load<const char*>(actual_key_header, 4);
    std::uint32_t hash = load<std::uint32_t>(actual_key_header, 0);
    const std::uint32_t step = (hash | 0x20u) >> 5;
    std::uint32_t remaining = hash;
    if (!data) data = actual_empty_00e177e4;
    while (remaining >= step) {
        const auto character = static_cast<unsigned char>(
            std::toupper(static_cast<signed char>(*data)));
        hash ^= std::uint32_t(character) + (hash << 5) + (hash >> 2);
        remaining -= step;
        ++data;
    }
    return hash & 0x3fu;
}

std::uint32_t native_enum_symbol_bucket_004895b0(const NativeString& key,
    const char* empty) {
    return bucket_for(key, empty);
}
std::uint32_t native_enum_table_bucket_00489610(const NativeString& key,
    const char* empty) {
    return bucket_for(key, empty);
}
const void* find_native_enum_symbol_node_0048d480(const void* receiver,
    const NativeString& key, std::uint32_t& bucket, const char* node_empty,
    const char* key_empty) {
    return find_node(receiver, key, bucket, node_empty, key_empty,
        native_enum_symbol_bucket_004895b0);
}
const void* find_native_enum_table_node_0048d4e0(const void* receiver,
    const NativeString& key, std::uint32_t& bucket, const char* node_empty,
    const char* key_empty) {
    return find_node(receiver, key, bucket, node_empty, key_empty,
        native_enum_table_bucket_00489610);
}

void* find_native_property_map_node_0043b8b0(const void* actual_map,
    const void* actual_query_header, std::uint32_t& bucket,
    const char* actual_empty_00e177e4, const char* actual_empty_00e17654) {
    bucket = native_property_key_bucket_0043b760(
        actual_query_header, actual_empty_00e177e4);
    void* node = load<void*>(actual_map, 8u + bucket * 4u);
    if (!node) return nullptr;
    const auto query_length = load<std::uint32_t>(actual_query_header, 0);
    do {
        if (load<std::uint32_t>(node, 0) == query_length) {
            const char* data = load<const char*>(node, 4);
            if (!data) data = actual_empty_00e177e4;
            const char* query = load<const char*>(actual_query_header, 4);
            if (!query) query = actual_empty_00e17654;
            if (query == data) return node;
            if (query && data && ::_stricmp(query, data) == 0) return node;
        }
        node = load<void*>(node, 0x0c);
    } while (node);
    return nullptr;
}
} // namespace bsp
