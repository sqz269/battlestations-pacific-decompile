#include "bsp/native_input_device_subtree.hpp"
#include "bsp/native_input_device_record_destroy.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
static_assert(sizeof(void*) == 4, "Actual device-tree storage requires Win32");

void unwind_native_input_device_subtree_key_00c7f5a0(void* saved_current_pair,
    NativeStringRawPoolContext& strings) {
    destroy_native_string_header_0041dd20(saved_current_pair, strings);
}

void __fastcall destroy_native_input_device_subtree_006a7540(
    void* opaque_tree, std::uint32_t, void* node,
    NativeStringRawPoolContext& strings) {
    std::uint32_t ordinary;
    __asm {
        mov ecx, node
        xor eax, eax
        cmp byte ptr [ecx + 0x99], 0
        sete al
        mov ordinary, eax
    }
    while (ordinary != 0) {
        void* right;
        __asm {
            mov ecx, node
            mov eax, dword ptr [ecx + 8]
            mov right, eax
        }
        destroy_native_input_device_subtree_006a7540(opaque_tree, 0, right, strings);

        void* left;
        void* pair;
        __asm {
            mov ecx, node
            mov eax, dword ptr [ecx]
            mov left, eax
            add ecx, 0x0c
            mov pair, ecx
        }
        // Complete independently compared record/key schedule, including its
        // state0 key unwind and CurrentCpp secondary-failure termination.
        // There is no second subtree key guard around this delegate.
        destroy_native_input_device_pair_0055b8b0(pair, 0, strings);
        singleton_lifetime_free(node);

        __asm {
            mov ecx, left
            xor eax, eax
            cmp byte ptr [ecx + 0x99], 0
            sete al
            mov ordinary, eax
            mov node, ecx
        }
    }
}
} // namespace bsp
