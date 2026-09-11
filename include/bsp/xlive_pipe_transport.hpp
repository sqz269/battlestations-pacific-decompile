#pragma once

#include "bsp/xlive_ipc.hpp"
#include <accctrl.h>
#include <tlhelp32.h>

namespace bsp {

// One canonical native 30h owner for transport, I/O, and framing consumers.
// Constructor leaves mode and OVERLAPPED bytes untouched. Successful open
// sets overlapped.hEvent (+1C) to completion_event_24; these are actual handles.
struct XLivePipeNativeState {
    void* protocol_00;
    std::uint32_t mode_04;
    HANDLE pipe_08;
    OVERLAPPED overlapped_0c;
    std::uint32_t io_state_20;
    HANDLE completion_event_24;
    HANDLE stop_event_28; // borrowed, never closed by this owner
    std::uint32_t wait_handle_count_2c;
};

// Actual A8h protocol allocation/CryptoAPI/arithmetic lifecycle remains a
// substantive required dependency. PIPEIPC names are hypotheses, not evidence
// of library authorship. No successful protocol default is supplied.
struct XLivePipeProtocolHost {
    virtual ~XLivePipeProtocolHost() = default;
    virtual void initialize_protocol_00a5f416(void*& slot) = 0;
    virtual void destroy_protocol_00a5f371(void* protocol) = 0;
};

struct XLivePipeTransport;
struct XLivePipeSystemHost {
    virtual ~XLivePipeSystemHost() = default;
    virtual XLivePipeTransport* allocate_owner() = 0;
    virtual void free_owner(XLivePipeTransport*) noexcept = 0;
    virtual void set_last_error(DWORD) = 0;
    virtual DWORD get_last_error() = 0;
    virtual void close_handle(HANDLE) = 0;
    virtual DWORD current_process_id() = 0;
    virtual HANDLE current_process() = 0;
    virtual HANDLE snapshot_processes() = 0;
    virtual bool process_first(HANDLE, PROCESSENTRY32W&) = 0;
    virtual bool process_next(HANDLE, PROCESSENTRY32W&) = 0;
    virtual HANDLE open_parent_process(DWORD pid) = 0;
    virtual bool process_times(HANDLE, FILETIME&, FILETIME&, FILETIME&, FILETIME&) = 0;
    virtual LONG compare_file_time(const FILETIME&, const FILETIME&) = 0;
    virtual bool open_process_token(HANDLE, HANDLE&) = 0;
    virtual bool token_groups(HANDLE, void*, DWORD bytes, DWORD& required) = 0;
    virtual HLOCAL local_alloc(SIZE_T bytes) = 0;
    virtual void local_free(HLOCAL) = 0;
    virtual DWORD sid_length(PSID) = 0;
    virtual bool copy_sid(DWORD bytes, PSID target, PSID source) = 0;
    virtual bool allocate_network_sid(PSID&) = 0;
    virtual void free_sid(PSID) = 0;
    virtual DWORD set_acl_entries(EXPLICIT_ACCESS_W* entries, PACL&) = 0;
    virtual bool initialize_security_descriptor(PSECURITY_DESCRIPTOR) = 0;
    virtual bool set_security_dacl(PSECURITY_DESCRIPTOR, PACL) = 0;
    virtual HANDLE create_server_pipe(const wchar_t*, SECURITY_ATTRIBUTES&) = 0;
    virtual HANDLE open_client_pipe(const wchar_t*) = 0;
    virtual bool set_client_message_mode(HANDLE, DWORD& mode) = 0;
    virtual HANDLE create_completion_event() = 0;
};

// Extra host context follows the exact native prefix; pointers remain fixed
// throughout all I/O. The allocation and protocol host must outlive this owner.
struct XLivePipeTransport {
    XLivePipeNativeState native;
    XLivePipeProtocolHost* protocol;
    XLivePipeSystemHost* system;
};

class Win32XLivePipeSystemHost : public XLivePipeSystemHost {
public:
    XLivePipeTransport* allocate_owner() override;
    void free_owner(XLivePipeTransport*) noexcept override;
    void set_last_error(DWORD) override;
    DWORD get_last_error() override;
    void close_handle(HANDLE) override;
    DWORD current_process_id() override;
    HANDLE current_process() override;
    HANDLE snapshot_processes() override;
    bool process_first(HANDLE, PROCESSENTRY32W&) override;
    bool process_next(HANDLE, PROCESSENTRY32W&) override;
    HANDLE open_parent_process(DWORD) override;
    bool process_times(HANDLE, FILETIME&, FILETIME&, FILETIME&, FILETIME&) override;
    LONG compare_file_time(const FILETIME&, const FILETIME&) override;
    bool open_process_token(HANDLE, HANDLE&) override;
    bool token_groups(HANDLE, void*, DWORD, DWORD&) override;
    HLOCAL local_alloc(SIZE_T) override;
    void local_free(HLOCAL) override;
    DWORD sid_length(PSID) override;
    bool copy_sid(DWORD, PSID, PSID) override;
    bool allocate_network_sid(PSID&) override;
    void free_sid(PSID) override;
    DWORD set_acl_entries(EXPLICIT_ACCESS_W*, PACL&) override;
    bool initialize_security_descriptor(PSECURITY_DESCRIPTOR) override;
    bool set_security_dacl(PSECURITY_DESCRIPTOR, PACL) override;
    HANDLE create_server_pipe(const wchar_t*, SECURITY_ATTRIBUTES&) override;
    HANDLE open_client_pipe(const wchar_t*) override;
    bool set_client_message_mode(HANDLE, DWORD&) override;
    HANDLE create_completion_event() override;
};

// A5E145 ECX=owner, RET/EAX=owner. Protocol init precedes all own field stores.
XLivePipeTransport* construct_xlive_pipe_00a5e145(XLivePipeTransport&);
// A5E710 stdcall(owner), RET4. Close pipe unless -1 (including null); close
// completion event only if nonnull; protocol teardown; free owner. Stop event
// and overlapped hEvent are borrowed aliases and are not separately closed.
void destroy_xlive_pipe_00a5e710(XLivePipeTransport*);
// A5DE68 stdcall(handle), RET4. Null and -1 are ignored.
void close_xlive_pipe_00a5de68(XLivePipeTransport*);
// A5E557 stdcall(DWORD*), RET4. Find this process in Toolhelp snapshot, open its
// recorded parent and accept it only if parent creation time is strictly older.
// Returns raw Win32 error; success alone writes output.
DWORD find_xlive_pipe_parent_00a5e557(DWORD& output, XLivePipeSystemHost&);
// A5E3D0 stdcall(HLOCAL*), RET4. Copy first SE_GROUP_LOGON_ID SID from token
// groups into a LocalAlloc block. Returns raw error; caller owns success block.
DWORD copy_xlive_pipe_logon_sid_00a5e3d0(HLOCAL& output, XLivePipeSystemHost&);
// A5E844 stdcall(mode, borrowedStopEvent, output*), RET12. Mode0 opens the
// parent's pipe; nonzero creates this process's server with recovered ACL.
// Returned failures free temporaries/owner and write null output, then convert
// positive Win32 errors to HRESULT. Null output itself returns E_INVALIDARG.
// Allocator/protocol exceptions propagate without an invented cleanup scope.
std::int32_t create_xlive_pipe_00a5e844(std::uint32_t mode, HANDLE stop_event,
    XLivePipeTransport** output, XLivePipeProtocolHost&, XLivePipeSystemHost&);
// A5DE34 stdcall(mode,event,output*), RET12. Uses local output and writes null
// on negative result. No raw EXE addresses are invoked by this projection.
std::int32_t open_xlive_pipe_00a5de34(std::uint32_t mode, HANDLE stop_event,
    XLivePipeTransport** output, XLivePipeProtocolHost&, XLivePipeSystemHost&);

} // namespace bsp
