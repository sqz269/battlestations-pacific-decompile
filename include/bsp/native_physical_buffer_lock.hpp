#pragma once

#include "bsp/native_diagnostic_sink_lifetime.hpp"

#include <cstdint>

namespace bsp {
struct NativePhysicalBufferLockContext {
    NativeDiagnosticSinkStorage* volatile& actual_diagnostic_0109cf14;
    SoundLifetimeAccess actual_lifetime_01090aa0;
    // ADDRESS of the actual sentinel bytes, not a pointer loaded from them.
    void* const actual_null_buffer_sentinel_00f8d4b8;
};
static_assert(sizeof(NativePhysicalBufferLockContext) == 16);

// Original: ECX actual 2Ch owner; stack bytes, extra offset, unused DWORD,
// output DWORD address, read-only byte; EAX data pointer; RET 14h. These new
// host bindings additionally require context in EDX, keeping all five native
// stack arguments. Original callers need an explicit service-context binding.
// The output address can be unaligned or overlap the actual owner. Native
// diagnostic, null-sentinel and failed-HRESULT paths all execute; no guards,
// fallback errors, COM retention or automatic Unlock/rollback are added.
void* __fastcall lock_native_physical_index_buffer_00b4b850(
    void* actual_owner, NativePhysicalBufferLockContext& actual_context,
    std::uint32_t bytes, std::uint32_t extra_offset, std::uint32_t unused,
    void* actual_base_offset_output, std::uint8_t read_only);
void* __fastcall lock_native_physical_vertex_buffer_00b4ba00(
    void* actual_owner, NativePhysicalBufferLockContext& actual_context,
    std::uint32_t bytes, std::uint32_t extra_offset, std::uint32_t unused,
    void* actual_base_offset_output, std::uint8_t read_only);
} // namespace bsp
