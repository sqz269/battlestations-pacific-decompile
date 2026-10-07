#pragma once

#include "bsp/native_pending_entity_producers.hpp"
#include "bsp/tick_element_overrides.hpp"

namespace bsp {

// Connected SOURCE adoption for006E6490's expiry pair at006E659F/006E65A9.
// Borrow the SAME actual F899A8/F899B4 owners and complete producer access;
// neither this service nor the tick host owns/caches an entity or field view.
// Identity must denote live mutable actual entity storage with the observer
// prefix at offset0. Access must resolve its actual fields/current virtuals and
// actual9248D0 lock owner, preserving the existing producer's lifetime domain.
// No native globals, profile installation, owner initialization/drain, private
// FH3/SEH, original binary ABI or gameplay binding is supplied by this facade.
class NativeProjectilePendingEntityServices final : public NativeProjectileTickObserverServices {
public:
    NativeProjectilePendingEntityServices(
        NativePendingEntityOwners&, NativePendingEntityProducerAccess&) noexcept;
    // PURE identical-address alias: no reads, callbacks, allocations or lookup.
    NativeObserverOwnerStorage& observed_prefix(const void* identity) const noexcept override;
    // Direct complete926D90 reuse. Signed source code converts modulo2^32;
    // native cause bits, flags, children, lock and deferred enqueue stay in that
    // body. No null guard, release callback substitute or synchronous deletion.
    void release_projectile_00926d90(const void* identity, int code) override;
private:
    NativePendingEntityOwners& owners_;
    NativePendingEntityProducerAccess& access_;
};

} // namespace bsp
