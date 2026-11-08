// Focused Win32 probe: trim a free first page, then return the remaining live
// node through its rewritten page identity. Link bsp_core and /MANIFEST:EMBED.
#include "bsp/native_enum_node_pool.hpp"

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <stdexcept>

namespace {
std::uint32_t word(const void* base, std::size_t offset) {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const char*>(base) + offset, sizeof value);
    return value;
}
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
}

int main() {
    static_assert(sizeof(void*) == 4);
    bsp::AllocatorListElement* head{};
    bsp::AllocatorListDomain list(head);
    alignas(8) unsigned char pool[0x38];
    bsp::bind_native_enum_node_pool_virtual0_00ce37a4(pool, list);
    bsp::initialize_native_enum_node_pool_00411050(pool, list);
    try {
        std::array<void*, 65> nodes;
        for (auto& node : nodes) node = bsp::allocate_native_enum_node_004e7c00(pool);
        const std::uint32_t payload = 0x12345678;
        std::memcpy(nodes.back(), &payload, sizeof payload);

        // Scan past the exhausted page to the partial second page.
        list.trim_all_004b46b0();
        require(word(pool, 0x2c) == 2 && word(pool, 0x34) == 1,
            "trim failed to find the partial second page");
        for (std::size_t i = 0; i < 64; ++i)
            bsp::return_native_enum_node_0043b0a0(pool, nodes[i]);
        list.trim_all_004b46b0();

        require(word(pool, 0x2c) == 1 && word(pool, 0x34) == 0,
            "trim failed to move the retained page to index zero");
        auto** table = reinterpret_cast<void**>(word(pool, 0x28));
        for (std::size_t i = 0; i < 64; ++i)
            require(word(table[0], 0x10 + i * 0x14) == 0,
                "moved page retained a stale slot identity");
        require(word(nodes.back(), 0) == payload, "trim changed the live payload");
        bsp::return_native_enum_node_0043b0a0(pool, nodes.back());
        list.trim_all_004b46b0();
        require(word(pool, 0x2c) == 0 && word(pool, 0x34) == 0xffffffffu,
            "returned moved node did not make its page reclaimable");
    } catch (...) {
        bsp::destroy_native_enum_node_pool_00410a60(pool, list);
        throw;
    }
    bsp::destroy_native_enum_node_pool_00410a60(pool, list);
    require(head == nullptr, "destroy left an allocator-list entry");
    std::puts("enum page move/return probe passed");
}
