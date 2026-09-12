#include "bsp/dyn_task_manager.hpp"
#include <windows.h>
#include <process.h>
#include <cstddef>
#include <new>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4, "Native Dyn task storage requires Win32");
static_assert(sizeof(CRITICAL_SECTION) == 0x18, "Native task CS at18h ends at30h");
static_assert(sizeof(DynTaskManagerStorage) == 0x358);

template<class T> T& at(void* object, std::size_t offset) {
    return *reinterpret_cast<T*>(static_cast<unsigned char*>(object) + offset);
}
void* allocate(const AvoidZoneDynHullMemory& memory, std::size_t size) {
    void* value = memory.allocate(memory.context, size);
    if (value == nullptr) throw std::bad_alloc();
    return value;
}

//00C375F0, ESI manager, plainRET. Every nonzero wait result takes the dequeue
//branch, as in native code; failed waits are outside the successful-OS domain.
void worker_loop_00c375f0(DynTaskManagerStorage& storage) {
    void* manager = &storage;
    for (;;) {
        HANDLE handles[2] = {at<HANDLE>(manager, 0x354), at<HANDLE>(manager, 0x350)};
        if (WaitForMultipleObjects(2, handles, FALSE, INFINITE) == WAIT_OBJECT_0) return;
        EnterCriticalSection(&at<CRITICAL_SECTION>(manager, 0x18));
        const auto count = at<std::uint32_t>(manager, 0x10);
        void* task = at<void**>(manager, 0xc)[count - 1];
        at<std::uint32_t>(manager, 0x10) = count - 1;
        LeaveCriticalSection(&at<CRITICAL_SECTION>(manager, 0x18));
        using Execute = void (__thiscall*)(void*);
        auto* vtable = at<Execute*>(task, 0);
        vtable[0](task);
        const auto group = at<std::uint32_t>(task, 4);
        if (InterlockedDecrement(&at<volatile LONG>(manager, 0x30 + group * 4)) == 0) {
            // Native reloads task+4 after the decrement, before fetching event.
            const auto finished_group = at<std::uint32_t>(task, 4);
            SetEvent(at<HANDLE>(manager, 0x1c0 + finished_group * 4));
        }
    }
}

//00C37680: actual _beginthreadex signature, one stack argument, RET4; EAX0.
unsigned __stdcall worker_entry_00c37680(void* manager) {
    worker_loop_00c375f0(*static_cast<DynTaskManagerStorage*>(manager));
    return 0;
}
} // namespace

DynTaskManagerStorage* dyn_task_manager_construct_00c37740(DynTaskManagerStorage& storage,
    const std::uint32_t& worker_count, const AvoidZoneDynHullMemory& memory) {
    const auto requested_count = worker_count;
    void* manager = &storage;
    at<void*>(manager, 0xc) = nullptr;
    at<std::uint32_t>(manager, 0x10) = 0;
    at<std::uint32_t>(manager, 0x14) = 0;
    InitializeCriticalSectionAndSpinCount(&at<CRITICAL_SECTION>(manager, 0x18), 10000);
    at<HANDLE>(manager, 0x350) = CreateSemaphoreA(nullptr, 0, 1000, nullptr);
    at<HANDLE>(manager, 0x354) = CreateEventA(nullptr, TRUE, FALSE, nullptr);
    for (std::uint32_t i = 0; i < 100; ++i) {
        at<LONG>(manager, 0x30 + i * 4) = 0;
        at<HANDLE>(manager, 0x1c0 + i * 4) = CreateEventA(nullptr, FALSE, FALSE, nullptr);
    }
    at<std::uint32_t>(manager, 4) = 0;
    dyn_task_manager_set_worker_count_00c37690(storage, requested_count, memory);
    return &storage;
}

void dyn_task_manager_set_worker_count_00c37690(DynTaskManagerStorage& storage,
    const std::uint32_t& worker_count, const AvoidZoneDynHullMemory& memory) {
    const auto requested_count = worker_count;
    void* manager = &storage;
    if (at<std::uint32_t>(manager, 4) != 0) {
        SetEvent(at<HANDLE>(manager, 0x354));
        WaitForMultipleObjects(at<std::uint32_t>(manager, 4),
            at<HANDLE*>(manager, 0), TRUE, INFINITE);
        ResetEvent(at<HANDLE>(manager, 0x354));
        memory.release(memory.context, at<void*>(manager, 0));
        at<void*>(manager, 0) = nullptr; //00C376D1, recovered after false-free gap
    }
    at<std::uint32_t>(manager, 4) = requested_count;
    if (requested_count == 0) return;
    const std::uint64_t bytes = static_cast<std::uint64_t>(requested_count) * 4;
    const auto allocation_size = bytes > 0xffffffffu ? 0xffffffffu : static_cast<std::uint32_t>(bytes);
    at<HANDLE*>(manager, 0) = static_cast<HANDLE*>(allocate(memory, allocation_size));
    for (std::uint32_t i = 0; i < requested_count; ++i) {
        const auto thread = _beginthreadex(nullptr, 0, &worker_entry_00c37680, manager,
            0, &at<unsigned>(manager, 8));
        at<HANDLE*>(manager, 0)[i] = reinterpret_cast<HANDLE>(thread);
        SetThreadPriority(at<HANDLE*>(manager, 0)[i], THREAD_PRIORITY_HIGHEST);
    }
}

void dyn_task_manager_run_batch_00c33140(DynTaskManagerStorage& storage, void* const* tasks,
    std::int32_t count, const AvoidZoneDynHullMemory& memory) {
    void* manager = &storage;
    EnterCriticalSection(&at<CRITICAL_SECTION>(manager, 0x18));
    const auto old_count = at<std::uint32_t>(manager, 0x10);
    const auto new_count = old_count + static_cast<std::uint32_t>(count);
    if (old_count < new_count) {
        if (at<std::uint32_t>(manager, 0x14) < new_count) {
            at<std::uint32_t>(manager, 0x14) = new_count;
            auto** replacement = static_cast<void**>(allocate(memory, new_count * 4u));
            for (std::uint32_t i = 0; i < at<std::uint32_t>(manager, 0x10); ++i)
                replacement[i] = at<void**>(manager, 0xc)[i];
            if (at<void*>(manager, 0xc) != nullptr)
                memory.release(memory.context, at<void*>(manager, 0xc));
            at<void**>(manager, 0xc) = replacement;
        }
        for (auto i = at<std::uint32_t>(manager, 0x10); i < new_count; ++i)
            at<void**>(manager, 0xc)[i] = nullptr;
    }
    at<std::uint32_t>(manager, 0x10) = new_count;
    auto** output = at<void**>(manager, 0xc) + old_count;
    std::uint32_t group = 0;
    while (at<volatile LONG>(manager, 0x30 + group * 4) != 0) ++group;
    for (std::int32_t i = 0; i < count; ++i) {
        at<std::uint32_t>(tasks[i], 4) = group;
        output[i] = tasks[i];
    }
    ReleaseSemaphore(at<HANDLE>(manager, 0x350), count, nullptr);
    at<volatile LONG>(manager, 0x30 + group * 4) = count;
    LeaveCriticalSection(&at<CRITICAL_SECTION>(manager, 0x18));
    WaitForSingleObject(at<HANDLE>(manager, 0x1c0 + group * 4), INFINITE);
}

void dyn_task_manager_destroy_00c40ff0(DynTaskManagerStorage& storage,
    const AvoidZoneDynHullMemory& memory) {
    dyn_task_manager_set_worker_count_00c37690(storage, 0, memory);
    void* manager = &storage;
    CloseHandle(at<HANDLE>(manager, 0x354));
    CloseHandle(at<HANDLE>(manager, 0x350));
    memory.release(memory.context, at<void*>(manager, 0));
    DeleteCriticalSection(&at<CRITICAL_SECTION>(manager, 0x18));
    if (at<void*>(manager, 0xc) != nullptr)
        memory.release(memory.context, at<void*>(manager, 0xc));
}
} // namespace bsp
