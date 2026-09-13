#include "bsp/native_pending_entity_producers.hpp"
#include "bsp/effect_deletion_queue.hpp"

#include <cstring>

namespace bsp {
NativePendingEntityNode* NativePendingEntityProducerAccess::call_00924b10(
    NativePendingEntityListStorage&, NativePendingEntityNode* next,
    NativePendingEntityNode* previous, const void* source_cell) {
    return create_effect_deletion_node_008665f0(next, previous, source_cell);
}

void NativePendingEntityProducerAccess::call_009267f0(
    NativePendingEntityListStorage& list, std::uint32_t increment) {
    grow_effect_deletion_list_count_008675e0(list, increment);
}

namespace {
class CapturedPendingSection final {
public:
    explicit CapturedPendingSection(TrackedCriticalSection* section) : section_(section) {
        if (section_) {
            EnterCriticalSection(&section_->native);
            add_depth(1u);
        }
    }
    ~CapturedPendingSection() {
        if (section_) {
            add_depth(0xffffffffu);
            LeaveCriticalSection(&section_->native);
        }
    }
    CapturedPendingSection(const CapturedPendingSection&) = delete;
    CapturedPendingSection& operator=(const CapturedPendingSection&) = delete;
private:
    void add_depth(std::uint32_t increment) noexcept {
        std::uint32_t bits;
        std::memcpy(&bits, &section_->depth, 4);
        bits += increment;
        std::memcpy(&section_->depth, &bits, 4);
    }
    TrackedCriticalSection* section_;
};

void append_pending(NativePendingEntityListStorage& list, void* entity,
    NativePendingEntityProducerAccess& access) {
    auto* const head = list.head_04;
    auto* const previous = head->previous_04;
    auto* const node = access.call_00924b10(list, head, previous, &entity);
    access.call_009267f0(list, 1);
    head->previous_04 = node;
    node->previous_04->next_00 = node;
}
} // namespace

void native_pending_entity_destroy_00926c80(NativePendingEntityOwners& owners,
    void* actual_entity, std::uint32_t recurse, NativePendingEntityProducerAccess& access) {
    const CapturedPendingSection guard(access.lock_owner_009248d0()->section_04);
    const auto entity = access.resolve_entity(actual_entity);
    if (entity.byte_60 != 0) return;
    const auto previous_cause = entity.cause_70; // CMP precedes flag store.
    entity.byte_60 = 1;
    if (previous_cause == 0) {
        void* const parent = entity.parent_3c;
        if (parent && access.resolve_entity(parent).byte_60 != 0)
            entity.cause_70 = access.resolve_entity(parent).cause_70;
        else
            entity.cause_70 = 1;
    }
    if (static_cast<std::uint8_t>(recurse) != 0) {
        for (void* child = entity.first_child_48; child;
            child = access.resolve_entity(child).next_sibling_44) {
            if (access.child_virtual_78(child, actual_entity)) {
                if (entity.cause_70 == 0) access.resolve_entity(child).cause_70 = 0;
                access.destroy_virtual_70(child, 1);
            }
        }
    }
    append_pending(owners.destroy_00f899a8, actual_entity, access);
}

void native_pending_entity_kill_00926d90(NativePendingEntityOwners& owners,
    void* actual_entity, std::uint32_t cause, NativePendingEntityProducerAccess& access) {
    const auto stored_cause = cause == 7 ? 2u : cause;
    const CapturedPendingSection guard(access.lock_owner_009248d0()->section_04);
    const auto entity = access.resolve_entity(actual_entity);
    if (entity.byte_5f != 0) return;
    const auto was_destroyed = entity.byte_60; // CMP precedes killed flag store.
    entity.byte_5f = 1;
    if (was_destroyed == 0) {
        entity.cause_70 = stored_cause;
        access.destroy_virtual_70(actual_entity, 1);
    }
    if (cause == 7) entity.cause_70 = stored_cause;
    for (void* child = entity.first_child_48; child;
        child = access.resolve_entity(child).next_sibling_44) {
        native_pending_entity_kill_00926d90(owners, child, cause, access);
    }
    append_pending(owners.kill_00f899b4, actual_entity, access);
}
} // namespace bsp
