// The scene-authored `StartSpeed` seed: what puts a non-zero throttle in a unit's
// order ring before the scene's `Cruise` command latches it.
//
// Packet cc_cruise_speed_setting. Every name below is a hypothesis, not a
// recovered symbol. docs/CRUISE_SPEED_SETTING.md carries the addresses, the
// evidence and the uncertainty for each rule.
//
// The chain the packet answers, end to end:
//
//   universe/Scenes/.../<mission>.scn      `StartSpeed = F 12.0000 ;`
//     -> 00925F20 BSP_SEntity_InitAll walks the pending-init list and calls the
//        entity's vtable slot 0A0h at 00926110
//     -> 00822C20 (that slot for a game unit) reads the key out of the entity's
//        property bag at 00823590 and, at 008235BA..008235F7,
//          ring +148h        = StartSpeed / 0080FC30(unit)     via 0080D9B0
//          hull axial speed  = 0080FC30(unit) * that ratio      via 0092D770
//     -> later, when the scene's queued `Cruise` becomes the unit's current
//        command, 00835E17 latches that same ring +148h as `cruiseThrust`
//        (bsp/cruise_command.hpp), so the ship holds the authored speed.
//
// Nothing on this path touches the commanded-speed pair at *(unit+73Ch) +24h /
// +28h. That pair has its own two producers, both Lua bindings, and they are
// bsp/unit_commanded_speed.hpp's, not this packet's.
#pragma once

#include <cstddef>
#include <cstdint>

#include "bsp/cruise_command.hpp"
#include "bsp/scene_property_bag.hpp"
#include "bsp/unit_state_message.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// The authored key
// ---------------------------------------------------------------------------
// Both literals are immediates in 00822C20's property-bag arm. `StartSpeed` is
// authored in 207 of the shipped `.scn` files; in usn_2_java.scn all fourteen
// `Cruise` units carry `StartSpeed = F 12.0000 ;` and nothing else in the file
// names a speed, a throttle or a heading.
inline constexpr const char* kSceneUnitStartSpeedKey = "StartSpeed";         // 00CFCCFC, 0082358B
inline constexpr const char* kSceneUnitShipYardLaunchKey = "ShipYardLaunch"; // 00D0659C, 0082356F

// ---------------------------------------------------------------------------
// The init slot and the entity's property-bag reference holder
// ---------------------------------------------------------------------------
// 00822C20 is not called directly: it is slot 0A0h of the game-unit vtable
// family. Five vtables hold it at that offset (00CF9150, 00CFA818, 00CFB7D8,
// 00CFC470, 00D09718) and four derived classes override the same slot with
// 0074BEC0, 007593D0, 00853630 and 00857C80. The one call site found for the
// slot is 00926110, inside BSP_SEntity_InitAll (00925F20-0092638A), whose
// callers include BSP_SceneFile_Read (0046DF00) and the fixed-step driver
// BSP_Game_RunFixedSimulationSteps (00875BB0).
inline constexpr std::size_t kUnitVtableSlotInitFromSceneProperties = 0xa0;

// entity+0C0h is the 12-byte reference holder scene entity creation stores there
// (bsp/scene_entity_create.hpp's kSceneCreateEntityPropertyBagRef). 00822C20
// reads its +4h as a **kind tag**, not as a reference count: 00922E20 (vtable
// 00D03D94) stores the literal 1 next to a cloned scene property bag at +8h, and
// 00774DC0 (vtable 00D03754) stores the literal 2 next to a different payload.
// 00823537..00823542 dispatches on exactly those two values and reads +8h as a
// property bag only for 1.
inline constexpr std::size_t kSceneEntityBagRefKindOffset = 0x04;    // 00823537
inline constexpr std::size_t kSceneEntityBagRefPayloadOffset = 0x08; // 0082356C
inline constexpr int kSceneEntityBagRefKindPropertyBag = 1;          // 0082353D, 00922E35
inline constexpr int kSceneEntityBagRefKindOther = 2;                // 00823542, 00774DE4

// ---------------------------------------------------------------------------
// Reading the authored value out of a property record
// ---------------------------------------------------------------------------
// 00823599..008235AA. The record's type word (kScenePropertyRecordTypeOffset)
// is compared against zero only: type 0 (`I`) is converted with CVTSI2SS, and
// **every other type**, not just type 1 (`F`), is read as a float32 straight out
// of kScenePropertyRecordValueOffset. Authored scenes only ever use `F` here, so
// the fall-through never bites in the shipped data, but the projection keeps the
// native's shape rather than a tidier one.
struct SceneStartSpeedProperty {
    bool present{false};                               // 00823597: the find returned 0
    ScenePropertyType type{ScenePropertyType::Float};  // record +4h
    std::int32_t value_int{0};                         // record +0Ch read as int
    float value_float{0.0f};                           // record +0Ch read as float32
};

// 00823599..008235AA, pure. Returns the authored speed in m/s.
float scene_start_speed_value_00823599(const SceneStartSpeedProperty& record) noexcept;

// ---------------------------------------------------------------------------
// The seed itself
// ---------------------------------------------------------------------------
// 008235B0..008235F7. The two values the native hands to its two callees. Both
// are float32 stores in the native and both are computed on the x87 stack:
//
//   008235B0  FLD   [ESP+14h]          ; the authored speed, promoted
//   008235B6  FSTP  double [ESP+20h]
//   008235BA  CALL  0080FC30           ; ST0 = the reference speed
//   008235BF  FDIVR double [ESP+20h]   ; ST0 = authored / reference
//   008235CA  FSTP  float [ESP+18h]    ; rounded to float32 -> ring_throttle
//   008235D5  CALL  0080D9B0           ; ring +148h = ring_throttle
//   008235DC  CALL  0080FC30           ; ST0 = the reference speed, fetched again
//   008235E1  FMUL  float [ESP+14h]    ; ST0 = reference * ring_throttle
//   008235EC  FSTP  float [ESP+18h]    ; rounded to float32 -> axial_speed
//   008235F7  CALL  0092D770           ; the hull's axial velocity component
//
// `axial_speed` is the authored speed round-tripped through the ratio, not the
// authored speed itself: 008235C3's PUSH shifts ESP by four, so 008235CA's
// [ESP+18h] overwrites the slot 008235AA wrote, and 008235E1's [ESP+14h] after
// 0080D9B0's RET 4 is that same slot. The two float32 roundings are why the
// projection reproduces them instead of passing the authored value through.
//
// 0080FC30 is called twice. The projection calls it twice too, because the
// native does; both calls are separate host steps in the report.
struct SceneStartSpeedSeed {
    bool authored{false};     // the key was present in the bag
    float start_speed{0.0f};  // the authored value, m/s
    float reference_speed{0.0f};  // 0080FC30(unit), from the first call at 008235BA
    float ring_throttle{0.0f};    // the float32 argument to 0080D9B0 at 008235D5
    float axial_speed{0.0f};      // the float32 argument to 0092D770 at 008235F7
    // 0082357F reads the `ShipYardLaunch` record's +0Ch byte into BL. The arm
    // only carries it; its consumer is 00823712..00823742 (unit+1130h becomes 5
    // when the byte is set and 0 otherwise), which is outside this projection.
    bool shipyard_launch{false};
};

// 008235B0..008235F7, pure in the authored speed and the two reference-speed
// readings. `reference_speed_second` is 0080FC30's result at 008235DC; pass the
// same value twice unless the gameplay scale is being varied deliberately.
SceneStartSpeedSeed scene_start_speed_seed_008235b0(float start_speed,
                                                    float reference_speed,
                                                    float reference_speed_second) noexcept;

// ---------------------------------------------------------------------------
// What a scene-authored `Cruise` ship ends up doing
// ---------------------------------------------------------------------------
// This composition is **not** a native routine. It chains three native rules in
// the order the game runs them, so that a host can answer "what speed does this
// cruise ship hold?" in one call:
//
//   1. scene_start_speed_seed_008235b0            (00822C20, at init)
//   2. cruise_command_begin_00835e17              (00835C70, when `Cruise`
//                                                  becomes the current command)
//   3. cruise_ordered_values_009e1170             (009E1170, every AI step)
//
// The ring the latch reads is the one step 1 seeded, which is the whole answer
// to why a scene `Cruise` ship moves in the game and stands still without the
// seed. The ring's rudder is untouched by the seed, so |rudder| < 0.01f holds
// and the latch captures the spawn heading rather than a rudder.
struct CruiseSpeedSettingOutcome {
    SceneStartSpeedSeed seed{};
    CruiseAutopilotFields latched{};
    CruiseOrderedValues ordered{};
};

CruiseSpeedSettingOutcome scene_cruise_ship_speed(const SceneStartSpeedProperty& record,
                                                  UnitOrderRing& ring,
                                                  float reference_speed,
                                                  float heading_radians,
                                                  float body_axis_speed,
                                                  const CruiseSpeedSetting& speed_setting =
                                                      CruiseSpeedSetting{}) noexcept;

// ---------------------------------------------------------------------------
// The host
// ---------------------------------------------------------------------------
// One pure-virtual method per native call site of 00822C20's property-bag arm,
// in the order the native makes them. `unit` and `bag` are opaque native
// addresses; the projection never dereferences them.
struct CruiseSpeedSettingHost {
    virtual ~CruiseSpeedSettingHost() = default;

    // 00823576: 008F2260(bag, "ShipYardLaunch"), the key pushed at 0082356F. The
    // found record's +0Ch byte is a direct load at 0082357F, so the host answers
    // with it; a missing key leaves BL at the XOR BL,BL of 00823527, `false`.
    virtual bool find_shipyard_launch_00823576(std::uint32_t bag, const char* key) = 0;

    // 00823590: 008F2260(bag, "StartSpeed"), the same callee at a second site.
    // The record's type word (0082359C) and its value (0082359E for an int,
    // 008235A5 for a float) are direct loads in the native, not calls, so the
    // host returns them together with the find.
    virtual SceneStartSpeedProperty find_start_speed_00823590(std::uint32_t bag,
                                                              const char* key) = 0;

    // 008235BA and 008235DC: 0080FC30(unit), the reference speed. Two call sites,
    // one callee; the host is asked twice because the native asks twice.
    virtual float unit_reference_speed_0080fc30(std::uint32_t unit) = 0;

    // 008235D5: 0080D9B0(unit+838h, ratio), the immediate ring throttle.
    virtual void set_order_ring_throttle_0080d9b0(std::uint32_t unit, float throttle) = 0;

    // 008235F7: 0092D770([unit+1018h], speed), the hull's axial velocity.
    virtual void set_controller_axial_speed_0092d770(std::uint32_t unit, float speed) = 0;
};

// Runs 0082356C..008235FB over the host, in native call order, and returns what
// it seeded. `bag_kind` is the holder's +4h at entity+0C0h: the arm runs only
// for kSceneEntityBagRefKindPropertyBag, which is 0082353D's branch. When the
// StartSpeed key is absent the arm makes neither reference-speed call and
// neither setter call, exactly as 00823597's JZ 008235FC does.
//
// Coverage: this is 00822C20's property-bag arm up to the CamoColor fetch at
// 008235FC, not the whole of 00822C20 (body 00822C20-00824B57).
SceneStartSpeedSeed run_start_speed_arm_0082356c(CruiseSpeedSettingHost& host,
                                                 std::uint32_t unit,
                                                 std::uint32_t bag,
                                                 int bag_kind) noexcept;

} // namespace bsp
