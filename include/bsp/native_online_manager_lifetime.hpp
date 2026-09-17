#pragma once

#include "bsp/native_online_manager_owner.hpp"
#include "bsp/native_online_ipc.hpp"
#include "bsp/native_online_pump.hpp"
#include "bsp/native_online_startup_sdk.hpp"

namespace bsp {

// Borrow these services until the raw singleton drain returns. The base
// context uses the SAME F8ABE8/01090AA0 cells as construction and registration.
// Memory releases the CRT allocations retained at +14C and +364; it must match
// the allocator used by the pump. Neither this context nor the raw owner owns
// a projected XLiveManagerOwner or an identity map.
struct NativeOnlineManagerLifetimeContext final {
    NativeOnlineManagerBaseContext& base;
    const NativeOnlineIpcRuntime& ipc;
    const NativeOnlineStorageMemory& memory;
};

// The two stack images occupy separate caller-owned storage. The constructor
// clears initialize_info, but deliberately retains wsadata's explicit preimage:
// the native parent reads its version even after a failed/partial SDK write.
struct NativeOnlineManagerStartupContext final {
    NativeOnlineManagerLifetimeContext& lifetime;
    void* volatile& current_renderer_00f8d394;
    NativeOnlineStartupSdkCalls& sdk;
    NativeOnlineInitializeInfo1c& initialize_info;
    NativeOnlineWsadata400& wsadata;
};

// Full A40DF0 normal body. Native ECX=actual 3F0h allocation, two DWORD stack
// callbacks, RET8, EAX=same allocation. The source captures the owner from
// pump.notifications.manager, so reset and pump operate on that exact owner.
// Publication occurs in A3F530, before derived initialization. The current
// renderer's +1A28 is passed directly to the SDK as a mutable pointer.
NativeOnlineManagerStorage* construct_native_online_manager_00a40df0(
    NativeOnlinePumpContext&, std::uint32_t callback20, std::uint32_t callback24,
    NativeOnlineManagerStartupContext&);

// A3F840 takes the actual embedded header at manager+360h, not the manager.
// Releases current header+4, then zeroes +4/+8/+C after release returns.
void destroy_native_online_vector_00a3f840(void* header,
    const NativeOnlineStorageMemory&);

// Full A3F9D0 normal body: IPC close; current +14C release/clear; current
// +364 vector release/clear; base unregister. No listener/SDK shutdown and no
// +3A0 release occur here. The retained +3AC slot is not cleared.
void destroy_native_online_manager_00a3f9d0(NativeOnlineManagerStorage&,
    NativeOnlineManagerLifetimeContext&);

// Full A3FDC0 scalar wrapper. Only flags bit0 frees the captured allocation,
// after destruction returns, and the captured address is returned after free.
// A consuming caller must use `new NativeOnlineManagerStorage` without () and
// relinquish it. These source interfaces do not reproduce native FH3/SEH ABI.
NativeOnlineManagerStorage* delete_native_online_manager_00a3fdc0(
    NativeOnlineManagerStorage&, std::uint32_t flags,
    NativeOnlineManagerLifetimeContext&);

} // namespace bsp
