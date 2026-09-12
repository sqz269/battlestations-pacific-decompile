#pragma once
// The spatial index singleton 0042E630 / 0042D450, the four-byte cell key that
// buckets a node into the 150x150 horizontal grid, the register / unregister
// pair 0098A310 / 0098A3D0, and the per-step re-bucket 0098BDB0 / 0098BC70 that
// the fixed step reaches as rows 3 and 4 of bsp/fixed_step_fanout.hpp
// (FixedStepFanoutHost::refresh_moved_spatial_nodes_0098bdb0).
//
// Hypotheses, not recovered symbols. The evidence for every rule is in
// docs/SPATIAL_INDEX.md. The grid mapping itself belongs to
// bsp/hit_narrowphase.hpp (cell_of_point_0098ad60 and the kSpatialGrid*
// constants) and is included rather than redeclared, as are the node offsets
// that the segment query already established.
//
// Nothing here allocates, and no STL or CRT routine is ported: operator new,
// memset and memcpy stay behind the host interfaces as contracts.
#include <cstddef>
#include <cstdint>

#include "bsp/camera_affine.hpp"
#include "bsp/hit_narrowphase.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// The singleton. 0x16018 bytes, published to the global 00F8A0D8 by 0042E630
// and zeroed by 0042D450.
//
// kSpatialGridLooseArrayOffset (+8h), kSpatialGridLooseCountOffset (+80h) and
// kSpatialGridHeadsOffset (+84h) come from bsp/hit_narrowphase.hpp.
// ---------------------------------------------------------------------------

inline constexpr std::size_t kSpatialIndexSize = 0x16018;        // 0042E686
inline constexpr std::size_t kSpatialIndexOffVTable = 0x00;      // 0042D461, 00CE3CEC
inline constexpr std::size_t kSpatialIndexOffUnused4 = 0x04;     // 0042D467, read by nothing
inline constexpr std::size_t kSpatialIndexOffRootList = 0x16014; // 0042D480, 0098BB16
inline constexpr std::size_t kSpatialIndexGridBytes = 0x15f90;   // the memset at 0042D453

// (0x80 - 0x8) / 4. There is no bounds check on the loose push at 0098BB32;
// entry 30 would overwrite the count at +80h.
inline constexpr int kSpatialIndexLooseCapacity = 30;

// 0x96 * 0x96 dword heads. The row stride 0x258 appears at 0098A49A.
inline constexpr int kSpatialGridCellCount = kSpatialGridDim * kSpatialGridDim;
inline constexpr std::size_t kSpatialGridRowStride = 0x258;

// The grid's own layout, for a reader that wants the singleton in one place.
// Sizes, not a binary-compatible declaration: the game's object is raw bytes at
// the offsets above and this program's pointers are not the game's.
struct SpatialIndexLayout {
    const void* vtable{nullptr};                                  // +0h
    std::uint32_t unused4{0};                                     // +4h
    const void* loose[kSpatialIndexLooseCapacity]{};              // +8h
    int loose_count{0};                                           // +80h
    const void* cell_head[kSpatialGridCellCount]{};               // +84h
    const void* root_list_head{nullptr};                          // +16014h
};

// ---------------------------------------------------------------------------
// The node fields this packet establishes. The proxy node's other offsets
// (+4Ch owner pose, +D0h/+F8h shapes, +FCh/+100h children, +13Ch/+148h world
// bounds) are the kCollisionNodeOff* constants of bsp/hit_narrowphase.hpp.
// ---------------------------------------------------------------------------

inline constexpr std::size_t kSpatialNodeOffStaticFlag = 0x08;    // 0098BA22, 0098BDC0
inline constexpr std::size_t kSpatialNodeOffCellLinks = 0x0c;     // 0098A34C
inline constexpr std::size_t kSpatialNodeCellLinkStride = 0x0c;   // ADD EAX,0xc at 0098A381
inline constexpr std::size_t kSpatialNodeOffCellKey = 0x3c;       // 0098A3C4
inline constexpr std::size_t kSpatialNodeOffCellCount = 0x40;     // 0098A3AB
inline constexpr std::size_t kSpatialNodeOffRootPrev = 0x44;      // 0098BAFD
inline constexpr std::size_t kSpatialNodeOffRootNext = 0x48;      // 0098BB06
inline constexpr std::size_t kSpatialNodeOffWorldMatrix = 0x50;   // 0098BC94
inline constexpr std::size_t kSpatialNodeOffInverseMatrix = 0x90; // 0098BCD1
inline constexpr std::size_t kSpatialNodeOffChildCapacity = 0x104; // 0098B929
inline constexpr std::size_t kSpatialNodeOffParent = 0x108;       // 0098B9A9, 0098BAA9
inline constexpr std::size_t kSpatialNodeOffLocalBoundsMin = 0x10c; // 0098A929
inline constexpr std::size_t kSpatialNodeOffLocalBoundsMax = 0x118; // 0098A947
inline constexpr std::size_t kSpatialNodeOffLocalCentre = 0x124;  // 0098A80F
inline constexpr std::size_t kSpatialNodeOffLocalExtent = 0x130;  // 0098A76D
inline constexpr std::size_t kSpatialNodeOffBoundsStamp = 0x154;  // 0098A761
inline constexpr std::size_t kSpatialNodeOffAttached = 0x158;     // 0098BB1D, 0098A505
inline constexpr std::size_t kSpatialNodeOffParentSlot = 0x15c;   // 0098B999

// The pose fields the refresh touches, all on node[+4Ch].
inline constexpr std::size_t kSpatialPoseOffWorldValid = 0xc8;    // 0098BC7A
inline constexpr std::size_t kSpatialPoseOffWorldMatrix = 0xcc;   // 0098BC8A
inline constexpr std::size_t kSpatialPoseOffInverseValid = 0x10c; // 0098BC9C
inline constexpr std::size_t kSpatialPoseOffInverseMatrix = 0x110; // 0098BCB2

// (0x3C - 0x0C) / 0x0C. The register loop is not bounded by it; the 2x2 test at
// 0098BAD2 is what keeps a node inside four links.
inline constexpr int kSpatialNodeCellLinkSlots = 4;

// node + 0Ch + 0Ch*slot, the record a cell head points at. 0098A310 writes prev
// and next; the owner back-pointer is filled in where the node is constructed,
// which this packet did not read, and 0098ADD0 reads it as the node to test.
struct SpatialCellLink {
    const void* prev{nullptr};  // +0h, 0098A365
    const void* next{nullptr};  // +4h, 0098A362, kCollisionCellNodeOffNext
    const void* owner{nullptr}; // +8h, kCollisionCellNodeOffEntity, producer unread
};

// ---------------------------------------------------------------------------
// The cell key. Pure rules; the caller supplies the cells.
// ---------------------------------------------------------------------------

struct SpatialCellRect {
    HitQueryCell min{};
    HitQueryCell max{};
};

// 0098BD01..0098BD1A and 0098A3AE..0098A3C4, which compute the same value.
// Written as the listing computes it: three SHL 8 with a signed ADD between,
// not four ORs. For cells inside [0, 0FFh] the two agree; outside it the adds
// borrow across the byte lanes and spatial_cell_key_unpack_0098a3d3 no longer
// inverts this, which is the whole risk of the missing clamp.
std::uint32_t spatial_cell_key_0098bd01(const SpatialCellRect& rect) noexcept;

// 0098A3D3..0098A3E9: AND 0FFh, MOVZX AH, SHR 10h + AND 0FFh, SHR 18h.
SpatialCellRect spatial_cell_key_unpack_0098a3d3(std::uint32_t key) noexcept;

// 0098A356 and 0098A44A: the head of cell (x, z) is index+84h + (x*96h + z)*4.
// No clamping and no range test; out-of-grid cells index outside the array.
int spatial_cell_head_index_0098a356(int cell_x, int cell_z) noexcept;

// The rectangle the two corner cells span, in the x-major, z-minor order both
// 0098A310 and 0098A3D0 walk. An empty rectangle (min.x > max.x, or a row with
// min.z > max.z) contributes no slots, which is what the JG at 0098A333 and the
// JG at 0098A46A do.
int spatial_cell_slot_count_0098a310(const SpatialCellRect& rect) noexcept;

// The cell a given slot of that walk lands in. Returns false when the slot is
// past the rectangle.
bool spatial_cell_of_slot_0098a310(const SpatialCellRect& rect, int slot,
                                   HitQueryCell& cell) noexcept;

// 0098BAD2..0098BAE8: a span of more than one cell on either axis sends the
// node to the loose array instead of the grid, because the node has only four
// link slots.
enum class SpatialPlacement {
    kGridCells,   // 0098BAEA, the 0098A310 path
    kLooseArray,  // 0098BB2C
};
SpatialPlacement spatial_placement_0098bad2(const SpatialCellRect& rect) noexcept;

// The two corners of a world AABB mapped through cell_of_point_0098ad60, as
// 0098BCE3..0098BCFC and 0098BAB4..0098BACD do it. Unclamped, deliberately.
SpatialCellRect spatial_cell_rect_of_bounds(const HitQueryBounds& bounds,
                                            float cell_size) noexcept;

// ---------------------------------------------------------------------------
// 0098A750: the node's world AABB, rebuilt at most once per frame stamp.
//
// The matrix is the node's +50h world matrix, the CameraMatrix of
// bsp/camera_affine.hpp (rows at 0, 4, 8), and the centre goes through that
// header's transform_point_004142e0, the same routine the listing calls. The
// extent rule is 00722C20 summed over the three rows: |row_i| * extent_i,
// component-wise.
// ---------------------------------------------------------------------------

struct SpatialLocalBox {
    HitQueryPoint centre{}; // node+124h
    HitQueryPoint extent{}; // node+130h, the half extents
};

// 00722C20 itself: acc += |v| * s, the absolute value taken on the float bits.
void accumulate_abs_scaled_00722c20(HitQueryPoint& acc, const HitQueryPoint& v,
                                    float scale) noexcept;

HitQueryBounds spatial_world_bounds_0098a750(const SpatialLocalBox& local,
                                             const CameraMatrix& node_world_matrix);

// 0098A920: extent = 0.5*max - 0.5*min, in that order, with the 0.5 taken from
// the double at 00D7A280. The centre store in the routine's tail is unread.
HitQueryPoint spatial_local_extent_0098a920(const HitQueryBounds& local) noexcept;

// ---------------------------------------------------------------------------
// The cell lists. One virtual per field the two routines touch; the caller owns
// the storage, exactly as the game's node does.
// ---------------------------------------------------------------------------

struct SpatialGridHost {
    virtual ~SpatialGridHost() = default;

    // index+84h + cell*4.
    virtual void* cell_head(int cell) = 0;
    virtual void set_cell_head(int cell, void* link) = 0;

    // node + 0Ch + 0Ch*slot, and the two pointers inside it.
    virtual void* node_link(void* node, int slot) = 0;
    virtual void* link_prev(void* link) = 0;
    virtual void* link_next(void* link) = 0;
    virtual void set_link_prev(void* link, void* value) = 0;
    virtual void set_link_next(void* link, void* value) = 0;

    // node+40h and node+3Ch.
    virtual int node_cell_count(void* node) = 0;
    virtual void set_node_cell_count(void* node, int count) = 0;
    virtual void set_node_cell_key(void* node, std::uint32_t key) = 0;
    virtual std::uint32_t node_cell_key(void* node) = 0;
};

// 0098A310: link the node into every cell of the rectangle, head first, then
// store the slot count and the packed key.
void spatial_register_node_0098a310(SpatialGridHost& host, void* node,
                                    const SpatialCellRect& rect) noexcept;

// 0098A3D0: unpack the stored key, unhook each link from its neighbours, then
// walk the same rectangle and replace any cell head that still points at one of
// them, then clear the count.
void spatial_unregister_node_0098a3d0(SpatialGridHost& host, void* node) noexcept;

// ---------------------------------------------------------------------------
// The per-step re-bucket, one virtual method per native call site.
//
// 0098BDB0 takes a float and never reads it; the signature keeps it so the
// record it replaces (FixedStepFanoutHost::refresh_moved_spatial_nodes_0098bdb0,
// RET 4) still lines up.
// ---------------------------------------------------------------------------

struct SpatialRefreshHost {
    virtual ~SpatialRefreshHost() = default;

    // index+16014h and node+48h, 0098BDB1 and 0098BDCD.
    virtual void* first_root_node() = 0;
    virtual void* next_root_node(void* node) = 0;
    // node+8h, 0098BDC0. Set means this root is static: never refresh it.
    virtual bool node_is_static(void* node) = 0;

    // node+4Ch, and the four matrix calls of 0098BC77..0098BCD1.
    virtual void* node_owner_pose(void* node) = 0;
    virtual bool pose_world_valid(void* pose) = 0;         // pose+C8h
    virtual void pose_refresh_world_00414db0(void* pose) = 0;
    virtual void node_copy_world_matrix_004134f0(void* node, void* pose) = 0;
    virtual bool pose_inverse_valid(void* pose) = 0;       // pose+10Ch
    virtual void pose_build_inverse_00b63d50(void* pose) = 0; // and pose+10Ch = 1
    virtual void node_copy_inverse_matrix_004134f0(void* node, void* pose) = 0;

    // 0098A750, then node+40h, the two 0098AD60 calls and node+3Ch.
    virtual void node_rebuild_world_bounds_0098a750(void* node) = 0;
    virtual int node_cell_count(void* node) = 0;
    virtual SpatialCellRect node_cell_rect_0098ad60(void* node) = 0;
    virtual std::uint32_t node_cell_key(void* node) = 0;

    // 0042E630 twice, then 0098A3D0 and 0098A310.
    virtual void unregister_node_0098a3d0(void* node) = 0;
    virtual void register_node_0098a310(void* node, const SpatialCellRect& rect) = 0;

    // node+FCh and node+100h, 0098BD45.
    virtual int node_child_count(void* node) = 0;
    virtual void* node_child(void* node, int slot) = 0;
};

// 0098BC70. Recursive: the children are refreshed whatever their static flag
// says, and only a node the grid holds (node+40h != 0) is ever re-bucketed.
void spatial_refresh_node_0098bc70(SpatialRefreshHost& host, void* node);

// 0098BDB0. step is the 0.05f the site pushes and the callee ignores.
void spatial_refresh_moved_nodes_0098bdb0(SpatialRefreshHost& host, float step);

} // namespace bsp
