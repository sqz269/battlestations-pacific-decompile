#pragma once

#include "bsp/camera_projection.hpp"

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace bsp::game {
class GameGunneryHost;
class GameHostLog;
class GameUnitsHost;

// Separate sparse key domain from ordinary unit indices. The native model
// facade maps keys rather than allocating a dense array of this extent.
inline constexpr std::size_t kNativeMissionDeviceRenderKeyBase = 0x80000000u;

struct GameNativeMissionDeviceBinding {
    std::size_t gun_index{}; // first canonical row for this physical platform
    std::size_t render_key{};
    std::size_t unit_index{};
    int platform_key{};
    int device_class{-1};
    std::string mesh; // immutable asset metadata; empty means authored meshless
    std::size_t hierarchy_items{};
};

struct GameNativeMissionDeviceCensus {
    std::size_t gun_rows{};
    std::size_t physical_platforms{};
    std::size_t visible_instances{};
    std::size_t meshless_platforms{};
    std::size_t duplicate_rows{};
    std::size_t distinct_mesh_strings{};
};

// Construct only under the existing BSP_NATIVE_MISSION_SHIP master, after the
// mission's gunnery host has populated its canonical rows/asset cache. Borrows
// that same host through native visual retirement. Owns identity/path metadata
// only, never a pose, angle, gun model, simulation unit or replacement scene.
class GameNativeMissionDevices final {
public:
    GameNativeMissionDevices(GameHostLog&, const GameUnitsHost&);
    GameNativeMissionDevices(const GameNativeMissionDevices&) = delete;
    GameNativeMissionDevices& operator=(const GameNativeMissionDevices&) = delete;

    const std::vector<GameNativeMissionDeviceBinding>& bindings() const noexcept;
    const GameNativeMissionDeviceCensus& census() const noexcept;
    // Before BeginScene only: accept appended rows after verifying every old
    // row/binding is unchanged. True means the gun census grew, possibly only
    // by duplicate physical-platform rows. Reborrow binding entries afterward
    // and load missing sparse keys before starting the native frame.
    bool refresh_source();
    // Strict during a frame: no source growth or identity change is admitted.
    // Call once before each actual camera/pose update.
    void validate_source() const;
    // Read a fresh world matrix for the ORIGINAL asset hierarchy index. Caller
    // synchronously publishes it to genuine model/palette nodes. False means
    // no usable pose; no identity/origin/last-shot fallback is supplied.
    bool current_node_world(std::size_t render_key, std::size_t hierarchy_item,
        CameraMatrix& out) const;

private:
    const GameGunneryHost& source() const;
    void validate_row(std::size_t gun_index, std::size_t binding_index) const;

    const GameUnitsHost& units_;
    const GameGunneryHost* const gunnery_;
    std::vector<GameNativeMissionDeviceBinding> bindings_;
    std::vector<std::size_t> row_bindings_;
    std::unordered_map<std::size_t, std::size_t> key_bindings_;
    GameNativeMissionDeviceCensus census_;
};

} // namespace bsp::game
