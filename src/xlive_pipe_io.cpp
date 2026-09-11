#include "bsp/xlive_pipe_io.hpp"

namespace bsp {
namespace {
constexpr std::int32_t result_bits(std::uint32_t value) noexcept {
    return value < 0x80000000u ? static_cast<std::int32_t>(value) :
        static_cast<std::int32_t>(static_cast<std::int64_t>(value) - 0x100000000ll);
}
constexpr auto pending_result = result_bits(0x8000000au);
constexpr auto unexpected_result = result_bits(0x8000ffffu);
constexpr auto abort_result = result_bits(0x80004004u);
constexpr auto fail_result = result_bits(0x80004005u);
constexpr auto invalid_owner_result = result_bits(0x80070006u);
constexpr auto invalid_argument_result = result_bits(0x80070057u);

std::int32_t error_to_result(DWORD error) noexcept {
    if (error == 0) error = 0x507u;
    if (result_bits(error) > 0) return result_bits((error & 0xffffu) | 0x80070000u);
    return result_bits(error);
}

bool invalid_owner(const XLivePipeTransport* owner) noexcept {
    return owner == nullptr || reinterpret_cast<std::uintptr_t>(owner) == 0xffffffffu;
}

void clear_pipe_offsets(XLivePipeNativeState& state) noexcept {
    state.overlapped_0c.OffsetHigh = 0;
    state.overlapped_0c.Offset = 0;
}

bool stop_is_signaled(XLivePipeNativeState& state, XLivePipeIoHost& host) {
    return state.wait_handle_count_2c == 2u &&
        host.wait_for_single_object(state.stop_event_28, 0) == WAIT_OBJECT_0;
}

std::int32_t submitted_io_result(XLivePipeNativeState& state, bool success,
    std::uint32_t next_state, XLivePipeIoHost& host) {
    if (!success) {
        const auto error = host.get_last_error();
        if (error != ERROR_IO_PENDING) return error_to_result(error);
    }
    state.io_state_20 = next_state;
    return pending_result;
}

static_assert(sizeof(void*) == 4);
static_assert(sizeof(XLivePipeNativeState) == 0x30);
static_assert(offsetof(XLivePipeTransport, native) == 0);
static_assert(offsetof(XLivePipeNativeState, overlapped_0c) == 0xc);
static_assert(offsetof(OVERLAPPED, Offset) == 8);
static_assert(offsetof(OVERLAPPED, OffsetHigh) == 0xc);
static_assert(offsetof(OVERLAPPED, hEvent) == 0x10);
static_assert(offsetof(XLivePipeNativeState, completion_event_24) == 0x24);
static_assert(offsetof(XLivePipeNativeState, stop_event_28) == 0x28);
static_assert(offsetof(XLivePipeNativeState, wait_handle_count_2c) == 0x2c);
} // namespace

void Win32XLivePipeIoHost::set_last_error(DWORD value) { SetLastError(value); }
DWORD Win32XLivePipeIoHost::get_last_error() { return GetLastError(); }
DWORD Win32XLivePipeIoHost::wait_for_single_object(HANDLE handle, DWORD timeout) {
    return WaitForSingleObject(handle, timeout);
}
DWORD Win32XLivePipeIoHost::wait_for_multiple_objects(DWORD count,
    const HANDLE* handles, bool wait_all, DWORD timeout) {
    return WaitForMultipleObjects(count, handles, wait_all ? TRUE : FALSE, timeout);
}
bool Win32XLivePipeIoHost::read_file(HANDLE handle, void* buffer, DWORD bytes,
    DWORD* transferred, OVERLAPPED& overlapped) {
    return ReadFile(handle, buffer, bytes, transferred, &overlapped) != FALSE;
}
bool Win32XLivePipeIoHost::write_file(HANDLE handle, const void* buffer, DWORD bytes,
    DWORD* transferred, OVERLAPPED& overlapped) {
    return WriteFile(handle, buffer, bytes, transferred, &overlapped) != FALSE;
}
bool Win32XLivePipeIoHost::get_overlapped_result(HANDLE handle, OVERLAPPED& overlapped,
    DWORD* transferred, bool wait) {
    return GetOverlappedResult(handle, &overlapped, transferred, wait ? TRUE : FALSE) != FALSE;
}
bool Win32XLivePipeIoHost::cancel_io(HANDLE handle) { return CancelIo(handle) != FALSE; }
bool Win32XLivePipeIoHost::connect_named_pipe(HANDLE handle, OVERLAPPED& overlapped) {
    return ConnectNamedPipe(handle, &overlapped) != FALSE;
}
bool Win32XLivePipeIoHost::set_event(HANDLE handle) { return SetEvent(handle) != FALSE; }

std::int32_t connect_xlive_pipe_handle_00a5e16e(HANDLE pipe, OVERLAPPED& overlapped,
    DWORD& pending, XLivePipeIoHost& host) {
    pending = 0;
    host.set_last_error(0);
    if (host.connect_named_pipe(pipe, overlapped)) return 0;
    const auto error = host.get_last_error();
    if (error == 0) return 0x507;
    if (error == ERROR_IO_PENDING) {
        pending = 1;
        return 0;
    }
    if (error != ERROR_PIPE_CONNECTED) return result_bits(error);
    host.set_last_error(0);
    if (!host.set_event(overlapped.hEvent)) return error_to_result(host.get_last_error());
    return 0;
}

std::int32_t wait_xlive_pipe_connection_00a5e750(XLivePipeNativeState& state,
    DWORD timeout, XLivePipeIoHost& host) {
    DWORD pending = 0;
    if (state.mode_04 == 0) return unexpected_result;
    const auto initial_state = state.io_state_20;
    if (initial_state == 3u) {
        // Native captures pipe/OVERLAPPED arguments before clearing offsets.
        const auto pipe = state.pipe_08;
        clear_pipe_offsets(state);
        const auto result = connect_xlive_pipe_handle_00a5e16e(
            pipe, state.overlapped_0c, pending, host);
        if (result < 0 || pending == 0) return result;
    } else if (initial_state != 0u) {
        return unexpected_result;
    }
    const auto waited = host.wait_for_multiple_objects(state.wait_handle_count_2c,
        &state.completion_event_24, false, timeout);
    if (waited == WAIT_TIMEOUT) {
        state.io_state_20 = 0;
        return result_bits(0x800705b4u);
    }
    if (waited >= state.wait_handle_count_2c) return unexpected_result;
    if (waited == WAIT_OBJECT_0) return 0;
    if (waited == WAIT_OBJECT_0 + 1u) {
        static_cast<void>(host.cancel_io(state.pipe_08));
        return abort_result;
    }
    return unexpected_result;
}

std::int32_t submit_xlive_pipe_read_00a5e1f7(XLivePipeNativeState& state,
    void* buffer, DWORD bytes, XLivePipeIoHost& host) {
    // PUSH ECX at entry creates this native local output-slot preimage.
    DWORD transferred = static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(&state));
    if (state.io_state_20 != 3u) return unexpected_result;
    if (stop_is_signaled(state, host)) return abort_result;
    clear_pipe_offsets(state);
    host.set_last_error(0);
    const auto success = host.read_file(state.pipe_08, buffer, bytes,
        &transferred, state.overlapped_0c);
    return submitted_io_result(state, success, 1u, host);
}

std::int32_t submit_xlive_pipe_write_00a5e28b(XLivePipeNativeState& state,
    const void* buffer, DWORD bytes, XLivePipeIoHost& host) {
    DWORD transferred = static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(&state));
    if (state.io_state_20 != 3u) return unexpected_result;
    if (stop_is_signaled(state, host)) return abort_result;
    clear_pipe_offsets(state);
    host.set_last_error(0);
    const auto success = host.write_file(state.pipe_08, buffer, bytes,
        &transferred, state.overlapped_0c);
    return submitted_io_result(state, success, 2u, host);
}

std::int32_t wait_xlive_pipe_io_00a5e31f(XLivePipeNativeState& state,
    DWORD* transferred, DWORD timeout, XLivePipeIoHost& host) {
    const auto waited = host.wait_for_multiple_objects(state.wait_handle_count_2c,
        &state.completion_event_24, false, timeout);
    if (waited == WAIT_TIMEOUT) return pending_result;
    if (waited >= state.wait_handle_count_2c) return unexpected_result;
    if (waited == WAIT_OBJECT_0) {
        host.set_last_error(0);
        if (host.get_overlapped_result(state.pipe_08, state.overlapped_0c, transferred, false))
            return *transferred == 0 ? fail_result : 0;
        const auto error = host.get_last_error();
        return error == ERROR_IO_INCOMPLETE ? pending_result : error_to_result(error);
    }
    if (waited == WAIT_OBJECT_0 + 1u) {
        static_cast<void>(host.cancel_io(state.pipe_08));
        return abort_result;
    }
    return unexpected_result;
}

std::int32_t finish_xlive_pipe_read_00a5e7e6(XLivePipeNativeState& state,
    DWORD* transferred, DWORD timeout, XLivePipeIoHost& host) {
    if (state.io_state_20 != 1u) return unexpected_result;
    const auto result = wait_xlive_pipe_io_00a5e31f(state, transferred, timeout, host);
    if (result != pending_result) state.io_state_20 = 3;
    return result;
}

std::int32_t finish_xlive_pipe_write_00a5e815(XLivePipeNativeState& state,
    DWORD* transferred, DWORD timeout, XLivePipeIoHost& host) {
    if (state.io_state_20 != 2u) return unexpected_result;
    const auto result = wait_xlive_pipe_io_00a5e31f(state, transferred, timeout, host);
    if (result != pending_result) state.io_state_20 = 3;
    return result;
}

std::int32_t connect_xlive_pipe_00a5de84(XLivePipeTransport* owner, DWORD timeout,
    XLivePipeIoHost& host) {
    if (invalid_owner(owner)) return invalid_owner_result;
    return wait_xlive_pipe_connection_00a5e750(owner->native, timeout, host);
}

std::int32_t read_xlive_pipe_00a5deaa(XLivePipeTransport* owner, void* buffer,
    DWORD bytes, XLivePipeIoHost& host) {
    if (invalid_owner(owner)) return invalid_owner_result;
    if (buffer == nullptr || bytes == 0) return invalid_argument_result;
    return submit_xlive_pipe_read_00a5e1f7(owner->native, buffer, bytes, host);
}

std::int32_t finish_xlive_pipe_read_00a5dee8(XLivePipeTransport* owner, DWORD* transferred,
    DWORD timeout, XLivePipeIoHost& host) {
    if (invalid_owner(owner)) return invalid_owner_result;
    if (transferred == nullptr) return invalid_argument_result;
    return finish_xlive_pipe_read_00a5e7e6(owner->native, transferred, timeout, host);
}

std::int32_t write_xlive_pipe_00a5df20(XLivePipeTransport* owner, const void* buffer,
    DWORD bytes, XLivePipeIoHost& host) {
    if (invalid_owner(owner)) return invalid_owner_result;
    if (buffer == nullptr || bytes == 0) return invalid_argument_result;
    return submit_xlive_pipe_write_00a5e28b(owner->native, buffer, bytes, host);
}

std::int32_t finish_xlive_pipe_write_00a5df5e(XLivePipeTransport* owner, DWORD* transferred,
    DWORD timeout, XLivePipeIoHost& host) {
    if (invalid_owner(owner)) return invalid_owner_result;
    if (transferred == nullptr) return invalid_argument_result;
    return finish_xlive_pipe_write_00a5e815(owner->native, transferred, timeout, host);
}
} // namespace bsp
