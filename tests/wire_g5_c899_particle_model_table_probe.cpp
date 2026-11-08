// One focused integration check: cross the actual particle-model slab table's
// initial capacity, retain slab identity, then return/trim/destroy the owner.
#include "bsp/native_particle_model_pool_allocate.hpp"
#include "bsp/native_particle_model_pool_owner.hpp"
#include "bsp/native_particle_model_pool_table.hpp"

#include <array>
#include <cstdio>
#include <cstring>
#include <stdexcept>

namespace {
void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
template<class T> T read(const void* base, std::size_t offset) {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(base) + offset, sizeof value);
    return value;
}
}

int main() {
    bsp::AllocatorListElement* head = nullptr;
    bsp::AllocatorListDomain list(head);
    alignas(std::max_align_t) std::byte pool[0x38];
    std::memset(pool, 0xa5, sizeof pool);
    bsp::initialize_native_particle_model_pool_00af6860(pool, list);
    bsp::NativeParticleModelPoolTableView table{pool + 0x28};
    require(head == reinterpret_cast<bsp::AllocatorListElement*>(pool)
        && table.count() == 0 && table.capacity() == 32,
        "particle-model constructor did not reserve the live table");

    std::array<void*, 1025> slots;
    std::array<std::byte*, 32> original_slabs;
    for (std::size_t i = 0; i < slots.size(); ++i) {
        if (i == 1024)
            std::memcpy(original_slabs.data(), table.data(), sizeof original_slabs);
        slots[i] = bsp::allocate_native_particle_model_slot_00af69e0(pool);
        require(read<std::uint32_t>(slots[i], 0x2dc) == i / 32,
            "particle-model hidden slab index changed");
    }
    require(table.count() == 33 && table.capacity() == 66
        && read<std::uint32_t>(pool, 0x34) == 32
        && read<std::uint32_t>(pool, 0x24) == 0,
        "particle-model growth changed count/capacity/free-slab/lock depth");
    for (std::size_t i = 0; i < original_slabs.size(); ++i)
        require(read<std::byte*>(table.data(), i * 4) == original_slabs[i],
            "particle-model growth lost a live slab");
    auto* last_slab = read<std::byte*>(table.data(), 32 * 4);
    require(read<std::uint16_t>(last_slab, 0x5c40) == 31,
        "particle-model append did not retain the initialized slab");

    for (void* slot : slots) bsp::return_native_particle_model_slot_00af60b0(pool, slot);
    bsp::trim_native_particle_model_pool_00af6940(pool);
    require(table.count() == 0 && read<std::uint32_t>(pool, 0x34) == 0xffffffffu,
        "particle-model retirement left empty slabs");
    bsp::destroy_native_particle_model_pool_00af5ff0(pool, list);
    require(head == nullptr, "particle-model destruction left allocator-list links");
    std::puts("c899 particle-model table: 1025 slots, 33 slabs, capacity32->66; identities/IDs/lock/retirement passed");
}
