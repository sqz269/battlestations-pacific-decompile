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
//   * Only hull-terrain pairs: no hull-hull or hull-object manifold joins the group, and the
//     terrain tile is taken as the one the vertex's grid cell truncates to (a vertex on a tile's
//     inclusive far edge is not offered to the next tile as well).
//   * One substep of the host's whole step, as the host's two integration phases already run.
//
// Descriptive names are hypotheses. Not the native layout or ABI.
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

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

    struct Census {
        unsigned long long steps{0}, contact_steps{0}, candidates{0}, rejected_normal{0};
        unsigned long long solves{0}, rows{0}, retired{0};
        std::size_t units_touched{0};
        float max_depth{0.0f};
        // Dyn hull builds (kHullTerrainDynHullVerticesBound): shapes built, their raw points
        // and the hull vertices 00C5DEB0 kept.
        unsigned long long hull_shapes{0}, raw_points{0}, hull_vertices{0};
    };
    const Census& census() const noexcept { return census_; }

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
    };
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
