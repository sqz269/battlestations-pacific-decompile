#include "bsp/game_native_traceline_pool.hpp"
#include "bsp/game_native_physical_pool.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

namespace {
bsp::AllocatorListDomain* domain;
bsp::AllocatorListElement* original_head;
void check(bool value, const char* reason) {
    if (!value) {
        std::fprintf(stderr, "FAIL: %s\n", reason);
        std::_Exit(2);
    }
}
std::uint32_t word(const void* base, std::size_t offset) {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::byte*>(base) + offset, 4);
    return value;
}
void observe_exit() {
    // The common physical process was constructed before this registration.
    // Inspect only its still-live list, after CE0B70 and process destruction.
    check(domain->head() == original_head, "CE0B70 did not restore shared list");
    std::puts("PASS: actual CRT exit unlinked canonical Traceline owner");
}
}

int main() {
    domain = &bsp::game::game_native_physical_pool_process().allocator_list_domain_00e188b4();
    original_head = domain->head();
    check(std::atexit(&observe_exit) == 0, "observer registration");
    auto& process = bsp::game::game_native_traceline_pool_process();
    bool rejected = false;
    try { (void)process.pool_storage(); }
    catch (const std::logic_error&) { rejected = true; }
    check(rejected, "cold borrow was accepted");
    check(process.initialize_once() == 0, "native atexit registration");
    void* const pool = process.pool_storage();
    check(domain->head() == pool && word(pool, 0) == 0x00d5d924u,
        "canonical owner/profile not published in shared domain");
    check(word(pool, 0x24) == 0 && word(pool, 0x28) != 0 &&
        word(pool, 0x2c) == 0 && word(pool, 0x30) == 32 &&
        word(pool, 0x34) == 0xffffffffu, "native constructor table fields");
    const auto table = word(pool, 0x28);
    check(process.initialize_once() == 0 && process.pool_storage() == pool &&
        word(pool, 0x28) == table && word(pool, 4) == 0,
        "repeat startup reinitialized or relinked the owner");
    domain->trim_all_004b46b0();
    check(domain->head() == pool && word(pool, 0x28) == table &&
        word(pool, 0x34) == 0xffffffffu, "actual profile trim binding changed empty pool");
    std::puts("PASS: canonical process startup, table, repeated call and virtual trim");
}
