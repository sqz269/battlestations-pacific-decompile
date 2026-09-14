#include "bsp/native_unit_group_state.hpp"
#include "bsp/native_model_group_selection.hpp"

namespace bsp {
void set_native_unit_group_state_00876ec0(NativeUnitGroupStateView unit,
    std::int32_t state, NativeUnitGroupStateContext& context) {
    if (state > unit.group_state_358 && context.world_00e188a8 != nullptr) {
        void* const checked_descriptor = unit.descriptor_354;
        if (checked_descriptor &&
            context.fields.descriptor_effect_40(checked_descriptor) >= 0) {
            if (static_cast<volatile std::uint8_t&>(unit.pose.world_valid_c8) == 0)
                refresh_pose_00414db0(unit.pose);

            const float x = static_cast<volatile float&>(unit.pose.world_cc[12]);
            void* const descriptor = unit.descriptor_354;
            std::array<float, 3> position;
            position[0] = x;
            position[1] = static_cast<volatile float&>(unit.pose.world_cc[13]);
            position[2] = static_cast<volatile float&>(unit.pose.world_cc[14]);
            const std::array<float, 3> secondary{0.0f, 0.0f, 0.0f};
            const auto effect = context.fields.descriptor_effect_40(descriptor);
            void* const world = context.world_00e188a8;
            void* const manager = context.fields.world_announcement_manager_21d0(world);
            (void)create_point_announcement_0049c940(manager, position,
                secondary, effect, context.announcements);
        }
    }
    void* const model = unit.model_360;
    unit.group_state_358 = state;
    if (model) {
        select_native_model_group_00710bb0(model, &context.model_groups,
            static_cast<std::uint32_t>(state));
    }
}
} // namespace bsp
