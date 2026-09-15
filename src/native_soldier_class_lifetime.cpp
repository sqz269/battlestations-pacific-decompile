#include "bsp/native_soldier_class_lifetime.hpp"

#include "bsp/native_soldier_loop_lengths.hpp"

#include <exception>

namespace bsp {
namespace {
void* volatile& word(void* storage, std::size_t offset) noexcept {
    return *reinterpret_cast<void* volatile*>(
        static_cast<unsigned char*>(storage) + offset);
}
volatile std::uint32_t& dword(void* storage, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(
        static_cast<unsigned char*>(storage) + offset);
}
} // namespace

void destroy_native_soldier_class_004b1120(
    void* owner, const NativeSoldierClassConstructionAccess& access,
    const SingletonLifetimeCallbacks& callbacks) {
    dword(owner, 0) = access.actual_soldier_vtable_00ce7150;
    void* const original_head = word(owner, 0x2c);
    void* const original_minimum = word(original_head, 0);
    void* const tree = static_cast<unsigned char*>(owner) + 0x28;
    NativeKeyboardTreeIterator ignored;
    try {
        erase_native_soldier_loop_range_00444b10(tree, &ignored,
            {tree, original_minimum}, {tree, original_head}, access.strings, callbacks);
        singleton_lifetime_free(word(tree, 4));
        word(tree, 4) = nullptr;
        dword(tree, 8) = 0;
    } catch (...) {
        // D8CDD0 state0 -> C649D0: actual489E80 base unwind. A completed
        // tree remains partially destroyed if any pool operation throws.
        try { destroy_native_named_class_base_00489e80(owner, access); }
        catch (...) { std::terminate(); }
        throw;
    }
    // Native state is -1 before this call; its failure never repeats cleanup.
    destroy_native_named_class_base_00489e80(owner, access);
}

void* delete_native_soldier_class_004b1310(
    void* owner, std::uint32_t flags,
    const NativeSoldierClassConstructionAccess& access,
    const SingletonLifetimeCallbacks& callbacks) {
    destroy_native_soldier_class_004b1120(owner, access, callbacks);
    if ((flags & 1u) != 0) singleton_lifetime_free(owner);
    return owner;
}
} // namespace bsp
