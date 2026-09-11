#include "bsp/native_resource_cache_leaves.hpp"

#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_vfs_date_leaf_providers.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <cstdint>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource-cache leaves require MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(LONG) == 4);

void* at_offset(void* storage, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(storage) + offset);
}
const void* at_offset(const void* storage, std::uint32_t offset) noexcept {
    return reinterpret_cast<const void*>(
        reinterpret_cast<std::uintptr_t>(storage) + offset);
}
std::uint32_t read_word(const void* storage, std::uint32_t offset) noexcept {
    return *static_cast<const volatile std::uint32_t*>(at_offset(storage, offset));
}
void write_word(void* storage, std::uint32_t offset, std::uint32_t value) noexcept {
    *static_cast<volatile std::uint32_t*>(at_offset(storage, offset)) = value;
}
void* read_pointer(const void* storage, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(read_word(storage, offset));
}
bool is_nil(const void* node) noexcept {
    return *static_cast<const volatile unsigned char*>(at_offset(node, 0x19)) != 0;
}

void* create_texture_source(std::uint32_t final_profile) {
    void* const resource = singleton_lifetime_allocate({
        SingletonAllocationKind::object, 0x34, 0x34});
    if (resource != nullptr) {
        try {
            construct_native_texture_source_00c30470(resource);
            write_word(resource, 0, final_profile);
        } catch (...) {
            // CC4B70/CC4BB0: free captured allocation, without a destructor.
            // No valid-storage C++ operation above throws; native SEH is not
            // reproduced, and the compiler can eliminate this cleanup.
            singleton_lifetime_free(resource);
            throw;
        }
    }
    return resource;
}
} // namespace

void* construct_native_texture_resource_base_00b19980(void* actual_storage) noexcept {
    write_word(actual_storage, 0, 0x00ceb130);
    write_word(actual_storage, 4, 1);
    write_word(actual_storage, 0, 0x00d5e554);
    return actual_storage;
}

void* construct_native_texture_source_00c30470(void* actual_storage) noexcept {
    construct_native_texture_resource_base_00b19980(actual_storage);
    write_word(actual_storage, 0, 0x00d79b54);
    write_word(actual_storage, 0x10, 0);
    write_word(actual_storage, 0x14, 0);
    write_word(actual_storage, 0x18, 0);
    write_word(actual_storage, 0x08, 0);
    *static_cast<volatile unsigned char*>(at_offset(actual_storage, 0x1c)) = 0;
    return actual_storage;
}

void* create_native_caustics_texture_source_00bbc6f0() {
    return create_texture_source(0x00d64478);
}

void* create_native_shore_wave_texture_source_00bbc810() {
    return create_texture_source(0x00d644b4);
}

void* copy_native_cache_requested_name_00b19e40(void* actual_output,
    const void* actual_requested, const void*,
    ActualNativeStringPoolStorage& actual_strings) {
    const bool same = actual_output == actual_requested;
    write_word(actual_output, 0, 0);
    write_word(actual_output, 4, 0);
    if (!same) {
        const auto length = read_word(actual_requested, 0);
        resize_native_string_header_0041dd40(actual_output, actual_strings, length, true);
        if (read_word(actual_requested, 0) != 0) {
            const auto current_length = read_word(actual_output, 0);
            const auto* source = read_pointer(actual_requested, 4);
            auto* destination = read_pointer(actual_output, 4);
            // Keep the established raw-string source boundary: native permits
            // zero-byte memcpy with pointers outside the standard C++ domain.
            if (current_length != 0) std::memcpy(destination, source, current_length);
        }
    }
    return actual_output;
}

void* retain_native_cache_resource_004ddb20(void* actual_resource) noexcept {
    InterlockedIncrement(static_cast<volatile LONG*>(at_offset(actual_resource, 4)));
    return actual_resource;
}

void* lower_bound_native_resource_factory_00b19b90(void* actual_tree,
    const void* actual_key) {
    auto* candidate = read_pointer(actual_tree, 4);
    auto* node = read_pointer(candidate, 4);
    while (!is_nil(node)) {
        if (less_native_string_headers_00443d00(at_offset(node, 0x0c), actual_key)) {
            node = read_pointer(node, 8);
        } else {
            candidate = node;
            node = read_pointer(node, 0);
        }
    }
    return candidate;
}

void* find_native_resource_factory_00b19d60(void* actual_tree,
    void* actual_iterator_output, const void* actual_key,
    const SingletonLifetimeCallbacks& invalid_parameters) {
    auto* node = lower_bound_native_resource_factory_00b19b90(actual_tree, actual_key);
    if (!actual_tree) {
        invalid_parameters.invalid_parameter(invalid_parameters.context);
    }
    if (node == read_pointer(actual_tree, 4) ||
        less_native_string_headers_00443d00(actual_key, at_offset(node, 0x0c))) {
        node = read_pointer(actual_tree, 4);
    }
    const auto owner_word = reinterpret_cast<std::uint32_t>(actual_tree);
    const auto node_word = reinterpret_cast<std::uint32_t>(node);
    write_word(actual_iterator_output, 0, owner_word);
    write_word(actual_iterator_output, 4, node_word);
    return actual_iterator_output;
}
} // namespace bsp
