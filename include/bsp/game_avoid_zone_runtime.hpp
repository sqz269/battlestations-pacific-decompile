#pragma once

#include "bsp/avoid_zone_geometry.hpp"
#include "bsp/ship_ai_avoid_zone_search.hpp"
#include "bsp/world_map_bounds.hpp"
#include <memory>

namespace bsp {
struct CameraAxesCrtAccess;
struct ShipAiPathLateralAnchor;
}
namespace bsp::game {
class GameHostLog;
class GameSceneContentsHost;

// Process ownership and identity adapter over recovered zone routines. Scene
// Path/Landscape creation and the native singleton ABI are separate work.
// Rebuild only while no path nodes retain zone/corner handles. The scene owner
// and CRT binding must outlive this object; geometry is fixed until destruction.
class GameAvoidZoneRuntime final {
public:
    GameAvoidZoneRuntime(GameHostLog&, const CameraAxesCrtAccess&);
    ~GameAvoidZoneRuntime();
    GameAvoidZoneRuntime(const GameAvoidZoneRuntime&) = delete;
    GameAvoidZoneRuntime& operator=(const GameAvoidZoneRuntime&) = delete;
    void rebuild(const GameSceneContentsHost&, std::int32_t mode,
        std::uint8_t forced, std::int32_t session);
    bool ready() const noexcept;
    std::uint32_t manager_handle() const;
    std::uint32_t containing(const std::array<float, 2>&, std::uint32_t layer) const;
    std::uint32_t group_for_layer(std::uint32_t layer) const;
    std::array<float, 2> push_out(std::uint32_t zone,
        const std::array<float, 2>&, float margin) const;
    std::array<float, 2> nearest(std::uint32_t group,
        const std::array<float, 2>&, float slack, float push);
    bool segment(std::uint32_t layer, const std::array<float, 2>& toward,
        const std::array<float, 2>& from, std::uint32_t& zone, std::int32_t& edge) const;
    bool segment_point(std::uint32_t layer, const std::array<float, 2>& toward,
        const std::array<float, 2>& from, std::array<float, 2>& hit) const;
    AvoidZoneTangentCorners detour(std::uint32_t zone,
        const std::array<float, 2>& far_point, std::int32_t edge,
        std::int32_t near_hint, std::int32_t side_hint, float margin) const;
    bool outside(const std::array<float, 3>&) const;
    std::uint32_t corner(std::uint32_t zone, std::int32_t index) const;
    void ensure_clearance(std::uint32_t zone, std::uint32_t corner);
    const ShipAiPathLateralAnchor* anchor(std::uint32_t corner);
    float corner_clearance(std::uint32_t corner) const;
    // The caller supplies the initial cache bounds and owns the selected list.
    // Clear it before this geometry/allocator owner disappears. These methods
    // do not manufacture the director flags required by 009DA6E0.
    bool refresh_search(ShipAiAvoidZoneSearcher&, ShipAiAvoidZoneSegmentList&,
        const ShipAiAvoidZoneQuery&);
    void clear_search(ShipAiAvoidZoneSegmentList&) noexcept;
    bool search_segment(const ShipAiAvoidZoneSegmentList&,
        const std::array<float, 2>& from, const std::array<float, 2>& toward,
        std::array<float, 2>& hit) const;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
