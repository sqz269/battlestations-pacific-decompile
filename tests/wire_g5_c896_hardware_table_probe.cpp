#include "bsp/allocator_list.hpp"
#include "bsp/native_hardware_layout_owner.hpp"
#include "bsp/native_hardware_layout_pool_allocate.hpp"
#include "bsp/native_hardware_layout_pool_lifetime.hpp"
#include "bsp/native_hardware_layout_pool_table.hpp"
#include "bsp/native_hardware_layout_pool_trim.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <stdexcept>

namespace {
std::uint32_t word(const void* p, std::size_t offset) {
    return *reinterpret_cast<const std::uint32_t*>(static_cast<const std::byte*>(p) + offset);
}
void require(bool condition) {
    if (!condition) throw std::runtime_error("hardware pool table probe failed");
}
}

int main() {
    // Use the primary-library pool producer, allocation, return and trim. The
    // 34th slab exercises 32->66 growth; removing slab zero moves the last
    // slab and must leave its existing raw slots usable for later returns.
    bsp::AllocatorListElement* head = nullptr;
    bsp::AllocatorListDomain list(head);
    alignas(4) std::array<std::byte, 0x38> pool{};
    bsp::bind_native_hardware_layout_pool_trim_00d62af0(pool.data(), list);
    require(bsp::initialize_native_hardware_layout_pool_00b604d0(pool.data(), list) == pool.data());
    require(word(pool.data(), 0x30) == 32);
    const auto initial_table = word(pool.data(), 0x28);
    bsp::reserve_native_hardware_layout_table_00b60470(pool.data() + 0x28, 16);
    require(word(pool.data(), 0x28) == initial_table && word(pool.data(), 0x30) == 32);
    std::array<void*, 1057> slots{};
    for (auto& slot : slots) {
        slot = bsp::allocate_native_hardware_layout_slot_00b605b0(pool.data());
        require(slot != nullptr);
    }
    require(word(pool.data(), 0x2c) == 34 && word(pool.data(), 0x30) == 66);
    for (std::size_t i = 0; i < 32; ++i)
        bsp::return_native_hardware_layout_slot_00b60110(pool.data(), slots[i]);
    list.trim_all_004b46b0();
    require(word(pool.data(), 0x2c) == 33 && word(slots.back(), 0x44) == 0);
    for (std::size_t i = 32; i < slots.size(); ++i)
        bsp::return_native_hardware_layout_slot_00b60110(pool.data(), slots[i]);
    list.trim_all_004b46b0();
    require(word(pool.data(), 0x2c) == 0 && word(pool.data(), 0x34) == 0xffffffffu);
    bsp::destroy_native_hardware_layout_pool_00b60270(pool.data(), list);
    list.unbind_virtual0(*reinterpret_cast<bsp::AllocatorListElement*>(pool.data()));
    require(head == nullptr);
    std::puts("hardware table: 1057 slots, 32->66 growth, moved-slab return and final trim passed");
}
