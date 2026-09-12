#include "bsp/tracer_parameter_curve.hpp"

namespace bsp {

TracerParameterCurveStorage* __thiscall
TracerParameterCurveStorage::initialize_00bacaa0() noexcept {
    // BACABE first publishes the refcount base table CEB130, then BACAD1
    // replaces it with D63FFC. No intervening call observes the base table.
    native_vtable = kTracerParameterCurveNativeVtable;
    reference_count = 1;
    records = nullptr;
    storage_size = 0;
    storage_capacity = 0;
    // BACAE8 calls BAC5E0 on +08 with size 0. Its zero-capacity/zero-size
    // branch makes no allocation and simply writes size 0 again.
    point_count = 0;
    current_segment = 0;
    return this;
}

__declspec(naked) float __thiscall
TracerParameterCurveStorage::sample_00ba9da0(float) noexcept {
    __asm {
        mov edx, dword ptr [ecx + 0x14] // BA9DA0: signed nonpositive count
        test edx, edx
        jg nonempty
        fldz
        ret 4
    nonempty:
        cmp edx, 1
        jne multiple_points
        mov eax, dword ptr [ecx + 8]
        fld dword ptr [eax + 4]
        ret 4
    multiple_points:
        fld dword ptr [esp + 4]
        push esi
        mov esi, dword ptr [ecx + 8]
        fld dword ptr [esi]
        fcomip st(0), st(1)
        jb above_first // Includes unordered, as native JB does.
        fstp st(0)
        fld dword ptr [esi + 4]
        pop esi
        ret 4
    above_first:
        lea eax, [edx + edx * 2]
        fld dword ptr [esi + eax * 4 - 0x0c]
        push edi
        lea edi, [esi + eax * 4]
        fxch st(1)
        fcomi st(0), st(1)
        fstp st(1)
        jb below_last // Includes unordered.
        fstp st(0)
        fld dword ptr [edi - 8]
        pop edi
        pop esi
        ret 4
    below_last:
        mov eax, dword ptr [ecx + 0x18]
        lea eax, [eax + eax * 2]
        fld dword ptr [esi + eax * 4]
        fcomip st(0), st(1)
        jbe cache_not_after_argument // Unordered does not reset the cache.
        mov dword ptr [ecx + 0x18], 0
    cache_not_after_argument:
        mov eax, dword ptr [ecx + 0x18]
        add eax, 1
        cmp eax, edx
        jae interpolate
    seek_segment:
        mov edx, dword ptr [ecx + 0x18]
        lea eax, [edx + edx * 2 + 3]
        fld dword ptr [esi + eax * 4]
        fxch st(1)
        fcomi st(0), st(1)
        fstp st(1)
        jbe interpolate // Equality does not advance the existing cache.
        add edx, 1
        mov dword ptr [ecx + 0x18], edx
        add edx, 1
        cmp edx, dword ptr [ecx + 0x14]
        jb seek_segment
    interpolate:
        mov ecx, dword ptr [ecx + 0x18]
        lea eax, [ecx + ecx * 2]
        fsub dword ptr [esi + eax * 4]
        lea eax, [esi + eax * 4]
        pop edi
        pop esi
        fmul dword ptr [eax + 8]
        fadd dword ptr [eax + 4]
        fstp dword ptr [esp + 4] // BA9E40: required binary32 rounding boundary
        fld dword ptr [esp + 4]
        ret 4 // BA9E48, length 3, inclusive last byte BA9E4A
    }
}

}  // namespace bsp
