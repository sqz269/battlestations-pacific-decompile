#pragma once

#include "bsp/native_logical_index_owner.hpp"

namespace bsp {

// Original numeric profile addresses are selectors, never host call targets.
// Borrow at least ELEVEN immutable DWORDs at D61DE0: slot0=BD30E0,
// slot+04=B4C1F0, slot+28=B48DC0. The owner's two physical profile views must
// each extend to EIGHT immutable DWORDs for this API (the lifetime-only owner
// API needs two): D61E10/+1C and D61E58/+1C both contain B4B840. These same
// physical views serve lifetime dispatch and the current physical getter.
struct NativeRendererIndexBindingContext {
    NativeLogicalIndexOwnerContext& actual_logical_owner;
    const volatile std::uint32_t* actual_logical_profile_00d61de0;
};

// Complete B48DC0 token adapter: native ECX logical -> current physical+08 ->
// current physical profile -> slot+1C -> full actual B4B840. Both supplied
// views must span eight immutable original-token DWORDs. Current physical
// profile may be D61E10 or D61E58; +28 is the borrowed IDirect3DIndexBuffer9*.
// The existing native_logical_index_stream_get_buffer_00b48dc0 naked entry is
// unchanged and separately supports genuinely relocated callable tables.
void* get_native_logical_index_buffer_00b48dc0(const void* actual_logical,
    const NativeLogicalIndexPhysicalProfiles& actual_eight_word_profiles);

// Complete B24B00: native ECX raw renderer, stack logical/base vertex, RET8.
// Borrow at least 1BBCh renderer bytes, actual 28h logical pool slots with real
// intrusive LONG+04, 30h pooled/2Ch private physical objects, initialized pools,
// renderer registries, actual service/global state and actual COM interfaces.
// Optional entry precedes the outer identity read. Base+17BC is always written
// after arming cleanup. A changed binding reloads old, publishes/increments new
// before decrementing captured old, then calls current logical/physical getter.
// Null input still calls current device SetIndices(NULL). Device and its table
// are read after the getter. Counter+1BB8 wraps only after returning COM work.
void bind_native_renderer_index_stream_00b24b00(void* actual_renderer,
    void* actual_logical, std::uint32_t base_vertex,
    NativeRendererIndexBindingContext&);

// Supported owners use original D61DE0 and D61E10/D61E58 profile tokens.
// The complete BD30E0 path rereads the current logical table before B4C1F0;
// full existing logical and physical lifetime providers handle nonempty state.
// No null repair, alternate-profile fallback, HRESULT gate or rollback exists.
// Accessed storage remains valid across callbacks. Skipped optional entry
// leaves native guard storage uninitialized: entry-disabled/exit-enabled mode
// transitions are outside the native valid domain. Normal leave is disarmed;
// exceptional cleanup uses actual B21110 and terminates on a second exception.
// New C++ interfaces; these are not drop-in original game ABI replacements.
} // namespace bsp
