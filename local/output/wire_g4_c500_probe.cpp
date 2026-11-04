#include "bsp/unit_weapons.hpp"
#include <array>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <limits>

// A focused body fixture. It is not gameplay or original-ABI evidence.
struct StockProbe final : bsp::TorpedoStockHost {
    std::array<float, 3> timers{5.0f, FLT_MAX, std::numeric_limits<float>::quiet_NaN()};
    int spare{}, consumed{}, unloaded{}, rearmed{};
    float other_timer{FLT_MAX};
    bool class_test(bsp::NativeHandle, int kind) override { return kind == bsp::kClassTestGun; }
    bsp::NativeHandle node_first_child(bsp::NativeHandle) override { return 1; }
    bsp::NativeHandle node_next_sibling(bsp::NativeHandle gun) override { return gun == 1 ? 2 : 0; }
    int gun_barrel_num(bsp::NativeHandle gun) override { return gun == 1 ? 3 : 1; }
    float gun_reload_timer(bsp::NativeHandle gun, int barrel) override {
        return gun == 1 ? timers[barrel] : other_timer;
    }
    int gun_weapon_type(bsp::NativeHandle gun) override { return gun == 1 ? 7 : 1; }
    float gun_reload_empty_threshold() override { return FLT_MAX; }
    int loaded_torpedo_barrels_00810e90(bsp::NativeHandle) override {
        int n = 0;
        for (float timer : timers) if (timer < FLT_MAX) ++n;
        return n;
    }
    void unload_random_torpedo_barrel_0081dcb0(bsp::NativeHandle) override {
        // The fixture fixes the pick; production uses the existing stream-1 draw.
        for (float& timer : timers) if (timer < FLT_MAX) { timer = FLT_MAX; ++unloaded; return; }
    }
    void unit_set_torpedo_spare(bsp::NativeHandle, int value) override { spare = value; }
    void gun_rearm_barrel(bsp::NativeHandle gun, int barrel, bool full) override {
        if (gun != 1) { other_timer = 99.0f; return; }
        if (spare == 0) { timers[barrel] = FLT_MAX; return; }
        if (spare > 0) { --spare; ++consumed; }
        if (full) { timers[barrel] = 5.0f; ++rearmed; }
    }
};

int main() {
    StockProbe p;
    bsp::set_torpedo_stock_0081f8b0(p, 1, 3);
    if (p.spare != 0 || p.consumed != 2 || p.rearmed != 2 || p.other_timer != FLT_MAX) return 1;
    for (float timer : p.timers) if (timer != 5.0f) return 2;
    bsp::set_torpedo_stock_0081f8b0(p, 1, -2);
    if (p.spare != 0 || p.unloaded != 3 || p.consumed != 2) return 3;
    for (float timer : p.timers) if (timer != FLT_MAX) return 4;
    p.timers[0] = std::numeric_limits<float>::quiet_NaN();
    bsp::set_torpedo_stock_0081f8b0(p, 1, 0);
    if (p.timers[0] != FLT_MAX || p.consumed != 2 || p.other_timer != FLT_MAX) return 5;
    std::puts("PASS: stock refill/unload, negative clamp, unordered timer, non-torpedo exclusion");
}
