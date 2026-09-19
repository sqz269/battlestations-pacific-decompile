#include "bsp/native_smoothed_remainder_triplet.hpp"
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native triplet arithmetic requires MSVC Win32 x87 and SSE.
#endif
namespace bsp {
void update_native_smoothed_remainder_triplet_007862c0(NativeSmoothedRemainderTriplet* triplet,
    std::uint32_t first_bits,std::uint32_t second_bits,const NativeTripletSmoothingContext& c){
    const volatile float* upper=&c.upper_00d7a24c;
    const volatile double* falling_new=&c.falling_new_00ce3e20;
    const volatile double* falling_old=&c.falling_old_00d04308;
    const volatile double* rising_new=&c.rising_new_00ce4d68;
    const volatile double* rising_old=&c.rising_old_00d04310;
    std::uint32_t saved_first;
    // Keep the original x87 operation order and spill widths. Ordered C++
    // comparisons would change NaN branches; decimal coefficients lose bits.
    __asm {
        mov ecx,triplet
        xorps xmm3,xmm3
        movss xmm1,first_bits
        comiss xmm3,xmm1
        mov eax,upper
        movss xmm2,dword ptr [eax]
        jbe first_upper
        movaps xmm1,xmm3
        jmp second_lower
    first_upper:
        comiss xmm1,xmm2
        jbe second_lower
        movaps xmm1,xmm2
    second_lower:
        movss xmm0,second_bits
        comiss xmm3,xmm0
        movss saved_first,xmm1
        jbe second_upper
        movaps xmm0,xmm3
        jmp make_remainder
    second_upper:
        comiss xmm0,xmm2
        jbe make_remainder
        movaps xmm0,xmm2
    make_remainder:
        movss first_bits,xmm0
        fld dword ptr first_bits
        movss dword ptr [ecx],xmm1
        fadd dword ptr saved_first
        movss dword ptr [ecx+4],xmm0
        fld1
        fsubrp st(1),st(0)
        fstp dword ptr first_bits
        fld dword ptr first_bits
        fldz
        fcomip st(0),st(1)
        fstp st(0)
        jbe keep_remainder
        movss first_bits,xmm3
        jmp load_previous
    keep_remainder:
        movss xmm0,first_bits
        movss first_bits,xmm0
    load_previous:
        fld dword ptr [ecx+8]
        fstp dword ptr second_bits
        fld dword ptr second_bits
        fld dword ptr first_bits
        fcomi st(0),st(1)
        fxch st(1)
        jc falling
        mov eax,rising_old
        fmul qword ptr [eax]
        fxch st(1)
        mov eax,rising_new
        fmul qword ptr [eax]
        faddp st(1),st(0)
        fstp dword ptr [ecx+8]
        jmp finished
    falling:
        mov eax,falling_old
        fmul qword ptr [eax]
        fxch st(1)
        mov eax,falling_new
        fmul qword ptr [eax]
        faddp st(1),st(0)
        fstp dword ptr [ecx+8]
    finished:
    }
}
} // namespace bsp
