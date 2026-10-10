#include "bsp/game_native_type_storage.hpp"
#include "bsp/native_fallback_resource_type_initializer.hpp"

#include <stdexcept>

namespace bsp::game {

GameNativeTypeStorage::GameNativeTypeStorage() noexcept
    : light_types_{root_guard_0109db80_, root_0109db84_,
          node_guard_0108ff54_, node_0108ff90_,
          light_guard_0109010d_, light_0109018c_,
          directional_guard_0109010e_, directional_0109019c_},
      camera_types_{camera_guard_0108ff9c_, camera_0108ffa0_},
      stream_types_{file_guard_0109db54_, file_0109db58_,
          memory_guard_0109db94_, memory_0109dba0_,
          physical_guard_0109dc2c_, physical_0109dc30_},
      mesh_resource_types_{scene_guard_0109020c_, scene_01090210_,
          mesh_guard_01090440_, mesh_01090444_, skined_guard_01090441_, skined_01090454_,
          matrix_guard_01090442_, matrix_01090468_},
      resource_selectors_{convex_guard_00e19a94_, convex_00e19a98_,
          aux_guard_00e19b51_, aux_00e19b64_, geom_mesh_guard_00e19bd4_, geom_mesh_00e19be4_},
      resource_extra_types_{animation_guard_01090264_, animation_01090268_,
          bone_guard_01090265_, bone_01090278_},
      model_types_{model_guard_01090030_, model_01090034_},
      model_base_types_{model_base_guard_01090031_, model_base_01090044_[0],
          model_base_01090044_[1], model_base_01090044_[2], model_base_01090044_[3]},
      group_types_{group_guard_010902e1_, group_0109032c_},
      fallback_resource_types_{fallback_guard_0109020d_, fallback_0109021c_} {}

ModelTypeBootstrapStorage& GameNativeTypeStorage::model_types() noexcept {
    return model_types_;
}

ModelBaseTypeStorage& GameNativeTypeStorage::model_base_types() noexcept {
    return model_base_types_;
}

NativeGroupTypeStorage& GameNativeTypeStorage::group_types() noexcept {
    return group_types_;
}

const GameNativeFallbackResourceTypeStorage&
GameNativeTypeStorage::fallback_resource_types() const noexcept {
    return fallback_resource_types_;
}

void GameNativeTypeStorage::initialize_fallback_00b86a00(
    TypeIdCounterLifetime& existing_counter, LightTypeBootstrap& common_root_bootstrap) {
    require_common_bootstrap(common_root_bootstrap);
    NativeMeshResourceTypeIds scene(existing_counter, common_root_bootstrap,
        mesh_resource_types_);
    const NativeFallbackResourceTypeInitializerContext context{
        fallback_resource_types_.guard_0109020d,
        fallback_resource_types_.resource_0109021c,
        0x00d631f4u, mesh_resource_types_.scene_01090210, scene, existing_counter};
    initialize_native_fallback_resource_type_00b86a00(context);
}

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

void GameNativeTypeStorage::initialize_resource_types(
    TypeIdCounterLifetime& existing_counter, LightTypeBootstrap& common_root_bootstrap) {
    require_common_bootstrap(common_root_bootstrap);
    NativeMeshResourceTypeIds mesh(existing_counter, common_root_bootstrap, mesh_resource_types_);
    NativeGameResourceSelectorTypes selectors(existing_counter, mesh, resource_selectors_);
    selectors.initialize_convex_object_00ccec00();
    selectors.initialize_aux_00ccf740();
    selectors.initialize_geom_mesh_00ccf980();
    CameraTypeBootstrap camera(existing_counter, common_root_bootstrap, camera_types_);
    camera.initialize_static_00cd7d80();
    ModelTypeBootstrap model(existing_counter, common_root_bootstrap, model_types_);
    model.initialize_static_00cd7e60();
    ModelBaseTypeBootstrap model_base(existing_counter, common_root_bootstrap,
        model_base_types_);
    model_base.initialize_static_type_descriptor_00cd7eb0();
    NativeResourceExtraTypeIds extra(existing_counter, mesh, resource_extra_types_);
    extra.initialize_animation_resource_00cd82f0();
    extra.initialize_bone_resource_00cd8340();
    // Explicit Source order. Keep B8F590's own guard-first/partial-state behavior.
    NativeGroupTypes group(existing_counter, common_root_bootstrap, group_types_);
    group.initialize_00b8f590(group_types_.group_0109032c);
    mesh.initialize_mesh_resource_00cd8690();
    mesh.initialize_skined_mesh_resource_00cd86f0();
    mesh.initialize_matrix_mesh_resource_00cd87b0();
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
