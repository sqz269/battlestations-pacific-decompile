#include "bsp/native_vfs_package_scan.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_vfs_date_route.hpp"

#include <cstdint>
#include <cstring>
#include <string.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native VFS package list storage requires MSVC Win32.
#endif

namespace bsp {
namespace {
template<class T> T& field(void* owner, std::uint32_t offset) {
    return *reinterpret_cast<T*>(static_cast<unsigned char*>(owner) + offset);
}
}

void* pop_native_vfs_package_name_00557a90(void* actual_list_owner,
    void* actual_output_string, ActualNativeStringPoolStorage& strings,
    const SingletonLifetimeCallbacks& invalid_parameters) {
    auto* const sentinel = field<NativeRenderResourceAliasNode*>(actual_list_owner, 4);
    auto* const first = sentinel->next_00;
    if (first == sentinel) {
        invalid_parameters.invalid_parameter(invalid_parameters.context);
    }

    // 557AC6..557AD4 initializes the destination even for a zero-length name.
    field<std::uint32_t>(actual_output_string, 0) = 0;
    field<char*>(actual_output_string, 4) = nullptr;
    if (actual_output_string != static_cast<void*>(&first->string_length_08)) {
        resize_native_string_header_0041dd40(actual_output_string, strings,
            first->string_length_08, true);
        if (first->string_length_08 != 0) {
            std::memcpy(field<char*>(actual_output_string, 4),
                first->string_data_0c, field<std::uint32_t>(actual_output_string, 0));
        }
    }

    volatile NativeRenderAliasIterator discarded{};
    erase_native_render_alias_node_004d0990(actual_list_owner, discarded,
        NativeRenderAliasIterator{actual_list_owner, sentinel->next_00},
        strings, invalid_parameters);
    return actual_output_string;
}

void* find_native_vfs_mounted_system_name_00bdb120(void* actual_manager,
    const void* actual_system_name, const SingletonLifetimeCallbacks& invalid_parameters) {
    auto* const tree = static_cast<unsigned char*>(actual_manager) + 0x3c;
    auto* const sentinel = field<void*>(tree, 4);
    void* iterator[2]{tree, field<void*>(sentinel, 0)};
    for (;;) {
        // BDB140 and BDB154 guard the current owner and sentinel before
        // reading the provider. A returning handler retains native flow.
        if (!iterator[0] || iterator[0] != tree) {
            invalid_parameters.invalid_parameter(invalid_parameters.context);
        }
        if (iterator[1] == field<void*>(tree, 4)) {
            return nullptr;
        }
        if (!iterator[0] || iterator[1] == field<void*>(iterator[0], 4)) {
            invalid_parameters.invalid_parameter(invalid_parameters.context);
        }
        auto* const provider = field<void*>(iterator[1], 0x18);
        auto* const stored = static_cast<unsigned char*>(provider) + 8;
        auto* const requested = static_cast<const unsigned char*>(actual_system_name);
        const auto stored_length = field<std::uint32_t>(stored, 0);
        const auto requested_length = *reinterpret_cast<const std::uint32_t*>(requested);
        if (stored_length == requested_length &&
            (stored_length == 0 || _stricmp(field<char*>(stored, 4),
                *reinterpret_cast<char* const*>(requested + 4)) == 0)) {
            if (iterator[1] == field<void*>(iterator[0], 4)) {
                invalid_parameters.invalid_parameter(invalid_parameters.context);
            }
            return field<void*>(iterator[1], 0x18);
        }
        advance_native_vfs_mount_iterator_00bd97e0(iterator, invalid_parameters);
    }
}
} // namespace bsp
