// Packet cc13-powerup-bind: the power-up category lists and their product. See
// include/bsp/game_hosts_powerups.hpp. Re-derived from the image bytes
// (disasm-raw); src/powerup_seg58_argon.cpp was a cross-check only. Names are
// hypotheses, not recovered symbols.

#include "bsp/game_hosts_powerups.hpp"

#include "bsp/cc13_powerup.hpp"
#include "bsp/cc13_x87_model.hpp"

#include <stdexcept>

namespace bsp::game_powerups {

namespace {

const PowerupUnitResolver* g_unit_resolver = nullptr;

// [00CE6A04], 1000000.0f: what 006F58D0, slot 0 of the command-building binding
// vtable 00D16194, loads. 008E477E..008E4786 reach it as the centre unit's
// vtable[64h]() (its binding at +758h) then that object's slot 0.
// PROVISIONAL: a centre id names the binding's building, the only unit kind
// whose vtable[64h] is not the null 0042B920 of the root vtable 00CE6290.
constexpr float kBindingRadiusSquared = 1000000.0f;

}  // namespace

// [BSP reconstruction 008e4680]
// Original ABI: __thiscall(ECX = manager, const record* = node+8h, unit*), RET 8;
// returns AL.
bool powerup_target_applies_to_unit_008e4680(const PowerupModifier& record,
    const PowerupUnitView& unit, const PowerupUnitResolver& resolver) {
    // 008E4689..008E46D3: the unit-id arm, only for a live unit. 008E46B9 calls
    // vtable[140h] (0047F320 in the root vtable: MOV EAX,ECX) and compares its
    // +174h word with record+0Ch.
    bool matched = record.unit_id != 0 && unit.live && record.unit_id == unit.id;
    // 008E46D5..008E46EB: the party arm, a signed record+8h below 2 equal to
    // unit+54h (read with no liveness test).
    if (record.party < 2 && record.party == unit.party) matched = true;
    // 008E46ED..008E4708: the slot arm, 009FFD20(unit) when record+10h is not -1.
    if (record.slot != -1 && record.slot == unit.slot) matched = true;
    if (matched) return true;  // 008E470A JNE 008E47CE
    // 008E4710..008E4717: no centre, no match.
    if (record.centre_id == 0) return false;
    // 008E471D..008E4755: the id table lookup; a missing unit fails.
    PowerupUnitView centre;
    if (!resolver.view_unit_by_id(record.centre_id, centre)) return false;
    // 008E4757..008E4775: a live centre on the unit's party.
    if (!centre.live || centre.party != unit.party) return false;
    // 008E4777..008E47A9: the radius (FSTP qword, kBindingRadiusSquared above),
    // then 00427C90 (centre position minus unit position, three float stores)
    // and 00427E30, the squared length (y*y + x*x) + z*z, FSTP to float at
    // 00427E49. Per x87 operation at the main thread's 24-bit precision.
    namespace x8 = bsp::cc13x87;
    const float dx = centre.position[0] - unit.position[0];
    const float dy = centre.position[1] - unit.position[1];
    const float dz = centre.position[2] - unit.position[2];
    const float length_sq = static_cast<float>(x8::x87_add24(
        x8::x87_add24(x8::x87_mul24(dy, dy), x8::x87_mul24(dx, dx)),
        x8::x87_mul24(dz, dz)));
    // 008E47AE..008E47B6: FLD radius; FCOMI ST(1); JBE fails. A match is
    // radius > length_sq.
    return static_cast<double>(kBindingRadiusSquared) > static_cast<double>(length_sq);
}

// [BSP reconstruction 008e6430]
// Original ABI: __thiscall(ECX = manager, category, unit*), RET 8; ST0 result.
float gameplay_modifier_product_008e6430(const std::list<PowerupModifier>& list,
    const PowerupUnitView& unit, const PowerupUnitResolver& resolver) {
    // 008E6435: the product starts at 1.0f (00D7A24C); 008E6460..008E6490 walk
    // the list from the head's next in order; each accepted record's +14h
    // (node+1Ch) multiplies it, FSTP to the float slot at 008E6480.
    float product = 1.0f;
    for (const PowerupModifier& record : list) {
        if (powerup_target_applies_to_unit_008e4680(record, unit, resolver)) {
            product = record.multiplier * product;
        }
    }
    return product;
}

namespace {

// The sites' gate (00470440 shows it whole): CMP BYTE [00E0C978],0, then the
// size at [00F88C30]+88h+category*0Ch. The host has no manager when the class
// registry did not load; its lists are then empty. Null when the gate is shut.
const std::list<PowerupModifier>* gated_list(int category) {
    cc13_powerup::PowerupRuntime& runtime = cc13_powerup::powerup_runtime();
    if (runtime.manager == nullptr || runtime.host == nullptr ||
        !runtime.host->powerups_enabled_00e0c978() || category < 0 ||
        category >= kPowerupCategoryCount) {
        return nullptr;
    }
    const std::list<PowerupModifier>& list =
        runtime.category_lists[static_cast<std::size_t>(category)];
    return list.empty() ? nullptr : &list;
}

}  // namespace

std::optional<float> powerup_site_product(int category, std::size_t unit_index) {
    std::optional<float> result;
    if constexpr (kPowerUpSubsystemCc13Bound) {
        if (const std::list<PowerupModifier>* list = gated_list(category)) {
            PowerupUnitView unit;
            if (g_unit_resolver == nullptr || !g_unit_resolver->view_unit(unit_index, unit)) {
                throw std::logic_error("008E6430 without a unit view");
            }
            result = gameplay_modifier_product_008e6430(*list, unit, *g_unit_resolver);
        }
    } else {
        (void)category;
        (void)unit_index;
    }
    return result;
}

std::optional<float> powerup_site_product_for_view(int category, const PowerupUnitView& view) {
    std::optional<float> result;
    if constexpr (kPowerUpSubsystemCc13Bound) {
        if (const std::list<PowerupModifier>* list = gated_list(category)) {
            if (g_unit_resolver == nullptr) {
                throw std::logic_error("008E6430 without a unit resolver");
            }
            result = gameplay_modifier_product_008e6430(*list, view, *g_unit_resolver);
        }
    } else {
        (void)category;
        (void)view;
    }
    return result;
}

void publish_powerup_unit_resolver(const PowerupUnitResolver* resolver) {
    g_unit_resolver = resolver;
}

}  // namespace bsp::game_powerups
