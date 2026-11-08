// One focused fixture for the production hull-filter adapter: remove terrain
// selection, invalidate every contact family for that unit, retain other
// units' contacts and the shape itself. Include the actual owner TU so its
// private manifold type and adapter are tested without a second implementation.
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <tuple>
#include <vector>
#include "bsp/avoid_zone_dyn_hull.hpp"
#include "bsp/dyn_lcp_impulse_math.hpp"
#include "bsp/rigid_body_integration.hpp"
#include "bsp/world_ocean.hpp"
#define private public
#include "bsp/hull_terrain_contact.hpp"
#undef private
#include "../src/hull_terrain_contact.cpp"

int main() {
    using Solver = bsp::HullTerrainContactSolver;
    Solver solver;
    constexpr std::size_t unit = 42, other = 7;
    solver.manifolds_[{unit, 0, 0, 0, 0}] = std::make_unique<Solver::Manifold>();
    solver.manifolds_[{other, 0, 0, 0, 0}] = std::make_unique<Solver::Manifold>();
    solver.pair_manifolds_[{unit, 43}] = std::make_unique<Solver::Manifold>();
    solver.pair_manifolds_[{1, unit}] = std::make_unique<Solver::Manifold>();
    solver.pair_manifolds_[{other, 8}] = std::make_unique<Solver::Manifold>();
    solver.fort_manifolds_[{1, unit}] = std::make_unique<Solver::Manifold>();
    solver.fort_manifolds_[{1, other}] = std::make_unique<Solver::Manifold>();
    solver.draft_manifolds_[{0, unit}] = std::make_unique<Solver::Manifold>();
    solver.draft_manifolds_[{0, other}] = std::make_unique<Solver::Manifold>();

    auto& shape = solver.hulls_[{unit, 0}];
    void* const body = shape.body;
    const std::uint32_t group = 1, initial_mask = 0x0d;
    std::memcpy(shape.convex + 4, &body, sizeof(body));
    std::memcpy(shape.convex + 0x2c, &group, sizeof(group));
    std::memcpy(shape.convex + 0x30, &initial_mask, sizeof(initial_mask));
    if (!bsp::dyn_shapes_overlap_filter(group, initial_mask, 8, 0)) return 1;
    {
        // world_step owns the same recursive lock when it calls the setter.
        const std::lock_guard<std::recursive_mutex> lock(solver.contact_mutex_);
        Solver::ShapeFilterCalls calls(solver, unit, body);
        bsp::cc13_dyn1::shape_collide_mask_clear_00c47f60(shape.convex, 8, calls);
    }
    std::uint32_t mask;
    std::memcpy(&mask, shape.convex + 0x30, sizeof(mask));
    if (mask != 5 || bsp::dyn_shapes_overlap_filter(group, mask, 8, 0)) return 2;
    if (bsp::cc13_dyn1::shape_group_00c32450(shape.convex) != group) return 3;
    if (solver.hulls_.size() != 1 || solver.hulls_.begin()->second.body != body) return 4;
    if (solver.manifolds_.size() != 1 || std::get<0>(solver.manifolds_.begin()->first) != other) return 5;
    if (solver.pair_manifolds_.size() != 1 || solver.pair_manifolds_.begin()->first.first != other) return 6;
    if (solver.fort_manifolds_.size() != 1 || solver.fort_manifolds_.begin()->first.second != other) return 7;
    if (solver.draft_manifolds_.size() != 1 || solver.draft_manifolds_.begin()->first.second != other) return 8;
    if (solver.census_.retired != 5) return 9;
    // Exercise the production class-mask publication on the same retained
    // shape after terrain removal: setting a class must not restore bit 8.
    shape.convex_ready = true;
    const std::vector<std::vector<bsp::OceanVec3>> shapes(1);
    bsp::HullWorldEntry entry;
    entry.unit = unit;
    entry.shapes = &shapes;
    solver.draft_manifolds_[{0, unit}] = std::make_unique<Solver::Manifold>();
    {
        const std::lock_guard<std::recursive_mutex> lock(solver.contact_mutex_);
        solver.publish_hull_class_bit(unit, shape, 0x40u);
    }
    std::memcpy(&mask, shape.convex + 0x30, sizeof(mask));
    if (mask != 0x45u || shape.class_bit != 0x40u) return 10;
    if (!solver.hull_selects_filter(entry, 0x40u, 0u)
        || solver.hull_selects_filter(entry, 8u, 0u)) return 11;
    if (solver.draft_manifolds_.size() != 1 || solver.census_.retired != 6) return 12;
    {
        const std::lock_guard<std::recursive_mutex> lock(solver.contact_mutex_);
        solver.publish_hull_class_bit(unit, shape, 0u);
    }
    std::memcpy(&mask, shape.convex + 0x30, sizeof(mask));
    if (mask != 5u || shape.class_bit != 0u
        || solver.hull_selects_filter(entry, 0x40u, 0u)) return 13;
    if (solver.hulls_.size() != 1 || solver.hulls_.begin()->second.body != body) return 14;
    std::puts("PASS: terrain and class masks updated on retained shape; contacts invalidated; unrelated contacts preserved");
    return 0;
}
