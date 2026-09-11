#include "bsp/observer_lifetime.hpp"
#include "bsp/observer_edges.hpp"
#include <algorithm>
#include <cstring>
#include <new>

namespace bsp {
namespace {
constexpr std::uint32_t lock_table = 0x00cf7e70;
constexpr std::uint32_t simple_owner_table = 0x00ce3818;
constexpr std::uint32_t callback_owner_table = 0x00ce3cd4;

std::uintptr_t address(const void* p) noexcept {
    return reinterpret_cast<std::uintptr_t>(p);
}
template<class T> T** offset(T** p, std::uint32_t count) noexcept {
    return reinterpret_cast<T**>(address(p) + count * 4u);
}

class ManagerGuard final {
public:
    explicit ManagerGuard(SystemSingletonCriticalSection* section) : section_(section) {
        if (section_) {
            singleton_enter_critical_section(*section_);
            ++section_->recursion_18;
        }
    }
    ~ManagerGuard() {
        if (section_) {
            --section_->recursion_18;
            singleton_leave_critical_section(*section_);
        }
    }
private:
    SystemSingletonCriticalSection* section_;
};
class ObserverGuard final {
public:
    explicit ObserverGuard(TrackedCriticalSection* section) : section_(section) {
        if (section_) {
            EnterCriticalSection(&section_->native);
            add_depth(1u);
        }
    }
    ~ObserverGuard() {
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

NativeObserverEdgeStorage** allocate_slots(std::uint32_t count) {
    // 00BF55BE is a JMP to 00BF681B; retain that existing CRT allocator.
    const std::uint32_t bytes = count * 4u;
    return static_cast<NativeObserverEdgeStorage**>(singleton_lifetime_allocate({
        SingletonAllocationKind::pointer_slots, bytes, bytes}));
}
void free_slots(NativeObserverEdgeStorage** data) noexcept {
    // 00BF6989 is a JMP to scalar _free at 00BF65AC.
    if (data) singleton_lifetime_free(data);
}
struct TemporarySlots final {
    NativeObserverEdgeSlots slots{nullptr, 0, 0};
    ~TemporarySlots() { free_slots(slots.data_00); }
};

// Inline growth sequence shared by 006944C0 and 00694F60. Store capacity
// BEFORE allocation. The new handler can call game code; reload count and old
// data after it returns, including the otherwise-unusual copy-after-count0.
void reserve_slots(NativeObserverEdgeSlots& slots, std::uint32_t requested) {
    if (slots.capacity_08 < requested) {
        slots.capacity_08 = requested;
        auto** replacement = allocate_slots(requested);
        auto** destination = replacement;
        for (std::uint32_t i = 0; i < slots.count_04; ++i) {
            if (destination) *destination = slots.data_00[i];
            destination = offset(destination, 1);
        }
        free_slots(slots.data_00);
        slots.data_00 = replacement;
    }
}
} // namespace

void exchange_observer_edge_arrays_006944c0(
    NativeObserverEdgeSlots& left, NativeObserverEdgeSlots& right) {
    TemporarySlots temporary;
    const std::uint32_t previous_count = left.count_04;
    if (previous_count != 0) {
        temporary.slots.data_00 = allocate_slots(previous_count);
        temporary.slots.count_04 = previous_count;
        temporary.slots.capacity_08 = previous_count;
        auto** destination = temporary.slots.data_00;
        for (std::uint32_t i = 0; i < previous_count; ++i) {
            if (destination) *destination = left.data_00[i];
            destination = offset(destination, 1);
        }
    }
    if (left.count_04 != 0) left.count_04 = 0;
    reserve_slots(left, right.count_04);
    left.count_04 = right.count_04;
    for (std::uint32_t i = 0; i < right.count_04; ++i) {
        auto** destination = offset(left.data_00, i);
        if (destination) *destination = right.data_00[i];
    }
    if (right.count_04 != 0) right.count_04 = 0;
    reserve_slots(right, previous_count);
    right.count_04 = previous_count;
    for (std::uint32_t i = 0; i < previous_count; ++i) {
        auto** destination = offset(right.data_00, i);
        if (destination) *destination = temporary.slots.data_00[i];
    }
}

void erase_observer_edge_00694f60(
    NativeObserverEdgeSlots& slots, NativeObserverEdgeStorage* value) {
    auto** first = slots.data_00;
    auto** end = offset(first, slots.count_04);
    // Use the host standard library for the native 00694D30 specialization.
    auto** new_end = first == end ? first : std::remove(first, end, value);
    // Native SUB followed by SAR2, not a size_t element-distance conversion.
    const auto count = static_cast<std::uint32_t>(
        static_cast<std::int32_t>(address(new_end) - address(slots.data_00)) >> 2);
    if (count > slots.count_04) {
        reserve_slots(slots, count);
        for (std::uint32_t i = slots.count_04; i < count; ++i) {
            auto** destination = offset(slots.data_00, i);
            if (destination) *destination = nullptr;
        }
    }
    slots.count_04 = count;
}

NativeObserverLifetime::NativeObserverLifetime(SingletonLifetimeDomain& domain,
    NativeObserverLockOwner* volatile& lock,
    NativeObserverDispatchStorage* volatile& dispatch,
    ObserverLifetimeServices& services) noexcept
    : domain_(domain), global_00e198e0_(lock), global_00e198e4_(dispatch),
      services_(services) {}

NativeObserverLockOwner* NativeObserverLifetime::initialize_lock_owner_00694200(void* storage) {
    auto* owner = ::new (storage) NativeObserverLockOwner;
    owner->native_vtable_00 = lock_table;
    try {
        owner->section_04 = critical_section_create_00bd1860();
    } catch (...) {
        // 00C7E910 jumps to the actual base cleanup at 00693C50.
        unwind_lock_owner_00693c50(*owner);
        throw;
    }
    return owner;
}
void NativeObserverLifetime::unwind_lock_owner_00693c50(NativeObserverLockOwner& owner) noexcept {
    global_00e198e0_ = nullptr;
    owner.native_vtable_00 = simple_owner_table;
}
NativeObserverLockOwner* NativeObserverLifetime::delete_lock_owner_00694ea0(
    NativeObserverLockOwner* owner, std::uint32_t flags) {
    owner->native_vtable_00 = lock_table;
    critical_section_destroy_owned_0041cc80(owner->section_04);
    global_00e198e0_ = nullptr;
    owner->native_vtable_00 = simple_owner_table;
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}
NativeObserverLockOwner* NativeObserverLifetime::lock_owner_00694280() {
    if (auto* current = global_00e198e0_) return current;
    {
        ManagerGuard guard(domain_.get_manager_00415350()->system_owner().section_10);
        if (!global_00e198e0_) {
            void* storage = singleton_lifetime_allocate({
                SingletonAllocationKind::object, 8, sizeof(NativeObserverLockOwner)});
            NativeObserverLockOwner* owner = nullptr;
            try {
                if (storage) owner = initialize_lock_owner_00694200(storage);
            } catch (...) {
                singleton_lifetime_free(storage); // 00C7E938
                throw;
            }
            global_00e198e0_ = owner;
            auto* manager = domain_.get_manager_00415350();
            manager->register_object(global_00e198e0_);
        }
    }
    return global_00e198e0_; // native final reload follows unlock
}

NativeObserverEdgeStorage* NativeObserverLifetime::find_pair_006949d0(
    NativeObserverOwnerStorage& first, NativeObserverOwnerStorage& callback_owner) {
    ObserverGuard guard(lock_owner_00694280()->section_04);
    if (first.edges_04.count_04 < callback_owner.edges_04.count_04) {
        auto** cursor = first.edges_04.data_00;
        auto** end = offset(cursor, first.edges_04.count_04);
        if (cursor != end) {
            end = offset(cursor, first.edges_04.count_04); // 00694A08 reload
            do {
                auto* edge = *cursor;
                if (edge->callback_owner_08 == &callback_owner) return edge;
                cursor = offset(cursor, 1);
            } while (cursor != end);
        }
    } else {
        auto** cursor = callback_owner.edges_04.data_00;
        auto** end = offset(cursor, callback_owner.edges_04.count_04);
        if (cursor != end) {
            end = offset(cursor, callback_owner.edges_04.count_04); // 00694A2F
            do {
                auto* edge = *cursor;
                if (edge->first_04 == &first) return edge;
                cursor = offset(cursor, 1);
            } while (cursor != end);
        }
    }
    return nullptr;
}

void NativeObserverLifetime::invalidate_dispatch_slots(
    NativeObserverEdgeStorage* selected_edge, NativeObserverOwnerStorage* selected_owner) {
    auto* const captured_owner = global_00e198e4_;
    auto** cursor = captured_owner->slots_04.begin;
    auto*** const captured_end_field = &captured_owner->slots_04.end;
    if (address(cursor) > address(*captured_end_field))
        services_.invalid_parameter_00bf6713();
    for (;;) {
        // Preserve the owner/end captured BEFORE each validation callback.
        auto* const current_owner = global_00e198e4_;
        auto** const current_end = current_owner->slots_04.end;
        if (address(current_owner->slots_04.begin) > address(current_end))
            services_.invalid_parameter_00bf6713();
        if (captured_owner != current_owner)
            services_.invalid_parameter_00bf6713();
        if (cursor == current_end) break;
        if (address(cursor) >= address(*captured_end_field))
            services_.invalid_parameter_00bf6713();
        auto* edge = static_cast<NativeObserverEdgeStorage*>(*cursor);
        const bool matches = selected_edge ? edge == selected_edge
            : edge && edge->callback_owner_08 == selected_owner;
        if (matches) {
            if (address(cursor) >= address(*captured_end_field))
                services_.invalid_parameter_00bf6713();
            *cursor = nullptr;
        }
        if (address(cursor) >= address(*captured_end_field))
            services_.invalid_parameter_00bf6713();
        cursor = offset(cursor, 1);
    }
}
void NativeObserverLifetime::remove_from_endpoints_and_delete(NativeObserverEdgeStorage& edge) {
    auto* const first = edge.first_04;
    auto* const callback_owner = edge.callback_owner_08; // before first remove
    erase_observer_edge_00694f60(first->edges_04, &edge);
    erase_observer_edge_00694f60(callback_owner->edges_04, &edge);
    const std::uint32_t table = edge.native_vtable_00; // after both removals
    if (table == kObserverEdgeVtable00cf7e64) {
        (void)delete_observer_edge_00693ca0(&edge, 1);
    } else {
        services_.delete_edge_virtual_00(edge, table, 1);
    }
}
void NativeObserverLifetime::unregister_pair_006952a0(
    NativeObserverOwnerStorage& first, NativeObserverOwnerStorage& callback_owner) {
    ObserverGuard guard(lock_owner_00694280()->section_04);
    auto* edge = find_pair_006949d0(first, callback_owner); // separately nested lock
    if (!edge) return;
    invalidate_dispatch_slots(edge, nullptr);
    --edge->references_0c;
    if (edge->references_0c == 0) remove_from_endpoints_and_delete(*edge);
}
void NativeObserverLifetime::detach_all_00695530(NativeObserverOwnerStorage& owner) {
    ObserverGuard guard(lock_owner_00694280()->section_04);
    invalidate_dispatch_slots(nullptr, &owner);
    TemporarySlots detached;
    exchange_observer_edge_arrays_006944c0(detached.slots, owner.edges_04);
    auto** cursor = detached.slots.data_00;
    auto** const end = offset(cursor, detached.slots.count_04);
    while (cursor != end) {
        auto* edge = *cursor;
        remove_from_endpoints_and_delete(*edge); // no reference-count decrement
        cursor = offset(cursor, 1);
    }
    // Detached allocation freed before the captured outer section is released.
}
void NativeObserverLifetime::destroy_callback_owner_00695870(NativeObserverOwnerStorage& owner) {
    owner.native_vtable_00 = callback_owner_table;
    try {
        ObserverGuard outer(lock_owner_00694280()->section_04);
        std::uint32_t count;
        {
            ObserverGuard nested(lock_owner_00694280()->section_04);
            count = owner.edges_04.count_04;
        }
        if (count != 0) detach_all_00695530(owner);
    } catch (...) {
        // 00C7EA90: array cleanup after the 00C7EA9B lock unwind.
        free_slots(owner.edges_04.data_00);
        throw;
    }
    free_slots(owner.edges_04.data_00); // reload after callbacks and unlock
    // Native leaves pointer/count/capacity bytes as they stand at destruction.
}
} // namespace bsp
