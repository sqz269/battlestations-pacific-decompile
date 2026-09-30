#include "bsp/hull_terrain_contact.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "bsp/dyn_collision_pass.hpp"
#include "bsp/game_hosts_scene_contents.hpp"
#include "bsp/native_dyn_collision_pass.hpp"
#include "bsp/native_dyn_narrow_phase.hpp"

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
    for (std::size_t s = 0; s < shapes.size(); ++s) {
        for (std::size_t li = 0; li < landscapes.size(); ++li) {
            const game::SceneWorldObject& object = lists.objects()[landscapes[li]];
            if (!object.terrain) continue;
            const game::SceneTerrainHeightField& t = *object.terrain;
            std::map<std::pair<int, int>, int> per_tile;
            for (const OceanVec3& v : shapes[s]) {
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
