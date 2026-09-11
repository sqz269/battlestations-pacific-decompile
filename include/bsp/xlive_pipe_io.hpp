#pragma once

#include "bsp/xlive_pipe_transport.hpp"

namespace bsp {

// Required OS boundary. All pointers/references refer to the live native prefix
// or the actual caller's output; implementations must preserve API writes on
// failure too. No successful asynchronous result or initialized output is assumed.
struct XLivePipeIoHost {
    virtual ~XLivePipeIoHost() = default;
    virtual void set_last_error(DWORD) = 0;
    virtual DWORD get_last_error() = 0;
    virtual DWORD wait_for_single_object(HANDLE, DWORD timeout) = 0;
    virtual DWORD wait_for_multiple_objects(DWORD count, const HANDLE* handles,
        bool wait_all, DWORD timeout) = 0;
    virtual bool read_file(HANDLE, void* buffer, DWORD bytes,
        DWORD* transferred, OVERLAPPED&) = 0;
    virtual bool write_file(HANDLE, const void* buffer, DWORD bytes,
        DWORD* transferred, OVERLAPPED&) = 0;
    virtual bool get_overlapped_result(HANDLE, OVERLAPPED&, DWORD* transferred,
        bool wait) = 0;
    virtual bool cancel_io(HANDLE) = 0;
    virtual bool connect_named_pipe(HANDLE, OVERLAPPED&) = 0;
    virtual bool set_event(HANDLE) = 0;
};

class Win32XLivePipeIoHost final : public XLivePipeIoHost {
public:
    void set_last_error(DWORD) override;
    DWORD get_last_error() override;
    DWORD wait_for_single_object(HANDLE, DWORD) override;
    DWORD wait_for_multiple_objects(DWORD, const HANDLE*, bool, DWORD) override;
    bool read_file(HANDLE, void*, DWORD, DWORD*, OVERLAPPED&) override;
    bool write_file(HANDLE, const void*, DWORD, DWORD*, OVERLAPPED&) override;
    bool get_overlapped_result(HANDLE, OVERLAPPED&, DWORD*, bool) override;
    bool cancel_io(HANDLE) override;
    bool connect_named_pipe(HANDLE, OVERLAPPED&) override;
    bool set_event(HANDLE) override;
};

// A5E16E stdcall(pipe,OVERLAPPED*,pendingDWORD*), RET12. Clear pending first.
// Most ConnectNamedPipe failures return raw Win32 errors (zero -> 507h).
// ERROR_IO_PENDING sets pending=1 and returns0; ERROR_PIPE_CONNECTED signals
// overlapped.hEvent. Only a failed SetEvent performs HRESULT conversion here.
std::int32_t connect_xlive_pipe_handle_00a5e16e(HANDLE, OVERLAPPED&,
    DWORD& pending, XLivePipeIoHost&);

// A5E750 ECX=owner, timeout stack, RET4. Nonzero mode and state3 start connect;
// state0 continues its wait. Timeout sets state0 and returns800705B4; completion
// success does not set state3 or query GetOverlappedResult. Other modes/states
// return E_UNEXPECTED. Stop wait cancels current pipe and returns E_ABORT.
std::int32_t wait_xlive_pipe_connection_00a5e750(XLivePipeNativeState&,
    DWORD timeout, XLivePipeIoHost&);

// A5E1F7/A5E28B ECX=owner, (buffer,bytes), RET8. Require state3. Clear only
// OffsetHigh then Offset, leaving Internal/InternalHigh/hEvent untouched.
// True Read/WriteFile OR ERROR_IO_PENDING sets state1/2 and returns E_PENDING.
std::int32_t submit_xlive_pipe_read_00a5e1f7(XLivePipeNativeState&,
    void* buffer, DWORD bytes, XLivePipeIoHost&);
std::int32_t submit_xlive_pipe_write_00a5e28b(XLivePipeNativeState&,
    const void* buffer, DWORD bytes, XLivePipeIoHost&);

// A5E31F ECX=owner, (DWORD* transferred,timeout), RET8. Wait on the actual
// adjacent +24/+28 handles; compare against the current +2C after the API.
// Completion retrieves the result without blocking: zero bytes -> E_FAIL,
// nonzero -> S_OK. Timeout/incomplete -> E_PENDING; stop -> CancelIo/E_ABORT.
std::int32_t wait_xlive_pipe_io_00a5e31f(XLivePipeNativeState&,
    DWORD* transferred, DWORD timeout, XLivePipeIoHost&);
// State1/read or state2/write only. Every result except E_PENDING resets state3,
// including errors and cancellation. Caller outputs are never precleared.
std::int32_t finish_xlive_pipe_read_00a5e7e6(XLivePipeNativeState&,
    DWORD* transferred, DWORD timeout, XLivePipeIoHost&);
std::int32_t finish_xlive_pipe_write_00a5e815(XLivePipeNativeState&,
    DWORD* transferred, DWORD timeout, XLivePipeIoHost&);

// Public stdcall wrappers. A5DE84 RET8; the remaining four RET12. First reject
// null/-1 owner with80070006; then reject null buffer/output or zero submitted
// byte count with80070057. A zero timeout is valid. No extra state is allocated.
std::int32_t connect_xlive_pipe_00a5de84(XLivePipeTransport*, DWORD timeout,
    XLivePipeIoHost&);
std::int32_t read_xlive_pipe_00a5deaa(XLivePipeTransport*, void* buffer,
    DWORD bytes, XLivePipeIoHost&);
std::int32_t finish_xlive_pipe_read_00a5dee8(XLivePipeTransport*, DWORD* transferred,
    DWORD timeout, XLivePipeIoHost&);
std::int32_t write_xlive_pipe_00a5df20(XLivePipeTransport*, const void* buffer,
    DWORD bytes, XLivePipeIoHost&);
std::int32_t finish_xlive_pipe_write_00a5df5e(XLivePipeTransport*, DWORD* transferred,
    DWORD timeout, XLivePipeIoHost&);

} // namespace bsp
