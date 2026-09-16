#pragma once
#include "bsp/native_renderer_synchronization_actual.hpp"

namespace bsp {
// All storage is borrowed. The profile views contain immutable original code
// words, not host callbacks: D62AD0[0]=BD30E0,[1]=B5FE40 and D5F0A8[10]=B27CF0.
// Both the current published renderer and its profile are reloaded after COM
// Release. This context owns no query, renderer, registry or operation state.
struct NativeRendererQueryTerminalContext {
    void* const volatile& actual_renderer_00f8d394;
    NativeRendererSynchronizationGlobals& actual_synchronization_0108d6dc;
    const volatile std::uint32_t* actual_query_profile_00d62ad0; // 2 DWORDs
    const volatile std::uint32_t* actual_renderer_profile_00d5f0a8; // 11 DWORDs
};

// Complete B25290: ECX actual {data,count,capacity}, stack pointer-to-DWORD,
// RET4/full EAX0 or1. Captured base/count/end/target; first match replaced with
// captured last cell, then decrement CURRENT count. Capacity/stale cell retained.
// Empty/wrapping-backward range does not read the target pointer. No ownership.
std::uint32_t remove_native_renderer_query_pointer_00b25290(
    void* actual_array, const void* actual_value_word) noexcept;

// B27CF0: ECX renderer, stack query, RET4; no stable EAX across optional leave.
// Actual array+19A0; reuse actual B33AD0/B33B00. No EH/RAII cleanup exists here.
// Disabled-entry/enabled-exit mode exposes an unwritten native guard record and
// is outside the valid native domain. Current exit mode governs leaving.
void unregister_native_renderer_query_00b27cf0(void* actual_renderer,
    void* actual_query, NativeRendererSynchronizationGlobals&);

// Complete11-byte B5FD40 base: native ECX/RET, stamp D62AB0 then tail BD30F0,
// whose genuine existing provider stamps CEB130. No other field/free operation.
void __fastcall destroy_native_renderer_query_base_00b5fd40(void*) noexcept;

// B5FDA0: stamp D62AD0, capture COM+10, arm base cleanup, actual captured COM
// Release via current slot8, then clear CURRENT field only after return. Reload
// current renderer publication/profile slot28; execute actual B27CF0. Disarm
// before normal base cleanup. Unwind restores base without retry or rollback.
// Accessed prefix is14h bytes; this does not assert the full allocation size.
void destroy_native_renderer_query_00b5fda0(void* actual_query,
    NativeRendererQueryTerminalContext&);

// B5FE40: destroy then shared CRT free iff flags bit0; return original address,
// possibly freed. Native ECX, stack flags, EAX original address, RET4.
void* delete_native_renderer_query_00b5fe40(void* actual_query,
    std::uint32_t flags, NativeRendererQueryTerminalContext&);

// Additional concrete BD30E0 finite-profile API. Null returns without context
// access; otherwise reload CURRENT D62AD0 profile, read current slot4, and call
// B5FE40(flags1). Caller supplies an already-selected slot0 terminal; this entry
// does not decrement references or reread slot0. Existing generic API retained.
void invoke_native_renderer_query_delete_00bd30e0(void* actual_query,
    NativeRendererQueryTerminalContext&);

// Raw extents and callable COM storage remain valid across providers; owning
// scalar flags require storage from the shared lifetime allocation domain.
// Source C++ exceptions preserve native publication/cleanup order, but original
// register/private-frame/FH3/asynchronous-SEH identity and gameplay are unproved.
} // namespace bsp
