#include "bsp/native_render_resource_record_construction.hpp"
#include "bsp/native_string_pool_storage.hpp"

#include "bsp/native_render_alias_insertion.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource-record construction requires MSVC Win32 pointer widths.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeRenderResourceAliasNode) == 0x10);
static_assert(sizeof(NativeRenderResourceRecord) == 0x2c);

template<class T> volatile T& owner_field(void* owner, std::size_t offset) {
    return *reinterpret_cast<volatile T*>(static_cast<unsigned char*>(owner) + offset);
}
NativeRenderResourceAliasNode* sentinel(const void* owner) {
    return owner_field<NativeRenderResourceAliasNode*>(const_cast<void*>(owner), 4);
}
} // namespace

NativeRenderResourceAliasNode* allocate_native_render_alias_sentinel_004c3020() {
    auto* const raw = singleton_lifetime_allocate({SingletonAllocationKind::object,
        0x10, sizeof(NativeRenderResourceAliasNode)});
    auto* node = static_cast<NativeRenderResourceAliasNode*>(raw);
    if (node != nullptr) {
        node = ::new (raw) NativeRenderResourceAliasNode; // No value initialization.
        volatile auto* const current = node;
        current->next_00 = node;
    }
    // 004C3030 tests the separately wrapped address, not the allocation again.
    const auto previous_address = reinterpret_cast<std::uintptr_t>(node) + 4u;
    if (previous_address != 0)
        *reinterpret_cast<NativeRenderResourceAliasNode* volatile*>(previous_address) = node;
    return node;
}

template<class Pool>
static void destroy_alias_list_with_pool(void* actual_owner,
    Pool& actual_string_pool) {
    clear_native_render_resource_aliases_004d05e0(actual_owner, actual_string_pool);
    singleton_lifetime_free(sentinel(actual_owner)); // Current, after clear callbacks.
    // 004D0A24 follows the returning free despite Ghidra's no-return truncation.
    owner_field<NativeRenderResourceAliasNode*>(actual_owner, 4) = nullptr;
}

void destroy_native_render_alias_list_004d0a10(void* actual_owner,
    SizedStoragePool& actual_string_pool) {
    destroy_alias_list_with_pool(actual_owner, actual_string_pool);
}

void destroy_native_render_alias_list_004d0a10(void* actual_owner,
    ActualNativeStringPoolStorage& actual_string_pool) {
    destroy_alias_list_with_pool(actual_owner, actual_string_pool);
}

template<class Pool>
static void* copy_alias_list_with_pool(void* actual_destination_owner,
    const void* actual_source_owner, Pool& actual_string_pool,
    const SingletonLifetimeCallbacks& callbacks) {
    auto* const allocated_sentinel = allocate_native_render_alias_sentinel_004c3020();
    owner_field<NativeRenderResourceAliasNode*>(actual_destination_owner, 4) = allocated_sentinel;
    owner_field<std::uint32_t>(actual_destination_owner, 8) = 0;
    auto* const source_end = sentinel(actual_source_owner); // After both publications.
    volatile auto* const source_end_fields = source_end;
    auto* const source_first = source_end_fields->next_00;
    volatile auto* const allocated_fields = allocated_sentinel;
    auto* const destination_first = allocated_fields->next_00; // Captured allocation.
    auto* const source_owner = const_cast<void*>(actual_source_owner);
    try { // State 0 at 004D48EB; pointer captures above precede this catch.
        insert_native_render_alias_range_004d26a0(actual_destination_owner,
            {actual_destination_owner, destination_first}, {source_owner, source_first},
            {source_owner, source_end}, actual_string_pool, callbacks);
    } catch (...) {
        // Actual catch004D490C: destroy current list, then native rethrow.
        // Cleanup failure is allowed to supersede the original exception.
        destroy_native_render_alias_list_004d0a10(actual_destination_owner, actual_string_pool);
        throw;
    }
    return actual_destination_owner;
}

void* copy_construct_native_render_alias_list_004d48a0(void* actual_destination_owner,
    const void* actual_source_owner, SizedStoragePool& actual_string_pool,
    const SingletonLifetimeCallbacks& callbacks) {
    return copy_alias_list_with_pool(actual_destination_owner, actual_source_owner, actual_string_pool, callbacks);
}

void* copy_construct_native_render_alias_list_004d48a0(void* actual_destination_owner,
    const void* actual_source_owner, ActualNativeStringPoolStorage& actual_string_pool,
    const SingletonLifetimeCallbacks& callbacks) {
    return copy_alias_list_with_pool(actual_destination_owner, actual_source_owner, actual_string_pool, callbacks);
}

NativeRenderResourceRecord& copy_construct_native_render_resource_record_00b2fc60(
    NativeRenderResourceRecord& actual_destination,
    const NativeRenderResourceRecord& actual_source, SizedStoragePool& actual_string_pool,
    const SingletonLifetimeCallbacks& callbacks) {
    volatile auto& destination = actual_destination;
    const volatile auto& source = actual_source;
    const bool identical = &actual_destination == &actual_source;
    destination.name_length_00 = 0;
    destination.name_data_04 = nullptr;
    PooledStringStorage storage(actual_string_pool);
    if (!identical) {
        const auto requested = source.name_length_00;
        resize_native_string_header_0041dd40(&actual_destination, storage, requested, true);
        if (source.name_length_00 != 0) {
            const auto copied = destination.name_length_00;
            auto* const data = destination.name_data_04;
            const auto* const source_data = source.name_data_04;
            if (copied != 0) std::memcpy(data, source_data, copied);
        }
    }
    // 00B2FCBC arms current-name cleanup only after initial name copy succeeds.
    try {
        copy_construct_native_render_alias_list_004d48a0(
            reinterpret_cast<unsigned char*>(&actual_destination) + 8,
            reinterpret_cast<const unsigned char*>(&actual_source) + 8,
            actual_string_pool, callbacks);
        for (unsigned index = 0; index != 5; ++index)
            destination.payload_14_24[index] = source.payload_14_24[index];
        destination.resource_28 = source.resource_28;
    } catch (...) {
        // DF6200 -> CBD900: cleanup only the current actual name header.
        destroy_native_string_header_0041dd20(&actual_destination, storage);
        throw;
    }
    return actual_destination;
}

} // namespace bsp
