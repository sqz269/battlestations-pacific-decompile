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
}

GameNativeMissionDevices::GameNativeMissionDevices(GameHostLog& log, const GameUnitsHost& units)
    : units_(units), gunnery_(units.gunnery())
{
    require_device(gunnery_ != nullptr, "native mission devices require the existing gunnery host");
    const auto& guns = gunnery_->guns();
    census_.gun_rows = guns.size();
    std::map<std::pair<std::size_t, int>, std::size_t> physical;
    std::set<std::string> mesh_strings;
    row_bindings_.reserve(guns.size());
    for (std::size_t index = 0; index != guns.size(); ++index) {
        GameDeviceVisualMetadata metadata;
        require_device(gunnery_->device_visual_metadata(index, metadata) && metadata.mesh,
            "native mission device has no canonical cached Mesh metadata");
        require_device(index < kNativeMissionDeviceRenderKeyBase
                && metadata.unit_index < kNativeMissionDeviceRenderKeyBase,
            "native mission device/unit index exceeds the disjoint render-key domains");
        const auto key = std::make_pair(metadata.unit_index, metadata.platform_key);
        const auto existing = physical.find(key);
        if (existing != physical.end()) {
            const auto& binding = bindings_[existing->second];
            require_device(binding.device_class == metadata.device_class
                    && binding.mesh == *metadata.mesh
                    && binding.hierarchy_items == metadata.hierarchy_items,
                "native mission physical platform has conflicting selected device metadata");
            row_bindings_.push_back(existing->second);
            ++census_.duplicate_rows;
            continue;
        }
        if (!metadata.mesh->empty()) {
            require_device(metadata.source_loaded && metadata.hierarchy_items != 0,
                "native mission device Mesh lacks the firing host's authored hierarchy provider");
            require_device(metadata.mount_known,
                "native mission device Mesh lacks the firing host's known platform attachment");
            ++census_.visible_instances;
            mesh_strings.insert(*metadata.mesh);
        } else {
            ++census_.meshless_platforms;
        }
        const std::size_t binding_index = bindings_.size();
        const std::size_t render_key = kNativeMissionDeviceRenderKeyBase | index;
        bindings_.push_back({index, render_key, metadata.unit_index, metadata.platform_key,
            metadata.device_class, *metadata.mesh, metadata.hierarchy_items});
        physical.emplace(key, binding_index);
        key_bindings_.emplace(render_key, binding_index);
        row_bindings_.push_back(binding_index);
    }
    census_.physical_platforms = bindings_.size();
    census_.distinct_mesh_strings = mesh_strings.size();
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
