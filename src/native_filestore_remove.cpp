#include "bsp/native_filestore_remove.hpp"

#include "bsp/native_filestore_open.hpp"
#include "bsp/native_filestore_resident_tree.hpp"
#include "bsp/native_pooled_resource_path.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_vfs_date_leaf_providers.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native FileStore removal requires MSVC Win32.
#endif

namespace bsp {
namespace {
void* at(void* p, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
void* pointer(const void* p, std::uint32_t offset) noexcept {
    const auto address = reinterpret_cast<std::uintptr_t>(p) + offset;
    return reinterpret_cast<void*>(*reinterpret_cast<const volatile std::uint32_t*>(address));
}
struct NameCleanup {
    void* header;
    NativeStringStorage& strings;
    ~NameCleanup() noexcept { destroy_native_string_header_0041dd20(header, strings); }
};
} // namespace

void remove_native_file_store_file_00be7130(void* store, const void* name,
    NativeStringStorage& strings, NativeAdoptedSubstreamDispatch& streams,
    const SingletonLifetimeCallbacks& callbacks) {
    std::uint32_t normalized[2];
    copy_construct_native_resource_path_header_00bee780(normalized, name, strings);
    const NameCleanup cleanup{normalized, strings}; // Native state0 after copy returns.
    auto* const tree = at(store, 0x14);
    NativeFileStoreNameIterator found;
    find_native_file_store_open_name_00be5e90(tree, &found, normalized, callbacks);
    auto* const owner = pointer(&found, 0); // BE7176, before returning CRT.
    auto* const end = pointer(tree, 4); // BE717C captures current head before CRT.
    if (!owner || owner != tree) callbacks.invalid_parameter(callbacks.context);
    auto* const node = pointer(&found, 4); // BE718A reloads only node afterward.
    if (node != end) {
        erase_native_file_store_resident_iterator_00be6760(tree, &found,
            owner, node, strings, streams, callbacks);
    }
    // Both message branches load current normalized data, then call 4254B0's
    // single RET. Its literal/null fallback is never dereferenced or logged.
    (void)pointer(normalized, 4);
}
} // namespace bsp
