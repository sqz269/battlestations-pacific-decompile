// Focused regression for review 1800: the production input binder must make
// the existing chooser reject surface-only/non-surface and immune-craft pairs.
// This fixture does not run the game or model native rack ownership.
#include "bsp/attack_commands.hpp"
#include "bsp/cc15_seg48.hpp"
#include "bsp/band_0080_cc13.hpp"
#include "bsp/projectile_kinds.hpp"
#include <cmath>
#include <cstdio>
#include <limits>
#include <stdexcept>

int main() {
    auto check = [](bool value, const char* label) {
        if (!value) throw std::runtime_error(label);
    };
    bsp::WeaponClassDescriptor slot;
    slot.sub_type = 0x0a;
    const auto accepts = [&](const bsp::WeaponClassDescriptor* first, bool surface,
        bool kind_0e = false, bool kind_0c = false, bool big = false, float speed = 0.0f) {
        bsp::AttackFeasibilityInputs in;
        in.unit_has_weapon_controller = in.target_present = in.has_torpedo_ordnance = true;
        in.unit_side = 1;
        in.target_side = 3;
        in.target_is_surface = surface;
        bsp::bind_attack_torpedo_inputs(in, first, kind_0e, kind_0c, big, speed);
        const bool result = bsp::cc15_seg48_unit_attack_command_applies_007ee8f0(
            bsp::kAttackCmdTorpedo, in);
        check(bsp::attack_command_choose(in, true, true)
            == (result ? bsp::kAttackCmdTorpedo : 0u), "chooser must use the bound torpedo predicate");
        return result;
    };
    check(!accepts(nullptr, false), "absent first slot retains BL=1");
    check(accepts(nullptr, true), "absent first slot permits a surface target");
    slot.sub_type = 1;
    check(!accepts(&slot, false), "a non-torpedo FIRST slot retains BL=1");
    slot.sub_type = 0x0a;
    slot.homing_horz_turn_speed = slot.homing_vert_turn_speed = 0.001f;
    check(!accepts(&slot, false), "both exact threshold endpoints retain BL=1");
    slot.homing_horz_turn_speed = std::nextafter(0.001f, 1.0f);
    check(accepts(&slot, false), "COMISS/JA clears BL above the horizontal threshold");
    slot.homing_horz_turn_speed = 0.0f;
    slot.homing_vert_turn_speed = std::nextafter(0.001f, 1.0f);
    check(accepts(&slot, false), "COMISS/JC clears BL above the vertical threshold");
    slot.homing_horz_turn_speed = std::numeric_limits<float>::quiet_NaN();
    slot.homing_vert_turn_speed = 0.0f;
    check(!accepts(&slot, false), "unordered horizontal comparison does not take JA");
    slot.homing_horz_turn_speed = 0.0f;
    slot.homing_vert_turn_speed = std::numeric_limits<float>::quiet_NaN();
    check(accepts(&slot, false), "unordered vertical comparison takes JC");

    constexpr float speed = bsp::cc13_band0080::kTorpedoProofSmallCraftThreshold;
    check(!accepts(&slot, true, true, false, true, speed), "kind 0Eh is immune at the speed threshold");
    check(accepts(&slot, true, true, false, false, std::nextafter(speed, 0.0f)),
        "kind 0Eh below the speed threshold is not immune");
    check(!accepts(&slot, true, false, true, false, speed), "small landing ship is immune");
    check(accepts(&slot, true, false, true, true, speed), "class+808h excludes big landing ships");
    check(accepts(&slot, true, false, false, false, speed), "speed alone does not confer immunity");
    check(accepts(&slot, true, true, false, false, std::numeric_limits<float>::quiet_NaN()),
        "00828EC0 COMISS/JB rejects unordered speed from immunity");
    std::puts("c826 review1800 torpedo input/chooser regression: PASS");
}
