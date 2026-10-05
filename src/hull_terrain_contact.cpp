#include "bsp/hull_terrain_contact.hpp"

#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "bsp/avoid_zone_dyn_hull.hpp"
#include "bsp/dyn_collision_pass.hpp"
#include "bsp/dyn_body_creation.hpp"
#include "bsp/dyn_contact_solver.hpp"
#include "bsp/game_native_dyn_process.hpp"
#include "bsp/game_hosts_scene_contents.hpp"
#include "bsp/native_dyn_collision_pass.hpp"
#include "bsp/native_dyn_general_convex.hpp"
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

// A hull body's shape chain as 00C44090 walks it (B+70h, next +208h): 00C5C940 prepends,
// so under kHullShapeChainOrderBound the last shape comes first.
std::vector<std::size_t> shape_chain_order(std::size_t count) {
    std::vector<std::size_t> order(count);
    for (std::size_t k = 0; k < count; ++k) {
        order[k] = kHullShapeChainOrderBound ? count - 1 - k : k;
    }
    return order;
}

}  // namespace

// The native manifold: points at +08h (30h each, four), count +0C8h, body A +0CCh, body B
// +0D0h (docs/NATIVE_DYN_NARROW_PHASE_R138.md). Body A is the terrain tile's static body (the
// identity frame, so its local point is the world point); body B the hull.
// A hull-hull manifold (kHullHullContactBound) uses the same record: `terrain` then holds
// body A's pose (the lower unit) and `hull` body B's.
struct HullTerrainContactSolver::Manifold {
    alignas(16) std::uint8_t bytes[0x100]{};
    NativeBody terrain;
    NativeBody hull;
    static constexpr std::size_t kStatic = static_cast<std::size_t>(-1);
    std::size_t unit_a{kStatic};   // kStatic: the terrain tile's static body
    std::size_t unit_b{kStatic};
    unsigned long long serial{0};  // creation order: the scene's manifold list (LABELLED)
    float friction{0.0f};          // manifold +0h, written by 00C44090 on every hit
    float restitution{0.0f};       // manifold +4h
    // Packet cc9_hull_fort_contact: a fort manifold's static body A has the fort's frame
    // (rows, translation), not the terrain's identity; the row build reads it.
    bool static_frame{false};
    float frame_a[12]{};
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
    for (auto it = pair_manifolds_.begin(); it != pair_manifolds_.end();) {
        if (it->first.first == unit || it->first.second == unit) it = pair_manifolds_.erase(it);
        else ++it;
    }
    for (auto it = fort_manifolds_.begin(); it != fort_manifolds_.end();) {
        if (it->first.first == unit || it->first.second == unit) it = fort_manifolds_.erase(it);
        else ++it;
    }
    fort_boxes_.erase(unit);
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
    h.handle.reset();
    h.convex_ready = false;
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
        static const AvoidZoneDynHullMemory memory{
            nullptr, [](void*, std::size_t bytes) -> void* { return std::malloc(bytes); },
            [](void*, void* p) { std::free(p); }};
        // Kept for the convex record (+210h) the hull pairs read; destroyed with the shape.
        h.handle = std::shared_ptr<AvoidZoneDynHullHandle>(new AvoidZoneDynHullHandle{},
            [](AvoidZoneDynHullHandle* p) {
                avoid_zone_dyn_hull_destroy_00c37450(*p, memory);
                delete p;
            });
        AvoidZoneDynHullHandle& handle = *h.handle;
        avoid_zone_dyn_hull_construct_00c5df30(handle, centred.data(),
                                               static_cast<std::uint32_t>(centred.size()), memory);
        const std::uint32_t n = avoid_zone_dyn_hull_vertex_count_00c32d20(handle);
        for (std::uint32_t i = 0; i < n; ++i) local.push_back(handle.data->vertices[i].point);
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
                for (const std::size_t s : shape_chain_order(hulls.size())) {
                    alignas(16) std::uint8_t result[4 + 8 * 36]{};
                    const bool hit = intersect_native_dyn_terrain_convex_00c53630(
                        result, tile.shape, tile.frame, hulls[s]->shape, hull_frame, crt);
                    // Diagnostic, env-gated: BSP_HULL_TERRAIN_COMPARE=<file> logs the vertices
                    // where the host test (00ADB480 height) and 00C53630 disagree (first 400).
                    static std::FILE* compare = [] {
                        char* path = nullptr;
                        std::size_t length = 0;
                        std::FILE* f = nullptr;
                        if (_dupenv_s(&path, &length, "BSP_HULL_TERRAIN_COMPARE") == 0 && path) {
                            fopen_s(&f, path, "w");
                            std::free(path);
                        }
                        return f;
                    }();
                    static int compare_lines = 0;
                    if (compare && compare_lines < 400) {
                        std::int32_t hits = 0;
                        std::memcpy(&hits, result, 4);
                        const float* rc = reinterpret_cast<const float*>(result + 4);
                        for (const OceanVec3& v : hulls[s]->vertices) {
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
                            if (!(u >= tx * 32.0f && u <= tx * 32.0f + 32.0f && g >= tz * 32.0f &&
                                  g <= tz * 32.0f + 32.0f)) {
                                continue;
                            }
                            const float h = t.grid_height_00adb480(u, g);
                            const bool host = w[1] <= h;
                            bool native = false;
                            for (std::int32_t k = 0; k < hits; ++k) {
                                const float* q = rc + 9 * k;
                                if (std::fabs(q[3] - v.x) < 1e-3f && std::fabs(q[4] - v.y) < 1e-3f &&
                                    std::fabs(q[5] - v.z) < 1e-3f) native = true;
                            }
                            if (host != native) {
                                std::fprintf(compare, "step=%llu unit=%zu s=%zu tile=%d,%d n=%d w=%.3f,%.3f,%.3f "
                                    "u=%.4f g=%.4f host_h=%.3f host=%d native=%d\n", census_.steps, unit, s,
                                    tx, tz, hits, w[0], w[1], w[2], u, g, h, host ? 1 : 0, native ? 1 : 0);
                                ++compare_lines;
                            }
                        }
                        if (compare_lines >= 400) std::fflush(compare);
                    }
                    if (!hit) continue;
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
                            found->second->unit_b = unit;
                            found->second->serial = ++manifold_serial_;
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
                    found->second->unit_b = unit;
                    found->second->serial = ++manifold_serial_;
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


// ---------------------------------------------------------------------------
// Packet cc9_hull_hull_contact (docs/GUNNERY_OPEN_ITEMS.md section 91): the world phase.
// ---------------------------------------------------------------------------

namespace {

DynSolverBodyInput dynamic_body_input(const DynBody& body, std::int16_t index) {
    DynSolverBodyInput in;
    for (int k = 0; k < 3; ++k) {
        in.row0[k] = body.row0[k];
        in.row1[k] = body.row1[k];
        in.row2[k] = body.row2[k];
        in.position[k] = body.position[k];
    }
    const DynMotionState& ms = *body.motion;
    in.linear_velocity[0] = ms.linear_velocity.x;
    in.linear_velocity[1] = ms.linear_velocity.y;
    in.linear_velocity[2] = ms.linear_velocity.z;
    in.angular_velocity[0] = ms.angular_velocity.x;
    in.angular_velocity[1] = ms.angular_velocity.y;
    in.angular_velocity[2] = ms.angular_velocity.z;
    in.inverse_mass = ms.inverse_mass;
    for (int k = 0; k < 9; ++k) in.inverse_inertia[k] = ms.inverse_inertia_world[k];
    in.solver_index = index;
    return in;
}

void body_frame(const DynBody& body, float out[12]) {
    for (int k = 0; k < 3; ++k) {
        out[k] = body.row0[k];
        out[3 + k] = body.row1[k];
        out[6 + k] = body.row2[k];
        out[9 + k] = body.position[k];
    }
}

}  // namespace

// The kind-4 record 00C57F50 builds for a hull shape, for 00C535E0: +0 the process's
// ConvexMeshShape table (its double-support slot +0Ch is 00C385B0 over the borrowed mesh),
// +4 the body (a copy of the hull's current 3x4 at +08h..+37h, which 00C51C20 and 00C48BE0
// read; 00C57C40's body refresh is not run), +8 kind 4, +0Ch the local box 00C57C40 writes,
// +24h restitution 0 (0093944D writes no shape restitution; section 84), +28h the material
// friction, +2Ch group 1, +30h mask 0Dh, +34h the shape frame (identity, the centre as
// translation, 006FAEA0), +210h the 00C5DEB0 hull.
HullTerrainContactSolver::HullShape* HullTerrainContactSolver::hull_convex(
    std::size_t unit, std::size_t shape, const std::vector<OceanVec3>& raw, float friction,
    const DynBody& body) {
    HullShape& h = hull_shape(unit, shape, raw);
    if (!h.handle || !h.handle->data || h.handle->data->vertex_count == 0) return nullptr;
    if (!h.convex_ready) {
        game::GameNativeDynProcess& process =
            game::game_native_dyn_process(game::application_camera_axes_crt());
        std::memset(h.convex, 0, sizeof(h.convex));
        const void* table = process.body_creation().convex_shape_vtable;
        const std::uint32_t kind = 4, group = 1, mask = 0x0D;
        const float restitution = 0.0f;
        const void* mesh = h.handle->data;
        const float frame[12] = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
                                 h.centre[0], h.centre[1], h.centre[2]};
        std::memcpy(h.convex + 0x00, &table, sizeof(table));
        std::memcpy(h.convex + 0x08, &kind, 4);
        std::memcpy(h.convex + 0x24, &restitution, 4);
        std::memcpy(h.convex + 0x28, &friction, 4);
        std::memcpy(h.convex + 0x2C, &group, 4);
        std::memcpy(h.convex + 0x30, &mask, 4);
        std::memcpy(h.convex + 0x34, frame, sizeof(frame));
        std::memcpy(h.convex + 0x210, &mesh, sizeof(mesh));
        dyn_convex_shape_local_bounds_00c57c40(*reinterpret_cast<DynConvexShapeStorage*>(h.convex));
        h.convex_ready = true;
    }
    std::memcpy(h.convex + 0x28, &friction, 4);
    std::memcpy(h.body + 0x08, body.row0, 12);
    std::memcpy(h.body + 0x14, body.row1, 12);
    std::memcpy(h.body + 0x20, body.row2, 12);
    std::memcpy(h.body + 0x2C, body.position, 12);
    const void* owner = h.body;
    std::memcpy(h.convex + 0x04, &owner, sizeof(owner));
    return &h;
}

// 00C44090 for every hull pair the broad phase would hand it. LABELLED substitutions: the
// SAP pair list is replaced by a world-box test (every hull vertex in world space, widened
// by 0.1 m, more than the 0.02 the image widens each shape box by, so no pair the SAP holds
// is missed; the narrow phase decides every hit); body A is the lower unit; the shape pairs
// go in shape order (the image walks each body's shape chain +70h / +208h).
void HullTerrainContactSolver::hull_hull_narrow_phase(std::vector<HullWorldEntry>& hulls,
                                                      bool apply) {
    constexpr std::uint32_t kHullGroup = 1, kHullMaskBase = 0x0D;
    if (!dyn_shapes_overlap_filter(kHullGroup, kHullMaskBase, kHullGroup, kHullMaskBase)) return;
    struct Box { float lo[3], hi[3]; bool ok; };
    std::vector<Box> boxes(hulls.size());
    for (std::size_t i = 0; i < hulls.size(); ++i) {
        Box& b = boxes[i];
        b.ok = false;
        for (int k = 0; k < 3; ++k) { b.lo[k] = 3.402823466e+38f; b.hi[k] = -3.402823466e+38f; }
        const DynBody& body = *hulls[i].body;
        for (std::size_t s = 0; s < hulls[i].shapes->size(); ++s) {
            for (const OceanVec3& v : hull_shape(hulls[i].unit, s, (*hulls[i].shapes)[s]).vertices) {
                for (int k = 0; k < 3; ++k) {
                    const float w = f32(static_cast<double>(body.position[k]) + body.row0[k] * v.x +
                                        body.row1[k] * v.y + body.row2[k] * v.z);
                    if (w < b.lo[k]) b.lo[k] = w;
                    if (w > b.hi[k]) b.hi[k] = w;
                    b.ok = true;
                }
            }
        }
    }
    game::GameNativeDynProcess& process =
        game::game_native_dyn_process(game::application_camera_axes_crt());
    DynGeneralConvexIntersectStorage& owner = process.general_convex_owner();
    const CameraAxesCrtAccess& crt = game::application_camera_axes_crt();
    // Diagnostic, env-gated: BSP_HULL_HULL_TRACE=<file> logs a self-test of 00C535E0 on the
    // first hull shape (against itself, shifted 1 m and 1000 m along x) and the first 300
    // near pairs with their boxes and hits.
    static std::FILE* trace = [] {
        char* path = nullptr;
        std::size_t length = 0;
        std::FILE* f = nullptr;
        if (_dupenv_s(&path, &length, "BSP_HULL_HULL_TRACE") == 0 && path) {
            fopen_s(&f, path, "w");
            std::free(path);
        }
        return f;
    }();
    static int trace_lines = 0;
    static bool self_tested = false;
    if (trace && !self_tested && !hulls.empty() && !hulls[0].shapes->empty()) {
        HullShape* h = hull_convex(hulls[0].unit, 0, (*hulls[0].shapes)[0], hulls[0].friction,
                                  *hulls[0].body);
        if (h != nullptr) {
            self_tested = true;
            float fa[12];
            body_frame(*hulls[0].body, fa);
            for (const float shift : {0.0f, 1.0f, 1000.0f}) {
                float fb[12];
                std::memcpy(fb, fa, sizeof(fb));
                fb[9] += shift;
                alignas(16) std::uint8_t other[0x240];
                alignas(16) std::uint8_t other_body[0x40];
                std::memcpy(other, h->convex, sizeof(other));
                std::memcpy(other_body, h->body, sizeof(other_body));
                std::memcpy(other_body + 0x2C, fb + 9, 4);
                const void* ob = other_body;
                std::memcpy(other + 0x04, &ob, sizeof(ob));
                alignas(16) std::uint8_t result[4 + 8 * 36]{};
                const bool hit = dispatch_native_dyn_general_convex_00c535e0(owner, result,
                    h->convex, fa, other, fb, crt);
                std::int32_t n = 0;
                std::memcpy(&n, result, 4);
                const float* c = reinterpret_cast<const float*>(result + 4);
                std::fprintf(trace, "selftest unit=%zu shift=%.1f hit=%d n=%d c=(%.3f %.3f %.3f | "
                    "%.3f %.3f %.3f | %.3f %.3f %.3f) box=(%.2f %.2f %.2f .. %.2f %.2f %.2f) "
                    "verts=%u\n", hulls[0].unit, shift, hit ? 1 : 0, n, c[0], c[1], c[2], c[3],
                    c[4], c[5], c[6], c[7], c[8],
                    *reinterpret_cast<const float*>(h->convex + 0x0C),
                    *reinterpret_cast<const float*>(h->convex + 0x10),
                    *reinterpret_cast<const float*>(h->convex + 0x14),
                    *reinterpret_cast<const float*>(h->convex + 0x18),
                    *reinterpret_cast<const float*>(h->convex + 0x1C),
                    *reinterpret_cast<const float*>(h->convex + 0x20),
                    h->handle->data->vertex_count);
            }
            std::fflush(trace);
        }
    }
    for (std::size_t i = 0; i < hulls.size(); ++i) {
        for (std::size_t j = i + 1; j < hulls.size(); ++j) {
            const Box& a = boxes[i];
            const Box& b = boxes[j];
            if (!a.ok || !b.ok) continue;
            bool overlap = true;
            for (int k = 0; k < 3 && overlap; ++k) {
                overlap = a.lo[k] - 0.1f <= b.hi[k] && b.lo[k] - 0.1f <= a.hi[k];
            }
            if (!overlap) continue;
            ++census_.hull_pairs_near;
            HullWorldEntry& ea = hulls[i];
            HullWorldEntry& eb = hulls[j];
            float frame_a[12], frame_b[12];
            body_frame(*ea.body, frame_a);
            body_frame(*eb.body, frame_b);
            int pair_hits = 0;
            float pair_depth = -3.402823466e+38f;
            for (const std::size_t sa : shape_chain_order(ea.shapes->size())) {
                HullShape* ha = hull_convex(ea.unit, sa, (*ea.shapes)[sa], ea.friction, *ea.body);
                if (ha == nullptr) continue;
                for (const std::size_t sb : shape_chain_order(eb.shapes->size())) {
                    HullShape* hb = hull_convex(eb.unit, sb, (*eb.shapes)[sb], eb.friction,
                                                 *eb.body);
                    if (hb == nullptr) continue;
                    ++census_.hull_shape_tests;
                    alignas(16) std::uint8_t result[4 + 8 * 36]{};
                    if (!dispatch_native_dyn_general_convex_00c535e0(owner, result, ha->convex,
                            frame_a, hb->convex, frame_b, crt)) {
                        continue;
                    }
                    std::int32_t n = 0;
                    std::memcpy(&n, result, 4);
                    const float* c = reinterpret_cast<const float*>(result + 4);
                    for (std::int32_t k = 0; k < n; ++k, c += 9) {
                        ++census_.hull_hits;
                        ++pair_hits;
                        ++ea.result.hull_candidates;
                        ++eb.result.hull_candidates;
                        // Witnesses are body-local (C48BE0); the depth is along the normal.
                        double wa[3], wb[3];
                        for (int q = 0; q < 3; ++q) {
                            wa[q] = static_cast<double>(frame_a[9 + q]) + frame_a[q] * c[0] +
                                    frame_a[3 + q] * c[1] + frame_a[6 + q] * c[2];
                            wb[q] = static_cast<double>(frame_b[9 + q]) + frame_b[q] * c[3] +
                                    frame_b[3 + q] * c[4] + frame_b[6 + q] * c[5];
                        }
                        const float depth = f32((wa[0] - wb[0]) * c[6] + (wa[1] - wb[1]) * c[7] +
                                                (wa[2] - wb[2]) * c[8]);
                        if (depth > pair_depth) pair_depth = depth;
                        if (!apply) continue;
                        // 00C44154..00C441DB: the combines, FindOrCreate(A, B) at 00C441C5,
                        // then friction and restitution into manifold +0h / +4h on every hit.
                        auto& slot = pair_manifolds_[{ea.unit, eb.unit}];
                        if (!slot) {
                            slot = std::make_unique<Manifold>();
                            slot->unit_a = ea.unit;
                            slot->unit_b = eb.unit;
                            slot->serial = ++manifold_serial_;
                        }
                        Manifold& m = *slot;
                        m.friction = dyn_combine_friction(ea.friction, eb.friction);
                        m.restitution = dyn_combine_restitution(0.0f, 0.0f);
                        m.terrain.set_pose(ea.body->row0, ea.body->row1, ea.body->row2,
                                           ea.body->position);
                        m.hull.set_pose(eb.body->row0, eb.body->row1, eb.body->row2,
                                        eb.body->position);
                        if (!dyn_contact_normal_is_unit(c + 6)) ++census_.rejected_normal;
                        float in[9];
                        std::memcpy(in, c, sizeof(in));
                        insert_native_dyn_contact_00c3f760(m.bytes, in);
                    }
                    // 00C4420D..00C44301: one event {manifold, shape A, shape B} per
                    // dispatcher hit, queued when a listener mask (hull: 7FF9h, 00939CD5)
                    // meets the other shape's group (1).
                    if (apply && n > 0) {
                        pending_events_.push_back(
                            {ea.unit, eb.unit, pair_manifolds_[{ea.unit, eb.unit}].get()});
                    }
                }
            }
            // After the first 50 lines only hits deeper than 3 m, with both velocities; with
            // BSP_HULL_HULL_TRACE_UNIT=<unit> every near line of that unit instead, from the
            // world step BSP_HULL_HULL_TRACE_STEP on.
            static const long trace_unit = [] {
                char* v = nullptr;
                std::size_t length = 0;
                long u = -1;
                if (_dupenv_s(&v, &length, "BSP_HULL_HULL_TRACE_UNIT") == 0 && v) {
                    u = std::strtol(v, nullptr, 10);
                    std::free(v);
                }
                return u;
            }();
            static const unsigned long long trace_step = [] {
                char* v = nullptr;
                std::size_t length = 0;
                unsigned long long n = 0;
                if (_dupenv_s(&v, &length, "BSP_HULL_HULL_TRACE_STEP") == 0 && v) {
                    n = std::strtoull(v, nullptr, 10);
                    std::free(v);
                }
                return n;
            }();
            const bool unit_line = trace_unit >= 0 && census_.world_steps >= trace_step &&
                (ea.unit == static_cast<std::size_t>(trace_unit) ||
                 eb.unit == static_cast<std::size_t>(trace_unit));
            if (trace && trace_lines < 400 &&
                (trace_unit >= 0 ? unit_line
                                 : (trace_lines < 50 || (pair_hits > 0 && pair_depth > 3.0f)))) {
                ++trace_lines;
                const DynMotionState* ma = ea.body->motion;
                const DynMotionState* mb = eb.body->motion;
                std::fprintf(trace, "near step=%llu a=%zu b=%zu hits=%d depth=%.3f "
                    "A=(%.1f %.1f %.1f .. %.1f %.1f %.1f) B=(%.1f %.1f %.1f .. %.1f %.1f %.1f) "
                    "vA=(%.2f %.2f %.2f) vB=(%.2f %.2f %.2f) imA=%.3g imB=%.3g\n",
                    census_.world_steps, ea.unit, eb.unit, pair_hits,
                    pair_hits > 0 ? pair_depth : 0.0f, a.lo[0], a.lo[1], a.lo[2], a.hi[0],
                    a.hi[1], a.hi[2], b.lo[0], b.lo[1], b.lo[2], b.hi[0], b.hi[1], b.hi[2],
                    ma ? ma->linear_velocity.x : 0.0f, ma ? ma->linear_velocity.y : 0.0f,
                    ma ? ma->linear_velocity.z : 0.0f, mb ? mb->linear_velocity.x : 0.0f,
                    mb ? mb->linear_velocity.y : 0.0f, mb ? mb->linear_velocity.z : 0.0f,
                    ma ? ma->inverse_mass : 0.0f, mb ? mb->inverse_mass : 0.0f);
                std::fflush(trace);
            }
            if (pair_hits > 0) {
                HullPairCensus& pc = hull_pairs_[{ea.unit, eb.unit}];
                if (pc.steps == 0) pc.first_step = census_.world_steps;
                ++pc.steps;
                if (pair_depth > pc.max_depth) pc.max_depth = pair_depth;
            }
        }
    }
}

void HullTerrainContactSolver::hull_hull_census(std::vector<HullWorldEntry>& hulls) {
    ++census_.world_steps;
    hull_hull_narrow_phase(hulls, false);
}

// Packet cc9_hull_fort_contact. The record 00C57F50 builds from one of 007482B0's descriptors:
// as hull_convex, but friction 1.0 ([00D7A24C] at 007486DD) and the local frame identity with
// a ZERO translation (the template's +18h..+44h at 00748626..00748689, which the loop never
// overwrites), so the 00C5DEB0 hull of the re-centred points sits on the fort origin. +4 is
// the fort body's 3x4, fixed at creation.
HullTerrainContactSolver::HullShape* HullTerrainContactSolver::fort_convex(
    std::size_t unit, std::size_t shape, const std::vector<OceanVec3>& raw,
    const float frame[12]) {
    HullShape& h = hull_shape(unit, shape, raw);
    if (!h.handle || !h.handle->data || h.handle->data->vertex_count == 0) return nullptr;
    if (!h.convex_ready) {
        game::GameNativeDynProcess& process =
            game::game_native_dyn_process(game::application_camera_axes_crt());
        std::memset(h.convex, 0, sizeof(h.convex));
        const void* table = process.body_creation().convex_shape_vtable;
        const std::uint32_t kind = 4, group = 1, mask = 0x0D;
        const float restitution = 0.0f, friction = 1.0f;
        const void* mesh = h.handle->data;
        const float local[12] = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
                                 0.0f, 0.0f, 0.0f};
        std::memcpy(h.convex + 0x00, &table, sizeof(table));
        std::memcpy(h.convex + 0x08, &kind, 4);
        std::memcpy(h.convex + 0x24, &restitution, 4);
        std::memcpy(h.convex + 0x28, &friction, 4);
        std::memcpy(h.convex + 0x2C, &group, 4);
        std::memcpy(h.convex + 0x30, &mask, 4);
        std::memcpy(h.convex + 0x34, local, sizeof(local));
        std::memcpy(h.convex + 0x210, &mesh, sizeof(mesh));
        dyn_convex_shape_local_bounds_00c57c40(*reinterpret_cast<DynConvexShapeStorage*>(h.convex));
        std::memcpy(h.body + 0x08, frame + 0, 12);
        std::memcpy(h.body + 0x14, frame + 3, 12);
        std::memcpy(h.body + 0x20, frame + 6, 12);
        std::memcpy(h.body + 0x2C, frame + 9, 12);
        const void* owner = h.body;
        std::memcpy(h.convex + 0x04, &owner, sizeof(owner));
        h.convex_ready = true;
    }
    return &h;
}

// 00C44090 for every (fort, hull) pair the broad phase would hand it, with the same LABELLED
// substitutions as the hull pairs (a world-box test widened by 0.1 m; shape order) and body A
// taken as the fort (the terrain's place: the SAP pair order is not read). No contact event
// is queued: 009377E0 finds no `other` for a fort (section 92, not IsKindOf(6)), and the
// fort's own listener is not read.
void HullTerrainContactSolver::hull_fort_narrow_phase(std::vector<HullWorldEntry>& hulls,
                                                      const std::vector<FortWorldEntry>& forts,
                                                      bool apply) {
    constexpr std::uint32_t kGroup = 1, kMask = 0x0D;
    if (!dyn_shapes_overlap_filter(kGroup, kMask, kGroup, kMask)) return;
    if (hulls.empty() || forts.empty()) return;
    const auto world_box = [](const float f[12], const std::vector<OceanVec3>& local,
                              FortBox& b) {
        for (const OceanVec3& v : local) {
            for (int k = 0; k < 3; ++k) {
                const float w = f32(static_cast<double>(f[9 + k]) + f[k] * v.x +
                                    f[3 + k] * v.y + f[6 + k] * v.z);
                if (w < b.lo[k]) b.lo[k] = w;
                if (w > b.hi[k]) b.hi[k] = w;
                b.ok = true;
            }
        }
    };
    const auto empty_box = [] {
        FortBox b;
        b.ok = false;
        for (int k = 0; k < 3; ++k) { b.lo[k] = 3.402823466e+38f; b.hi[k] = -3.402823466e+38f; }
        return b;
    };
    std::vector<FortBox> hull_boxes(hulls.size());
    std::vector<std::array<float, 12>> hull_frames(hulls.size());
    for (std::size_t i = 0; i < hulls.size(); ++i) {
        hull_boxes[i] = empty_box();
        body_frame(*hulls[i].body, hull_frames[i].data());
        for (std::size_t s = 0; s < hulls[i].shapes->size(); ++s) {
            world_box(hull_frames[i].data(),
                      hull_shape(hulls[i].unit, s, (*hulls[i].shapes)[s]).vertices,
                      hull_boxes[i]);
        }
    }
    game::GameNativeDynProcess& process =
        game::game_native_dyn_process(game::application_camera_axes_crt());
    DynGeneralConvexIntersectStorage& owner = process.general_convex_owner();
    const CameraAxesCrtAccess& crt = game::application_camera_axes_crt();
    for (const FortWorldEntry& fort : forts) {
        if (fort.shapes == nullptr) continue;
        auto fb = fort_boxes_.find(fort.unit);
        if (fb == fort_boxes_.end()) {
            FortBox box = empty_box();
            for (std::size_t s = 0; s < fort.shapes->size(); ++s) {
                HullShape& h = hull_shape(fort.unit, s, (*fort.shapes)[s]);
                std::vector<OceanVec3> local;
                local.reserve(h.vertices.size());
                for (const OceanVec3& v : h.vertices) {
                    local.push_back(OceanVec3{f32(static_cast<double>(v.x) - h.centre[0]),
                                              f32(static_cast<double>(v.y) - h.centre[1]),
                                              f32(static_cast<double>(v.z) - h.centre[2])});
                }
                world_box(fort.frame, local, box);
            }
            fb = fort_boxes_.emplace(fort.unit, box).first;
            if (!forts_seen_[fort.unit]) {
                forts_seen_[fort.unit] = true;
                ++census_.forts;
                census_.fort_shapes += fort.shapes->size();
                if (fort.shapes->size() > 0x1E) ++census_.forts_over_reserve;
            }
        }
        const FortBox& a = fb->second;
        if (!a.ok) continue;
        for (std::size_t j = 0; j < hulls.size(); ++j) {
            const FortBox& b = hull_boxes[j];
            if (!b.ok) continue;
            bool overlap = true;
            for (int k = 0; k < 3 && overlap; ++k) {
                overlap = a.lo[k] - 0.1f <= b.hi[k] && b.lo[k] - 0.1f <= a.hi[k];
            }
            if (!overlap) continue;
            ++census_.fort_pairs_near;
            HullWorldEntry& eb = hulls[j];
            const float* frame_b = hull_frames[j].data();
            int pair_hits = 0;
            float pair_depth = -3.402823466e+38f;
            // 007482B0 builds the fort through 00C5D580 as well, so its chain is reversed too.
            for (const std::size_t sa : shape_chain_order(fort.shapes->size())) {
                HullShape* ha = fort_convex(fort.unit, sa, (*fort.shapes)[sa], fort.frame);
                if (ha == nullptr) continue;
                for (const std::size_t sb : shape_chain_order(eb.shapes->size())) {
                    HullShape* hb = hull_convex(eb.unit, sb, (*eb.shapes)[sb], eb.friction,
                                                *eb.body);
                    if (hb == nullptr) continue;
                    ++census_.fort_shape_tests;
                    alignas(16) std::uint8_t result[4 + 8 * 36]{};
                    if (!dispatch_native_dyn_general_convex_00c535e0(owner, result, ha->convex,
                            fort.frame, hb->convex, frame_b, crt)) {
                        continue;
                    }
                    std::int32_t n = 0;
                    std::memcpy(&n, result, 4);
                    const float* c = reinterpret_cast<const float*>(result + 4);
                    for (std::int32_t k = 0; k < n; ++k, c += 9) {
                        ++census_.fort_hits;
                        ++pair_hits;
                        double wa[3], wb[3];
                        for (int q = 0; q < 3; ++q) {
                            wa[q] = static_cast<double>(fort.frame[9 + q]) + fort.frame[q] * c[0] +
                                    fort.frame[3 + q] * c[1] + fort.frame[6 + q] * c[2];
                            wb[q] = static_cast<double>(frame_b[9 + q]) + frame_b[q] * c[3] +
                                    frame_b[3 + q] * c[4] + frame_b[6 + q] * c[5];
                        }
                        const float depth = f32((wa[0] - wb[0]) * c[6] + (wa[1] - wb[1]) * c[7] +
                                                (wa[2] - wb[2]) * c[8]);
                        if (depth > pair_depth) pair_depth = depth;
                        if (!apply) continue;
                        auto& slot = fort_manifolds_[{fort.unit, eb.unit}];
                        if (!slot) {
                            slot = std::make_unique<Manifold>();
                            slot->unit_a = Manifold::kStatic;
                            slot->unit_b = eb.unit;
                            slot->serial = ++manifold_serial_;
                            slot->static_frame = true;
                            std::memcpy(slot->frame_a, fort.frame, sizeof(slot->frame_a));
                            slot->terrain.set_pose(fort.frame + 0, fort.frame + 3,
                                                   fort.frame + 6, fort.frame + 9);
                        }
                        Manifold& m = *slot;
                        m.friction = dyn_combine_friction(1.0f, eb.friction);
                        m.restitution = dyn_combine_restitution(0.0f, 0.0f);
                        m.hull.set_pose(eb.body->row0, eb.body->row1, eb.body->row2,
                                        eb.body->position);
                        if (!dyn_contact_normal_is_unit(c + 6)) ++census_.rejected_normal;
                        float in[9];
                        std::memcpy(in, c, sizeof(in));
                        insert_native_dyn_contact_00c3f760(m.bytes, in);
                    }
                }
            }
            if (pair_hits > 0) {
                ++census_.fort_hit_steps;
                if (pair_depth > census_.fort_max_depth) census_.fort_max_depth = pair_depth;
                HullPairCensus& pc = fort_pairs_[{fort.unit, eb.unit}];
                if (pc.steps == 0) pc.first_step = census_.world_steps;
                ++pc.steps;
                if (pair_depth > pc.max_depth) pc.max_depth = pair_depth;
            }
        }
    }
}

void HullTerrainContactSolver::world_step(std::vector<HullWorldEntry>& hulls, float dt,
                                          bool hull_hull) {
    world_step(hulls, {}, dt, hull_hull, false);
}

void HullTerrainContactSolver::world_step(std::vector<HullWorldEntry>& hulls,
                                          const std::vector<FortWorldEntry>& forts, float dt,
                                          bool hull_hull, bool hull_fort) {
    ++census_.world_steps;
    std::map<std::size_t, HullWorldEntry*> by_unit;
    for (HullWorldEntry& e : hulls) {
        e.result = HullTerrainContactStepResult{};
        by_unit[e.unit] = &e;
    }
    // ManifoldUpdate (00C549D0) over every manifold, both bodies at their current poses; a
    // manifold whose hull is not stepping this step, or left with no point, is retired.
    for (auto it = manifolds_.begin(); it != manifolds_.end();) {
        const auto found = by_unit.find(std::get<0>(it->first));
        Manifold& m = *it->second;
        if (found != by_unit.end()) {
            const DynBody& body = *found->second->body;
            m.hull.set_pose(body.row0, body.row1, body.row2, body.position);
            refresh_native_dyn_manifold_00c4b9b0(m.bytes);
        }
        // kSunkHullTerrainMaskBound: a hull whose shapes lost mask bit 8 fails 00C44104's
        // filter against the terrain, so its terrain manifolds go with the pair.
        const bool filtered = found != by_unit.end() && found->second->terrain_mask_cleared;
        if (found == by_unit.end() || filtered || m.count() <= 0) {
            ++census_.retired;
            it = manifolds_.erase(it);
        } else {
            ++it;
        }
    }
    for (auto it = pair_manifolds_.begin(); it != pair_manifolds_.end();) {
        const auto fa = by_unit.find(it->first.first);
        const auto fb = by_unit.find(it->first.second);
        Manifold& m = *it->second;
        const bool live = fa != by_unit.end() && fb != by_unit.end();
        if (live) {
            const DynBody& a = *fa->second->body;
            const DynBody& b = *fb->second->body;
            m.terrain.set_pose(a.row0, a.row1, a.row2, a.position);
            m.hull.set_pose(b.row0, b.row1, b.row2, b.position);
            refresh_native_dyn_manifold_00c4b9b0(m.bytes);
        }
        if (!live || m.count() <= 0) {
            ++census_.retired;
            it = pair_manifolds_.erase(it);
        } else {
            ++it;
        }
    }
    // Packet cc9_hull_fort_contact: the fort manifolds (fort, hull); the fort's pose is fixed.
    std::map<std::size_t, bool> fort_live;
    for (const FortWorldEntry& f : forts) fort_live[f.unit] = true;
    for (auto it = fort_manifolds_.begin(); it != fort_manifolds_.end();) {
        const auto fh = by_unit.find(it->first.second);
        Manifold& m = *it->second;
        const bool live = fh != by_unit.end() && fort_live.count(it->first.first) != 0;
        if (live) {
            const DynBody& b = *fh->second->body;
            m.hull.set_pose(b.row0, b.row1, b.row2, b.position);
            refresh_native_dyn_manifold_00c4b9b0(m.bytes);
        }
        if (!live || m.count() <= 0) {
            ++census_.retired;
            it = fort_manifolds_.erase(it);
        } else {
            ++it;
        }
    }
    // The narrow phase: terrain per hull (section 87), then the hull pairs.
    for (HullWorldEntry& e : hulls) {
        ++census_.steps;
        // 00C44104..00C44110 with the hull mask 0Dh & ~8 (00826410..0082643B) and the terrain's
        // group 8, mask 0: neither direction selects the other.
        if (e.terrain_mask_cleared &&
            !dyn_shapes_overlap_filter(1u, 0x0Du & ~8u, 8u, 0u)) {
            continue;
        }
        e.result = native_narrow_phase(e.unit, *e.body, *e.shapes, true);
        if (e.result.contact) {
            ++census_.contact_steps;
            if (!touched_[e.unit]) { touched_[e.unit] = true; ++census_.units_touched; }
            if (e.result.max_depth > census_.max_depth) census_.max_depth = e.result.max_depth;
        }
    }
    events_.clear();
    pending_events_.clear();
    if (hull_hull) hull_hull_narrow_phase(hulls, true);
    hull_fort_narrow_phase(hulls, forts, hull_fort);
    // 00C35480 (the collision pass's last step, 00C57827), per queued hull-pair event: the
    // manifold's point 0 on A in world space (00C354E6..00C35542; the listener reads the first
    // of the transformed points at record+8h), then each listener in turn, A with (A, B) and B
    // with (B, A). 009377E0 reads each body's velocity at that point: v + w x (p - B+2Ch)
    // (00C35300, 00C31F20 angular M+0Ch, 00C31F40 linear M+0h) and its length (0042B2F0).
    for (const PendingEvent& pe : pending_events_) {
        Manifold& m = *pe.manifold;
        if (m.count() <= 0) continue;
        const DynBody& a = *by_unit.at(pe.unit_a)->body;
        const DynBody& b = *by_unit.at(pe.unit_b)->body;
        const DynSolverContactPoint& p0 = m.points()[0];
        HullContactEvent ev;
        ev.unit_a = pe.unit_a;
        ev.unit_b = pe.unit_b;
        for (int k = 0; k < 3; ++k) {
            ev.point[k] = f32(static_cast<double>(a.row0[k]) * p0.local_point_a[0] +
                              static_cast<double>(a.row1[k]) * p0.local_point_a[1] +
                              static_cast<double>(a.row2[k]) * p0.local_point_a[2] +
                              a.position[k]);
        }
        const auto point_speed = [&ev](const DynBody& body) {
            const DynMotionState& ms = *body.motion;
            const float r[3] = {ev.point[0] - body.position[0], ev.point[1] - body.position[1],
                                ev.point[2] - body.position[2]};
            const float w[3] = {ms.angular_velocity.x, ms.angular_velocity.y,
                                ms.angular_velocity.z};
            const float v[3] = {ms.linear_velocity.x + (r[2] * w[1] - r[1] * w[2]),
                                ms.linear_velocity.y + (r[0] * w[2] - w[0] * r[2]),
                                ms.linear_velocity.z + (w[0] * r[1] - r[0] * w[1])};
            return std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
        };
        ev.speed_a = point_speed(a);
        ev.speed_b = point_speed(b);
        events_.push_back(ev);
    }

    // The scene's manifold list in creation order (LABELLED: 00C3F4D0's list order is not
    // read), each body's contact array (B+74h) in the same order.
    std::vector<Manifold*> list;
    for (auto& [key, m] : manifolds_) {
        m->friction = dyn_combine_friction(by_unit.at(std::get<0>(key))->friction, kTerrainFriction);
        m->restitution = dyn_combine_restitution(0.0f, kTerrainRestitution);
        list.push_back(m.get());
    }
    for (auto& [key, m] : pair_manifolds_) list.push_back(m.get());
    for (auto& [key, m] : fort_manifolds_) list.push_back(m.get());
    std::sort(list.begin(), list.end(),
              [](const Manifold* a, const Manifold* b) { return a->serial < b->serial; });

    // 00C4B610 through its host interface. Body handle 1 is every terrain tile (static, never
    // expanded through); a hull's handle is 2 + its index in `hulls`.
    struct Groups final : DynGroupFormationHost {
        std::vector<Manifold*>& list;
        std::map<std::size_t, std::size_t> body_of_unit;
        std::vector<std::int16_t> manifold_mark, body_mark;
        std::vector<std::vector<DynHandle>> contacts;
        std::vector<std::vector<DynHandle>> groups;
        Groups(std::vector<Manifold*>& l, const std::vector<HullWorldEntry>& hulls) : list(l) {
            for (std::size_t i = 0; i < hulls.size(); ++i) body_of_unit[hulls[i].unit] = i + 2;
            manifold_mark.assign(list.size(), kDynGroupMarkUnassigned);
            body_mark.assign(hulls.size() + 2, kDynGroupMarkUnassigned);
            contacts.resize(hulls.size() + 2);
            for (std::size_t k = 0; k < list.size(); ++k) {
                for (const DynHandle b : {body(list[k]->unit_a), body(list[k]->unit_b)}) {
                    if (b >= 2) contacts[b].push_back(static_cast<DynHandle>(k + 1));
                }
            }
        }
        DynHandle body(std::size_t unit) const {
            if (unit == Manifold::kStatic) return 1;
            return static_cast<DynHandle>(body_of_unit.at(unit));
        }
        Manifold& m(DynHandle h) const { return *list[h - 1]; }
        void clear_groups_00c3f410() override { groups.clear(); }
        void reset_marks_00c36ac0() override {
            std::fill(manifold_mark.begin(), manifold_mark.end(), kDynGroupMarkUnassigned);
            std::fill(body_mark.begin(), body_mark.end(), kDynGroupMarkUnassigned);
        }
        std::int32_t scene_manifold_count() override { return static_cast<std::int32_t>(list.size()); }
        DynHandle manifold_list_head() override { return 1; }
        DynHandle manifold_list_sentinel() override { return static_cast<DynHandle>(list.size() + 1); }
        DynHandle manifold_next(DynHandle h) override { return h + 1; }
        std::int32_t manifold_point_count(DynHandle h) override { return m(h).count(); }
        std::int16_t manifold_group_mark(DynHandle h) override { return manifold_mark[h - 1]; }
        void set_manifold_group_mark(DynHandle h, std::int16_t v) override { manifold_mark[h - 1] = v; }
        DynHandle manifold_body_a(DynHandle h) override { return body(m(h).unit_a); }
        DynHandle manifold_body_b(DynHandle h) override { return body(m(h).unit_b); }
        // LABELLED: a hull body is taken as awake (B+50h bits 0 and 1 clear).
        std::uint32_t body_flags(DynHandle b) override { return b == 1 ? kDynBodyFlagStatic : 0u; }
        void wake_body(DynHandle) override {}
        std::int16_t body_group_mark(DynHandle b) override { return body_mark[b]; }
        void set_body_group_mark(DynHandle b, std::int16_t v) override { body_mark[b] = v; }
        std::int32_t body_contact_count(DynHandle b) override {
            return static_cast<std::int32_t>(contacts[b].size());
        }
        DynHandle body_contact(DynHandle b, std::int32_t i) override {
            return contacts[b][static_cast<std::size_t>(i)];
        }
        void set_group_count(std::int32_t count) override {
            groups.resize(static_cast<std::size_t>(count));
        }
        void append_to_group_00c36b60(std::int32_t g, DynHandle h) override {
            groups[static_cast<std::size_t>(g)].push_back(h);
        }
    } formation(list, hulls);
    dyn_create_contact_groups_00c4b610(formation);

    // One solve (00403720) per group. 00C4DE40 indexes the bodies as it meets them, A then B
    // per manifold (00C4DEDB): a static body is 0, each new dynamic body the next index.
    const DynSolverWorldSettings settings{};   // 0.1, 1.0, 0.5, 10: the shipped world
    for (const std::vector<DynHandle>& group : formation.groups) {
        if (group.empty()) continue;
        std::map<std::size_t, std::int16_t> index_of;
        std::vector<HullWorldEntry*> indexed{nullptr};
        const auto index = [&](std::size_t unit) -> std::int16_t {
            if (unit == Manifold::kStatic) return 0;
            const auto found = index_of.find(unit);
            if (found != index_of.end()) return found->second;
            const std::int16_t k = static_cast<std::int16_t>(indexed.size());
            index_of[unit] = k;
            indexed.push_back(by_unit.at(unit));
            return k;
        };
        const std::int32_t friction_base = static_cast<std::int32_t>(group.size()) * 4;
        std::vector<DynConstraintRow> rows(group.size() * 8);
        std::vector<DynSolverContactPoint*> row_points;
        std::int32_t row = 0;
        for (const DynHandle h : group) {
            Manifold& m = formation.m(h);
            const std::int16_t ia = index(m.unit_a);
            const std::int16_t ib = index(m.unit_b);
            DynSolverBodyInput in_a = ia == 0 ? DynSolverBodyInput{}
                : dynamic_body_input(*by_unit.at(m.unit_a)->body, ia);
            if (ia == 0 && m.static_frame) {
                // The fort's static body: its frame, no velocity, no mass, index 0.
                for (int k = 0; k < 3; ++k) {
                    in_a.row0[k] = m.frame_a[k];
                    in_a.row1[k] = m.frame_a[3 + k];
                    in_a.row2[k] = m.frame_a[6 + k];
                    in_a.position[k] = m.frame_a[9 + k];
                }
            }
            const DynSolverBodyInput in_b = ib == 0 ? DynSolverBodyInput{}
                : dynamic_body_input(*by_unit.at(m.unit_b)->body, ib);
            for (std::int32_t p = 0; p < m.count(); ++p) {
                DynConstraintBuildInput in;
                in.body_a = in_a;
                in.body_b = in_b;
                in.point = m.points()[p];
                in.friction = m.friction;
                in.restitution = m.restitution;
                dyn_build_contact_rows_00c4de40(in, settings, dt,
                    rows[static_cast<std::size_t>(row)],
                    rows[static_cast<std::size_t>(friction_base + row)]);
                row_points.push_back(&m.points()[p]);
                ++row;
            }
        }
        if (row == 0) continue;
        std::vector<DynSolverBodyVelocity> velocities(indexed.size());
        DynConstraintBatch batch;
        batch.rows = rows.data();
        batch.velocities = velocities.data();
        batch.velocity_count = static_cast<std::int32_t>(indexed.size());
        batch.normal_row_count = row;
        batch.friction_row_base = friction_base;
        dyn_apply_warm_start_00c42ba0(batch);
        dyn_solve_group_00403720(batch, settings.iterations);
        std::vector<DynSolverVelocityWriteBack> back(indexed.size());
        dyn_write_back_velocities_00c37b50(velocities.data(), batch.velocity_count, back.data());
        dyn_store_impulses_00c35020(batch, row_points.data(), row);
        const int bodies = static_cast<int>(indexed.size()) - 1;
        ++census_.groups;
        if (bodies >= 2) ++census_.multi_hull_groups;
        if (bodies > census_.max_group_bodies) census_.max_group_bodies = bodies;
        census_.rows += static_cast<unsigned long long>(row);
        for (std::size_t k = 1; k < indexed.size(); ++k) {
            HullWorldEntry& e = *indexed[k];
            DynMotionState& mw = *e.body->motion;
            const DynSolverVelocityWriteBack& d = back[k];
            mw.linear_velocity.x += d.linear_velocity[0];
            mw.linear_velocity.y += d.linear_velocity[1];
            mw.linear_velocity.z += d.linear_velocity[2];
            mw.angular_velocity.x += d.angular_velocity[0];
            mw.angular_velocity.y += d.angular_velocity[1];
            mw.angular_velocity.z += d.angular_velocity[2];
            mw.linear_bias.x += d.linear_bias[0];
            mw.linear_bias.y += d.linear_bias[1];
            mw.linear_bias.z += d.linear_bias[2];
            mw.angular_bias.x += d.angular_bias[0];
            mw.angular_bias.y += d.angular_bias[1];
            mw.angular_bias.z += d.angular_bias[2];
            e.result.points = row;
            e.result.manifolds = static_cast<int>(group.size());
            e.result.group_bodies = bodies;
            e.result.delta_linear = OceanVec3{d.linear_velocity[0], d.linear_velocity[1],
                                              d.linear_velocity[2]};
            e.result.delta_linear_bias = OceanVec3{d.linear_bias[0], d.linear_bias[1],
                                                   d.linear_bias[2]};
            ++census_.solves;
        }
    }
}

}  // namespace bsp
