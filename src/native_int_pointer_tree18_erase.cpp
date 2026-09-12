#include "bsp/native_int_pointer_tree18_erase.hpp"
#include "bsp/native_int_pointer_tree18_leaves.hpp"
#include "bsp/native_hardware_layout_tree.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdlib>

namespace bsp {
namespace {
__declspec(naked) void __cdecl erase_tree18_invalid_parameter() {
    __asm { jmp _invalid_parameter_noinfo }
}
struct CompletedTemporary {
    NativeLegacySboStringStorage& storage;
    ~CompletedTemporary() noexcept { native_legacy_sbo_string_destroy_004072d0(storage); }
};
[[noreturn]] __declspec(noinline) void __cdecl throw_tree18_invalid_iterator() {
    NativeLegacySboStringStorage temporary;
    temporary.capacity_18 = 15;
    temporary.length_14 = 0;
    temporary.buffer_04.inline_bytes[0] = 0;
    native_legacy_sbo_string_assign_counted_00408720(
        temporary, "invalid map/set<T> iterator", 27);
    // Native state0 starts after assignment. C95BB0 destroys this string.
    const CompletedTemporary completed{temporary};
    throw NativeHardwareLayoutInvalidIterator{temporary};
}
} // namespace

// Complete 0086e8a0; full raw normal paths, actual source providers.
__declspec(naked) void* __fastcall erase_native_int_pointer_tree18_iterator_0086e8a0(void*, void*, void*, void*, void*) {
    __asm {
        // Preserve native normal-frame offsets without installing its FH3 frame.
        mov eax, dword ptr [esp + 0x0c]
        sub esp, 0x54
        cmp byte ptr [eax + 0x15], 0
        push ebp
        push esi
        mov ebp, ecx
        jz L_0086e912
        // Source throw owns its cleanup; restore entry EBP/ESI/ESP first.
        pop esi
        pop ebp
        add esp, 0x54
        jmp throw_tree18_invalid_iterator
    L_0086e912:
        push ebx
        mov ebx, eax
        push edi
        lea ecx, [esp + 0x6c]
        mov dword ptr [esp + 0x10], ebx
        call increment_native_int_pointer_tree18_00869a20
        mov eax, dword ptr [ebx]
        cmp byte ptr [eax + 0x15], 0
        je L_0086e930
        mov edi, dword ptr [ebx + 8]
        jmp L_0086e948
    L_0086e930:
        mov edx, dword ptr [ebx + 8]
        cmp byte ptr [edx + 0x15], 0
        je L_0086e93d
        mov edi, eax
        jmp L_0086e948
    L_0086e93d:
        mov ecx, dword ptr [esp + 0x70]
        cmp ecx, ebx
        mov edi, dword ptr [ecx + 8]
        jne L_0086e9af
    L_0086e948:
        cmp byte ptr [edi + 0x15], 0
        mov esi, dword ptr [ebx + 4]
        jne L_0086e954
        mov dword ptr [edi + 4], esi
    L_0086e954:
        mov eax, dword ptr [ebp + 4]
        cmp dword ptr [eax + 4], ebx
        jne L_0086e961
        mov dword ptr [eax + 4], edi
        jmp L_0086e96c
    L_0086e961:
        cmp dword ptr [esi], ebx
        jne L_0086e969
        mov dword ptr [esi], edi
        jmp L_0086e96c
    L_0086e969:
        mov dword ptr [esi + 8], edi
    L_0086e96c:
        mov ebx, dword ptr [ebp + 4]
        mov eax, dword ptr [ebx]
        cmp eax, dword ptr [esp + 0x10]
        jne L_0086e98a
        cmp byte ptr [edi + 0x15], 0
        je L_0086e981
        mov eax, esi
        jmp L_0086e988
    L_0086e981:
        mov ecx, edi
        call minimum_native_int_pointer_tree18_008697d0
    L_0086e988:
        mov dword ptr [ebx], eax
    L_0086e98a:
        mov ebx, dword ptr [ebp + 4]
        mov ecx, dword ptr [esp + 0x10]
        cmp dword ptr [ebx + 8], ecx
        jne L_0086ea06
        cmp byte ptr [edi + 0x15], 0
        je L_0086e9a3
        mov eax, esi
        mov dword ptr [ebx + 8], eax
        jmp L_0086ea06
    L_0086e9a3:
        mov ecx, edi
        call maximum_native_int_pointer_tree18_008697b0
        mov dword ptr [ebx + 8], eax
        jmp L_0086ea06
    L_0086e9af:
        mov dword ptr [eax + 4], ecx
        mov edx, dword ptr [ebx]
        mov dword ptr [ecx], edx
        cmp ecx, dword ptr [ebx + 8]
        jne L_0086e9bf
        mov esi, ecx
        jmp L_0086e9d9
    L_0086e9bf:
        cmp byte ptr [edi + 0x15], 0
        mov esi, dword ptr [ecx + 4]
        jne L_0086e9cb
        mov dword ptr [edi + 4], esi
    L_0086e9cb:
        mov dword ptr [esi], edi
        mov eax, dword ptr [ebx + 8]
        mov dword ptr [ecx + 8], eax
        mov edx, dword ptr [ebx + 8]
        mov dword ptr [edx + 4], ecx
    L_0086e9d9:
        mov eax, dword ptr [ebp + 4]
        cmp dword ptr [eax + 4], ebx
        jne L_0086e9e6
        mov dword ptr [eax + 4], ecx
        jmp L_0086e9f4
    L_0086e9e6:
        mov eax, dword ptr [ebx + 4]
        cmp dword ptr [eax], ebx
        jne L_0086e9f1
        mov dword ptr [eax], ecx
        jmp L_0086e9f4
    L_0086e9f1:
        mov dword ptr [eax + 8], ecx
    L_0086e9f4:
        mov eax, dword ptr [ebx + 4]
        mov dword ptr [ecx + 4], eax
        mov dl, byte ptr [ebx + 0x14]
        mov al, byte ptr [ecx + 0x14]
        mov byte ptr [ecx + 0x14], dl
        mov byte ptr [ebx + 0x14], al
    L_0086ea06:
        mov eax, dword ptr [esp + 0x10]
        mov bl, 1
        cmp byte ptr [eax + 0x14], bl
        jne L_0086eb07
        mov ecx, dword ptr [ebp + 4]
        cmp edi, dword ptr [ecx + 4]
        je L_0086eb04
    L_0086ea21:
        cmp byte ptr [edi + 0x14], bl
        jne L_0086eb04
        mov eax, dword ptr [esi]
        cmp edi, eax
        jne L_0086ea93
        mov eax, dword ptr [esi + 8]
        cmp byte ptr [eax + 0x14], 0
        jne L_0086ea4b
        mov byte ptr [eax + 0x14], bl
        push esi
        mov ecx, ebp
        mov byte ptr [esi + 0x14], 0
        call rotate_left_native_int_pointer_tree18_0086a2f0
        mov eax, dword ptr [esi + 8]
    L_0086ea4b:
        cmp byte ptr [eax + 0x15], 0
        jne L_0086eac3
        mov edx, dword ptr [eax]
        cmp byte ptr [edx + 0x14], bl
        jne L_0086ea60
        mov ecx, dword ptr [eax + 8]
        cmp byte ptr [ecx + 0x14], bl
        je L_0086eabf
    L_0086ea60:
        mov ecx, dword ptr [eax + 8]
        cmp byte ptr [ecx + 0x14], bl
        jne L_0086ea7a
        mov byte ptr [edx + 0x14], bl
        push eax
        mov ecx, ebp
        mov byte ptr [eax + 0x14], 0
        call rotate_right_native_int_pointer_tree18_00869810
        mov eax, dword ptr [esi + 8]
    L_0086ea7a:
        mov cl, byte ptr [esi + 0x14]
        mov byte ptr [eax + 0x14], cl
        mov byte ptr [esi + 0x14], bl
        mov edx, dword ptr [eax + 8]
        push esi
        mov ecx, ebp
        mov byte ptr [edx + 0x14], bl
        call rotate_left_native_int_pointer_tree18_0086a2f0
        jmp L_0086eb04
    L_0086ea93:
        cmp byte ptr [eax + 0x14], 0
        jne L_0086eaaa
        mov byte ptr [eax + 0x14], bl
        push esi
        mov ecx, ebp
        mov byte ptr [esi + 0x14], 0
        call rotate_right_native_int_pointer_tree18_00869810
        mov eax, dword ptr [esi]
    L_0086eaaa:
        cmp byte ptr [eax + 0x15], 0
        jne L_0086eac3
        mov edx, dword ptr [eax + 8]
        cmp byte ptr [edx + 0x14], bl
        jne L_0086ead6
        mov ecx, dword ptr [eax]
        cmp byte ptr [ecx + 0x14], bl
        jne L_0086ead6
    L_0086eabf:
        mov byte ptr [eax + 0x14], 0
    L_0086eac3:
        mov edx, dword ptr [ebp + 4]
        mov edi, esi
        cmp edi, dword ptr [edx + 4]
        mov esi, dword ptr [esi + 4]
        jne L_0086ea21
        jmp L_0086eb04
    L_0086ead6:
        mov ecx, dword ptr [eax]
        cmp byte ptr [ecx + 0x14], bl
        jne L_0086eaee
        mov byte ptr [edx + 0x14], bl
        push eax
        mov ecx, ebp
        mov byte ptr [eax + 0x14], 0
        call rotate_left_native_int_pointer_tree18_0086a2f0
        mov eax, dword ptr [esi]
    L_0086eaee:
        mov cl, byte ptr [esi + 0x14]
        mov byte ptr [eax + 0x14], cl
        mov byte ptr [esi + 0x14], bl
        mov edx, dword ptr [eax]
        push esi
        mov ecx, ebp
        mov byte ptr [edx + 0x14], bl
        call rotate_right_native_int_pointer_tree18_00869810
    L_0086eb04:
        mov byte ptr [edi + 0x14], bl
    L_0086eb07:
        mov eax, dword ptr [esp + 0x10]
        push eax
        call singleton_lifetime_free
        mov eax, dword ptr [ebp + 8]
        add esp, 4
        test eax, eax
        pop edi
        pop ebx
        jbe L_0086eb23
        add eax, -1
        mov dword ptr [ebp + 8], eax
    L_0086eb23:
        mov ecx, dword ptr [esp + 0x64]
        mov eax, dword ptr [esp + 0x60]
        mov edx, dword ptr [esp + 0x68]
        mov dword ptr [eax], ecx
        pop esi
        mov dword ptr [eax + 4], edx
        pop ebp
        add esp, 0x54
        ret 0xc
    }
}

// Complete 0086ee50; full raw normal paths, actual source providers.
__declspec(naked) void* __fastcall erase_native_int_pointer_tree18_range_0086ee50(void*, void*, void*, void*, void*, void*, void*) {
    __asm {
        sub esp, 8
        push ebx
        push ebp
        push esi
        push edi
        mov edi, dword ptr [esp + 0x20]
        test edi, edi
        mov esi, ecx
        mov eax, dword ptr [esi + 4]
        mov ebp, dword ptr [eax]
        je L_0086ee6a
        cmp edi, esi
        je L_0086ee6f
    L_0086ee6a:
        call erase_tree18_invalid_parameter
    L_0086ee6f:
        mov ebx, dword ptr [esp + 0x24]
        cmp ebx, ebp
        jne L_0086eed0
        mov eax, dword ptr [esp + 0x28]
        test eax, eax
        mov ebp, dword ptr [esi + 4]
        je L_0086ee86
        cmp eax, esi
        je L_0086ee8b
    L_0086ee86:
        call erase_tree18_invalid_parameter
    L_0086ee8b:
        cmp dword ptr [esp + 0x2c], ebp
        jne L_0086eed0
        mov ecx, dword ptr [esi + 4]
        mov edx, dword ptr [ecx + 4]
        push edx
        mov ecx, esi
        call erase_subtree_native_int_pointer_tree18_0086aa60
        mov eax, dword ptr [esi + 4]
        mov dword ptr [eax + 4], eax
        mov eax, dword ptr [esi + 4]
        mov dword ptr [esi + 8], 0
        mov dword ptr [eax], eax
        mov eax, dword ptr [esi + 4]
        mov dword ptr [eax + 8], eax
        mov eax, dword ptr [esi + 4]
        mov ecx, dword ptr [eax]
        mov eax, dword ptr [esp + 0x1c]
        pop edi
        mov dword ptr [eax], esi
        pop esi
        pop ebp
        mov dword ptr [eax + 4], ecx
        pop ebx
        add esp, 8
        ret 0x14
        nop
    L_0086eed0:
        test edi, edi
        je L_0086eeda
        cmp edi, dword ptr [esp + 0x28]
        je L_0086eedf
    L_0086eeda:
        call erase_tree18_invalid_parameter
    L_0086eedf:
        cmp ebx, dword ptr [esp + 0x2c]
        je L_0086ef06
        lea ecx, [esp + 0x20]
        call increment_native_int_pointer_tree18_00869a20
        push ebx
        push edi
        lea edx, [esp + 0x18]
        push edx
        mov ecx, esi
        call erase_native_int_pointer_tree18_iterator_0086e8a0
        mov ebx, dword ptr [esp + 0x24]
        mov edi, dword ptr [esp + 0x20]
        jmp L_0086eed0
    L_0086ef06:
        mov eax, dword ptr [esp + 0x1c]
        mov dword ptr [eax], edi
        pop edi
        pop esi
        pop ebp
        mov dword ptr [eax + 4], ebx
        pop ebx
        add esp, 8
        ret 0x14
    }
}

// Complete 0086fde0; full raw normal paths, actual source providers.
__declspec(naked) void __fastcall destroy_native_int_pointer_tree18_0086fde0(void*) {
    __asm {
        sub esp, 8
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 4]
        mov ecx, dword ptr [eax]
        push eax
        push esi
        push ecx
        push esi
        lea eax, [esp + 0x14]
        push eax
        mov ecx, esi
        call erase_native_int_pointer_tree18_range_0086ee50
        mov ecx, dword ptr [esi + 4]
        push ecx
        call singleton_lifetime_free
        add esp, 4
        xor eax, eax
        mov dword ptr [esi + 4], eax
        mov dword ptr [esi + 8], eax
        pop esi
        add esp, 8
        ret
    }
}

} // namespace bsp
