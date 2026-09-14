#include "bsp/native_unit_scene_initialization.hpp"
#include "bsp/scene_property_bag.hpp"
#include <cstring>
#include <emmintrin.h>

namespace bsp {
void NativeUnitSceneInitializationBindings::propagate_root_00b6d890(void* node, void* root) {
    auto& actual_node = node_transform(node);
    auto* actual_root = root_list(root);
    propagate_native_node_root_00b6d890(nodes_, actual_node, actual_root);
}

void initialize_native_unit_scene_00955420(NativeUnitSceneInitializationView unit,
    NativeUnitSceneInitializationGlobals globals, NativeUnitSceneInitializationBindings& bindings) {
    void* const identity = unit.scene.unit.canonical_unit; // ESI, preserved across calls
    bindings.call_0087bcc0(identity);
    void* const initial_model = unit.model_360;
    void* const initial_inner = initial_model ? bindings.model_inner_160(initial_model) : nullptr;
    const bool refresh = unit.pose.world_valid_c8 == 0; // CMP before +0C read/store
    void* const handle = bindings.inner_handle_0c(initial_inner);
    publish_native_unit_scene_handle_00955448(unit.scene, handle);
    if (refresh) refresh_pose_00414db0(unit.pose);
    bindings.node_position_30(unit.scene.scene_handle_4a4, unit.pose.world_cc.data() + 12);
    void* const root = bindings.world_root_19ec(globals.world_00e188a8);
    bindings.propagate_root_00b6d890(unit.scene.scene_handle_4a4, root);
    const auto primary_result = bindings.unit_primary_10(identity);
    const auto class_index = bindings.descriptor_class_70(unit.descriptor_538);
    bindings.call_009292b0(identity, class_index, primary_result);
    unit.registration_168 = bindings.call_00740fe0(globals.manager_00e1aea0);

    void* holder = unit.property_holder_c0;
    if (holder && bindings.property_holder(holder).kind_04 == 1) {
        void* const bag = bindings.property_holder(holder).bag_08;
        if (bindings.find_property_008f2260(bag, "MinLevel")) {
            void* const current_bag = bindings.property_holder(unit.property_holder_c0).bag_08;
            unit.cloned_properties_724 = bindings.clone_bag_008f41f0(current_bag);
        }
    }
    holder = unit.property_holder_c0; // gate again, including after clone callbacks
    if (holder && bindings.property_holder(holder).kind_04 == 1) {
        void* const bag = bindings.property_holder(holder).bag_08;
        if (bindings.find_property_008f2260(bag, "XOceanReflect")) {
            void* const current_bag = bindings.property_holder(unit.property_holder_c0).bag_08;
            void* const found = bindings.find_property_008f2260(current_bag, "XOceanReflect");
            std::uint8_t enabled;
            std::memcpy(&enabled, bindings.property_record(found).payload_0c, 1);
            void* const node = unit.scene.scene_handle_4a4;
            if (enabled) bindings.set_node_mask_00475e60(node, 2);
            else bindings.clear_node_mask_00728340(node, 2);
        }
        void* const shadow_bag = bindings.property_holder(unit.property_holder_c0).bag_08;
        if (bindings.find_property_008f2260(shadow_bag, "XStaticShadowFactor")) {
            void* const current_bag = bindings.property_holder(unit.property_holder_c0).bag_08;
            void* const found = bindings.find_property_008f2260(current_bag, "XStaticShadowFactor");
            const auto property = bindings.property_record(found);
            __m128 factor;
            if (property.type_04 == static_cast<std::int32_t>(ScenePropertyType::Float)) {
                std::uint32_t word;
                std::memcpy(&word, property.payload_0c, 4);
                factor = _mm_castsi128_ps(_mm_cvtsi32_si128(static_cast<int>(word)));
            } else {
                std::int32_t value;
                std::memcpy(&value, property.payload_0c, 4);
                factor = _mm_cvtsi32_ss(_mm_setzero_ps(), value);
            }
            void* const current_model = unit.model_360; // captured before 52C store
            _mm_store_ss(&unit.shadow_factor_52c, factor);
            void* const inner = bindings.model_inner_160(current_model);
            void* const current_node = bindings.inner_handle_0c(inner);
            bindings.call_00747560(current_node, &unit.shadow_factor_52c);
        }
    }
    unit.byte_62c = 0;
}
} // namespace bsp
