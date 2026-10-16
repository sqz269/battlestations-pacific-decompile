#include "bsp/native_input_device_tree_range.hpp"
#include "bsp/native_input_device_subtree.hpp"
#include "bsp/native_input_device_tree_erase.hpp"
#include "bsp/native_input_keyboard_iterators.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"

#include <cstddef>

namespace bsp {
static_assert(sizeof(void*) == 4, "Actual InputDevice range erasure requires Win32");
static_assert(sizeof(NativeKeyboardTreeIterator) == 8);
static_assert(offsetof(NativeKeyboardTreeIterator, owner) == 0);
static_assert(offsetof(NativeKeyboardTreeIterator, node) == 4);

void* __fastcall erase_native_raw_input_device_tree_range_006a7aa0(
    void* tree, std::uint32_t, void* volatile output,
    void* volatile first_owner, void* volatile first_node,
    void* volatile last_owner, void* volatile last_node,
    NativeStringRawPoolContext& strings) {
    void* volatile receiver = tree;
    void* volatile cached_owner;
    void* volatile cached_node;
    void* boundary;
    unsigned char diagnostic_required;
    unsigned char selected_full;
    NativeKeyboardTreeIterator current;
    NativeKeyboardTreeIterator ignored_erase_output;

    // Form the Source private8B iterator progressively, not an eager pair copy.
    // Original private argument/frame aliases are qualified. Volatile original
    // Source input words prevent premature node/end/output parameter snapshots.
    __asm {
        mov edi, first_owner
        mov cached_owner, edi
        mov current.owner, edi
        test edi, edi
        mov esi, receiver
        mov eax, dword ptr [esi + 4]
        mov ebx, dword ptr [eax]
        mov boundary, ebx
        jz initial_bad_owner
        cmp edi, esi
        je initial_good_owner
    initial_bad_owner:
        mov diagnostic_required, 1
        jmp initial_owner_done
    initial_good_owner:
        mov diagnostic_required, 0
    initial_owner_done:
    }
    if (diagnostic_required != 0) {
        invoke_native_invalid_parameter_00bf6713();
    }
    __asm {
        mov ebx, first_node
        mov cached_node, ebx
        mov current.node, ebx
        cmp ebx, boundary
        sete selected_full
    }
    if (selected_full != 0) {
        __asm {
            mov eax, last_owner
            test eax, eax
            mov ecx, receiver
            mov edx, dword ptr [ecx + 4]
            mov boundary, edx
            jz selection_bad_owner
            cmp eax, ecx
            je selection_good_owner
        selection_bad_owner:
            mov diagnostic_required, 1
            jmp selection_owner_done
        selection_good_owner:
            mov diagnostic_required, 0
        selection_owner_done:
        }
        if (diagnostic_required != 0) {
            invoke_native_invalid_parameter_00bf6713();
        }
        __asm {
            mov eax, last_node
            cmp eax, boundary
            sete selected_full
        }
        if (selected_full != 0) {
            void* root;
            __asm {
                mov ecx, receiver
                mov eax, dword ptr [ecx + 4]
                mov edx, dword ptr [eax + 4]
                mov root, edx
            }
            destroy_native_input_device_subtree_006a7540(
                receiver, 0, root, strings);
            void* result;
            __asm {
                mov esi, receiver
                mov eax, dword ptr [esi + 4]
                mov dword ptr [eax + 4], eax
                mov eax, dword ptr [esi + 4]
                mov dword ptr [esi + 8], 0
                mov dword ptr [eax], eax
                mov eax, dword ptr [esi + 4]
                mov dword ptr [eax + 8], eax
                mov eax, dword ptr [esi + 4]
                mov ecx, dword ptr [eax]
                mov eax, output
                mov dword ptr [eax], esi
                mov dword ptr [eax + 4], ecx
                mov result, eax
            }
            return result;
        }
    }

    for (;;) {
        __asm {
            mov edi, cached_owner
            test edi, edi
            jz loop_bad_owner
            cmp edi, last_owner
            je loop_good_owner
        loop_bad_owner:
            mov diagnostic_required, 1
            jmp loop_owner_done
        loop_good_owner:
            mov diagnostic_required, 0
        loop_owner_done:
        }
        if (diagnostic_required != 0) {
            invoke_native_invalid_parameter_00bf6713();
        }
        __asm {
            mov ebx, cached_node
            cmp ebx, last_node
            sete selected_full
        }
        if (selected_full != 0) {
            break;
        }
        advance_native_input_device_005540c0(&current);
        erase_native_input_device_tree_iterator_006a71b0(
            receiver, 0, &ignored_erase_output, cached_owner, cached_node, strings);
        __asm {
            mov eax, current.node
            mov cached_node, eax
            mov eax, current.owner
            mov cached_owner, eax
        }
    }
    void* result;
    __asm {
        mov eax, output
        mov ecx, cached_owner
        mov edx, cached_node
        mov dword ptr [eax], ecx
        mov dword ptr [eax + 4], edx
        mov result, eax
    }
    return result;
}
} // namespace bsp
