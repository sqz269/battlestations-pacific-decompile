#pragma once

#include "bsp/native_hardware_layout_construct.hpp"

namespace bsp {

// Complete B60A10[666], including its unreachable four-byte alignment gap.
// Original: ECX actual owner, RET, no semantic result. New Win32 interface:
// ECX owner, EDX context, RET. Does not reproduce native FS:[0]/FH3 ABI.
// Nonzero DWORD owner+40 returns before ANY other owner/context/provider read;
// that path permits a null context. A miss requires the actual fixed context.
// Capture owner+38 once, using its UNSIGNED bound to copy existing pointers
// owner+8+0C*i into four local slots BEFORE diagnostics. Later traversal uses
// the signed captured count. The valid domain is 0..4; no new guard is added.
// Streams are neither appended nor released. Their raw data/count remain live.
// Raw usage DWORDs must be <14 and total output including END must fit16slots.
// Output owner+40 goes directly to current renderer/device COM+158; HRESULT is
// ignored. Full B47D60 then reads the current owner stream list/count/strides.
// actual_string_storage identifies the owning allocator for BOTH diagnostics;
// actual pool composition uses ActualNativeStringPoolStorage with the one
// publication, shutdown gate and shared lifetime domain. A semantic allocator
// is a narrower host boundary. release is noexcept; native cleanup-allocation
// failures/SEH are not covered by that pre-existing storage contract.
void __fastcall create_native_hardware_layout_if_missing_00b60a10(
    void* actual_owner, NativeHardwareLayoutConstructContext* actual_context);

} // namespace bsp
