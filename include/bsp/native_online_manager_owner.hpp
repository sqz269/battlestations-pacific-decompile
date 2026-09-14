#pragma once

#include "bsp/native_online_notifications.hpp"

#include <cstdint>

namespace bsp {

// Stable references to the application's actual publication cells. Both
// values remain mutable, including across the two singleton-manager getter
// calls in each native body. No independent manager state is stored here.
struct NativeOnlineManagerBaseContext final {
    NativeOnlineManagerStorage* volatile& current_manager_00f8abe8;
    void* volatile& singleton_manager_01090aa0;
};

// Complete A3F530 normal body and C++ cleanup: ECX is the caller-owned raw
// 3F0h allocation, EAX returns that same address, plain RET. The function
// writes D24138, captures the first singleton manager's +10h section, then
// publishes F8ABE8 before the second getter/current-slot registration. A
// failed registration retains publication/partial registration and restores
// root profile CE3818 after releasing the captured section.
NativeOnlineManagerStorage* construct_native_online_manager_base_00a3f530(
    NativeOnlineManagerStorage&, NativeOnlineManagerBaseContext&);

// Complete A3F5D0 normal body and C++ cleanup: ECX is the captured raw owner,
// plain RET. Unregisters the CURRENT F8ABE8 value rather than assuming it is
// the captured owner, then clears that publication and writes CE3818.
void destroy_native_online_manager_base_00a3f5d0(
    NativeOnlineManagerStorage&, NativeOnlineManagerBaseContext&);

// Complete A3F670 scalar deleting wrapper: ECX is the captured raw owner,
// flags occupy one DWORD stack slot, RET4, EAX returns captured ECX even after
// free. Only bit0 frees storage. A flags1 caller must have allocated the exact
// NativeOnlineManagerStorage object with `new NativeOnlineManagerStorage`
// (default initialization, no whole-blob zeroing) and must relinquish it.
// Neither destructor nor delete releases any projected context or SDK state.
NativeOnlineManagerStorage* delete_native_online_manager_base_00a3f670(
    NativeOnlineManagerStorage&, std::uint32_t flags,
    NativeOnlineManagerBaseContext&);

} // namespace bsp
