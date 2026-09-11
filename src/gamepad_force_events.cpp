#include "bsp/gamepad_force_events.hpp"

#include <cmath>
#include <memory>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
void initialize_event(GamepadForceEvent& event, const void* definition, void* subject) noexcept {
    event.references_04 = 1;
    event.active_0c = true;
    event.subject_10 = subject;
    event.definition_14 = definition;
    event.event_type_18 = 6;
    // Native does not initialize+1C until submission returns. Typed storage is
    // defined during required-host calls; this is not native allocation parity.
    event.request_handle_1c = 0;
}
float difference(float target, float source) noexcept {
    float result;
    __asm {
        fld target
        fsub source
        fstp result
    }
    return result;
}
float distance_gain(float distance, float radius) noexcept {
    float result;
    __asm {
        fld distance
        fdiv radius
        fld1
        fsubrp st(1),st(0)
        fstp result
    }
    // Native tests 0>result with an ordered x87 comparison: NaN survives.
    return result < 0.0f ? 0.0f : result;
}
float scale_amplitude(float amplitude, float gain) noexcept {
    float result;
    __asm {
        fld amplitude
        fmul gain
        fstp result
    }
    return result;
}
}

float force_event_vector_length_0042b2f0(const std::array<float, 3>& vector) {
    const float x = vector[0], y = vector[1], z = vector[2];
    float squared;
    __asm {
        fld y
        fmul y
        fld x
        fmul x
        faddp st(1),st(0)
        fld z
        fmul z
        faddp st(1),st(0)
        fstp squared
    }
    // CE3820 exact double1e-10. Native BF7030 is the actual CRT sqrt boundary;
    // preserve the float argument/result spills without implementing the CRT.
    if (static_cast<double>(squared) > 1.0e-10)
        return static_cast<float>(std::sqrt(static_cast<double>(squared)));
    return 0.0f;
}

GamepadForceEvent& construct_constant_force_event_00873450(GamepadForceEvent& event,
    const ConstantForceEventParameters& parameters, void* subject, GamepadForceContext& context) {
    initialize_event(event, &parameters, subject);
    auto* request = new (std::nothrow) ConstantGamepadForceRequest(parameters.channel_20,
        parameters.amplitude_2c, parameters.duration_24);
    event.request_handle_1c = submit_gamepad_force_request_00a95bf0(0, request, context);
    return event;
}
GamepadForceEvent& construct_fading_force_event_00873560(GamepadForceEvent& event,
    const FadingForceEventParameters& parameters, void* subject, GamepadForceContext& context,
    ForceEventSpatialHost& spatial) {
    initialize_event(event, &parameters, subject);
    float gain = 0.0f;
    if (spatial.current_target_00e188a8_1ed4()) {
        if (!subject) throw std::invalid_argument("spatial force event requires its actual subject");
        auto& transform = spatial.subject_transform_110(subject);
        if ((transform.valid_flags & 2u) == 0) refresh_camera_world_00b6db70(transform);
        const std::array<float, 3> source{{transform.world[12], transform.world[13], transform.world[14]}};
        auto* target = spatial.current_target_00e188a8_1ed4(); // reload after source refresh
        if (!target) throw std::runtime_error("force-event target disappeared after source refresh");
        if (!target->world_valid_c8) spatial.refresh_target_pose_00414db0(*target);
        const std::array<float, 3> separation{{
            difference(target->world_cc[12], source[0]),
            difference(target->world_cc[13], source[1]),
            difference(target->world_cc[14], source[2])}};
        const float distance = force_event_vector_length_0042b2f0(separation);
        gain = distance_gain(distance, parameters.radius_28);
    }
    // Allocate before evaluating the request-constructor arguments, as native;
    // even zero gain still multiplies the actual amplitude (including NaN/-0).
    auto* request = new (std::nothrow) FadingGamepadForceRequest(parameters.channel_20,
        scale_amplitude(parameters.amplitude_2c, gain), parameters.duration_24);
    event.request_handle_1c = submit_gamepad_force_request_00a95bf0(0, request, context);
    return event;
}
GamepadForceEvent& construct_alternating_force_event_00873750(GamepadForceEvent& event,
    const AlternatingForceEventParameters& parameters, void* subject, GamepadForceContext& context) {
    initialize_event(event, &parameters, subject);
    auto* request = new (std::nothrow) AlternatingGamepadForceRequest(parameters.channel_20,
        parameters.second_first_2c, parameters.first_value_30, parameters.second_value_34,
        parameters.first_period_38, parameters.second_period_3c, parameters.duration_24);
    event.request_handle_1c = submit_gamepad_force_request_00a95bf0(0, request, context);
    return event;
}

GamepadForceEvent* create_constant_force_event_00869010(const ConstantForceEventParameters& parameters,
    void* subject, GamepadForceContext& context) {
    std::unique_ptr<GamepadForceEvent> event(new (std::nothrow) GamepadForceEvent);
    if (!event) return nullptr;
    construct_constant_force_event_00873450(*event, parameters, subject, context);
    return event.release();
}
GamepadForceEvent* create_fading_force_event_008690f0(const FadingForceEventParameters& parameters,
    void* subject, GamepadForceContext& context, ForceEventSpatialHost& spatial) {
    std::unique_ptr<GamepadForceEvent> event(new (std::nothrow) GamepadForceEvent);
    if (!event) return nullptr;
    construct_fading_force_event_00873560(*event, parameters, subject, context, spatial);
    return event.release();
}
GamepadForceEvent* create_alternating_force_event_00869290(const AlternatingForceEventParameters& parameters,
    void* subject, GamepadForceContext& context) {
    std::unique_ptr<GamepadForceEvent> event(new (std::nothrow) GamepadForceEvent);
    if (!event) return nullptr;
    construct_alternating_force_event_00873750(*event, parameters, subject, context);
    return event.release();
}
bool force_event_complete_00872180(GamepadForceEvent& event, GamepadForceContext& context) {
    validate_gamepad_force_handle_00a957d0(event.request_handle_1c, context);
    return event.request_handle_1c == 0;
}
void cancel_force_event_00872160(GamepadForceEvent& event, GamepadForceContext& context) {
    if (event.request_handle_1c)
        remove_current_gamepad_force_request_00a957f0(event.request_handle_1c, context);
    event.request_handle_1c = 0;
}
void force_event_callback_noop_00872150(GamepadForceEvent&, std::uintptr_t, std::uintptr_t) noexcept {}
GamepadForceEvent* delete_force_event(GamepadForceEvent& event, std::uint32_t flags) {
    if ((flags & 1u) != 0) delete &event;
    else event.~GamepadForceEvent();
    return &event; // native scalar wrapper EAX=this, even when freed
}

} // namespace bsp
