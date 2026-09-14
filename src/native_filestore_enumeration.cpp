#include "bsp/native_filestore_enumeration.hpp"

#include "bsp/native_filestore_subtree.hpp"
#include "bsp/native_vfs_date_leaf_providers.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace bsp {
namespace {
const void* at(const void* base, std::uint32_t offset) noexcept {
    return static_cast<const std::byte*>(base) + offset;
}
void* pointer(const void* base, std::uint32_t offset) noexcept {
    return *reinterpret_cast<void* const volatile*>(at(base, offset));
}
std::uint32_t length(const void* header) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(header);
}
const char* data(const void* header) noexcept {
    return *reinterpret_cast<char* const volatile*>(at(header, 4));
}
} // namespace

void enumerate_native_filestore_names_00be6480(void* actual_provider,
    const void* actual_directory, const void* actual_extension,
    std::uint32_t flags, NativeStringVectorStorage& output,
    NativeStringStorage& strings, const SingletonLifetimeCallbacks& callbacks) {
    (void)flags; // The original body never loads the stack flags argument.
    auto* const tree = static_cast<std::byte*>(actual_provider) + 0x14;
    auto* const saved_head = pointer(tree, 4);
    NativeFileStoreNameIterator iterator{tree, pointer(saved_head, 0)};
    while (iterator.node_04 != saved_head) {
        if (iterator.node_04 == pointer(tree, 4)) {
            callbacks.invalid_parameter(callbacks.context);
        }
        auto* const record = at(iterator.node_04, 0x0c);
        const char* const name = data(record);
        const char* const directory = data(actual_directory);
        if (name && directory) {
            const char* const found_directory = std::strstr(name, directory);
            if (found_directory == name) {
                std::uint32_t extension_index = UINT32_MAX;
                const char* const extension = data(actual_extension);
                if (extension) {
                    const char* const found_extension = std::strstr(name, extension);
                    if (found_extension) {
                        extension_index = static_cast<std::uint32_t>(found_extension - name);
                    }
                }
                if (extension_index == length(record) - length(actual_extension)) {
                    append_native_string_vector_004cdc20(output,
                        *static_cast<const NativeString*>(record), strings);
                }
            }
        }
        advance_native_file_store_resident_iterator_00be4c30(&iterator, callbacks);
    }
}
} // namespace bsp
