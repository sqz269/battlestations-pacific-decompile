#include "bsp/game_native_mission_units.hpp"

#include "bsp/game_hosts_lua.hpp"
#include "bsp/game_hosts_units.hpp"
#include "bsp/game_hosts_vfs.hpp"

#include <utility>

namespace bsp::game {

GameNativeMissionUnits::GameNativeMissionUnits(GameNativeMissionUnitSource source,
    GameVfsHost& vfs) noexcept : source_(source), vfs_(&vfs) {}

GameNativeMissionUnits::GameNativeMissionUnits(const GameUnitsHost& units,
    GameMissionLuaHost& lua, GameVfsHost& vfs) noexcept
    : GameNativeMissionUnits(GameNativeMissionUnitSource{&units, &lua}, vfs) {}

std::size_t GameNativeMissionUnits::count() const noexcept {
    // create_units updates this from slots.size() after every creation batch,
    // including runtime spawns. Avoid units(), which rebuilds diagnostic rows.
    return source_ ? source_.units->summary().units : 0;
}

bool GameNativeMissionUnits::descriptor(std::size_t index,
    GameNativeMissionUnitDescriptor& out) const {
    if (!source_) return false;
    const GameUnitRow* row = source_.units->unit_row(index);
    if (row == nullptr || row->type_id < 0) return false;

    GameNativeMissionUnitDescriptor found;
    found.unit_index = index;
    found.instance_name = row->name;
    found.class_name = row->class_name;
    found.type_symbol = row->type_symbol;
    found.type_id = row->type_id;
    // This is the same live installed-class reader the unit host already uses
    // for class model Aux/physics data. Keep the original VFS logical name.
    found.model_logical_name = source_.lua->read_vehicle_class_string(row->type_id, "Mesh");
    if (found.model_logical_name.empty() || !vfs_->exists(found.model_logical_name)) return false;
    out = std::move(found);
    return true;
}

bool GameNativeMissionUnits::read_transform(std::size_t index, CameraMatrix& out) const {
    if (!source_ || !source_.units->unit_pose_valid_00c8(index)) return false;
    CameraMatrix current{};
    if (!source_.units->unit_pose(index, current.data(), current.data() + 4,
            current.data() + 8, current.data() + 12)) return false;
    // unit_pose lends the xyz lanes of the authoritative +CC/+DC/+EC/+FC rows.
    // Their affine fourth lanes are 0,0,0,1; do not derive a basis from heading.
    current[15] = 1.0f;
    out = current;
    return true;
}

bool GameNativeMissionUnits::visible(std::size_t index) const {
    return source_ && source_.units->unit_alive_and_visible(index)
        && !source_.units->unit_hit_node_detached(index);
}

}  // namespace bsp::game
