#include "bsp/native_pod8_raw_array.hpp"

#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
// Native BF55BE is a complete JMP to BF681B. Preserve its actual byte request,
// including zero and multiplication wrap, through the genuine current provider.
__declspec(noinline) void* __cdecl allocate_pod8_raw_bytes(std::uint32_t bytes) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}

// Native BF6989 -> BF65AC -> BF9DC8. This is the matching actual Source domain.
__declspec(noinline) void __cdecl free_pod8_raw_bytes(void* pointer) noexcept {
    singleton_lifetime_free(pointer);
}
} // namespace

__declspec(naked) void __fastcall set_capacity_native_pod8_raw_array_00414010(
    void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]
        push esi
        mov esi, ecx
        mov dword ptr [esi + 8], eax
        add eax, eax
        add eax, eax
        add eax, eax
        push edi
        push eax
        call allocate_pod8_raw_bytes // 00414022: actual bytes, capacity published.
        xor ecx, ecx
        add esp, 4
        cmp dword ptr [esi + 4], ecx
        mov edi, eax
        jbe copied
    next_element:
        test eax, eax
        je skip_copy
        mov edx, dword ptr [esi]
        fld dword ptr [edx + ecx * 8]
        lea edx, [edx + ecx * 8]
        fstp dword ptr [eax]
        fld dword ptr [edx + 4]
        fstp dword ptr [eax + 4]
    skip_copy:
        add ecx, 1
        add eax, 8
        cmp ecx, dword ptr [esi + 4]
        jb next_element
    copied:
        mov eax, dword ptr [esi]
        test eax, eax
        je publish
        push eax
        call free_pod8_raw_bytes // 00414059: current owner pointer, late reload.
        add esp, 4
    publish:
        mov dword ptr [esi], edi
        pop edi
        pop esi
        ret 4
    }
}

__declspec(naked) void __fastcall grow_native_pod8_raw_array_00414070(void*, void*) {
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 8]
        lea eax, [eax + eax + 2]
        mov dword ptr [esi + 8], eax
        add eax, eax
        add eax, eax
        add eax, eax
        push edi
        push eax
        call allocate_pod8_raw_bytes // 00414085.
        xor ecx, ecx
        add esp, 4
        cmp dword ptr [esi + 4], ecx
        mov edi, eax
        jbe copied
    next_element:
        test eax, eax
        je skip_copy
        mov edx, dword ptr [esi]
        fld dword ptr [edx + ecx * 8]
        lea edx, [edx + ecx * 8]
        fstp dword ptr [eax]
        fld dword ptr [edx + 4]
        fstp dword ptr [eax + 4]
    skip_copy:
        add ecx, 1
        add eax, 8
        cmp ecx, dword ptr [esi + 4]
        jb next_element
    copied:
        mov eax, dword ptr [esi]
        test eax, eax
        je publish
        push eax
        call free_pod8_raw_bytes // 004140BC.
        add esp, 4
    publish:
        mov dword ptr [esi], edi
        pop edi
        pop esi
        ret
    }
}

__declspec(naked) void __fastcall append_native_pod8_raw_array_00418ea0(
    void*, void*, const void*) {
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 8]
        cmp dword ptr [esi + 4], eax
        jne append_value
        lea eax, [eax + eax + 2]
        mov dword ptr [esi + 8], eax
        add eax, eax
        add eax, eax
        add eax, eax
        push edi
        push eax
        call allocate_pod8_raw_bytes // 00418EBA.
        xor ecx, ecx
        add esp, 4
        cmp dword ptr [esi + 4], ecx
        mov edi, eax
        jbe copied
        jmp next_element
        // Exact unexecuted 00418ECD..00418ECF LEA ECX,[ECX+00] padding.
        _emit 08dh
        _emit 049h
        _emit 000h
    next_element:
        test eax, eax
        je skip_copy
        mov edx, dword ptr [esi]
        fld dword ptr [edx + ecx * 8]
        lea edx, [edx + ecx * 8]
        fstp dword ptr [eax]
        fld dword ptr [edx + 4]
        fstp dword ptr [eax + 4]
    skip_copy:
        add ecx, 1
        add eax, 8
        cmp ecx, dword ptr [esi + 4]
        jb next_element
    copied:
        mov eax, dword ptr [esi]
        test eax, eax
        je publish
        push eax
        call free_pod8_raw_bytes // 00418EF6.
        add esp, 4
    publish:
        mov dword ptr [esi], edi
        pop edi
    append_value:
        mov eax, dword ptr [esi + 4]
        mov ecx, dword ptr [esi]
        lea eax, [ecx + eax * 8]
        test eax, eax
        je increment_size
        mov ecx, dword ptr [esp + 8] // Physical incoming value, after any free.
        fld dword ptr [ecx]
        fstp dword ptr [eax]
        fld dword ptr [ecx + 4]
        fstp dword ptr [eax + 4]
    increment_size:
        add dword ptr [esi + 4], 1
        pop esi
        ret 4
    }
}

__declspec(naked) void __fastcall resize_native_pod8_raw_array_00418f30(
    void*, void*, std::uint32_t) {
    __asm {
        push esi
        push edi
        mov edi, dword ptr [esp + 0ch]
        mov esi, ecx
        cmp edi, dword ptr [esi + 4]
        jbe set_size
        cmp edi, dword ptr [esi + 8]
        jbe set_size
        lea eax, [edi * 8]
        push ebx
        push eax
        mov dword ptr [esi + 8], edi
        call allocate_pod8_raw_bytes // 00418F4E.
        xor ecx, ecx
        add esp, 4
        cmp dword ptr [esi + 4], ecx
        mov ebx, eax
        jbe copied
        nop // Exact00418F5F padding, reached before the loop.
    next_element:
        test eax, eax
        je skip_copy
        mov edx, dword ptr [esi]
        fld dword ptr [edx + ecx * 8]
        lea edx, [edx + ecx * 8]
        fstp dword ptr [eax]
        fld dword ptr [edx + 4]
        fstp dword ptr [eax + 4]
    skip_copy:
        add ecx, 1
        add eax, 8
        cmp ecx, dword ptr [esi + 4]
        jb next_element
    copied:
        mov eax, dword ptr [esi]
        test eax, eax
        je publish
        push eax
        call free_pod8_raw_bytes // 00418F86.
        add esp, 4
    publish:
        mov dword ptr [esi], ebx
        pop ebx
    set_size:
        mov dword ptr [esi + 4], edi
        pop edi
        pop esi
        ret 4
    }
}

__declspec(naked) void __fastcall reserve_native_pod8_raw_array_00418fb0(
    void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]
        push esi
        mov esi, ecx
        cmp dword ptr [esi + 8], eax
        jae finished
        mov dword ptr [esi + 8], eax
        add eax, eax
        add eax, eax
        add eax, eax
        push edi
        push eax
        call allocate_pod8_raw_bytes // 00418FC7.
        xor ecx, ecx
        add esp, 4
        cmp dword ptr [esi + 4], ecx
        mov edi, eax
        jbe copied
    next_element:
        test eax, eax
        je skip_copy
        mov edx, dword ptr [esi]
        fld dword ptr [edx + ecx * 8]
        lea edx, [edx + ecx * 8]
        fstp dword ptr [eax]
        fld dword ptr [edx + 4]
        fstp dword ptr [eax + 4]
    skip_copy:
        add ecx, 1
        add eax, 8
        cmp ecx, dword ptr [esi + 4]
        jb next_element
    copied:
        mov eax, dword ptr [esi]
        test eax, eax
        je publish
        push eax
        call free_pod8_raw_bytes // 00418FFE.
        add esp, 4
    publish:
        mov dword ptr [esi], edi
        pop edi
    finished:
        pop esi
        ret 4
    }
}
} // namespace bsp
