#include "bsp/native_render_batch_lifetime.hpp"
#include <cstring>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
constexpr std::uint32_t reference_table = 0x00ceb130;
constexpr std::uint32_t base_batch_table = 0x00d62064;
constexpr std::uint32_t pooled_batch_table = 0x00d5e5ac;
constexpr std::uint32_t pool_table = 0x00d5e5dc;
constexpr std::uint32_t lock_table = 0x00d5e5d4;
constexpr std::uint32_t simple_owner_table = 0x00ce3818;

class BatchGuard final {
public:
    explicit BatchGuard(TrackedCriticalSection* section) : section_(section) {
        if (section_) {
            EnterCriticalSection(&section_->native);
            add_depth(1);
        }
    }
    ~BatchGuard() {
        if (section_) {
            add_depth(0xffffffffU);
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

template<class T>
void reserve(T**& data, std::int32_t& count, std::int32_t& capacity,
    std::int32_t requested, std::int32_t minimum) {
    if (requested < minimum) requested = minimum;
    if (capacity >= requested) return;
    auto** replacement = static_cast<T**>(singleton_lifetime_allocate({
        SingletonAllocationKind::pointer_slots, static_cast<std::size_t>(requested) * 4,
        static_cast<std::size_t>(requested) * sizeof(T*)}));
    for (std::int32_t i = 0; i < count; ++i) replacement[i] = data[i];
    singleton_lifetime_free(data);
    data = replacement;
    capacity = requested;
}
template<class T>
void resize(T**& data, std::int32_t& count, std::int32_t& capacity,
    std::int32_t requested, std::int32_t minimum) {
    if (requested > capacity) reserve(data, count, capacity, requested, minimum);
    for (std::int32_t i = count; i < requested; ++i) data[i] = nullptr;
    while (requested < count) --count;
    count = requested;
}
} // namespace

NativeRenderBatchStorage* initialize_native_render_batch_00b51d20(void* storage) noexcept {
    auto* batch = ::new (storage) NativeRenderBatchStorage;
    batch->native_vtable_00 = reference_table;
    batch->references_04.store(1, std::memory_order_relaxed);
    batch->native_vtable_00 = base_batch_table;
    batch->entries_0c = nullptr;
    batch->count_10 = 0;
    batch->capacity_14 = 0;
    return batch;
}
void reserve_native_render_batch_entries_00b51b50(
    NativeRenderBatchStorage& batch, std::int32_t requested) {
    reserve(batch.entries_0c, batch.count_10, batch.capacity_14, requested, 256);
}
void resize_native_render_batch_entries_00b51c60(
    NativeRenderBatchStorage& batch, std::int32_t requested) {
    resize(batch.entries_0c, batch.count_10, batch.capacity_14, requested, 256);
}
void destroy_native_render_batch_entries_00b51d00(NativeRenderBatchStorage& batch) {
    resize_native_render_batch_entries_00b51c60(batch, 0);
    singleton_lifetime_free(batch.entries_0c);
}
void destroy_native_render_batch_00b51d50(NativeRenderBatchStorage& batch) {
    batch.native_vtable_00 = base_batch_table;
    try {
        resize_native_render_batch_entries_00b51c60(batch, 0);
    } catch (...) {
        try {
            destroy_native_render_batch_entries_00b51d00(batch);
        } catch (...) {
            batch.native_vtable_00 = reference_table;
            throw;
        }
        batch.native_vtable_00 = reference_table;
        throw;
    }
    try {
        destroy_native_render_batch_entries_00b51d00(batch);
    } catch (...) {
        batch.native_vtable_00 = reference_table;
        throw;
    }
    batch.native_vtable_00 = reference_table;
}
NativeRenderBatchStorage* delete_native_render_batch_00b1c630(
    NativeRenderBatchStorage* batch, std::uint32_t flags) {
    destroy_native_render_batch_00b51d50(*batch);
    if (flags & 1U) singleton_lifetime_free(batch);
    return batch;
}

void reserve_native_render_batch_free_slots_00b1c830(
    NativeRenderBatchFreeSlots& slots, std::int32_t requested) {
    reserve(slots.data_00, slots.count_04, slots.capacity_08, requested, 1);
}
void resize_native_render_batch_free_slots_00b1ce50(
    NativeRenderBatchFreeSlots& slots, std::int32_t requested) {
    resize(slots.data_00, slots.count_04, slots.capacity_08, requested, 1);
}
void destroy_native_render_batch_free_slots_00b1d8a0(NativeRenderBatchFreeSlots& slots) {
    for (std::int32_t i = 0; i < slots.count_04; ++i)
        singleton_lifetime_free(slots.data_00[i]);
    resize_native_render_batch_free_slots_00b1ce50(slots, 0);
    singleton_lifetime_free(slots.data_00);
}

NativeRenderBatchLifetime::NativeRenderBatchLifetime(SoundLifetimeAccess lifetime,
    NativeRenderBatchPoolStorage* volatile& pool,
    NativeRenderBatchLockOwner* volatile& lock,
    const volatile std::uint32_t* table)
    : lifetime_(lifetime), global_0108fe8c_(pool), global_0109dbbc_(lock),
      vtable_00d5e5ac_(table) {
    if (!table) throw std::invalid_argument("Native render batch requires actual D5E5AC table");
}
NativeRenderBatchPoolStorage* NativeRenderBatchLifetime::pool_00b1e870() {
    if (auto* current = global_0108fe8c_) return current;
    {
        CapturedSoundLifetimeSection guard(lifetime_);
        if (!global_0108fe8c_) {
            void* storage = singleton_lifetime_allocate({SingletonAllocationKind::object,
                0x10, sizeof(NativeRenderBatchPoolStorage)});
            NativeRenderBatchPoolStorage* owner = nullptr;
            if (storage) {
                owner = ::new (storage) NativeRenderBatchPoolStorage;
                owner->slots_04.data_00 = nullptr;
                owner->slots_04.count_04 = 0;
                owner->slots_04.capacity_08 = 0;
                owner->native_vtable_00 = pool_table;
            }
            global_0108fe8c_ = owner;
            auto manager = lifetime_.get_manager_00415350();
            manager.register_object(global_0108fe8c_);
        }
    }
    // The native final reload is AFTER unlocking, which can expose a changed
    // publication. Do not evaluate a return expression before guard teardown.
    return global_0108fe8c_;
}
NativeRenderBatchLockOwner* NativeRenderBatchLifetime::initialize_lock_owner_00b1c9d0(
    void* storage) {
    auto* owner = ::new (storage) NativeRenderBatchLockOwner;
    owner->native_vtable_00 = lock_table;
    try {
        owner->section_04 = critical_section_create_00bd1860();
    } catch (...) {
        unwind_lock_owner_00b1c3a0(*owner);
        throw;
    }
    return owner;
}
void NativeRenderBatchLifetime::unwind_lock_owner_00b1c3a0(
    NativeRenderBatchLockOwner& owner) noexcept {
    global_0109dbbc_ = nullptr;
    owner.native_vtable_00 = simple_owner_table;
}
NativeRenderBatchLockOwner* NativeRenderBatchLifetime::lock_owner_00b1cd90() {
    if (auto* current = global_0109dbbc_) return current;
    {
        CapturedSoundLifetimeSection guard(lifetime_);
        if (!global_0109dbbc_) {
            void* storage = singleton_lifetime_allocate({SingletonAllocationKind::object,
                8, sizeof(NativeRenderBatchLockOwner)});
            NativeRenderBatchLockOwner* owner = nullptr;
            try {
                if (storage) owner = initialize_lock_owner_00b1c9d0(storage);
            } catch (...) {
                singleton_lifetime_free(storage);
                throw;
            }
            global_0109dbbc_ = owner;
            auto manager = lifetime_.get_manager_00415350();
            manager.register_object(global_0109dbbc_);
        }
    }
    return global_0109dbbc_;
}
NativeRenderBatchPoolStorage* NativeRenderBatchLifetime::delete_pool_00b1e930(
    NativeRenderBatchPoolStorage* owner, std::uint32_t flags) {
    global_0108fe8c_ = nullptr;
    owner->native_vtable_00 = simple_owner_table;
    destroy_native_render_batch_free_slots_00b1d8a0(owner->slots_04);
    if (flags & 1U) singleton_lifetime_free(owner);
    return owner;
}
NativeRenderBatchLockOwner* NativeRenderBatchLifetime::delete_lock_owner_00b1d530(
    NativeRenderBatchLockOwner* owner, std::uint32_t flags) {
    owner->native_vtable_00 = lock_table;
    critical_section_destroy_owned_0041cc80(owner->section_04);
    global_0109dbbc_ = nullptr;
    owner->native_vtable_00 = simple_owner_table;
    if (flags & 1U) singleton_lifetime_free(owner);
    return owner;
}
NativeRenderBatchStorage* NativeRenderBatchLifetime::acquire_00b1d5b0(
    NativeRenderBatchFreeSlots& slots) {
    NativeRenderBatchStorage* batch;
    {
        BatchGuard guard(lock_owner_00b1cd90()->section_04);
        if (slots.count_04 != 0) {
            batch = slots.data_00[slots.count_04 - 1];
            --slots.count_04;
            if (batch) {
                initialize_native_render_batch_00b51d20(batch);
                batch->native_vtable_00 = pooled_batch_table;
            }
        } else {
            void* storage = singleton_lifetime_allocate({SingletonAllocationKind::object,
                0x18, sizeof(NativeRenderBatchStorage)});
            batch = storage ? initialize_native_render_batch_00b51d20(storage) : nullptr;
            if (batch) batch->native_vtable_00 = pooled_batch_table;
        }
    }
    return batch;
}
void NativeRenderBatchLifetime::append_dead_slot_00b555e0(
    NativeRenderBatchFreeSlots& slots, NativeRenderBatchStorage* batch) {
    BatchGuard guard(lock_owner_00b1cd90()->section_04);
    if (slots.count_04 == slots.capacity_08)
        reserve_native_render_batch_free_slots_00b1c830(slots, slots.capacity_08 * 2);
    slots.data_00[slots.count_04] = batch;
    ++slots.count_04;
}
void NativeRenderBatchLifetime::recycle_zero_reference_00b55680(NativeRenderBatchStorage& batch) {
    const std::uint32_t current_table = batch.native_vtable_00;
    if (current_table != pooled_batch_table || vtable_00d5e5ac_[1] != 0x00b1c630)
        throw std::invalid_argument("Unsupported native render batch deleting virtual+4");
    delete_native_render_batch_00b1c630(&batch, 0);
    auto* current_pool = pool_00b1e870();
    append_dead_slot_00b555e0(current_pool->slots_04, &batch);
}
} // namespace bsp
