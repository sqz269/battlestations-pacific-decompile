#pragma once

#include <cstddef>
#include <cstdint>

#include "bsp/rigid_body_integration.hpp"
#include "bsp/world_ocean.hpp"

// Every channel through which anything outside the Dyn library writes the ship hull
// body's motion state, enumerated from the complete xref sets of the six mutators.
//
// docs/UNIT_FORCE_CHANNEL.md carries the addresses, the original ABI and the uncertainty.
// Everything here is a semantic C++ interface for MSVC Win32, not a drop-in binary
// replacement, and every descriptive name is a hypothesis rather than a recovered symbol.
//
// docs/RIGID_BODY_INTEGRATION.md proposed this packet as "the list through which engine
// thrust, damage and explosions push force onto a unit ... `unit+10D4h`". That premise is
// wrong twice over and this header records the correction:
//
//   * `unit+10D4h` is the leak (flooding) model, not a force list. Its layout and both
//     halves of its tick are already reconstructed in bsp/unit_forces.hpp;
//     docs/UNIT_FORCE_COMMANDS.md carries the record table. 0074F2E0 turns the water
//     weights into a heeling torque that 009329C0 stages, and nothing else pushes into it.
//   * There is no damage or explosion force path onto the hull body at all. Dyn's
//     AddForce 00C35360 has exactly one caller in the whole image (00933B01, inside the
//     hydrodynamic model) and AddTorque 00C35330 exactly three, none of which is a damage
//     or explosion routine.
//
// The one genuinely external channel is a unit message, kind 93h, carrying a world-space
// torque. Its handler is the only site outside the controller family that touches the
// body at all.

namespace bsp {

// What a site writes. The names are the mutators' own established contracts from
// docs/RIGID_BODY_INTEGRATION.md.
enum class DynBodyWriteKind {
    kAddForce,            // 00C35360, M+38h += f, then B+50h &= ~12h
    kAddTorque,           // 00C35330, M+44h += t, then B+50h &= ~12h
    kSetLinearVelocity,   // 00C37E50, M+00h = v, then B+50h &= ~12h
    kSetAngularVelocity,  // 00C37E20, M+0Ch = w, then B+50h &= ~12h
    kSetForce,            // 00C32050, M+38h = f
    kSetInertia,          // 00C37E70, M+54h..+5Ch = 1/I
};

// One call site. `address` is the call instruction, `containing` the Ghidra function whose
// body holds it, `native` the mutator it calls.
struct UnitForceChannelSite {
    std::uint32_t address{0};
    std::uint32_t containing{0};
    std::uint32_t native{0};
    DynBodyWriteKind kind{DynBodyWriteKind::kAddForce};
    const char* note{""};
};

// The complete set, from `ghidra xrefs` on each mutator. Only the sites that reach a ship
// controller's hull body are listed; 00447A34 (BSP_GameDynamicsList_Add) and the buoyancy
// list's 00C32050 site write the debris bodies of docs/GAME_DYNAMICS_LIST.md instead, and
// are marked as such rather than left out.
inline constexpr UnitForceChannelSite kUnitHullBodyWriteSites[] = {
    {0x00933B01u, 0x009329C0u, 0x00C35360u, DynBodyWriteKind::kAddForce,
     "the hydrodynamic model's summed force; the only AddForce site in the image"},
    {0x00933B38u, 0x009329C0u, 0x00C35330u, DynBodyWriteKind::kAddTorque,
     "the hydrodynamic model's summed torque, which carries the leak heeling term"},
    {0x00937613u, 0x00937440u, 0x00C35330u, DynBodyWriteKind::kAddTorque,
     "the ship override's rudder torque"},
    {0x0092BF33u, 0x0092BF30u, 0x00C35330u, DynBodyWriteKind::kAddTorque,
     "the controller's AddTorque helper, reached only from unit message 93h"},
    {0x009329ECu, 0x009329C0u, 0x00C37E50u, DynBodyWriteKind::kSetLinearVelocity,
     "the disabled-controller path, which zeroes both velocities"},
    {0x00932A0Eu, 0x009329C0u, 0x00C37E20u, DynBodyWriteKind::kSetAngularVelocity,
     "the same disabled-controller path"},
    {0x0092D588u, 0x0092D300u, 0x00C37E50u, DynBodyWriteKind::kSetLinearVelocity,
     "the throttle half of the motion tick, an acceleration-limited approach"},
    {0x0092D84Du, 0x0092D770u, 0x00C37E50u, DynBodyWriteKind::kSetLinearVelocity,
     "the unconditional axial-speed setter"},
    {0x0092EB8Fu, 0x0092E8C0u, 0x00C37E20u, DynBodyWriteKind::kSetAngularVelocity,
     "the rudder half of the motion tick"},
    {0x0092E812u, 0x0092E5B0u, 0x00C37E50u, DynBodyWriteKind::kSetLinearVelocity,
     "a second controller routine, not read by this packet"},
    {0x0092E895u, 0x0092E5B0u, 0x00C37E20u, DynBodyWriteKind::kSetAngularVelocity,
     "the same routine"},
    {0x0093739Au, 0x00936DC0u, 0x00C37E50u, DynBodyWriteKind::kSetLinearVelocity,
     "the depth-holding override"},
    {0x009373A7u, 0x00936DC0u, 0x00C37E20u, DynBodyWriteKind::kSetAngularVelocity,
     "the depth-holding override"},
    {0x00937675u, 0x00937630u, 0x00C37E50u, DynBodyWriteKind::kSetLinearVelocity,
     "the frozen override, which zeroes both velocities"},
    {0x00937697u, 0x00937630u, 0x00C37E20u, DynBodyWriteKind::kSetAngularVelocity,
     "the frozen override"},
    {0x00939C05u, 0x00937C90u, 0x00C37E70u, DynBodyWriteKind::kSetInertia,
     "the hull body's own creation; docs/SHIP_HULL_BODY.md"},
};

inline constexpr std::size_t kUnitHullBodyWriteSiteCount =
    sizeof(kUnitHullBodyWriteSites) / sizeof(kUnitHullBodyWriteSites[0]);

// ---------------------------------------------------------------------------
// The external channel: unit message 93h
// ---------------------------------------------------------------------------

// The `switch (msg+10h)` in the unit message handler 00821E80 selects on a byte. Kind 93h
// is the only case that reaches the hull body. 91h and 92h reach the leak model at
// unit+10D4h instead (008221F3 and 0082221B), which is why the two were once read as one
// force channel.
inline constexpr std::uint8_t kUnitMessageAddHullTorque = 0x93;

// The payload the handler reads at 00822235..0082223B: three floats at msg+1Ch, +20h,
// +24h. The producer is 0080FFD0, which writes exactly those three slots from its second,
// third and fourth arguments, so the layout is settled by the producer and not by the
// handler's use of it.
struct UnitHullTorqueMessage {
    OceanVec3 torque{};
};

// 0092BF30, a two-instruction helper: `MOV ECX,[ECX+2Ch]` then `JMP 00C35330`. Its ECX is
// a controller, so it is the controller's own AddTorque on the hull body at
// controller+2Ch. Its only caller is the message handler.
void unit_controller_add_torque_0092bf30(DynBody& body, const OceanVec3& torque) noexcept;

// 00821E80 case 93h, 00822235..00822255: copy the three payload floats into a local vec3,
// load the controller from unit+1018h and call the helper. Returns 1, like every handled
// case.
bool unit_handle_add_hull_torque_00822235(const UnitHullTorqueMessage& message,
                                          DynBody& controller_body) noexcept;

}  // namespace bsp
