// Packet cc7_gun_mount_positions. See docs/GUN_MOUNT_POSITIONS.md.
//
// Pure projections of the mount-point tail of BSP_Gun_Fire 00730160
// (00730762..00730A0E) and of the two class-side helpers it depends on. No
// host, no globals, no scene graph.
#include "bsp/gun_mount_positions.hpp"

#include "bsp/camera_affine.hpp"  // transform_point_004142e0

namespace bsp {

// 0072EE65..0072EE8F. The store at 0072EE6F writes null first, so "no node" is
// the default and every later branch only overwrites it.
GunMountNode gun_mount_node_pick_0072ee65(bool have_barrel, bool have_base,
    bool have_root) noexcept {
    if (have_barrel) return GunMountNode::kBarrel;  // 0072EE6D/0072EE75
    if (have_base) return GunMountNode::kBase;      // 0072EE7D/0072EE7F
    if (have_root) return GunMountNode::kRoot;      // 0072EE87/0072EE89
    return GunMountNode::kNone;
}

// 0072AB80 (and the identical 006FDD70). The early return at 0072ABA5 is the
// "begin pointer is null" case and yields 1; the tail at 0072ABAC recomputes
// the same quotient and returns it unclamped, so a non-empty list is its own
// size. The divide is the 0x2AAAAAAB magic with SAR EDX,1, which is /0Ch, not
// the /18h docs/GUN_DISPERSION.md records for 006FDD70.
int gun_muzzle_count_0072ab80(std::size_t offset_count) noexcept {
    if (offset_count == 0) return 1;
    return static_cast<int>(offset_count);
}

// 007309ED..00730A0E: EAX = gun+44Ch + 1; CDQ; IDIV gun+448h; gun+44Ch = EDX.
int gun_advance_barrel_index_007309ed(int barrel_index, int muzzle_count) noexcept {
    if (muzzle_count <= 0) return 0;  // the native IDIV's undefined case
    return (barrel_index + 1) % muzzle_count;
}

// 00730799..007307A9 for the list case, 00730899..007308B2 for the empty one.
GunMuzzleOffset gun_muzzle_local_offset_00730799(const GunMuzzleMount& mount) noexcept {
    if (mount.muzzle_offsets.empty()) return GunMuzzleOffset{0.0f, 0.0f, 0.0f};
    const std::size_t count = mount.muzzle_offsets.size();
    std::size_t index = 0;
    if (mount.barrel_index > 0) {
        index = static_cast<std::size_t>(mount.barrel_index);
        // 004181A0 range-checks and traps; the fire path cannot reach the trap
        // because gun+44Ch is kept modulo gun+448h, which is this same count.
        if (index >= count) index %= count;
    }
    return mount.muzzle_offsets[index];
}

// 00730762..007307FC. The guards at 00730776 (begin == null) and 00730793
// (count <= 0) are what route a class with no "fire" nodes to the fallback.
std::array<float, 3> gun_muzzle_world_position_00730762(const GunMuzzleMount& mount) {
    if (mount.muzzle_offsets.empty()) {
        // 007308C1/007308D6/007308ED read node+120h..+128h, which is the
        // translation row of the same matrix at node+0F0h.
        return std::array<float, 3>{
            mount.node_world[kNodeMatrixRowTranslation + 0],
            mount.node_world[kNodeMatrixRowTranslation + 1],
            mount.node_world[kNodeMatrixRowTranslation + 2]};
    }
    const GunMuzzleOffset local = gun_muzzle_local_offset_00730799(mount);
    std::array<float, 3> world{};
    transform_point_004142e0(local, mount.node_world, world);  // 007307D3
    return world;
}

// 0073022A/0073023C/00730249 read node matrix +20h, +24h, +28h. The three
// stores that follow land on the same contiguous triple once the PUSH EBX at
// 00730242 and the PUSH 24h at 0073024F are unwound.
std::array<float, 3> gun_muzzle_direction_0073022a(
    const CameraMatrix& node_world) noexcept {
    return std::array<float, 3>{node_world[kNodeMatrixRowForward + 0],
        node_world[kNodeMatrixRowForward + 1],
        node_world[kNodeMatrixRowForward + 2]};
}

}  // namespace bsp
