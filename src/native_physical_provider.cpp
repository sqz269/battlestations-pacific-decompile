#include "bsp/native_physical_provider.hpp"

#include "bsp/native_file_provider_base.hpp"
#include "bsp/native_physical_index_tree.hpp"
#include "bsp/native_physical_pending_records.hpp"
#include "bsp/native_physical_provider_pool.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_compare.hpp"
#include "bsp/native_vfs_container_allocation.hpp"
#include "bsp/singleton_lifetime.hpp"

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
void put_byte(void* base, std::uint32_t offset, std::uint8_t value) noexcept {
    *static_cast<volatile std::uint8_t*>(at(base, offset)) = value;
}
void* pointer(std::uint32_t value) noexcept {
    return reinterpret_cast<void*>(value);
}
std::uint32_t address(const void* value) noexcept {
    return reinterpret_cast<std::uint32_t>(value);
}

struct ProviderUnwind {
    void* owner;
    NativeStringStorage& strings;
    int state = -1;
    ~ProviderUnwind() noexcept(false) {
        // Both reviewed three-state FH3 maps chain2 ->1 ->0 ->-1.
        // The index itself is not a cleanup action in either map.
        if (state >= 2)
            destroy_native_string_header_0041dd20(at(owner, 0x20), strings);
        if (state >= 1)
            destroy_native_physical_pending_records_00bf4b80(at(owner, 0x14), strings);
        if (state >= 0)
            destroy_native_file_provider_base_00bb5380(owner, strings);
    }
};

struct ProviderSlotUnwind {
    void* slot;
    NativePhysicalProviderPoolContext& pool;
    bool armed = true;
    ~ProviderSlotUnwind() noexcept(false) {
        // Both independent factory states target the same BF3200 wrapper.
        if (armed) return_native_physical_provider_slot_00bf3200(slot, pool);
    }
};
} // namespace

void* construct_native_physical_provider_00bf4d30(void* owner,
    const void* system_name, std::uint32_t flags, NativeStringStorage& strings) {
    ProviderUnwind unwind{owner, strings};
    construct_native_file_provider_base_00bb5590(owner, system_name, strings);
    put(owner, 0, 0x00d69168u);
    unwind.state = 0;
    put(owner, 0x14, 0);
    put(owner, 0x18, 0);
    put(owner, 0x1c, 0);
    put(owner, 0x20, 0);
    put(owner, 0x24, 0);
    put_byte(owner, 0x28, static_cast<std::uint8_t>(flags));
    auto* const index = at(owner, 0x2c);
    unwind.state = 2;
    void* const allocated = allocate_native_tree_node_00bdaba0();
    put(index, 4, address(allocated));
    put_byte(allocated, 0x1d, 1);
    auto* current = pointer(word(index, 4));
    put(current, 4, address(current));
    current = pointer(word(index, 4));
    put(current, 0, address(current));
    current = pointer(word(index, 4));
    put(current, 8, address(current));
    put(index, 8, 0);
    // BF4DAA calls4254B0("cPhysicalDirectoryX86::exit"). The complete verified
    // callee is one RET; there is no diagnostic, callback or cleanup effect.
    unwind.state = -1;
    return owner;
}

void* create_native_physical_provider_00bf4df0(void*, const void* system_name,
    const void* virtual_name, NativePhysicalProviderContext& context) {
    const auto length = word(system_name);
    if (length == 0) return nullptr;
    const auto last = word(system_name, 4) + length - 1u;
    if (*reinterpret_cast<const volatile std::uint8_t*>(last) != 0x5cu)
        return nullptr;
    const bool persistent = equal_native_string_header_00425850(virtual_name,
        context.persistent_name_00cff208);
    void* const slot = acquire_native_physical_provider_slot_00bf34d0(
        context.pool_0109dbf0);
    ProviderSlotUnwind unwind{slot, context.pool_0109dbf0};
    if (slot == nullptr) {
        unwind.armed = false;
        return nullptr;
    }
    void* const result = construct_native_physical_provider_00bf4d30(slot,
        system_name, persistent ? 1u : 0u, context.strings);
    unwind.armed = false;
    return result;
}

void destroy_native_physical_provider_00bf4c70(void* owner,
    NativeStringStorage& strings, const SingletonLifetimeCallbacks& invalid) {
    ProviderUnwind unwind{owner, strings};
    put(owner, 0, 0x00d69168u);
    auto* const last = pointer(word(owner, 0x30));
    auto* const first = pointer(word(last));
    auto* const index = at(owner, 0x2c);
    std::uint32_t output[2];
    unwind.state = 2;
    erase_native_physical_index_range_00be0c30(index, output, index, first,
        index, last, strings, invalid);
    singleton_lifetime_free(pointer(word(index, 4)));
    put(index, 4, 0);
    put(index, 8, 0);
    const auto cache_data = word(owner, 0x24);
    unwind.state = 1;
    if (cache_data != 0) {
        const auto bytes = word(owner, 0x20) + 1u;
        strings.release(reinterpret_cast<char*>(cache_data), bytes);
    }
    auto* const pending = at(owner, 0x14);
    unwind.state = 0;
    resize_native_physical_pending_records_00bf3ed0(pending, 0, strings);
    singleton_lifetime_free(pointer(word(pending)));
    unwind.state = -1;
    destroy_native_file_provider_base_00bb5380(owner, strings);
}

void* delete_native_physical_provider_00bf4dd0(void* owner,
    std::uint32_t flags, NativePhysicalProviderContext& context) {
    destroy_native_physical_provider_00bf4c70(owner, context.strings,
        context.invalid_parameters);
    if ((flags & 1u) != 0)
        return_native_physical_provider_slot_00bf2fc0(context.pool_0109dbf0, owner);
    return owner;
}
} // namespace bsp
