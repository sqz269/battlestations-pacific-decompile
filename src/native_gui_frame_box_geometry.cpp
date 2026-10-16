#include "bsp/native_gui_frame_box_geometry.hpp"

#include "bsp/game_native_readonly_data.hpp"
#include "bsp/native_gui_frame_box_factory.hpp"
#include "bsp/native_gui_rectangle_writer.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"

#include <cstddef>
#include <cstring>
#include <limits>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual FrameBox geometry requires MSVC Win32.
#endif

namespace bsp {
namespace {
using U=std::uint32_t;
using Phase=NativeGuiFrameBoxGeometryPhase;
struct Region { const void* address; std::size_t bytes; };
std::uintptr_t first(const Region& r) {
    return reinterpret_cast<std::uintptr_t>(r.address);
}
// Only the leading Source-private/metadata objects must be mutually separate
// and separated from all following regions. Public/raw aliases are admitted.
void check_separation(const Region* regions,std::size_t count,std::size_t protected_count) {
    for(std::size_t i=0;i<count;++i) {
        const auto a=first(regions[i]);
        if(!a || !regions[i].bytes ||
            regions[i].bytes>std::numeric_limits<std::uintptr_t>::max()-a)
            throw std::invalid_argument("FrameBox geometry region is invalid");
    }
    for(std::size_t i=0;i<protected_count;++i)
        for(std::size_t j=i+1;j<count;++j)
            if(first(regions[i])<first(regions[j])+regions[j].bytes &&
                first(regions[j])<first(regions[i])+regions[i].bytes)
                throw std::invalid_argument("FrameBox geometry private regions alias");
}
template<class T> T current(const void* p,std::uintptr_t offset=0) {
    return *reinterpret_cast<const volatile T*>(reinterpret_cast<std::uintptr_t>(p)+offset);
}

// Native ACF9CB..ACFA0D. No Source admission/call while spans are live in x87.
// EBX is the independent fake-F base, not the physical ESP; only private cells
// are rebased. The captured F4 and fresh signed WORD retain their reached order.
__declspec(noinline) U capture_uv_and_begin(const void* actual_box,
    const void* returned_record,U* native_frame,volatile U* captured_index) noexcept {
    U captured_begin;
    __asm {
        mov esi, actual_box
        mov eax, returned_record
        mov ebx, native_frame
        mov ecx, dword ptr [eax+1ch] // 00acf9cb
        mov edx, dword ptr [eax+20h] // 00acf9ce
        movsx edi, word ptr [esi+0ech] // 00acf9d1
        add eax, 1ch // 00acf9d8
        mov dword ptr [ebx+54h], ecx // 00acf9db
        mov ecx, dword ptr [eax+8] // 00acf9df
        mov dword ptr [ebx+5ch], ecx // 00acf9e2
        mov dword ptr [ebx+58h], edx // 00acf9e6
        fld dword ptr [ebx+5ch] // 00acf9ea
        mov edx, dword ptr [eax+0ch] // 00acf9ee
        fsub dword ptr [ebx+54h] // 00acf9f1
        mov eax, dword ptr [esi+0f4h] // 00acf9f5
        test eax, eax // 00acf9fb
        mov dword ptr [ebx+60h], edx // 00acf9fd
        fstp dword ptr [ebx+50h] // 00acfa01
        fld dword ptr [ebx+60h] // 00acfa05
        fsub dword ptr [ebx+58h] // 00acfa09
        fstp dword ptr [ebx+4ch] // 00acfa0d
        mov ecx, captured_index
        mov dword ptr [ecx], edi
        mov captured_begin, eax
    }
    return captured_begin;
}

// Native ACFA4A..ACFF60, with all ESP-private accesses translated using the
// verified push-depth map to independent fake-F+08..83 backing. Physical pushes
// still carry the four genuine stdcall160 arguments. Only160 calls occur in
// this block; its verified body preserves XMM3..7 and all nonvolatile GPRs.
// No C++ operation/admission is interposed among the x87/SSE math or nine calls.
__declspec(noinline) void emit_nine_quads(void* actual_box,void* actual_stream,
    const void* actual_size,const volatile float* live_scale,U* native_frame) noexcept {
    __asm {
        mov esi, actual_box
        mov ebx, native_frame
        mov eax, actual_size // 00acfa4a
        fld dword ptr [ebx + 0x38] // 00acfa51
        fld dword ptr [ebx + 0x50] // 00acfa55
        fld st(0) // 00acfa59
        fmulp st(2), st(0) // 00acfa5b
        fxch st(1) // 00acfa5d
        fstp dword ptr [ebx + 0x38] // 00acfa5f
        fld dword ptr [ebx + 0x3c] // 00acfa63
        fld dword ptr [ebx + 0x4c] // 00acfa67
        fld st(0) // 00acfa6b
        fmulp st(2), st(0) // 00acfa6d
        fxch st(1) // 00acfa6f
        fstp dword ptr [ebx + 0x3c] // 00acfa71
        fld dword ptr [esi + 0x10c] // 00acfa75
        fstp dword ptr [ebx + 0x28] // 00acfa7b
        fld dword ptr [esi + 0x110] // 00acfa7f
        fstp dword ptr [ebx + 0x1c] // 00acfa85
        fld dword ptr [esi + 0x114] // 00acfa89
        mov ecx, live_scale // Source pointer only; reached scalar load follows
        fld dword ptr [ecx] // 00acfa8f
        fld st(0) // 00acfa95
        fmulp st(2), st(0) // 00acfa97
        fxch st(1) // 00acfa99
        fstp dword ptr [ebx + 0x2c] // 00acfa9b
        fld dword ptr [esi + 0x118] // 00acfa9f
        fmul st(0), st(1) // 00acfaa5
        fstp dword ptr [ebx + 0x14] // 00acfaa7
        fld dword ptr [eax] // 00acfaab
        fld dword ptr [ebx + 0x28] // 00acfaad
        fld st(0) // 00acfab1
        fsubp st(2), st(0) // 00acfab3
        fld dword ptr [ebx + 0x1c] // 00acfab5
        fld st(0) // 00acfab9
        fsubp st(3), st(0) // 00acfabb
        fxch st(2) // 00acfabd
        fstp dword ptr [ebx + 0x24] // 00acfabf
        fld dword ptr [eax + 4] // 00acfac3
        fmul st(0), st(3) // 00acfac6
        fld dword ptr [ebx + 0x2c] // 00acfac8
        fld st(0) // 00acfacc
        fsubp st(2), st(0) // 00acface
        fld dword ptr [ebx + 0x14] // 00acfad0
        fsubp st(2), st(0) // 00acfad4
        fxch st(1) // 00acfad6
        fstp dword ptr [ebx + 0x50] // 00acfad8
        fld dword ptr [ebx + 0x38] // 00acfadc
        fld st(0) // 00acfae0
        fdivp st(3), st(0) // 00acfae2
        fld st(6) // 00acfae4
        fmulp st(3), st(0) // 00acfae6
        fxch st(2) // 00acfae8
        fstp dword ptr [ebx + 0x10] // 00acfaea
        fxch st(2) // 00acfaee
        fdivrp st(1), st(0) // 00acfaf0
        fmul st(0), st(4) // 00acfaf2
        fstp dword ptr [ebx + 0x18] // 00acfaf4
        fld dword ptr [ebx + 0x10] // 00acfaf8
        fsubp st(4), st(0) // 00acfafc
        fld dword ptr [ebx + 0x18] // 00acfafe
        fsubp st(4), st(0) // 00acfb02
        fxch st(3) // 00acfb04
        fstp dword ptr [ebx + 0x48] // 00acfb06
        fld dword ptr [ebx + 0x3c] // 00acfb0a
        fld st(0) // 00acfb0e
        fdivp st(4), st(0) // 00acfb10
        fld st(1) // 00acfb12
        fdivp st(4), st(0) // 00acfb14
        fld st(2) // 00acfb16
        fmulp st(4), st(0) // 00acfb18
        fxch st(3) // 00acfb1a
        fstp dword ptr [ebx + 0xc] // 00acfb1c
        fld dword ptr [ebx + 0x14] // 00acfb20
        fdivrp st(3), st(0) // 00acfb24
        fdivp st(2), st(0) // 00acfb26
        fld st(0) // 00acfb28
        fmulp st(2), st(0) // 00acfb2a
        fxch st(1) // 00acfb2c
        movss xmm0, dword ptr [ebx + 0x10] // 00acfb2e
        fstp dword ptr [ebx + 0x20] // 00acfb34
        mov edi, actual_stream // 00acfb38
        fsub dword ptr [ebx + 0xc] // 00acfb3f
        xorps xmm3, xmm3 // 00acfb43
        movss xmm6, dword ptr [ebx + 0x54] // 00acfb46
        movss xmm7, dword ptr [ebx + 0x58] // 00acfb4c
        fsub dword ptr [ebx + 0x20] // 00acfb52
        movss xmm4, dword ptr [ebx + 0x28] // 00acfb56
        movss xmm5, dword ptr [ebx + 0x2c] // 00acfb5c
        lea ecx, [ebx + 0x38] // 00acfb62
        push ecx // 00acfb66
        fstp dword ptr [ebx + 0x4c] // 00acfb67
        lea edx, [ebx + 0x64] // 00acfb6b
        push edx // 00acfb6f
        lea eax, [ebx + 0x8] // 00acfb70
        push eax // 00acfb74
        movss dword ptr [ebx + 0x40], xmm0 // 00acfb75
        movss xmm0, dword ptr [ebx + 0xc] // 00acfb7b
        push edi // 00acfb81
        mov ecx, esi // 00acfb82
        mov dword ptr [ebx + 0x8], 0 // 00acfb84
        movss dword ptr [ebx + 0x38], xmm6 // 00acfb8c
        movss dword ptr [ebx + 0x3c], xmm7 // 00acfb92
        movss dword ptr [ebx + 0x44], xmm0 // 00acfb98
        movss dword ptr [ebx + 0x64], xmm3 // 00acfb9e
        movss dword ptr [ebx + 0x68], xmm3 // 00acfba4
        movss dword ptr [ebx + 0x6c], xmm4 // 00acfbaa
        movss dword ptr [ebx + 0x70], xmm5 // 00acfbb0
        call write_native_gui_rectangle_00acf260 // 00acfbb9
        fld dword ptr [ebx + 0x10] // 00acfbbe
        fadd dword ptr [ebx + 0x54] // 00acfbc2
        movss xmm0, dword ptr [ebx + 0x18] // 00acfbc6
        lea ecx, [ebx + 0x64] // 00acfbcc
        push ecx // 00acfbd0
        fst qword ptr [ebx + 0x38] // 00acfbd1
        lea edx, [ebx + 0x74] // 00acfbd5
        fadd dword ptr [ebx + 0x48] // 00acfbd9
        push edx // 00acfbdd
        movss dword ptr [ebx + 0x6c], xmm0 // 00acfbde
        movss xmm0, dword ptr [ebx + 0xc] // 00acfbe4
        fstp dword ptr [ebx + 0x30] // 00acfbea
        lea eax, [ebx + 0x8] // 00acfbee
        fld dword ptr [ebx + 0x30] // 00acfbf2
        push eax // 00acfbf6
        fstp dword ptr [ebx + 0x64] // 00acfbf7
        movss dword ptr [ebx + 0x70], xmm0 // 00acfbfb
        fld dword ptr [ebx + 0x24] // 00acfc01
        movss xmm0, dword ptr [ebx + 0x1c] // 00acfc05
        fadd dword ptr [ebx + 0x28] // 00acfc0b
        push edi // 00acfc0f
        mov ecx, esi // 00acfc10
        movss dword ptr [ebx + 0x68], xmm7 // 00acfc12
        fstp dword ptr [ebx + 0x28] // 00acfc18
        movss dword ptr [ebx + 0x78], xmm3 // 00acfc1c
        fld dword ptr [ebx + 0x28] // 00acfc25
        movss dword ptr [ebx + 0x7c], xmm0 // 00acfc29
        fstp dword ptr [ebx + 0x74] // 00acfc32
        movss dword ptr [ebx + 0x80], xmm5 // 00acfc39
        call write_native_gui_rectangle_00acf260 // 00acfc42
        fld dword ptr [ebx + 0xc] // 00acfc47
        fadd dword ptr [ebx + 0x58] // 00acfc4b
        movss xmm0, dword ptr [ebx + 0x10] // 00acfc4f
        movss dword ptr [ebx + 0x7c], xmm0 // 00acfc55
        movss xmm0, dword ptr [ebx + 0x20] // 00acfc5b
        fst qword ptr [ebx + 0x54] // 00acfc61
        movss dword ptr [ebx + 0x80], xmm0 // 00acfc65
        fadd dword ptr [ebx + 0x4c] // 00acfc6e
        movss xmm0, dword ptr [ebx + 0x14] // 00acfc72
        movss dword ptr [ebx + 0x74], xmm6 // 00acfc78
        movss dword ptr [ebx + 0x64], xmm3 // 00acfc7e
        fstp dword ptr [ebx + 0x34] // 00acfc84
        movss dword ptr [ebx + 0x6c], xmm4 // 00acfc88
        fld dword ptr [ebx + 0x34] // 00acfc8e
        movss dword ptr [ebx + 0x70], xmm0 // 00acfc92
        fstp dword ptr [ebx + 0x78] // 00acfc98
        fld dword ptr [ebx + 0x50] // 00acfc9c
        fadd dword ptr [ebx + 0x2c] // 00acfca0
        fstp dword ptr [ebx + 0x2c] // 00acfca4
        fld dword ptr [ebx + 0x2c] // 00acfca8
        fstp dword ptr [ebx + 0x68] // 00acfcac
        lea ecx, [ebx + 0x74] // 00acfcb0
        push ecx // 00acfcb4
        lea edx, [ebx + 0x64] // 00acfcb5
        push edx // 00acfcb9
        lea eax, [ebx + 0x8] // 00acfcba
        push eax // 00acfcbe
        push edi // 00acfcbf
        mov ecx, esi // 00acfcc0
        call write_native_gui_rectangle_00acf260 // 00acfcc2
        fld dword ptr [ebx + 0x2c] // 00acfcc7
        movss xmm0, dword ptr [ebx + 0x30] // 00acfccb
        fstp dword ptr [ebx + 0x68] // 00acfcd1
        movss dword ptr [ebx + 0x74], xmm0 // 00acfcd5
        movss xmm0, dword ptr [ebx + 0x34] // 00acfcdb
        movss dword ptr [ebx + 0x78], xmm0 // 00acfce1
        movss xmm0, dword ptr [ebx + 0x18] // 00acfce7
        movss dword ptr [ebx + 0x7c], xmm0 // 00acfced
        movss xmm0, dword ptr [ebx + 0x20] // 00acfcf3
        lea ecx, [ebx + 0x74] // 00acfcf9
        push ecx // 00acfcfd
        movss dword ptr [ebx + 0x80], xmm0 // 00acfcfe
        movss xmm0, dword ptr [ebx + 0x28] // 00acfd07
        lea edx, [ebx + 0x64] // 00acfd0d
        push edx // 00acfd11
        movss dword ptr [ebx + 0x64], xmm0 // 00acfd12
        movss xmm0, dword ptr [ebx + 0x1c] // 00acfd18
        lea eax, [ebx + 0x8] // 00acfd1e
        push eax // 00acfd22
        movss dword ptr [ebx + 0x6c], xmm0 // 00acfd23
        movss xmm0, dword ptr [ebx + 0x14] // 00acfd29
        push edi // 00acfd2f
        mov ecx, esi // 00acfd30
        movss dword ptr [ebx + 0x70], xmm0 // 00acfd32
        call write_native_gui_rectangle_00acf260 // 00acfd3b
        fld qword ptr [ebx + 0x38] // 00acfd40
        movss xmm0, dword ptr [ebx + 0xc] // 00acfd44
        fstp dword ptr [ebx + 0x38] // 00acfd4a
        fld dword ptr [ebx + 0x38] // 00acfd4e
        lea ecx, [ebx + 0x74] // 00acfd52
        push ecx // 00acfd56
        fstp dword ptr [ebx + 0x74] // 00acfd57
        lea edx, [ebx + 0x64] // 00acfd5b
        push edx // 00acfd5f
        lea eax, [ebx + 0x8] // 00acfd60
        push eax // 00acfd64
        movss dword ptr [ebx + 0x78], xmm7 // 00acfd65
        movss xmm7, dword ptr [ebx + 0x48] // 00acfd6e
        movss dword ptr [ebx + 0x80], xmm0 // 00acfd74
        movss xmm0, dword ptr [ebx + 0x24] // 00acfd7d
        push edi // 00acfd83
        mov ecx, esi // 00acfd84
        movss dword ptr [ebx + 0x7c], xmm7 // 00acfd86
        movss dword ptr [ebx + 0x64], xmm4 // 00acfd8f
        movss dword ptr [ebx + 0x68], xmm3 // 00acfd95
        movss dword ptr [ebx + 0x6c], xmm0 // 00acfd9b
        movss dword ptr [ebx + 0x70], xmm5 // 00acfda1
        call write_native_gui_rectangle_00acf260 // 00acfdaa
        fld qword ptr [ebx + 0x54] // 00acfdaf
        movss xmm0, dword ptr [ebx + 0x10] // 00acfdb3
        fstp dword ptr [ebx + 0xc] // 00acfdb9
        fld dword ptr [ebx + 0xc] // 00acfdbd
        lea ecx, [ebx + 0x74] // 00acfdc1
        push ecx // 00acfdc5
        fstp dword ptr [ebx + 0x78] // 00acfdc6
        lea edx, [ebx + 0x64] // 00acfdca
        push edx // 00acfdce
        lea eax, [ebx + 0x8] // 00acfdcf
        push eax // 00acfdd3
        movss dword ptr [ebx + 0x74], xmm6 // 00acfdd4
        movss xmm6, dword ptr [ebx + 0x4c] // 00acfddd
        movss dword ptr [ebx + 0x64], xmm3 // 00acfde3
        movss xmm3, dword ptr [ebx + 0x50] // 00acfde9
        push edi // 00acfdef
        mov ecx, esi // 00acfdf0
        movss dword ptr [ebx + 0x7c], xmm0 // 00acfdf2
        movss dword ptr [ebx + 0x80], xmm6 // 00acfdfb
        movss dword ptr [ebx + 0x68], xmm5 // 00acfe04
        movss dword ptr [ebx + 0x6c], xmm4 // 00acfe0a
        movss dword ptr [ebx + 0x70], xmm3 // 00acfe10
        call write_native_gui_rectangle_00acf260 // 00acfe19
        movss xmm0, dword ptr [ebx + 0x30] // 00acfe1e
        fld dword ptr [ebx + 0xc] // 00acfe24
        lea ecx, [ebx + 0x74] // 00acfe28
        push ecx // 00acfe2c
        fstp dword ptr [ebx + 0x78] // 00acfe2d
        movss dword ptr [ebx + 0x74], xmm0 // 00acfe31
        movss xmm0, dword ptr [ebx + 0x18] // 00acfe37
        lea edx, [ebx + 0x64] // 00acfe3d
        push edx // 00acfe41
        movss dword ptr [ebx + 0x7c], xmm0 // 00acfe42
        movss xmm0, dword ptr [ebx + 0x28] // 00acfe4b
        lea eax, [ebx + 0x8] // 00acfe51
        push eax // 00acfe55
        movss dword ptr [ebx + 0x64], xmm0 // 00acfe56
        movss xmm0, dword ptr [ebx + 0x1c] // 00acfe5c
        push edi // 00acfe62
        mov ecx, esi // 00acfe63
        movss dword ptr [ebx + 0x80], xmm6 // 00acfe65
        movss dword ptr [ebx + 0x68], xmm5 // 00acfe6e
        movss dword ptr [ebx + 0x6c], xmm0 // 00acfe74
        movss dword ptr [ebx + 0x70], xmm3 // 00acfe7a
        call write_native_gui_rectangle_00acf260 // 00acfe83
        movss xmm0, dword ptr [ebx + 0x38] // 00acfe88
        movss dword ptr [ebx + 0x74], xmm0 // 00acfe8e
        movss xmm0, dword ptr [ebx + 0x34] // 00acfe94
        movss dword ptr [ebx + 0x78], xmm0 // 00acfe9a
        movss xmm0, dword ptr [ebx + 0x20] // 00acfea0
        lea ecx, [ebx + 0x74] // 00acfea6
        push ecx // 00acfeaa
        movss dword ptr [ebx + 0x80], xmm0 // 00acfeab
        movss xmm0, dword ptr [ebx + 0x2c] // 00acfeb4
        lea edx, [ebx + 0x64] // 00acfeba
        push edx // 00acfebe
        movss dword ptr [ebx + 0x68], xmm0 // 00acfebf
        movss xmm0, dword ptr [ebx + 0x24] // 00acfec5
        lea eax, [ebx + 0x8] // 00acfecb
        push eax // 00acfecf
        movss dword ptr [ebx + 0x6c], xmm0 // 00acfed0
        movss xmm0, dword ptr [ebx + 0x14] // 00acfed6
        push edi // 00acfedc
        mov ecx, esi // 00acfedd
        movss dword ptr [ebx + 0x7c], xmm7 // 00acfedf
        movss dword ptr [ebx + 0x64], xmm4 // 00acfee8
        movss dword ptr [ebx + 0x70], xmm0 // 00acfeee
        call write_native_gui_rectangle_00acf260 // 00acfef7
        movss xmm0, dword ptr [ebx + 0x38] // 00acfefc
        lea ecx, [ebx + 0x74] // 00acff02
        push ecx // 00acff06
        lea edx, [ebx + 0x64] // 00acff07
        push edx // 00acff0b
        movss dword ptr [ebx + 0x74], xmm0 // 00acff0c
        movss xmm0, dword ptr [ebx + 0xc] // 00acff12
        lea eax, [ebx + 0x8] // 00acff18
        push eax // 00acff1c
        movss dword ptr [ebx + 0x78], xmm0 // 00acff1d
        movss xmm0, dword ptr [ebx + 0x24] // 00acff26
        push edi // 00acff2c
        mov ecx, esi // 00acff2d
        movss dword ptr [ebx + 0x7c], xmm7 // 00acff2f
        movss dword ptr [ebx + 0x80], xmm6 // 00acff38
        movss dword ptr [ebx + 0x64], xmm4 // 00acff41
        movss dword ptr [ebx + 0x68], xmm5 // 00acff47
        movss dword ptr [ebx + 0x6c], xmm0 // 00acff4d
        movss dword ptr [ebx + 0x70], xmm3 // 00acff53
        call write_native_gui_rectangle_00acf260 // 00acff5c
    }
}
} // namespace

struct NativeGuiFrameBoxGeometryAccess final {
    static void preflight(void* box,void* stream,const void* size,
        const NativeGuiFrameBoxGeometryContext& c,
        const NativeGuiFrameBoxGeometryFrame& f) {
        const Region regions[]={{&f,sizeof(f)},{&c,sizeof(c)},
            {c.data_,sizeof(*c.data_)},{c.normalization_,sizeof(*c.normalization_)},
            {box,0x120},{stream,0x2c},{size,8},
            {const_cast<const float*>(c.live_scale_),4},
            {c.aliases_[0],0x9c},{c.aliases_[1],0x50},{c.aliases_[2],4},
            {c.aliases_[3],8},{c.aliases_[4],8}};
        check_separation(regions,sizeof(regions)/sizeof(regions[0]),4);
        if(reinterpret_cast<std::uintptr_t>(box)&3u)
            throw std::invalid_argument("FrameBox geometry actual Box is unaligned");
        if(f.diagnostics_.phase!=Phase::fresh ||
            f.normalization_.diagnostics().phase!=NativeGuiTextureNormalizationPhase::fresh)
            throw std::logic_error("FrameBox geometry frame was already entered");
    }
    static void run(void* box,void* stream,const void* size,
        const NativeGuiFrameBoxGeometryContext& c,
        NativeGuiFrameBoxGeometryFrame& f) {
        preflight(box,stream,size,c,f);
        auto& d=f.diagnostics_;
        d.actual_box=box;d.actual_stream=stream;d.actual_size=size;
        d.phase=Phase::entered;
        try {
            const auto initial_index=current<std::uint16_t>(box,0xec);
            const U profile=current<U>(box);
            const U target=current<U>(reinterpret_cast<const void*>(profile),0x98);
            d.initial_index=initial_index;d.widget_profile=profile;d.state_target=target;
            d.phase=Phase::state_selected;
            if(profile!=reinterpret_cast<std::uintptr_t>(c.aliases_[0]) || target!=0x00ad2750)
                throw std::logic_error("current actual FrameBox state route is unsupported");
            std::int16_t signed_initial;
            std::memcpy(&signed_initial,&initial_index,sizeof(signed_initial));
            const void* record=native_gui_frame_box_state_record_00ad2750(box,signed_initial);
            d.returned_record=record;
            d.phase=Phase::state_returned;
            const U begin=capture_uv_and_begin(box,record,f.native_frame_,&f.captured_index_);
            const U saved_index=f.captured_index_;
            d.saved_index_bits=saved_index;d.count_begin=begin;
            d.phase=Phase::uv_captured;
            bool in_range=false;
            if(begin) {
                const U end=current<U>(box,0xf8);
                const U difference=end-begin;
                std::int32_t signed_difference;
                std::memcpy(&signed_difference,&difference,sizeof(signed_difference));
                const std::int32_t count=signed_difference/0x2c;
                d.count_end=end;
                in_range=saved_index<static_cast<U>(count);
            }
            if(!in_range) invoke_native_invalid_parameter_00bf6713();
            const U selected_begin=current<U>(box,0xf4);
            const U address=selected_begin+saved_index*0x2cu;
            const U texture=current<U>(reinterpret_cast<const void*>(address));
            d.selected_begin=selected_begin;
            d.texture=reinterpret_cast<const void*>(texture);
            d.phase=Phase::texture_selected;
            normalize_gui_texture_size_00aceb70(
                reinterpret_cast<std::byte*>(f.native_frame_)+0x38,
                reinterpret_cast<const void*>(texture),*c.normalization_,f.normalization_);
            d.phase=Phase::normalized;
            d.phase=Phase::emitting;
            emit_nine_quads(box,stream,size,c.live_scale_,f.native_frame_);
            d.phase=Phase::completed;
        } catch(...) {
            d.failed_from=d.phase;d.phase=Phase::failed;
            throw;
        }
    }
};

NativeGuiFrameBoxGeometryContext::NativeGuiFrameBoxGeometryContext(
    const game::GameNativeReadOnlyData& data,
    const NativeGuiTextureNormalizationProfiles& normalization,
    const volatile float* live_scale_00e1301c) {
    const void* aliases[]={data.data_at(0x00d5d130,0x9c),
        data.data_at(0x00d61948,0x50),data.data_at(0x00ce3978,4),
        data.data_at(0x00cec380,8),data.data_at(0x00cef1b8,8)};
    const Region regions[]={{this,sizeof(*this)},{&data,sizeof(data)},
        {&normalization,sizeof(normalization)},
        {aliases[0],0x9c},{aliases[1],0x50},{aliases[2],4},
        {aliases[3],8},{aliases[4],8},
        {const_cast<const float*>(live_scale_00e1301c),4}};
    check_separation(regions,sizeof(regions)/sizeof(regions[0]),3);
    data_=&data;normalization_=&normalization;live_scale_=live_scale_00e1301c;
    for(std::size_t i=0;i<5;++i)aliases_[i]=aliases[i];
}

void write_native_gui_frame_box_geometry_00acf9b0(void* actual_box,
    void* actual_stream,const void* actual_size,
    const NativeGuiFrameBoxGeometryContext& context,
    NativeGuiFrameBoxGeometryFrame& frame) {
    NativeGuiFrameBoxGeometryAccess::run(actual_box,actual_stream,actual_size,context,frame);
}
} // namespace bsp
