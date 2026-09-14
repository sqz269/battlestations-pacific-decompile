#include "bsp/game_native_type_storage.hpp"

#include <stdexcept>

namespace bsp::game {

GameNativeTypeStorage::GameNativeTypeStorage() noexcept
    : light_types_{root_guard_0109db80_, root_0109db84_,
          node_guard_0108ff54_, node_0108ff90_,
          light_guard_0109010d_, light_0109018c_,
          directional_guard_0109010e_, directional_0109019c_},
      stream_types_{file_guard_0109db54_, file_0109db58_,
          memory_guard_0109db94_, memory_0109dba0_,
          physical_guard_0109dc2c_, physical_0109dc30_} {}

void GameNativeTypeStorage::require_common_bootstrap(
    const LightTypeBootstrap& common) const {
    const auto actual = common.storage();
    if (&actual.root_guard_0109db80 != &light_types_.root_guard_0109db80 ||
        &actual.root_0109db84 != &light_types_.root_0109db84 ||
        &actual.node_guard_0108ff54 != &light_types_.node_guard_0108ff54 ||
        &actual.node_0108ff90 != &light_types_.node_0108ff90 ||
        &actual.light_guard_0109010d != &light_types_.light_guard_0109010d ||
        &actual.light_0109018c != &light_types_.light_0109018c ||
        &actual.directional_guard_0109010e != &light_types_.directional_guard_0109010e ||
        &actual.directional_0109019c != &light_types_.directional_0109019c) {
        throw std::invalid_argument("common type bootstrap uses another storage domain");
    }
}

void GameNativeTypeStorage::initialize_memory_00cd8fc0(
    TypeIdCounterLifetime& existing_counter, LightTypeBootstrap& common_root_bootstrap) {
    require_common_bootstrap(common_root_bootstrap);
    NativeStreamTypeIds types(existing_counter, common_root_bootstrap, stream_types_);
    types.initialize_memory_00cd8fc0();
}

void GameNativeTypeStorage::initialize_physical_00cd9030(
    TypeIdCounterLifetime& existing_counter, LightTypeBootstrap& common_root_bootstrap) {
    require_common_bootstrap(common_root_bootstrap);
    NativeStreamTypeIds types(existing_counter, common_root_bootstrap, stream_types_);
    types.initialize_physical_00cd9030();
}

} // namespace bsp::game
