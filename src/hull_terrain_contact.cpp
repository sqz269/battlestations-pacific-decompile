#include "bsp/hull_terrain_contact.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "bsp/avoid_zone_dyn_hull.hpp"
#include "bsp/dyn_collision_pass.hpp"
#include "bsp/game_hosts_scene_contents.hpp"
#include "bsp/native_dyn_collision_pass.hpp"
#include "bsp/native_dyn_narrow_phase.hpp"
#include "bsp/native_dyn_terrain_convex.hpp"

#include <algorithm>

namespace bsp::game {
const CameraAxesCrtAccess& application_camera_axes_crt() noexcept;   // game_hosts.hpp
}

namespace bsp {
namespace {

constexpr float kTerrainCell = 9.375f;       // +1Ch [00D0E658], the Dyn tile spacing too
constexpr int kTileCells = 32;               // [00D5D658]
constexpr int kCandidatesPerPair = 8;        // 00C53630 stops at eight
constexpr float kTerrainFriction = 0.0f;     // 00882AC0: ESP+4Ch, 008835A4 / 00883684
constexpr float kTerrainRestitution = 0.0f;  // 00882AC0: ESP+48h, 0088359E

// The native body, as far as 00C3F760 and 00C4B9B0 read it: the 3x4 at B+08h.
struct alignas(16) NativeBody {
    std::uint8_t bytes[0x100]{};
    void set_pose(const float r0[3], const float r1[3], const float r2[3], const float p[3]) {
        std::memcpy(bytes + 0x08, r0, 12);
        std::memcpy(bytes + 0x14, r1, 12);
        std::memcpy(bytes + 0x20, r2, 12);
        std::memcpy(bytes + 0x2C, p, 12);
    }
};

float f32(double v) { return static_cast<float>(v); }

}  // namespace

// The native manifold: points at +08h (30h each, four), count +0C8h, body A +0CCh, body B
// +0D0h (docs/NATIVE_DYN_NARROW_PHASE_R138.md). Body A is the terrain tile's static body (the
// identity frame, so its local point is the world point); body B the hull.
struct HullTerrainContactSolver::Manifold {
    alignas(16) std::uint8_t bytes[0x100]{};
    NativeBody terrain;
    NativeBody hull;
    Manifold() {
        const float r0[3] = {1.0f, 0.0f, 0.0f}, r1[3] = {0.0f, 1.0f, 0.0f},
                    r2[3] = {0.0f, 0.0f, 1.0f}, p[3] = {0.0f, 0.0f, 0.0f};
        terrain.set_pose(r0, r1, r2, p);
        void* a = &terrain;
        void* b = &hull;
        std::memcpy(bytes + 0xCC, &a, sizeof(a));
        std::memcpy(bytes + 0xD0, &b, sizeof(b));
    }
    std::int32_t& count() { return *reinterpret_cast<std::int32_t*>(bytes + 0xC8); }
    DynSolverContactPoint* points() { return reinterpret_cast<DynSolverContactPoint*>(bytes + 0x08); }
};
static_assert(sizeof(DynSolverContactPoint) == 0x30, "the native point is 30h bytes");

HullTerrainContactSolver::HullTerrainContactSolver() = default;
HullTerrainContactSolver::~HullTerrainContactSolver() = default;

void HullTerrainContactSolver::forget(std::size_t unit) {
    for (auto it = manifolds_.begin(); it != manifolds_.end();) {
        if (std::get<0>(it->first) == unit) it = manifolds_.erase(it);
        else ++it;
    }
    for (auto it = hulls_.begin(); it != hulls_.end();) {
        if (it->first.first == unit) it = hulls_.erase(it);
        else ++it;
    }
}


// 006FAD70's shape build: centre on the box (006F9EE0), 00C5DEB0 on the re-centred points,
// the centre back as the shape's translation. Rebuilt only when the raw points change. With
// kHullTerrainDynHullVerticesBound false the shape is the raw points with a zero centre.
HullTerrainContactSolver::HullShape& HullTerrainContactSolver::hull_shape(
    std::size_t unit, std::size_t shape, const std::vector<OceanVec3>& raw) {
    auto found = hulls_.find({unit, shape});
    if (found != hulls_.end() && found->second.raw.size() == raw.size() &&
        (raw.empty() ||
         std::memcmp(found->second.raw.data(), raw.data(), raw.size() * sizeof(OceanVec3)) == 0)) {
        return found->second;
    }
    HullShape& h = hulls_[{unit, shape}];
    h.raw = raw;
    h.vertices.clear();
    h.records.clear();
    std::vector<OceanVec3> local;
    float centre[3] = {0.0f, 0.0f, 0.0f};
    if (kHullTerrainDynHullVerticesBound) {
        // 006F9EE0: min/max from +-FLT_MAX [00D7A244/00D7A248], the centre (min + max) * 0.5,
        // each sum rounded to float first (006FA25D..006FA2AF).
        float lo[3] = {3.402823466e+38f, 3.402823466e+38f, 3.402823466e+38f};
        float hi[3] = {-3.402823466e+38f, -3.402823466e+38f, -3.402823466e+38f};
        for (const OceanVec3& p : raw) {
            const float c[3] = {p.x, p.y, p.z};
            for (int k = 0; k < 3; ++k) {
                if (lo[k] > c[k]) lo[k] = c[k];
                if (hi[k] < c[k]) hi[k] = c[k];
            }
        }
        for (int k = 0; k < 3; ++k) {
            centre[k] = f32(static_cast<double>(f32(static_cast<double>(lo[k]) + hi[k])) * 0.5);
        }
        std::vector<OceanVec3> centred(raw.size());
        for (std::size_t i = 0; i < raw.size(); ++i) {
            centred[i] = OceanVec3{f32(static_cast<double>(raw[i].x) - centre[0]),
                                   f32(static_cast<double>(raw[i].y) - centre[1]),
                                   f32(static_cast<double>(raw[i].z) - centre[2])};
        }
        const AvoidZoneDynHullMemory memory{
            nullptr, [](void*, std::size_t bytes) -> void* { return std::malloc(bytes); },
            [](void*, void* p) { std::free(p); }};
        AvoidZoneDynHullHandle handle{};
        avoid_zone_dyn_hull_construct_00c5df30(handle, centred.data(),
                                               static_cast<std::uint32_t>(centred.size()), memory);
        const std::uint32_t n = avoid_zone_dyn_hull_vertex_count_00c32d20(handle);
        for (std::uint32_t i = 0; i < n; ++i) local.push_back(handle.data->vertices[i].point);
        avoid_zone_dyn_hull_destroy_00c37450(handle, memory);
        ++census_.hull_shapes;
        census_.raw_points += raw.size();
        census_.hull_vertices += n;
    } else {
        local = raw;
    }
    for (int k = 0; k < 3; ++k) h.centre[k] = centre[k];
    h.vertices.reserve(local.size());
    h.records.reserve(local.size() * 4);
    for (const OceanVec3& v : local) {
        h.vertices.push_back(OceanVec3{f32(static_cast<double>(v.x) + centre[0]),
                                       f32(static_cast<double>(v.y) + centre[1]),
                                       f32(static_cast<double>(v.z) + centre[2])});
        h.records.insert(h.records.end(), {v.x, v.y, v.z, 0.0f});
    }
    // The kind-4 record 00C53630 reads: +08h kind, +34h..+63h the local frame (identity, the
    // centre as translation), +210h the mesh {vertices +0, count +4, 16-byte stride}.
    std::memset(h.shape, 0, sizeof(h.shape));
    const std::uint32_t kind = 4;
    std::memcpy(h.shape + 0x08, &kind, 4);
    const float frame[12] = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
                             centre[0], centre[1], centre[2]};
    std::memcpy(h.shape + 0x34, frame, sizeof(frame));
    h.mesh.vertices = h.records.data();
    h.mesh.count = static_cast<std::uint32_t>(local.size());
    const HullShape::Mesh* mesh = &h.mesh;
    std::memcpy(h.shape + 0x210, &mesh, sizeof(mesh));
    return h;
}

const std::vector<OceanVec3>& HullTerrainContactSolver::dyn_hull_vertices(
    std::size_t unit, std::size_t shape, const std::vector<OceanVec3>& raw) {
    return hull_shape(unit, shape, raw).vertices;
}

// The tile 00882AC0 builds (00883540..008839E2, one static body per tile with one shape).
struct HullTerrainContactSolver::TerrainTile {
    std::uint8_t shape[0x240]{};
    std::vector<std::uint16_t> samples;
    float frame[12]{};   // the tile body's 3x4: rows, then the translation
};

HullTerrainContactStepResult HullTerrainContactSolver::native_narrow_phase(
    std::size_t unit, DynBody& body, const std::vector<std::vector<OceanVec3>>& shapes,
    bool apply) {
    HullTerrainContactStepResult out;
    const float hull_frame[12] = {body.row0[0], body.row0[1], body.row0[2],
                                  body.row1[0], body.row1[1], body.row1[2],
                                  body.row2[0], body.row2[1], body.row2[2],
                                  body.position[0], body.position[1], body.position[2]};
    std::vector<HullShape*> hulls;
    float min_x = 3.402823466e+38f, max_x = -3.402823466e+38f;
    float min_z = 3.402823466e+38f, max_z = -3.402823466e+38f;
    for (std::size_t s = 0; s < shapes.size(); ++s) {
        HullShape& h = hull_shape(unit, s, shapes[s]);
        hulls.push_back(&h);
        for (const OceanVec3& v : h.vertices) {
            const float x = f32(static_cast<double>(body.position[0]) + body.row0[0] * v.x +
                                body.row1[0] * v.y + body.row2[0] * v.z);
            const float z = f32(static_cast<double>(body.position[2]) + body.row0[2] * v.x +
                                body.row1[2] * v.y + body.row2[2] * v.z);
            if (x < min_x) min_x = x;
            if (x > max_x) max_x = x;
            if (z < min_z) min_z = z;
            if (z > max_z) max_z = z;
        }
    }
    if (!(min_x <= max_x)) return out;
    const game::SceneWorldClassLists& lists = game::scene_world_class_lists();
    const std::vector<std::size_t>& landscapes = lists.list(game::kSceneLandscapeClassId);
    constexpr double kTile = 300.0;   // [00CE3AE8], 32 cells of 9.375
    const CameraAxesCrtAccess& crt = game::application_camera_axes_crt();
    for (std::size_t li = 0; li < landscapes.size(); ++li) {
        const game::SceneWorldObject& object = lists.objects()[landscapes[li]];
        if (!object.terrain) continue;
        const game::SceneTerrainHeightField& t = *object.terrain;
        // Every tile whose inclusive x/z range [x0, x0 + 300] meets the hull's (a vertex on a
        // shared edge belongs to both tiles; 00C53630 bounds-checks each vertex itself).
        const double bx = static_cast<double>(t.node_x) + t.origin_x;
        const double bz = static_cast<double>(t.node_z) + t.origin_z;
        const int tx_lo = (std::max)(0, static_cast<int>(std::floor((min_x - bx) / kTile)) - 1);
        const int tx_hi = (std::min)(t.tiles_wide - 1, static_cast<int>(std::floor((max_x - bx) / kTile)));
        const int tz_lo = (std::max)(0, static_cast<int>(std::floor((min_z - bz) / kTile)) - 1);
        const int tz_hi = (std::min)(t.tiles_deep - 1, static_cast<int>(std::floor((max_z - bz) / kTile)));
        for (int tx = tx_lo; tx <= tx_hi; ++tx) {
            for (int tz = tz_lo; tz <= tz_hi; ++tz) {
                const int bi = t.block_index[static_cast<std::size_t>(t.tiles_wide) * tz + tx];
                if (bi < 0) continue;   // 00883564: no block, no body
                auto& slot = tiles_[{&t, tx, tz}];
                if (!slot) {
                    slot = std::make_unique<TerrainTile>();
                    TerrainTile& tile = *slot;
                    const game::SceneTerrainBlock& block = t.blocks[static_cast<std::size_t>(bi)];
                    // 33 x 33 samples. LABELLED: the right/down neighbours an inclusive edge
                    // coordinate reads past the block are padded with FFFFh (-1000.0); the image
                    // reads whatever follows the block's samples.
                    tile.samples.assign(block.samples.begin(), block.samples.end());
                    tile.samples.resize(block.samples.size() + 34, 0xFFFFu);
                    const std::uint32_t kind = 5, width = 33, mode = 1;
                    const float spacing = kTerrainCell;
                    const float frame[12] = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                                             0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f};
                    const std::uint16_t* samples = tile.samples.data();
                    std::memcpy(tile.shape + 0x08, &kind, 4);
                    std::memcpy(tile.shape + 0x34, frame, sizeof(frame));
                    std::memcpy(tile.shape + 0x210, &samples, sizeof(samples));
                    std::memcpy(tile.shape + 0x214, &width, 4);
                    std::memcpy(tile.shape + 0x218, &width, 4);
                    std::memcpy(tile.shape + 0x21C, &spacing, 4);
                    std::memcpy(tile.shape + 0x220, &spacing, 4);
                    std::memcpy(tile.shape + 0x224, &block.inv_scale, 4);   // 1 / desc+58h
                    std::memcpy(tile.shape + 0x228, &block.offset, 4);      // desc+54h
                    std::memcpy(tile.shape + 0x22C, &mode, 4);
                    // 0088384F..00883905: the Landscape frame's translation plus
                    // (origin_x + 300 tx, 0, origin_z + 300 tz), each sum rounded to float.
                    // LABELLED: the frame's rotation is taken as identity, as 84.1 does.
                    tile.frame[0] = 1.0f;
                    tile.frame[4] = 1.0f;
                    tile.frame[8] = 1.0f;
                    tile.frame[9] = f32(static_cast<double>(
                        f32(static_cast<double>(t.origin_x) + static_cast<float>(300 * tx))) + t.node_x);
                    tile.frame[10] = f32(0.0 + t.node_y);
                    tile.frame[11] = f32(static_cast<double>(
                        f32(static_cast<double>(t.origin_z) + static_cast<float>(300 * tz))) + t.node_z);
                }
                const TerrainTile& tile = *slot;
                for (std::size_t s = 0; s < hulls.size(); ++s) {
                    alignas(16) std::uint8_t result[4 + 8 * 36]{};
                    if (!intersect_native_dyn_terrain_convex_00c53630(
                            result, tile.shape, tile.frame, hulls[s]->shape, hull_frame, crt)) {
                        continue;
                    }
                    std::int32_t n = 0;
                    std::memcpy(&n, result, 4);
                    const float* c = reinterpret_cast<const float*>(result + 4);
                    for (std::int32_t k = 0; k < n; ++k, c += 9) {
                        // Point A is tile-local; the host's terrain body is the identity frame,
                        // so it goes in as the tile translation plus the local point.
                        const float cand[9] = {f32(static_cast<double>(c[0]) + tile.frame[9]),
                                               f32(static_cast<double>(c[1]) + tile.frame[10]),
                                               f32(static_cast<double>(c[2]) + tile.frame[11]),
                                               c[3], c[4], c[5], c[6], c[7], c[8]};
                        const double wx = static_cast<double>(body.position[0]) +
                            body.row0[0] * c[3] + body.row1[0] * c[4] + body.row2[0] * c[5];
                        const double wy = static_cast<double>(body.position[1]) +
                            body.row0[1] * c[3] + body.row1[1] * c[4] + body.row2[1] * c[5];
                        const double wz = static_cast<double>(body.position[2]) +
                            body.row0[2] * c[3] + body.row1[2] * c[4] + body.row2[2] * c[5];
                        const float depth = f32((cand[0] - wx) * cand[6] + (cand[1] - wy) * cand[7] +
                                                (cand[2] - wz) * cand[8]);
                        ++out.candidates;
                        ++census_.candidates;
                        out.contact = true;
                        if (depth > out.max_depth) out.max_depth = depth;
                        if (!apply) continue;
                        if (!dyn_contact_normal_is_unit(cand + 6)) ++census_.rejected_normal;
                        const int shape_key = kHullTerrainBodyPairManifoldBound ? -1 : static_cast<int>(s);
                        const Key key{unit, shape_key, static_cast<int>(li), tx, tz};
                        auto found = manifolds_.find(key);
                        if (found == manifolds_.end()) {
                            found = manifolds_.emplace(key, std::make_unique<Manifold>()).first;
                        }
                        Manifold& m = *found->second;
                        m.hull.set_pose(body.row0, body.row1, body.row2, body.position);
                        float in[9];
                        std::memcpy(in, cand, sizeof(in));
                        insert_native_dyn_contact_00c3f760(m.bytes, in);
                    }
                }
            }
        }
    }
    return out;
}


HullTerrainContactStepResult HullTerrainContactSolver::step(
    std::size_t unit, DynBody& body, const std::vector<std::vector<OceanVec3>>& shapes,
    float hull_friction, float dt, bool apply) {
    HullTerrainContactStepResult out;
    ++census_.steps;
    const Key lo{unit, -1, -1, -1, -1};
    const auto first = manifolds_.lower_bound(lo);

    // ManifoldUpdate (00C549D0): refresh this hull's manifolds against its current pose, then
    // retire the empty ones. Only with apply: the census build keeps no manifold.
    if (apply) {
        for (auto it = first; it != manifolds_.end() && std::get<0>(it->first) == unit;) {
            Manifold& m = *it->second;
            m.hull.set_pose(body.row0, body.row1, body.row2, body.position);
            refresh_native_dyn_manifold_00c4b9b0(m.bytes);
            if (m.count() <= 0) {
                ++census_.retired;
                it = manifolds_.erase(it);
            } else {
                ++it;
            }
        }
    }

    // The narrow phase: 00C53630 per (shape, tile), then 00C3F760 per candidate.
    // 00C44104..00C44110, the shape filter. A hull shape's group is 1 (009394DD) and its mask
    // 0Dh | the class bit (009394A9, 009395E2); NavigatorSetAvoidLandCollision(false) resets it
    // to 0Dh (008A3C79 -> 0092BD00 -> 00C48020, SHIP_AI 87.6). The terrain's group is 8, its
    // mask 0 (00882AC0). Both hull masks carry bit 8, so every hull-terrain pair passes.
    constexpr std::uint32_t kHullGroup = 1, kHullMaskBase = 0x0D, kTerrainGroup = 8,
                            kTerrainMask = 0;
    if (!dyn_shapes_overlap_filter(kHullGroup, kHullMaskBase, kTerrainGroup, kTerrainMask)) {
        return out;
    }
    const game::SceneWorldClassLists& lists = game::scene_world_class_lists();
    const std::vector<std::size_t>& landscapes = lists.list(game::kSceneLandscapeClassId);
    if (kHullTerrainNativeTerrainTestBound) out = native_narrow_phase(unit, body, shapes, apply);
    for (std::size_t s = 0; !kHullTerrainNativeTerrainTestBound && s < shapes.size(); ++s) {
        const std::vector<OceanVec3>& vertices =
            kHullTerrainDynHullVerticesBound ? dyn_hull_vertices(unit, s, shapes[s]) : shapes[s];
        for (std::size_t li = 0; li < landscapes.size(); ++li) {
            const game::SceneWorldObject& object = lists.objects()[landscapes[li]];
            if (!object.terrain) continue;
            const game::SceneTerrainHeightField& t = *object.terrain;
            std::map<std::pair<int, int>, int> per_tile;
            for (const OceanVec3& v : vertices) {
                const float w[3] = {
                    f32(static_cast<double>(body.position[0]) + body.row0[0] * v.x +
                        body.row1[0] * v.y + body.row2[0] * v.z),
                    f32(static_cast<double>(body.position[1]) + body.row0[1] * v.x +
                        body.row1[1] * v.y + body.row2[1] * v.z),
                    f32(static_cast<double>(body.position[2]) + body.row0[2] * v.x +
                        body.row1[2] * v.y + body.row2[2] * v.z)};
                const float inv = f32(1.0 / kTerrainCell);
                const float u = f32((static_cast<double>(w[0]) - t.node_x - t.origin_x) * inv);
                const float g = f32((static_cast<double>(w[2]) - t.node_z - t.origin_z) * inv);
                if (!(u >= 0.0f) || !(g >= 0.0f)) continue;
                const int i = static_cast<int>(u);
                const int j = static_cast<int>(g);
                int tx = i / kTileCells, tz = j / kTileCells;
                if (tx >= t.tiles_wide) tx = t.tiles_wide - 1;
                if (tz >= t.tiles_deep) tz = t.tiles_deep - 1;
                if (tx < 0 || tz < 0) continue;
                int& n = per_tile[{tx, tz}];
                if (n >= kCandidatesPerPair) continue;
                const float h = t.grid_height_00adb480(u, g);
                if (!(w[1] <= h)) continue;   // 00C54680..00C5468C: y <= h is a contact
                float normal[3]{};
                t.cell_normal_00adaa40(i, j, normal);
                ++n;
                ++out.candidates;
                ++census_.candidates;
                out.contact = true;
                const float depth = f32(static_cast<double>(h) - w[1]);
                if (depth > out.max_depth) out.max_depth = depth;
                if (!apply) continue;
                // The candidate: local on A (the terrain, identity: world), local on B (the
                // hull, body space), the normal from the terrain towards the hull.
                float cand[9] = {
                    f32(w[0] + static_cast<double>(depth) * normal[0]),
                    f32(w[1] + static_cast<double>(depth) * normal[1]),
                    f32(w[2] + static_cast<double>(depth) * normal[2]),
                    v.x, v.y, v.z, normal[0], normal[1], normal[2]};
                if (!dyn_contact_normal_is_unit(normal)) ++census_.rejected_normal;
                const Key key{unit, static_cast<int>(s), static_cast<int>(li), tx, tz};
                auto found = manifolds_.find(key);
                if (found == manifolds_.end()) {
                    found = manifolds_.emplace(key, std::make_unique<Manifold>()).first;
                }
                Manifold& m = *found->second;
                m.hull.set_pose(body.row0, body.row1, body.row2, body.position);
                insert_native_dyn_contact_00c3f760(m.bytes, cand);
            }
        }
    }
    if (out.contact) {
        ++census_.contact_steps;
        if (!touched_[unit]) { touched_[unit] = true; ++census_.units_touched; }
        if (out.max_depth > census_.max_depth) census_.max_depth = out.max_depth;
    }
    if (!apply) return out;

    // The solve (00403720) of the hull's group: rows in manifold-then-point order.
    std::vector<Manifold*> group;
    for (auto it = manifolds_.lower_bound(lo);
         it != manifolds_.end() && std::get<0>(it->first) == unit; ++it) {
        if (it->second->count() > 0) group.push_back(it->second.get());
    }
    if (group.empty()) return out;
    out.manifolds = static_cast<int>(group.size());
    const std::int32_t friction_base = static_cast<std::int32_t>(group.size()) * 4;
    std::vector<DynConstraintRow> rows(group.size() * 8);
    std::vector<DynSolverContactPoint*> row_points;
    DynSolverBodyInput terrain_in;   // static: solver index 0, zero mass and velocity
    DynSolverBodyInput hull_in;
    for (int k = 0; k < 3; ++k) {
        hull_in.row0[k] = body.row0[k];
        hull_in.row1[k] = body.row1[k];
        hull_in.row2[k] = body.row2[k];
        hull_in.position[k] = body.position[k];
    }
    const DynMotionState& ms = *body.motion;
    hull_in.linear_velocity[0] = ms.linear_velocity.x;
    hull_in.linear_velocity[1] = ms.linear_velocity.y;
    hull_in.linear_velocity[2] = ms.linear_velocity.z;
    hull_in.angular_velocity[0] = ms.angular_velocity.x;
    hull_in.angular_velocity[1] = ms.angular_velocity.y;
    hull_in.angular_velocity[2] = ms.angular_velocity.z;
    hull_in.inverse_mass = ms.inverse_mass;
    for (int k = 0; k < 9; ++k) hull_in.inverse_inertia[k] = ms.inverse_inertia_world[k];
    hull_in.solver_index = 1;
    const float friction = dyn_combine_friction(hull_friction, kTerrainFriction);
    const float restitution = dyn_combine_restitution(0.0f, kTerrainRestitution);
    const DynSolverWorldSettings settings{};   // 0.1, 1.0, 0.5, 10: the shipped world
    std::int32_t row = 0;
    for (Manifold* m : group) {
        for (std::int32_t p = 0; p < m->count(); ++p) {
            DynConstraintBuildInput in;
            in.body_a = terrain_in;
            in.body_b = hull_in;
            in.point = m->points()[p];
            in.friction = friction;
            in.restitution = restitution;
            dyn_build_contact_rows_00c4de40(in, settings, dt, rows[static_cast<std::size_t>(row)],
                rows[static_cast<std::size_t>(friction_base + row)]);
            row_points.push_back(&m->points()[p]);
            ++row;
        }
    }
    DynSolverBodyVelocity velocities[2]{};
    DynConstraintBatch batch;
    batch.rows = rows.data();
    batch.velocities = velocities;
    batch.velocity_count = 2;
    batch.normal_row_count = row;
    batch.friction_row_base = friction_base;
    dyn_apply_warm_start_00c42ba0(batch);
    dyn_solve_group_00403720(batch, settings.iterations);
    DynSolverVelocityWriteBack back[2]{};
    dyn_write_back_velocities_00c37b50(velocities, batch.velocity_count, back);
    dyn_store_impulses_00c35020(batch, row_points.data(), row);
    DynMotionState& mw = *body.motion;
    mw.linear_velocity.x += back[1].linear_velocity[0];
    mw.linear_velocity.y += back[1].linear_velocity[1];
    mw.linear_velocity.z += back[1].linear_velocity[2];
    mw.angular_velocity.x += back[1].angular_velocity[0];
    mw.angular_velocity.y += back[1].angular_velocity[1];
    mw.angular_velocity.z += back[1].angular_velocity[2];
    mw.linear_bias.x += back[1].linear_bias[0];
    mw.linear_bias.y += back[1].linear_bias[1];
    mw.linear_bias.z += back[1].linear_bias[2];
    mw.angular_bias.x += back[1].angular_bias[0];
    mw.angular_bias.y += back[1].angular_bias[1];
    mw.angular_bias.z += back[1].angular_bias[2];
    // Diagnostic, env-gated: BSP_HULL_TERRAIN_TRACE=<file> appends one line per solve.
    static std::FILE* trace = [] {
        char* path = nullptr;
        std::size_t length = 0;
        std::FILE* file = nullptr;
        if (_dupenv_s(&path, &length, "BSP_HULL_TERRAIN_TRACE") == 0 && path != nullptr) {
            if (fopen_s(&file, path, "a") != 0) file = nullptr;
        }
        std::free(path);
        return file;
    }();
    if (trace != nullptr) {
        std::fprintf(trace, "unit=%zu pos=(%.2f %.2f %.2f) v0=(%.3f %.3f %.3f) w0=(%.4f %.4f "
            "%.4f) dv=(%.3f %.3f %.3f) dw=(%.4f %.4f %.4f) bias=(%.3f %.3f %.3f) rows=%d "
            "imass=%.3g iI=(%.3g %.3g %.3g) depth0=%.2f n0=(%.3f %.3f %.3f)\n", unit,
            body.position[0], body.position[1], body.position[2], hull_in.linear_velocity[0],
            hull_in.linear_velocity[1], hull_in.linear_velocity[2], hull_in.angular_velocity[0],
            hull_in.angular_velocity[1], hull_in.angular_velocity[2],
            back[1].linear_velocity[0], back[1].linear_velocity[1], back[1].linear_velocity[2],
            back[1].angular_velocity[0], back[1].angular_velocity[1],
            back[1].angular_velocity[2], back[1].linear_bias[0], back[1].linear_bias[1],
            back[1].linear_bias[2], row, hull_in.inverse_mass, hull_in.inverse_inertia[0],
            hull_in.inverse_inertia[4], hull_in.inverse_inertia[8], row_points[0]->depth,
            row_points[0]->normal[0], row_points[0]->normal[1], row_points[0]->normal[2]);
    }
    out.points = row;
    out.delta_linear =OceanVec3{back[1].linear_velocity[0], back[1].linear_velocity[1],
                                 back[1].linear_velocity[2]};
    out.delta_linear_bias = OceanVec3{back[1].linear_bias[0], back[1].linear_bias[1],
                                      back[1].linear_bias[2]};
    for (std::int32_t k = 0; k < row; ++k) {
        out.normal_impulse += rows[static_cast<std::size_t>(k)].impulse;
        out.bias_impulse += rows[static_cast<std::size_t>(k)].impulse_bias;
    }
    ++census_.solves;
    census_.rows += static_cast<unsigned long long>(row);
    return out;
}

}  // namespace bsp
