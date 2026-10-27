#pragma once

// Packet cc13-powerup-bind: the power-up manager's sixteen category lists
// (manager+80h + category*0Ch, list head +84h, size +88h) and their readers.
// Every gameplay modifier site of this image tests EnablePowerups [00E0C978] and
// the category list's size, then multiplies by 008E6430's product or takes the
// 1.0f at 00D7A24C. The bodies here are re-derived from the image bytes; the
// earlier src/powerup_seg58_argon.cpp copies served only as a cross-check and
// are not called. Names are hypotheses, not recovered symbols.
//
// Phase 1 (this commit's scope): the store, 008E6430 and 008E4680, and every
// host reader routed through powerup_site_product. Nothing inserts into the
// lists yet, so each gate fails and every site keeps its previous code: the
// switch ON is identical to OFF.

#include <array>
#include <cstddef>
#include <cstdint>
#include <list>
#include <optional>

namespace bsp::game_powerups {

// kPowerUpSubsystemCc13Bound (OFF): the host's modifier sites read the category
// lists below through their image gate. OFF: every site keeps its 1.0f stand-in
// (the host had no store). Sites, by category (powerupclasses.lua PUET_):
//   1 FIREPOWER 0047057F / 0047062F (00470510 / 004705C0 hull and part damage)
//   2 ARMOR 0087784F, 00877875, 00877965, 008779A3 (008777D0)
//   3 SHIP_REPAIR 0082791B (00826F10), 0093C16A, 0093C25A, 0093C592, 0093C7C5,
//     0093C8C5; the HUD rows 006496A1 / 006496B7 are not hosted
//   4 SHIP_SPEED 0080FC4F (0080FC30), 00824252 / 00824538 / 00824736 (00822C20),
//     00826A21 (00825F20)
//   5 SHIP_TURNFACTOR 008118BA (00811890), 00811A59 (00811A30)
//   6 PLANE_SPEED 007DB7DB      7 TARGETING_ERROR 00730619 (through 00470440)
//   8 DEVICE_RELOADING 0072CF4E 9 TURBO_RELOADING 007CEBCC
//   10 CAPTURE_POWER 006F69B9, 006F6A13, 006F6B4C, 006F6BA3, 006F6CFA
//   11 CAPTURE_RESISTANCE 006F6D90
//   12 RECON 008043D5, 0080466E, 0080491E
//   13 PLANE_TURN 007DA067      14 TORPEDO_SPEED 008576A4
//   15 TORPEDO_TURN 008570B8
// The host file and line of each site carry the same address.
inline constexpr bool kPowerUpSubsystemCc13Bound = false;

inline constexpr int kPowerupCategoryCount = 16;  // 008EB14E, 16 lists

// One modifier, the value at list node+8h. 008E4680 reads +8h..+10h and
// 008E6430 multiplies by +14h. The fields the insert (008EA250) writes besides
// these are added with the insert.
struct PowerupModifier {
    std::int32_t party{2};          // +8h, compared with unit+54h when below 2
    std::uint16_t unit_id{0};       // +0Ch, unit vtable[140h]()+174h; 0 = none
    std::uint16_t centre_id{0};     // +0Eh, the binding building's id; 0 = none
    std::int32_t slot{-1};          // +10h, compared with 009FFD20(unit); -1 = none
    float multiplier{1.0f};         // +14h
};

using PowerupCategoryLists = std::array<std::list<PowerupModifier>, kPowerupCategoryCount>;

// What 008E4680 reads off a unit.
struct PowerupUnitView {
    bool live{false};               // 0043F080: +5Ch set, +5Dh, +60h, +5Eh clear
    std::int32_t party{0};          // +54h
    std::uint16_t id{0};            // vtable[140h]() (0047F320 returns ECX) +174h
    std::int32_t slot{-1};          // 009FFD20
    float position[3]{};            // 00427EB0
};

// The units a modifier names. The mission frame publishes one with the manager.
class PowerupUnitResolver {
public:
    virtual ~PowerupUnitResolver() = default;
    // False when the index names no unit.
    virtual bool view_unit(std::size_t unit_index, PowerupUnitView& out) const = 0;
    // 008E4710..008E474F: the id table lookup; false when the id names no unit.
    virtual bool view_unit_by_id(std::uint16_t id, PowerupUnitView& out) const = 0;
};

// 008E4680 __thiscall(ECX = manager, record = node+8h, unit), RET 8.
bool powerup_target_applies_to_unit_008e4680(const PowerupModifier& record,
    const PowerupUnitView& unit, const PowerupUnitResolver& resolver);

// 008E6430 __thiscall(ECX = manager, category, unit), RET 8: 1.0f times the
// +14h of every record of the list 008E4680 accepts, in list order.
float gameplay_modifier_product_008e6430(const std::list<PowerupModifier>& list,
    const PowerupUnitView& unit, const PowerupUnitResolver& resolver);

// A site's gate and call: the product when EnablePowerups and the category list
// is not empty, else nullopt (the site's 1.0f arm, which the caller keeps as its
// previous code). `unit_index` is the units-host index of the unit the site
// passes.
std::optional<float> powerup_site_product(int category, std::size_t unit_index);
// The same for an entity the units host does not hold (a torpedo projectile at
// 008570B8 / 008576A4), described by the caller.
std::optional<float> powerup_site_product_for_view(int category, const PowerupUnitView& view);

// The resolver the sites use, published by the mission frame while its manager
// exists; null clears it.
void publish_powerup_unit_resolver(const PowerupUnitResolver* resolver);

}  // namespace bsp::game_powerups
