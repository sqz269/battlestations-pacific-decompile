#include "bsp/observer_edges.hpp"
#include <cstring>
#include <new>

namespace bsp {
namespace {
class ObserverEdgeGuard final {
public:
    explicit ObserverEdgeGuard(TrackedCriticalSection* section) : section_(section) {
        if (section_) {
            EnterCriticalSection(&section_->native);
            add_depth(1u);
        }
    }
    ~ObserverEdgeGuard() {
        if (section_) {
            add_depth(0xffffffffu);
            LeaveCriticalSection(&section_->native);
        }
    }
private:
    void add_depth(std::uint32_t amount) noexcept {
        std::uint32_t bits;
        std::memcpy(&bits, &section_->depth, 4);
        bits += amount;
        std::memcpy(&section_->depth, &bits, 4);
    }
    TrackedCriticalSection* section_;
};

NativeObserverEdgeStorage** offset(NativeObserverEdgeStorage** data,
    std::uint32_t count) noexcept {
    return reinterpret_cast<NativeObserverEdgeStorage**>(
        reinterpret_cast<std::uintptr_t>(data) + count * 4u);
}

// The two inlined append sequences in00694850. Equality triggers growth; do
// not replace it with >= or impose a different capacity policy. A CRT new
// handler can reenter game code, so count and old data are reloaded afterward.
void append_edge(NativeObserverEdgeSlots& slots, NativeObserverEdgeStorage* edge) {
    const std::uint32_t old_capacity = slots.capacity_08;
    if (slots.count_04 == old_capacity) {
        const std::uint32_t capacity = old_capacity * 2u + 2u;
        slots.capacity_08 = capacity;
        const std::uint32_t bytes = capacity * 4u;
        auto** replacement = static_cast<NativeObserverEdgeStorage**>(
            singleton_lifetime_allocate({SingletonAllocationKind::pointer_slots, bytes, bytes}));
        auto** destination = replacement;
        for (std::uint32_t i = 0; i < slots.count_04; ++i) {
            if (destination) *destination = slots.data_00[i];
            destination = offset(destination, 1);
        }
        if (slots.data_00) singleton_lifetime_free(slots.data_00);
        slots.data_00 = replacement;
    }
    auto** destination = offset(slots.data_00, slots.count_04);
    if (destination) *destination = edge;
    ++slots.count_04;
}
} // namespace

NativeObserverEdgeStorage* create_observer_edge_00694850(
    NativeObserverOwnerStorage& first, NativeObserverOwnerStorage& callback_owner,
    NativeObserverLifetime& lifetime) {
    ObserverEdgeGuard guard(lifetime.lock_owner_00694280()->section_04);
    void* storage = singleton_lifetime_allocate({
        SingletonAllocationKind::object, 0x10, sizeof(NativeObserverEdgeStorage)});
    NativeObserverEdgeStorage* edge = nullptr;
    if (storage) {
        edge = ::new (storage) NativeObserverEdgeStorage;
        edge->native_vtable_00 = kObserverEdgeVtable00cf7e64;
        edge->first_04 = &first;
        edge->callback_owner_08 = &callback_owner;
        edge->references_0c = 1;
    }
    append_edge(first.edges_04, edge);
    append_edge(callback_owner.edges_04, edge);
    // Native EH00C7E970 only releases the captured section. An array allocation
    // exception leaves any completed earlier append and allocated edge intact.
    return edge;
}

void register_observer_pair_00694a60(
    NativeObserverOwnerStorage& first, NativeObserverOwnerStorage& callback_owner,
    NativeObserverLifetime& lifetime) {
    ObserverEdgeGuard guard(lifetime.lock_owner_00694280()->section_04);
    if (auto* edge = lifetime.find_pair_006949d0(first, callback_owner)) {
        ++edge->references_0c;
    } else {
        (void)create_observer_edge_00694850(first, callback_owner, lifetime);
    }
}

NativeObserverEdgeStorage* delete_observer_edge_00693ca0(
    NativeObserverEdgeStorage* edge, std::uint32_t flags) noexcept {
    edge->native_vtable_00 = kObserverEdgeVtable00cf7e64;
    if ((flags & 1u) != 0) singleton_lifetime_free(edge);
    return edge;
}
} // namespace bsp
