#pragma once
#include "bsp/native_damageable_class_section_msh_field_fragment.hpp"

namespace bsp {

struct NativeDamageableClassSectionIndexFragmentScratch {
    NativeDamageableClassSectionMshFieldFragment& live_msh_owner;
    NativeDamageableClassSectionMshFieldFragmentScratch& same_msh_scratch;
    void* fresh_field_at_parent_6c; // actual aligned14h, previous object dead
    const char* actual_index_key_00ce55b4;
    const volatile std::uint32_t& actual_crt_conversion_mode_0109eea4;
};

// Only [0087CF35,0087CF70), ordinary state17->18->17; not an original entrypoint.
// Borrow the SAME successful live Msh owner/scratch/S2C from this invocation.
// Keep Msh S18 and saved captures live17; this function never accesses/closes
// that string. The owner reference is a caller precondition, not a state check.
// The actual captured raw30h row stays valid/writable/alive across callbacks;
// do not reselect or repair it after returning invalid-parameter callbacks.
// Borrow the genuine live CE55B4 NUL-terminated key and actual mutable dword
// mode storage. Stable addresses/bindings are required; its VALUE is read by
// the genuine converter only after the genuine numeric getter has returned.
//
// Actual Lua owner/index/tracking/capacity/error-handler contracts apply. S6C,
// row, outer storage, keys/mode and private Source frames must be disjoint and
// aligned as required. No reentry/private-frame alias or binding mutation;
// preserve valid callback writes to actual row/header/mode storage.
//
// Real protected lookup returns before18. Direct B66270 may coerce numeric
// strings; do not replace it with exact-number-or. An assembly caller consumes
// the actual Source getter's ST0 return through BF7420 using the live DWORD
// mode. Store EAX bits to raw row+8 while18, lower17, then destroy actualS6C.
// An exceptional private guard lowers17 and destroys only armedS6C; no retry.
//
// Ordinary MSVC Win32 Source. Existing provider ABI spills remain qualified.
// No FP mode reset/mask/save, bool-mode substitute, numeric cast or float alias.
// Native register ABI, precision/status/trap/fault, FH3/SEH/longjmp, actual
// application binding, full parent and runtime proof remain held. Secondary
// guard cleanup failure follows ordinary noexcept termination.
void read_native_damageable_class_section_index_fragment_0087cf35(
    void* actual_selected_row,
    NativeDamageableClassSectionIndexFragmentScratch& actual_scratch);

} // namespace bsp
