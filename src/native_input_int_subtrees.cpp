#include "bsp/native_input_int_subtrees.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
// Raw x86 loads retain byte/word widths and their observed ordering, without
// imposing alignment, C++ node lifetime or an invented node/payload overlay.
__forceinline bool nil_11(void* node) {
    std::uint32_t value;
    __asm {
        mov ecx, node
        movzx eax, byte ptr [ecx + 11h]
        mov value, eax
    }
    return value != 0;
}
__forceinline bool nil_15(void* node) {
    std::uint32_t value;
    __asm {
        mov ecx, node
        movzx eax, byte ptr [ecx + 15h]
        mov value, eax
    }
    return value != 0;
}
__forceinline void* right_link(void* node) {
    void* value;
    __asm {
        mov ecx, node
        mov eax, dword ptr [ecx + 8]
        mov value, eax
    }
    return value;
}
__forceinline void* left_link(void* node) {
    void* value;
    __asm {
        mov ecx, node
        mov eax, dword ptr [ecx]
        mov value, eax
    }
    return value;
}
} // namespace

void __fastcall destroy_native_input_int_only_subtree_004c1890(
    void* opaque_tree, std::uint32_t, void* node) {
    while (!nil_11(node)) {
        void* const current = node;
        destroy_native_input_int_only_subtree_004c1890(
            opaque_tree, 0, right_link(current));
        void* const left = left_link(current);
        singleton_lifetime_free(current);
        node = left;
    }
}

void __fastcall destroy_native_input_int_scalar_subtree_00554f10(
    void* opaque_tree, std::uint32_t, void* node) {
    while (!nil_15(node)) {
        void* const current = node;
        destroy_native_input_int_scalar_subtree_00554f10(
            opaque_tree, 0, right_link(current));
        void* const left = left_link(current);
        singleton_lifetime_free(current);
        node = left;
    }
}
} // namespace bsp
