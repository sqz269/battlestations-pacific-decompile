#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native shadow-job lifetime requires MSVC Win32.
#endif

namespace bsp {

// Borrow the application's SAME raw manager/job publication cells through
// singleton drain. The actual allocation is8bytes: primary+0 D5B570 and
// registered secondary+4 D5B56C. This context owns no object or registry.
struct NativeShadowJobContext {
    void* volatile& actual_manager_publication_01090aa0;
    void* volatile& actual_job_publication_00e18ad4;
};

// Complete A8E090..A8E15D[206]. Native no public inputs, EAX primary, RET;
// this source interface adds context. Captured fast return, guarded second
// check, transient/final table stores and nullable secondary registration
// argument before the second manager lookup, then fresh post-leave result.
// The sole native EH action releases the captured guard, with no rollback of
// allocation/publication. Original FH3/SEH and arbitrary IAT replacement are
// not reproduced. Job execution at primary slot0/A8AE50 remains separate.
void* get_native_shadow_job_00a8e090(NativeShadowJobContext&);

// Complete A8DDE0..A8DE13[52]. Native ECX primary, stack flags, EAX primary,
// RET4. EDX now supplies context. Capture flags bit0 BEFORE clearing the
// actual publication and stamping secondary CE3818; conditionally free the
// actual primary. No preceding destructor callback. Null primary retains
// the native invalid-null store; no successful null/identity fallback.
void* __fastcall delete_native_shadow_job_00a8dde0(
    void* actual_primary, NativeShadowJobContext&, std::uint32_t flags) noexcept;

// Complete A8DDB0..A8DDB7[8]: SUB ECX,4 then tail JMP to the terminal above.
// EDX context and the original public flags stack word pass through unchanged.
// Only final registered D5B56C is admitted by the existing singleton map;
// transient D5B568 and primary job execution are not deleting substitutes.
void* __fastcall delete_native_shadow_job_secondary_00a8ddb0(
    void* actual_secondary, NativeShadowJobContext&, std::uint32_t flags) noexcept;

} // namespace bsp
