#pragma once

// Packet cc13-powerup-bind: the power-up manager's sixteen category lists
// (manager+80h + category*0Ch, list head +84h, size +88h) and their readers.
// Every gameplay modifier site of this image tests EnablePowerups [00E0C978] and
// the category list's size, then multiplies by 008E6430's product or takes the
// 1.0f at 00D7A24C. The bodies here are re-derived from the image bytes; the
// earlier src/powerup_seg58_argon.cpp copies served only as a cross-check and
// are not called. Names are hypotheses, not recovered symbols.
//
// Phase 1: the store, 008E6430 and 008E4680, and every host reader routed
// through powerup_site_product.
// Phase 2a: the writers a scene reaches without player input. 008ED720 (a
// command building's PowerupClassID, plain names only) grants through 008ED4B0,
// whose passive arm uses the item at once (008EADA0 -> 008EA250 inserts); the
// manager observes the building (00694A60) and 008ED620 regrants on code 5
// (00927D20's notify at 00927E31) after 008E8A30 removes the old item; 008EB110
// expires records past their end time through 008E8C30.
// Campaign rows: only IJN05 and USN16 author PowerupClassID, always
// "repairtest", which names no class (008E6CB0 misses), so no list fills there.
// The multi scenes grant plain names; nine of them (multi\duelOne, scene1 ..
// scene8) also author the bracket form "S[a,b,..]", which 008ED720 splits at
// each "]" and hands chunk by chunk to 008EC650 for a pick.
// Phase 2b: 008EC650. The chunk's first character picks the mode ('A' one
// fixed pick per chunk, 'T' picks without replacement, anything else 'S' with
// replacement); a "*" token becomes "<mode>*", and tokens naming a manager+1B8h
// random_names row expand to its names; manager+1ACh caches each chunk's list.
// The pick draws on BSP_RandomThreads stream 0 (00BD2FC0, ECX = 0): once when an
// 'A' chunk's list is built (008ECAAE) and once per 'S' / 'T' pick (008ECCF7).
// LABELLED stand-in: a stream-0 generator drawn only in game_hosts_powerups.cpp,
// held by the runtime and reset with the manager, so each scene starts the
// same sequence (the image's stream 0 runs on across scenes).
// Its seed and MT sequence are not modelled, so its values (and the picks)
// differ from the image's; stream 1, the gameplay stream, is unaffected. A
// chunk list with no names ('A') throws where the image divides by zero, and a
// tail with no "]" throws where 008ED720 loops forever.
// Follow-up (option ii): a shared stream-0 generator for the host, which the
// gunnery and HUD stream-0 consumers would draw on too, would replace this
// stand-in; until then those consumers and this one do not share a sequence.

#include <array>
#include <cstddef>
#include <cstdint>
#include <list>
#include <optional>
#include <string_view>

namespace bsp {
struct PowerupClassConfig;
namespace cc13_powerup {
struct PowerupItem;
}  // namespace cc13_powerup
}  // namespace bsp

namespace bsp::game_powerups {

// kPowerUpSubsystemCc13Bound (OFF): the host's modifier sites read the category
// lists below through their image gate. OFF: every site keeps its 1.0f stand-in
// (the host had no store). Sites, by category (powerupclasses.lua PUET_):
//   1 FIREPOWER 0047057F / 0047062F (00470510 / 004705C0 hull and part damage)
//   2 ARMOR 0087784F, 00877875, 00877965, 008779A3 (008777D0)
//   3 SHIP_REPAIR 0093C16A, 0093C25A, 0093C592, 0093C7C5, 0093C8C5; not hosted:
//     0082791B, in the undefined body 00827850..0082795B (not in 00826F10), which
//     nothing references (no rel32 or absolute dword); it returns
//     float((... * unit+1150h) * m) through RET 4. The HUD rows 006496A1 /
//     006496B7 are not hosted either
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
// ON (cc9-recon2): campaign identity. Pairs on afa0394fc (phases 1, 2a, 2b):
// IJN05 and USN16 stock (the "repairtest" scenes), USN02 and LOMP06 WINS rows,
// all gameplay identical; only the gameplay-scale description line differs.
inline constexpr bool kPowerUpSubsystemCc13Bound = true;

inline constexpr int kPowerupCategoryCount = 16;  // 008EB14E, 16 lists

// One modifier, the value at list node+8h, with 008EA250's template values
// (008EA27F..008EA2CA). 008E4680 reads +8h..+10h and 008E6430 multiplies by +14h.
struct PowerupModifier {
    cc13_powerup::PowerupItem* item{nullptr};  // +0h, the using item (null once released)
    std::int32_t category{16};      // +4h
    std::int32_t party{3};          // +8h, compared with unit+54h when below 2
    std::uint16_t unit_id{0};       // +0Ch, unit vtable[140h]()+174h; 0 = none
    std::uint16_t centre_id{0};     // +0Eh, the binding building's id; 0 = none
    std::int32_t slot{-1};          // +10h, compared with 009FFD20(unit); -1 = none
    float multiplier{1.0f};         // +14h
    float end_time{1.0e10f};        // +18h, 008EB110 expires it once the clock passes
    std::int32_t owner_slot{-1};    // +1Ch, the using slot
    const PowerupClassConfig* cls{nullptr};  // +20h holds the class name; its class here
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

// powerup_site_product throws when the gate is open but the unit has no view
// (rev5 N4). Three callers sit under a noexcept reconstruction, where a throw
// terminates: 009E1170's reference_speed (the commands host hands over a value
// computed before the call), 00822C20's 0080FC30 (StartSpeedSeedBinding reads a
// slot still under creation through powerup_site_product_for_view, no lookup)
// and 008777D0's armour (the victim is always in the units table). None reaches
// the throw unless the host itself is inconsistent.
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

// --- phase 2a --------------------------------------------------------------

// 008ED720 __thiscall(ECX = binding at building+758h, creation record), RET 4,
// for the building at units-host index `building` and its scene PowerupClassID
// text: the plain-name lookup (008E6CB0) and, on a hit, the grant 008ED4B0 and
// the observer registration 00694A60. Throws on the bracket form (008EC650).
void command_building_powerup_init_008ed720(std::size_t building,
    std::string_view powerup_class_id);

// 008ED620 __thiscall(ECX = manager, observed, code, arg), RET 0Ch: on code 5 for
// a unit the manager observes, 008E8A30 then 008ED4B0 on its binding.
void powerup_manager_observed_event_008ed620(std::size_t unit, int code);

// 008EB110's sweep (008EB139..008EB181): each category list in order, each
// record whose end time the clock has passed expires through 008E8C30.
void expire_due_modifiers_008eb110();

}  // namespace bsp::game_powerups
