#include "bsp/gui_section_runtime.hpp"
#include "bsp/gui_timed_entry_types.hpp"
#include "bsp/native_render_batch_keys.hpp"
#include <cmath>
#include <cstring>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
struct SectionOperation {
    std::uint32_t& calls;
    bool& failed;
    int exceptions;
    SectionOperation(std::uint32_t& count, bool& failure)
        : calls(count), failed(failure), exceptions(std::uncaught_exceptions()) { ++calls; }
    ~SectionOperation() {
        if (std::uncaught_exceptions() > exceptions) failed = true;
        --calls;
    }
};
template<class Function> Function current_slot(void* object, std::size_t offset) {
    require(object != nullptr, "Section requires a current actual renderer");
    const auto* table = *static_cast<const std::uintptr_t* const*>(object);
    return reinterpret_cast<Function>(table[offset / 4]);
}
void require_logical_profile(const void* actual, std::uint32_t profile) {
    require(actual != nullptr, "Section requires an actual logical buffer");
    std::uint32_t current;
    std::memcpy(&current, actual, sizeof(current));
    require(current == profile, "Section logical buffer current profile has no supported mapping dispatch");
}

// Native stack temporaries and live aliases only. The staged kernel contains
// no throwing calls. Mapping calls remain in ordinary C++ in native order.
struct SectionKernel {
    GuiSectionFields* fields;                         // 00
    const GuiWidgetSize* size;                        // 04
    const bool* bounds_enabled;                       // 08
    const volatile float* one;                        // 0c
    const volatile double* half;                      // 10
    const volatile double* sectors;                   // 14
    const volatile double* epsilon;                   // 18
    const volatile double* two_pi;                    // 1c
    const volatile double* angular_step;              // 20
    const volatile std::uint32_t* conversion;          // 24
    void* vertex;                                     // 28
    void* index;                                      // 2c
    void* indices;                                   // 30
    std::uint32_t stage;                              // 34
    std::uint32_t scratch[20];                        // 38 (native ESP+10..5f)
};
static_assert(offsetof(SectionKernel, scratch) == 0x38);

// ABF8D6..AC007E, six stages divided ONLY at native map/unmap boundaries.
// All n_ADDRESS labels refer to original instructions. Added PUSH/POP EAX
// loads dereference live aliases; they do not alter native flags or x87.
__declspec(naked) void __fastcall section_numeric(SectionKernel&) noexcept {
    __asm {
        push ecx
        sub esp, 0x50
        push ebx
        push ebp
        push esi
        push edi
        lea edi, [esp + 0x10]
        lea esi, [ecx + 0x38]
        mov ecx, 20
        rep movsd
        mov ecx, dword ptr [esp + 0x60]
        mov edi, dword ptr [ecx]
        mov esi, dword ptr [ecx + 0x28]
        mov ebx, dword ptr [ecx + 0x2c]
        mov eax, dword ptr [ecx + 0x30]
        cmp dword ptr [ecx + 0x34], 0
        je kernel_stage_0
        cmp dword ptr [ecx + 0x34], 1
        je kernel_stage_1
        cmp dword ptr [ecx + 0x34], 2
        je kernel_stage_2
        cmp dword ptr [ecx + 0x34], 3
        je kernel_stage_3
        cmp dword ptr [ecx + 0x34], 4
        je kernel_stage_4
        cmp dword ptr [ecx + 0x34], 5
        je kernel_stage_5
        jmp kernel_exit
    kernel_stage_0:
        // 00abf8d6: fld dword ptr [edi + 0xf4]
        fld dword ptr [edi + 0x8]
        // 00abf8df: fstp dword ptr [esp + 0x3c]
        fstp dword ptr [esp + 0x3c]
        // 00abf8e3: fld dword ptr [esp + 0x3c]
        fld dword ptr [esp + 0x3c]
        // 00abf8e9: fld st(0)
        fld st(0)
        // 00abf8eb: mov dword ptr [esp + 0x28], ebx
        mov dword ptr [esp + 0x28], ebx
        // 00abf8ef: fld qword ptr [0xcf0058]
        push eax
        mov eax, dword ptr [esp + 0x64]
        mov eax, dword ptr [eax + 0x14]
        fld qword ptr [eax]
        pop eax
        // 00abf8f5: fmul st(1), st(0)
        fmul st(1), st(0)
        // 00abf8f7: fxch st(1)
        fxch st(1)
        // 00abf8f9: call 0xbf7420
        mov eax, dword ptr [esp + 0x60]
        mov ecx, dword ptr [eax + 0x24]
        call native_crt_truncate_st0_00bf7420
        // 00abf8fe: mov ecx, eax
        mov ecx, eax
        // 00abf900: mov dword ptr [esp + 0x3c], ecx
        mov dword ptr [esp + 0x3c], ecx
        // 00abf904: fild dword ptr [esp + 0x3c]
        fild dword ptr [esp + 0x3c]
        // 00abf908: mov dword ptr [edi + 0x128], ecx
        mov dword ptr [edi + 0x3c], ecx
        // 00abf90e: fdivrp st(1)
        fdivrp st(1), st(0)
        // 00abf910: fsubr st(1)
        fsubr st(0), st(1)
        // 00abf912: fstp dword ptr [esp + 0x3c]
        fstp dword ptr [esp + 0x3c]
        // 00abf916: fld qword ptr [0xce3c70]
        push eax
        mov eax, dword ptr [esp + 0x64]
        mov eax, dword ptr [eax + 0x18]
        fld qword ptr [eax]
        pop eax
        // 00abf91c: fld dword ptr [esp + 0x3c]
        fld dword ptr [esp + 0x3c]
        // 00abf920: fcompi st(1)
        fcomip st(0), st(1)
        // 00abf922: fstp st(0)
        fstp st(0)
        // 00abf924: jbe 0xabf934
        jbe n_00abf934
        // 00abf926: cmp ecx, 0x48
        cmp ecx, 0x48
        // 00abf929: jge 0xabf934
        jge n_00abf934
        // 00abf92b: add ecx, 1
        add ecx, 1
        // 00abf92e: mov dword ptr [edi + 0x128], ecx
        mov dword ptr [edi + 0x3c], ecx
    n_00abf934: // cmp byte ptr [edi + 0x74], 0
        push eax
        mov eax, dword ptr [esp + 0x64]
        mov eax, dword ptr [eax + 8]
        cmp byte ptr [eax], 0
        pop eax
        // 00abf938: fmul qword ptr [0xce3828]
        push eax
        mov eax, dword ptr [esp + 0x64]
        mov eax, dword ptr [eax + 0x1c]
        fmul qword ptr [eax]
        pop eax
        // 00abf93e: fstp dword ptr [esp + 0x20]
        fstp dword ptr [esp + 0x20]
        jmp kernel_exit
    kernel_stage_1:
        // 00abf965: fld dword ptr [edi + 0x124]
        fld dword ptr [edi + 0x38]
        // 00abf96d: fadd dword ptr [edi + 0x11c]
        fadd dword ptr [edi + 0x30]
        // 00abf976: fld qword ptr [0xd7a280]
        push eax
        mov eax, dword ptr [esp + 0x64]
        mov eax, dword ptr [eax + 0x10]
        fld qword ptr [eax]
        pop eax
        // 00abf97e: fmul st(1), st(0)
        fmul st(1), st(0)
        // 00abf982: fxch st(1)
        fxch st(1)
        // 00abf988: fstp dword ptr [esp + 0x30]
        fstp dword ptr [esp + 0x24]
        // 00abf98c: fld dword ptr [edi + 0x120]
        fld dword ptr [edi + 0x34]
        // 00abf992: fadd dword ptr [edi + 0x118]
        fadd dword ptr [edi + 0x2c]
        // 00abf998: fmulp st(1)
        fmulp st(1), st(0)
        // 00abf99a: fstp dword ptr [esp + 0x38]
        fstp dword ptr [esp + 0x2c]
        jmp kernel_exit
    kernel_stage_2:
        // 00abf9a0: fld dword ptr [edi + 0x11c]
        fld dword ptr [edi + 0x30]
        // 00abf9a6: mov eax, dword ptr [esi + 0x10]
        mov eax, dword ptr [esi + 0x10]
        // 00abf9a9: fsub dword ptr [edi + 0x124]
        fsub dword ptr [edi + 0x38]
        // 00abf9af: add eax, dword ptr [esi + 8]
        add eax, dword ptr [esi + 8]
        // 00abf9b2: fld qword ptr [0xd7a280]
        push eax
        mov eax, dword ptr [esp + 0x64]
        mov eax, dword ptr [eax + 0x10]
        fld qword ptr [eax]
        pop eax
        // 00abf9b8: xorps xmm3, xmm3
        xorps xmm3, xmm3
        // 00abf9bb: fmul st(1), st(0)
        fmul st(1), st(0)
        // 00abf9bd: movss xmm2, dword ptr [esp + 0x2c]
        movss xmm2, dword ptr [esp + 0x2c]
        // 00abf9c3: fxch st(1)
        fxch st(1)
        // 00abf9c5: movss dword ptr [esp + 0x14], xmm3
        movss dword ptr [esp + 0x14], xmm3
        // 00abf9cb: fstp dword ptr [esp + 0x40]
        fstp dword ptr [esp + 0x40]
        // 00abf9cf: fld dword ptr [edi + 0x120]
        fld dword ptr [edi + 0x34]
        // 00abf9d5: fsub dword ptr [edi + 0x118]
        fsub dword ptr [edi + 0x2c]
        // 00abf9db: fmul st(1)
        fmul st(0), st(1)
        // 00abf9dd: fstp dword ptr [esp + 0x44]
        fstp dword ptr [esp + 0x44]
        // 00abf9e1: fld dword ptr [edi + 0x20]
        push eax
        mov eax, dword ptr [esp + 0x64]
        mov eax, dword ptr [eax + 4]
        fld dword ptr [eax]
        pop eax
        // 00abf9e4: fmul st(1)
        fmul st(0), st(1)
        // 00abf9e6: fstp dword ptr [esp + 0x4c]
        fstp dword ptr [esp + 0x4c]
        // 00abf9ea: fmul dword ptr [edi + 0x24]
        push eax
        mov eax, dword ptr [esp + 0x64]
        mov eax, dword ptr [eax + 4]
        fmul dword ptr [eax + 4]
        pop eax
        // 00abf9ed: movss dword ptr [eax + 8], xmm3
        movss dword ptr [eax + 8], xmm3
        // 00abf9f2: fstp dword ptr [esp + 0x48]
        fstp dword ptr [esp + 0x48]
        // 00abf9f6: fld dword ptr [esp + 0x4c]
        fld dword ptr [esp + 0x4c]
        // 00abf9fa: fstp dword ptr [eax]
        fstp dword ptr [eax]
        // 00abf9fc: fld dword ptr [esp + 0x48]
        fld dword ptr [esp + 0x48]
        // 00abfa00: fstp dword ptr [eax + 4]
        fstp dword ptr [eax + 4]
        // 00abfa03: mov eax, dword ptr [edi + 0x110]
        mov eax, dword ptr [edi + 0x24]
        // 00abfa09: sub eax, 0
        sub eax, 0
        // 00abfa0c: je 0xabfdcc
        je n_00abfdcc
        // 00abfa12: sub eax, 2
        sub eax, 2
        // 00abfa15: jne 0xabfde3
        jne n_00abfde3
        // 00abfa1b: movss xmm0, dword ptr [esp + 0x24]
        movss xmm0, dword ptr [esp + 0x24]
        // 00abfa21: jmp 0xabfdd4
        jmp n_00abfdd4
    n_00abfdcc: // movss xmm0, dword ptr [edi + 0x124]
        movss xmm0, dword ptr [edi + 0x38]
    n_00abfdd4: // mov eax, dword ptr [esi + 0x28]
        mov eax, dword ptr [esi + 0x28]
        // 00abfdd7: add eax, dword ptr [esi + 8]
        add eax, dword ptr [esi + 8]
        // 00abfdda: movss dword ptr [eax + 4], xmm0
        movss dword ptr [eax + 4], xmm0
        // 00abfddf: movss dword ptr [eax], xmm2
        movss dword ptr [eax], xmm2
    n_00abfde3: // mov ecx, dword ptr [esi + 0x34]
        mov ecx, dword ptr [esi + 0x34]
        // 00abfde6: test ecx, ecx
        test ecx, ecx
        // 00abfde8: movss xmm1, dword ptr [0xd7a24c]
        push eax
        mov eax, dword ptr [esp + 0x64]
        mov eax, dword ptr [eax + 0xc]
        movss xmm1, dword ptr [eax]
        pop eax
        // 00abfdf0: mov al, 0xff
        mov al, 0xff
        // 00abfdf2: mov byte ptr [esp + 0x1e], al
        mov byte ptr [esp + 0x1e], al
        // 00abfdf6: mov byte ptr [esp + 0x1d], al
        mov byte ptr [esp + 0x1d], al
        // 00abfdfa: mov byte ptr [esp + 0x1c], al
        mov byte ptr [esp + 0x1c], al
        // 00abfdfe: mov byte ptr [esp + 0x1f], al
        mov byte ptr [esp + 0x1f], al
        // 00abfe02: jl 0xabfe10
        jl n_00abfe10
        // 00abfe04: mov edx, dword ptr [esi + 8]
        mov edx, dword ptr [esi + 8]
        // 00abfe07: mov ebp, dword ptr [esp + 0x1c]
        mov ebp, dword ptr [esp + 0x1c]
        // 00abfe0b: mov dword ptr [ecx + edx], ebp
        mov dword ptr [ecx + edx], ebp
        // 00abfe0e: jmp 0xabfe33
        jmp n_00abfe33
    n_00abfe10: // mov edx, dword ptr [esi + 0x38]
        mov edx, dword ptr [esi + 0x38]
        // 00abfe13: mov ecx, dword ptr [esi + 8]
        mov ecx, dword ptr [esi + 8]
        // 00abfe16: movss dword ptr [ecx + edx], xmm1
        movss dword ptr [ecx + edx], xmm1
        // 00abfe1b: mov edx, dword ptr [esi + 0x3c]
        mov edx, dword ptr [esi + 0x3c]
        // 00abfe1e: movss dword ptr [ecx + edx], xmm1
        movss dword ptr [ecx + edx], xmm1
        // 00abfe23: mov edx, dword ptr [esi + 0x40]
        mov edx, dword ptr [esi + 0x40]
        // 00abfe26: movss dword ptr [ecx + edx], xmm1
        movss dword ptr [ecx + edx], xmm1
        // 00abfe2b: mov edx, dword ptr [esi + 0x44]
        mov edx, dword ptr [esi + 0x44]
        // 00abfe2e: movss dword ptr [ecx + edx], xmm1
        movss dword ptr [ecx + edx], xmm1
    n_00abfe33: // mov edx, dword ptr [edi + 0x128]
        mov edx, dword ptr [edi + 0x3c]
        // 00abfe39: mov ecx, 1
        mov ecx, 1
        // 00abfe3e: add edx, 2
        add edx, 2
        // 00abfe41: cmp edx, ecx
        cmp edx, ecx
        // 00abfe43: jle 0xac0008
        jle kernel_exit
        // 00abfe49: movss xmm4, dword ptr [esp + 0x20]
        movss xmm4, dword ptr [esp + 0x20]
        // 00abfe4f: fld dword ptr [esp + 0x14]
        fld dword ptr [esp + 0x14]
        // 00abfe53: mov byte ptr [esp + 0x1e], al
        mov byte ptr [esp + 0x1e], al
        // 00abfe57: mov byte ptr [esp + 0x1d], al
        mov byte ptr [esp + 0x1d], al
        // 00abfe5b: mov byte ptr [esp + 0x1c], al
        mov byte ptr [esp + 0x1c], al
        // 00abfe5f: mov byte ptr [esp + 0x1f], al
        mov byte ptr [esp + 0x1f], al
        // 00abfe63: mov edx, dword ptr [esp + 0x1c]
        mov edx, dword ptr [esp + 0x1c]
    n_00abfe67: // cmp byte ptr [edi + 0x10c], 0
        cmp byte ptr [edi + 0x20], 0
        // 00abfe6e: je 0xabfe78
        je n_00abfe78
        // 00abfe70: fadd dword ptr [edi + 0xf8]
        fadd dword ptr [edi + 0xc]
        // 00abfe76: jmp 0xabfe7e
        jmp n_00abfe7e
    n_00abfe78: // fsubr dword ptr [edi + 0xf8]
        fsubr dword ptr [edi + 0xc]
    n_00abfe7e: // fstp dword ptr [esp + 0x1c]
        fstp dword ptr [esp + 0x1c]
        // 00abfe82: fld dword ptr [esp + 0x1c]
        fld dword ptr [esp + 0x1c]
        // 00abfe86: fsin
        fsin
        // 00abfe88: fstp dword ptr [esp + 0x3c]
        fstp dword ptr [esp + 0x3c]
        // 00abfe8c: fld dword ptr [esp + 0x3c]
        fld dword ptr [esp + 0x3c]
        // 00abfe90: fld qword ptr [0xd7a280]
        push eax
        mov eax, dword ptr [esp + 0x64]
        mov eax, dword ptr [eax + 0x10]
        fld qword ptr [eax]
        pop eax
        // 00abfe96: fmul st(1), st(0)
        fmul st(1), st(0)
        // 00abfe98: faddp st(1)
        faddp st(1), st(0)
        // 00abfe9a: fstp dword ptr [esp + 0x4c]
        fstp dword ptr [esp + 0x4c]
        // 00abfe9e: fld dword ptr [esp + 0x1c]
        fld dword ptr [esp + 0x1c]
        // 00abfea2: fcos
        fcos
        // 00abfea4: fstp dword ptr [esp + 0x38]
        fstp dword ptr [esp + 0x38]
        // 00abfea8: fld dword ptr [esp + 0x38]
        fld dword ptr [esp + 0x38]
        // 00abfeac: mov eax, dword ptr [esi + 0xc]
        mov eax, dword ptr [esi + 0xc]
        // 00abfeaf: fld qword ptr [0xd7a280]
        push eax
        mov eax, dword ptr [esp + 0x64]
        mov eax, dword ptr [eax + 0x10]
        fld qword ptr [eax]
        pop eax
        // 00abfeb5: imul eax, ecx
        imul eax, ecx
        // 00abfeb8: fmul st(1), st(0)
        fmul st(1), st(0)
        // 00abfeba: fsubrp st(1)
        fsubrp st(1), st(0)
        // 00abfebc: fstp dword ptr [esp + 0x48]
        fstp dword ptr [esp + 0x48]
        // 00abfec0: fld dword ptr [edi + 0x20]
        push eax
        mov eax, dword ptr [esp + 0x64]
        mov eax, dword ptr [eax + 4]
        fld dword ptr [eax]
        pop eax
        // 00abfec3: add eax, dword ptr [esi + 0x10]
        add eax, dword ptr [esi + 0x10]
        // 00abfec6: fmul dword ptr [esp + 0x4c]
        fmul dword ptr [esp + 0x4c]
        // 00abfeca: add eax, dword ptr [esi + 8]
        add eax, dword ptr [esi + 8]
        // 00abfecd: fstp dword ptr [esp + 0x4c]
        fstp dword ptr [esp + 0x4c]
        // 00abfed1: fld dword ptr [edi + 0x24]
        push eax
        mov eax, dword ptr [esp + 0x64]
        mov eax, dword ptr [eax + 4]
        fld dword ptr [eax + 4]
        pop eax
        // 00abfed4: fmul dword ptr [esp + 0x48]
        fmul dword ptr [esp + 0x48]
        // 00abfed8: movss dword ptr [eax + 8], xmm3
        movss dword ptr [eax + 8], xmm3
        // 00abfedd: fstp dword ptr [esp + 0x48]
        fstp dword ptr [esp + 0x48]
        // 00abfee1: fld dword ptr [esp + 0x4c]
        fld dword ptr [esp + 0x4c]
        // 00abfee5: fstp dword ptr [eax]
        fstp dword ptr [eax]
        // 00abfee7: fld dword ptr [esp + 0x48]
        fld dword ptr [esp + 0x48]
        // 00abfeeb: fstp dword ptr [eax + 4]
        fstp dword ptr [eax + 4]
        // 00abfeee: mov eax, dword ptr [esi + 0x34]
        mov eax, dword ptr [esi + 0x34]
        // 00abfef1: test eax, eax
        test eax, eax
        // 00abfef3: jl 0xabff05
        jl n_00abff05
        // 00abfef5: mov ebp, dword ptr [esi + 0xc]
        mov ebp, dword ptr [esi + 0xc]
        // 00abfef8: imul ebp, ecx
        imul ebp, ecx
        // 00abfefb: add ebp, eax
        add ebp, eax
        // 00abfefd: mov eax, dword ptr [esi + 8]
        mov eax, dword ptr [esi + 8]
        // 00abff00: mov dword ptr [eax + ebp], edx
        mov dword ptr [eax + ebp], edx
        // 00abff03: jmp 0xabff2e
        jmp n_00abff2e
    n_00abff05: // mov eax, dword ptr [esi + 0xc]
        mov eax, dword ptr [esi + 0xc]
        // 00abff08: mov ebp, dword ptr [esi + 0x38]
        mov ebp, dword ptr [esi + 0x38]
        // 00abff0b: imul eax, ecx
        imul eax, ecx
        // 00abff0e: add eax, dword ptr [esi + 8]
        add eax, dword ptr [esi + 8]
        // 00abff11: movss dword ptr [eax + ebp], xmm1
        movss dword ptr [eax + ebp], xmm1
        // 00abff16: mov ebp, dword ptr [esi + 0x3c]
        mov ebp, dword ptr [esi + 0x3c]
        // 00abff19: movss dword ptr [eax + ebp], xmm1
        movss dword ptr [eax + ebp], xmm1
        // 00abff1e: mov ebp, dword ptr [esi + 0x40]
        mov ebp, dword ptr [esi + 0x40]
        // 00abff21: movss dword ptr [eax + ebp], xmm1
        movss dword ptr [eax + ebp], xmm1
        // 00abff26: mov ebp, dword ptr [esi + 0x44]
        mov ebp, dword ptr [esi + 0x44]
        // 00abff29: movss dword ptr [eax + ebp], xmm1
        movss dword ptr [eax + ebp], xmm1
    n_00abff2e: // mov eax, dword ptr [edi + 0x110]
        mov eax, dword ptr [edi + 0x24]
        // 00abff34: sub eax, 0
        sub eax, 0
        // 00abff37: je 0xabffad
        je n_00abffad
        // 00abff39: sub eax, 2
        sub eax, 2
        // 00abff3c: jne 0xabffca
        jne n_00abffca
        // 00abff42: fld dword ptr [edi + 0xfc]
        fld dword ptr [edi + 0x10]
        // 00abff48: fadd dword ptr [esp + 0x1c]
        fadd dword ptr [esp + 0x1c]
        // 00abff4c: fstp dword ptr [esp + 0x4c]
        fstp dword ptr [esp + 0x4c]
        // 00abff50: fld dword ptr [esp + 0x4c]
        fld dword ptr [esp + 0x4c]
        // 00abff54: fsin
        fsin
        // 00abff56: fstp dword ptr [esp + 0x34]
        fstp dword ptr [esp + 0x34]
        // 00abff5a: fld dword ptr [esp + 0x34]
        fld dword ptr [esp + 0x34]
        // 00abff5e: fmul dword ptr [esp + 0x44]
        fmul dword ptr [esp + 0x44]
        // 00abff62: fadd dword ptr [esp + 0x2c]
        fadd dword ptr [esp + 0x2c]
        // 00abff66: fstp dword ptr [esp + 0x48]
        fstp dword ptr [esp + 0x48]
        // 00abff6a: fld dword ptr [edi + 0xfc]
        fld dword ptr [edi + 0x10]
        // 00abff70: fadd dword ptr [esp + 0x1c]
        fadd dword ptr [esp + 0x1c]
        // 00abff74: fstp dword ptr [esp + 0x4c]
        fstp dword ptr [esp + 0x4c]
        // 00abff78: fld dword ptr [esp + 0x4c]
        fld dword ptr [esp + 0x4c]
        // 00abff7c: fcos
        fcos
        // 00abff7e: fstp dword ptr [esp + 0x28]
        fstp dword ptr [esp + 0x28]
        // 00abff82: mov eax, dword ptr [esi + 0xc]
        mov eax, dword ptr [esi + 0xc]
        // 00abff85: fld dword ptr [esp + 0x28]
        fld dword ptr [esp + 0x28]
        // 00abff89: fmul dword ptr [esp + 0x40]
        fmul dword ptr [esp + 0x40]
        // 00abff8d: imul eax, ecx
        imul eax, ecx
        // 00abff90: fadd dword ptr [esp + 0x24]
        fadd dword ptr [esp + 0x24]
        // 00abff94: fstp dword ptr [esp + 0x4c]
        fstp dword ptr [esp + 0x4c]
        // 00abff98: fld dword ptr [esp + 0x48]
        fld dword ptr [esp + 0x48]
        // 00abff9c: add eax, dword ptr [esi + 0x28]
        add eax, dword ptr [esi + 0x28]
        // 00abff9f: add eax, dword ptr [esi + 8]
        add eax, dword ptr [esi + 8]
        // 00abffa2: fstp dword ptr [eax]
        fstp dword ptr [eax]
        // 00abffa4: fld dword ptr [esp + 0x4c]
        fld dword ptr [esp + 0x4c]
        // 00abffa8: fstp dword ptr [eax + 4]
        fstp dword ptr [eax + 4]
        // 00abffab: jmp 0xabffca
        jmp n_00abffca
    n_00abffad: // mov eax, dword ptr [esi + 0xc]
        mov eax, dword ptr [esi + 0xc]
        // 00abffb0: movss xmm0, dword ptr [edi + 0x11c]
        movss xmm0, dword ptr [edi + 0x30]
        // 00abffb8: imul eax, ecx
        imul eax, ecx
        // 00abffbb: add eax, dword ptr [esi + 0x28]
        add eax, dword ptr [esi + 0x28]
        // 00abffbe: add eax, dword ptr [esi + 8]
        add eax, dword ptr [esi + 8]
        // 00abffc1: movss dword ptr [eax], xmm2
        movss dword ptr [eax], xmm2
        // 00abffc5: movss dword ptr [eax + 4], xmm0
        movss dword ptr [eax + 4], xmm0
    n_00abffca: // fld dword ptr [esp + 0x14]
        fld dword ptr [esp + 0x14]
        // 00abffce: fadd qword ptr [0xd5c970]
        push eax
        mov eax, dword ptr [esp + 0x64]
        mov eax, dword ptr [eax + 0x20]
        fadd qword ptr [eax]
        pop eax
        // 00abffd4: fstp dword ptr [esp + 0x14]
        fstp dword ptr [esp + 0x14]
        // 00abffd8: fld dword ptr [esp + 0x20]
        fld dword ptr [esp + 0x20]
        // 00abffdc: fld dword ptr [esp + 0x14]
        fld dword ptr [esp + 0x14]
        // 00abffe0: fcomi st(1)
        fcomi st(0), st(1)
        // 00abffe2: fstp st(1)
        fstp st(1)
        // 00abffe4: jbe 0xabfff2
        jbe n_00abfff2
        // 00abffe6: movss dword ptr [esp + 0x14], xmm4
        movss dword ptr [esp + 0x14], xmm4
        // 00abffec: fstp st(0)
        fstp st(0)
        // 00abffee: fld dword ptr [esp + 0x14]
        fld dword ptr [esp + 0x14]
    n_00abfff2: // mov eax, dword ptr [edi + 0x128]
        mov eax, dword ptr [edi + 0x3c]
        // 00abfff8: add ecx, 1
        add ecx, 1
        // 00abfffb: add eax, 2
        add eax, 2
        // 00abfffe: cmp ecx, eax
        cmp ecx, eax
        // 00ac0000: jl 0xabfe67
        jl n_00abfe67
        // 00ac0006: fstp st(0)
        fstp st(0)
        jmp kernel_exit
    kernel_stage_3:
        // 00abfa38: fld dword ptr [edi + 0x108]
        fld dword ptr [edi + 0x1c]
        // 00abfa3e: fld1
        fld1
        // 00abfa40: mov eax, dword ptr [edi + 0x128]
        mov eax, dword ptr [edi + 0x3c]
        // 00abfa46: fsubrp st(1)
        fsubrp st(1), st(0)
        // 00abfa48: mov dword ptr [esp + 0x34], eax
        mov dword ptr [esp + 0x34], eax
        // 00abfa4c: xorps xmm3, xmm3
        xorps xmm3, xmm3
        // 00abfa4f: xor ebp, ebp
        xor ebp, ebp
        // 00abfa51: add eax, 1
        add eax, 1
        // 00abfa54: test eax, eax
        test eax, eax
        // 00abfa56: fstp dword ptr [esp + 0x44]
        fstp dword ptr [esp + 0x44]
        // 00abfa5a: fld dword ptr [edi + 0x118]
        fld dword ptr [edi + 0x2c]
        // 00abfa60: movss dword ptr [esp + 0x14], xmm3
        movss dword ptr [esp + 0x14], xmm3
        // 00abfa66: fstp dword ptr [esp + 0x3c]
        fstp dword ptr [esp + 0x3c]
        // 00abfa6a: fld dword ptr [edi + 0x120]
        fld dword ptr [edi + 0x34]
        // 00abfa70: fld dword ptr [esp + 0x3c]
        fld dword ptr [esp + 0x3c]
        // 00abfa74: fld st(0)
        fld st(0)
        // 00abfa76: fsubp st(2)
        fsubp st(2), st(0)
        // 00abfa78: fxch st(1)
        fxch st(1)
        // 00abfa7a: fstp dword ptr [esp + 0x38]
        fstp dword ptr [esp + 0x38]
        // 00abfa7e: fld dword ptr [edi + 0x100]
        fld dword ptr [edi + 0x14]
        // 00abfa84: fstp dword ptr [esp + 0x3c]
        fstp dword ptr [esp + 0x3c]
        // 00abfa88: fld dword ptr [edi + 0x104]
        fld dword ptr [edi + 0x18]
        // 00abfa8e: fld dword ptr [esp + 0x3c]
        fld dword ptr [esp + 0x3c]
        // 00abfa92: mov dword ptr [esp + 0x3c], ebp
        mov dword ptr [esp + 0x3c], ebp
        // 00abfa96: fld st(0)
        fld st(0)
        // 00abfa98: fsubp st(2)
        fsubp st(2), st(0)
        // 00abfa9a: fld dword ptr [esp + 0x38]
        fld dword ptr [esp + 0x38]
        // 00abfa9e: fld st(0)
        fld st(0)
        // 00abfaa0: fmulp st(3)
        fmulp st(3), st(0)
        // 00abfaa2: fild dword ptr [esp + 0x34]
        fild dword ptr [esp + 0x34]
        // 00abfaa6: fdivp st(3)
        fdivp st(3), st(0)
        // 00abfaa8: fxch st(2)
        fxch st(2)
        // 00abfaaa: fstp dword ptr [esp + 0x4c]
        fstp dword ptr [esp + 0x4c]
        // 00abfaae: fmulp st(1)
        fmulp st(1), st(0)
        // 00abfab0: faddp st(1)
        faddp st(1), st(0)
        // 00abfab2: fstp dword ptr [esp + 0x2c]
        fstp dword ptr [esp + 0x2c]
        // 00abfab6: jle 0xabfd25
        jle kernel_exit
        // 00abfabc: movss xmm4, dword ptr [esp + 0x20]
        movss xmm4, dword ptr [esp + 0x20]
        // 00abfac2: fld dword ptr [esp + 0x14]
        fld dword ptr [esp + 0x14]
        // 00abfac6: movss xmm2, dword ptr [0xd7a24c]
        push eax
        mov eax, dword ptr [esp + 0x64]
        mov eax, dword ptr [eax + 0xc]
        movss xmm2, dword ptr [eax]
        pop eax
        // 00abface: mov al, 0xff
        mov al, 0xff
        // 00abfad0: mov byte ptr [esp + 0x1a], al
        mov byte ptr [esp + 0x1a], al
        // 00abfad4: mov byte ptr [esp + 0x19], al
        mov byte ptr [esp + 0x19], al
        // 00abfad8: mov byte ptr [esp + 0x18], al
        mov byte ptr [esp + 0x18], al
        // 00abfadc: mov byte ptr [esp + 0x1b], al
        mov byte ptr [esp + 0x1b], al
        // 00abfae0: mov byte ptr [esp + 0x1e], al
        mov byte ptr [esp + 0x1e], al
        // 00abfae4: mov byte ptr [esp + 0x1d], al
        mov byte ptr [esp + 0x1d], al
        // 00abfae8: mov byte ptr [esp + 0x1c], al
        mov byte ptr [esp + 0x1c], al
        // 00abfaec: mov byte ptr [esp + 0x1f], al
        mov byte ptr [esp + 0x1f], al
        // 00abfaf0: lea edx, [ebp + 1]
        lea edx, [ebp + 1]
    n_00abfaf3: // cmp byte ptr [edi + 0x10c], 0
        cmp byte ptr [edi + 0x20], 0
        // 00abfafa: je 0xabfb04
        je n_00abfb04
        // 00abfafc: fadd dword ptr [edi + 0xf8]
        fadd dword ptr [edi + 0xc]
        // 00abfb02: jmp 0xabfb0a
        jmp n_00abfb0a
    n_00abfb04: // fsubr dword ptr [edi + 0xf8]
        fsubr dword ptr [edi + 0xc]
    n_00abfb0a: // fstp dword ptr [esp + 0x24]
        fstp dword ptr [esp + 0x24]
        // 00abfb0e: fld dword ptr [esp + 0x24]
        fld dword ptr [esp + 0x24]
        // 00abfb12: fsin
        fsin
        // 00abfb14: fstp dword ptr [esp + 0x34]
        fstp dword ptr [esp + 0x34]
        // 00abfb18: fld dword ptr [esp + 0x34]
        fld dword ptr [esp + 0x34]
        // 00abfb1c: fmul qword ptr [0xd7a280]
        push eax
        mov eax, dword ptr [esp + 0x64]
        mov eax, dword ptr [eax + 0x10]
        fmul qword ptr [eax]
        pop eax
        // 00abfb22: fstp dword ptr [esp + 0x40]
        fstp dword ptr [esp + 0x40]
        // 00abfb26: fld dword ptr [esp + 0x24]
        fld dword ptr [esp + 0x24]
        // 00abfb2a: fcos
        fcos
        // 00abfb2c: fstp dword ptr [esp + 0x38]
        fstp dword ptr [esp + 0x38]
        // 00abfb30: fld dword ptr [esp + 0x38]
        fld dword ptr [esp + 0x38]
        // 00abfb34: mov eax, dword ptr [esi + 0xc]
        mov eax, dword ptr [esi + 0xc]
        // 00abfb37: fchs
        fchs
        // 00abfb39: lea ecx, [ebp + ebp]
        lea ecx, [ebp + ebp]
        // 00abfb3d: fld qword ptr [0xd7a280]
        push eax
        mov eax, dword ptr [esp + 0x64]
        mov eax, dword ptr [eax + 0x10]
        fld qword ptr [eax]
        pop eax
        // 00abfb43: imul eax, ecx
        imul eax, ecx
        // 00abfb46: fmul st(1), st(0)
        fmul st(1), st(0)
        // 00abfb48: fxch st(1)
        fxch st(1)
        // 00abfb4a: fstp dword ptr [esp + 0x48]
        fstp dword ptr [esp + 0x48]
        // 00abfb4e: fld dword ptr [edi + 0x20]
        push eax
        mov eax, dword ptr [esp + 0x64]
        mov eax, dword ptr [eax + 4]
        fld dword ptr [eax]
        pop eax
        // 00abfb51: fstp dword ptr [esp + 0x24]
        fstp dword ptr [esp + 0x24]
        // 00abfb55: fld dword ptr [esp + 0x24]
        fld dword ptr [esp + 0x24]
        // 00abfb59: fld st(0)
        fld st(0)
        // 00abfb5b: add eax, dword ptr [esi + 0x10]
        add eax, dword ptr [esi + 0x10]
        // 00abfb5e: fld dword ptr [esp + 0x40]
        fld dword ptr [esp + 0x40]
        // 00abfb62: fld st(0)
        fld st(0)
        // 00abfb64: fmulp st(2)
        fmulp st(2), st(0)
        // 00abfb66: fld dword ptr [esp + 0x44]
        fld dword ptr [esp + 0x44]
        // 00abfb6a: add eax, dword ptr [esi + 8]
        add eax, dword ptr [esi + 8]
        // 00abfb6d: fld st(0)
        fld st(0)
        // 00abfb6f: fmulp st(3)
        fmulp st(3), st(0)
        // 00abfb71: fxch st(3)
        fxch st(3)
        // 00abfb73: fmul st(4)
        fmul st(0), st(4)
        // 00abfb75: faddp st(2)
        faddp st(2), st(0)
        // 00abfb77: fxch st(1)
        fxch st(1)
        // 00abfb79: fstp dword ptr [esp + 0x40]
        fstp dword ptr [esp + 0x40]
        // 00abfb7d: fld dword ptr [edi + 0x24]
        push eax
        mov eax, dword ptr [esp + 0x64]
        mov eax, dword ptr [eax + 4]
        fld dword ptr [eax + 4]
        pop eax
        // 00abfb80: movss dword ptr [eax + 8], xmm3
        movss dword ptr [eax + 8], xmm3
        // 00abfb85: fstp dword ptr [esp + 0x24]
        fstp dword ptr [esp + 0x24]
        // 00abfb89: fld dword ptr [esp + 0x24]
        fld dword ptr [esp + 0x24]
        // 00abfb8d: fld st(0)
        fld st(0)
        // 00abfb8f: fld dword ptr [esp + 0x48]
        fld dword ptr [esp + 0x48]
        // 00abfb93: fld st(0)
        fld st(0)
        // 00abfb95: fmulp st(2)
        fmulp st(2), st(0)
        // 00abfb97: fxch st(1)
        fxch st(1)
        // 00abfb99: fmulp st(4)
        fmulp st(4), st(0)
        // 00abfb9b: fxch st(1)
        fxch st(1)
        // 00abfb9d: fmul st(4)
        fmul st(0), st(4)
        // 00abfb9f: faddp st(3)
        faddp st(3), st(0)
        // 00abfba1: fxch st(2)
        fxch st(2)
        // 00abfba3: fstp dword ptr [esp + 0x48]
        fstp dword ptr [esp + 0x48]
        // 00abfba7: fld dword ptr [esp + 0x40]
        fld dword ptr [esp + 0x40]
        // 00abfbab: fstp dword ptr [eax]
        fstp dword ptr [eax]
        // 00abfbad: fld dword ptr [esp + 0x48]
        fld dword ptr [esp + 0x48]
        // 00abfbb1: fstp dword ptr [eax + 4]
        fstp dword ptr [eax + 4]
        // 00abfbb4: mov eax, dword ptr [esi + 0x34]
        mov eax, dword ptr [esi + 0x34]
        // 00abfbb7: test eax, eax
        test eax, eax
        // 00abfbb9: jl 0xabfbd3
        jl n_00abfbd3
        // 00abfbbb: mov ebx, dword ptr [esi + 0xc]
        mov ebx, dword ptr [esi + 0xc]
        // 00abfbbe: mov ebp, dword ptr [esp + 0x18]
        mov ebp, dword ptr [esp + 0x18]
        // 00abfbc2: imul ebx, ecx
        imul ebx, ecx
        // 00abfbc5: add ebx, eax
        add ebx, eax
        // 00abfbc7: mov eax, dword ptr [esi + 8]
        mov eax, dword ptr [esi + 8]
        // 00abfbca: mov dword ptr [ebx + eax], ebp
        mov dword ptr [ebx + eax], ebp
        // 00abfbcd: mov ebp, dword ptr [esp + 0x3c]
        mov ebp, dword ptr [esp + 0x3c]
        // 00abfbd1: jmp 0xabfbfc
        jmp n_00abfbfc
    n_00abfbd3: // mov eax, dword ptr [esi + 0xc]
        mov eax, dword ptr [esi + 0xc]
        // 00abfbd6: mov ebx, dword ptr [esi + 0x38]
        mov ebx, dword ptr [esi + 0x38]
        // 00abfbd9: imul eax, ecx
        imul eax, ecx
        // 00abfbdc: add eax, dword ptr [esi + 8]
        add eax, dword ptr [esi + 8]
        // 00abfbdf: movss dword ptr [eax + ebx], xmm2
        movss dword ptr [eax + ebx], xmm2
        // 00abfbe4: mov ebx, dword ptr [esi + 0x3c]
        mov ebx, dword ptr [esi + 0x3c]
        // 00abfbe7: movss dword ptr [eax + ebx], xmm2
        movss dword ptr [eax + ebx], xmm2
        // 00abfbec: mov ebx, dword ptr [esi + 0x40]
        mov ebx, dword ptr [esi + 0x40]
        // 00abfbef: movss dword ptr [eax + ebx], xmm2
        movss dword ptr [eax + ebx], xmm2
        // 00abfbf4: mov ebx, dword ptr [esi + 0x44]
        mov ebx, dword ptr [esi + 0x44]
        // 00abfbf7: movss dword ptr [eax + ebx], xmm2
        movss dword ptr [eax + ebx], xmm2
    n_00abfbfc: // mov eax, dword ptr [esi + 0xc]
        mov eax, dword ptr [esi + 0xc]
        // 00abfbff: movss xmm0, dword ptr [edi + 0x124]
        movss xmm0, dword ptr [edi + 0x38]
        // 00abfc07: imul eax, ecx
        imul eax, ecx
        // 00abfc0a: add eax, dword ptr [esi + 0x28]
        add eax, dword ptr [esi + 0x28]
        // 00abfc0d: movss xmm1, dword ptr [esp + 0x2c]
        movss xmm1, dword ptr [esp + 0x2c]
        // 00abfc13: mov ebx, dword ptr [esp + 0x28]
        mov ebx, dword ptr [esp + 0x28]
        // 00abfc17: add eax, dword ptr [esi + 8]
        add eax, dword ptr [esi + 8]
        // 00abfc1a: movss dword ptr [eax], xmm1
        movss dword ptr [eax], xmm1
        // 00abfc1e: movss dword ptr [eax + 4], xmm0
        movss dword ptr [eax + 4], xmm0
        // 00abfc23: fld dword ptr [edi + 0x20]
        push eax
        mov eax, dword ptr [esp + 0x64]
        mov eax, dword ptr [eax + 4]
        fld dword ptr [eax]
        pop eax
        // 00abfc26: fstp dword ptr [esp + 0x24]
        fstp dword ptr [esp + 0x24]
        // 00abfc2a: mov eax, dword ptr [esi + 0xc]
        mov eax, dword ptr [esi + 0xc]
        // 00abfc2d: fld dword ptr [esp + 0x24]
        fld dword ptr [esp + 0x24]
        // 00abfc31: imul eax, edx
        imul eax, edx
        // 00abfc34: fld st(0)
        fld st(0)
        // 00abfc36: fmulp st(2)
        fmulp st(2), st(0)
        // 00abfc38: fmul st(3)
        fmul st(0), st(3)
        // 00abfc3a: add eax, dword ptr [esi + 0x10]
        add eax, dword ptr [esi + 0x10]
        // 00abfc3d: faddp st(1)
        faddp st(1), st(0)
        // 00abfc3f: fstp dword ptr [esp + 0x48]
        fstp dword ptr [esp + 0x48]
        // 00abfc43: fld dword ptr [edi + 0x24]
        push eax
        mov eax, dword ptr [esp + 0x64]
        mov eax, dword ptr [eax + 4]
        fld dword ptr [eax + 4]
        pop eax
        // 00abfc46: add eax, dword ptr [esi + 8]
        add eax, dword ptr [esi + 8]
        // 00abfc49: fstp dword ptr [esp + 0x24]
        fstp dword ptr [esp + 0x24]
        // 00abfc4d: fld dword ptr [esp + 0x24]
        fld dword ptr [esp + 0x24]
        // 00abfc51: movss dword ptr [eax + 8], xmm3
        movss dword ptr [eax + 8], xmm3
        // 00abfc56: fld st(0)
        fld st(0)
        // 00abfc58: fmulp st(2)
        fmulp st(2), st(0)
        // 00abfc5a: fmulp st(2)
        fmulp st(2), st(0)
        // 00abfc5c: faddp st(1)
        faddp st(1), st(0)
        // 00abfc5e: fstp dword ptr [esp + 0x40]
        fstp dword ptr [esp + 0x40]
        // 00abfc62: fld dword ptr [esp + 0x48]
        fld dword ptr [esp + 0x48]
        // 00abfc66: fstp dword ptr [eax]
        fstp dword ptr [eax]
        // 00abfc68: fld dword ptr [esp + 0x40]
        fld dword ptr [esp + 0x40]
        // 00abfc6c: fstp dword ptr [eax + 4]
        fstp dword ptr [eax + 4]
        // 00abfc6f: mov eax, dword ptr [esi + 0x34]
        mov eax, dword ptr [esi + 0x34]
        // 00abfc72: test eax, eax
        test eax, eax
        // 00abfc74: jl 0xabfc8e
        jl n_00abfc8e
        // 00abfc76: mov ecx, dword ptr [esi + 0xc]
        mov ecx, dword ptr [esi + 0xc]
        // 00abfc79: mov ebx, dword ptr [esp + 0x1c]
        mov ebx, dword ptr [esp + 0x1c]
        // 00abfc7d: imul ecx, edx
        imul ecx, edx
        // 00abfc80: add ecx, eax
        add ecx, eax
        // 00abfc82: mov eax, dword ptr [esi + 8]
        mov eax, dword ptr [esi + 8]
        // 00abfc85: mov dword ptr [ecx + eax], ebx
        mov dword ptr [ecx + eax], ebx
        // 00abfc88: mov ebx, dword ptr [esp + 0x28]
        mov ebx, dword ptr [esp + 0x28]
        // 00abfc8c: jmp 0xabfcb7
        jmp n_00abfcb7
    n_00abfc8e: // mov eax, dword ptr [esi + 0xc]
        mov eax, dword ptr [esi + 0xc]
        // 00abfc91: mov ecx, dword ptr [esi + 0x38]
        mov ecx, dword ptr [esi + 0x38]
        // 00abfc94: imul eax, edx
        imul eax, edx
        // 00abfc97: add eax, dword ptr [esi + 8]
        add eax, dword ptr [esi + 8]
        // 00abfc9a: movss dword ptr [eax + ecx], xmm2
        movss dword ptr [eax + ecx], xmm2
        // 00abfc9f: mov ecx, dword ptr [esi + 0x3c]
        mov ecx, dword ptr [esi + 0x3c]
        // 00abfca2: movss dword ptr [eax + ecx], xmm2
        movss dword ptr [eax + ecx], xmm2
        // 00abfca7: mov ecx, dword ptr [esi + 0x40]
        mov ecx, dword ptr [esi + 0x40]
        // 00abfcaa: movss dword ptr [eax + ecx], xmm2
        movss dword ptr [eax + ecx], xmm2
        // 00abfcaf: mov ecx, dword ptr [esi + 0x44]
        mov ecx, dword ptr [esi + 0x44]
        // 00abfcb2: movss dword ptr [eax + ecx], xmm2
        movss dword ptr [eax + ecx], xmm2
    n_00abfcb7: // mov eax, dword ptr [esi + 0xc]
        mov eax, dword ptr [esi + 0xc]
        // 00abfcba: fld dword ptr [esp + 0x14]
        fld dword ptr [esp + 0x14]
        // 00abfcbe: fadd qword ptr [0xd5c970]
        push eax
        mov eax, dword ptr [esp + 0x64]
        mov eax, dword ptr [eax + 0x20]
        fadd qword ptr [eax]
        pop eax
        // 00abfcc4: movss xmm0, dword ptr [edi + 0x11c]
        movss xmm0, dword ptr [edi + 0x30]
        // 00abfccc: imul eax, edx
        imul eax, edx
        // 00abfccf: fstp dword ptr [esp + 0x14]
        fstp dword ptr [esp + 0x14]
        // 00abfcd3: fld dword ptr [esp + 0x20]
        fld dword ptr [esp + 0x20]
        // 00abfcd7: fld dword ptr [esp + 0x14]
        fld dword ptr [esp + 0x14]
        // 00abfcdb: add eax, dword ptr [esi + 0x28]
        add eax, dword ptr [esi + 0x28]
        // 00abfcde: add eax, dword ptr [esi + 8]
        add eax, dword ptr [esi + 8]
        // 00abfce1: fcomi st(1)
        fcomi st(0), st(1)
        // 00abfce3: fstp st(1)
        fstp st(1)
        // 00abfce5: movss dword ptr [eax], xmm1
        movss dword ptr [eax], xmm1
        // 00abfce9: movss dword ptr [eax + 4], xmm0
        movss dword ptr [eax + 4], xmm0
        // 00abfcee: jbe 0xabfcfc
        jbe n_00abfcfc
        // 00abfcf0: movss dword ptr [esp + 0x14], xmm4
        movss dword ptr [esp + 0x14], xmm4
        // 00abfcf6: fstp st(0)
        fstp st(0)
        // 00abfcf8: fld dword ptr [esp + 0x14]
        fld dword ptr [esp + 0x14]
    n_00abfcfc: // fld dword ptr [esp + 0x2c]
        fld dword ptr [esp + 0x2c]
        // 00abfd00: mov eax, dword ptr [edi + 0x128]
        mov eax, dword ptr [edi + 0x3c]
        // 00abfd06: fadd dword ptr [esp + 0x4c]
        fadd dword ptr [esp + 0x4c]
        // 00abfd0a: add ebp, 1
        add ebp, 1
        // 00abfd0d: add eax, 1
        add eax, 1
        // 00abfd10: add edx, 2
        add edx, 2
        // 00abfd13: cmp ebp, eax
        cmp ebp, eax
        // 00abfd15: fstp dword ptr [esp + 0x2c]
        fstp dword ptr [esp + 0x2c]
        // 00abfd19: mov dword ptr [esp + 0x3c], ebp
        mov dword ptr [esp + 0x3c], ebp
        // 00abfd1d: jl 0xabfaf3
        jl n_00abfaf3
        // 00abfd23: fstp st(0)
        fstp st(0)
        jmp kernel_exit
    kernel_stage_4:
        // 00ac0023: xor ecx, ecx
        xor ecx, ecx
        // 00ac0025: cmp dword ptr [edi + 0x128], ecx
        cmp dword ptr [edi + 0x3c], ecx
        // 00ac002b: jle 0xac0065
        jle kernel_exit
        // 00ac002d: lea ecx, [ecx]
        lea ecx, [ecx]
    n_00ac0030: // mov word ptr [eax], 0
        mov word ptr [eax], 0
        // 00ac0035: cmp byte ptr [edi + 0x10c], 0
        cmp byte ptr [edi + 0x20], 0
        // 00ac003c: movzx edx, cx
        movzx edx, cx
        // 00ac003f: je 0xac0049
        je n_00ac0049
        // 00ac0041: lea esi, [edx + 1]
        lea esi, [edx + 1]
        // 00ac0044: add edx, 2
        add edx, 2
        // 00ac0047: jmp 0xac004f
        jmp n_00ac004f
    n_00ac0049: // lea esi, [edx + 2]
        lea esi, [edx + 2]
        // 00ac004c: add edx, 1
        add edx, 1
    n_00ac004f: // mov word ptr [eax + 4], dx
        mov word ptr [eax + 4], dx
        // 00ac0053: mov word ptr [eax + 2], si
        mov word ptr [eax + 2], si
        // 00ac0057: add ecx, 1
        add ecx, 1
        // 00ac005a: add eax, 6
        add eax, 6
        // 00ac005d: cmp ecx, dword ptr [edi + 0x128]
        cmp ecx, dword ptr [edi + 0x3c]
        // 00ac0063: jl 0xac0030
        jl n_00ac0030
        jmp kernel_exit
    kernel_stage_5:
        // 00abfd40: xor esi, esi
        xor esi, esi
        // 00abfd42: cmp dword ptr [edi + 0x128], esi
        cmp dword ptr [edi + 0x3c], esi
        // 00abfd48: jle 0xabfdad
        jle kernel_exit
        // 00abfd4a: lea ebx, [ebx]
        lea ebx, [ebx]
    n_00abfd50: // cmp byte ptr [edi + 0x10c], 0
        cmp byte ptr [edi + 0x20], 0
        // 00abfd57: lea ecx, [esi + esi]
        lea ecx, [esi + esi]
        // 00abfd5a: mov word ptr [eax], cx
        mov word ptr [eax], cx
        // 00abfd5d: je 0xabfd80
        je n_00abfd80
        // 00abfd5f: lea edx, [ecx + 1]
        lea edx, [ecx + 1]
        // 00abfd62: mov word ptr [eax + 2], dx
        mov word ptr [eax + 2], dx
        // 00abfd66: lea edx, [ecx + 3]
        lea edx, [ecx + 3]
        // 00abfd69: mov word ptr [eax + 4], dx
        mov word ptr [eax + 4], dx
        // 00abfd6d: add eax, 6
        add eax, 6
        // 00abfd70: mov word ptr [eax], cx
        mov word ptr [eax], cx
        // 00abfd73: add ecx, 2
        add ecx, 2
        // 00abfd76: mov word ptr [eax + 2], dx
        mov word ptr [eax + 2], dx
        // 00abfd7a: mov word ptr [eax + 4], cx
        mov word ptr [eax + 4], cx
        // 00abfd7e: jmp 0xabfd9f
        jmp n_00abfd9f
    n_00abfd80: // lea edx, [ecx + 3]
        lea edx, [ecx + 3]
        // 00abfd83: mov word ptr [eax + 2], dx
        mov word ptr [eax + 2], dx
        // 00abfd87: lea ebp, [ecx + 1]
        lea ebp, [ecx + 1]
        // 00abfd8a: mov word ptr [eax + 4], bp
        mov word ptr [eax + 4], bp
        // 00abfd8e: add eax, 6
        add eax, 6
        // 00abfd91: mov word ptr [eax], cx
        mov word ptr [eax], cx
        // 00abfd94: add ecx, 2
        add ecx, 2
        // 00abfd97: mov word ptr [eax + 2], cx
        mov word ptr [eax + 2], cx
        // 00abfd9b: mov word ptr [eax + 4], dx
        mov word ptr [eax + 4], dx
    n_00abfd9f: // add esi, 1
        add esi, 1
        // 00abfda2: add eax, 6
        add eax, 6
        // 00abfda5: cmp esi, dword ptr [edi + 0x128]
        cmp esi, dword ptr [edi + 0x3c]
        // 00abfdab: jl 0xabfd50
        jl n_00abfd50
        jmp kernel_exit
    kernel_exit:
        lea esi, [esp + 0x10]
        mov edi, dword ptr [esp + 0x60]
        add edi, 0x38
        mov ecx, 20
        rep movsd
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x54
        ret
    }
}

void create_buffers(NativeMeshStorage& mesh, GuiTextBufferServices& services) {
    NativeString format;
    format.assign_0041e870(services.strings, "SimpleColor.mvfm");
    using Declaration = void* (__thiscall*)(void*, NativeString*);
    using Vertex = void* (__thiscall*)(void*, std::uint32_t, std::uint32_t, void*);
    using Index = void* (__thiscall*)(void*, std::uint32_t, std::uint32_t, std::uint32_t);
    void* declaration;
    try {
        void* renderer = services.current_renderer_00f8d394;
        declaration = current_slot<Declaration>(renderer, 0x38)(renderer, &format);
    } catch (...) {
        destroy_native_string_header_0041dd20(&format, services.strings);
        throw;
    }
    destroy_native_string_header_0041dd20(&format, services.strings);
    void* renderer = services.current_renderer_00f8d394;
    void* vertex = current_slot<Vertex>(renderer, 0x5c)(renderer, 0x92, 1, declaration);
    auto& owners = services.geometry.actual_owners();
    set_native_mesh_vertex_stream_00b73bb0(mesh, owners, 0, vertex);
    release_native_render_actual_owner(owners, vertex);
    release_native_render_actual_owner(owners, declaration);
    renderer = services.current_renderer_00f8d394;
    void* index = current_slot<Index>(renderer, 0x60)(renderer, 0x1b0, 1, 0x65);
    set_native_mesh_index_stream_00b73b70(mesh, owners, index);
    release_native_render_actual_owner(owners, index);
}

float normalize_angle(float angle, const GuiSectionConstants& constants) {
    // ABE960 uses the CRT _CIfmod intrinsic with ST1=value, ST0=period.
    // Host CRT fmod supplies the same mathematical library operation; the
    // original CRT's dispatch/error-handler and binary rounding are not claimed.
    float result = static_cast<float>(std::fmod(static_cast<double>(angle),
        constants.two_pi_00ce3828));
    const auto* period = &constants.two_pi_00ce3828;
    const auto* low_angle_pointer = &constants.negative_pi_00ce3d18;
    const auto* high_angle_pointer = &constants.positive_pi_00ce3d28;
    __asm {
        fld result
        fstp result
        fld result
        mov eax, low_angle_pointer
        fld qword ptr [eax]
        fcomip st(0), st(1)
        jc angle_upper
        mov eax, period
        fadd qword ptr [eax]
        fstp result
        jmp angle_done
    angle_upper:
        mov eax, high_angle_pointer
        fld qword ptr [eax]
        fxch st(1)
        fcomi st(0), st(1)
        fstp st(1)
        jbe angle_store
        mov eax, period
        fsub qword ptr [eax]
    angle_store:
        fstp result
    angle_done:
    }
    return result;
}
} // namespace

GuiSectionRuntimeImplementation::GuiSectionRuntimeImplementation(
    GuiWidgetOwner& owner, GuiSectionRuntimeServices services)
    : owner_(owner), services_(services) {
    require(owner.layout().type == GuiWidgetType::Section &&
        owner.layout().transform.type_id == 17 && &services.buffers.widgets == &owner.runtime(),
        "Section requires the same canonical type17 widget owner");
    const float one = services_.constants.one_00d7a24c;
    const float rim = services_.constants.rim_default_00ce3800;
    fields_.texture_mode_110 = 0;
    fields_.texture_114 = nullptr;
    fields_.value_f4 = fields_.start_angle_f8 = fields_.texture_angle_fc = fields_.u0_100 = 0;
    fields_.u1_104 = one;
    fields_.rim_width_percent_108 = rim;
    fields_.clockwise_10c = 1;
    fields_.atlas_u0_118 = fields_.atlas_v0_11c = 0;
    fields_.atlas_u1_120 = fields_.atlas_v1_124 = one;
}
void GuiSectionRuntimeImplementation::require_owner(GuiWidgetOwner& owner) const {
    require(&owner == &owner_, "Section operation changed canonical owner");
}
NativeModelOwner& GuiSectionRuntimeImplementation::model() const {
    auto* reference = owner_.model_reference();
    require(reference != nullptr, "Section requires its canonical live Model companion");
    auto& result = reference->model_owner();
    require(result.phase == NativeModelOwner::Phase::live &&
        &result.environment.retained_owners == &services_.buffers.geometry.actual_owners() &&
        &services_.buffers.materials.retained_owners == &result.environment.retained_owners &&
        &services_.colors.widgets == &owner_.runtime() &&
        &services_.colors.actual_owners == &result.environment.retained_owners,
        "Section model, material and geometry owner domains must agree");
    return result;
}
GuiSectionRuntimeImplementation::~GuiSectionRuntimeImplementation() noexcept {
    if (has_active_operation()) std::terminate();
}
void GuiSectionRuntimeImplementation::constructed74(GuiWidgetOwner& owner) {
    require_owner(owner);
    require(!has_active_operation(), "Section construction overlaps an unfinished operation");
    SectionOperation active(active_calls_, failed_);
    if (!owner_.layout().transform.bounds_enabled)
        services_.buffers.geometry.construct_and_associate74_fragment(model());
    owner_.set_position_00aa7dc0({0, 0, 0});
}
void GuiSectionRuntimeImplementation::properties_bound(GuiWidgetOwner& owner, const GuiTable&) {
    require_owner(owner);
    throw std::logic_error("Section AC0280 requires native property/texture-loading ownership");
}
void GuiSectionRuntimeImplementation::loaded78(GuiWidgetOwner& owner) {
    require_owner(owner);
    require(!has_active_operation(), "Section load overlaps an unfinished operation");
    SectionOperation active(active_calls_, failed_);
    owner_.base_loaded78_00aa7170();
}
void GuiSectionRuntimeImplementation::set_active60(GuiWidgetOwner& owner, bool value) {
    require_owner(owner); owner_.base_set_active60_00aa6a30(value);
}
bool GuiSectionRuntimeImplementation::is_visible38(GuiWidgetOwner& owner) {
    require_owner(owner); return owner_.base_is_visible38_00a9e0d0();
}
void GuiSectionRuntimeImplementation::visibility_changed3c(GuiWidgetOwner& owner, bool value) {
    require_owner(owner); owner_.base_visibility_changed3c_00a9e100(value);
}
std::int32_t GuiSectionRuntimeImplementation::type5c(GuiWidgetOwner& owner) {
    require_owner(owner); return owner_.layout().transform.type_id;
}
void GuiSectionRuntimeImplementation::before_scene_release(GuiWidgetOwner& owner) {
    require_owner(owner);
    require(!has_active_operation(), "Section resource operations must finish before retirement");
    require(fields_.texture_114 == nullptr && fields_.texture_name_ec.length() == 0,
        "Section authored texture/string destruction requires its native lifetime owner");
}

void GuiSectionRuntimeImplementation::set_values_00abe6e0(
    float value, float start_angle, float u0, float u1) {
    require(!emitting_ && !failed_, "Section setter cannot reenter or replay unfinished emission");
    auto* fields = &fields_;
    std::uint8_t changed;
    __asm {
        mov ecx, fields
        fld dword ptr [ecx + 0xc]
        fld start_angle
        fucomip st(0), st(1)
        fstp st(0)
        lahf
        test ah, 0x44
        jp values_changed
        fld dword ptr [ecx + 8]
        fld value
        fucomip st(0), st(1)
        fstp st(0)
        lahf
        test ah, 0x44
        jp values_changed
        fld u0
        fld dword ptr [ecx + 0x14]
        fucomip st(0), st(1)
        fstp st(0)
        lahf
        test ah, 0x44
        jp values_changed
        fld u1
        fld dword ptr [ecx + 0x18]
        fucomip st(0), st(1)
        fstp st(0)
        lahf
        test ah, 0x44
        jnp values_equal
    values_changed:
        movss xmm0, value
        movss dword ptr [ecx + 8], xmm0
        movss xmm0, start_angle
        movss dword ptr [ecx + 0xc], xmm0
        movss xmm0, u0
        movss dword ptr [ecx + 0x14], xmm0
        movss xmm0, u1
        movss dword ptr [ecx + 0x18], xmm0
        mov changed, 1
        jmp values_done
    values_equal:
        mov changed, 0
    values_done:
    }
    if (changed) emit7c_00abf770();
}

void GuiSectionRuntimeImplementation::emit7c_00abf770() {
    require(!emitting_ && !failed_, "Section emission cannot reenter or replay unfinished work");
    SectionOperation active(active_calls_, failed_);
    struct Emission {
        bool& flag;
        explicit Emission(bool& value) : flag(value) { flag = true; }
        ~Emission() { flag = false; }
    } emitting(emitting_);
    auto* fields = &fields_;
    const auto* one = &services_.constants.one_00d7a24c;
    __asm {
        mov eax, fields
        xorps xmm1, xmm1
        mov edx, one
        movss xmm2, dword ptr [edx]
        movss xmm0, dword ptr [eax + 8]
        comiss xmm1, xmm0
        jbe value_upper
        movaps xmm0, xmm1
        jmp value_store
    value_upper:
        comiss xmm0, xmm2
        jbe value_store
        movaps xmm0, xmm2
    value_store:
        movss dword ptr [eax + 8], xmm0
    }
    fields_.start_angle_f8 = normalize_angle(fields_.start_angle_f8, services_.constants);
    auto* mesh = static_cast<NativeMeshStorage*>(gui_model_geometry_00b74640(model().storage.model, 0));
    require(mesh != nullptr, "Section current74 must associate its actual mesh before emission");
    if (mesh->vertex_stream_count_7c == 0) create_buffers(*mesh, services_.buffers);
    SectionKernel kernel{&fields_, &owner_.layout().transform.size,
        &owner_.layout().transform.bounds_enabled, one, &services_.constants.half_00d7a280,
        &services_.constants.sectors_00cf0058, &services_.constants.remainder_epsilon_00ce3c70,
        &services_.constants.two_pi_00ce3828, &services_.constants.angular_step_00d5c970,
        &services_.constants.sse2_conversion_0109eea4, mesh->vertex_streams_64[0],
        mesh->index_stream_60, nullptr, 0, {}};
    section_numeric(kernel);
    std::uint32_t primitive_count = 0, vertex_count = 0;
    if (!owner_.layout().transform.bounds_enabled) {
        const auto mode = fields_.texture_mode_110;
        if (mode == 0 || mode == 1 || mode == 2) {
            require(kernel.vertex && kernel.index, "Section requires actual logical vertex/index streams");
            const bool rim = mode == 1;
            if (!rim) { kernel.stage = 1; section_numeric(kernel); }
            require_logical_profile(kernel.vertex, 0x00d61d6c);
            lock_native_logical_vertex_stream_00b49980(kernel.vertex, services_.mapping,
                rim ? 0x92u : 0x4au, 0, 0);
            kernel.stage = rim ? 3u : 2u;
            section_numeric(kernel);
            require_logical_profile(kernel.vertex, 0x00d61d6c);
            unlock_native_logical_vertex_stream_00b49a80(kernel.vertex, services_.mapping);
            require_logical_profile(kernel.index, 0x00d61de0);
            kernel.indices = lock_native_logical_index_stream_00b49b60(kernel.index,
                services_.mapping, rim ? 0x1b0u : 0xd8u, 0, 0);
            require(kernel.indices != nullptr, "Section logical index Lock returned null");
            kernel.stage = rim ? 5u : 4u;
            section_numeric(kernel);
            require_logical_profile(kernel.index, 0x00d61de0);
            unlock_native_logical_index_stream_00b49c70(kernel.index, services_.mapping);
            const auto count = static_cast<std::uint32_t>(fields_.segment_count_128);
            primitive_count = rim ? count * 2u : count;
            vertex_count = primitive_count + 2u;
        }
    }
    auto& buffers = services_.buffers;
    auto& owners = buffers.geometry.actual_owners();
    auto selection = acquire_gui_native_section_fragment(*mesh, buffers.geometry);
    auto* section = selection.section;
    NativeMaterialStorage* material;
    if (selection.append_on_publish) {
        NativeString name;
        name.assign_0041e870(buffers.strings, "GuiDefault.mshd");
        try {
            material = buffers.geometry.create_material_for_effect_00535320(name,
                buffers.current_renderer_00f8d394, buffers.materials, buffers.material_vtable_00d5e520);
        } catch (...) {
            destroy_native_string_header_0041dd20(&name, buffers.strings); throw;
        }
        destroy_native_string_header_0041dd20(&name, buffers.strings);
    } else {
        material = static_cast<NativeMaterialStorage*>(section->material_20);
        require(material != nullptr, "Existing Section draw section requires its actual material");
        material->references_04.fetch_add(1);
    }
    NativeString parameter;
    parameter.assign_0041e870(buffers.strings, "cOverbrightFactor");
    try {
        register_native_material_float_00b18b20(*material, &parameter,
            &owner_.extra_fields().overbright_94, services_.parameters);
    } catch (...) {
        destroy_native_string_header_0041dd20(&parameter, buffers.strings); throw;
    }
    destroy_native_string_header_0041dd20(&parameter, buffers.strings);
    set_native_material_texture_00b189f0(*material, 0, fields_.texture_114, owners);
    set_native_mesh_section_material_00b864c0(*section, owners, material);
    release_native_render_actual_owner(owners, material);
    section->range_words_0c[0] = 0;
    section->range_words_0c[2] = 0;
    section->range_words_0c[3] = primitive_count;
    section->primitive_08 = 4;
    section->range_words_0c[1] = vertex_count;
    rebuild_native_mesh_section_vertex_layout_00b865a0(*section, owners, mesh, services_.layouts);
    if (selection.append_on_publish) append_native_mesh_draw_section_00b73c60(*mesh, section);
    release_native_render_actual_owner(owners, section);
    owner_.recompose_00aa7220();
    set_gui_color_00aa6870(owner_, owner_.layout().color, services_.colors);
}

bool update_gui_section_timed_owner_00abe7b0(GuiSectionRuntimeImplementation& section,
    GuiTimedEntryStorage& entry, float delta) {
    require(entry.widget_08 == &section.owner().layout(), "Section timed entry changed canonical widget identity");
    auto& fields = section.fields();
    float start, u0, candidate;
    auto* start_pointer = &fields.start_angle_f8;
    auto* u0_pointer = &fields.u0_100;
    __asm {
        mov eax, start_pointer
        movss xmm0, dword ptr [eax]
        movss start, xmm0
        mov eax, u0_pointer
        movss xmm0, dword ptr [eax]
        movss u0, xmm0
    }
    if (!approach_gui_timed_entry_section_00abe7b0(fields.value_f4,
        entry.target_0c, entry.rate_10, delta, candidate)) return false;
    float one;
    __asm {
        fld1
        fstp one
        fld u0
        fstp u0
        fld start
        fstp start
        fld candidate
        fstp candidate
    }
    section.set_values_00abe6e0(candidate, start, u0, one);
    return true;
}
} // namespace bsp
