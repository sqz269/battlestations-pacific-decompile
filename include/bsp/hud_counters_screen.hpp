#pragma once

// HUD counters screen (51h, constructor 0052B030, vtable 00CED0AC): its +18h,
// +1Ch and +20h virtuals. The bodies are in src/menu_screens_2.cpp; this header
// is split out of bsp/menu_screens_2.hpp (which includes it) so the HUD host,
// whose other headers clash with that one, can bind them (goal 5 chain 662).
// Names are hypotheses, not recovered symbols.

#include <cstdint>
#include <vector>

namespace bsp {

struct HudCountersHost {
    virtual ~HudCountersHost() = default;
    virtual void widget8_virtual_34(std::int32_t arg) = 0;            // *(screen+8)
    virtual void counter_tick_0052b080(std::uintptr_t counter, float seconds) = 0;  // ECX = counter
    virtual void finish_00529810() = 0;                               // ECX = screen
};

struct HudCountersState {
    std::uintptr_t counter_0c{};                // +0Ch, ticked without a null test
    std::uintptr_t counter_10{};                // +10h
    std::vector<std::uintptr_t> counters_14;    // +14h array, +18h count; null entries skipped
};

// 00528C40 / 00528C50: slots +18h/+1Ch, __thiscall(screen), RET.
void hud_counters_enter_00528c40(HudCountersHost& host);
void hud_counters_exit_00528c50(HudCountersHost& host);
// 0052BAA0: slot +20h, __thiscall(screen, float seconds), RET 4.
void hud_counters_update_0052baa0(const HudCountersState& screen, float seconds, HudCountersHost& host);

}  // namespace bsp
