#include "bsp/native_shadow_pointer_vector.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {

static_assert(sizeof(void*) == 4 && sizeof(std::size_t) == 4,
              "Native pointer vector arithmetic requires Win32 DWORDs");

// BF55BE tails to BF681B. The caller already computed the wrapped DWORD size;
// both request sizes use those exact bits with the existing malloc/new-handler
// provider. This source adapter receives no additional native body credit.
__declspec(noinline) void* __cdecl allocate_shadow_vector_bytes(
    std::uint32_t wrapped_bytes) {
    return singleton_lifetime_allocate({
        SingletonAllocationKind::object, wrapped_bytes, wrapped_bytes});
}

} // namespace

__declspec(naked) void __fastcall reserve_native_shadow_pointer_vector_00ae1440(
    void*, void*, std::int32_t) {
    __asm {
        push esi
        push edi
        mov edi, dword ptr [esp + 0ch]
        cmp edi, 4
        mov esi, ecx
        jge reserve_capacity_check
        mov edi, 4
    reserve_capacity_check:
        cmp dword ptr [esi + 8], edi
        jge reserve_done
        lea eax, [edi*4 + 0]
        push ebx
        push eax
        call allocate_shadow_vector_bytes // AE1460 -> original BF55BE
        mov ebx, eax
        xor eax, eax
        add esp, 4
        cmp dword ptr [esi + 4], eax
        jle reserve_release_old
        mov ecx, ebx
    reserve_copy_word:
        test ecx, ecx
        jz reserve_advance
        mov edx, dword ptr [esi]
        mov edx, dword ptr [edx + eax*4]
        mov dword ptr [ecx], edx
    reserve_advance:
        add eax, 1
        add ecx, 4
        cmp eax, dword ptr [esi + 4]
        jl reserve_copy_word
    reserve_release_old:
        mov eax, dword ptr [esi]
        push eax
        call singleton_lifetime_free // AE148C -> original BF6989
        add esp, 4
        mov dword ptr [esi], ebx
        mov dword ptr [esi + 8], edi
        pop ebx
    reserve_done:
        pop edi
        pop esi
        ret 4
    }
}

__declspec(naked) void __fastcall resize_native_shadow_pointer_vector_00ae19b0(
    void*, void*, std::int32_t) {
    __asm {
        push esi
        push edi
        mov edi, dword ptr [esp + 0ch]
        mov esi, ecx
        cmp edi, dword ptr [esi + 8]
        jle resize_count_check
        push edi
        call reserve_native_shadow_pointer_vector_00ae1440 // AE19BE
    resize_count_check:
        mov eax, dword ptr [esi + 4]
        cmp eax, edi
        jge resize_shrink_check
        // AE19CA: exact six-byte LEA EBX,[EBX+disp32 zero].
        _emit 08dh
        _emit 09bh
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
    resize_zero_word:
        mov ecx, dword ptr [esi]
        lea ecx, [ecx + eax*4]
        test ecx, ecx
        jz resize_advance
        mov dword ptr [ecx], 0
    resize_advance:
        add eax, 1
        cmp eax, edi
        jl resize_zero_word
    resize_shrink_check:
        cmp edi, dword ptr [esi + 4]
        jge resize_store_count
        or eax, 0ffffffffh
        // AE19EE: exact MOV EDI,EDI alignment bytes.
        _emit 08bh
        _emit 0ffh
    resize_decrement_count:
        add dword ptr [esi + 4], eax
        cmp edi, dword ptr [esi + 4]
        jl resize_decrement_count
    resize_store_count:
        mov dword ptr [esi + 4], edi
        pop edi
        pop esi
        ret 4
    }
}

} // namespace bsp
