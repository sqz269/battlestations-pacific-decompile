#include "bsp/game_native_particle_pools.hpp"
#include "bsp/game_native_physical_pool.hpp"
#include "bsp/native_tracer_pool.hpp"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace {
std::uint32_t word(const void* p, unsigned offset) {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::byte*>(p) + offset, 4);
    return value;
}
void check(bool condition, const char* message) {
    if (!condition) { std::fprintf(stderr, "%s\n", message); std::abort(); }
}
}

int main() {
    auto& process = bsp::game::game_native_particle_pool_process();
    check(process.initialize_tracer_once_00cd88d0() == 0, "tracer startup failed");
    void* const pool = process.tracer_pool_0109049c();
    bsp::NativeTracerPoolTableView table{static_cast<std::byte*>(pool) + 0x28};
    check(word(pool, 0x2c) == 0 && word(pool, 0x30) == 32,
        "BAC210 did not reserve the canonical startup table");
    const auto backing = word(pool, 0x28);
    bsp::reserve_native_tracer_pool_table_00bac210(table, 31);
    bsp::reserve_native_tracer_pool_table_00bac210(table, 32);
    check(word(pool, 0x28) == backing && word(pool, 0x30) == 32,
        "reserve changed an equal/larger existing capacity");

    // The existing real raw-slot producer reaches BAC190 and BA9D10 at its
    // initial 32-slabs x 8-slots boundary. These calls are fixture coverage;
    // the game still lacks a production tracer-object allocation consumer.
    std::array<void*, 257> slots{};
    for (auto& slot : slots) slot = bsp::allocate_native_tracer_slot_00bac4a0(pool);
    check(word(pool, 0x2c) == 33 && word(pool, 0x30) == 66 && word(pool, 0x24) == 0,
        "real pool append/grow boundary changed count/capacity/lock depth");
    for (auto* slot : slots) bsp::return_native_tracer_slot_00babf70(pool, slot);
    bsp::game::game_native_physical_pool_process().allocator_list_domain_00e188b4()
        .trim_all_004b46b0();
    check(word(pool, 0x2c) == 0 && word(pool, 0x34) == 0xffffffffu,
        "grown slab pointers were not preserved through return/trim");

    // One adversarial pointer-cell case: source aliases header.data, so append
    // must read its NEW value after growth, rather than the freed old backing.
    struct Header { void* data; std::uint32_t count, capacity; } alias{};
    bsp::NativeTracerPoolTableView alias_table{&alias};
    bsp::reserve_native_tracer_pool_table_00bac210(alias_table, 1);
    std::memcpy(alias.data, &pool, sizeof(pool));
    alias.count = 1;
    bsp::append_native_tracer_pool_table_00bac190(alias_table, &alias.data);
    void* appended;
    std::memcpy(&appended, static_cast<std::byte*>(alias.data) + 4, sizeof(appended));
    check(alias.count == 2 && alias.capacity == 4 && appended == alias.data &&
        word(alias.data, 0) == reinterpret_cast<std::uintptr_t>(pool),
        "append captured an aliased source before growth or lost the old element");
    bsp::free_native_tracer_pool_table_00ba9c80(&alias);
    std::puts("c898 PASS: actual startup reserve, 257-slot growth/trim, late aliased-cell load");
}
