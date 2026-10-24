#include "bsp/game_native_mission_units.hpp"

#include "bsp/game_hosts_lua.hpp"
#include "bsp/game_hosts_units.hpp"
#include "bsp/game_hosts_vfs.hpp"

#include <utility>

namespace bsp::game {
namespace {

bool is_stationary_prop(GameNativeMissionUnitSource source, std::size_t index,
    const GameUnitRow& row) {
    // Match create_units' classification: a StationaryClass name with no
    // vehicle-kind descriptor. Do not substitute a VehicleClass numeric row.
    return source.units->unit_class_id(index) < 0
        && source.lua->stationary_class_exists(row.type_symbol);
}

}  // namespace

GameNativeMissionUnits::GameNativeMissionUnits(GameNativeMissionUnitSource source,
    GameVfsHost& vfs) noexcept : source_(source), vfs_(&vfs) {}

GameNativeMissionUnits::GameNativeMissionUnits(const GameUnitsHost& units,
    GameMissionLuaHost& lua, GameVfsHost& vfs) noexcept
    : GameNativeMissionUnits(GameNativeMissionUnitSource{&units, &lua}, vfs) {}

std::size_t GameNativeMissionUnits::count() const noexcept {
    // The existing count reads canonical slots.size(), including runtime spawns.
    // Avoid units(), which rebuilds diagnostic rows.
    return source_ ? source_.units->count() : 0;
}

bool GameNativeMissionUnits::descriptor(std::size_t index,
    GameNativeMissionUnitDescriptor& out) const {
    if (!source_) return false;
    const GameUnitRow* row = source_.units->unit_row(index);
    if (row == nullptr) return false;

    GameNativeMissionUnitDescriptor found;
    found.unit_index = index;
    found.instance_name = row->name;
    found.class_name = row->class_name;
    found.type_symbol = row->type_symbol;
    found.type_id = row->type_id;
    found.stationary_prop = is_stationary_prop(source_, index, *row);
    if (found.stationary_prop) {
        found.model_logical_name = source_.lua->read_stationary_class_string(row->type_symbol, "Mesh");
    } else {
        if (row->type_id < 0) return false;
        // The existing unit host uses this same reader for model Aux/physics
        // data. Both class tables carry complete installed VFS logical names.
        found.model_logical_name = source_.lua->read_vehicle_class_string(row->type_id, "Mesh");
    }
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
    if (!source_ || source_.units->unit_hit_node_detached(index)) return false;
    bsp::SceneNodeFlags flags;
    if (source_.units->unit_scene_node_flags(index, flags))
        return source_.units->unit_alive_and_visible(index);

    // Props have no vehicle creator, so the host deliberately does not lend
    // full native lifecycle flags. Its actual +5C/+5D cells are still readable.
    const GameUnitRow* row = source_.units->unit_row(index);
    return row != nullptr && is_stationary_prop(source_, index, *row)
        && source_.units->unit_active(index) && !source_.units->unit_flag_005d(index);
}

}  // namespace bsp::game
