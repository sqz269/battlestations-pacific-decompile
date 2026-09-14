#include "bsp/native_online_ipc.hpp"
#include "bsp/xlive_pipe_services.hpp"

#include <atomic>
#include <cstdlib>
#include <cstring>
#include <new>
#include <stdexcept>

namespace bsp {
static_assert(sizeof(NativeOnlineIpcStorage) == 0x2c);
static_assert(offsetof(NativeOnlineIpcStorage, received_bytes_14) == 0x14);
static_assert(offsetof(NativeOnlineIpcStorage, send_bytes_20) == 0x20);
static_assert(sizeof(void*) == 4);
namespace {
constexpr auto unexpected = static_cast<std::int32_t>(0x8000ffffu);
constexpr auto aborted = static_cast<std::int32_t>(0x80004004u);
constexpr auto pending = static_cast<std::int32_t>(0x8000000au);
std::atomic<const NativeOnlineIpcRuntime*> worker_runtime{};

void* __cdecl allocate_nothrow(std::size_t bytes) {
    return ::operator new(bytes, std::nothrow);
}
std::int32_t hresult(std::uint32_t error) {
    return static_cast<std::int32_t>(error) > 0
        ? static_cast<std::int32_t>((error & 0xffffu) | 0x80070000u)
        : static_cast<std::int32_t>(error);
}
[[noreturn]] void fail_creation(NativeOnlineIpcStorage& n,
    const NativeOnlineIpcRuntime& r) {
    if (n.pipe_00) { r.pipes.close_00a5de68(n.pipe_00); n.pipe_00 = nullptr; }
    if (n.stop_event_08) { r.system.close_handle(n.stop_event_08); n.stop_event_08 = nullptr; }
    if (n.receive_buffer_0c) { r.memory.release(n.receive_buffer_0c); n.receive_buffer_0c = nullptr; }
    if (n.send_buffer_18) { r.memory.release(n.send_buffer_18); n.send_buffer_18 = nullptr; }
    r.memory.release(&n);
    r.system.exit_process(0); // A4C20D, hidden by the old false-noreturn free.
    std::terminate();
}
}

NativeOnlineIpcMemory standard_native_online_ipc_memory() noexcept {
    // This MSVC CRT's operator new is malloc-backed; native BFD017/BFD012 use
    // nothrow new and BF6989/BF65AC release with the matching CRT free.
    return {allocate_nothrow, std::free};
}
NativeOnlineIpcRuntime make_win32_native_online_ipc_runtime(
    ReconstructedXLivePipeServices& pipes, std::uint32_t capacity_preimage,
    std::uint32_t sent_preimage) {
    static Win32XLiveIpcSystemHost system;
    return {pipes, system, standard_native_online_ipc_memory(),
        capacity_preimage, sent_preimage};
}
void bind_native_online_ipc_worker_runtime(const NativeOnlineIpcRuntime& runtime) {
    const NativeOnlineIpcRuntime* expected = nullptr;
    if (!worker_runtime.compare_exchange_strong(expected, &runtime) && expected != &runtime)
        throw std::logic_error("Native IPC worker runtime is already bound");
}

std::uint32_t native_online_ipc_last_error_00a4bc80(XLiveIpcSystemHost& system) {
    const auto value = system.get_last_error();
    return value ? value : 0x507u;
}
std::int32_t create_native_online_ipc_00a4c030(NativeOnlineIpcStorage** output,
    const NativeOnlineIpcRuntime& r) {
    if (!output) return static_cast<std::int32_t>(0x80070057u);
    auto* const allocation = r.memory.allocate_nothrow(0x2c);
    if (!allocation) { *output = nullptr; return static_cast<std::int32_t>(0x8007000eu); }
    auto& n = *::new (allocation) NativeOnlineIpcStorage; // no value-initialization
    n.pipe_00 = nullptr;
    n.thread_04 = nullptr;
    n.stop_event_08 = nullptr;
    n.receive_buffer_0c = nullptr;
    n.receive_capacity_10 = 0;
    n.send_buffer_18 = nullptr;
    n.send_capacity_1c = 0;
    n.phase_24 = 0;
    n.counter_28 = 1;
    r.system.set_last_error(0);
    n.stop_event_08 = r.system.create_stop_event();
    if (!n.stop_event_08) {
        static_cast<void>(hresult(native_online_ipc_last_error_00a4bc80(r.system)));
        fail_creation(n, r);
    }
    if (r.pipes.open_00a5de34(0, n.stop_event_08, n.pipe_00) < 0) fail_creation(n, r);
    n.phase_24 = 1;
    auto bytes = r.capacity_preimage;
    if (r.pipes.send_capacity_00a5df96(n.pipe_00, 8, bytes) < 0) fail_creation(n, r);
    n.send_capacity_1c = bytes + 4u;
    if (n.send_capacity_1c > 0x400u) { n.send_capacity_1c = 0; fail_creation(n, r); }
    n.send_buffer_18 = static_cast<std::uint8_t*>(r.memory.allocate_nothrow(n.send_capacity_1c));
    if (!n.send_buffer_18) { n.send_capacity_1c = 0; fail_creation(n, r); }
    auto result = r.pipes.receive_capacity_00a5dfce(n.pipe_00, 8, bytes);
    if (result < 0) fail_creation(n, r);
    n.receive_capacity_10 = bytes + 4u;
    if (n.receive_capacity_10 > 0x400u) { n.receive_capacity_10 = 0; fail_creation(n, r); }
    n.receive_buffer_0c = static_cast<std::uint8_t*>(r.memory.allocate_nothrow(n.receive_capacity_10));
    if (!n.receive_buffer_0c) { n.receive_capacity_10 = 0; fail_creation(n, r); }
    r.system.set_last_error(0);
    n.thread_04 = r.system.create_thread(run_native_online_ipc_thread_00a4c000, &n);
    if (!n.thread_04) {
        const auto error = native_online_ipc_last_error_00a4bc80(r.system);
        if (error) result = hresult(error);
    }
    if (result < 0) fail_creation(n, r);
    *output = &n;
    return result;
}
std::int32_t initialize_native_online_ipc_slot_00a4c250(std::uint32_t* slot,
    const NativeOnlineIpcRuntime& r) {
    if (!slot) return static_cast<std::int32_t>(0x80070057u);
    NativeOnlineIpcStorage* local;
    const auto result = create_native_online_ipc_00a4c030(&local, r);
    *slot = result < 0 ? 0u : reinterpret_cast<std::uint32_t>(local);
    return result;
}
void destroy_native_online_ipc_00a4bde0(NativeOnlineIpcStorage* ipc,
    const NativeOnlineIpcRuntime& r) {
    if (!ipc) return;
    auto& n = *ipc;
    r.system.set_event(n.stop_event_08);
    static_cast<void>(r.system.wait_for_single_object(n.thread_04, 1000));
    r.system.close_handle(n.thread_04);
    const auto pipe = n.pipe_00;
    n.thread_04 = nullptr;
    r.pipes.close_00a5de68(pipe);
    const auto event = n.stop_event_08;
    n.pipe_00 = nullptr;
    r.system.close_handle(event);
    auto* receive = n.receive_buffer_0c;
    n.stop_event_08 = nullptr;
    r.memory.release(receive);
    auto* send = n.send_buffer_18;
    n.receive_buffer_0c = nullptr;
    r.memory.release(send);
    n.send_buffer_18 = nullptr;
    r.memory.release(ipc);
}
void close_native_online_ipc_handle_00a4c280(std::uint32_t handle,
    const NativeOnlineIpcRuntime& r) {
    if (handle && handle != 0xffffffffu)
        destroy_native_online_ipc_00a4bde0(reinterpret_cast<NativeOnlineIpcStorage*>(handle), r);
}
std::int32_t __stdcall encode_native_online_ipc_00a4bd40(void* buffer,
    std::uint32_t* available, void* context) {
    if (*available < 8) return static_cast<std::int32_t>(0x8007007au);
    auto& n = *static_cast<NativeOnlineIpcStorage*>(context);
    const std::uint32_t marker = 8;
    std::memcpy(buffer, &marker, 4);
    const auto value = n.counter_28 + 0x27u;
    std::memcpy(static_cast<std::uint8_t*>(buffer) + 4, &value, 4);
    ++n.counter_28;
    n.phase_24 = 3;
    return 0;
}
std::int32_t __stdcall decode_native_online_ipc_00a4bd80(const void* buffer,
    std::uint32_t bytes, void* context) {
    if (bytes != 8) return unexpected;
    std::uint32_t marker;
    std::memcpy(&marker, buffer, 4);
    if (marker != 8) return unexpected;
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(buffer) + 4, 4);
    auto& n = *static_cast<NativeOnlineIpcStorage*>(context);
    if (~value != n.counter_28) return unexpected;
    n.phase_24 = 1;
    return 0;
}
std::int32_t run_native_online_ipc_loop_00a4be50(NativeOnlineIpcStorage& n,
    const NativeOnlineIpcRuntime& r) {
    std::int32_t result = 0;
    auto sent = r.sent_preimage; // one retained local DWORD, not reset each phase4
    for (;;) {
        switch (n.phase_24) {
        case 1: {
            r.system.sleep(0x3a31);
            SYSTEMTIME discarded;
            r.system.get_system_time(discarded);
            n.phase_24 = 2;
            break;
        }
        case 2: {
            auto bytes = n.send_capacity_1c - 4u;
            auto* const buffer = n.send_buffer_18;
            n.phase_24 = 0;
            result = r.pipes.encode_00a5e055(n.pipe_00,
                reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(buffer) + 4u),
                bytes, encode_native_online_ipc_00a4bd40, &n);
            if (result < 0) n.phase_24 = 0;
            else { n.send_bytes_20 = bytes + 4u; std::memcpy(buffer, &bytes, 4); }
            break;
        }
        case 3:
            result = r.pipes.send_00a5df20(n.pipe_00, n.send_buffer_18, n.send_bytes_20);
            if (result == pending) { n.phase_24 = 4; result = 0; continue; }
            n.phase_24 = 0;
            break;
        case 4:
            result = r.pipes.wait_send_00a5df5e(n.pipe_00, sent, 5000);
            if (result >= 0 && sent != n.send_bytes_20) result = unexpected;
            n.phase_24 = result < 0 ? 0u : 5u;
            break;
        case 5:
            result = r.pipes.receive_00a5deaa(n.pipe_00, n.receive_buffer_0c, n.receive_capacity_10);
            if (result == pending) { n.phase_24 = 6; result = 0; continue; }
            n.phase_24 = 0;
            break;
        case 6:
            result = r.pipes.wait_receive_00a5dee8(n.pipe_00, n.received_bytes_14, 5000);
            n.phase_24 = result < 0 ? 0u : 7u;
            break;
        case 7: {
            const auto received = n.received_bytes_14;
            n.phase_24 = 0;
            if (received < 4) return unexpected;
            auto* const buffer = n.receive_buffer_0c;
            std::uint32_t bytes;
            std::memcpy(&bytes, buffer, 4);
            if (bytes != received - 4u) return unexpected;
            result = r.pipes.decode_00a5e09e(n.pipe_00, buffer + 4, bytes,
                decode_native_online_ipc_00a4bd80, &n);
            if (result >= 0) continue;
            n.phase_24 = 0;
            break;
        }
        default: return unexpected;
        }
        if (result < 0) return result;
    }
}
DWORD WINAPI run_native_online_ipc_thread_00a4c000(void* parameter) {
    const auto* const runtime = worker_runtime.load();
    if (!runtime) std::terminate(); // outside the documented bound-runtime domain
    const auto result = run_native_online_ipc_loop_00a4be50(
        *static_cast<NativeOnlineIpcStorage*>(parameter), *runtime);
    if (result != aborted)
        runtime->system.terminate_process(runtime->system.get_current_process(), 0x8000ffffu);
    return static_cast<DWORD>(result);
}
} // namespace bsp
