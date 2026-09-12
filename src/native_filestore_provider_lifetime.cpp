#include "bsp/native_filestore_provider_lifetime.hpp"

#include "bsp/native_file_provider_base.hpp"
#include "bsp/native_filestore_container_allocation.hpp"
#include "bsp/native_filestore_pending_tree.hpp"
#include "bsp/native_filestore_resident_tree.hpp"
#include "bsp/native_string_compare.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native FileStore provider lifetime requires MSVC Win32.
#endif

namespace bsp {
namespace {
void* at(void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
const void* at(const void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
std::uint32_t word(const void* base, std::uint32_t offset = 0) noexcept {
    return *static_cast<const volatile std::uint32_t*>(at(base, offset));
}
void put(void* base, std::uint32_t offset, std::uint32_t value) noexcept {
    *static_cast<volatile std::uint32_t*>(at(base, offset)) = value;
}
void* pointer(const void* base, std::uint32_t offset = 0) noexcept {
    return reinterpret_cast<void*>(word(base, offset));
}
void form_sentinel(void* tree, void* node) noexcept {
    put(tree, 4, reinterpret_cast<std::uint32_t>(node));
    *static_cast<volatile std::uint8_t*>(at(node, 0x19)) = 1;
    // The constructor reloads the tree head before each independent self-link.
    auto* current = pointer(tree, 4);
    put(current, 4, reinterpret_cast<std::uint32_t>(current));
    current = pointer(tree, 4);
    put(current, 0, reinterpret_cast<std::uint32_t>(current));
    current = pointer(tree, 4);
    put(current, 8, reinterpret_cast<std::uint32_t>(current));
    put(tree, 8, 0);
}
struct ConstructCleanup {
    void* owner;
    void* temporary;
    NativeFileStoreProviderLifetimeContext& context;
    int state = -1;
    ~ConstructCleanup() noexcept {
        // E01798: state0 -> temp; state2 -> base; state3 -> resident then base.
        // State1 -> base then temp is metadata-only: normal body never arms it.
        if (state == 3) destroy_native_file_store_resident_tree_00be7bb0(
            at(owner, 0x14), context.strings, context.streams, context.invalid_parameters);
        if (state >= 2) destroy_native_file_provider_base_00bb5380(owner, context.strings);
        if (state == 0) destroy_native_string_header_0041dd20(temporary, context.strings);
    }
};
struct DestroyCleanup {
    void* owner;
    NativeFileStoreProviderLifetimeContext& context;
    int state = 2;
    ~DestroyCleanup() noexcept {
        // E01720 actions CC6E03,CC6DF8,CC6DF0. A member whose normal cleanup
        // has started is excluded before entering that member's erase call.
        if (state >= 2) destroy_native_file_store_pending_tree_unwind_00be7a70(
            at(owner, 0x20), context.strings, context.invalid_parameters);
        if (state >= 1) destroy_native_file_store_resident_tree_00be7bb0(
            at(owner, 0x14), context.strings, context.streams, context.invalid_parameters);
        if (state >= 0) destroy_native_file_provider_base_00bb5380(owner, context.strings);
    }
};
struct AllocationCleanup {
    void* allocation;
    bool armed = true;
    ~AllocationCleanup() noexcept { if (armed) singleton_lifetime_free(allocation); }
};
} // namespace

void* construct_native_file_store_provider_00be7fa0(
    void* owner, NativeFileStoreProviderLifetimeContext& context) {
    alignas(4) std::uint32_t temporary[2]{};
    resize_native_string_header_0041dd40(temporary, context.strings, 0, true);
    auto* const temporary_data = pointer(temporary, 4);
    if (temporary_data) {
        // CE3A0C is the verified empty literal. With the actual zero-length
        // resize this conditional has no normal reachable allocation path.
        std::memmove(temporary_data, "", word(temporary) + 1u);
    }
    ConstructCleanup cleanup{owner, temporary, context};
    cleanup.state = 0;
    construct_native_file_provider_base_00bb5590(owner, temporary, context.strings);
    cleanup.state = 2;
    if (temporary_data) context.strings.release(static_cast<char*>(temporary_data), word(temporary) + 1u);
    put(owner, 0, 0x00d689e8);
    auto* tree = at(owner, 0x14);
    form_sentinel(tree, allocate_native_file_store_node_00be55e0());
    tree = at(owner, 0x20);
    cleanup.state = 3;
    form_sentinel(tree, allocate_native_file_store_node_00be5690());
    cleanup.state = -1;
    return owner;
}

void* get_native_file_store_provider_00be80b0(
    void* factory, NativeFileStoreProviderLifetimeContext& context) {
    void* const cached = pointer(factory, 8);
    if (cached) return cached;
    void* const allocation = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x2c, 0x2c});
    AllocationCleanup cleanup{allocation};
    void* const result = allocation
        ? construct_native_file_store_provider_00be7fa0(allocation, context) : nullptr;
    put(factory, 8, reinterpret_cast<std::uint32_t>(result));
    cleanup.armed = false;
    return result;
}

void* create_native_file_store_provider_00be8120(void* factory,
    const void* system_name, const void*, NativeFileStoreProviderLifetimeContext& context) {
    if (word(system_name) == 0) return nullptr;
    if (!equal_native_string_header_00425850(system_name, "filestore")) return nullptr;
    return get_native_file_store_provider_00be80b0(factory, context);
}

void destroy_native_file_store_provider_00be7bf0(
    void* owner, NativeFileStoreProviderLifetimeContext& context) {
    put(owner, 0, 0x00d689e8);
    DestroyCleanup cleanup{owner, context};
    clear_native_file_store_00be72c0(owner, context.strings, context.streams, context.invalid_parameters);

    auto* tree = at(owner, 0x20);
    auto* head = pointer(tree, 4);
    auto* first = pointer(head);
    alignas(4) std::uint32_t output[2];
    cleanup.state = 1;
    erase_native_file_store_pending_range_00be7580(tree, output, tree, first, tree, head,
        context.strings, context.invalid_parameters);
    singleton_lifetime_free(pointer(tree, 4));
    put(tree, 4, 0);
    put(tree, 8, 0);

    tree = at(owner, 0x14);
    head = pointer(tree, 4);
    first = pointer(head);
    cleanup.state = 0;
    erase_native_file_store_resident_range_00be7690(tree, output, tree, first, tree, head,
        context.strings, context.streams, context.invalid_parameters);
    singleton_lifetime_free(pointer(tree, 4));
    put(tree, 4, 0);
    put(tree, 8, 0);
    cleanup.state = -1;
    destroy_native_file_provider_base_00bb5380(owner, context.strings);
}

void* delete_native_file_store_provider_00be8090(void* owner,
    std::uint32_t flags, NativeFileStoreProviderLifetimeContext& context) {
    destroy_native_file_store_provider_00be7bf0(owner, context);
    if ((flags & 1u) != 0) singleton_lifetime_free(owner);
    return owner;
}
} // namespace bsp
