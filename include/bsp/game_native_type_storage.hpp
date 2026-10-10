#pragma once

#include "bsp/light_type_bootstrap.hpp"
#include "bsp/camera_type_bootstrap.hpp"
#include "bsp/native_stream_type_ids.hpp"
#include "bsp/native_mesh_subset_loading.hpp"
#include "bsp/native_resource_extra_type_ids.hpp"
#include "bsp/model_type_bootstrap.hpp"
#include "bsp/native_model_base_bootstrap.hpp"
#include "bsp/native_group_owner.hpp"

#include <cstdint>

namespace bsp::game {

// One application's process-owned type cells. The host must retain this object
// through the shared singleton drain and any later native type consumers.
// The counter, its publication, and the common bootstrap are borrowed only by
// the initializer calls; this owner cannot keep a short-lived service alive.
class GameNativeTypeStorage final {
public:
    GameNativeTypeStorage() noexcept;
    GameNativeTypeStorage(const GameNativeTypeStorage&) = delete;
    GameNativeTypeStorage& operator=(const GameNativeTypeStorage&) = delete;
    GameNativeTypeStorage(GameNativeTypeStorage&&) = delete;
    GameNativeTypeStorage& operator=(GameNativeTypeStorage&&) = delete;

    // Stable aggregate views: their references always target this same set of
    // guards and descriptors. GameNativeVfsRuntimeInputs can borrow stream_types().
    LightTypeBootstrapStorage& light_types() noexcept { return light_types_; }
    CameraTypeBootstrapStorage& camera_types() noexcept { return camera_types_; }
    NativeStreamTypeIdStorage& stream_types() noexcept { return stream_types_; }
    NativeMeshResourceTypeStorage& mesh_resource_types() noexcept { return mesh_resource_types_; }
    NativeGameResourceSelectorStorage& resource_selectors() noexcept { return resource_selectors_; }
    NativeResourceExtraTypeIdStorage& resource_extra_types() noexcept { return resource_extra_types_; }
    // Borrow the canonical live cells; these views do not initialize or copy IDs.
    // Borrowers must finish before this process owner is destroyed.
    ModelTypeBootstrapStorage& model_types() noexcept;
    ModelBaseTypeStorage& model_base_types() noexcept;
    NativeGroupTypeStorage& group_types() noexcept;

    // Existing recovered relative CRT order: CCEC00/CCF740/CCF980 selectors, then
    // CD7D80 camera, CD7E60 model, CD7EB0 model-base, CD82F0/CD8340 extra
    // resource types and CD8690/CD86F0/CD87B0 mesh types, before stream types.
    // Group B8F590 is explicitly inserted between extra and mesh types; this
    // Source composition does not establish its original CRT-table position.
    // Other native CRT entries remain separate; this does not promise their
    // execution or original absolute numeric IDs. All represented families
    // share this application's counter/root/scene and stable selector cells.
    void initialize_resource_types(TypeIdCounterLifetime& existing_counter,
        LightTypeBootstrap& common_root_bootstrap);

    // Preserve the caller's CRT order: memory type, separate physical pool
    // initializer, then physical type. Neither entry initializes that pool.
    void initialize_memory_00cd8fc0(TypeIdCounterLifetime& existing_counter,
        LightTypeBootstrap& common_root_bootstrap);
    void initialize_physical_00cd9030(TypeIdCounterLifetime& existing_counter,
        LightTypeBootstrap& common_root_bootstrap);

private:
    void require_common_bootstrap(const LightTypeBootstrap&) const;

    volatile std::uint8_t root_guard_0109db80_{};
    volatile RootTypeDescriptor root_0109db84_{};
    volatile std::uint8_t node_guard_0108ff54_{};
    volatile NodeTypeDescriptor node_0108ff90_{};
    volatile std::uint8_t light_guard_0109010d_{};
    volatile LightTypeDescriptor light_0109018c_{};
    volatile std::uint8_t directional_guard_0109010e_{};
    volatile DirectionalLightTypeDescriptor directional_0109019c_{};
    volatile std::uint8_t camera_guard_0108ff9c_{};
    volatile CameraTypeDescriptor camera_0108ffa0_{};
    volatile std::uint8_t file_guard_0109db54_{};
    volatile NativeFileTypeDescriptor file_0109db58_{};
    volatile std::uint8_t memory_guard_0109db94_{};
    volatile NativeDerivedFileTypeDescriptor memory_0109dba0_{};
    volatile std::uint8_t physical_guard_0109dc2c_{};
    volatile NativeDerivedFileTypeDescriptor physical_0109dc30_{};
    volatile std::uint8_t scene_guard_0109020c_{};
    volatile std::uint32_t scene_01090210_[3]{};
    volatile std::uint8_t animation_guard_01090264_{};
    volatile std::uint32_t animation_01090268_[4]{};
    volatile std::uint8_t bone_guard_01090265_{};
    volatile std::uint32_t bone_01090278_[4]{};
    volatile std::uint8_t mesh_guard_01090440_{};
    volatile std::uint32_t mesh_01090444_[4]{};
    volatile std::uint8_t skined_guard_01090441_{};
    volatile std::uint32_t skined_01090454_[5]{};
    volatile std::uint8_t matrix_guard_01090442_{};
    volatile std::uint32_t matrix_01090468_[5]{};
    volatile std::uint8_t convex_guard_00e19a94_{};
    volatile std::uint32_t convex_00e19a98_[4]{};
    volatile std::uint8_t aux_guard_00e19b51_{};
    volatile std::uint32_t aux_00e19b64_[4]{};
    volatile std::uint8_t geom_mesh_guard_00e19bd4_{};
    volatile std::uint32_t geom_mesh_00e19be4_[4]{};
    LightTypeBootstrapStorage light_types_;
    CameraTypeBootstrapStorage camera_types_;
    NativeStreamTypeIdStorage stream_types_;
    NativeMeshResourceTypeStorage mesh_resource_types_;
    NativeGameResourceSelectorStorage resource_selectors_;
    NativeResourceExtraTypeIdStorage resource_extra_types_;

    // Append backing and views so the existing cells retain their host offsets.
    // These names describe Native roles, not fixed-address mappings. The model
    // descriptor's existing type and the model-base interface both name four
    // DWORDs: own, node, root and Native name address, in that order.
    volatile std::uint8_t model_guard_01090030_{};
    volatile ModelTypeDescriptor model_01090034_{};
    volatile std::uint8_t model_base_guard_01090031_{};
    volatile std::uint32_t model_base_01090044_[4]{};
    ModelTypeBootstrapStorage model_types_;
    ModelBaseTypeStorage model_base_types_;

    // Exact group guard and descriptor, separate from the 010902F4 group pool.
    // The view borrows these same live fields without copying or initializing.
    volatile std::uint8_t group_guard_010902e1_{};
    volatile NativeGroupTypeDescriptor group_0109032c_{};
    NativeGroupTypeStorage group_types_;
};

} // namespace bsp::game
