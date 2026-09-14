#pragma once
// Where a gun's shot actually starts: the mount point.
//
// Addresses: 00730160, 0072F830, 0072E6D0, 007325A0, 0072AB80, 006FDD70,
// 006E3DC0, 006FE160, 004181A0, 004142E0, 004134F0, 00B6DB70.
// docs/GUN_MOUNT_POSITIONS.md carries the derivations.
//
// Every rule here is pure. The caller supplies the resolved node world matrix
// and the class's muzzle-offset list explicitly; nothing here reaches a host, a
// global, a scene graph or a resource. The model-node system that produces the
// world matrix is a contract, not a dependency: see the header comment on
// GunMuzzleMount.
#include <array>
#include <cstddef>
#include <vector>

#include "bsp/camera_projection.hpp"  // CameraMatrix = std::array<float,16>

namespace bsp {

// ---------------------------------------------------------------------------
// Gun instance fields this packet established. gun+44Ch (the barrel index) is
// already declared as kGunArcOffBarrelIndex in bsp/gun_platform_arc.hpp and is
// deliberately not repeated here.
// ---------------------------------------------------------------------------
inline constexpr std::size_t kGunMountOffRootNode = 0x3bc;      // 0072E98A
inline constexpr std::size_t kGunMountOffBaseNode = 0x3c4;      // 0072E9F0, node "base"
inline constexpr std::size_t kGunMountOffBarrelNode = 0x3c8;    // 0072EA1C, node "barrel"
inline constexpr std::size_t kGunMountOffMuzzleNode = 0x3cc;    // 0072EE6F/0072EE8B, the pick
inline constexpr std::size_t kGunMountOffClassDescriptor = 0x3f4;
inline constexpr std::size_t kGunMountOffMuzzleCount = 0x448;   // 0072E71A, max(1, n)

// Weapon class descriptor fields (the object at gun+3F4h).
inline constexpr std::size_t kGunClassOffMuzzleOffsetVector = 0x98;  // header
inline constexpr std::size_t kGunClassOffMuzzleOffsetBegin = 0x9c;   // 0072AB81
inline constexpr std::size_t kGunClassOffMuzzleOffsetEnd = 0xa0;     // 0072AB8B
inline constexpr std::size_t kGunClassMuzzleOffsetStride = 0x0c;     // 004181D0

// Transform-node fields read by the gun side. These belong to the model/scene
// packet; they are quoted here only because the gun reads them directly.
inline constexpr std::size_t kTransformNodeOffFlags = 0x5c;         // 007301C3
inline constexpr std::uint8_t kTransformNodeWorldValidBit = 0x02;   // 007301C3
inline constexpr std::size_t kTransformNodeOffWorldMatrix = 0xf0;   // 007301D1
inline constexpr std::size_t kTransformNodeOffWorldTranslation = 0x120;  // 007308C1

// Row indices into the node's world matrix as the gun path uses them. The
// matrix is the 16-float row-major layout bsp::CameraMatrix already models.
inline constexpr std::size_t kNodeMatrixRowRight = 0;   // floats 0..2, 0073022A's +00h
inline constexpr std::size_t kNodeMatrixRowUp = 4;      // floats 4..6
inline constexpr std::size_t kNodeMatrixRowForward = 8; // floats 8..10, the shot direction
inline constexpr std::size_t kNodeMatrixRowTranslation = 12;  // floats 12..14

// ---------------------------------------------------------------------------
// The node pick, 0072EE65..0072EE8F inside BSP_Gun_SetupFromDescriptor.
// gun+3CCh is the first non-null of gun+3C8h ("barrel"), gun+3C4h ("base") and
// gun+3BCh (the model's own first node). All three are looked up once at setup.
// ---------------------------------------------------------------------------
enum class GunMountNode {
    kNone,    // all three null; the gun has no transform to fire from
    kBarrel,  // gun+3C8h, the model node named "barrel" (00CF70C4)
    kBase,    // gun+3C4h, the model node named "base"   (00CFACD8)
    kRoot,    // gun+3BCh, [[gun+360h]+160h]+0Ch
};

// `have_*` are "the lookup returned a node", not "the node is loaded".
GunMountNode gun_mount_node_pick_0072ee65(bool have_barrel, bool have_base,
    bool have_root) noexcept;

// ---------------------------------------------------------------------------
// The muzzle-offset list, weapon class descriptor +98h.
// A std::vector<float[3]> of mount points in the node's local space, copied at
// class load by 007325A0 from the model's "fire" node group. The vector is the
// only per-barrel geometry the gun has; nothing else spreads a battery.
// ---------------------------------------------------------------------------
using GunMuzzleOffset = std::array<float, 3>;

// 0072AB80 and 006FDD70: (end - begin) / 0Ch, floored to 1 when the list is
// empty. gun+448h is seeded from this at 0072E71A, and the fire path takes the
// barrel index modulo it.
int gun_muzzle_count_0072ab80(std::size_t offset_count) noexcept;

// 007309ED..00730A0E: nextBarrel = (nextBarrel + 1) % gun+448h, stored after
// the shot. A zero or negative count is the native IDIV's undefined case and is
// reported here as index 0 rather than trapping.
int gun_advance_barrel_index_007309ed(int barrel_index, int muzzle_count) noexcept;

// ---------------------------------------------------------------------------
// The mount.
//
// `node_world` is the world matrix of the picked node, already refreshed. The
// native path reaches it as gun[+3CCh]+0F0h after 00B6DB70 has cleared the
// dirty bit (007301C3), and optionally pre-multiplies it by a matrix the object
// at gun+3Ch supplies through its vtable slot 94h (00730217, 00413920,
// 004134F0) when slot 8Ch answers true. Both the refresh and the slot-94h
// override are **contracts of the model/scene packet**; this API takes the
// finished matrix.
// ---------------------------------------------------------------------------
struct GunMuzzleMount {
    CameraMatrix node_world{};                // gun[+3CCh]+0F0h, post-refresh
    std::vector<GunMuzzleOffset> muzzle_offsets{};  // class+9Ch..+0A0h
    int barrel_index{0};                      // gun+44Ch
};

// 00730799..007307A9 and 007308C1: the local-space mount point for this shot.
// With a non-empty list it is offsets[barrel_index] verbatim (004181A0 indexes
// with a 0Ch stride); with an empty list the native fallback branch at 00730899
// zeroes the triple and fires from the node origin.
GunMuzzleOffset gun_muzzle_local_offset_00730799(const GunMuzzleMount& mount) noexcept;

// 00730762..007307FC: the world mount point, argument 1 of
// BSP_Gun_SpawnShotAndEffects.
//   list non-empty: TransformAffinePoint(offsets[barrel_index], node_world)
//   list empty:     node_world's translation row, node+120h verbatim
// This is the whole of "where the gun fires from". No class Height, no hull
// centre and no vertical raise appears anywhere on the path.
std::array<float, 3> gun_muzzle_world_position_00730762(
    const GunMuzzleMount& mount);

// 0073022A..00730253: the unperturbed shot direction, argument 3 of the spawn,
// and the `in` of gun vtable slot 1E0h. It is row 2 of the same node matrix.
// It is a direction, not a point: see docs/GUN_MOUNT_POSITIONS.md section 6.
std::array<float, 3> gun_muzzle_direction_0073022a(
    const CameraMatrix& node_world) noexcept;

}  // namespace bsp
