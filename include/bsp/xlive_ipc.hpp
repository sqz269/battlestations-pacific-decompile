#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <cstddef>
#include <cstdint>

namespace bsp {

using XLiveIpcEncodeCallback = std::int32_t (__stdcall*)(void*, std::uint32_t*, void*);
using XLiveIpcDecodeCallback = std::int32_t (__stdcall*)(const void*, std::uint32_t, void*);

// These named-pipe operations have direct bodies in the EXE, not xlive.dll
// imports. Existing PIPEIPC names and library provenance are analyst hypotheses.
// A host must bind their genuine implementation; there is no substitute
// protocol, successful default, or raw fixed-address dispatch here.
struct XLiveIpcPipeHost {
    virtual ~XLiveIpcPipeHost() = default;
    virtual std::int32_t open_00a5de34(std::uint32_t mode, HANDLE stop_event, void*& pipe) = 0;
    virtual void close_00a5de68(void* pipe) = 0; // return value is not defined/consumed
    virtual std::int32_t send_capacity_00a5df96(void* pipe, std::uint32_t channel, std::uint32_t&) = 0;
    virtual std::int32_t receive_capacity_00a5dfce(void* pipe, std::uint32_t channel, std::uint32_t&) = 0;
    virtual std::int32_t encode_00a5e055(void* pipe, void* buffer, std::uint32_t& bytes,
        XLiveIpcEncodeCallback, void* callback_context) = 0;
    virtual std::int32_t send_00a5df20(void* pipe, const void* buffer, std::uint32_t bytes) = 0;
    virtual std::int32_t wait_send_00a5df5e(void* pipe, std::uint32_t& bytes, std::uint32_t timeout) = 0;
    virtual std::int32_t receive_00a5deaa(void* pipe, void* buffer, std::uint32_t capacity) = 0;
    virtual std::int32_t wait_receive_00a5dee8(void* pipe, std::uint32_t& bytes, std::uint32_t timeout) = 0;
    virtual std::int32_t decode_00a5e09e(void* pipe, const void* buffer, std::uint32_t bytes,
        XLiveIpcDecodeCallback, void* callback_context) = 0;
};

struct XLiveIpc;
struct XLiveIpcSystemHost {
    virtual ~XLiveIpcSystemHost() = default;
    // Raw object allocation: native request is 2Ch. Preserve allocation
    // preimages at native +14/+20; create writes neither of them.
    virtual XLiveIpc* allocate_object() = 0;
    virtual std::uint8_t* allocate_bytes(std::uint32_t) = 0;
    virtual void free_object(XLiveIpc*) noexcept = 0;
    virtual void free_bytes(std::uint8_t*) noexcept = 0;
    virtual void set_last_error(std::uint32_t) = 0;
    virtual std::uint32_t get_last_error() = 0;
    virtual HANDLE create_stop_event() = 0; // CreateEventA(null,TRUE,FALSE,null)
    virtual HANDLE create_thread(LPTHREAD_START_ROUTINE, void*) = 0;
    virtual void set_event(HANDLE) = 0;
    virtual std::uint32_t wait_for_single_object(HANDLE, std::uint32_t milliseconds) = 0;
    virtual void close_handle(HANDLE) = 0;
    virtual void sleep(std::uint32_t milliseconds) = 0;
    virtual void get_system_time(SYSTEMTIME&) = 0;
    virtual HANDLE get_current_process() = 0;
    virtual void terminate_process(HANDLE, std::uint32_t code) = 0;
    [[noreturn]] virtual void exit_process(int code) = 0;
};

// Exact 2Ch Win32 projection. No field has an invented constructor default.
struct XLiveIpcNativeState {
    void* pipe_00;
    HANDLE thread_04;
    HANDLE stop_event_08;
    std::uint8_t* receive_buffer_0c;
    std::uint32_t receive_capacity_10;
    std::uint32_t received_bytes_14;
    std::uint8_t* send_buffer_18;
    std::uint32_t send_capacity_1c;
    std::uint32_t send_bytes_20;
    std::uint32_t phase_24;
    std::uint32_t counter_28;
};

// Host pointers supply the actual reconstructed thread's services. They do not
// occupy native fields and must outlive the object and every running callback.
struct XLiveIpc {
    XLiveIpcNativeState native;
    XLiveIpcPipeHost* pipes;
    XLiveIpcSystemHost* system;
};

class Win32XLiveIpcSystemHost final : public XLiveIpcSystemHost {
public:
    XLiveIpc* allocate_object() override;
    std::uint8_t* allocate_bytes(std::uint32_t) override;
    void free_object(XLiveIpc*) noexcept override;
    void free_bytes(std::uint8_t*) noexcept override;
    void set_last_error(std::uint32_t) override;
    std::uint32_t get_last_error() override;
    HANDLE create_stop_event() override;
    HANDLE create_thread(LPTHREAD_START_ROUTINE, void*) override;
    void set_event(HANDLE) override;
    std::uint32_t wait_for_single_object(HANDLE, std::uint32_t) override;
    void close_handle(HANDLE) override;
    void sleep(std::uint32_t) override;
    void get_system_time(SYSTEMTIME&) override;
    HANDLE get_current_process() override;
    void terminate_process(HANDLE, std::uint32_t) override;
    [[noreturn]] void exit_process(int) override;
};

// A4BC80: RET; GetLastError with zero -> 507h, no HRESULT conversion here.
std::uint32_t xlive_ipc_last_error_00a4bc80(XLiveIpcSystemHost&);
// A4C030: ECX=output pointer, RET, signed HRESULT. Only null output and initial
// allocation failure return normally on failure. Later failure cleans up and
// calls _exit(0); output remains unwritten. CreateThread uses the real entry
// below, with this same object as its parameter; it can run before publication.
std::int32_t create_xlive_ipc_00a4c030(XLiveIpc** output,
    XLiveIpcPipeHost&, XLiveIpcSystemHost&);
// A4BDE0: ECX=object, RET. Null is ignored. Signal, wait1000 (result ignored),
// close thread/pipe/event, free buffers/object. It does not prove worker exit:
// a still-running native worker can access freed memory after this deadline.
void destroy_xlive_ipc_00a4bde0(XLiveIpc*);

// Both callbacks are __stdcall with three DWORD stack slots, RET12.
std::int32_t __stdcall encode_xlive_ipc_probe_00a4bd40(void*, std::uint32_t*, void*);
std::int32_t __stdcall decode_xlive_ipc_probe_00a4bd80(const void*, std::uint32_t, void*);
// A4BE50: ECX=object, RET. Full synchronous worker loop; genuine PIPEIPC calls
// synchronously use the callbacks to advance phase. No dummy worker supplied.
std::int32_t run_xlive_ipc_loop_00a4be50(XLiveIpc&);
// A4C000: stdcall thread entry, RET4. Any result except E_ABORT requests actual
// TerminateProcess(GetCurrentProcess(), E_UNEXPECTED). Preserve result if that
// API returns. Caller must not launch it without the real pipe implementation.
DWORD WINAPI run_xlive_ipc_thread_00a4c000(void*);

} // namespace bsp
