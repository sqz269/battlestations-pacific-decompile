#pragma once

// Segment 50 (0080DA00-0081AA10) reference models: the barrel counts behind the
// firepower ratios, the ship's command availability (vtable[168h]), the
// "engine"/"steering" control factors, the avoid-zone point test and the helm
// record's save keys. Packet cc13-gpr50; docs/CC13_GPR_50.md.
//
// Most of segment 50's gameplay is already hosted (the jams and failures in
// src/game_hosts_gunnery.cpp, torpedo stock and loaded tubes, the turn circle,
// the wake samples, torpedo threats and the transport-ready test in the ship AI
// and AI hosts); those have pointer records only. These are semantic
// interfaces read from the disk bytes, not ABI-compatible replacements. Field
// names are hypotheses; "_NNN" is the native byte offset. Build-tested only;
// not wired, not game-run.
//
// ADOPTION NOTE (cc9 terms; the full table is in docs/CC13_GPR_50.md):
// - Replaces:
//   * the barrel counts (00812310..00812730): nothing; they are the inputs that
//     band_0080_cc13's refresh_firepower_ratios_008127e0 takes as given. The
//     ratios feed the Lua native GetFirepower (0088DC10), which no host binds.
//   * ship_command_available_00812b00: the ShipAiCommandAvailabilityHost
//     callback entity_offers_command_vtable168 (include/bsp/ship_ai_states.hpp)
//     that 008162B0 asks; no host implements that interface today, and the
//     script-order host asks 00779D50 directly (game_hosts_script_orders.cpp:3046).
//   * the control factors 008131E0 / 00813250: nothing; their only callers are
//     the Lua natives SetControlFactor / GetControlFactor (0088EB50, 0088EDC0),
//     listed unbound in src/mission_lua_host.cpp:45-46.
// - Inputs: the unit's devices per category (unit+390h + c*0Ch lists), each
//   device's barrel count +448h, disabled byte +3B8h, kind 20h test, class
//   function +3F4h->+80h; the weapon director +738h; the control factor fields
//   +9D8h / +9DCh; the navigation group's avoid zones.
// - RNG: none.
// - OFF-switch prediction: each model off changes nothing. On: GetFirepower
//   answers live/total ratios (scripts that read it can branch differently);
//   008162B0's vtable[168h] refuses a ship any command outside the twelve
//   names, and "artillery"/"torpedo" when no operational device of that kind is
//   left; SetControlFactor has no caller in this installation's scripts.

#include <cstdint>
#include <vector>

namespace bsp {
namespace gameplay_seg50_cc13 {

// ---------------------------------------------------------------------------
// 1. Barrel counts (the device iterator 00952080 / 009520E0 / 00952140 walks the
//    unit's per-category lists, unit+394h count and +398h head per category c at
//    stride 0Ch, in the order of a category list ended by a value >= 0Ch).
// ---------------------------------------------------------------------------
inline constexpr std::int32_t kArtilleryCategories_00e0a4f8[] = {1, 2, 3, 4, 6};
inline constexpr std::int32_t kAaCategories_00e0a510[] = {1, 5, 6};
inline constexpr std::int32_t kTorpedoCategories_00e0a520[] = {7};
inline constexpr std::int32_t kDepthChargeCategories_00e0a528[] = {8, 9};

struct UnitDevice {
    std::int32_t category{0};      // the per-category list the device sits in
    std::int32_t barrels_448{0};   // gun+448h, the barrel count
    bool disabled_3b8{false};      // gun+3B8h
};

// Totals (stored by 0081F980 at unit+103Ch..+1048h).
float total_artillery_barrels_00812310(const std::vector<UnitDevice>& devices);
float total_aa_barrels_008123a0(const std::vector<UnitDevice>& devices);
float total_torpedo_barrels_00812430(const std::vector<UnitDevice>& devices);
float total_depth_charge_barrels_008124c0(const std::vector<UnitDevice>& devices);
// Live sums (sole caller 008127E0): a disabled device adds 0.
float live_artillery_barrels_00812550(const std::vector<UnitDevice>& devices);
// 00812600 adds 1.0 (qword 00D7A210) per AA-list device, disabled or not; it
// reads neither +448h nor +3B8h.
float aa_mount_count_00812600(const std::vector<UnitDevice>& devices);
float live_torpedo_barrels_00812680(const std::vector<UnitDevice>& devices);
float live_depth_charge_barrels_00812730(const std::vector<UnitDevice>& devices);

// ---------------------------------------------------------------------------
// 2. Ship vtable[168h], 00812B00 (00CFC3D0 family; eight ship vtables; the
//    mothership's 007582F0 calls it too).
// ---------------------------------------------------------------------------
struct ShipChildDevice {
    bool kind_20{false};             // vtable[5Ch](20h)
    bool artillery_kind_005459b0{false};
    bool operational_00729f10{false};
    std::int32_t class_function_80{0};  // [child+3F4h]+80h
};
class ShipCommandHost {
public:
    virtual ~ShipCommandHost() = default;
    virtual bool director_has_target() = 0;  // [unit+738h]->vtable[2Ch]() != 0
};
// `children` is the unit's +48h chain (next at +44h), in order.
bool ship_command_available_00812b00(const char* command,
                                     const std::vector<ShipChildDevice>& children,
                                     ShipCommandHost& host);

// ---------------------------------------------------------------------------
// 3. Control factors: ship vtable[1A0h] 008131E0 (set) and [1A4h] 00813250 (get,
//    no Ghidra function). "engine" is unit+9D8h, "steering" unit+9DCh.
// ---------------------------------------------------------------------------
struct ControlFactors {
    float engine_9d8{0.0f};
    float steering_9dc{0.0f};
};
// `name` is the native string's data; `name_length` its recorded length.
bool set_control_factor_008131e0(ControlFactors& f, const char* name,
                                 std::uint32_t name_length, float value);
bool get_control_factor_00813250(const ControlFactors& f, const char* name,
                                 std::uint32_t name_length, float& out);

// ---------------------------------------------------------------------------
// 4. Ship vtable[170h], 00812C90: true when no zone of the class navigation
//    group at depth 0 (0082ADA0) contains (x, z) (004178F0).
// ---------------------------------------------------------------------------
class AvoidZoneHost {
public:
    virtual ~AvoidZoneHost() = default;
    virtual bool zone_contains_004178f0(float x, float z) = 0;
};
bool point_outside_avoid_zones_00812c90(float x, float z, AvoidZoneHost& host);

// ---------------------------------------------------------------------------
// 5. The helm record {thrust +0h, toTurn +4h}: 00815520 reads each through the
//    archive's vtable[10h]({0, key}, {2, &field}) (sole caller 0081F980), and
//    00815590 (no Ghidra function) writes each value through vtable[0Ch].
// ---------------------------------------------------------------------------
struct HelmSaveKey {
    const char* key;
    std::uint32_t offset;
    int type;
};
inline constexpr HelmSaveKey kHelmSaveKeys_00815520[] = {
    {"thrust", 0x0, 2},
    {"toTurn", 0x4, 2},
};

}  // namespace gameplay_seg50_cc13
}  // namespace bsp
