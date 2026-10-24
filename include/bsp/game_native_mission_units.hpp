#pragma once

#include <cstddef>
#include <string>

#include "bsp/camera_projection.hpp"

namespace bsp::game {

class GameMissionLuaHost;
class GameUnitsHost;
class GameVfsHost;

// Borrowed from the existing mission owner. Query the owner again each render
// frame; neither pointer may survive mission/frame-host destruction.
struct GameNativeMissionUnitSource {
    const GameUnitsHost* units{};
    GameMissionLuaHost* lua{};

    explicit operator bool() const noexcept { return units != nullptr && lua != nullptr; }
};

// Resource identity only. A descriptor never contains a copied simulation pose.
struct GameNativeMissionUnitDescriptor {
    std::size_t unit_index{};
    std::string instance_name;
    std::string class_name;
    std::string type_symbol;
    int type_id{-1};
    std::string model_logical_name;  // installed VehicleClass[type_id].Mesh, unchanged
};

// New C++ render binding, not an original object layout or calling convention.
// Borrows the mission's existing unit/Lua owners and mounted VFS. It performs no
// simulation step, pose publication, scene load, or preload. See
// docs/OCT24_RENDER_D_UNITS.md for addresses, ABI evidence and limitations.
class GameNativeMissionUnits final {
public:
    GameNativeMissionUnits(GameNativeMissionUnitSource, GameVfsHost&) noexcept;
    GameNativeMissionUnits(const GameUnitsHost&, GameMissionLuaHost&, GameVfsHost&) noexcept;

    std::size_t count() const noexcept;
    // False for an absent unit, missing Mesh string, or unavailable installed
    // asset. Hidden units can still have a descriptor and become visible later.
    bool descriptor(std::size_t index, GameNativeMissionUnitDescriptor& out) const;
    // Read the current valid canonical affine rows for every draw/frame.
    // Failure leaves out unchanged. No world matrix is retained here.
    bool read_transform(std::size_t index, CameraMatrix& out) const;
    bool visible(std::size_t index) const;

private:
    GameNativeMissionUnitSource source_;
    GameVfsHost* vfs_{};
};

}  // namespace bsp::game
