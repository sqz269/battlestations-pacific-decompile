#pragma once
#include "bsp/native_online_notifications.hpp"
#include <cstdint>

namespace bsp {
struct XLiveIpc;

// Borrow the canonical raw publication cells through the complete shared
// singleton drain. No projected XLiveManagerOwner or second allocation is
// involved. The manager and all +14C/+364 allocations use the same CRT domain
// as singleton_lifetime_free. Non-null/non-minus-one +3AC must identify a real
// reconstructed XLiveIpc with services alive through its worker teardown.
struct NativeOnlineLifetimeContext final {
    void* volatile& actual_manager_01090aa0;
    NativeOnlineManagerStorage* volatile& actual_online_00f8abe8;
};

// A3F530[145]: stamp D24138, capture/enter the first manager's section,
// publish this actual 3F0h allocation, get the manager again, reload F8ABE8
// and register that identity. Only +0 is initialized; preserve all other bytes.
// A3F5D0[153]: unregister CURRENT F8ABE8 from the second manager (not self),
// clear the cell, leave the captured section, then stamp CE3818 on self.
// Original ECX=self, RET; constructor returns self. Source adds EDX=context.
NativeOnlineManagerStorage* __fastcall construct_native_online_base_00a3f530(
    NativeOnlineManagerStorage*, NativeOnlineLifetimeContext&);
void __fastcall destroy_native_online_base_00a3f5d0(
    NativeOnlineManagerStorage*, NativeOnlineLifetimeContext&);

// A3F840[42]: ECX is the embedded header at manager+360, NOT manager.
// Free its non-null +4, then zero +4/+8/+C; preserve the header's +0 word.
void __fastcall destroy_native_online_achievement_ids_00a3f840(void* actual_vector_360);
// A4C280[15]: ECX handle, RET; null/-1 do nothing, otherwise tail teardown
// through the existing concrete IPC body. Does not clear the retained slot.
void __fastcall close_native_online_ipc_00a4c280(XLiveIpc*);
// A3F9D0[159]: derived profile, IPC close, free/zero +14C, free/zero vector,
// base teardown. No listener, SDK, Winsock or achievement-batch cleanup.
void __fastcall destroy_native_online_00a3f9d0(
    NativeOnlineManagerStorage*, NativeOnlineLifetimeContext&);
// A3F670/A3FDC0[30 each]: destructor, bit0 free, return captured pointer even
// after free. Native ECX self / stack DWORD flags / RET4; source adds EDX.
NativeOnlineManagerStorage* __fastcall delete_native_online_base_00a3f670(
    NativeOnlineManagerStorage*, NativeOnlineLifetimeContext&, std::uint32_t flags);
NativeOnlineManagerStorage* __fastcall delete_native_online_00a3fdc0(
    NativeOnlineManagerStorage*, NativeOnlineLifetimeContext&, std::uint32_t flags);

// Ordinary source C++ cleanup preserves the recovered ownership actions:
// failed base registration releases its guard, restores CE3818, and leaves
// publication intact; failed IPC close destroys vector/base but not +14C.
// This is not FH3/SEH or binary-ABI equivalence. Full A40DF0 construction and
// application SDK/IPC/online-pump composition are separate dependencies.
} // namespace bsp
