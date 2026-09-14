#include "bsp/native_material_record_assignment.hpp"

#include "bsp/native_material_record_ranges.hpp"
#include "bsp/native_renderer_begin_frame.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native material record assignment requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);

__declspec(naked) void* __cdecl copy_assign_native_material_records_backward_00b13310(
    const void*, const void*, void*, NativeStringRawPoolContext&) {
    __asm {
        push ebx
        mov ebx, dword ptr [esp + 8]
        push esi
        mov esi, dword ptr [esp + 10h]
        cmp ebx, esi
        je empty_range
        push edi
        mov edi, dword ptr [esp + 18h]
        push ebp
        mov ebp, dword ptr [esp + 20h]
    next_record:
        sub esi, 12Ch
        sub edi, 12Ch
        push ebp
        push esi
        push edi
        call assign_native_renderer_record_00b13180
        add esp, 0Ch
        cmp esi, ebx
        jne next_record
        mov eax, edi
        pop ebp
        pop edi
        pop esi
        pop ebx
        ret
    empty_range:
        mov eax, dword ptr [esp + 14h]
        pop esi
        pop ebx
        ret
    }
}

__declspec(naked) void* __cdecl copy_assign_native_material_record_head_00b13720(
    const void*, const void*, void*, NativeStringRawPoolContext&) {
    __asm {
        push ebx
        mov ebx, dword ptr [esp + 8]
        push esi
        mov esi, dword ptr [esp + 10h]
        push edi
        mov edi, dword ptr [esp + 18h]
        push dword ptr [esp + 1Ch]
        push edi
        push esi
        push ebx
        call copy_assign_native_material_records_backward_00b13310
        add esp, 10h
        sub esi, ebx
        mov eax, 1B4E81B5h
        imul esi
        sar edx, 5
        mov eax, edx
        shr eax, 1Fh
        add eax, edx
        mov ecx, eax
        imul ecx, ecx, 12Ch
        mov eax, edi
        pop edi
        pop esi
        sub eax, ecx
        pop ebx
        ret
    }
}

void* __cdecl copy_assign_native_material_record_head_wrapper_00b13f50(
    const void* first, const void* last, void* output_end,
    NativeStringRawPoolContext& strings) {
    return copy_assign_native_material_record_head_00b13720(
        first, last, output_end, strings);
}

__declspec(naked) void __cdecl fill_assign_native_material_records_00b13ad0(
    void*, void*, const void*, NativeStringRawPoolContext&) {
    __asm {
        push esi
        mov esi, dword ptr [esp + 8]
        push edi
        mov edi, dword ptr [esp + 10h]
        cmp esi, edi
        je done
        push ebx
        mov ebx, dword ptr [esp + 18h]
        push ebp
        mov ebp, dword ptr [esp + 20h]
    next_record:
        push ebp
        push ebx
        push esi
        call assign_native_renderer_record_00b13180
        add esp, 0Ch
        add esi, 12Ch
        cmp esi, edi
        jne next_record
        pop ebp
        pop ebx
    done:
        pop edi
        pop esi
        ret
    }
}

void* __stdcall copy_construct_native_material_record_tail_00b14550(
    const void* first, const void* last, void* output,
    NativeStringRawPoolContext& strings) {
    return copy_construct_native_material_records_00b13920(
        first, last, output, strings);
}
} // namespace bsp
