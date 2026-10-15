#include "bsp/native_enum_dictionary_lookup.hpp"

#include "bsp/entity_identity.hpp"
#include "bsp/native_lua_script_overrides.hpp"
#include "bsp/native_enum_node_pool.hpp"
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

std::uint32_t read_property_word(const void* owner, std::uint32_t offset) {
    const auto address = reinterpret_cast<std::uintptr_t>(owner) + offset;
    return *reinterpret_cast<const volatile std::uint32_t*>(address);
}

void write_property_word(void* owner, std::uint32_t value) {
    *static_cast<volatile std::uint32_t*>(owner) = value;
}

const char* read_property_data(const void* owner) {
    return reinterpret_cast<const char*>(read_property_word(owner, 4));
}

signed char read_property_byte(const char* data) {
    return *reinterpret_cast<const volatile signed char*>(data);
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
    const char* data = read_property_data(actual_key_header);
    std::uint32_t hash = read_property_word(actual_key_header, 0);
    const std::uint32_t step = (hash | 0x20u) >> 5;
    std::uint32_t remaining = hash;
    if (!data) data = actual_empty_00e177e4;
    while (remaining >= step) {
        const auto character = static_cast<unsigned char>(
            std::toupper(read_property_byte(data)));
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
    const void* actual_query_header, void* actual_bucket4,
    const char* actual_empty_00e177e4, const char* actual_empty_00e17654) {
    const auto captured_bucket = native_property_key_bucket_0043b760(
        actual_query_header, actual_empty_00e177e4);
    write_property_word(actual_bucket4, captured_bucket);
    void* node = reinterpret_cast<void*>(read_property_word(
        actual_map, 8u + captured_bucket * 4u));
    if (!node) return nullptr;
    const auto query_length = read_property_word(actual_query_header, 0);
    do {
        if (read_property_word(node, 0) == query_length) {
            const char* data = read_property_data(node);
            if (!data) data = actual_empty_00e177e4;
            const char* query = read_property_data(actual_query_header);
            if (!query) query = actual_empty_00e17654;
            if (query == data) return node;
            if (query && data && ::_stricmp(query, data) == 0) return node;
        }
        node = reinterpret_cast<void*>(read_property_word(node, 0x0c));
    } while (node);
    return nullptr;
}

void* find_native_property_map_node_0043b8b0(const void* actual_map,
    const void* actual_query_header, std::uint32_t& bucket,
    const char* actual_empty_00e177e4, const char* actual_empty_00e17654) {
    return find_native_property_map_node_0043b8b0(actual_map,
        actual_query_header, static_cast<void*>(&bucket),
        actual_empty_00e177e4, actual_empty_00e17654);
}

std::uint32_t publish_native_property_map_new_node_008f2930_fragment(
    void* actual_map, const void* actual_query_header,
    std::uint32_t lookup_bucket, std::uint32_t incoming_mapped_record_word,
    void* actual_initialized_property_pool_00e175b0,
    NativeStringRawPoolContext& actual_key_context) {
    const auto write_word = [](void* owner, std::uint32_t offset, std::uint32_t value) {
        std::memcpy(static_cast<char*>(owner) + offset, &value, sizeof value);
    };
    void* node = allocate_native_enum_node_004e7c00(
        actual_initialized_property_pool_00e175b0);
    if (node) {
        write_word(node, 0, 0);
        write_word(node, 4, 0);
    }
    if (node != actual_query_header) {
        resize_native_string_header_0041dd40(node, actual_key_context,
            load<std::uint32_t>(actual_query_header, 0), true);
        if (load<std::uint32_t>(actual_query_header, 0) != 0) {
            const auto length = load<std::uint32_t>(node, 0);
            const void* source = load<const void*>(actual_query_header, 4);
            void* destination = load<void*>(node, 4);
            std::memmove(destination, source, length);
        }
    }
    write_word(node, 8, incoming_mapped_record_word);
    const auto head_offset = 8u + lookup_bucket * 4u;
    void* old_head = load<void*>(actual_map, head_offset);
    std::memcpy(static_cast<char*>(node) + 0x0c, &old_head, sizeof old_head);
    std::memcpy(static_cast<char*>(actual_map) + head_offset, &node, sizeof node);
    write_word(actual_map, 4, load<std::uint32_t>(actual_map, 4) + 1u);
    return load<std::uint32_t>(actual_map, 4);
}
} // namespace bsp
