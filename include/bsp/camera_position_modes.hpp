#pragma once

#include "bsp/camera_path_sampler.hpp"
#include "bsp/system_camera_axes.hpp"

namespace bsp {

// Borrowed fields only: these views neither own nor initialize native state.
// All identities, slots, records and poses must remain alive through each call.
struct CameraPositionRecordView {
    void*& parent_14;
    void*& target_24;
    std::array<float, 3>& point_28;
    float& theta_34;
    float& rho_38;
    std::uint32_t& kind_48;
};

struct CameraPositionView {
    std::array<float, 3>& position_394;
    float& theta_3c4;
    float& rho_3c8;
    float& divisor_460;
    void*& path_464;
    float& parameter_468;
    void**& records_begin_47c;
    void**& records_end_480;
    void*& configuration_4d0;
};

class CameraPositionHost : public CameraPathHost {
public:
    // Pure lookup of the actual object/field; no copies, replacement identities,
    // allocation, side effects or unsupported-owner defaults are permitted.
    virtual CameraPositionRecordView& resolve_position_record(void* actual_record) = 0;
    virtual float& resolve_configuration_scale_54(void* actual_configuration) = 0;

    // Path lookup and the returning CRT invalid-parameter callback are inherited
    // from CameraPathHost. Mode0 calls the canonical recovered sampler directly.
};

// Native stack float / RET4 / ST0 float. Staged sqrt/atan identity with ordered
// clamps to promoted binary32 +/-pi/2. Uses genuine current host CRT entries;
// original CRT dispatch globals, diagnostics and exceptional policy stay external.
float __stdcall camera_asin_clamped_0042cf10(float value);

// Native ECX matrix, EDX output XYZ, EAX same output, RET. Canonical 42D2E0
// writes output X,Z,Y in order; writable matrix/output overlap is supported.
float* matrix_angles_006e47a0(const CameraMatrix&, float* output_xyz);

// Native ECX camera; three by-value stack floats; RET0Ch. Writes rho first,
// then theta. The original object layout is represented by the borrowed view.
void aim_camera_at_point_00794070(CameraPositionView&,
    std::array<float, 3> point, const CameraAxesCrtAccess&);

// Native ECX camera; stack actual target owner, scale; RET8. Ordered-negative
// scale loads configuration+54. Uses the actual target's canonical pose view.
void aim_camera_at_target_00794130(CameraPositionView&, void* actual_target,
    float scale, CameraPositionHost&, const CameraAxesCrtAccess&);

// Native ECX camera; stack unsigned record index; RET4. Valid pointer ranges
// are required; null start or unsigned index >= length invokes genuine CRT.
// Captures the selected actual record once; unknown kinds preserve all fields.
void apply_camera_position_mode_007954a0(CameraPositionView&, std::uint32_t index,
    CameraPositionHost&, const CameraAxesCrtAccess&);

} // namespace bsp
