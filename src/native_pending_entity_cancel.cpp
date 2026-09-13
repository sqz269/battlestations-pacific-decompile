#include "bsp/native_pending_entity_cancel.hpp"

#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
// ADD [section+18h], +/-1 is wrapping DWORD arithmetic, including nonzero
// caller depths. Avoid signed-overflow UB while retaining the canonical field.
void adjust_depth(TrackedCriticalSection& section, std::uint32_t amount) noexcept {
    std::uint32_t bits;
    std::memcpy(&bits, &section.depth, sizeof(bits));
    bits += amount;
    std::memcpy(&section.depth, &bits, sizeof(bits));
}
}

void cancel_native_pending_entity_00925a00(void* entity,
    NativePendingEntityOwners& owners, const NativePendingEntityCancelProviders& providers) {
    if (!providers.lock_owner_009248d0 || !providers.resolve_entity ||
        !providers.remove_all_matching_00781260)
        throw std::logic_error("pending entity cancellation requires actual lock/entity/list providers");

    auto* const section = providers.lock_owner_009248d0(providers.context)->section_04;
    if (section) {
        EnterCriticalSection(&section->native);
        adjust_depth(*section, 1);
    }
    auto target = providers.resolve_entity(providers.context, entity);
    void* const parent_identity = target.parent_3c;
    bool allowed = parent_identity == nullptr;
    if (parent_identity) {
        auto parent = providers.resolve_entity(providers.context, parent_identity);
        allowed = parent.active_5c != 0 && parent.simulate_5d == 0 &&
            parent.destroyed_60 == 0 && parent.pending_5e == 0;
    }
    if (allowed) {
        if (target.killed_5f != 0) {
            void* value = entity;
            providers.remove_all_matching_00781260(providers.context, owners.kill_00f899b4, &value);
            target.killed_5f = 0;
        }
        //00925A55 CMP precedes00925A58 store; preserve the captured branch.
        const bool destroyed = target.destroyed_60 != 0;
        target.pending_5e = 0;
        if (destroyed) {
            void* value = entity;
            providers.remove_all_matching_00781260(providers.context, owners.destroy_00f899a8, &value);
            target.destroyed_60 = 0;
        }
        target.simulate_5d = 0;
    }
    if (section) {
        adjust_depth(*section, 0xffffffffu);
        LeaveCriticalSection(&section->native);
    }
}
} // namespace bsp
