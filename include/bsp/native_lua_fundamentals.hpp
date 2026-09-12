#pragma once

#include "bsp/native_lua_bootstrap.hpp"
#include "bsp/singleton_lifetime.hpp"
#include "bsp/native_lua_vfs_dispatch.hpp"
#include <cstddef>

namespace bsp {
// Same actual0Ch record consumed by B6A020. No C++ vptr or hidden ownership.
// B68340 initializes only vtable00 on the closed-stream path; callers must
// retain allocation preimage rather than value-initialize data04/size08.
using NativeLuaFundamentalsOwner = NativeLuaFundamentalsView;
static_assert(offsetof(NativeLuaFundamentalsOwner, bytes_04) == 4);
static_assert(offsetof(NativeLuaFundamentalsOwner, size_08) == 8);

struct NativeLuaFundamentalsContext {
    SingletonLifetimeDomain& lifetime; // application's canonical01090AA0 domain
    NativeLuaFundamentalsOwner* volatile& publication_0108ff1c;
    NativeStringStorage& strings; // may be ActualNativeStringPoolStorage
    // Actual VFS owner. The default adapter requires callable original-ABI
    // tables; an explicit NativeVfsRuntimeBindings handles native identities.
    void* const volatile& manager_0109ceec;
    NativeLuaVfsDispatch& vfs=callable_native_lua_vfs_dispatch();
};

// B68340..B68458 normal flow and EH transitions within the returning-cleanup
// host domain: ECX raw allocation, EAX owner, RET. Native string length18h,
// mode2; unchecked stream, slot18 gate, slot30
// lowDWORD size, exact byte allocation, slot24 read (result/count ignored).
// Closed stream retains data/size preimage AND stream reference. Success
// decrements actual refs04 and invokes current slot00 at zero. No terminator.
// On unwind only constructed path and singleton base are destroyed; neither
// stream nor byte allocation is released. Original FH3 ABI is not supplied.
// NativeStringStorage::release is noexcept. ActualNativeStringPoolStorage
// covers only a returning pool getter during release; failure while lazily
// recreating that pool terminates and is excluded from EH coverage here.
NativeLuaFundamentalsOwner* construct_native_lua_fundamentals_00b68340(
    void* actual_allocation, NativeLuaFundamentalsContext&);

// Full00884770..0088482C: captured shared lock, double check, raw0Ch allocation,
// publication before second manager lookup/registration, final post-unlock
// reload. Unwind frees only still-unpublished raw construction storage.
// Constructor cleanup inherits the returning-getter domain described above.
NativeLuaFundamentalsOwner* get_native_lua_fundamentals_00884770(
    NativeLuaFundamentalsContext&);
// Direct adapter for NativeLuaBootstrapInputs::get_fundamentals_00884770.
const NativeLuaFundamentalsView* native_lua_fundamentals_callback(void* context);

// B667D0 ECX owner, RET: clear publication unconditionally and writeCE3818.
// This is the constructor's state0 unwind, not the owning destructor.
void destroy_native_lua_fundamentals_base_00b667d0(NativeLuaFundamentalsOwner&,
    NativeLuaFundamentalsOwner* volatile& publication) noexcept;
// Full diskB66B80..B66BC5: ECX owner, stack flags, EAX original, RET4.
// WriteD62C18; free nonnull bytes then clear data04; clear publication;
// writeCE3818; free owner if flags&1. size08 is never cleared. No unregister.
NativeLuaFundamentalsOwner* delete_native_lua_fundamentals_00b66b80(
    NativeLuaFundamentalsOwner&, std::uint32_t flags,
    NativeLuaFundamentalsOwner* volatile& publication) noexcept;

// Compose before constructing the canonical domain. D62C18 is identity data;
// this binding dispatches rebuilt destruction directly and forwards all other
// owners and validation to REQUIRED callbacks. Retain until domain shutdown.
class NativeLuaFundamentalsLifetimeBinding final {
public:
    NativeLuaFundamentalsLifetimeBinding(NativeLuaFundamentalsOwner* volatile&,
        SingletonLifetimeCallbacks next);
    NativeLuaFundamentalsLifetimeBinding(const NativeLuaFundamentalsLifetimeBinding&) = delete;
    NativeLuaFundamentalsLifetimeBinding& operator=(const NativeLuaFundamentalsLifetimeBinding&) = delete;
    SingletonLifetimeCallbacks callbacks() noexcept;
private:
    static void destroy_registered(void*, void*, std::uint32_t) noexcept;
    static void invalid_parameter(void*);
    NativeLuaFundamentalsOwner* volatile& publication_;
    SingletonLifetimeCallbacks next_;
};
} // namespace bsp
