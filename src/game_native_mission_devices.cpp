#include "bsp/game_native_mission_devices.hpp"

#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_gunnery.hpp"
#include "bsp/game_hosts_units.hpp"

#include <map>
#include <set>
#include <stdexcept>
#include <utility>

namespace bsp::game {
namespace {
void require_device(bool condition, const char* message)
{
    if (!condition) throw std::logic_error(message);
}

struct DeviceMetadata {
    std::vector<GameNativeMissionDeviceBinding> bindings;
    std::vector<std::size_t> row_bindings;
    std::unordered_map<std::size_t, std::size_t> key_bindings;
    GameNativeMissionDeviceCensus census;
};

DeviceMetadata read_device_metadata(const GameGunneryHost& source)
{
    DeviceMetadata next;
    const auto& guns = source.guns();
    next.census.gun_rows = guns.size();
    std::map<std::pair<std::size_t, int>, std::size_t> physical;
    std::set<std::string> mesh_strings;
    next.row_bindings.reserve(guns.size());
    for (std::size_t index = 0; index != guns.size(); ++index) {
        GameDeviceVisualMetadata metadata;
        require_device(source.device_visual_metadata(index, metadata) && metadata.mesh,
            "native mission device has no canonical cached Mesh metadata");
        require_device(index < kNativeMissionDeviceRenderKeyBase
                && metadata.unit_index < kNativeMissionDeviceRenderKeyBase,
            "native mission device/unit index exceeds the disjoint render-key domains");
        if (!metadata.mesh->empty()) {
            require_device(metadata.source_loaded && metadata.hierarchy_items != 0,
                "native mission device Mesh lacks the firing host's authored hierarchy provider");
            require_device(metadata.mount_known,
                "native mission device Mesh lacks the firing host's known platform attachment");
        }
        const auto key = std::make_pair(metadata.unit_index, metadata.platform_key);
        const auto existing = physical.find(key);
        if (existing != physical.end()) {
            const auto& binding = next.bindings[existing->second];
            require_device(binding.device_class == metadata.device_class
                    && binding.mesh == *metadata.mesh
                    && binding.hierarchy_items == metadata.hierarchy_items,
                "native mission physical platform has conflicting selected device metadata");
            next.row_bindings.push_back(existing->second);
            ++next.census.duplicate_rows;
            continue;
        }
        if (!metadata.mesh->empty()) {
            ++next.census.visible_instances;
            mesh_strings.insert(*metadata.mesh);
        } else {
            ++next.census.meshless_platforms;
        }
        const std::size_t binding_index = next.bindings.size();
        const std::size_t render_key = kNativeMissionDeviceRenderKeyBase | index;
        next.bindings.push_back({index, render_key, metadata.unit_index, metadata.platform_key,
            metadata.device_class, *metadata.mesh, metadata.hierarchy_items});
        physical.emplace(key, binding_index);
        next.key_bindings.emplace(render_key, binding_index);
        next.row_bindings.push_back(binding_index);
    }
    next.census.physical_platforms = next.bindings.size();
    next.census.distinct_mesh_strings = mesh_strings.size();
    return next;
}

bool same_binding(const GameNativeMissionDeviceBinding& old,
    const GameNativeMissionDeviceBinding& next) noexcept
{
    return old.gun_index == next.gun_index && old.render_key == next.render_key
        && old.unit_index == next.unit_index && old.platform_key == next.platform_key
        && old.device_class == next.device_class && old.mesh == next.mesh
        && old.hierarchy_items == next.hierarchy_items;
}
}

GameNativeMissionDevices::GameNativeMissionDevices(GameHostLog& log, const GameUnitsHost& units)
    : units_(units), gunnery_(units.gunnery())
{
    require_device(gunnery_ != nullptr, "native mission devices require the existing gunnery host");
    auto initial = read_device_metadata(*gunnery_);
    bindings_.swap(initial.bindings);
    row_bindings_.swap(initial.row_bindings);
    key_bindings_.swap(initial.key_bindings);
    census_ = initial.census;
    validate_source();
    log.notef("native mission devices: gun_rows=%zu physical_platforms=%zu visible_instances=%zu "
        "meshless=%zu duplicate_rows=%zu distinct_mesh_strings=%zu "
        "pose=live_gunnery_hierarchy render_key=80000000|first_gun_index",
        census_.gun_rows, census_.physical_platforms, census_.visible_instances,
        census_.meshless_platforms, census_.duplicate_rows, census_.distinct_mesh_strings);
}

const std::vector<GameNativeMissionDeviceBinding>& GameNativeMissionDevices::bindings() const noexcept
{
    return bindings_;
}

const GameNativeMissionDeviceCensus& GameNativeMissionDevices::census() const noexcept
{
    return census_;
}

bool GameNativeMissionDevices::refresh_source()
{
    require_device(units_.gunnery() == gunnery_,
        "native mission device source host changed before visual retirement");
    const std::size_t current_rows = gunnery_->guns().size();
    require_device(current_rows >= census_.gun_rows,
        "native mission device source shrank; refresh only admits appended rows");
    if (current_rows == census_.gun_rows) {
        validate_source();
        return false;
    }
    auto next = read_device_metadata(*gunnery_);
    require_device(units_.gunnery() == gunnery_ && next.census.gun_rows == current_rows
            && current_rows == gunnery_->guns().size()
            && next.row_bindings.size() >= row_bindings_.size()
            && next.bindings.size() >= bindings_.size(),
        "native mission device source changed while preparing append-only metadata");
    for (std::size_t row = 0; row != row_bindings_.size(); ++row)
        require_device(next.row_bindings[row] == row_bindings_[row],
            "native mission existing gun row changed its physical-platform association");
    for (std::size_t index = 0; index != bindings_.size(); ++index)
        require_device(same_binding(bindings_[index], next.bindings[index]),
            "native mission existing device binding changed during append-only refresh");
    // Every allocation, provider read and old-identity check precedes these
    // swaps. No game object, pose or previously retained native owner is changed.
    bindings_.swap(next.bindings);
    row_bindings_.swap(next.row_bindings);
    key_bindings_.swap(next.key_bindings);
    census_ = next.census;
    return true;
}

const GameGunneryHost& GameNativeMissionDevices::source() const
{
    require_device(units_.gunnery() == gunnery_,
        "native mission device source host changed before visual retirement");
    require_device(gunnery_->guns().size() == census_.gun_rows,
        "native mission device source census changed; new assets require explicit rebinding");
    return *gunnery_;
}

void GameNativeMissionDevices::validate_row(std::size_t gun_index, std::size_t binding_index) const
{
    const auto& binding = bindings_[binding_index];
    GameDeviceVisualMetadata metadata;
    require_device(source().device_visual_metadata(gun_index, metadata) && metadata.mesh,
        "native mission device lost its canonical metadata provider");
    require_device(metadata.unit_index == binding.unit_index
            && metadata.platform_key == binding.platform_key
            && metadata.device_class == binding.device_class
            && *metadata.mesh == binding.mesh
            && metadata.hierarchy_items == binding.hierarchy_items,
        "native mission device identity/asset association changed; explicit rebinding required");
    require_device(binding.mesh.empty() || (metadata.source_loaded && metadata.mount_known),
        "native mission device lost its authored hierarchy/platform pose provider");
}

void GameNativeMissionDevices::validate_source() const
{
    (void)source();
    for (std::size_t row = 0; row != row_bindings_.size(); ++row)
        validate_row(row, row_bindings_[row]);
}

bool GameNativeMissionDevices::current_node_world(std::size_t render_key,
    std::size_t hierarchy_item, CameraMatrix& out) const
{
    const auto found = key_bindings_.find(render_key);
    if (found == key_bindings_.end()) return false;
    const auto& binding = bindings_[found->second];
    if (binding.mesh.empty() || hierarchy_item >= binding.hierarchy_items) return false;
    validate_row(binding.gun_index, found->second);
    return source().device_visual_node_world(binding.gun_index, hierarchy_item, out);
}

} // namespace bsp::game
