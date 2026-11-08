// Standalone MSVC Win32 check of the actual cube pool across table growth.
// Link the built bsp_core library and its real shared CRT allocation services.
#include "bsp/native_cube_texture_pool_allocate.hpp"
#include "bsp/native_cube_texture_pool_lifetime.hpp"
#include "bsp/native_cube_texture_pool_trim.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace {
void require(bool condition) { if (!condition) std::abort(); }
std::uint32_t word(const void* base, std::size_t offset) {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const unsigned char*>(base) + offset, 4);
    return value;
}
}

int main() {
    bsp::AllocatorListElement* head = nullptr;
    bsp::AllocatorListDomain list(head);
    alignas(4) unsigned char pool[0x38];
    std::memset(pool, 0xa5, sizeof(pool));
    bsp::bind_native_cube_texture_pool_trim_00d61944(pool, list);
    require(bsp::initialize_native_cube_texture_pool_00b3f090(pool, list) == pool);
    require(word(pool, 0x28) && word(pool, 0x30) == 32);
    require(word(pool, 0x2c) == 0 && word(pool, 0x34) == 0xffffffffu);
    void* slots[1025];
    for (std::uint32_t index = 0; index < 1025; ++index) {
        slots[index] = bsp::allocate_native_cube_texture_slot_00b3f170(pool);
        std::memcpy(slots[index], &index, 4);
    }
    require(word(pool, 0x2c) == 33 && word(pool, 0x30) == 66);
    require(word(pool, 0x24) == 0 && head == reinterpret_cast<void*>(pool));
    for (std::uint32_t index = 0; index < 1025; ++index) {
        require(word(slots[index], 0) == index);
        require(word(slots[index], 0x30) == index / 32);
    }
    bsp::return_native_cube_texture_slot_00b3d940(pool, slots[1024]);
    list.trim_all_004b46b0();
    require(word(pool, 0x2c) == 32 && word(pool, 0x30) == 66);
    for (std::uint32_t index = 0; index < 1024; ++index)
        bsp::return_native_cube_texture_slot_00b3d940(pool, slots[index]);
    list.trim_all_004b46b0();
    require(word(pool, 0x2c) == 0 && word(pool, 0x30) == 66);
    require(word(pool, 0x28) && word(pool, 0x34) == 0xffffffffu);
    bsp::destroy_native_cube_texture_pool_00b3e5b0(pool, list);
    require(!head);
    list.unbind_virtual0(*reinterpret_cast<bsp::AllocatorListElement*>(pool));
    std::puts("Cube table probe PASS: 1025 live slots, 33 slabs, capacity32->66, "
        "payloads/indices retained, trim retains table, teardown unlinks owner");
}
