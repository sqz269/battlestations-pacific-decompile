#include "bsp/native_renderer_capability_array_reserves.hpp"

#include "bsp/singleton_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Capability array reserves require MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(std::int32_t) == 4);

__declspec(noinline) void* __cdecl allocate_capability_array(std::uint32_t bytes) {
    return singleton_lifetime_allocate({
        SingletonAllocationKind::object, bytes, bytes});
}

__declspec(noinline) void __cdecl free_capability_array(void* allocation) noexcept {
    singleton_lifetime_free(allocation);
}
} // namespace

__declspec(naked) void __fastcall reserve_native_capability_records_00b22b30(
    void*, std::uint32_t, std::int32_t) {
    __asm {
        push ebx
        mov ebx, dword ptr [esp + 8]
        cmp ebx, 1
        push esi
        mov esi, ecx
        jge records_clamped
        mov ebx, 1
    records_clamped:
        cmp dword ptr [esi + 8], ebx
        jge records_return
        lea eax, [ebx + ebx * 2]
        push ebp
        add eax, eax
        add eax, eax
        push edi
        push eax
        call allocate_capability_array
        xor edi, edi
        add esp, 4
        cmp dword ptr [esi + 4], edi
        mov ebp, eax
        mov dword ptr [esp + 14h], ebp
        jle records_free
        xor edx, edx
        mov ecx, ebp
        // Original six-byte LEA EBX,[EBX+0] loop alignment instruction.
        _emit 08Dh
        _emit 09Bh
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
    records_copy:
        test ecx, ecx
        jz records_next
        mov eax, dword ptr [esi]
        mov ebp, dword ptr [eax + edx]
        add eax, edx
        mov dword ptr [ecx], ebp
        mov ebp, dword ptr [eax + 4]
        mov dword ptr [ecx + 4], ebp
        mov eax, dword ptr [eax + 8]
        mov dword ptr [ecx + 8], eax
    records_next:
        add edi, 1
        add edx, 0Ch
        add ecx, 0Ch
        cmp edi, dword ptr [esi + 4]
        jl records_copy
        mov ebp, dword ptr [esp + 14h]
    records_free:
        mov ecx, dword ptr [esi]
        push ecx
        call free_capability_array
        // Full returning-free publication tail omitted by saved analysis.
        add esp, 4
        pop edi
        mov dword ptr [esi], ebp
        mov dword ptr [esi + 8], ebx
        pop ebp
    records_return:
        pop esi
        pop ebx
        ret 4
    }
}

__declspec(naked) void __fastcall reserve_native_capability_dwords_00b236b0(
    void*, std::uint32_t, std::int32_t) {
    __asm {
        push esi
        push edi
        mov edi, dword ptr [esp + 0Ch]
        cmp edi, 1
        mov esi, ecx
        jge dwords_clamped
        mov edi, 1
    dwords_clamped:
        cmp dword ptr [esi + 8], edi
        jge dwords_return
        lea eax, [edi * 4]
        push ebx
        push eax
        call allocate_capability_array
        mov ebx, eax
        xor eax, eax
        add esp, 4
        cmp dword ptr [esi + 4], eax
        jle dwords_free
        mov ecx, ebx
    dwords_copy:
        test ecx, ecx
        jz dwords_next
        mov edx, dword ptr [esi]
        mov edx, dword ptr [edx + eax * 4]
        mov dword ptr [ecx], edx
    dwords_next:
        add eax, 1
        add ecx, 4
        cmp eax, dword ptr [esi + 4]
        jl dwords_copy
    dwords_free:
        mov eax, dword ptr [esi]
        push eax
        call free_capability_array
        // Full returning-free publication tail omitted by saved analysis.
        add esp, 4
        mov dword ptr [esi], ebx
        mov dword ptr [esi + 8], edi
        pop ebx
    dwords_return:
        pop edi
        pop esi
        ret 4
    }
}

} // namespace bsp
