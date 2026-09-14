#include "bsp/native_vfs_package_scan.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_render_resource_record_construction.hpp"
#include "bsp/native_vfs_enumeration.hpp"
#include "bsp/native_vfs_mount_registration.hpp"
#include "bsp/native_vfs_date_route.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cstddef>
#include <string.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native VFS package list storage requires MSVC Win32.
#endif

namespace bsp {
namespace {
template<class T> T& field(void* owner, std::uint32_t offset) {
    return *reinterpret_cast<T*>(static_cast<unsigned char*>(owner) + offset);
}
struct PooledName {
    explicit PooledName(ActualNativeStringPoolStorage& strings) : strings(strings) {}
    ~PooledName() { value.release_to(strings); }
    NativeString value{};
    ActualNativeStringPoolStorage& strings;
};
struct NameList {
    explicit NameList(ActualNativeStringPoolStorage& strings) : strings(strings) {
        field<NativeRenderResourceAliasNode*>(words, 4) =
            allocate_native_render_alias_sentinel_004c3020();
        field<std::uint32_t>(words, 8) = 0;
    }
    ~NameList() { clear(); }
    void clear() noexcept {
        auto*& head = field<NativeRenderResourceAliasNode*>(words, 4);
        if (!head) return;
        clear_native_render_resource_aliases_004d05e0(words, strings);
        singleton_lifetime_free(head);
        head = nullptr;
    }
    void* actual() noexcept { return words; }
    std::uint32_t count() const noexcept {
        return *reinterpret_cast<const std::uint32_t*>(words + 8);
    }
    alignas(4) unsigned char words[12]; // Native +0 is an unwritten preimage.
    ActualNativeStringPoolStorage& strings;
};
}

void scan_native_vfs_packages_0073cb10(NativeVfsPackageScanContext& context) {
    NameList names(context.strings);
    PooledName extension(context.strings), directory(context.strings);
    construct_native_string_cstring_0041e870(&extension.value, "mpkg", context.strings);
    construct_native_string_cstring_0041e870(&directory.value, ".", context.strings);
    enumerate_native_vfs_resources_00bdd990(context.manager_0109ceec,
        &directory.value, &extension.value, 0, names.actual(), context.enumeration);
    directory.value.release_to(context.strings);
    extension.value.release_to(context.strings);

    while (names.count() != 0) {
        PooledName patch(context.strings), name(context.strings);
        construct_native_string_cstring_0041e870(&patch.value, "patch", context.strings);
        pop_native_vfs_package_name_00557a90(names.actual(), &name.value,
            context.strings, context.invalid_parameters);

        std::uint32_t priority = 1000;
        if (name.value.data() && patch.value.data() && name.value.length() >= patch.value.length()) {
            const auto last = name.value.length() - patch.value.length();
            for (std::uint32_t offset = 0; offset <= last; ++offset) {
                if (_strnicmp(name.value.data() + offset, patch.value.data(),
                        patch.value.length()) != 0) continue;
                if (offset == 0) {
                    PooledName suffix(context.strings);
                    construct_native_string_substring_00469840(&name.value,
                        &suffix.value, patch.value.length(), 0x7fffffffU, context.strings);
                    const auto decimal = std::atol(suffix.value.data() ? suffix.value.data() : "");
                    priority += static_cast<std::uint32_t>(decimal);
                }
                break;
            }
        }

        if (!find_native_vfs_mounted_system_name_00bdb120(
                context.manager_0109ceec, &name.value, context.invalid_parameters)) {
            PooledName virtual_directory(context.strings);
            construct_native_string_cstring_0041e870(&virtual_directory.value,
                ".", context.strings);
            mount_native_vfs_system_path_00be1890(context.manager_0109ceec,
                &name.value, &virtual_directory.value, priority, 0, 0xffffffffU,
                context.mounting);
        }
        name.value.release_to(context.strings);
        patch.value.release_to(context.strings);
    }
    names.clear();
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
