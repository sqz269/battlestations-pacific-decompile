#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native render service base requires MSVC Win32.
#endif

namespace bsp {

// Stable bindings to actual raw publication cells; their values remain live.
// The manager has its original vector fields and section pointer at +10h.
struct NativeRenderServiceBaseContext {
    void* volatile& actual_manager_01090aa0;
    void* volatile& actual_service_00f8d39c;
};

// Complete B0F020[145]: ECX actual receiver, no stack arguments, EAX receiver,
// RET. Publish/register under the first manager's captured section. The return
// is the original receiver even if a current provider changes publication.
void* publish_native_render_service_base_00b0f020(
    void* actual_receiver, NativeRenderServiceBaseContext& context);

// Complete B0F0C0[153]: ECX actual receiver, no stack arguments, RET, void.
// Unregister the CURRENT publication, then clear it only after success. Both
// normal completion and C++ unwind reset the original receiver's base profile.
void destroy_native_render_service_base_00b0f0c0(
    void* actual_receiver, NativeRenderServiceBaseContext& context);

// Complete 412430[7]: ECX receiver, no arguments, RET. One profile DWORD store.
void destroy_native_generic_singleton_base_00412430(void* actual_receiver) noexcept;

// Raw-domain overloads of the existing typed getter entries. LEA EAX,[ECX+84]
// and LEA EAX,[ECX+1D8], respectively; no memory reads or typed service overlay.
// Each native body is seven bytes, no stack arguments, plain RET.
const void* render_time_region_00b0cf30(const void* actual_service) noexcept;
const void* get_system_camera_service_matrix_00b0d100(
    const void* actual_service) noexcept;

// This source API adds context arguments; it is not a drop-in original ABI.
// Actual raw manager, registration/removal, guard and Win32 section providers
// are required. Numeric profile DWORDs retain identity, not callable host
// vtables. First-enter failures arm only base cleanup; state1 unwind runs full
// 411EE0 before 412430. A second escaping MSVC C++ cleanup exception terminates.
// Private original EH spills, hardware-fault cleanup, invalid storage, original
// CRT exception identity and concurrent access are outside the proved domain.
// No service initialization, full derived destruction or game validation claim.
} // namespace bsp

namespace bsp {
// Dormant BEA810 singleton-base interface over borrowed actual Application
// storage. Both bindings are fixed, distinct actual cells, disjoint from the
// receiver, context and private guard. Their values remain live. The manager
// authority must be the GameNativeStringProcess/GameSingletonHost domain.
struct NativeApplicationSingletonBaseContext {
    void* volatile& actual_manager_01090aa0;
    void* volatile& actual_application_e1ae90;
};

// BEA810 singleton-base body: original ECX receiver, no stack arguments, EAX receiver,
// RET. This explicit-context Source API is not the original binary ABI.
// Caller retains stable aligned actual storage, valid uint32 access at +0 and
// the enclosing 1Ch preimages/domains. No direct stores affect +04..+1B. Actual
// manager results/vector, captured native section and its depth DWORD at +18
// remain valid through providers/cleanup; no null-manager fallback, EH-stack
// aliases, concurrent invalidation or SystemSingletonCriticalSection projection.
// State0 precedes D68BC4; state1 follows Enter/current-depth increment and stays
// armed through normal Leave. C++ escape uses genuine 411EE0 then 412430 only
// if guard cleanup returns. A second escaping MSVC C++ cleanup exception takes
// the existing termination boundary; original FH3/hardware faults are unproved.
// Normal-Leave escape may decrement/Leave again, with no safe disposition claim.
// Publish original receiver unconditionally; second getter precedes selecting
// current E1AE90 for BD0C30 (including null). Return original receiver after
// successful release. Publication/registration effects persist on failure:
// no clear, restore, unregister, free, retry, retain or counter compensation.
// This creates no owner, actual-cell producer, outer constructor/destructor,
// callable profile, startup activation, +14 lifetime or E1AE78 retirement right.
void* construct_native_application_singleton_base_00bea810(
    void* actual_receiver, NativeApplicationSingletonBaseContext& context);
} // namespace bsp
