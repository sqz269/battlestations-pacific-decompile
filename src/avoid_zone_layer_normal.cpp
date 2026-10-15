#include "bsp/avoid_zone_layer_normal.hpp"
#include "bsp/native_render_batch_keys.hpp"

namespace bsp {
namespace {

std::int32_t base_cell_index(float coordinate) noexcept {
    std::int32_t result;
    __asm {
        cvttss2si eax, coordinate
        mov result, eax
    }
    return result;
}

std::int32_t neighboring_cell_index(float coordinate,
    const volatile std::uint32_t* actual_0109eea4) noexcept {
    // 00D7A210 = double 1.0. Keep the sum extended until BF7420 consumes ST0.
    const double one = 1.0;
    std::int32_t result;
    __asm {
        fld coordinate
        fadd one
        mov ecx, actual_0109eea4
        call native_crt_truncate_st0_00bf7420
        mov result, eax
    }
    return result;
}

} // namespace

std::array<float, 3>& avoid_zone_layer_normal_0041bd10(
    AvoidZoneLayerNormalHost& host, const TerrainGridLayerRecord& layer,
    std::array<float, 3>& out, const std::array<float, 2>& point,
    const volatile std::uint32_t* actual_0109eea4, const CameraAxesCrtAccess& crt) {
    const float* world = point.data();
    float world_x;
    float world_z;
    __asm {
        mov eax, world
        fld dword ptr [eax + 4]
        fstp world_z
        fld dword ptr [eax]
        fstp world_x
    }
    std::array<float, 2> cell;
    host.world_to_cell_00417fa0(layer, cell, world_x, world_z); // 0041BD31
    const std::int32_t z0 = base_cell_index(cell[1]); // 0041BD36
    const std::int32_t x0 = base_cell_index(cell[0]); // 0041BD3C
    float height00 = host.cell_height_0041bae0(layer, x0, z0); // 0041BD44
    const std::int32_t x1 = neighboring_cell_index(cell[0], actual_0109eea4); // 0041BD57
    float height10 = host.cell_height_0041bae0(layer, x1, z0); // 0041BD62
    const std::int32_t z1 = neighboring_cell_index(cell[1], actual_0109eea4); // 0041BD75
    float height01 = host.cell_height_0041bae0(layer, x0, z1); // 0041BD82
    float* destination = out.data();
    const float* spacing = &layer.cell_size; // established producer's native +0Ch
    __asm {
        mov eax, destination
        mov edx, spacing
        fld height00
        movss xmm0, dword ptr [edx]
        fld st(0)
        fsub height10
        fstp dword ptr [eax]
        movss dword ptr [eax + 4], xmm0
        fsub height01
        fstp dword ptr [eax + 8]
    }
    float height11 = host.cell_height_0041bae0(layer, x1, z1); // 0041BDB4
    float second_x;
    float second_y;
    float second_z;
    float sum_x;
    float sum_y;
    float sum_z;
    float vector_length;
    float reciprocal;
    float scaled_x;
    float scaled_y;
    float scaled_z;
    const CameraAxesCrtAccess* crt_access = &crt;
    // 0041BDBF-0041BE76: retain the float stores between gradient differences,
    // sums, length, reciprocal and products. In particular JBE includes NaN.
    __asm {
        mov eax, destination
        mov edx, spacing
        fld height01
        fld height11
        fld st(0)
        fsubp st(2), st(0)
        fxch
        fstp second_x
        fld dword ptr [edx]
        fstp second_y
        fsubr height10
        fstp second_z
        fld dword ptr [eax]
        fadd second_x
        fstp sum_x
        fld dword ptr [eax + 4]
        fadd second_y
        fstp sum_y
        fld dword ptr [eax + 8]
        fadd second_z
        fstp sum_z
        fld sum_x
        fstp dword ptr [eax]
        fld sum_y
        fstp dword ptr [eax + 4]
        fld sum_z
        fstp dword ptr [eax + 8]
        mov ecx, eax
        mov edx, crt_access
        call camera_vector_length_00419440 // 0041BE14; existing CRT binding
        fstp vector_length
        fldz
        fld vector_length
        fcomi st(0), st(1)
        fstp st(1)
        jbe zero_reciprocal
        fld1
        fdivrp st(1), st(0) // native 0041BE2B bytes DE F1: 1/length
        fstp reciprocal
        jmp scale_gradient
    zero_reciprocal:
        fstp st(0)
        mov reciprocal, 0
    scale_gradient:
        fld sum_x
        fld reciprocal
        fld st(0)
        fmulp st(2), st(0)
        fxch
        fstp scaled_x
        fld sum_y
        fmul st(0), st(1) // native 0041BE57 bytes D8 C9
        fstp scaled_y
        fmul sum_z
        fstp scaled_z
        mov eax, destination
        fld scaled_x
        fstp dword ptr [eax]
        fld scaled_y
        fstp dword ptr [eax + 4]
        fld scaled_z
        fstp dword ptr [eax + 8]
    }
    return out;
}

} // namespace bsp
