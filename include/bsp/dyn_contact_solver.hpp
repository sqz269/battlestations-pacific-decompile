#pragma once

#include <cstddef>
#include <cstdint>

#include "bsp/rigid_body_integration.hpp"

// The collision, group and LCP phases of one Dyn substep: 00C5BB5F..00C5C455 inside
// 00C5BB30, plus 00C4B610 (CreateGroups), 00C4B550 (SleepGroups), 00C33140 (the task
// fork-join) and the contact report the substep hands to the listener at world+24h.
//
// docs/DYN_CONTACT_SOLVER.md carries the addresses, the original ABI and the
// uncertainty. Everything here is a semantic C++ interface for MSVC Win32, not a
// drop-in binary replacement, and every descriptive name is a hypothesis rather than a
// recovered symbol. Three names are not: the image's own RTTI spells
// Dyn::Scene::LCPSolverTask (vtable 00D7A088, TypeDescriptor 00E17394) and
// Dyn::Scene::LCPSolver2Task (vtable 00D7A090, TypeDescriptor 00E1736C), and the
// profiler labels inside the block are the image strings "Collide", "BroadPhase",
// "BroadPhaseUpdate", "ManifoldUpdate", "IntersectLoop", "GetManifold",
// "CreateGroups", "Solve", "SolverPreStep", "SolveConstraints" and "SleepGroups".
//
// What is NOT here: the per-constraint impulse math. It lives below
// Dyn_Scene_LCPSolverTask_vslot0 (00403720) in 00C4F040, 00C42530, 00C42230, 00C37B50
// and 00C35020, none of which this packet read. The doc's coverage table lists those
// ranges as unread. The phases below are the schedule around them.

namespace bsp {

// ---------------------------------------------------------------------------
// The scene's manifold list (the collision pass's output)
//
// The scene is the 0E8h-byte object at world+444h (operator_new at 00C41BF2,
// constructed by 00C38070 at 00C41C09). Its manifold container is at scene+0B0h;
// the offsets below are inside that container and inside one manifold node. Every
// one of them is read at 00C5C13D..00C5C42D, the record-building walk.
// ---------------------------------------------------------------------------
inline constexpr std::size_t kDynSceneManifoldContainerOffset = 0xb0;  // 00C5C143
inline constexpr std::size_t kDynSceneManifoldListHeadOffset = 0xec;   // 00C5C18B
inline constexpr std::size_t kDynSceneManifoldListSentinelOffset = 0xf0;  // 00C5C193
inline constexpr std::size_t kDynSceneManifoldCountOffset = 0x1d0;     // 00C5C149

inline constexpr std::size_t kDynManifoldPointArrayOffset = 0x08;  // 00C5C1CB, minus 14h
inline constexpr std::size_t kDynManifoldPointStride = 0x30;       // pfVar25 += 0Ch floats
inline constexpr std::size_t kDynManifoldPointCountOffset = 0xc8;  // 00C5C1B2
inline constexpr std::size_t kDynManifoldBodyAOffset = 0xcc;       // 00C5C1D0
inline constexpr std::size_t kDynManifoldBodyBOffset = 0xd0;       // 00C5C27C
inline constexpr std::size_t kDynManifoldGroupMarkOffset = 0xd4;   // int16, 00C4B610
inline constexpr std::size_t kDynManifoldNextOffset = 0xdc;        // 00C5C41A

// The body fields the group phases use, on top of the ones
// bsp/rigid_body_integration.hpp already declares for B.
inline constexpr std::size_t kDynBodyContactArrayOffset = 0x74;  // manifold* array
inline constexpr std::size_t kDynBodyContactCountOffset = 0x78;  // its count
inline constexpr std::size_t kDynBodyGroupMarkOffset = 0x64;     // int16 visit mark

// -1 is "not yet assigned to a group this substep"; both marks are int16 and both are
// set to 0 the moment the flood fill reaches the node (00C4B610).
inline constexpr std::int16_t kDynGroupMarkUnassigned = -1;
inline constexpr std::int16_t kDynGroupMarkAssigned = 0;

// ---------------------------------------------------------------------------
// One contact point inside a manifold node
//
// Stride 30h from manifold+08h. The field meanings come from the record builder,
// which is the only reader this packet read; the producer of the four fields is the
// narrow phase under 00C57070, which this packet did not read, so `normal_scale` is
// named for what the builder does with it (it multiplies the normal by +s for the
// record's first impulse slot and by -s for the second) and not for what the narrow
// phase meant by it. Penetration depth and accumulated normal impulse are both
// plausible; this packet does not choose.
// ---------------------------------------------------------------------------
struct DynContactPoint {
    float normal[3]{};         // point+00h
    float local_point_a[3]{};  // point+0Ch, in body A's frame
    float local_point_b[3]{};  // point+18h, in body B's frame
    float normal_scale{0.0f};  // point+24h
};

// ---------------------------------------------------------------------------
// The 58h-byte record the substep hands to the contact listener
//
// 00C5C1BE..00C5C40C fills 00h..53h of each record; 54h is never written and the
// allocation rounds the count up to four points per manifold
// (00C5C149 manifoldCount * 4, 00C5C15E * 58h).
// ---------------------------------------------------------------------------
inline constexpr std::size_t kDynContactReportRecordSize = 0x58;
inline constexpr std::size_t kDynContactReportRecordsPerManifold = 4;

struct DynContactReportRecord {
    float world_point_a[3]{};  // +00h, body A's transform applied to local_point_a
    float world_point_b[3]{};  // +0Ch, body B's transform applied to local_point_b
    float normal[3]{};         // +18h, copied unchanged
    float impulse_on_a[3]{};   // +24h, normal *  normal_scale
    float impulse_on_b[3]{};   // +30h, normal * -normal_scale
    float zeroed_3c[3]{};      // +3Ch, written as zero, no writer of a value found
    float zeroed_48[3]{};      // +48h, written as zero, no writer of a value found
};

// 00C5C1D6..00C5C40C for one point, in the listing's order. `p' = px*row0 + py*row1
// + pz*row2 + position`, the same 3x4 row layout bsp/rigid_body_integration.hpp
// declares for B (rows at B+08h, +14h, +20h, position at B+2Ch). All x87 at the
// game's 24-bit precision, so every intermediate is a float32.
void dyn_build_contact_report_record(const DynContactPoint& point, const DynBody& body_a,
                                     const DynBody& body_b,
                                     DynContactReportRecord& out) noexcept;

// ---------------------------------------------------------------------------
// The solver task partition, 00C5BC2B..00C5BCC7 and 00C5C04E..00C5C0E7
//
// The two blocks are the same code against two task arrays. world+458h is the group
// count, world+460h / world+478h the task capacity (both sized to the scheduler's
// worker count at 00C41C32). The partition is integer division: the first
// `task_count - 1` tasks take `group_count / task_count` groups each and the last
// task takes everything left, so the last task is the one that absorbs the remainder.
// ---------------------------------------------------------------------------
enum class DynSolverTaskKind : std::int32_t {
    // world+10h == 0: the array at world+45Ch, pointers at world+468h, vtable
    // 00D7A088, Dyn::Scene::LCPSolverTask. This is the shipped mode: 004DE19F
    // stores EBX = 0 into desc+20h and 00C41B06 copies it to world+10h.
    kLcpSolverTask = 0,
    // world+10h == 1: the array at world+474h, pointers at world+480h, vtable
    // 00D7A090, Dyn::Scene::LCPSolver2Task.
    kLcpSolver2Task = 1,
};

// world+10h outside {0, 1} skips the solve, the contact report and nothing else:
// 00C5BBF5/00C5BBFE jump straight to 00C5C456, the UpdatePosition phase.
bool dyn_solver_mode_is_known(std::int32_t world_solver_mode) noexcept;

struct DynSolverTaskRange {
    std::int32_t first_group{0};  // task+0Ch
    std::int32_t last_group{-1};  // task+10h, inclusive
    float dt{0.0f};               // task+14h
};

// min(group_count, capacity). 00C5BC37/00C5C05A; the caller has already checked that
// the group count is non-zero (00C5BC06 / 00C5C02A), so this is never called with 0.
std::int32_t dyn_solver_task_count(std::int32_t group_count, std::int32_t capacity) noexcept;

// The range task `task_index` of `task_count` gets. The last index takes
// `group_count - 1` as its inclusive end whatever the division left.
DynSolverTaskRange dyn_solver_task_range(std::int32_t group_count, std::int32_t task_count,
                                         std::int32_t task_index, float dt) noexcept;

// world+38h, the iteration count each solver task runs per group
// (00C5C71F `local_4 = *(*task_world + 38h)` inside 00C5C710, and the same field at
// 004037xx inside the LCPSolverTask body). 004DE1A3 authors it as 10, which
// docs/DYN_WORLD_SETTINGS.md carried as a guess; this packet's reader settles it.
inline constexpr std::size_t kDynWorldSolverIterationsOffset = 0x38;

// ---------------------------------------------------------------------------
// Group formation, 00C4B610 ("CreateGroups")
//
// A flood fill over the manifold graph. Seeds are manifolds that have contact points,
// are unassigned, and have at least one body that is neither static nor asleep. From a
// seed the fill walks both bodies' own manifold arrays, so one group is one island of
// touching bodies. Every manifold the fill reaches wakes both of its bodies.
//
// The host is addressed with opaque handles because the native ones are raw pointers;
// 0 is the null handle and the sentinel is reported by `manifold_list_sentinel`.
// ---------------------------------------------------------------------------
using DynHandle = std::uint32_t;

struct DynGroupFormationHost {
    virtual ~DynGroupFormationHost() = default;

    // 00C4B632, the outlined block at 00C3F410 with the manager in ESI (00C4B630): frees
    // group's contact array, then manager+8h (world+450h) = 0 and manager+10h
    // (world+458h) = 0.
    virtual void clear_groups_00c3f410() = 0;
    // 00C4B639, 00C36AC0 with the manager in ECX (00C4B637), read in full: it walks the world body list (world+204h,
    // sentinel world+208h, next at B+84h) writing -1 into every B+64h, then the
    // scene's manifold list writing -1 into every manifold+0D4h. Both marks are the
    // ones the fill below tests, so this is the fill's reset pass.
    virtual void reset_marks_00c36ac0() = 0;

    virtual std::int32_t scene_manifold_count() = 0;  // scene+0B0h +1D0h, 00C4B64C
    virtual DynHandle manifold_list_head() = 0;       // container+0ECh
    virtual DynHandle manifold_list_sentinel() = 0;   // container+0F0h
    virtual DynHandle manifold_next(DynHandle manifold) = 0;  // manifold+0DCh

    virtual std::int32_t manifold_point_count(DynHandle manifold) = 0;   // +0C8h
    virtual std::int16_t manifold_group_mark(DynHandle manifold) = 0;    // +0D4h
    virtual void set_manifold_group_mark(DynHandle manifold, std::int16_t mark) = 0;
    virtual DynHandle manifold_body_a(DynHandle manifold) = 0;  // +0CCh
    virtual DynHandle manifold_body_b(DynHandle manifold) = 0;  // +0D0h

    virtual std::uint32_t body_flags(DynHandle body) = 0;  // B+50h
    // B+50h &= 0FFFFFFEDh, the same wake mask every body mutator uses
    // (kDynBodyWakeMask in bsp/rigid_body_integration.hpp).
    virtual void wake_body(DynHandle body) = 0;
    virtual std::int16_t body_group_mark(DynHandle body) = 0;  // B+64h
    virtual void set_body_group_mark(DynHandle body, std::int16_t mark) = 0;
    virtual std::int32_t body_contact_count(DynHandle body) = 0;             // B+78h
    virtual DynHandle body_contact(DynHandle body, std::int32_t index) = 0;  // B+74h[i]

    // The group vector grows to `count` entries of 0Ch bytes each, from 00C4B700; each
    // entry is a {data, size, capacity} vector of manifold pointers. `count` is
    // world+458h (incremented at 00C4B700) and world+450h, which the growth keeps equal.
    virtual void set_group_count(std::int32_t count) = 0;
    // 00C4B86C, the outlined push_back at 00C36B60 with the group entry in ESI.
    virtual void append_to_group_00c36b60(std::int32_t group_index, DynHandle manifold) = 0;
};

// Returns the number of groups formed, which is what the substep then reads as
// world+458h.
std::int32_t dyn_create_contact_groups_00c4b610(DynGroupFormationHost& host);

// ---------------------------------------------------------------------------
// Group sleep, 00C4B550 ("SleepGroups")
//
// A group sleeps when every one of its contacts has both bodies already static or
// asleep (B+50h & 3). A sleeping group has bit 4 set on both bodies of every contact,
// which is exactly the bit both integration phases test to skip a body. The routine
// then clears the whole group vector through the same 00C3F410 block CreateGroups
// opens with, so the groups do not survive the substep.
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kDynBodyStaticOrAsleepMask = 0x3u;  // 00C4B58B, 00C4B597

struct DynContactGroupHost {
    virtual ~DynContactGroupHost() = default;

    virtual std::int32_t group_count() = 0;  // manager+8h == world+450h, 00C4B55B
    virtual std::int32_t group_contact_count(std::int32_t group) = 0;  // entry+4h
    virtual DynHandle group_contact(std::int32_t group, std::int32_t index) = 0;
    virtual std::uint32_t body_flags(DynHandle body) = 0;
    virtual void set_body_flags(DynHandle body, std::uint32_t flags) = 0;
    virtual DynHandle manifold_body_a(DynHandle manifold) = 0;
    virtual DynHandle manifold_body_b(DynHandle manifold) = 0;
    virtual void clear_groups_00c3f410() = 0;  // 00C4B5FF
};

// Returns how many groups were put to sleep.
std::int32_t dyn_sleep_contact_groups_00c4b550(DynContactGroupHost& host);

}  // namespace bsp
