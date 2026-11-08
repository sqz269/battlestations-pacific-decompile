#include "bsp/gameplay_seg61_62_cc13.hpp"
#include "bsp/cc13_sweepA.hpp"
#include "bsp/save_settings_cc13.hpp"
#include <array>
#include <cstdlib>
#include <cstdio>
#include <stdexcept>
#include <vector>

// One focused regression: cache release callbacks can change the publication.
// The second resize must use the reloaded owner, and each reference drops once.
struct Probe final : bsp::gameplay_seg61_62_cc13::ClassRegistryClearCalls,
    bsp::cc13_sweepA::PartyRowsStorageHost {
    alignas(4) std::array<unsigned char, 0x401c> first{}, second{};
    int refs[2]{2, 1};
    std::vector<void*> cache{&refs[0], &refs[1]};
    unsigned char* publication = first.data();
    int step = 0;
    void require(bool pass) { if (!pass) throw std::runtime_error("clear order/state mismatch"); }
    void* get_registry_00437f50() override {
        require(step == 0 || step == 2);
        ++step;
        return publication;
    }
    void resize_cache_00437bd0(void* actual, std::int32_t count) override {
        require(step++ == 1 && actual == first.data() + 4 && count == 0);
        bsp::cc13_save::resize_descriptor_cache_00437bd0(cache, count,
            [&](void* value) { --*static_cast<int*>(value); publication = second.data(); });
    }
    void resize_party_rows_004359e0(void* actual, std::int32_t count) override {
        require(step++ == 3 && actual == second.data() + 0x4010 && count == 0);
        bsp::cc13_sweepA::party_rows_resize_004359e0(
            {static_cast<std::uint8_t*>(actual)}, count, *this);
    }
    void* operator_new_00bf55be(std::uint32_t n) override { return std::malloc(n); }
    void free_00bf6989(void* p) override { std::free(p); }
};
int main() {
    Probe p;
    const bsp::cc13_sweepA::Raw a{p.first.data() + 0x4010};
    const bsp::cc13_sweepA::Raw b{p.second.data() + 0x4010};
    a.at<std::int32_t>(4) = 5;
    b.at<std::int32_t>(4) = 7;
    bsp::gameplay_seg61_62_cc13::class_registry_clear_0095ba30(p);
    p.require(p.step == 4 && p.cache.empty() && p.refs[0] == 1 && p.refs[1] == 0
        && a.at<std::int32_t>(4) == 5 && b.at<std::int32_t>(4) == 0);
    std::puts("PASS: actual cache release once; second getter observes new party-row owner");
}
