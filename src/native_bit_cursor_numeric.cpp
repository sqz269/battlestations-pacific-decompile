#include "bsp/native_bit_cursor_numeric.hpp"
#include "bsp/native_render_batch_keys.hpp"
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native numeric bit codecs require MSVC Win32 x87/SSE.
#endif
namespace bsp {
using U=std::uint32_t;
void write_native_signed_dword_bits_00429090(NativeBitCursor* cursor,U value,U bits){
    write_native_bits_00428f50(cursor,&value,bits);
}
void write_native_numeric_float_004295c0(NativeBitCursor* cursor,U value,U zero_flag,
    U signed_flag,U scale,U bits,const NativeBitNumericContext& c){
    const auto* zero=&c.zero_00d7a218;
    if(static_cast<std::uint8_t>(zero_flag)!=0){
        U marker=0;
        __asm {
            movss xmm0, value
            mov ecx, zero
            ucomiss xmm0, dword ptr [ecx]
            lahf
            test ah, 44h
            jp not_zero
            mov marker, 1
        not_zero:
        }
        write_native_bits_00428f50(cursor,&marker,1);
        if(marker==1)return;
    }
    const auto* sentinel=&c.unscaled_00d7a278;
    const auto* minus_one=&c.minus_one_00d7a260;
    const auto* plus_one=&c.plus_one_00d7a24c;
    U normalized;
    __asm {
        fld dword ptr scale
        fld st(0)
        mov ecx, sentinel
        fld qword ptr [ecx]
        fxch st(1)
        fucomip st(0), st(1)
        fstp st(0)
        lahf
        test ah, 44h
        jp normalize_value
        fstp st(0)
        movss xmm0, value
        movss normalized, xmm0
        jmp normalized_done
    normalize_value:
        fdivr dword ptr value
        fstp dword ptr normalized
        fld dword ptr normalized
        mov ecx, minus_one
        fld dword ptr [ecx]
        fcomip st(0), st(1)
        fstp st(0)
        jbe upper_bound
        movss xmm0, dword ptr [ecx]
        movss normalized, xmm0
        jmp normalized_done
    upper_bound:
        movss xmm0, normalized
        mov ecx, plus_one
        movss xmm1, dword ptr [ecx]
        comiss xmm0, xmm1
        jbe normalized_done
        movss normalized, xmm1
    normalized_done:
    }
    if(bits==32){write_native_bits_00428f50(cursor,&normalized,32);return;}
    const bool is_signed=static_cast<std::uint8_t>(signed_flag)!=0;
    U denominator=(1u<<((bits-(is_signed?1u:0u))&31u))-1u;
    const auto* half=&c.half_00d7a280;
    const auto* mode=&c.conversion_mode_0109eea4;
    U encoded;
    // Retain native extended precision until conversion, including unordered
    // taking the subtract-half branch. Do not replace this with a C++ cast.
    __asm {
        fild dword ptr denominator
        fmul dword ptr normalized
        fldz
        fxch st(1)
        fcomi st(0), st(1)
        fstp st(1)
        mov ecx, half
        jbe subtract_half
        fadd qword ptr [ecx]
        jmp rounded
    subtract_half:
        fsub qword ptr [ecx]
    rounded:
    }
    if(is_signed){
        __asm {
            mov ecx, mode
            call native_crt_truncate_st0_00bf7420
            mov encoded, eax
        }
    }else{
        std::uint16_t original_control,truncate_control;
        std::int64_t wide;
        __asm {
            fnstcw original_control
            mov ax, original_control
            or ax, 0c00h
            mov truncate_control, ax
            fldcw truncate_control
            fistp qword ptr wide
            mov eax, dword ptr wide
            mov encoded, eax
            fldcw original_control
        }
    }
    write_native_bits_00428f50(cursor,&encoded,bits);
}
void read_native_numeric_float_004293f0(NativeBitCursor* cursor,U* value,U zero_flag,
    U signed_flag,U scale,U bits,const NativeBitNumericContext& c){
    if(static_cast<std::uint8_t>(zero_flag)!=0){
        std::uint8_t marker=0;
        read_native_bits_00428bb0(cursor,&marker,1);
        if(marker==1){*value=0;return;}
    }
    if(bits==32)read_native_bits_00428bb0(cursor,value,32);
    else{
        U encoded=0;
        read_native_bits_00428bb0(cursor,&encoded,bits);
        if(static_cast<std::uint8_t>(signed_flag)!=0){
            if(static_cast<std::int32_t>(bits)<32){
                const U mask=0xffffffffu<<((bits-1u)&31u);
                if((encoded&mask)!=0)encoded|=mask;
            }
            U denominator=(1u<<((bits-1u)&31u))-1u;
            __asm {
                fild dword ptr encoded
                fidiv dword ptr denominator
                mov ecx, value
                fstp dword ptr [ecx]
            }
        }else{
            U denominator=(1u<<(bits&31u))-1u;
            const auto* bias=&c.uint32_bias_00ce3978;
            __asm {
                fild dword ptr encoded
                cmp encoded, 0
                jge unsigned_ready
                mov ecx, bias
                fadd dword ptr [ecx]
            unsigned_ready:
                fidiv dword ptr denominator
                mov ecx, value
                fstp dword ptr [ecx]
            }
        }
    }
    const auto* sentinel=&c.unscaled_00d7a278;
    __asm {
        fld dword ptr scale
        fld st(0)
        mov ecx, sentinel
        fld qword ptr [ecx]
        fxch st(1)
        fucomip st(0), st(1)
        fstp st(0)
        lahf
        test ah, 44h
        jnp unscaled_result
        mov ecx, value
        fmul dword ptr [ecx]
        fstp dword ptr [ecx]
        jmp read_done
    unscaled_result:
        fstp st(0)
    read_done:
    }
}
} // namespace bsp
