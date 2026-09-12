// The gameplay tuning settings the ship AI reads, their Lua keys, and the
// per-unit navigator parameter block the attackmove approach calls `tune`.
//
// Packet cc_ai_settings_block, read-only Ghidra analysis. Every name below is a
// hypothesis built from the shipped Lua key strings; none is a recovered symbol.
// docs/SHIP_AI_SETTINGS_BLOCK.md carries the address, evidence, original ABI and
// uncertainty for every rule and every table row here.
//
// This header adds no settings field names: the singleton's layout lives in
// bsp/gameplay_settings.hpp (`GameplayTuningSettings`) and its four weapon
// accuracy sub-objects in bsp/gameplay_settings_tail.hpp. The navigator block's
// offsets live in bsp/unit_commanded_speed.hpp. What is new here is the binding
// between them: which offsets the ship AI reads, at which sites, and the copy
// that turns `GameplayTuningSettings +160h..+178h` into the approach `tune`
// block that bsp/ship_ai_approach_update.hpp asks its host for.
#pragma once

#include <cstddef>
#include <cstdint>

#include "bsp/gameplay_settings.hpp"
#include "bsp/gameplay_settings_tail.hpp"
#include "bsp/unit_commanded_speed.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// The approach tuning block, `tune`
// ---------------------------------------------------------------------------
// docs/SHIP_AI_APPROACH_UPDATE.md and docs/SHIP_AI_RING_SCAN.md call
// `[brain+0AB0h]` the tuning block and record that it has no producer. It does.
//
//   009F11A4  MOV EAX,[EDI+73Ch]   brain+0AB0h = *(unit+73Ch)
//
// in BSP_ShipAi_BrainRecordConstruct, so `tune` is the navigator parameter block
// bsp/unit_commanded_speed.hpp already describes, and its first seven floats are
// the `ShipGlobals.AttackMoveDirector` sub-table copied by 00822B70.
//
// The layout below is the block as the ship AI reads it. The allocation size is
// still unread (0081F283 allocates through a helper this packet did not open),
// so there is no size assertion; the fields above +2Ch are unknown, not absent.
struct UnitNavigatorParamsBlock {
    // +0h..+18h: the AttackMoveDirector copy. Constructor seeds at
    // 0081F214..0081F269; 00822B70 overwrites all seven at 00822B95..00822C0E.
    float my_damage_weight;         // +00h  AttackMoveDirector.MyDamageWeight
    float ideal_dist_weight;        // +04h  AttackMoveDirector.IdealDistWeight
    float nearby_enemy_weight;      // +08h  AttackMoveDirector.NearbyEnemyWeight
    float nearby_enemy_reference;   // +0Ch  AttackMoveDirector.NearbyEnemyReference
    float nearest_move_dir_weight;  // +10h  AttackMoveDirector.NearestMoveDirWeight
    float prev_move_dir_weight;     // +14h  AttackMoveDirector.PrevMoveDirWeight
    float prev_move_dir_range;      // +18h  AttackMoveDirector.PrevMoveDirRange
    // +1Ch: the explicit standoff-range override 009E6EC5 tests for `>= 0.0f`.
    // 00822B70 does not touch it; the only writer found in the image is the
    // constructor's -1.0f at 0081F26E.
    float range_override;           // +1Ch
    std::uint8_t allow_max_depth;             // +20h  00822B84, 008A33FF
    std::uint8_t force_move_close_to_target;  // +21h  0081F27D, 008A35A4
    std::uint8_t pad_22h[2];
    float commanded_speed;          // +24h  00890E86, 008A3901
    float commanded_speed_time;     // +28h  00890E97, 008A3912
};

static_assert(offsetof(UnitNavigatorParamsBlock, my_damage_weight) == 0x00, "+00h 00822BAD");
static_assert(offsetof(UnitNavigatorParamsBlock, ideal_dist_weight) == 0x04, "+04h 00822B99");
static_assert(offsetof(UnitNavigatorParamsBlock, prev_move_dir_range) == 0x18, "+18h 00822C10");
static_assert(offsetof(UnitNavigatorParamsBlock, range_override) == 0x1C, "+1Ch 0081F26E");
static_assert(offsetof(UnitNavigatorParamsBlock, allow_max_depth) == 0x20, "+20h 00822B84");
static_assert(offsetof(UnitNavigatorParamsBlock, commanded_speed) == 0x24, "+24h 00890E86");
static_assert(offsetof(UnitNavigatorParamsBlock, commanded_speed_time) == 0x28, "+28h 00890E97");

// The number of floats 00822B70 copies, and the stride between them: seven
// consecutive singles, settings +160h + 4*i -> block +4*i, i in [0,7).
inline constexpr int kNavigatorTuningFieldCount = 7;          // 00822BAD..00822C10
inline constexpr std::size_t kNavigatorTuningFieldStride = 4; // consecutive FSTP displacements

// 0081F214..0081F278, the block's constructor seed inside
// BSP_UnitVehicleBase_Construct. The seven tuning floats are seeded with the
// loader's own AttackMoveDirector fallbacks, so a unit built before the settings
// singleton is loaded still runs the authored defaults.
//   +00h [00CE38B8] 10.0    +04h [00CE3D34] 4.0     +08h [00CE3854] 3.0
//   +0Ch [00CE3804] 1000.0  +10h [00D7A24C] 1.0     +14h [00CE3868] 0.25
//   +18h [00D05AAC] 1.0471976 (DEG(60))
//   +1Ch/+24h/+28h [00D7A260] -1.0   +21h 1
void seed_navigator_params_0081f214(UnitNavigatorParamsBlock& block) noexcept;

// 00822B70 BSP_UnitInstance_ResetNavigatorParams, __thiscall(unit)(char reset),
// RET 4, body 00822B70-00822C15, complete. When `reset` is zero the whole body
// is skipped. Otherwise +20h is set to 1 (00822B84) and the seven tuning floats
// are refilled from the singleton. The image reloads the singleton pointer for
// every field (seven separate calls to 00424C40 at 00822B88..00822BFF) and
// writes +04h before +00h; neither is observable, so the projection writes them
// in offset order.
void apply_navigator_params_from_settings_00822b70(const GameplayTuningSettings& settings,
                                                   UnitNavigatorParamsBlock& block,
                                                   bool reset) noexcept;

// ---------------------------------------------------------------------------
// The `AvoidAllShipCollision` gate, settings +4h
// ---------------------------------------------------------------------------
// +4h carries no Lua key, but it is not uninitialised. The loader stores the
// immediate 1 at 0083BCD5, on its straight-line path right after the last
// ShipAvoidance key; the constructor 00424A10 never touches it. The only other
// writer is the Lua binding `luaMW_NavigatorSetAvoidAllShipCollision` (008D0740),
// which stores the boolean argument at 008D0852. Four ship AI routines gate on it.
inline constexpr std::size_t kGameSettingsOffAvoidAllShipCollision = 0x004;  // 0083BCD5, 008D0852
inline constexpr bool kAvoidAllShipCollisionLoaderDefault = true;           // 0083BCD5

// 009F1B76..009F1B9A in BSP_ShipAi_BrainPrePass: `XOR ECX,ECX / CMP byte
// [EAX+4],CL / SETNZ CL / LEA ECX,[ECX*4-1]`, stored at brain+3F8h. The value is
// a side filter, not a boolean: 3 when the setting is set, -1 when it is clear.
inline constexpr int kShipAvoidanceSideAll = 3;       // 009F1B93 with CL = 1
inline constexpr int kShipAvoidanceSideDisabled = -1; // 009F1B93 with CL = 0
int ship_avoidance_side_filter_009f1b78(bool avoid_all_ship_collision) noexcept;

// 009EC79B..009EC7BE, the tail of the obstacle-side predicate. Partial
// projection: the two gates before it are not modelled here, because both need
// a host. They are `(int)record+3F0h < 0` (009EC773) and the byte at
// `0080E160(record+3FCh)+241h` (009EC787), and the settings gate itself
// (009EC795). Call this only once all three have passed.
bool ship_avoidance_side_accepted_009ec79b(int query_side, int record_side_filter) noexcept;

// ---------------------------------------------------------------------------
// The weapon hit accuracy accessor, settings +240h..+39Fh
// ---------------------------------------------------------------------------
// docs/GAMEPLAY_SETTINGS_TAIL.md records the four 58h sub-objects and says their
// consumer is unread, reached "through a computed base". The computed base is
// 008387B0 `ADD ECX,240h/298h/2F0h/348h`: a __thiscall on the settings singleton
// that selects the category from an integer weapon kind. BSP_ShipAi_ApproachFrameState
// calls it at 009F2D87 with kind 7.
//
// __thiscall(settings)(int kind, float range, float scale, float target_size),
// RET 10h, body 008387B0-00838897, complete:
//     008387B0(kind, range, scale, size) = 00838530(profile, range, size) * scale
// with an unmatched kind returning `scale` unchanged (00838891).
bool weapon_hit_accuracy_category_008387b0(int weapon_kind,
                                           WeaponHitAccuracyCategory& out) noexcept;

// ---------------------------------------------------------------------------
// The tables
// ---------------------------------------------------------------------------
// How the loader reads one key. 00B66270 takes no fallback, so a missing key
// leaves whatever BSP_LuaObject_GetNumber returns; 00B66330 carries a float
// fallback pushed from .rdata in the call itself.
enum class ShipAiSettingsGetter : int {
    kNumber = 0,         // 00B66270 BSP_LuaObject_GetNumber, no fallback
    kFloatOrDefault = 1, // 00B66330 BSP_LuaReference_GetFloatOrDefault
    kBoolean = 2,        // 00B66250, the Lua binding path only
};

// One row of the loader-order table: the keys 0083B5E0 writes that the ship AI
// later reads, in the order the loader's getter call sites appear.
struct ShipAiSettingsKeyRecord {
    std::uint32_t settings_offset;   // displacement on the singleton
    const char* lua_path;            // path under the global `ShipGlobals`
    int array_index;                 // 1-based Lua array index, 0 when scalar
    ShipAiSettingsGetter getter;
    float loader_default;            // meaningful only for kFloatOrDefault
    float installed_value;           // scripts/datatables/shipglobals.lua
    std::uint32_t loader_site;       // the getter CALL site in 0083B5E0
    std::uint32_t store_site;        // the store into the singleton
};

// One settings read by a ship AI routine.
struct ShipAiSettingsReadSite {
    std::uint32_t settings_offset;
    std::uint32_t reader_function;   // the Ghidra function that contains the read
    std::uint32_t getter_call_site;  // the CALL 00424C40 the pointer came from
    std::uint32_t read_site;         // the instruction that dereferences it
};

const ShipAiSettingsKeyRecord* ship_ai_settings_keys(std::size_t& count) noexcept;
const ShipAiSettingsReadSite* ship_ai_settings_read_sites(std::size_t& count) noexcept;

// ---------------------------------------------------------------------------
// The host the executable must implement, in call order
// ---------------------------------------------------------------------------
// Steps 1..7 are one run of 0083B5E0 at process start; step 8 runs per unit
// during BSP_UnitInstance_SEntityInit; step 9 is the mission script, which may
// never run. docs/SHIP_AI_SETTINGS_BLOCK.md's host table carries the native call
// site for each method.
struct ShipAiSettingsHost {
    virtual ~ShipAiSettingsHost() = default;

    // 1. 0083B60E / 0083B625: construct the Lua state owner and open it with 41h.
    virtual void open_script_state_0083b60e() = 0;
    // 2. 0083B672: run `Scripts\global\luaMW_init.lua` (1Dh characters).
    virtual void run_script_0083b672(const char* path) = 0;
    // 3. 0083B6E6: run `Scripts\datatables\ShipGlobals.lua` (22h characters).
    virtual void run_script_0083b6e6(const char* path) = 0;
    // 4. 0083B721 / 0083B73D: take the globals table and select `ShipGlobals`.
    virtual void select_global_table_0083b73d(const char* name) = 0;
    // 5. 00B67690, ten sites: enter a sub-table by assigning the child wrapper
    //    into the current slot. This is what makes `Navigator.AutoThrust` and
    //    `Navigator.TurnMultipliers` read against their own parent.
    virtual void enter_subtable_00b67690(const char* name) = 0;
    // 6. 00B66270 and 00B66330, one per row of ship_ai_settings_keys(): read the
    //    key (optionally its 1-based array element through 00B67720) and store
    //    the result at the row's `settings_offset`.
    virtual float read_number_00b66270(const char* path, int array_index) = 0;
    virtual float read_float_or_default_00b66330(const char* path, int array_index,
                                                 float fallback) = 0;
    // 7. 00842937: close the state.
    virtual void close_script_state_00842937() = 0;
    // 8. 00822CCB in BSP_UnitInstance_SEntityInit: per unit, with a non-zero
    //    reset, copy +160h..+178h into *(unit+73Ch)+0h..+18h. The other call
    //    site, 00835C65, passes a literal 0 and is a no-op.
    virtual void reset_navigator_params_00822ccb(std::uint32_t unit, bool reset) = 0;
    // 9. 008D0852 in luaMW_NavigatorSetAvoidAllShipCollision: the mission script
    //    writes settings+4h. Only usn_09_leyte.lua calls it in the shipped data,
    //    with `false`.
    virtual void set_avoid_all_ship_collision_008d0852(bool value) = 0;
};

}  // namespace bsp
