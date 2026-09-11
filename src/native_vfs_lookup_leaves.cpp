#include "bsp/native_vfs_lookup_leaves.hpp"

#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_vfs_date_leaf_providers.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native VFS lookup leaves require MSVC Win32.
#endif

namespace bsp {
namespace {
const void* at(const void* p, std::uint32_t offset) noexcept {
    return reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
void* at(void* p, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
std::uint32_t word(const void* p, std::uint32_t offset = 0) noexcept {
    return *static_cast<const volatile std::uint32_t*>(at(p, offset));
}
void* pointer(const void* p, std::uint32_t offset = 0) noexcept {
    return reinterpret_cast<void*>(word(p, offset));
}
void invalid(const SingletonLifetimeCallbacks& crt) {
    crt.invalid_parameter(crt.context);
}
bool contains_file_store(void* store, const void* name,
    const SingletonLifetimeCallbacks& crt) {
    auto* const tree = at(store, 0x14);
    std::uint32_t iterator[2];
    find_native_file_store_name_00be5a50(tree, iterator, name, crt);
    auto* const returned_owner = pointer(iterator);
    auto* const captured_head = pointer(tree, 4);
    if (!returned_owner || returned_owner != tree) invalid(crt);
    return pointer(iterator, 4) != captured_head;
}
std::uint32_t msar_row_count(const void* provider) noexcept {
    const auto begin = word(provider, 0x1c);
    if (begin == 0) return 0;
    const auto difference = word(provider, 0x20) - begin;
    return static_cast<std::uint32_t>(static_cast<std::int32_t>(difference) / 0x18);
}
} // namespace

bool contains_native_file_store_file_00be5c00(void* store, const void* name,
    const SingletonLifetimeCallbacks& crt) {
    return contains_file_store(store, name, crt);
}

bool probe_native_file_store_name_00be5c40(void* store, const void* name,
    const SingletonLifetimeCallbacks& crt) {
    return contains_file_store(store, name, crt);
}

void* copy_native_file_store_name_00be6040(void* output, const void* name,
    ActualNativeStringPoolStorage& strings) {
    // These native inline instructions have the same current-header ordering
    // as the already-complete 426060 body, including identity and failure.
    return copy_construct_native_string_header_00426060(output, name, strings);
}

void* copy_native_package_name_00bb5540(void* output, const void* name,
    ActualNativeStringPoolStorage& strings) {
    return copy_construct_native_string_header_00426060(output, name, strings);
}

bool contains_native_mpkg_entry_00bb8e00(const void* state, const void* name) {
    std::uint32_t index = 0;
    if (static_cast<std::int32_t>(word(state, 0x2c)) <= 0) return false;
    std::uint32_t offset = 0;
    do {
        auto* row = at(pointer(state, 0x28), offset);
        const auto row_length = word(row);
        const auto name_length = word(name);
        if (name_length == row_length) {
            if (name_length == 0) return true;
            const auto* row_data = static_cast<const char*>(pointer(row, 4));
            const auto* name_data = static_cast<const char*>(pointer(name, 4));
            if (_stricmp(name_data, row_data) == 0) return true;
        }
        ++index;
        offset += 0x24;
    } while (static_cast<std::int32_t>(index) < static_cast<std::int32_t>(word(state, 0x2c)));
    return false;
}

bool contains_native_mpkg_file_00bb8e80(const void* provider, const void* name) {
    return contains_native_mpkg_entry_00bb8e00(pointer(provider, 0x14), name);
}

std::uint32_t find_native_msar_file_index_00bba650(const void* provider,
    const void* name, const SingletonLifetimeCallbacks& crt) {
    std::uint32_t index = 0;
    std::uint32_t offset = 0;
    for (;;) {
        if (index >= msar_row_count(provider)) return 0xffffffffu;
        if (index >= msar_row_count(provider)) invalid(crt);
        auto* row = at(pointer(provider, 0x1c), offset);
        const auto row_length = word(row);
        const auto name_length = word(name);
        if (row_length == name_length) {
            if (row_length == 0) return index;
            const auto* name_data = static_cast<const char*>(pointer(name, 4));
            const auto* row_data = static_cast<const char*>(pointer(row, 4));
            if (_stricmp(row_data, name_data) == 0) return index;
        }
        ++index;
        offset += 0x18;
    }
}

bool contains_native_msar_file_00bba710(const void* provider, const void* name,
    const SingletonLifetimeCallbacks& crt) {
    return find_native_msar_file_index_00bba650(provider, name, crt) != 0xffffffffu;
}
} // namespace bsp
