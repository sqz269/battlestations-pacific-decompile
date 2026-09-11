#include "bsp/gameplay_point_effect.hpp"

#include <cstddef>
#include <cstring>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4, "Actual gameplay point effects require Win32");

std::array<float, 3> separation(const CameraMatrix& node, const CameraMatrix& target) noexcept {
    const float* const node_position = node.data() + 12;
    const float* const target_position = target.data() + 12;
    std::array<float, 3> captured, delta;
    float* const source = captured.data();
    float* const result = delta.data();
    __asm {
        mov eax, node_position
        mov ecx, source
        fld dword ptr [eax]
        fstp dword ptr [ecx]
        fld dword ptr [eax+4]
        fstp dword ptr [ecx+4]
        fld dword ptr [eax+8]
        fstp dword ptr [ecx+8]
        mov edx, target_position
        mov eax, result
        fld dword ptr [ecx]
        fsub dword ptr [edx]
        fstp dword ptr [eax]
        fld dword ptr [ecx+4]
        fsub dword ptr [edx+4]
        fstp dword ptr [eax+4]
        fld dword ptr [ecx+8]
        fsub dword ptr [edx+8]
        fstp dword ptr [eax+8]
    }
    return delta;
}
} // namespace

std::uint8_t effect_component_restart_false_0086b7b0(const void*) noexcept { return 0; }

std::uint8_t admit_effect_component_0086b7d0(const void*,
    EffectPointView, CameraTransform&) noexcept {
    return 1;
}

void accumulate_target_shake_0042c3d0(void* actual_target, float contribution) noexcept {
    float sum, clamp_value;
    __asm {
        mov ecx, actual_target
        fld contribution
        fadd dword ptr [ecx+1c0h]
        fstp dword ptr [ecx+1c0h]
        fld dword ptr [ecx+1c0h]
        fstp sum
        fld dword ptr [ecx+1c8h]
        fstp clamp_value
        fld sum
        fld clamp_value
        fcomip st(0), st(1)
        jbe lower_ok
        movss xmm0, clamp_value
        fstp st(0)
        movss dword ptr [ecx+1c0h], xmm0
        jmp finished
    lower_ok:
        fld dword ptr [ecx+1c4h]
        fstp clamp_value
        fld clamp_value
        fxch st(1)
        fcomip st(0), st(1)
        fstp st(0)
        jbe upper_ok
        movss xmm0, clamp_value
        movss dword ptr [ecx+1c0h], xmm0
        jmp finished
    upper_ok:
        movss xmm0, sum
        movss dword ptr [ecx+1c0h], xmm0
    finished:
    }
}

RenderCommandReference* create_point_shake_0086a820(const void* actual_component,
    void* actual_subject, ForceEventSpatialHost& spatial) {
    std::uint8_t persistent;
    std::memcpy(&persistent, static_cast<const std::byte*>(actual_component) + 0x20, 1);
    if (persistent) return nullptr;
    auto* const target = spatial.current_target_00e188a8_1ed4();
    if (!target) return nullptr;
    if (!target->world_valid_c8) spatial.refresh_target_pose_00414db0(*target);
    auto& node = spatial.subject_transform_110(actual_subject); // AFTER target refresh.
    if ((node.valid_flags & 2u) == 0) refresh_camera_world_00b6db70(node);
    const auto delta = separation(node.world, target->world_cc);
    const float distance = force_event_vector_length_0042b2f0(delta);
    float strength;
    unsigned char positive;
    __asm {
        mov eax, actual_component
        fld distance
        fdiv dword ptr [eax+24h]
        fld1
        fsubrp st(1), st(0)
        fmul dword ptr [eax+28h]
        fstp strength
        fldz
        fld strength
        fcomi st(0), st(1)
        fstp st(1)
        seta positive
        fstp st(0)
    }
    if (positive) {
        auto* const current = spatial.current_target_00e188a8_1ed4();
        accumulate_target_shake_0042c3d0(current->identity, strength);
    }
    return nullptr;
}
} // namespace bsp
