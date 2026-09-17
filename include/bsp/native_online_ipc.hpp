#pragma once

#include "bsp/xlive_ipc.hpp"

namespace bsp {
class ReconstructedXLivePipeServices;

// The producer A4C030 requests exactly 2Ch bytes. Reuse the established layout,
// not XLiveIpc (which appends projected service pointers). +14/+20 retain their
// allocation preimages until written by the worker.
using NativeOnlineIpcStorage = XLiveIpcNativeState;

struct NativeOnlineIpcMemory final {
    void* (__cdecl* allocate_nothrow)(std::size_t);
    void (__cdecl* release)(void*);
};
NativeOnlineIpcMemory standard_native_online_ipc_memory() noexcept;

// Services live outside the native allocation. The concrete pipe implementation
// is ReconstructedXLivePipeServices, whose returned owner stays opaque here.
// Explicit local preimages cover success-without-output host observations; they
// do not claim to recover the original stack contents.
struct NativeOnlineIpcRuntime final {
    XLiveIpcPipeHost& pipes;
    XLiveIpcSystemHost& system;
    NativeOnlineIpcMemory memory;
    std::uint32_t capacity_preimage;
    std::uint32_t sent_preimage;
};
NativeOnlineIpcRuntime make_win32_native_online_ipc_runtime(
    ReconstructedXLivePipeServices&, std::uint32_t capacity_preimage,
    std::uint32_t sent_preimage);

// One immutable process binding supplies the original fixed service/code
// environment to the stdcall thread entry. Bind the FINAL runtime address before
// creating a real worker. Runtime and its dependencies must last until process
// exit. Rebinding to another address is rejected, including after a timeout.
// There is no endpoint registry, per-owner context, or thread payload allocation.
void bind_native_online_ipc_worker_runtime(const NativeOnlineIpcRuntime&);

// Complete normal bodies; descriptive names are hypotheses. Original ECX is the
// output/owner/slot pointer, RET, EAX signed HRESULT where applicable. These C++
// interfaces are source bindings, not drop-in register-ABI replacements.
std::int32_t create_native_online_ipc_00a4c030(NativeOnlineIpcStorage**,
    const NativeOnlineIpcRuntime&);
std::int32_t initialize_native_online_ipc_slot_00a4c250(std::uint32_t* slot,
    const NativeOnlineIpcRuntime&);
void destroy_native_online_ipc_00a4bde0(NativeOnlineIpcStorage*,
    const NativeOnlineIpcRuntime&);
// Ignores 0/FFFFFFFF; never clears the caller's retained slot.
void close_native_online_ipc_handle_00a4c280(std::uint32_t handle,
    const NativeOnlineIpcRuntime&);
std::uint32_t native_online_ipc_last_error_00a4bc80(XLiveIpcSystemHost&);

// Native callbacks: stdcall, three DWORD arguments, RET12. Context is the actual
// 2Ch endpoint. Loop: ECX=endpoint, RET. Worker: stdcall, RET4. The worker may run
// before +04/output publication. Destruction ignores the 1000ms wait result and
// frees the endpoint even if the worker has not exited; it is not a safe join.
std::int32_t __stdcall encode_native_online_ipc_00a4bd40(void*,
    std::uint32_t*, void*);
std::int32_t __stdcall decode_native_online_ipc_00a4bd80(const void*,
    std::uint32_t, void*);
std::int32_t run_native_online_ipc_loop_00a4be50(NativeOnlineIpcStorage&,
    const NativeOnlineIpcRuntime&);
DWORD WINAPI run_native_online_ipc_thread_00a4c000(void*);
} // namespace bsp
