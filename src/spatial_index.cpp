// The spatial index's cell key, its bucket rules and the per-step re-bucket.
// docs/SPATIAL_INDEX.md carries the evidence; every rule below is a projection
// of one listing range, named in the comment above it.
#include "bsp/spatial_index.hpp"

#include <array>
#include <cmath>

namespace bsp {
namespace {

// The x-major, z-minor walk both 0098A310 and 0098A3D0 perform. Returning the
// slot the way the listing counts it is what lets the unregister find each link
// from the key alone.
template <typename Visit>
void walk_cell_rect(const SpatialCellRect& rect, Visit visit) {
    int slot = 0;
    // JG 0x0098a3ab at 0098A333: an inverted x range links nothing at all.
    for (int x = rect.min.x; x <= rect.max.x; ++x) {
        // JG 0x0098a398 at 0098A346: an inverted z range skips the row.
        for (int z = rect.min.z; z <= rect.max.z; ++z) {
            visit(slot, x, z);
            ++slot;
        }
    }
}

} // namespace

// 0098BD01..0098BD1A, and the identical sequence at 0098A3AE..0098A3C4. Three
// SHL 8 with an ADD between, evaluated most significant cell first. Unsigned
// arithmetic here reproduces the hardware ADD, including the borrow an
// out-of-grid negative cell index pushes into the next byte lane.
std::uint32_t spatial_cell_key_0098bd01(const SpatialCellRect& rect) noexcept {
    std::uint32_t key = static_cast<std::uint32_t>(rect.max.z);
    key = (key << 8) + static_cast<std::uint32_t>(rect.max.x);
    key = (key << 8) + static_cast<std::uint32_t>(rect.min.z);
    key = (key << 8) + static_cast<std::uint32_t>(rect.min.x);
    return key;
}

// 0098A3D3..0098A3E9. Four zero-extended bytes, so this inverts the packing
// only while every cell index is inside [0, 0FFh].
SpatialCellRect spatial_cell_key_unpack_0098a3d3(std::uint32_t key) noexcept {
    SpatialCellRect rect{};
    rect.min.x = static_cast<int>(key & 0xffu);
    rect.min.z = static_cast<int>((key >> 8) & 0xffu);
    rect.max.x = static_cast<int>((key >> 16) & 0xffu);
    rect.max.z = static_cast<int>(key >> 24);
    return rect;
}

// 0098A356 (LEA ESI,[ESI+ECX*4+0x84]) and 0098A44A. No clamp and no range test.
int spatial_cell_head_index_0098a356(int cell_x, int cell_z) noexcept {
    return cell_x * kSpatialGridDim + cell_z;
}

int spatial_cell_slot_count_0098a310(const SpatialCellRect& rect) noexcept {
    int count = 0;
    walk_cell_rect(rect, [&count](int, int, int) { ++count; });
    return count;
}

bool spatial_cell_of_slot_0098a310(const SpatialCellRect& rect, int slot,
                                   HitQueryCell& cell) noexcept {
    bool found = false;
    walk_cell_rect(rect, [&](int walked, int x, int z) {
        if (walked == slot) {
            cell.x = x;
            cell.z = z;
            found = true;
        }
    });
    return found;
}

// 0098BAD2..0098BAE8: CMP against EBX = 1 on both spans, JG to the loose push.
SpatialPlacement spatial_placement_0098bad2(const SpatialCellRect& rect) noexcept {
    if (rect.max.x - rect.min.x > 1 || rect.max.z - rect.min.z > 1) {
        return SpatialPlacement::kLooseArray;
    }
    return SpatialPlacement::kGridCells;
}

// 0098BCE3..0098BCFC and 0098BAB4..0098BACD: the min corner then the max
// corner, each through 0098AD60, neither clamped.
SpatialCellRect spatial_cell_rect_of_bounds(const HitQueryBounds& bounds,
                                            float cell_size) noexcept {
    SpatialCellRect rect{};
    rect.min = cell_of_point_0098ad60(bounds.min, cell_size);
    rect.max = cell_of_point_0098ad60(bounds.max, cell_size);
    return rect;
}

// 00722C20. The native clears the sign bit of the spilled float
// (AND 0x7fffffff at 00722C36, 00722C5B, 00722C7D) rather than calling an abs
// helper; std::fabs is the same value for every input the routine can see.
void accumulate_abs_scaled_00722c20(HitQueryPoint& acc, const HitQueryPoint& v,
                                    float scale) noexcept {
    acc.x += std::fabs(v.x) * scale;
    acc.y += std::fabs(v.y) * scale;
    acc.z += std::fabs(v.z) * scale;
}

// 0098A750, the body after the frame-stamp gate: three accumulate calls over
// the matrix rows, the centre through 004142E0, then min = centre - extent and
// max = centre + extent one axis at a time (0098A868..0098A8CD).
HitQueryBounds spatial_world_bounds_0098a750(const SpatialLocalBox& local,
                                             const CameraMatrix& node_world_matrix) {
    HitQueryPoint extent{0.0f, 0.0f, 0.0f};
    const HitQueryPoint row0{node_world_matrix[0], node_world_matrix[1], node_world_matrix[2]};
    const HitQueryPoint row1{node_world_matrix[4], node_world_matrix[5], node_world_matrix[6]};
    const HitQueryPoint row2{node_world_matrix[8], node_world_matrix[9], node_world_matrix[10]};
    accumulate_abs_scaled_00722c20(extent, row0, local.extent.x);
    accumulate_abs_scaled_00722c20(extent, row1, local.extent.y);
    accumulate_abs_scaled_00722c20(extent, row2, local.extent.z);

    const std::array<float, 3> centre_local{local.centre.x, local.centre.y, local.centre.z};
    std::array<float, 3> centre_world{};
    transform_point_004142e0(centre_local, node_world_matrix, centre_world);

    HitQueryBounds bounds{};
    bounds.min.x = centre_world[0] - extent.x;
    bounds.min.y = centre_world[1] - extent.y;
    bounds.min.z = centre_world[2] - extent.z;
    bounds.max.x = centre_world[0] + extent.x;
    bounds.max.y = centre_world[1] + extent.y;
    bounds.max.z = centre_world[2] + extent.z;
    return bounds;
}

// 0098A9EF..0098A9E9: each corner is scaled by the 0.5 at 00D7A280 before the
// subtraction, which is not the same rounding as 0.5f * (max - min).
HitQueryPoint spatial_local_extent_0098a920(const HitQueryBounds& local) noexcept {
    const float half = 0.5f;
    HitQueryPoint extent{};
    extent.x = local.max.x * half - local.min.x * half;
    extent.y = local.max.y * half - local.min.y * half;
    extent.z = local.max.z * half - local.min.z * half;
    return extent;
}

// 0098A310. Head insertion into every cell of the rectangle, then the slot
// count at node+40h and the packed key at node+3Ch. The listing writes the
// link's prev twice (0098A365 and 0098A375); once is the same state.
void spatial_register_node_0098a310(SpatialGridHost& host, void* node,
                                    const SpatialCellRect& rect) noexcept {
    int linked = 0;
    walk_cell_rect(rect, [&](int slot, int x, int z) {
        void* link = host.node_link(node, slot);
        const int cell = spatial_cell_head_index_0098a356(x, z);
        void* head = host.cell_head(cell);
        host.set_link_next(link, head);
        host.set_link_prev(link, nullptr);
        if (head != nullptr) {
            host.set_link_prev(head, link);
        }
        host.set_cell_head(cell, link);
        linked = slot + 1;
    });
    host.set_node_cell_count(node, linked);
    host.set_node_cell_key(node, spatial_cell_key_0098bd01(rect));
}

// 0098A3D0. Phase one repairs the neighbours of every one of the node+40h
// links, phase two walks the rectangle the key names and replaces any cell head
// that still points at one of them. A link that was a head has a null prev, so
// only phase two can unhook it.
void spatial_unregister_node_0098a3d0(SpatialGridHost& host, void* node) noexcept {
    const SpatialCellRect rect = spatial_cell_key_unpack_0098a3d3(host.node_cell_key(node));
    const int count = host.node_cell_count(node);
    for (int slot = 0; slot < count; ++slot) {
        void* link = host.node_link(node, slot);
        void* prev = host.link_prev(link);
        void* next = host.link_next(link);
        if (prev != nullptr) {
            host.set_link_next(prev, next);
        }
        if (next != nullptr) {
            host.set_link_prev(next, prev);
        }
    }
    walk_cell_rect(rect, [&](int slot, int x, int z) {
        void* link = host.node_link(node, slot);
        const int cell = spatial_cell_head_index_0098a356(x, z);
        if (host.cell_head(cell) == link) {
            host.set_cell_head(cell, host.link_next(link));
        }
    });
    // 0098A4AA and 0098A4B6: both exits clear the count. node+3Ch keeps the old
    // key, which no reader consults while the count is zero.
    host.set_node_cell_count(node, 0);
}

// 0098BC70.
void spatial_refresh_node_0098bc70(SpatialRefreshHost& host, void* node) {
    void* pose = host.node_owner_pose(node);
    if (!host.pose_world_valid(pose)) {
        host.pose_refresh_world_00414db0(pose);
    }
    host.node_copy_world_matrix_004134f0(node, pose);

    // 0098BC99 reloads node+4Ch before the inverse block.
    pose = host.node_owner_pose(node);
    if (!host.pose_inverse_valid(pose)) {
        host.pose_refresh_world_00414db0(pose);
        host.pose_build_inverse_00b63d50(pose);
    }
    host.node_copy_inverse_matrix_004134f0(node, pose);

    host.node_rebuild_world_bounds_0098a750(node);

    // 0098BCDD: only a node the grid holds is re-bucketed. A child (node+108h
    // set) and a loose node both keep a zero count and stop here.
    if (host.node_cell_count(node) != 0) {
        const SpatialCellRect rect = host.node_cell_rect_0098ad60(node);
        const std::uint32_t key = spatial_cell_key_0098bd01(rect);
        if (key != host.node_cell_key(node)) {
            host.unregister_node_0098a3d0(node);
            host.register_node_0098a310(node, rect);
        }
    }

    // 0098BD45: the array and the count are read once, then every child is
    // refreshed whatever its own static flag says.
    const int children = host.node_child_count(node);
    for (int slot = 0; slot < children; ++slot) {
        spatial_refresh_node_0098bc70(host, host.node_child(node, slot));
    }
}

// 0098BDB0. The float is consumed by the RET 4 and never read.
void spatial_refresh_moved_nodes_0098bdb0(SpatialRefreshHost& host, float step) {
    (void)step;
    void* node = host.first_root_node();
    while (node != nullptr) {
        if (!host.node_is_static(node)) {
            spatial_refresh_node_0098bc70(host, node);
        }
        // 0098BDCD reads the next pointer after the callee returns.
        node = host.next_root_node(node);
    }
}

} // namespace bsp
