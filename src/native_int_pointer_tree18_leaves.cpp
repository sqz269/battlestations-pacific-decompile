#include "bsp/native_int_pointer_tree18_leaves.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdint>
#include <cstdlib>

namespace bsp {
namespace {
// Actual source services, with native and host allocation sizes identical.
__declspec(noinline) void* __cdecl allocate_tree18_storage(std::uint32_t bytes) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}
__declspec(naked) void __cdecl tree18_invalid_parameter() {
    __asm { jmp _invalid_parameter_noinfo }
}
} // namespace

// Complete 008697b0[28]; native memory/register schedule.
__declspec(naked) void* __fastcall maximum_native_int_pointer_tree18_008697b0(void*) {
    __asm {
        mov eax, ecx
        mov edx, dword ptr [eax + 8]
        cmp byte ptr [edx + 0x15], 0
        jne L_008697cb
        jmp L_008697c0
        // Preserve native alignment LEA encoding.
        _emit 0x8d
        _emit 0x49
        _emit 0x00
    L_008697c0:
        mov eax, edx
        mov edx, dword ptr [eax + 8]
        cmp byte ptr [edx + 0x15], 0
        je L_008697c0
    L_008697cb:
        ret
    }
}

// Complete 008697d0[27]; native memory/register schedule.
__declspec(naked) void* __fastcall minimum_native_int_pointer_tree18_008697d0(void*) {
    __asm {
        mov eax, ecx
        mov edx, dword ptr [eax]
        cmp byte ptr [edx + 0x15], 0
        jne L_008697ea
        // Preserve native alignment LEA encoding.
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
    L_008697e0:
        mov eax, edx
        mov edx, dword ptr [eax]
        cmp byte ptr [edx + 0x15], 0
        je L_008697e0
    L_008697ea:
        ret
    }
}

// Complete 00869810[82]; native memory/register schedule.
__declspec(naked) void __fastcall rotate_right_native_int_pointer_tree18_00869810(void*, void*, void*) {
    __asm {
        mov edx, dword ptr [esp + 4]
        mov eax, dword ptr [edx]
        push esi
        mov esi, dword ptr [eax + 8]
        mov dword ptr [edx], esi
        mov esi, dword ptr [eax + 8]
        cmp byte ptr [esi + 0x15], 0
        jne L_00869828
        mov dword ptr [esi + 4], edx
    L_00869828:
        mov esi, dword ptr [edx + 4]
        mov dword ptr [eax + 4], esi
        mov ecx, dword ptr [ecx + 4]
        cmp edx, dword ptr [ecx + 4]
        pop esi
        jne L_00869843
        mov dword ptr [ecx + 4], eax
        mov dword ptr [eax + 8], edx
        mov dword ptr [edx + 4], eax
        ret 4
    L_00869843:
        mov ecx, dword ptr [edx + 4]
        cmp edx, dword ptr [ecx + 8]
        jne L_00869857
        mov dword ptr [ecx + 8], eax
        mov dword ptr [eax + 8], edx
        mov dword ptr [edx + 4], eax
        ret 4
    L_00869857:
        mov dword ptr [ecx], eax
        mov dword ptr [eax + 8], edx
        mov dword ptr [edx + 4], eax
        ret 4
    }
}

// Complete 0086a2f0[78]; native memory/register schedule.
__declspec(naked) void __fastcall rotate_left_native_int_pointer_tree18_0086a2f0(void*, void*, void*) {
    __asm {
        mov edx, dword ptr [esp + 4]
        mov eax, dword ptr [edx + 8]
        push esi
        mov esi, dword ptr [eax]
        mov dword ptr [edx + 8], esi
        mov esi, dword ptr [eax]
        cmp byte ptr [esi + 0x15], 0
        jne L_0086a308
        mov dword ptr [esi + 4], edx
    L_0086a308:
        mov esi, dword ptr [edx + 4]
        mov dword ptr [eax + 4], esi
        mov ecx, dword ptr [ecx + 4]
        cmp edx, dword ptr [ecx + 4]
        pop esi
        jne L_0086a322
        mov dword ptr [ecx + 4], eax
        mov dword ptr [eax], edx
        mov dword ptr [edx + 4], eax
        ret 4
    L_0086a322:
        mov ecx, dword ptr [edx + 4]
        cmp edx, dword ptr [ecx]
        jne L_0086a333
        mov dword ptr [ecx], eax
        mov dword ptr [eax], edx
        mov dword ptr [edx + 4], eax
        ret 4
    L_0086a333:
        mov dword ptr [ecx + 8], eax
        mov dword ptr [eax], edx
        mov dword ptr [edx + 4], eax
        ret 4
    }
}

// Complete 00869a20[99]; native memory/register schedule.
__declspec(naked) void __fastcall increment_native_int_pointer_tree18_00869a20(void*) {
    __asm {
        push esi
        mov esi, ecx
        cmp dword ptr [esi], 0
        jne L_00869a2d
        call tree18_invalid_parameter
    L_00869a2d:
        mov eax, dword ptr [esi + 4]
        cmp byte ptr [eax + 0x15], 0
        je L_00869a3c
        pop esi
        jmp tree18_invalid_parameter
    L_00869a3c:
        mov ecx, dword ptr [eax + 8]
        cmp byte ptr [ecx + 0x15], 0
        jne L_00869a5f
        mov eax, dword ptr [ecx]
        cmp byte ptr [eax + 0x15], 0
        jne L_00869a5a
        // Preserve native alignment LEA encoding.
        _emit 0x8d
        _emit 0x49
        _emit 0x00
    L_00869a50:
        mov ecx, eax
        mov eax, dword ptr [ecx]
        cmp byte ptr [eax + 0x15], 0
        je L_00869a50
    L_00869a5a:
        mov dword ptr [esi + 4], ecx
        pop esi
        ret
    L_00869a5f:
        mov eax, dword ptr [eax + 4]
        cmp byte ptr [eax + 0x15], 0
        jne L_00869a7e
    L_00869a68:
        mov ecx, dword ptr [esi + 4]
        cmp ecx, dword ptr [eax + 8]
        jne L_00869a7e
        mov dword ptr [esi + 4], eax
        mov edx, eax
        mov eax, dword ptr [edx + 4]
        cmp byte ptr [eax + 0x15], 0
        je L_00869a68
    L_00869a7e:
        mov dword ptr [esi + 4], eax
        pop esi
        ret
    }
}

// Complete 0086aa60[53]; native memory/register schedule.
__declspec(naked) void __fastcall erase_subtree_native_int_pointer_tree18_0086aa60(void*, void*, void*) {
    __asm {
        push ebx
        push esi
        push edi
        mov edi, dword ptr [esp + 0x10]
        cmp byte ptr [edi + 0x15], 0
        mov ebx, ecx
        mov esi, edi
        jne L_0086aa8f
    L_0086aa71:
        mov eax, dword ptr [esi + 8]
        push eax
        mov ecx, ebx
        call erase_subtree_native_int_pointer_tree18_0086aa60
        mov esi, dword ptr [esi]
        push edi
        call singleton_lifetime_free
        add esp, 4
        cmp byte ptr [esi + 0x15], 0
        mov edi, esi
        je L_0086aa71
    L_0086aa8f:
        pop edi
        pop esi
        pop ebx
        ret 4
    }
}

// Complete 0086ac00[55]; native memory/register schedule.
__declspec(naked) void* __cdecl allocate_native_int_pointer_tree18_node_0086ac00() {
    __asm {
        push 0x18
        call allocate_tree18_storage
        add esp, 4
        test eax, eax
        je L_0086ac14
        mov dword ptr [eax], 0
    L_0086ac14:
        lea ecx, [eax + 4]
        test ecx, ecx
        je L_0086ac21
        mov dword ptr [ecx], 0
    L_0086ac21:
        lea ecx, [eax + 8]
        test ecx, ecx
        je L_0086ac2e
        mov dword ptr [ecx], 0
    L_0086ac2e:
        mov byte ptr [eax + 0x14], 1
        mov byte ptr [eax + 0x15], 0
        ret
    }
}

} // namespace bsp
