#include "bsp/native_scene_class_name_lookup.hpp"

#include "bsp/native_scene_name_hash_iterator.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"

#include <cstring>

namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == sizeof(U));
U address(const void* p) noexcept { return reinterpret_cast<U>(p); }
void* pointer(U p) noexcept { return reinterpret_cast<void*>(p); }
const char* text(U p) noexcept { return reinterpret_cast<const char*>(p); }
U read(U p, U offset = 0) noexcept {
    return *reinterpret_cast<const volatile U*>(p + offset);
}
std::uint8_t read_byte(U p) noexcept {
    return *reinterpret_cast<const volatile std::uint8_t*>(p);
}
void write(U p, U value) noexcept { *reinterpret_cast<volatile U*>(p) = value; }

bool compare_cached(U query_data, U query_length, U node_text) {
    if (query_data == 0) {
        if (node_text == 0) return true;
        const U after_first = node_text + 1u;
        U cursor = node_text;
        for (;;) {
            const auto character = read_byte(cursor);
            ++cursor;
            if (character == 0) break;
        }
        return cursor - after_first == 0;
    }
    if (node_text == 0) return query_length == 0;
    return ::_stricmp(text(query_data), text(node_text)) == 0;
}

void release_cached(U data, U length, NativeStringRawPoolContext& pool) {
    if (data != 0) {
        const U bytes = length + 1u;
        auto* const owner = native_string_pool_get_or_create_00419cc0(
            pool.actual_published_01090aa8,
            pool.actual_manager_publication_01090aa0);
        return_native_string_pool_00bd1510(owner, pointer(data), bytes,
            pool.actual_small_returns_disabled_01090aa4);
    }
}
} // namespace

std::uint32_t find_native_scene_class_id_00469690(void* actual_scene_database,
    const NativeSceneClassNameLookupFrame& frame, NativeStringRawPoolContext& pool,
    const char* actual_empty_00e18560) {
    const U iterator = address(frame.actual_iterator12);
    write(iterator, address(actual_scene_database) + 0x34u);
    begin_native_scene_name_hash_00466bb0(pointer(iterator));
    const U key = read(address(frame.actual_parameter4));
    void* const header = frame.actual_key_header8;
    construct_native_string_header_0041e870(header, pool, text(key));
    U node = read(iterator, 4);
    const U data = read(address(header), 4);
    const U length = read(address(header));
    for (;;) {
        if (node == 0) {
            release_cached(data, length, pool);
            return 0;
        }
        U node_text = read(node, 4);
        if (node_text == 0) node_text = address(actual_empty_00e18560);
        if (compare_cached(data, length, node_text)) {
            const U descriptor = read(node, 8);
            const U result = read(descriptor);
            release_cached(data, length, pool);
            return result;
        }
        (void)next_native_scene_name_hash_00466c00(pointer(iterator));
        node = read(iterator, 4);
    }
}

} // namespace bsp
