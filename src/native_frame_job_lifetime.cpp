#include "bsp/native_frame_job_lifetime.hpp"
#include <cstdlib>
#include <cstring>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native frame-job lifetime requires MSVC Win32.
#endif

namespace bsp {
namespace {
std::int32_t signed_word(std::uint32_t word) noexcept {
    std::int32_t value;
    std::memcpy(&value, &word, 4);
    return value;
}
void* allocate_array(std::int32_t count) {
    const auto product = static_cast<std::uint64_t>(static_cast<std::uint32_t>(count)) * 4u;
    const auto bytes = product > 0xffffffffu ? 0xffffffffu : static_cast<std::uint32_t>(product);
    return singleton_lifetime_allocate({SingletonAllocationKind::pointer_slots, bytes, bytes});
}
template<class T> void free_and_clear(T* volatile& pointer) noexcept {
    if (auto* const captured = pointer) {
        singleton_lifetime_free(captured);
        pointer = nullptr;
    }
}
class PoolBaseUnwind final {
public:
    explicit PoolBaseUnwind(NativeFrameJobPoolStorage& value) noexcept : pool(value) {}
    ~PoolBaseUnwind() { if (armed) unwind_frame_job_pool_base_00be2c30(pool); }
    bool armed{true};
private:
    NativeFrameJobPoolStorage& pool;
};
class OwnerBaseUnwind final {
public:
    OwnerBaseUnwind(NativeFrameJobOwnerStorage& value,
        NativeFrameJobOwnerStorage* volatile& published) noexcept : owner(value), global(published) {}
    ~OwnerBaseUnwind() { if (armed) unwind_frame_job_owner_base_004b7a00(owner, global); }
    bool armed{true};
private:
    NativeFrameJobOwnerStorage& owner;
    NativeFrameJobOwnerStorage* volatile& global;
};
class CapturedSection final {
public:
    explicit CapturedSection(SystemSingletonCriticalSection* value) : section(value) {
        if (section) { singleton_enter_critical_section(*section); ++section->recursion_18; }
    }
    ~CapturedSection() {
        if (section) { --section->recursion_18; singleton_leave_critical_section(*section); }
    }
private:
    SystemSingletonCriticalSection* section;
};
}

NativeFrameJobLifetimeBindings::NativeFrameJobLifetimeBindings(NativeFrameJobExecution& value,
    std::atomic<std::int32_t>& counter, NativeFrameJobStartRoutine routine)
    : execution(value), scope(counter), entry(routine) {
    if (!entry) throw std::invalid_argument("Frame job lifetime requires the application's actual worker entry binding");
}
std::int32_t frame_job_processor_count_00be4800() {
    char* text = nullptr;
    std::size_t bytes;
    if (_dupenv_s(&text, &bytes, "NUMBER_OF_PROCESSORS") != 0) return 1;
    const auto count = std::atol(text);
    std::free(text);
    return static_cast<std::int32_t>(count);
}
void unwind_frame_job_pool_base_00be2c30(NativeFrameJobPoolStorage& pool) noexcept {
    pool.native_vtable_00 = 0x00d68638u;
}
void unwind_frame_job_owner_base_004b7a00(NativeFrameJobOwnerStorage& owner,
    NativeFrameJobOwnerStorage* volatile& global) noexcept {
    global = nullptr;
    owner.native_vtable_00 = 0x00ce3818u;
}
void create_native_frame_job_workers_00be2c70(NativeFrameJobPoolStorage& pool,
    std::int32_t processors, NativeFrameJobLifetimeBindings& bindings) {
    pool.dispatch_active_18.store(0, std::memory_order_relaxed);
    pool.stop_requested_1c.store(0, std::memory_order_relaxed);
    pool.worker_count_14 = signed_word(static_cast<std::uint32_t>(processors) - 1u);
    pool.remaining_20.store(0, std::memory_order_relaxed);
    std::memset(pool.slots_24.data(), 0, sizeof(pool.slots_24));
    if (pool.worker_count_14 > 0) {
        pool.thread_handles_04 = static_cast<void**>(allocate_array(pool.worker_count_14));
        pool.thread_ids_08 = static_cast<std::uint32_t*>(allocate_array(pool.worker_count_14));
        pool.start_events_0c = static_cast<NativeEventOwnerStorage**>(allocate_array(pool.worker_count_14));
        pool.done_events_10 = static_cast<NativeEventOwnerStorage**>(allocate_array(pool.worker_count_14));
        for (std::int32_t index = 0; index < pool.worker_count_14; ++index) {
            auto* const start = create_native_event_owner_00bd1970(1);
            pool.start_events_0c[index] = start;
            auto* const done = create_native_event_owner_00bd1970(1);
            pool.done_events_10[index] = done;
            bindings.execution.signal(pool.done_events_10[index]);
            auto* const handle = CreateThread(nullptr, 0, bindings.entry, &pool, CREATE_SUSPENDED,
                reinterpret_cast<DWORD*>(&pool.thread_ids_08[index]));
            pool.thread_handles_04[index] = handle;
            (void)SetThreadPriority(pool.thread_handles_04[index], 2);
        }
    }
}
NativeFrameJobPoolStorage* construct_native_frame_job_pool_00be3040(
    NativeFrameJobPoolStorage& pool, std::int32_t processors, NativeFrameJobLifetimeBindings& bindings) {
    PoolBaseUnwind unwind(pool);
    pool.native_vtable_00 = 0x00d68650u;
    pool.thread_handles_04 = nullptr;
    pool.thread_ids_08 = nullptr;
    pool.done_events_10 = nullptr;
    for (auto& slot : pool.slots_24) { slot.owner_00 = nullptr; slot.argument_04 = 0; }
    if (processors == -1) processors = frame_job_processor_count_00be4800();
    create_native_frame_job_workers_00be2c70(pool, processors, bindings);
    unwind.armed = false;
    return &pool;
}
void stop_native_frame_job_workers_00be2da0(NativeFrameJobPoolStorage& pool,
    NativeFrameJobLifetimeBindings& bindings) {
    for (std::int32_t index = 0; index < pool.worker_count_14; ++index)
        bindings.execution.wait(pool.done_events_10[index]);
    bindings.execution.require_dispatch_virtual_08(pool);
    pool.stop_requested_1c.store(1, std::memory_order_relaxed);
    dispatch_native_frame_jobs_00be3150(pool, 1, bindings.scope, bindings.execution);
    auto* const handles = pool.thread_handles_04;
    const auto count = pool.worker_count_14;
    (void)WaitForMultipleObjects(static_cast<DWORD>(count), handles, TRUE, INFINITE);
    for (std::int32_t index = 0; index < pool.worker_count_14; ++index) {
        (void)CloseHandle(pool.thread_handles_04[index]);
        auto* const start_cell = &pool.start_events_0c[index];
        if (auto* const start = *start_cell) {
            bindings.execution.delete_event(start);
            *start_cell = nullptr;
        }
        auto* const done_cell = &pool.done_events_10[index];
        if (auto* const done = *done_cell) {
            bindings.execution.delete_event(done);
            *done_cell = nullptr;
        }
    }
    free_and_clear(pool.start_events_0c);
    free_and_clear(pool.done_events_10);
    free_and_clear(pool.thread_handles_04);
    free_and_clear(pool.thread_ids_08);
}
void destroy_native_frame_job_pool_00be30c0(NativeFrameJobPoolStorage& pool,
    NativeFrameJobLifetimeBindings& bindings) {
    pool.native_vtable_00 = 0x00d68650u;
    PoolBaseUnwind unwind(pool);
    stop_native_frame_job_workers_00be2da0(pool, bindings);
}
NativeFrameJobPoolStorage* delete_native_frame_job_pool_00be31e0(
    NativeFrameJobPoolStorage* pool, std::uint32_t flags, NativeFrameJobLifetimeBindings& bindings) {
    auto* const original = pool;
    destroy_native_frame_job_pool_00be30c0(*pool, bindings);
    if (flags & 1u) { pool->~NativeFrameJobPoolStorage(); singleton_lifetime_free(pool); }
    return original;
}
NativeFrameJobOwnerStorage* construct_native_frame_job_owner_004bfa40(
    NativeFrameJobOwnerStorage& owner, NativeFrameJobOwnerStorage* volatile& global,
    NativeFrameJobLifetimeBindings& bindings) {
    owner.native_vtable_00 = 0x00ce7520u;
    OwnerBaseUnwind unwind(owner, global);
    (void)construct_native_frame_job_pool_00be3040(owner.pool_04, -1, bindings);
    owner.pool_04.native_vtable_00 = 0x00ce7554u;
    owner.native_vtable_00 = 0x00ce7550u;
    unwind.armed = false;
    return &owner;
}
void destroy_native_frame_job_owner_004bfad0(NativeFrameJobOwnerStorage& owner,
    NativeFrameJobOwnerStorage* volatile& global, NativeFrameJobLifetimeBindings& bindings) {
    OwnerBaseUnwind unwind(owner, global);
    destroy_native_frame_job_pool_00be30c0(owner.pool_04, bindings);
}
NativeFrameJobOwnerStorage* delete_native_frame_job_owner_004bfb30(
    NativeFrameJobOwnerStorage* owner, std::uint32_t flags,
    NativeFrameJobOwnerStorage* volatile& global, NativeFrameJobLifetimeBindings& bindings) {
    auto* const original = owner;
    destroy_native_frame_job_owner_004bfad0(*owner, global, bindings);
    if (flags & 1u) { owner->~NativeFrameJobOwnerStorage(); singleton_lifetime_free(owner); }
    return original;
}
NativeFrameJobOwnerStorage* delete_native_frame_job_secondary_004bfac0(
    NativeFrameJobPoolStorage* pool, std::uint32_t flags,
    NativeFrameJobOwnerStorage* volatile& global, NativeFrameJobLifetimeBindings& bindings) {
    auto* const primary = reinterpret_cast<NativeFrameJobOwnerStorage*>(
        reinterpret_cast<std::uintptr_t>(pool) - 4u);
    return delete_native_frame_job_owner_004bfb30(primary, flags, global, bindings);
}
NativeFrameJobOwnerStorage* native_frame_job_singleton_004c1130(
    NativeFrameJobOwnerStorage* volatile& global, SingletonLifetimeDomain& domain,
    NativeFrameJobLifetimeBindings& bindings) {
    if (auto* existing = global) return existing;
    {
        CapturedSection section(domain.get_manager_00415350()->system_owner().section_10);
        if (!global) {
            void* const raw = singleton_lifetime_allocate(
                {SingletonAllocationKind::object, 0x138a8, sizeof(NativeFrameJobOwnerStorage)});
            NativeFrameJobOwnerStorage* created = nullptr;
            if (raw) {
                created = ::new (raw) NativeFrameJobOwnerStorage;
                try { construct_native_frame_job_owner_004bfa40(*created, global, bindings); }
                catch (...) {
                    created->~NativeFrameJobOwnerStorage();
                    singleton_lifetime_free(raw);
                    throw;
                }
            }
            global = created;
            auto* const manager = domain.get_manager_00415350();
            manager->register_object(global);
        }
    }
    return global;
}
} // namespace bsp
