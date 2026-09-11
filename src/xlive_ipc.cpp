#include "bsp/xlive_ipc.hpp"

#include <cstring>
#include <exception>
#include <new>
#include <process.h>
#include <stdexcept>

namespace bsp {
static_assert(sizeof(XLiveIpcNativeState) == 0x2c);
static_assert(offsetof(XLiveIpcNativeState, received_bytes_14) == 0x14);
static_assert(offsetof(XLiveIpcNativeState, counter_28) == 0x28);
namespace {
constexpr auto unexpected = static_cast<std::int32_t>(0x8000ffffu);
constexpr auto abort_result = static_cast<std::int32_t>(0x80004004u);
constexpr auto async_pending = static_cast<std::int32_t>(0x8000000au);

std::int32_t hresult_from_error(std::uint32_t error) noexcept {
    const auto signed_error = static_cast<std::int32_t>(error);
    return signed_error > 0
        ? static_cast<std::int32_t>((error & 0xffffu) | 0x80070000u) : signed_error;
}

[[noreturn]] void fail_creation(XLiveIpc& ipc) {
    auto& n = ipc.native;
    auto& system = *ipc.system;
    if (n.pipe_00) {
        ipc.pipes->close_00a5de68(n.pipe_00);
        n.pipe_00 = nullptr;
    }
    if (n.stop_event_08) {
        system.close_handle(n.stop_event_08);
        n.stop_event_08 = nullptr;
    }
    if (n.receive_buffer_0c) {
        system.free_bytes(n.receive_buffer_0c);
        n.receive_buffer_0c = nullptr;
    }
    if (n.send_buffer_18) {
        system.free_bytes(n.send_buffer_18);
        n.send_buffer_18 = nullptr;
    }
    system.free_object(&ipc);
    system.exit_process(0); // A4C20C..A4C211, hidden after false no-return free
    std::terminate();
}

void require_frame_buffer(const std::uint8_t* buffer, std::uint32_t capacity) {
    if (!buffer || capacity < 4)
        throw std::logic_error("native XLive IPC frame requires a retained four-byte header");
}
}

XLiveIpc* Win32XLiveIpcSystemHost::allocate_object() { return new (std::nothrow) XLiveIpc; }
std::uint8_t* Win32XLiveIpcSystemHost::allocate_bytes(std::uint32_t bytes) {
    return new (std::nothrow) std::uint8_t[bytes];
}
void Win32XLiveIpcSystemHost::free_object(XLiveIpc* value) noexcept { delete value; }
void Win32XLiveIpcSystemHost::free_bytes(std::uint8_t* value) noexcept { delete[] value; }
void Win32XLiveIpcSystemHost::set_last_error(std::uint32_t error) { SetLastError(error); }
std::uint32_t Win32XLiveIpcSystemHost::get_last_error() { return GetLastError(); }
HANDLE Win32XLiveIpcSystemHost::create_stop_event() { return CreateEventA(nullptr, TRUE, FALSE, nullptr); }
HANDLE Win32XLiveIpcSystemHost::create_thread(LPTHREAD_START_ROUTINE entry, void* argument) {
    return CreateThread(nullptr, 0, entry, argument, 0, nullptr);
}
void Win32XLiveIpcSystemHost::set_event(HANDLE event) { static_cast<void>(SetEvent(event)); }
std::uint32_t Win32XLiveIpcSystemHost::wait_for_single_object(HANDLE handle, std::uint32_t timeout) {
    return WaitForSingleObject(handle, timeout);
}
void Win32XLiveIpcSystemHost::close_handle(HANDLE handle) { static_cast<void>(CloseHandle(handle)); }
void Win32XLiveIpcSystemHost::sleep(std::uint32_t milliseconds) { Sleep(milliseconds); }
void Win32XLiveIpcSystemHost::get_system_time(SYSTEMTIME& time) { GetSystemTime(&time); }
HANDLE Win32XLiveIpcSystemHost::get_current_process() { return GetCurrentProcess(); }
void Win32XLiveIpcSystemHost::terminate_process(HANDLE process, std::uint32_t code) {
    static_cast<void>(TerminateProcess(process, code));
}
[[noreturn]] void Win32XLiveIpcSystemHost::exit_process(int code) { _exit(code); }

std::uint32_t xlive_ipc_last_error_00a4bc80(XLiveIpcSystemHost& system) {
    const auto error = system.get_last_error();
    return error ? error : 0x507u;
}

std::int32_t create_xlive_ipc_00a4c030(XLiveIpc** output,
    XLiveIpcPipeHost& pipes, XLiveIpcSystemHost& system) {
    if (!output) return static_cast<std::int32_t>(0x80070057u);
    auto* ipc = system.allocate_object();
    if (!ipc) {
        *output = nullptr;
        return static_cast<std::int32_t>(0x8007000eu);
    }
    ipc->pipes = &pipes;
    ipc->system = &system;
    auto& n = ipc->native;
    n.pipe_00 = nullptr;
    n.thread_04 = nullptr;
    n.stop_event_08 = nullptr;
    n.receive_buffer_0c = nullptr;
    n.receive_capacity_10 = 0;
    n.send_buffer_18 = nullptr;
    n.send_capacity_1c = 0;
    n.phase_24 = 0;
    n.counter_28 = 1;
    // +14/+20 remain the actual raw allocation preimage until worker writes.
    system.set_last_error(0);
    n.stop_event_08 = system.create_stop_event();
    if (!n.stop_event_08) {
        static_cast<void>(hresult_from_error(xlive_ipc_last_error_00a4bc80(system)));
        fail_creation(*ipc);
    }
    if (pipes.open_00a5de34(0, n.stop_event_08, n.pipe_00) < 0) fail_creation(*ipc);
    n.phase_24 = 1;
    std::uint32_t bytes;
    if (pipes.send_capacity_00a5df96(n.pipe_00, 8, bytes) < 0) fail_creation(*ipc);
    n.send_capacity_1c = bytes + 4u; // native DWORD wrap, then unsigned bound
    if (n.send_capacity_1c > 0x400) {
        n.send_capacity_1c = 0;
        fail_creation(*ipc);
    }
    n.send_buffer_18 = system.allocate_bytes(n.send_capacity_1c);
    if (!n.send_buffer_18) {
        n.send_capacity_1c = 0;
        fail_creation(*ipc);
    }
    const auto result = pipes.receive_capacity_00a5dfce(n.pipe_00, 8, bytes);
    if (result < 0) fail_creation(*ipc);
    n.receive_capacity_10 = bytes + 4u;
    if (n.receive_capacity_10 > 0x400) {
        n.receive_capacity_10 = 0;
        fail_creation(*ipc);
    }
    n.receive_buffer_0c = system.allocate_bytes(n.receive_capacity_10);
    if (!n.receive_buffer_0c) {
        n.receive_capacity_10 = 0;
        fail_creation(*ipc);
    }
    system.set_last_error(0);
    n.thread_04 = system.create_thread(run_xlive_ipc_thread_00a4c000, ipc);
    if (!n.thread_04) {
        static_cast<void>(hresult_from_error(xlive_ipc_last_error_00a4bc80(system)));
        fail_creation(*ipc);
    }
    *output = ipc;
    return result; // EDI holds the successful receive-capacity result
}

void destroy_xlive_ipc_00a4bde0(XLiveIpc* ipc) {
    if (!ipc) return;
    auto& n = ipc->native;
    auto& system = *ipc->system;
    system.set_event(n.stop_event_08);
    static_cast<void>(system.wait_for_single_object(n.thread_04, 1000));
    system.close_handle(n.thread_04);
    const auto pipe = n.pipe_00; // native loads before nulling thread +4
    n.thread_04 = nullptr;
    ipc->pipes->close_00a5de68(pipe);
    const auto event = n.stop_event_08;
    n.pipe_00 = nullptr;
    system.close_handle(event);
    auto* receive = n.receive_buffer_0c;
    n.stop_event_08 = nullptr;
    system.free_bytes(receive);
    auto* send = n.send_buffer_18;
    n.receive_buffer_0c = nullptr;
    system.free_bytes(send);
    n.send_buffer_18 = nullptr;
    system.free_object(ipc);
}

std::int32_t __stdcall encode_xlive_ipc_probe_00a4bd40(void* buffer,
    std::uint32_t* available, void* context) {
    if (*available < 8) return static_cast<std::int32_t>(0x8007007au);
    auto& n = static_cast<XLiveIpc*>(context)->native;
    const std::uint32_t marker = 8;
    std::memcpy(buffer, &marker, 4);
    const auto value = n.counter_28 + 0x27u;
    std::memcpy(static_cast<std::uint8_t*>(buffer) + 4, &value, 4);
    ++n.counter_28;
    n.phase_24 = 3;
    return 0; // native does not change *available
}

std::int32_t __stdcall decode_xlive_ipc_probe_00a4bd80(const void* buffer,
    std::uint32_t bytes, void* context) {
    if (bytes != 8) return unexpected;
    std::uint32_t marker;
    std::memcpy(&marker, buffer, 4);
    if (marker != 8) return unexpected;
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(buffer) + 4, 4);
    auto& n = static_cast<XLiveIpc*>(context)->native;
    if (~value != n.counter_28) return unexpected;
    n.phase_24 = 1;
    return 0;
}

std::int32_t run_xlive_ipc_loop_00a4be50(XLiveIpc& ipc) {
    auto& n = ipc.native;
    std::int32_t result = 0; // EDI is reset only at entry and after pending I/O
    for (;;) {
        switch (n.phase_24) {
        case 1: {
            ipc.system->sleep(0x3a31);
            SYSTEMTIME discarded;
            ipc.system->get_system_time(discarded);
            n.phase_24 = 2;
            break;
        }
        case 2: {
            auto* const buffer = n.send_buffer_18;
            auto bytes = n.send_capacity_1c - 4u;
            n.phase_24 = 0;
            require_frame_buffer(buffer, n.send_capacity_1c);
            result = ipc.pipes->encode_00a5e055(n.pipe_00, buffer + 4, bytes,
                encode_xlive_ipc_probe_00a4bd40, &ipc);
            if (result < 0) n.phase_24 = 0;
            else {
                n.send_bytes_20 = bytes + 4u;
                std::memcpy(buffer, &bytes, 4);
            }
            break;
        }
        case 3:
            result = ipc.pipes->send_00a5df20(n.pipe_00, n.send_buffer_18, n.send_bytes_20);
            if (result == async_pending) {
                n.phase_24 = 4;
                result = 0;
                continue;
            }
            n.phase_24 = 0;
            break;
        case 4: {
            std::uint32_t sent;
            result = ipc.pipes->wait_send_00a5df5e(n.pipe_00, sent, 5000);
            if (result >= 0 && sent != n.send_bytes_20) result = unexpected;
            n.phase_24 = result < 0 ? 0u : 5u;
            break;
        }
        case 5:
            result = ipc.pipes->receive_00a5deaa(n.pipe_00, n.receive_buffer_0c, n.receive_capacity_10);
            if (result == async_pending) {
                n.phase_24 = 6;
                result = 0;
                continue;
            }
            n.phase_24 = 0;
            break;
        case 6:
            result = ipc.pipes->wait_receive_00a5dee8(n.pipe_00, n.received_bytes_14, 5000);
            n.phase_24 = result < 0 ? 0u : 7u;
            break;
        case 7: {
            const auto received = n.received_bytes_14;
            n.phase_24 = 0;
            if (received < 4) return unexpected;
            require_frame_buffer(n.receive_buffer_0c, n.receive_capacity_10);
            std::uint32_t bytes;
            std::memcpy(&bytes, n.receive_buffer_0c, 4);
            if (bytes != received - 4u) return unexpected;
            result = ipc.pipes->decode_00a5e09e(n.pipe_00, n.receive_buffer_0c + 4, bytes,
                decode_xlive_ipc_probe_00a4bd80, &ipc);
            if (result >= 0) continue; // preserve callback's live phase
            n.phase_24 = 0;
            break;
        }
        default:
            return unexpected;
        }
        if (result < 0) return result;
    }
}

DWORD WINAPI run_xlive_ipc_thread_00a4c000(void* parameter) {
    auto& ipc = *static_cast<XLiveIpc*>(parameter);
    auto& system = *ipc.system;
    const auto result = run_xlive_ipc_loop_00a4be50(ipc);
    if (result != abort_result) {
        const auto process = system.get_current_process();
        system.terminate_process(process, 0x8000ffffu);
    }
    return static_cast<DWORD>(result);
}
} // namespace bsp
