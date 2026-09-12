#include "bsp/native_gameplay_effect_destruction.hpp"

#include "bsp/native_gameplay_effect_construction.hpp"
#include "bsp/native_int_pointer_tree18_erase.hpp"
#include "bsp/native_int_pointer_tree18_leaves.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>

namespace bsp {
namespace {
void* volatile& word(void* owner, std::size_t offset) noexcept {
    return *reinterpret_cast<void* volatile*>(static_cast<std::byte*>(owner) + offset);
}
volatile std::uint32_t& integer(void* owner, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(static_cast<std::byte*>(owner) + offset);
}
struct RawIterator { void* owner; void* node; };
static_assert(sizeof(RawIterator) == 8 && sizeof(void*) == 4);
} // namespace

__declspec(noinline) void __fastcall destroy_native_gameplay_effect_manager_0086fe20(
    void* owner, void* volatile& actual_publication_00f87664) {
    integer(owner, 0) = 0x00d0da64;
    void* const initial_head = word(owner, 8);
    void* const initial_root = word(initial_head, 4);
    void* const tree = static_cast<std::byte*>(owner) + 4;
    // Native state1 begins after profile/root capture, immediately before erase.
    try {
        try {
            erase_subtree_native_int_pointer_tree18_0086aa60(tree, nullptr, initial_root);
            void* head = word(tree, 4);
            word(head, 4) = head;
            head = word(tree, 4);
            integer(tree, 8) = 0;
            word(head, 0) = head;
            head = word(tree, 4);
            word(head, 8) = head;
        } catch (...) {
            destroy_native_int_pointer_tree18_0086fde0(tree);
            throw;
        }

        // State1 is disarmed before full range erase; base state0 remains.
        void* const head = word(tree, 4);
        void* const first = word(head, 0);
        RawIterator result;
        erase_native_int_pointer_tree18_range_0086ee50(
            tree, nullptr, &result, tree, first, tree, head);
        singleton_lifetime_free(word(tree, 4));
        word(tree, 4) = nullptr;
        integer(tree, 8) = 0;
        reset_native_gameplay_effect_manager_00869c50(owner, actual_publication_00f87664);
    } catch (...) {
        reset_native_gameplay_effect_manager_00869c50(owner, actual_publication_00f87664);
        throw;
    }
}

__declspec(naked) void* __fastcall delete_native_gameplay_effect_manager_008703e0(
    void*, void* volatile&, std::uint32_t) {
    __asm {
        push esi
        mov esi, ecx
        call destroy_native_gameplay_effect_manager_0086fe20
        test byte ptr [esp + 8], 1
        je keep_owner
        push esi
        call singleton_lifetime_free
        add esp, 4
    keep_owner:
        mov eax, esi
        pop esi
        ret 4
    }
}
} // namespace bsp
