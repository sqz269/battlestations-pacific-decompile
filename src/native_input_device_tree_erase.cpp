#include "bsp/native_input_device_tree_erase.hpp"
#include "bsp/frontend_seg16_24_28_cc13.hpp"
#include "bsp/native_hardware_layout_tree.hpp"
#include "bsp/native_input_device_record_destroy.hpp"
#include "bsp/native_input_device_tree_rotations.hpp"
#include "bsp/native_input_keyboard_iterators.hpp"
#include "bsp/native_legacy_sbo_string.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <type_traits>

namespace bsp {
static_assert(sizeof(void*) == 4, "Actual InputDevice erasure requires Win32");
static_assert(sizeof(cc13_fe5::Raw) == 4 && offsetof(cc13_fe5::Raw, p) == 0);
static_assert(std::is_trivially_copyable_v<cc13_fe5::Raw>);
using cc13_fe5::input_device_tree_min_00552c70;
using cc13_fe5::input_device_tree_max_00552c50;

namespace {
struct CompletedMessage {
    NativeLegacySboStringStorage& message;
    ~CompletedMessage() noexcept {
        unwind_native_input_device_erase_message_00c7f580(message);
    }
};
} // namespace

void unwind_native_input_device_erase_message_00c7f580(
    NativeLegacySboStringStorage& completed_message) noexcept {
    native_legacy_sbo_string_destroy_004072d0(completed_message);
}

void* __fastcall erase_native_input_device_tree_iterator_006a71b0(
    void* tree, std::uint32_t, void* output, void* owner, void* node,
    NativeStringRawPoolContext& strings) {
    void* volatile receiver = tree;
    void* volatile removed = node;
    unsigned char invalid_node;
    __asm {
        mov eax, removed
        cmp byte ptr [eax + 99h], 0
        setne invalid_node
    }
    if (invalid_node != 0) {
        NativeLegacySboStringStorage message;
        message.capacity_18 = 15;
        message.length_14 = 0;
        message.buffer_04.inline_bytes[0] = 0;
        native_legacy_sbo_string_assign_counted_00408720(
            message, "invalid map/set<T> iterator", 27);
        const CompletedMessage completed{message};
        throw NativeHardwareLayoutInvalidIterator{message};
    }

    NativeKeyboardTreeIterator current{owner, removed};
    advance_native_input_device_005540c0(&current);

    // Preserve the native ordered mutation/fixup blocks. Only allocation-free
    // accepted Source leaves are invoked here: min/max use their public cdecl
    // trivial4B Raw argument; rotations use fastcall tree/unusedEDX/stacknode.
    // Throwing iterator/pair calls stay typed C++ calls outside this block.
    __asm {
        mov ebx, removed
        mov eax, dword ptr [ebx]
        cmp byte ptr [eax + 99h], 0
        je e7246
        mov edi, dword ptr [ebx + 8]
        jmp e7261
    e7246:
        mov edx, dword ptr [ebx + 8]
        cmp byte ptr [edx + 99h], 0
        je e7256
        mov edi, eax
        jmp e7261
    e7256:
        mov ecx, current.node
        cmp ecx, ebx
        mov edi, dword ptr [ecx + 8]
        jne e72d5
    e7261:
        cmp byte ptr [edi + 99h], 0
        mov esi, dword ptr [ebx + 4]
        jne e7270
        mov dword ptr [edi + 4], esi
    e7270:
        mov eax, receiver
        mov eax, dword ptr [eax + 4]
        cmp dword ptr [eax + 4], ebx
        jne e727d
        mov dword ptr [eax + 4], edi
        jmp e7288
    e727d:
        cmp dword ptr [esi], ebx
        jne e7285
        mov dword ptr [esi], edi
        jmp e7288
    e7285:
        mov dword ptr [esi + 8], edi
    e7288:
        mov ebx, receiver
        mov ebx, dword ptr [ebx + 4]
        mov eax, dword ptr [ebx]
        cmp eax, removed
        jne e72a9
        cmp byte ptr [edi + 99h], 0
        je e72a0
        mov eax, esi
        jmp e72a7
    e72a0:
        push edi
        call input_device_tree_min_00552c70
        add esp, 4
    e72a7:
        mov dword ptr [ebx], eax
    e72a9:
        mov ebx, receiver
        mov ebx, dword ptr [ebx + 4]
        mov ecx, removed
        cmp dword ptr [ebx + 8], ecx
        jne e733b
        cmp byte ptr [edi + 99h], 0
        je e72c9
        mov eax, esi
        mov dword ptr [ebx + 8], eax
        jmp e733b
    e72c9:
        push edi
        call input_device_tree_max_00552c50
        add esp, 4
        mov dword ptr [ebx + 8], eax
        jmp e733b
    e72d5:
        mov dword ptr [eax + 4], ecx
        mov edx, dword ptr [ebx]
        mov dword ptr [ecx], edx
        cmp ecx, dword ptr [ebx + 8]
        jne e72e5
        mov esi, ecx
        jmp e7302
    e72e5:
        cmp byte ptr [edi + 99h], 0
        mov esi, dword ptr [ecx + 4]
        jne e72f4
        mov dword ptr [edi + 4], esi
    e72f4:
        mov dword ptr [esi], edi
        mov eax, dword ptr [ebx + 8]
        mov dword ptr [ecx + 8], eax
        mov edx, dword ptr [ebx + 8]
        mov dword ptr [edx + 4], ecx
    e7302:
        mov eax, receiver
        mov eax, dword ptr [eax + 4]
        cmp dword ptr [eax + 4], ebx
        jne e730f
        mov dword ptr [eax + 4], ecx
        jmp e731d
    e730f:
        mov eax, dword ptr [ebx + 4]
        cmp dword ptr [eax], ebx
        jne e731a
        mov dword ptr [eax], ecx
        jmp e731d
    e731a:
        mov dword ptr [eax + 8], ecx
    e731d:
        mov eax, dword ptr [ebx + 4]
        mov dword ptr [ecx + 4], eax
        mov dl, byte ptr [ebx + 98h]
        mov al, byte ptr [ecx + 98h]
        mov byte ptr [ecx + 98h], dl
        mov byte ptr [ebx + 98h], al
    e733b:
        mov eax, removed
        mov bl, 1
        cmp byte ptr [eax + 98h], bl
        jne e74ac
        mov ecx, receiver
        mov ecx, dword ptr [ecx + 4]
        cmp edi, dword ptr [ecx + 4]
        je e74a6
    e7360:
        cmp byte ptr [edi + 98h], bl
        jne e74a6
        mov eax, dword ptr [esi]
        cmp edi, eax
        jne e740b
        mov eax, dword ptr [esi + 8]
        cmp byte ptr [eax + 98h], 0
        jne e739a
        mov byte ptr [eax + 98h], bl
        push esi
        mov ecx, receiver
        mov byte ptr [esi + 98h], 0
        xor edx, edx
        call rotate_native_input_device_tree_left_00553f00
        mov eax, dword ptr [esi + 8]
    e739a:
        cmp byte ptr [eax + 99h], 0
        jne e7450
        mov edx, dword ptr [eax]
        cmp byte ptr [edx + 98h], bl
        jne e73c0
        mov ecx, dword ptr [eax + 8]
        cmp byte ptr [ecx + 98h], bl
        je e7449
    e73c0:
        mov ecx, dword ptr [eax + 8]
        cmp byte ptr [ecx + 98h], bl
        jne e73e3
        mov byte ptr [edx + 98h], bl
        push eax
        mov ecx, receiver
        mov byte ptr [eax + 98h], 0
        xor edx, edx
        call rotate_native_input_device_tree_right_00552340
        mov eax, dword ptr [esi + 8]
    e73e3:
        mov cl, byte ptr [esi + 98h]
        mov byte ptr [eax + 98h], cl
        mov byte ptr [esi + 98h], bl
        mov edx, dword ptr [eax + 8]
        push esi
        mov ecx, receiver
        mov byte ptr [edx + 98h], bl
        xor edx, edx
        call rotate_native_input_device_tree_left_00553f00
        jmp e74a6
    e740b:
        cmp byte ptr [eax + 98h], 0
        jne e742b
        mov byte ptr [eax + 98h], bl
        push esi
        mov ecx, receiver
        mov byte ptr [esi + 98h], 0
        xor edx, edx
        call rotate_native_input_device_tree_right_00552340
        mov eax, dword ptr [esi]
    e742b:
        cmp byte ptr [eax + 99h], 0
        jne e7450
        mov edx, dword ptr [eax + 8]
        cmp byte ptr [edx + 98h], bl
        jne e7463
        mov ecx, dword ptr [eax]
        cmp byte ptr [ecx + 98h], bl
        jne e7463
    e7449:
        mov byte ptr [eax + 98h], 0
    e7450:
        mov edx, receiver
        mov edx, dword ptr [edx + 4]
        mov edi, esi
        cmp edi, dword ptr [edx + 4]
        mov esi, dword ptr [esi + 4]
        jne e7360
        jmp e74a6
    e7463:
        mov ecx, dword ptr [eax]
        cmp byte ptr [ecx + 98h], bl
        jne e7484
        mov byte ptr [edx + 98h], bl
        push eax
        mov ecx, receiver
        mov byte ptr [eax + 98h], 0
        xor edx, edx
        call rotate_native_input_device_tree_left_00553f00
        mov eax, dword ptr [esi]
    e7484:
        mov cl, byte ptr [esi + 98h]
        mov byte ptr [eax + 98h], cl
        mov byte ptr [esi + 98h], bl
        mov edx, dword ptr [eax]
        push esi
        mov ecx, receiver
        mov byte ptr [edx + 98h], bl
        xor edx, edx
        call rotate_native_input_device_tree_right_00552340
    e74a6:
        mov byte ptr [edi + 98h], bl
    e74ac:
    }

    destroy_native_input_device_pair_0055b8b0(
        static_cast<unsigned char*>(removed) + 0x0c, 0, strings);
    singleton_lifetime_free(removed);
    void* result;
    __asm {
        mov ecx, receiver
        mov eax, dword ptr [ecx + 8]
        test eax, eax
        jbe e74d4
        add eax, -1
        mov dword ptr [ecx + 8], eax
    e74d4:
        mov ecx, current.owner
        mov eax, output
        mov edx, current.node
        mov dword ptr [eax], ecx
        mov dword ptr [eax + 4], edx
        mov result, eax
    }
    return result;
}
} // namespace bsp
