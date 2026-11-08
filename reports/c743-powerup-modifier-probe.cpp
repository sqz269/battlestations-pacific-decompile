// Standalone Win32 fixture; run command is in wire-g5-c743.json. No game runs.
#include "bsp/game_hosts_powerups.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/powerup_config.hpp"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <float.h>
#include <list>
#include <stdexcept>

namespace {
void check(bool condition) {
    if (!condition) throw std::runtime_error("modifier lifetime fixture failed");
}
std::uint32_t bits(float value) {
    std::uint32_t out;
    std::memcpy(&out, &value, sizeof(out));
    return out;
}
}

int main() {
    unsigned saved_control = 0;
    _controlfp_s(&saved_control, 0, 0);
    unsigned ignored = 0;
    _controlfp_s(&ignored, _MCW_EM | _PC_24, _MCW_EM | _MCW_PC);
    auto& strings = bsp::game::game_native_string_process().strings();
    bsp::PowerupClassConfig cls{};
    cls.name_00.assign_0041e870(strings, "fixture powerup");
    {
        std::list<bsp::game_powerups::PowerupModifier> modifiers;
        // The actual 008EA250 host path: a native default temporary is copied
        // into the list, then the authored class name replaces its "0" name.
        modifiers.push_back(bsp::game_powerups::PowerupModifier{});
        auto& source = modifiers.front();
        check(!source.item && source.category == 16 && source.party == 3);
        check(!source.unit_id && !source.centre_id && source.slot == -1);
        check(bits(source.multiplier) == 0x3f800000u);
        check(bits(source.end_time) == 0x501502f9u && source.owner_slot == -1);
        check(source.name_20.length() == 1 && std::strcmp(source.name_20.data(), "0") == 0);
        source.bind_class(cls);
        source.category = 4;
        source.unit_id = 321;
        source.owner_slot = 2;
        const std::uint32_t signaling_nan = 0x7f800001u;
        std::memcpy(&source.multiplier, &signaling_nan, sizeof(signaling_nan));
        _clearfp();
        modifiers.push_back(source);
        auto& copy = modifiers.back();
        check((_statusfp() & _SW_INVALID) != 0);
        check(bits(copy.multiplier) == 0x7fc00001u);
        check(copy.category == 4 && copy.unit_id == 321 && copy.owner_slot == 2);
        check(copy.cls == &cls && copy.name_20.data() != source.name_20.data());
        check(copy.name_20.length() == cls.name_00.length());
        check(std::strcmp(copy.name_20.data(), cls.name_00.data()) == 0);
        modifiers.pop_front();
        check(std::strcmp(copy.name_20.data(), "fixture powerup") == 0);
        bsp::game_powerups::PowerupModifier assigned;
        assigned = copy;
        check(assigned.name_20.data() != copy.name_20.data());
        const char* before = assigned.name_20.data();
        assigned = assigned;
        check(assigned.name_20.data() == before);
    }
    cls.name_00.release_to(strings);
    _clearfp();
    _controlfp_s(&ignored, saved_control, _MCW_EM | _MCW_PC);
    std::puts("PASS: native defaults, x87 scalar copy, independent names, class binding, assignment and list retirement");
}
