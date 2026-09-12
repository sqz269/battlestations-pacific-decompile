#include "bsp/ship_ai_hull_geometry.hpp"
#include "bsp/native_vector2_math.hpp"

#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <xmmintrin.h>

namespace bsp {
namespace {
const float negative_zero = -0.0f; // 00D7A208, bits80000000.
const double reference_floor_compare = 0x1.6666660000000p+0; //00D045F0.
const float reference_floor = 1.4f; //00D06874, bits3FB33333.

static_assert(offsetof(ShipAiHullGeometry, forward_1ac) == 0x38);
static_assert(offsetof(ShipAiHullGeometry, beam_19c) == 0x28);
static_assert(offsetof(ShipAiHullGeometry, position_184) == 0x10);
static_assert(sizeof(HitQueryPoint) == 12);
static_assert(offsetof(HitQueryBounds, max) == 12);

// ECX geometry, EDX real CRT access, stack pointer to actual shoulder1B8.
// Native009DE3EE..009DE524 schedule, with geometry base rebased from174h.
__declspec(naked) void __fastcall finish_planar_geometry(
    ShipAiHullGeometry*, const CameraAxesCrtAccess*, const float*) {
    __asm {
        push ebx
        push esi
        push edi
        push ebp
        mov esi,ecx
        mov ebx,edx
        mov ebp,dword ptr [esp+0x14]
        sub esp,8
        fld dword ptr [esi]
        fsub dword ptr [esi+0x10]
        lea edi,[esi+0x38]
        mov ecx,edi
        fstp dword ptr [esp]
        fld dword ptr [esi+4]
        fsub dword ptr [esi+0x14]
        fstp dword ptr [esp+4]
        fld dword ptr [esp]
        fstp dword ptr [edi]
        fld dword ptr [esp+4]
        fstp dword ptr [edi+4]
        mov edx,ebx
        call native_vector2_reciprocal_length_00419260
        fstp dword ptr [esp]
        movss xmm0,negative_zero
        fld dword ptr [edi]
        movaps xmm1,xmm0
        fld dword ptr [esp]
        fld st(0)
        fmulp st(2),st(0)
        fxch
        fstp dword ptr [esp]
        fmul dword ptr [edi+4]
        fstp dword ptr [esp+4]
        fld dword ptr [esp]
        fstp dword ptr [edi]
        fld dword ptr [esp+4]
        fstp dword ptr [edi+4]
        subss xmm1,dword ptr [esi+0x3c]
        fld dword ptr [edi]
        movss dword ptr [esi+0x28],xmm1
        fstp dword ptr [esi+0x2c]
        movaps xmm1,xmm0
        subss xmm1,dword ptr [esi+0x28]
        subss xmm0,dword ptr [esi+0x2c]
        movss dword ptr [esi+0x30],xmm1
        movss dword ptr [esi+0x34],xmm0
        fld dword ptr [ebp]
        fstp dword ptr [esp]
        fld dword ptr [esi+0x28]
        fld dword ptr [esp]
        fld st(0)
        fmulp st(2),st(0)
        fxch
        fstp dword ptr [esp]
        fmul dword ptr [esi+0x2c]
        fstp dword ptr [esp+4]
        fld dword ptr [esp]
        fstp dword ptr [esi+0x18]
        fld dword ptr [esp+4]
        fstp dword ptr [esi+0x1c]
        fld dword ptr [esi+0x10]
        fsub dword ptr [esi+0x18]
        fstp dword ptr [esp]
        fld dword ptr [esi+0x14]
        fsub dword ptr [esi+0x1c]
        fstp dword ptr [esp+4]
        fld dword ptr [esp]
        fstp dword ptr [esi+0x20]
        fld dword ptr [esp+4]
        fstp dword ptr [esi+0x24]
        fld dword ptr [esi+0x18]
        fadd dword ptr [esi+0x10]
        fstp dword ptr [esi+0x18]
        fld dword ptr [esi+0x14]
        fadd dword ptr [esi+0x1c]
        fstp dword ptr [esi+0x1c]
        add esp,8
        pop ebp
        pop edi
        pop esi
        pop ebx
        ret 4
    }
}

float limit_reference_speed(float value) noexcept {
    float result;
    __asm {
        fld value
        fld reference_floor_compare
        fcomip st(0),st(1)
        fstp st(0)
        jbe keep_value
        movss xmm0,reference_floor
        jmp store_value
    keep_value:
        movss xmm0,value
    store_value:
        movss result,xmm0
    }
    return result;
}
} // namespace

const CameraMatrix& ship_ai_hull_world_pose(PoseRefreshView& pose) {
    if (pose.world_valid_c8 == 0) refresh_pose_00414db0(pose);
    return pose.world_cc;
}

void ship_ai_hull_copy_bounds_0098a8e0(
    const HitQueryBounds& bounds, HitQueryPoint& minimum, HitQueryPoint& maximum) noexcept {
    const HitQueryBounds* source = &bounds;
    HitQueryPoint* minimum_out = &minimum;
    HitQueryPoint* maximum_out = &maximum;
    __asm {
        mov ecx,source
        fld dword ptr [ecx]
        mov eax,minimum_out
        fstp dword ptr [eax]
        fld dword ptr [ecx+4]
        fstp dword ptr [eax+4]
        fld dword ptr [ecx+8]
        fstp dword ptr [eax+8]
        mov eax,maximum_out
        fld dword ptr [ecx+12]
        fstp dword ptr [eax]
        fld dword ptr [ecx+16]
        fstp dword ptr [eax+4]
        fld dword ptr [ecx+20]
        fstp dword ptr [eax+8]
    }
}

void ship_ai_hull_geometry_009de2f0(ShipAiHullGeometry& output,
    const float& distance, const float& shoulder,
    ShipAiHullUnitAccess& unit, const CameraAxesCrtAccess& crt) {
    std::array<float, 3> local{{0.0f, 0.0f, distance}};
    std::array<float, 3> world;
    transform_point_004142e0(local, unit.unit_world_pose_3fc(), world);
    output.bow_174 = {world[0], world[2]};
    local[2] = _mm_cvtss_f32(_mm_sub_ss(_mm_set_ss(negative_zero), _mm_set_ss(distance)));
    transform_point_004142e0(local, unit.unit_world_pose_3fc(), world);
    output.stern_17c = {world[0], world[2]};
    const CameraMatrix& matrix = unit.unit_world_pose_3fc();
    output.position_184 = {matrix[12], matrix[14]};
    finish_planar_geometry(&output, &crt, &shoulder);
    if (unit.unit_model_vtable20() != nullptr) {
        const HitQueryBounds* model = unit.unit_model_vtable20();
        if (model == nullptr)
            throw std::invalid_argument("native model disappeared between vtable20 calls");
        HitQueryPoint minimum, maximum;
        ship_ai_hull_copy_bounds_0098a8e0(*model, minimum, maximum);
        output.max_y_1bc = maximum.y;
        output.min_y_1c0 = minimum.y;
    } else {
        output.max_y_1bc = 50.0f;
        output.min_y_1c0 = -10.0f;
    }
}

void ship_ai_hull_pre_step_009e0270(ShipAiHullPreStepView& block,
    ShipAiHullPreStepAccess& unit, std::uint32_t ignored_native_argument,
    const CameraAxesCrtAccess& crt) {
    (void)ignored_native_argument;
    block.class_reference_168 = unit.class_reference_0570();
    block.reference_speed_3c4 = limit_reference_speed(unit.unit_reference_speed_0080fc30());
    ship_ai_hull_geometry_009de2f0(block.geometry, block.distance_3e4,
        block.shoulder_offset_1b8, unit, crt);
    const float turn = unit.unit_turn_circle_00811a30(0.5f);
    ship_ai_build_sector_shapes_009e0270(
        turn, block.distance_3e4, unit.unit_full_beam_09cc(), block.sectors_808);
    block.flag_3ea = 0;
    block.flag_3e9 = 0;
    block.flag_3e8 = 0;
    if (block.profile_valid_045 == 0) {
        block.profile_valid_045 = 1;
        std::memset(block.profile_004.data(), 0, block.profile_004.size());
    }
}
} // namespace bsp
