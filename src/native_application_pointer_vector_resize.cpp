#include "bsp/native_application_pointer_vector_resize.hpp"

#include <cstddef>

namespace bsp {
namespace {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::int32_t) == 4);
static_assert(offsetof(NativeApplicationPointerVectorStorage, data_00) == 0);
static_assert(offsetof(NativeApplicationPointerVectorStorage, count_04) == 4);
static_assert(offsetof(NativeApplicationPointerVectorStorage, capacity_08) == 8);

// Private Source ABI: ECX actual header, EDX borrowed allocation binding,
// stack signed request, RET4. Forward the same objects to the real reserve
// service; no copied binding, default callback or alternate allocation domain.
__declspec(noinline) void __fastcall reserve_for_resize(
    NativeApplicationPointerVectorStorage* actual_rows,
    const NativeApplicationPointerVectorAllocation* allocation,
    std::int32_t requested) {
    reserve_native_application_pointer_vector_00735ec0(
        *actual_rows, requested, *allocation);
}

// EDX carries the extra Source binding only as far as the sole reserve call.
// No preceding instruction changes EDX, and no later instruction needs it.
// The native stack layout and instruction schedule require no added spill.
__declspec(naked) void __fastcall resize_body(
    NativeApplicationPointerVectorStorage*,
    const NativeApplicationPointerVectorAllocation*, std::int32_t) {
    __asm {
        push esi                              // 00735F30
        push edi                              // 00735F31
        mov edi, dword ptr [esp + 0ch]         // 00735F32
        mov esi, ecx                          // 00735F36
        cmp edi, dword ptr [esi + 8]           // 00735F38
        jle short resize_current_count        // 00735F3B
        push edi                              // 00735F3D
        call reserve_for_resize               // 00735F3E
    resize_current_count:
        mov eax, dword ptr [esi + 4]           // 00735F43
        cmp eax, edi                          // 00735F46
        jge short resize_shrink_check         // 00735F48
        _emit 08dh                            // 00735F4A: LEA EBX,[EBX+disp32(0)]
        _emit 09bh
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
    resize_growth:
        mov ecx, dword ptr [esi]               // 00735F50
        lea ecx, [ecx + eax*4]                 // 00735F52
        test ecx, ecx                         // 00735F55
        jz short resize_growth_next           // 00735F57
        mov dword ptr [ecx], 0                 // 00735F59
    resize_growth_next:
        add eax, 1                            // 00735F5F
        cmp eax, edi                          // 00735F62
        jl short resize_growth                // 00735F64
    resize_shrink_check:
        cmp edi, dword ptr [esi + 4]           // 00735F66
        jge short resize_publish              // 00735F69
        or eax, -1                            // 00735F6B
        _emit 08bh                            // 00735F6E: MOV EDI,EDI, exact 8B FF
        _emit 0ffh
    resize_shrink:
        add dword ptr [esi + 4], eax           // 00735F70
        cmp edi, dword ptr [esi + 4]           // 00735F73
        jl short resize_shrink                // 00735F76
    resize_publish:
        mov dword ptr [esi + 4], edi           // 00735F78
        pop edi                               // 00735F7B
        pop esi                               // 00735F7C
        ret 4                                 // 00735F7D
    }
}

} // namespace

__declspec(noinline) void resize_native_application_pointer_vector_00735f30(
    NativeApplicationPointerVectorStorage& actual_rows,
    std::int32_t requested,
    const NativeApplicationPointerVectorAllocation& allocation) {
    resize_body(&actual_rows, &allocation, requested);
}

} // namespace bsp
