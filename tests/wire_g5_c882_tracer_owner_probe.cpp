#include "bsp/game_native_particle_pools.hpp"
#include "bsp/game_native_physical_pool.hpp"
#include "bsp/native_tracer_pool.hpp"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>

namespace {
bsp::AllocatorListDomain* actual_list;
void* actual_pool;

void require(bool condition, const char* message) {
    if (!condition) {
        std::fprintf(stderr, "c882 FAIL: %s\n", message);
        std::abort();
    }
}

std::uint32_t word(std::uint32_t offset) {
    return *reinterpret_cast<const std::uint32_t*>(
        static_cast<const std::byte*>(actual_pool) + offset);
}

void after_native_exit() {
    // Registered before CD88D0, after construction of both process objects.
    // Native CE0F90 must have freed the table/section and unlinked this owner.
    require(actual_list->head() != actual_pool, "native exit did not unlink owner");
    require(word(0) == bsp::AllocatorListDomain::base_vtable,
        "native exit did not reach base element destruction");
    std::puts("c882 PASS: native exit precedes process/list destruction");
}
}

int main() {
    auto& process = bsp::game::game_native_particle_pool_process();
    actual_list = &bsp::game::game_native_physical_pool_process()
        .allocator_list_domain_00e188b4();
    bool rejected = false;
    try { process.tracer_pool_0109049c(); }
    catch (const std::logic_error&) { rejected = true; }
    require(rejected, "fresh owner accepted a client before explicit startup");
    require(std::atexit(&after_native_exit) == 0, "probe exit observer registration");

    const int status = process.initialize_tracer_once_00cd88d0();
    require(status == 0, "native CRT callback registration");
    actual_pool = process.tracer_pool_0109049c();
    require(process.initialize_tracer_once_00cd88d0() == status &&
        process.tracer_pool_0109049c() == actual_pool, "startup changed identity/status");
    require(actual_list->head() == actual_pool && word(0) == 0x00d63f90 &&
        word(0x24) == 0 && word(0x28) && word(0x2c) == 0 &&
        word(0x30) == 32 && word(0x34) == 0xffffffffu, "fresh native pool fields/list");

    // Raw-slot clients exercise the SAME production storage, not tracer object
    // construction. 257 allocations cross the binary's initial 32x8 boundary.
    std::array<void*, 257> slots{};
    for (auto& slot : slots) slot = bsp::allocate_native_tracer_slot_00bac4a0(actual_pool);
    require(word(0x2c) == 33 && word(0x30) == 66 && word(0x24) == 0,
        "native slab growth/depth");
    auto* const header = static_cast<std::byte*>(actual_pool) + 0x28;
    // Check BA9CC0 with a nonempty table as well as its production empty use.
    bsp::reserve_native_tracer_pool_table_00ba9cc0(header, word(0x30) * 2u + 2u);
    require(word(0x2c) == 33 && word(0x30) == 134, "reserve changed count/capacity");
    for (auto* slot : slots) bsp::return_native_tracer_slot_00babf70(actual_pool, slot);
    actual_list->trim_all_004b46b0();
    require(word(0x2c) == 0 && word(0x30) == 134 &&
        word(0x34) == 0xffffffffu && word(0x24) == 0,
        "shared-list trim failed after nonempty reserve copied slab pointers");
    std::puts("c882 PASS: canonical owner/startup/reserve/growth/return/shared trim");
}
