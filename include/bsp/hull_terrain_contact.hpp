#pragma once
// Packet cc9_hull_terrain_contact_solver (docs/GUNNERY_OPEN_ITEMS.md section 84; the read is
// section 83). The Dyn library's contact phase for one hull body against the terrain, in the
// order 00C5BB30 runs it between the velocity phase 00C41550 and the position phase 00C5B1B0:
//
//   * ManifoldUpdate: every manifold of the hull goes through 00C4B9B0 (native, R152); a
//     manifold left with no point is retired (00C549D0's second pass).
//   * The narrow phase 00C44090 for each (hull convex shape, terrain tile) pair: the terrain/
//     convex test 00C53630 gives up to eight candidates (every hull vertex at or below the
//     terrain height, in vertex order); each goes into the pair's manifold through 00C3F760
//     (native, R138, the four-point reduction included). The manifold's friction is
//     combine(hull material Friction, 0.0) and its restitution (0 + 0) * 0.5 (the terrain shape
//     00882AC0 builds has friction 0, restitution 0).
//   * The solve 00403720 for the hull's group: the row build 00C4DE40, the warm start 00C42BA0,
//     world+38h = 10 iterations of 00C42530 / 00C42230, the write-back 00C37B50 into the
//     motion state's velocities and pseudo-velocities, and 00C35020 into the points.
//
// SUBSTITUTIONS, labelled:
//   * The terrain test uses the terrain object's own bilinear height 00ADB480 and cell normal
//     00ADAA40 over the same samples (the Dyn shape of each tile reads the tile block 00882AC0
//     hands it), not 00C53630's own interpolation schedule. The candidate is 00C53630's: the
//     surface point is the vertex moved (h - y) along the normal, and the normal points up.
//   * The hull vertices are the ConvexObject's raw points (MmodHullConvexBox::shape_points), not
//     the vertices of the hull 00C5DEB0 builds from them, unless kHullTerrainDynHullVerticesBound
//     (below) is ON.
//   * Only hull-terrain pairs (and hull-hull with kHullHullContactBound; no hull-object
//     manifold) join the group, and the
//     terrain tile is taken as the one the vertex's grid cell truncates to (a vertex on a tile's
//     inclusive far edge is not offered to the next tile as well).
//   * One substep of the host's whole step, as the host's two integration phases already run.
//     Not a substitution (GUNNERY_OPEN_ITEMS 88): world+00h is 0.05f ([00CE7638] stored at
//     004DE168), so 00C5C540's plan with the fixed 0.05f step runs no full substep and one
//     remainder substep of 0.05 (the accumulator world+48h is cleared every call).
//
// Descriptive names are hypotheses. Not the native layout or ABI.
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "bsp/avoid_zone_dyn_hull.hpp"
#include "bsp/dyn_lcp_impulse_math.hpp"
#include "bsp/rigid_body_integration.hpp"
#include "bsp/world_ocean.hpp"

namespace bsp {

// Packet cc9_hull_terrain_contact_solver. True: every ship hull runs the contact phase above
// against the terrain after the velocity phase, and the kind-8 latch (+1010h) is set from its
// contacts. False: the narrow phase runs as a census only (no state written), and the hull
// crosses land as before.
// ON by the pairs of 2026-09-30 (docs/GUNNERY_OPEN_ITEMS.md section 84.3).
inline constexpr bool kHullTerrainContactSolverBound = true;

// Packet cc9_hull_terrain_dyn_hull_vertices (docs/GUNNERY_OPEN_ITEMS.md section 86). The
// ConvexObject parse 006FAD70 copies the points (xyz at +4h of each 20h record, 006FAE40),
// centres them on their box (006F9EE0: (min + max) * 0.5 [00D7A280], stored at shape+14h;
// 006FAEA0 subtracts it) and builds the Dyn hull 00C5DEB0 at shape+0Ch from the re-centred
// points; the shape's translation is the centre. True: the narrow phase tests that hull's
// vertices (avoid_zone_dyn_hull_replace_00c5deb0, in its vertex order) plus the centre, per
// shape. False: the raw points in file order, as before.
// ON by the pairs of 2026-09-30 (docs/GUNNERY_OPEN_ITEMS.md section 86.4).
inline constexpr bool kHullTerrainDynHullVerticesBound = true;

// Packet cc9_hull_terrain_native_test (docs/GUNNERY_OPEN_ITEMS.md section 87). True: each
// (hull shape, terrain tile) pair goes through the reconstructed 00C53630 itself
// (intersect_native_dyn_terrain_convex_00c53630: its own grid transform, sample decode,
// interpolation, witness point and normal) on shape records built as 00882AC0 builds the tile
// (one static body per tile: body frame = the Landscape frame with the translation moved by
// (origin + 300 * tile, 0), shape kind 5 with an identity local frame, 33 x 33 u16 samples,
// spacing 9.375, mode 1) and as 006FAD70 builds the hull shape (kind 4, the hull vertices,
// the centre as its translation). Every tile whose x/z range meets the hull's is tested.
// False: the terrain object's height 00ADB480 and cell normal 00ADAA40 (84.1).
// ON by the pairs of 2026-09-30 (docs/GUNNERY_OPEN_ITEMS.md section 87.4).
inline constexpr bool kHullTerrainNativeTerrainTestBound = true;
// Same packet. 00C44090 keeps one manifold per body pair (00C3F4D0 FindOrCreate(body A,
// body B)): every hull shape's contacts with one tile go into one manifold. True: the key
// drops the shape. False: one manifold per (hull shape, tile), as 84.1 built it.
// ON by the pairs of 2026-09-30 (section 87.4).
inline constexpr bool kHullTerrainBodyPairManifoldBound = true;

// Packet cc9_hull_hull_contact (docs/GUNNERY_OPEN_ITEMS.md section 91; the read is 89). The
// fixed-step fanout runs 00C5C540 (row 1, 00875E0C) once for the world, not once per unit:
// every body's velocity phase 00C41550, then the collision pass (ManifoldUpdate, then the
// narrow phase 00C44090 over the broad phase's pairs), 00C4B610's groups and one solve per
// group, then every body's position phase 00C5B1B0. True: the units host runs every ship's
// motion tick and velocity phase first, then this world phase over every hull (the terrain
// manifolds, grouped by 00C4B610 and solved per group with the bodies indexed in manifold
// order, 00C4DEDB), then each hull's position phase and the rest of its tick, in unit order.
// False: each hull's contact and position phases run inside its own tick, as before.
// ON by the pairs of 2026-09-30 (docs/GUNNERY_OPEN_ITEMS.md section 91.4).
inline constexpr bool kDynWorldContactPhaseBound = true;
// Same packet. Needs kDynWorldContactPhaseBound. True: every pair of hulls whose world
// boxes meet goes through 00C44090's convex-convex path: the shape filter (group 1, mask
// 0Dh: every hull pair passes), the dispatcher cell 4 * 6 + 4 = 00C535E0 on real kind-4
// convex records (+0 the process's ConvexMeshShape table, +0Ch the 00C57C40 box, +34h the
// shape frame, +210h the 00C5DEB0 hull), one manifold per body pair (00C3F4D0) with the
// friction combine(fA, fB) and the restitution (rA + rB) * 0.5, each hit through 00C3F760;
// both hulls then join one group and one solve. False: hulls pass through each other (the
// narrow phase still runs as a census, no state written).
// ON by the pairs of 2026-09-30 (section 91.4). The collision report (009377E0 -> 008145B0,
// ramming damage) is not bound: hull-hull contact is physics only.
inline constexpr bool kHullHullContactBound = true;

struct HullTerrainContactStepResult {
    int candidates{0};        // 00C53630 outputs over all pairs this step
    int manifolds{0};         // manifolds holding a point after the insert
    int points{0};            // points solved
    float max_depth{0.0f};    // the deepest candidate (h - y)
    float normal_impulse{0.0f};   // sum of the accumulated normal impulses after the solve
    float bias_impulse{0.0f};     // likewise, the pseudo-velocity rows
    OceanVec3 delta_linear{};     // velocity change written back
    OceanVec3 delta_linear_bias{};
    bool contact{false};      // a candidate was produced (the kind-8 report)
    int hull_candidates{0};   // 00C535E0 hits against other hulls this step
    int group_bodies{0};      // dynamic bodies in the hull's group (world phase)
};

// One hull for the world phase: the body after its velocity phase, its convex shapes in body
// space and its material friction. `result` is written by the phase.
struct HullWorldEntry {
    std::size_t unit{0};
    DynBody* body{nullptr};
    const std::vector<std::vector<OceanVec3>>* shapes{nullptr};
    float friction{0.0f};
    HullTerrainContactStepResult result{};
};

class HullTerrainContactSolver {
public:
    HullTerrainContactSolver();
    ~HullTerrainContactSolver();
    HullTerrainContactSolver(const HullTerrainContactSolver&) = delete;
    HullTerrainContactSolver& operator=(const HullTerrainContactSolver&) = delete;

    // `shapes` are the hull's convex shapes in body space; `hull_friction` the material's
    // Friction (00939365). With `apply` false only the narrow phase runs, into the census.
    HullTerrainContactStepResult step(std::size_t unit, DynBody& body,
                                      const std::vector<std::vector<OceanVec3>>& shapes,
                                      float hull_friction, float dt, bool apply);
    // Drops a unit's manifolds (death, removal).
    void forget(std::size_t unit);
    // kDynWorldContactPhaseBound: 00C5BB30's contact phase once for every hull, in `hulls`
    // order (ascending unit): ManifoldUpdate over every manifold, the terrain narrow phase per
    // hull, with `hull_hull` the hull pairs, 00C4B610's groups and one solve per group.
    void world_step(std::vector<HullWorldEntry>& hulls, float dt, bool hull_hull);
    // The hull-pair narrow phase as a census only (no manifold, no state): the OFF build's
    // record of which hulls would touch.
    void hull_hull_census(std::vector<HullWorldEntry>& hulls);

    struct Census {
        unsigned long long steps{0}, contact_steps{0}, candidates{0}, rejected_normal{0};
        unsigned long long solves{0}, rows{0}, retired{0};
        std::size_t units_touched{0};
        float max_depth{0.0f};
        // Dyn hull builds (kHullTerrainDynHullVerticesBound): shapes built, their raw points
        // and the hull vertices 00C5DEB0 kept.
        unsigned long long hull_shapes{0}, raw_points{0}, hull_vertices{0};
        // Hull pairs (00C535E0): world steps, pairs whose world boxes met, shape pairs
        // tested, hits, 00C4B610 groups solved, groups with two or more hulls and the largest.
        unsigned long long world_steps{0}, hull_pairs_near{0}, hull_shape_tests{0};
        unsigned long long hull_hits{0}, groups{0}, multi_hull_groups{0};
        int max_group_bodies{0};
    };
    const Census& census() const noexcept { return census_; }
    // Per hull pair (lower unit first): the first census step with a hit, the steps with one
    // and the deepest hit (witness A minus witness B along the normal, world space).
    struct HullPairCensus {
        unsigned long long first_step{0}, steps{0};
        float max_depth{0.0f};
    };
    const std::map<std::pair<std::size_t, std::size_t>, HullPairCensus>& hull_pairs() const noexcept {
        return hull_pairs_;
    }

private:
    struct Manifold;
    struct HullShape {
        std::vector<OceanVec3> raw;         // the raw points the build came from
        std::vector<OceanVec3> vertices;    // body space: hull vertex + centre
        // The native convex shape record (kind 4) for 00C53630: +34h local frame (identity,
        // translation the centre), +210h -> {16-byte vertex records, count}.
        float centre[3]{};
        std::vector<float> records;         // 4 floats per vertex: the centred point, pad
        struct Mesh { const float* vertices; std::uint32_t count; } mesh{};
        std::uint8_t shape[0x240]{};
        // The 00C5DEB0 hull kept for the kind-4 convex record 00C535E0 reads (+210h).
        std::shared_ptr<AvoidZoneDynHullHandle> handle;
        alignas(16) std::uint8_t convex[0x240]{};
        bool convex_ready{false};
        // The body the record's +4h points to: 00C51C20 and 00C48BE0 read its 3x4 at
        // +08h..+37h (the rows, then the translation at +2Ch).
        std::uint8_t body[0x40]{};
    };
    HullShape* hull_convex(std::size_t unit, std::size_t shape,
                           const std::vector<OceanVec3>& raw, float friction,
                           const DynBody& body);
    void hull_hull_narrow_phase(std::vector<HullWorldEntry>& hulls, bool apply);
    std::map<std::pair<std::size_t, std::size_t>, std::unique_ptr<Manifold>> pair_manifolds_;
    std::map<std::pair<std::size_t, std::size_t>, HullPairCensus> hull_pairs_;
    unsigned long long manifold_serial_{0};
    const std::vector<OceanVec3>& dyn_hull_vertices(std::size_t unit, std::size_t shape,
                                                    const std::vector<OceanVec3>& raw);
    HullShape& hull_shape(std::size_t unit, std::size_t shape, const std::vector<OceanVec3>& raw);
    std::map<std::pair<std::size_t, std::size_t>, HullShape> hulls_;
    // The native terrain tile records (kind 5), per height field and tile.
    struct TerrainTile;
    std::map<std::tuple<const void*, int, int>, std::unique_ptr<TerrainTile>> tiles_;
    HullTerrainContactStepResult native_narrow_phase(std::size_t unit, DynBody& body,
        const std::vector<std::vector<OceanVec3>>& shapes, bool apply);
    using Key = std::tuple<std::size_t, int, int, int, int>;  // unit, shape, landscape, tx, tz
    std::map<Key, std::unique_ptr<Manifold>> manifolds_;
    std::map<std::size_t, bool> touched_;
    Census census_;
};

}  // namespace bsp
