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
// This entry uses a fenced four-byte copy into caller-provided raw backing;
// no C++ uint32_t field object is required. The fence adds no synchronization.
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

namespace bsp {
// Dormant borrowed BEA970 loop-base shell; descriptive name, original ECX
// receiver/no stack arguments/EAX original receiver/RET0. This explicit-context
// Source API is not the original binary ABI. Retain stable aligned actual 1Ch
// enclosing storage, valid uint32 object/access at +0, BYTE access at +04, and
// all BEA810 actual-cell/shared-manager/section/provider/disjointness/lifetime
// prerequisites. Preserve every other incoming byte and owner domain.
// Call genuine BEA810 once on the original receiver with the same context;
// ignore its return value. Only after normal return store DWORD D68BC8 at +0,
// then exactly BYTE zero at +04, and return the original receiver.
// Child escape/nonreturn reaches neither store. Reuse its ordinary C++ cleanup
// unchanged, including conditional null-section handling, current-depth and
// captured-section roles, normal-Leave uncertainty and second-cleanup termination.
// Publication/registration occur under D68BC4 before these stores. Escaped
// references retain their lifetime obligations; CE3818 reset can reach the
// generic flags1 drain/free path and is not storage-return authority.
// No owner/cell producer, full 737970, caller, activation, callable profile,
// outer cleanup, rollback, retain, free, retirement or storage-return policy.
// Hardware faults, Original FH3/private frame and external ABI remain unproved.
void* construct_native_application_loop_base_00bea970(
    void* actual_receiver, NativeApplicationSingletonBaseContext& context);
} // namespace bsp

namespace bsp {
// Dormant borrowed 737970 full shell; descriptive name, Original ECX receiver,
// no stack arguments, EAX original receiver and RET0. This explicit-context
// Source API is not the Original binary ABI. Entry requires stable aligned
// actual enclosing 1Ch storage with already-live uint32 profile at +0, actual
// NativeApplicationPointerVectorStorage at +08 (live void** data and signed
// int32 count/capacity members), and a distinct live void* Game object at +14.
// These objects and valid byte access must exist before the child call; this
// helper starts no lifetime. Prior ownership must permit overwriting their
// preimages without freeing or resetting them. Retain all BEA970/BEA810 actual
// cell, shared-manager, provider, captured-section, current-depth, disjointness,
// reentrancy and escaped-reference lifetime prerequisites. The new outer stores
// touch only the specified fields, preserving +05..07 and +1B themselves;
// genuine child/provider effects remain governed by the inherited contract.
// Call genuine BEA970 once on the captured original receiver and same context,
// ignore its result, then after normal return store CFEAB0 at +0, typed null
// data at +08, signed zero count/capacity at +0C/+10, typed null Game at +14,
// and bytes +19=0, +1A=0, +18=1, in that order; return the original receiver.
// Child escape/nonreturn reaches no outer store. Its ordinary C++ cleanup,
// null-section branch, Leave uncertainty and second-cleanup termination stay
// unchanged. Escaped publication/registry references may survive; CE3818 reset
// can reach generic flags1 drain/free and grants no storage-return authority.
// No owner/cell producer, caller, activation, callable profile, outer cleanup,
// rollback, retain, free, retirement or storage-return policy. Volatile is not
// synchronization; invalid storage, hardware/foreign exceptions and Original
// FH3/private-frame behavior remain outside this Source contract.
void* construct_native_application_00737970(
    void* actual_receiver, NativeApplicationSingletonBaseContext& context);
} // namespace bsp

namespace bsp {
// Dormant borrowed BEA8B0 ordinary Application singleton-base retirement;
// descriptive name. Original ECX receiver, no stack arguments, RET0, no
// semantic EAX result. This explicit-context Source API is not Original ABI.
// Caller keeps stable aligned actual enclosing 1Ch storage, a live uint32
// profile object/access at +0 and valid preimages/byte domains. Borrow fixed
// distinct actual manager/Application cells in the GameNativeStringProcess /
// GameSingletonHost shared-manager domain, disjoint from receiver, context,
// section and private guard. Both manager results/vectors, captured native
// section, its current depth DWORD+18 and providers remain valid throughout
// normal/escaped execution. No null-manager fallback, projection lock,
// concurrent invalidation, EH-stack alias or invented object lifetime.
// Store D68BC4 at receiver+0, then arm state0. First getter precedes unguarded
// section capture. Arm state1 after conditional Enter/current-depth increment,
// including null section. Second getter precedes loading current E1AE90; pass
// that pointer (even null or different from receiver) to genuine BCFCA0, then
// unconditionally clear the current publication with literal zero. Decrement
// current depth and Leave the first captured section before CE3818 receiver
// reset. M1/M2 and receiver/current publication may differ; none is cached,
// compared, substituted, restored or compensated.
// Reuse existing ordinary C++ cleanup: genuine 411EE0 then 412430 only if
// guard cleanup returns. Guard stays armed through normal Leave; Leave escape
// can decrement/Leave again. A second escaping MSVC C++ cleanup exception takes
// the existing termination boundary. No safe cleanup, rollback, owner, free,
// payload cleanup, quiescence, storage return, callable table or activation.
// Candidate classic-x86 map/actions are conditional evidence only; Original
// FH3/frame/invocation, hardware/foreign faults and external ABI are excluded.
void retire_native_application_singleton_base_00bea8b0(
    void* actual_receiver, NativeApplicationSingletonBaseContext& context);
} // namespace bsp

namespace bsp {
// Dormant borrowed BEA990 derived Application-base retirement; descriptive name.
// Original ECX DWORD store D68BC8, then physical tail JMP BEA8B0; no own RET or
// semantic EAX, and ESP/registers/arithmetic flags unchanged before the child.
// This explicit-context void C++ call does not preserve that tail-transfer ABI.
// Borrow the same actual stable aligned uint32 receiver+0, valid1C preimages /
// access domains, fixed actual cells and disjoint manager/vector/section/depth /
// guard/provider domains and lifetimes required by BEA8B0. Inherit its qualified
// ordinary C++ cleanup, repeated decrement/Leave and termination hazards.
// No local RAII, invented owner/callable table/activation, free/payload cleanup,
// outer7379A0 or scalar-delete authority. Original FH3/frame/invocation,
// hardware/foreign exceptions and external ABI remain excluded.
void retire_native_application_base_00bea990(
    void* actual_receiver, NativeApplicationSingletonBaseContext& context);
} // namespace bsp

namespace bsp {

struct NativeApplicationPointerVectorStorage;
struct NativeApplicationPointerVectorAllocation;

// Dormant explicit-context reconstruction of 007379A0 (93 bytes, 28 instructions).
// Borrow actual already-live 0Ch rows at actual_receiver+08; no overlay creates
// their lifetime. The same genuine allocation provider and all receiver, row,
// fixed-cell, manager, section and provider domains must remain valid through
// normal retirement and C++ cleanup. Signed/live resize(0) is not a universal clear.
// Reload current row data after resize, array-free that data, then retire the
// same receiver through BEA990. No outer free, field reset or element retirement.
// Escaping C++ resize/free invokes BEA990 under the existing cleanup termination
// policy. Original FH3/frame, hardware/foreign exceptions and ECX/tail-transfer
// ABI are not authenticated by this C++ interface. No owner or activation.
void retire_native_application_007379a0(
    void* actual_receiver,
    NativeApplicationPointerVectorStorage& actual_rows,
    const NativeApplicationPointerVectorAllocation& allocation,
    NativeApplicationSingletonBaseContext& context);

} // namespace bsp

#include <cstdint>

namespace bsp {

// Dormant descriptive reconstruction of 00737E00 (30 bytes, 11 instructions).
// Qualified C++ captured-receiver model: genuine parent once, then one actual
// late volatile selector byte read; bit0 selects the explicit BF65AC provider.
// All accepted parent A+8 row/context/allocation domains and C++ cleanup remain.
// Selector must remain live through its post-parent read. Selected outer free
// requires actual A at a compatible live allocation START in the supplied host
// CRT family; BF6989 data free is distinct. Return is an address, not ownership.
// Original TEST uses current SS:[post-parent ESP+8] and MOV EAX uses current ESI;
// entry-frame/captured-A interpretations require callee balance/preservation.
// No default heap, owner, overlay, table, activation or Original ABI is supplied.
void* retire_native_application_scalar_00737e00(
    void* actual_receiver,
    NativeApplicationPointerVectorStorage& actual_rows,
    const NativeApplicationPointerVectorAllocation& allocation,
    NativeApplicationSingletonBaseContext& context,
    const volatile std::uint8_t& actual_selector,
    void (*free_object_00bf65ac)(void*));

} // namespace bsp
