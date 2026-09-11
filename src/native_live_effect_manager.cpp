#include "bsp/native_live_effect_manager.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native live-effect manager storage requires MSVC Win32.
#endif

namespace bsp {

NativeEffectDeletionNode* create_native_effect_deletion_sentinel_004c3200() {
    void* const raw = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x0c, sizeof(NativeEffectDeletionNode)});
    // Keep the two native address tests independent. A null allocator result
    // does not represent a successfully constructed empty sentinel.
    if (raw) {
        auto* const node = ::new (raw) NativeEffectDeletionNode;
        node->next_00 = node;
    }
    const auto previous = reinterpret_cast<std::uintptr_t>(raw) + 4u;
    if (previous) std::memcpy(reinterpret_cast<void*>(previous), &raw, sizeof(raw));
    return static_cast<NativeEffectDeletionNode*>(raw);
}

void destroy_native_effect_deletion_list_004c5940(
    NativeEffectDeletionListStorage& input) noexcept {
    volatile auto& list = input;
    auto* const initial_head = list.head_04;
    auto* cursor = initial_head->next_00;
    initial_head->next_00 = initial_head;
    auto* const current_head = list.head_04;
    current_head->previous_04 = current_head;
    list.count_08 = 0;
    while (cursor != list.head_04) {
        auto* const next = cursor->next_00;
        singleton_lifetime_free(cursor);
        cursor = next;
    }
    singleton_lifetime_free(list.head_04);
    list.head_04 = nullptr;
}

void unwind_native_live_effect_manager_base_004b7f50(
    NativeLiveEffectManagerStorage& owner,
    NativeLiveEffectManagerStorage* volatile& global) noexcept {
    global = nullptr;
    owner.native_vtable_00 = 0x00ce3818u;
}

NativeLiveEffectManagerStorage& construct_native_live_effect_manager_004cf700(
    NativeLiveEffectManagerStorage& input,
    NativeLiveEffectManagerStorage* volatile& global) {
    volatile auto& owner = input;
    owner.native_vtable_00 = 0x00ce789cu;
    NativeEffectDeletionNode* sentinel;
    try {
        sentinel = create_native_effect_deletion_sentinel_004c3200();
    } catch (...) {
        unwind_native_live_effect_manager_base_004b7f50(input, global);
        throw;
    }
    owner.pending_04.head_04 = sentinel;
    owner.pending_04.count_08 = 0;
    owner.effects_10.data_00 = nullptr;
    owner.effects_10.count_04 = 0;
    owner.effects_10.capacity_08 = 0;
    owner.references_1c.data_00 = nullptr;
    owner.references_1c.count_04 = 0;
    owner.references_1c.capacity_08 = 0;
    return input;
}

} // namespace bsp
