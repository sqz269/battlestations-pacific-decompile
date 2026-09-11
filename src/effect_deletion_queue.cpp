#include "bsp/effect_deletion_queue.hpp"
#include "bsp/native_alias_count_growth.hpp"
#include <cstring>
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Effect deletion queue requires MSVC Win32.
#endif

namespace bsp {
namespace {
class CapturedSection final {
public:
    explicit CapturedSection(SystemSingletonCriticalSection* section) : section_(section) {
        if (section_) {
            singleton_enter_critical_section(*section_);
            ++section_->recursion_18;
        }
    }
    ~CapturedSection() {
        if (section_) {
            --section_->recursion_18;
            singleton_leave_critical_section(*section_);
        }
    }
    CapturedSection(const CapturedSection&) = delete;
    CapturedSection& operator=(const CapturedSection&) = delete;
private:
    SystemSingletonCriticalSection* section_;
};
struct CompletedLengthMessage {
    NativeLegacySboStringStorage& value;
    ~CompletedLengthMessage() noexcept { native_legacy_sbo_string_destroy_004072d0(value); }
};
}

void unwind_effect_deletion_lock_base_00866040(EffectManager& owner,
    EffectManager* volatile& global) noexcept {
    global = nullptr;
    owner.original_vtable_identity_00 = 0x00ce3818u;
}

EffectManager& construct_effect_deletion_lock_008662b0(EffectManager& owner,
    EffectManager* volatile& global, EffectManagerLifetimeAccess& access) {
    owner.original_vtable_identity_00 = 0x00d0d3d0u;
    try {
        owner.section_04 = access.create_section_00bd1860();
    } catch (...) {
        unwind_effect_deletion_lock_base_00866040(owner, global);
        throw;
    }
    return owner;
}

EffectManager* effect_deletion_lock_singleton_00866500(
    EffectManager* volatile& global, EffectManagerLifetimeAccess& access) {
    if (auto* existing = global) return existing;
    {
        CapturedSection section(access.manager_00415350().section_10);
        if (!global) {
            void* const raw = access.allocate_00bf681b(
                {SingletonAllocationKind::object, 8, sizeof(EffectManager)});
            EffectManager* created = nullptr;
            if (raw) {
                created = ::new (raw) EffectManager;
                try {
                    construct_effect_deletion_lock_008662b0(*created, global, access);
                } catch (...) {
                    created->~EffectManager();
                    access.free_00bf65ac(raw);
                    throw;
                }
            }
            global = created;
            auto& manager = access.manager_00415350();
            access.register_00bd0c30(manager, global);
        }
    }
    return global;
}

EffectManager* delete_effect_deletion_lock_00866790(EffectManager* owner,
    std::uint32_t flags, EffectManager* volatile& global,
    EffectManagerLifetimeAccess& access) noexcept {
    auto* const original = owner;
    owner->original_vtable_identity_00 = 0x00d0d3d0u;
    access.destroy_section_0041cc80(owner->section_04);
    unwind_effect_deletion_lock_base_00866040(*owner, global);
    if (flags & 1u) {
        owner->~EffectManager();
        access.free_00bf65ac(owner);
    }
    return original;
}

NativeEffectDeletionNode* create_effect_deletion_node_008665f0(
    NativeEffectDeletionNode* next, NativeEffectDeletionNode* previous, const void* source) {
    void* const raw = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x0c, sizeof(NativeEffectDeletionNode)});
    if (raw) {
        auto* const node = ::new (raw) NativeEffectDeletionNode;
        node->next_00 = next;
    }
    const auto previous_cell = reinterpret_cast<std::uintptr_t>(raw) + 4u;
    if (previous_cell) std::memcpy(reinterpret_cast<void*>(previous_cell), &previous, 4);
    const auto payload_cell = reinterpret_cast<std::uintptr_t>(raw) + 8u;
    if (payload_cell) {
        void* captured;
        std::memcpy(&captured, source, 4);
        std::memcpy(reinterpret_cast<void*>(payload_cell), &captured, 4);
    }
    return static_cast<NativeEffectDeletionNode*>(raw);
}

void grow_effect_deletion_list_count_008675e0(
    NativeEffectDeletionListStorage& list, std::uint32_t increment) {
    volatile auto& actual_count = list.count_08;
    const auto captured = actual_count;
    if (std::uint32_t{0x3fffffff} - captured < increment) {
        NativeLegacySboStringStorage message;
        message.capacity_18 = 15;
        message.length_14 = 0;
        message.buffer_04.inline_bytes[0] = '\0';
        native_legacy_sbo_string_assign_counted_00408720(message, "list<T> too long", 16);
        const CompletedLengthMessage completed{message};
        throw NativeAliasListLengthError{message};
    }
    actual_count = captured + increment;
}

void enqueue_effect_deletion_00868010(NativeLiveEffectManagerStorage& owner, void* effect,
    EffectManager* volatile& global, EffectManagerLifetimeAccess& access) {
    auto* const manager = effect_deletion_lock_singleton_00866500(global, access);
    CapturedSection section(manager->section_04);
    auto* const head = owner.pending_04.head_04;
    auto* const previous = head->previous_04;
    auto* const node = create_effect_deletion_node_008665f0(head, previous, &effect);
    grow_effect_deletion_list_count_008675e0(owner.pending_04, 1);
    head->previous_04 = node;
    node->previous_04->next_00 = node;
}

void flush_effect_deletions_008671a0(NativeLiveEffectManagerStorage& owner,
    EffectDeletionDispatch& dispatch) {
    volatile auto& list = owner.pending_04;
    while (list.count_08 != 0) {
        auto* head = list.head_04;
        auto* node = head->next_00;
        if (node == head) dispatch.invalid_parameter_00bf6713();
        if (void* const payload = node->payload_08) {
            dispatch.scalar_delete_current_04(payload, 1);
            node->payload_08 = nullptr;
        }
        head = list.head_04;
        node = head->next_00;
        if (node == head) dispatch.invalid_parameter_00bf6713();
        if (node != list.head_04) {
            volatile auto& current = *node;
            current.previous_04->next_00 = current.next_00;
            current.next_00->previous_04 = current.previous_04;
            singleton_lifetime_free(node);
            list.count_08 = list.count_08 - 1u;
        }
    }
}

} // namespace bsp
