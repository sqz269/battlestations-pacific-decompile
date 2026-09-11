#pragma once

#include "bsp/camera_transform.hpp"
#include "bsp/gamepad_force_requests.hpp"

namespace bsp {

struct ConstantForceEventParameters {
    std::uint32_t channel_20;
    float duration_24;
    float amplitude_2c;
};
struct FadingForceEventParameters {
    std::uint32_t channel_20;
    float duration_24;
    float radius_28;
    float amplitude_2c;
};
struct AlternatingForceEventParameters {
    std::uint32_t channel_20;
    float duration_24;
    bool second_first_2c;
    float first_value_30, second_value_34;
    float first_period_38, second_period_3c;
};

// Typed projection of the common20h event state. Definition and subject are
// borrowed. References/type/active fields are evidence, not a native vtable or
// a replacement for the surrounding event system's intrusive ownership.
struct GamepadForceEvent {
    std::uint32_t references_04{1};
    bool active_0c{true};
    void* subject_10{};
    const void* definition_14{};
    std::uint32_t event_type_18{6};
    std::uint32_t request_handle_1c{};
};

// View onto the current game+1ED4 target's existing pose storage. This is a
// different pose representation from the subject's canonical CameraTransform.
// The owner implements the actual00414DB0 refresh, as existing unit hosts do.
struct ForceEventTargetPose {
    void* identity;
    bool& world_valid_c8;
    CameraMatrix& world_cc;
};
class ForceEventSpatialHost {
public:
    virtual ~ForceEventSpatialHost() = default;
    virtual ForceEventTargetPose* current_target_00e188a8_1ed4() = 0;
    virtual CameraTransform& subject_transform_110(void* actual_subject) = 0;
    virtual void refresh_target_pose_00414db0(ForceEventTargetPose&) = 0;
};

// Native x87 sum-of-squares ->float, strict double1e-10 threshold, actual CRT
// sqrt ->float. ECX=three floats, ST0=result, RET; no vector/transform copy host.
float force_event_vector_length_0042b2f0(const std::array<float, 3>&);

GamepadForceEvent& construct_constant_force_event_00873450(GamepadForceEvent&,
    const ConstantForceEventParameters&, void* subject, GamepadForceContext&);
GamepadForceEvent& construct_fading_force_event_00873560(GamepadForceEvent&,
    const FadingForceEventParameters&, void* subject, GamepadForceContext&, ForceEventSpatialHost&);
GamepadForceEvent& construct_alternating_force_event_00873750(GamepadForceEvent&,
    const AlternatingForceEventParameters&, void* subject, GamepadForceContext&);

// Native definition virtual creators: ECX=definition, stack subject, RET4.
// Outer standard-new allocation may return null. Each constructor submits an
// actual request to active gamepad index0; it does not fabricate a force signal.
GamepadForceEvent* create_constant_force_event_00869010(const ConstantForceEventParameters&,
    void* subject, GamepadForceContext&);
GamepadForceEvent* create_fading_force_event_008690f0(const FadingForceEventParameters&,
    void* subject, GamepadForceContext&, ForceEventSpatialHost&);
GamepadForceEvent* create_alternating_force_event_00869290(const AlternatingForceEventParameters&,
    void* subject, GamepadForceContext&);

bool force_event_complete_00872180(GamepadForceEvent&, GamepadForceContext&);
void cancel_force_event_00872160(GamepadForceEvent&, GamepadForceContext&);
void force_event_callback_noop_00872150(GamepadForceEvent&,
    std::uintptr_t unused_first, std::uintptr_t unused_second) noexcept;
// The three scalar wrappers873530/873720/873860 do not cancel the handle.
// Flags bit0 frees standard-new storage; flags0 only ends the object lifetime.
GamepadForceEvent* delete_force_event(GamepadForceEvent&, std::uint32_t flags);

} // namespace bsp
