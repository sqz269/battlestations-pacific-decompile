// The plane squadron's formation: the wing member's station point.
//
// Addresses: 007F23A0, 007ED260, 009BFD70, 009BED80, 009C1FD0, 007D01C3,
// 007D68E4, 007F50CF, 00D049A8, 00D7A208.
//
// Packet cc8_formation. Every descriptive name here is a hypothesis, not a
// recovered symbol. docs/PLANE_FORMATION.md carries the evidence per line.
//
// This is NOT the ship unit group of bsp/ship_ai_formation.hpp. The two are
// separate machines and the plane never enters the ship one: the only two call
// sites that can enrol an entity through 0077C8D0 are both gated on the class
// family 6 (0094870E under `class->vtable[18h](6)` at 009486CB, and 00A10E67
// under `vtable[5Ch](6)`), and a plane's class chain has no 6 in it. A plane
// squadron's wing is spread by the routine below instead.
//
// The three fields the machine runs on, all read from the listing:
//   * plane+9D0h     the member's formation index. The property-bag pair at
//                    007D68E4/007D690D names it `formationIndex` (00D05DC4).
//                    BSP_PlaneUnitInstance_Construct zeroes it at 007D01C3 and
//                    the attach 007F4B43 never writes it, so every member of a
//                    freshly spawned squadron carries 0 until 007ED260 runs.
//   * plane+9D8h     the member's array slot, stamped by 007F4B43 and rewritten
//                    by 007ED260 at 007ED292.
//   * squadron+3E4h  the formation SHAPE. The property-bag pair at
//                    007F50CF/007F50FB names it `psFormation` (00D08AA0);
//                    007F2C60 seeds it to 1. 007F2556-007F256F is its only
//                    reader: `MOV EAX,[ESI+3E4h] / ADD EAX,-1 / CMP EAX,4 /
//                    JA default / JMP [EAX*4+007F2900]`, a five-entry jump
//                    table, so the shapes are 1 to 5.
//   * squadron+3E8h  the morale float (docs/PLANE_SQUADRON.md), seeded 1.0f by
//                    007F2C60 and republished every follow tick by 009BFDA6-
//                    009BFDB2. It scales the whole station offset.
#pragma once

#include <cstdint>

#include "bsp/camera_affine.hpp"

namespace bsp {

// The four native offsets above, for anything that has to name them.
inline constexpr std::uint32_t kPlaneFormationIndexOffset = 0x9D0u;
inline constexpr std::uint32_t kPlaneFormationSpawnStampOffset = 0x9D8u;
inline constexpr std::uint32_t kPlaneSquadronFormationShapeOffset = 0x3E4u;
inline constexpr std::uint32_t kPlaneSquadronMoraleOffset = 0x3E8u;

// 007F2C60 seeds squadron+3E4h to 1 and squadron+3E8h to 1.0f
// (docs/PLANE_SQUADRON.md lines 98 and 262).
inline constexpr int kPlaneFormationDefaultShape = 1;
inline constexpr float kPlaneFormationDefaultMorale = 1.0f;

// The member array is five slots (+3D0h..+3E0h), so an index never exceeds 4
// and 007ED260's taken table is exactly five entries wide.
inline constexpr int kPlaneFormationMaxMembers = 5;

// 007F2520 `FMUL double ptr [00D049A8]`, read at the width of the loading
// instruction: a double, 1.7999999523162842. It multiplies the leader class's
// float at +A4h to give a floor under the lateral spacing.
inline constexpr double kPlaneFormationWidthSpacingMul = 1.7999999523162842;

// 007F285F/007F2882/007F28AB `MOVSS XMM0,[00D7A208] / SUBSS XMM0,x`, and
// 00D7A208 is -0.0f, so every mirror is a plain negation.
inline constexpr float kPlaneFormationMirrorBase = -0.0f;

// ---------------------------------------------------------------------------
// 007ED260, the index assignment
// ---------------------------------------------------------------------------
// `__thiscall(squadron)`, `RET`, body 007ED260-007ED374, read complete.
//
// It walks the `+3CCh` live members of `+3D0h` twice. The first walk stamps
// `member+9D8h = i` (007ED292) and marks the member's CURRENT `+9D0h` in a
// five-entry table (007ED2AC `MOV [ESP+ESI*4+0Ch],1`). The second walk gives
// slot 0 the index 0 (007ED2D2) and then, per member, takes the lowest free
// ODD index from 1 and the lowest free EVEN index from 2 and keeps the smaller,
// which is what puts a pair on either side of the leader.
//
// `formation_index` and `spawn_stamp` are the two per-member arrays, in `+3D0h`
// array order; both are written. `count` is `+3CCh` and is clamped to the five
// slots the array holds.
void plane_formation_assign_indices_007ed260(std::int32_t* formation_index,
                                             std::int32_t* spawn_stamp,
                                             int count) noexcept;

// ---------------------------------------------------------------------------
// 007F23A0, the station point
// ---------------------------------------------------------------------------
// `__thiscall(squadron)(int formationIndex, float out[5])`, `RET 8`, body
// 007F23A0-007F28FF. Its two callers are 007F2920 and 009BFD70; 009BFD70 is the
// proof of the argument, pushing `[[task+4]+4]+9D0h` at 009BFDBE-009BFDCC with
// ECX = `[[task+4]+0Ch]`, the squadron.
struct PlaneFormationStationInputs {
    // plane+9D0h. The odd/even bit decides the side: 007F2853
    // `TEST byte ptr [ESP+0C4h],1`, and [ESP+0C4h] is the first stack argument.
    int formation_index{0};
    // squadron+3E4h, 1 to 5. Only shape 1 is bound; see `produced` below.
    int shape{kPlaneFormationDefaultShape};
    // squadron+3E8h.
    float morale{kPlaneFormationDefaultMorale};
    // The tuning triple: `Pilot/Follow/BomberDisplacement` (singleton+3DCh,
    // 007F2465) when the LEADER answers `vtable[5Ch](10h)` or `vtable[5Ch](14h)`
    // - MPlaneBomber/LevelBomber, or the ReconPlane family - and
    // `Pilot/Follow/SmallPlaneDisplacement` (singleton+3D0h, 007F240C)
    // otherwise. Branch senses taken from the bytes: `84 c0 75 6c` at 007F23F0
    // and `84 c0 75 59` at 007F2403, both JNZ into the bomber arm at 007F2460.
    // A TorpedoBomber (11h) and a DiveBomber (12h) are NOT in either family, so
    // they take the small-plane triple; the class chains are
    // docs/ATTACK_GATE_TAILS.md.
    float displacement[3]{0.0f, 0.0f, 0.0f};
    // `[[squadron+35Ch]+A4h]`, 007F250E-007F2514. squadron+35Ch is the plane
    // class descriptor (docs/PLANE_SQUADRON.md line 77) and class+A4h is the
    // authored Lua `Width`, which 00960368 stores and this repository already
    // carries as `class_width_00a4`. The term only ever raises the lateral
    // spacing, and only when `width * 1.8` exceeds `displacement[0]`.
    float leader_class_width{0.0f};
    // The game tuning singleton's two booleans, +3E8h and +3E9h, re-fetched in
    // the tail at 007F287A and 007F28A2. Both gates SKIP their mirror when the
    // byte is non-zero.
    bool symmetrical_position{true};
    bool symmetrical_altitude{false};
    // The frame the local offset is transformed by, 004142E0 at 007F28CD.
    // Small-plane arm: the leader's whole world matrix, leader+0CCh, copied at
    // 007F2456. Bomber arm: a pure yaw rotation built by 00B646E0 at 007F24AB
    // from `leader->vtable[50h]()`, with the leader's world position written
    // into its translation row at 007F24E4-007F2508.
    bsp::CameraMatrix leader_frame{};
};

struct PlaneFormationStation {
    // False when `shape` is one of the four cases this packet did not read, or
    // when it is outside 1..5. 007F2569 `JA 007F2853` takes an out-of-range
    // shape to the tail with the three components still at whatever the frame
    // held, which this process refuses to reproduce rather than guess.
    bool produced{false};
    // The offset in the leader's frame, before 004142E0.
    float local[3]{0.0f, 0.0f, 0.0f};
    // The station in world space.
    float world[3]{0.0f, 0.0f, 0.0f};
};

PlaneFormationStation plane_formation_station_007f23a0(
    const PlaneFormationStationInputs& in) noexcept;

// The displacement selector of 007F23F0/007F2403, as a rule on the leader's
// class chain. `level_bomber` is `vtable[5Ch](10h)` and `recon_family` is
// `vtable[5Ch](14h)`, which 15h SmallReconPlane and 16h LargeReconPlane also
// answer because 14h is in their chains.
inline bool plane_formation_uses_bomber_displacement(bool leader_is_level_bomber,
                                                     bool leader_is_recon_family) noexcept {
    return leader_is_level_bomber || leader_is_recon_family;
}

}  // namespace bsp
