#include "bsp/unit_hull_extents.hpp"
#include <cstddef>

namespace bsp {
namespace {
const double twice = 2.0; //00D7A308, bits4000000000000000.
static_assert(offsetof(HitQueryBounds, max) == 12);
static_assert(offsetof(HitQueryPoint, z) == 8);
static_assert(offsetof(UnitHullExtents, width_09cc) == 4);
}

void produce_unit_hull_extents_00810faf(UnitHullExtents& output,
    const UnitHullExtentClassInputs* unit_class) noexcept {
    if (!unit_class) return;
    auto* destination = &output;
    const auto* box = unit_class->model_local_box;
    if (!box) {
        const float class_width = unit_class->width_00a4;
        const float class_length = unit_class->length_00a0;
        __asm {
            mov eax,destination
            fld class_width
            fstp dword ptr [eax+4]
            fld class_length
            fstp dword ptr [eax]
        }
        return;
    }
    float negative_minimum;
    float maximum;
    // Retain the double2 on the x87 stack after the X store, as0081100C
    // DC C9 (FMUL ST(1),ST(0)) does. SSE-only arithmetic would change
    // denormal handling and x87 rounding/status behavior.
    __asm {
        mov ecx,box
        mov eax,destination
        fld dword ptr [ecx]
        fchs
        fstp negative_minimum
        fld dword ptr [ecx+12]
        fstp maximum
        fld negative_minimum
        fld maximum
        fcomip st(0),st(1)
        fstp st(0)
        jbe width_negative
        movss xmm0,maximum
        jmp width_selected
    width_negative:
        movss xmm0,negative_minimum
    width_selected:
        movss negative_minimum,xmm0
        fld negative_minimum
        fld twice
        fmul st(1),st(0)
        fxch
        fstp dword ptr [eax+4]
        fld dword ptr [ecx+8]
        fchs
        fstp negative_minimum
        fld dword ptr [ecx+20]
        fstp maximum
        fld negative_minimum
        fld maximum
        fcomip st(0),st(1)
        fstp st(0)
        jbe length_negative
        fmul maximum
        jmp length_selected
    length_negative:
        fmul negative_minimum
    length_selected:
        fstp dword ptr [eax]
    }
}
}
