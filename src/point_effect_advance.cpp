#include "bsp/point_effect_advance.hpp"
#include "bsp/point_effect_controls.hpp"
#include "bsp/camera_multiply.hpp"
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Point-effect advancement requires MSVC Win32.
#endif

namespace bsp {
namespace {
__declspec(naked) void __fastcall add_age(float*, const float*) {
    __asm {
        fld dword ptr [edx]
        fadd dword ptr [ecx]
        fstp dword ptr [ecx]
        ret
    }
}
__declspec(naked) bool __fastcall sample_gate(float*, const float*, const float*) {
    __asm {
        mov eax,dword ptr [esp+4]
        fld dword ptr [ecx]
        fadd dword ptr [eax]
        fld dword ptr [edx]
        fxch
        fcomi st(0),st(1)
        fstp st(1)
        jbe no_sample
        fstp st(0)
        mov al,1
        ret 4
    no_sample:
        fstp dword ptr [ecx]
        xor eax,eax
        ret 4
    }
}
__declspec(naked) void __fastcall copy_xyz_x87(float*, const float*) {
    __asm {
        fld dword ptr [edx]
        fstp dword ptr [ecx]
        fld dword ptr [edx+4]
        fstp dword ptr [ecx+4]
        fld dword ptr [edx+8]
        fstp dword ptr [ecx+8]
        ret
    }
}
__declspec(naked) void __fastcall latch_xyz(float*, const float*) {
    __asm {
        movss xmm0,dword ptr [edx]
        movss xmm1,dword ptr [edx+4]
        movss xmm2,dword ptr [edx+8]
        movss dword ptr [ecx],xmm0
        movss dword ptr [ecx+4],xmm1
        movss dword ptr [ecx+8],xmm2
        ret
    }
}
__declspec(naked) bool __fastcall positive_delta(const float*) {
    __asm {
        movss xmm1,dword ptr [ecx]
        xorps xmm0,xmm0
        comiss xmm1,xmm0
        seta al
        ret
    }
}
// All differences must spill BEFORE the refreshed timer is added to delta.
__declspec(naked) void __fastcall difference_xyz(float*, const float*, const float*) {
    __asm {
        mov eax,dword ptr [esp+4]
        fld dword ptr [edx]
        fsub dword ptr [eax]
        fstp dword ptr [ecx]
        fld dword ptr [edx+4]
        fsub dword ptr [eax+4]
        fstp dword ptr [ecx+4]
        fld dword ptr [edx+8]
        fsub dword ptr [eax+8]
        fstp dword ptr [ecx+8]
        ret 4
    }
}
__declspec(naked) void __fastcall divide_xyz(float*, const float*,
    const float*, const float*) {
    __asm {
        sub esp,4
        mov eax,dword ptr [esp+8]
        fld dword ptr [eax]
        mov eax,dword ptr [esp+12]
        fadd dword ptr [eax]
        fstp dword ptr [esp]
        fld dword ptr [edx]
        fld dword ptr [esp]
        fld st(0)
        fdivp st(2),st(0)
        fxch
        fstp dword ptr [ecx]
        fld dword ptr [edx+4]
        fdiv st(0),st(1)
        fstp dword ptr [ecx+4]
        fdivr dword ptr [edx+8]
        fstp dword ptr [ecx+8]
        add esp,4
        ret 8
    }
}
}

namespace detail {
void advance_point_effect_age(float& age, const float& delta) noexcept {
    add_age(&age, &delta);
}
bool begin_point_effect_sample(PointEffectSampleFields fields,
    const float& delta) noexcept {
    if (fields.track_velocity == 0 && fields.track_displacement == 0) return false;
    if (!sample_gate(&fields.timer, &fields.interval, &delta)) return false;
    copy_xyz_x87(fields.previous_xyz, fields.current_xyz);
    return true;
}
void finish_point_effect_sample(PointEffectSampleFields fields,
    const float& delta, const float* xyz) noexcept {
    latch_xyz(fields.current_xyz, xyz);
    // Interpret +88 as signed without implementation-defined unsigned casts.
    if (fields.sample_count > 1u && fields.sample_count <= 0x7fffffffu &&
        positive_delta(&delta)) {
        float travel[3];
        if (fields.track_velocity != 0) {
            difference_xyz(travel, fields.current_xyz, fields.previous_xyz);
            float velocity[3];
            divide_xyz(velocity, travel, &fields.timer, &delta);
            copy_xyz_x87(fields.velocity_xyz, velocity);
        }
        if (fields.track_displacement != 0) {
            difference_xyz(travel, fields.current_xyz, fields.previous_xyz);
            copy_xyz_x87(fields.displacement_xyz, travel);
        }
    }
    fields.timer = 0.0f;
}
}

void advance_point_effect_00867d00(PointEffectInstanceStorage& effect,
    float delta, void* unused_reference, PointEffectAdvanceRuntime& runtime) {
    (void)unused_reference;
    if (effect.field_0a != 0) restart_point_effect_00866f50(effect, runtime.rows);
    detail::advance_point_effect_age(effect.field_80, delta);
    if (auto* const parent = effect.parent_8c) {
        if (runtime.nodes.parent_storage(*parent).released_44 == 0) {
            if ((parent->valid_flags & 2u) == 0)
                refresh_camera_world_00b6db70(*parent);
            // Native captures node table BEFORE multiply, then reloads receiver
            // and reads captured table+34 AFTER it. The canonical multiply has
            // no callbacks; capture the existing dispatch binding accordingly.
            auto& dispatch = effect.node_110->scene_attachment;
            CameraMatrix product;
            multiply_camera_matrices_00413920(product, effect.relative_d0, parent->world);
            const auto invoke = dispatch.set_world_matrix;
            if (!invoke) throw std::logic_error("point advancement requires current node virtual+34");
            auto& receiver = effect.node_110->scene_attachment;
            invoke(runtime.scenes, receiver, product);
        } else {
            consume_point_effect_parent_0042d9a0(effect.parent_8c, nullptr, runtime.links);
            stop_point_effect_00867b10(effect, runtime.events,
                runtime.actual_manager, runtime.manager_lifetime);
            effect.field_09 = 1;
        }
    }
    const detail::PointEffectSampleFields fields{
        effect.field_48, effect.field_4c, effect.fields_30.data(),
        effect.fields_30.data() + 3, effect.fields_5c.data(),
        effect.fields_5c.data() + 3, effect.sample_count_88,
        effect.field_28, effect.field_2c};
    if (!detail::begin_point_effect_sample(fields, delta)) return;
    auto* const node = effect.node_110;
    if ((node->transform.valid_flags & 2u) == 0)
        refresh_camera_world_00b6db70(node->transform);
    detail::finish_point_effect_sample(fields, delta, node->storage.world_f0.data() + 12);
}
} // namespace bsp
