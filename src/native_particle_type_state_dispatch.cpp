#include "bsp/native_particle_type_state_dispatch.hpp"
#include "bsp/native_particle_type_state.hpp"
#include "bsp/native_particle_sprite_state.hpp"
#include "bsp/native_particle_object_state.hpp"
#include "bsp/native_particle_tracer_state.hpp"
#include "bsp/native_particle_emission_state.hpp"
#include "bsp/native_point_light_owner.hpp"
#include <stdexcept>

namespace bsp {
namespace {
void require(bool value) {
    if (!value) throw std::logic_error("particle state dispatch requires the same actual application domains");
}
void common(const NativeParticleTypeStateAccess* expected,
    const NativeParticleTypeStateAccess* actual) {
    require(expected && actual);
    require(expected->random && expected->random==actual->random &&
        expected->zero_00d7a218==actual->zero_00d7a218 &&
        expected->one_00d7a24c==actual->one_00d7a24c &&
        expected->random_scale_00d5da30==actual->random_scale_00d5da30 &&
        expected->base_00d7a210==actual->base_00d7a210 &&
        expected->angle_scale_00d5daf8==actual->angle_scale_00d5daf8 &&
        expected->floating_counter_00f8d384==actual->floating_counter_00f8d384);
}
}
bool dispatch_known_native_particle_type_state(const NativeParticleTypeStateDispatch& d,
    const NativeParticleEmissionStateAccess& emission, void* definition,
    std::uint32_t target, void* state, const void* record) {
    switch (target) {
    case 0x00b059c0u:
        common(d.common,d.common);
        initialize_native_particle_axial_state_00b059c0(definition,d.common,state,record);
        return true;
    case 0x00b077d0u:
        common(d.common,d.common);
        initialize_native_particle_floating_state_00b077d0(definition,d.common,state,record);
        return true;
    case 0x00b08f60u:
        require(d.sprite!=nullptr);
        common(d.common,&d.sprite->type);
        require(d.sprite->unit.random==d.common->random);
        if (d.sprite->lights) {
            const auto& light=*d.sprite->lights;
            require(&light.environment.nodes==&emission.population.nodes &&
                &light.actual_manager_01090aa0==&emission.actual_manager_01090aa0 &&
                &light.actual_lock_0108ff50==&emission.actual_lock_0108ff50);
        }
        initialize_native_particle_sprite_state_00b08f60(definition,d.sprite,state,record);
        return true;
    case 0x00af90a0u:
        require(d.object!=nullptr);
        common(d.common,d.object->type);
        require(d.object->unit_random && d.object->unit_random->random==d.common->random &&
            &d.object->lights.nodes==&emission.population.nodes &&
            &d.object->actual_manager_01090aa0==&emission.actual_manager_01090aa0 &&
            &d.object->actual_lock_0108ff50==&emission.actual_lock_0108ff50);
        initialize_native_particle_object_state_00af90a0(definition,d.object,state,record);
        return true;
    case 0x00b0b6a0u:
        require(d.tracer!=nullptr);
        common(d.common,&d.tracer->common);
        require(d.tracer->unit_random && d.tracer->unit_random->random==d.common->random &&
            d.tracer->actual_manager_01090aa0==&emission.actual_manager_01090aa0 &&
            d.tracer->actual_lock_0108ff50==&emission.actual_lock_0108ff50);
        if (d.tracer->point_lights)
            require(&d.tracer->point_lights->nodes==&emission.population.nodes);
        if (d.tracer->nodes)
            require(d.tracer->nodes==&emission.population.nodes.attachments);
        initialize_native_particle_tracer_state_00b0b6a0(definition,d.tracer,state,record);
        return true;
    default:
        return false;
    }
}
} // namespace bsp
