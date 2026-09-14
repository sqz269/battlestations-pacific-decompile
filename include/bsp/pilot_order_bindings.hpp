#pragma once
#include <cstdint>

// The five mission-Lua pilot order bindings: `PilotSetTarget` 008A4C90,
// `PilotMoveTo` 008A4150, `PilotMoveToRange` 008A4590, `PilotRetreat` 008A4300
// and `PilotLand` 008A47B0. All five end at 0077D600 BSP_Entity_IssueCommand.
//
// docs/PILOT_ORDER_BINDINGS.md carries the evidence, the ABI and the coverage
// table. The five binding names and their entry points are recovered symbols:
// they come from the registration table at 00E0BC90, whose `{const char*, fn}`
// pairs name each address in .rdata. Everything else here is a hypothesis.
// Nothing in this header is a binary-compatible layout.
//
// Contracts named but not reconstructed:
//  * 004C7730, the world-object query `PilotRetreat` uses to find the retreat
//    zone. Its call shape is established (008A4429-008A4439); its body is not.
//  * 007EEC50 BSP_Unit_ChooseAttackCommand, reconstructed by an earlier packet
//    in docs/ATTACK_COMMANDS.md; modelled here as one host call.
#include "bsp/airfield_taxi.hpp"       // EntityLivenessBytes
#include "bsp/scene_deferred_refs.hpp"  // SceneCommandTarget

namespace bsp {

// ---------------------------------------------------------------------------
// Command class singletons (docs/SCENE_COMMAND_TYPES.md)
// ---------------------------------------------------------------------------

// 008A42A0 and 008A471C both push this; docs/ATTACK_COMMANDS.md names it `moveto`.
inline constexpr std::uint32_t kPilotOrderClassMoveTo = 0x00E08F68u;
// 008A44EF. `retreat`.
inline constexpr std::uint32_t kPilotOrderClassRetreat = 0x00E08F90u;
// 008A4900. `land`, SCENE_COMMAND_TYPES row 22.
inline constexpr std::uint32_t kPilotOrderClassLand = 0x00E08FA0u;

// Every one of the five pushes the literal 1 as 0077D600's third argument
// (008A4299, 008A444B, 008A471C, 008A48F9, 008A4EA2).
inline constexpr int kPilotOrderIssueFlags = 1;

// ---------------------------------------------------------------------------
// Lua argument slots
// ---------------------------------------------------------------------------

// 00B677E0 BSP_LuaObject_ArgumentAt takes a zero-based index and builds a
// LuaObject on stack slot `pack->base + index`, where 00B679B0 set base = 1
// (00B677C0 ADD ECX,[ESP+14h]). Index 0 is therefore the first Lua argument.
inline constexpr int kPilotOrderUnitArgument = 0;    // 008A4D6F, 008A422D, ...
inline constexpr int kPilotOrderTargetArgument = 1;  // 008A4DD8, 008A4266, ...
inline constexpr int kPilotOrderOptionArgument = 2;  // 008A4E19, 008A46EA

// ---------------------------------------------------------------------------
// `PilotSetTarget`'s third argument
// ---------------------------------------------------------------------------

// Recovered from the shipped scripts, not a hypothesis:
// scripts/global/luamw_init.lua:237-239 defines ATTACKTYPE_ANY = 1,
// ATTACKTYPE_GUN_ONLY = 2, ATTACKTYPE_BOMB_OR_TORPEDO = 3, and
// scripts/global/commandhelpers.lua:2936 documents them as this argument.
inline constexpr int kPilotAttackTypeAny = 1;
inline constexpr int kPilotAttackTypeGunOnly = 2;
inline constexpr int kPilotAttackTypeBombOrTorpedo = 3;

// 008A4D59 MOV EBP,1: the value when the binding does not read argument 2.
inline constexpr int kPilotAttackTypeDefault = kPilotAttackTypeAny;

// The two booleans 008A4E54-008A4E5F derive and push into 007EEC50.
struct PilotAttackSelectorFlags {
    bool prefer_ordnance = true;  // 007EEC50 arg 2, SETNZ AL after CMP EBP,2
    bool allow_guns = true;       // 007EEC50 arg 3, SETNZ BL after CMP EBP,3
};

// 008A4E54-008A4E5F.
PilotAttackSelectorFlags pilot_attack_selector_flags_008a4e54(int attack_type);

// 008A4E0B-008A4E38. `argc` is 00B663F0 (`pack+0Ch`, the lua_gettop 00B679B0
// captured). The read only happens on an exact match with 3; anything else
// leaves the default. `argument_as_int` is 00B66290 = _ftol(lua_tonumber),
// so a nil third argument yields 0, not the default.
int pilot_set_target_attack_type_008a4e0b(int argc, int argument_as_int);

// ---------------------------------------------------------------------------
// `PilotMoveToRange`'s third argument
// ---------------------------------------------------------------------------

// 008A46DC-008A4708. Same exact-3 gate, but 00B66270 (float lua_tonumber) and
// the result lands in the descriptor's +14h. The default is whatever 0088A810
// left there, and 0088A8C7 always clears it.
float pilot_move_to_range_008a46dc(int argc, float argument_as_float);

// ---------------------------------------------------------------------------
// `PilotRetreat`'s position rule
// ---------------------------------------------------------------------------

// The four Vector3 corners 004C7730's record holds at +10h, +1Ch, +28h and
// +34h, read at stride 0Ch. `PilotRetreat` touches only .x (+0h) and .z (+8h)
// of each: 008A443E/008A4448, 008A448C/008A4493, 008A44B2/008A44B9,
// 008A44D8/008A44DF.
struct PilotRetreatZoneCorners {
    float corner[4][3]{};
};

// FLD double [00D7A348] at 008A450C; the eight bytes are 3FD0000000000000.
inline constexpr float kPilotRetreatCornerWeight = 0.25f;

// 008A443E-008A452E. The mean of the four corners in x and z; y is forced to
// zero by the XORPS at 008A4445 stored at 008A4465, not taken from the corners.
void pilot_retreat_position_008a443e(const PilotRetreatZoneCorners& zone, float out[3]);

// ---------------------------------------------------------------------------
// `PilotSetTarget`'s gates
// ---------------------------------------------------------------------------

enum class PilotSetTargetOutcome {
    kUnitNotLive,     // 008A4DAA-008A4DD2, the unit fails the liveness quartet
    kTargetNotLive,   // 008A4E70-008A4E8A, the resolved target fails it
    kNoCommandClass,  // 008A4EA0, both 007EEC50 calls answered null
    kIssued,          // 008A4EA2, 0077D600 runs
};

// The literal control flow of 008A4DA8-008A4EAC. `second_selection` is only
// consulted when `first_selection` is null; 008A4E90-008A4E99 repeats the call
// with the same three arguments (see the doc: this duplication is faithful,
// not an analysis artefact).
PilotSetTargetOutcome pilot_set_target_gate_008a4da8(bool unit_live,
                                                     bool target_live,
                                                     bool first_selection,
                                                     bool second_selection);

// ---------------------------------------------------------------------------
// The five bindings as sequences over a host
// ---------------------------------------------------------------------------

// One virtual per native call site the bindings make, in body order. Nothing
// here has a default: no member stands in for unrecovered behaviour.
struct PilotOrderHost {
    virtual ~PilotOrderHost() = default;

    // 00B663F0, `pack+0Ch`: lua_gettop at frame open.
    virtual int argument_count() = 0;
    // 00B677E0 then 00888AA0: the Lua table's `Ptr` field (literal 00CFAD08)
    // converted with 00B662D0. NOT the `ID` field. May answer null.
    virtual void* unit_from_argument(int index) = 0;
    // 00B677E0 then 0088A810: the 18h-byte descriptor. Reads the table's `ID`
    // field (literal 00CE59B4) as the discriminator and, when it is not nil,
    // takes the pointer from `Ptr` through 00888AA0.
    virtual SceneCommandTarget command_target_from_argument(int index) = 0;
    // 00B66290, _ftol(lua_tonumber(L, base + index)).
    virtual int argument_as_int(int index) = 0;
    // 00B66270, (float)lua_tonumber(L, base + index).
    virtual float argument_as_float(int index) = 0;

    // The entity+5Ch/+5Dh/+5Eh/+60h quartet, read directly by the bindings.
    virtual EntityLivenessBytes liveness(void* entity) = 0;
    // 00521EA0 BSP_CommandTarget_ResolveObject; caches into descriptor +4h.
    virtual void* resolve_target_object(SceneCommandTarget& target) = 0;
    // 007EEC50 BSP_Unit_ChooseAttackCommand, ECX = unit.
    virtual void* choose_attack_command(void* unit, void* target,
                                        bool prefer_ordnance, bool allow_guns) = 0;
    // 0077D600 BSP_Entity_IssueCommand, ECX = unit.
    virtual void issue_command(void* unit, std::uint32_t command_class,
                               const SceneCommandTarget& target, int flags) = 0;

    // byte entity+C8h, the world-pose valid flag 00414DB0 sets.
    virtual bool world_pose_valid(void* entity) = 0;
    // 00414DB0 BSP_EntityPose_RefreshWorld.
    virtual void refresh_world_pose(void* entity) = 0;
    // int entity+54h, the side (docs/ATTACK_COMMANDS.md).
    virtual int entity_side(void* entity) = 0;
    // float[3] at entity+FCh, the world position 008A4432 passes by address.
    virtual const float* entity_world_position(void* entity) = 0;
    // 004C7730 on the world object [00E188A8], with (&position, side, 0, 0).
    // Body unreconstructed; only the four corners it yields are modelled.
    virtual PilotRetreatZoneCorners retreat_zone(const float position[3], int side) = 0;
};

// 008A4C90 `PilotSetTarget(unit, target [, attackType])`.
PilotSetTargetOutcome pilot_set_target_008a4c90(PilotOrderHost& host);
// 008A4150 `PilotMoveTo(unit, target)`.
void pilot_move_to_008a4150(PilotOrderHost& host);
// 008A4590 `PilotMoveToRange(unit, target [, range])`.
void pilot_move_to_range_008a4590(PilotOrderHost& host);
// 008A4300 `PilotRetreat(unit)`.
void pilot_retreat_008a4300(PilotOrderHost& host);
// 008A47B0 `PilotLand(plane, base)`.
void pilot_land_008a47b0(PilotOrderHost& host);

}  // namespace bsp
