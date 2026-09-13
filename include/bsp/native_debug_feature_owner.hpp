#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native debug-feature owner storage requires MSVC Win32.
#endif

namespace bsp {
class NativeStringStorage;

// Stable borrowed cells in the application's existing raw lifetime domain.
// strings must use the same actual pool publication/return gate as other
// native owners (ActualNativeStringPoolStorage); this context owns no service.
struct NativeDebugFeatureOwnerContext {
    void* volatile& actual_manager_publication_01090aa0;
    void* volatile& actual_owner_publication_0109db70;
    NativeStringStorage& strings;
};

// 51F460[189]: native cdecl(), EAX owner, RET. The slow path captures the
// first manager's raw section, enters/increments, rechecks, constructs raw34h,
// publishes, gets the manager again and reloads the owner for BD0C30. A failed
// registration retains publication; cleanup releases only the captured guard.
// Fast return is captured. Slow return reloads publication after section leave.
void* get_native_debug_feature_owner_0051f460(NativeDebugFeatureOwnerContext&);

// BE94F0[109]: ECX raw34h, EAX input, RET. D68B94 at+00; string-array
// data/count/capacity at+04/+08/+0C; empty native8h string at+10; feature-record
// data/count/capacity at+18/+1C/+20; bytes+24/+2C; words+28/+30. All named
// fields except the profile are zero. Bytes+25..27 and+2D..2F are untouched.
// Constructor failure unwinds the +04 array then the publication/base state.
void* construct_native_debug_feature_owner_00be94f0(
    void* actual_owner, NativeDebugFeatureOwnerContext&);

// BE8210[17]: ECX owner, RET. Unconditionally clear the current0109DB70
// publication before stamping the actual owner with base profile CE3818.
void destroy_native_debug_feature_owner_base_00be8210(
    void* actual_owner, NativeDebugFeatureOwnerContext&) noexcept;

// BE8350[1]: native RET with no reads/writes, including no EAX write. This
// source no-effect entry does not reproduce the original register-level ABI.
void native_debug_feature_shutdown_hook_00be8350(void* actual_owner) noexcept;

// BE8DF0[308]/BE8F30[154]: ECX actual0Ch {data,signed count,signed capacity},
// signed capacity/count on stack, RET4. Records are88h: an8h pooled string
// followed by80h flag bytes. Reserve clamps to1, deep-copies strings and copies
// flags as forward DWORDs, releases old strings forward, frees current backing,
// then publishes. Its native unwind calls a bare RET: no copy/backing rollback.
// Resize clears new records and shrinks backward, decrementing live count
// BEFORE the release. Preserve current headers around callback boundaries.
void reserve_native_debug_feature_records_00be8df0(
    void* actual_vector, std::int32_t capacity, NativeStringStorage&);
void resize_native_debug_feature_records_00be8f30(
    void* actual_vector, std::int32_t count, NativeStringStorage&);

// BE9560[156], including the disk tail beyond the old Ghidra free boundary:
// ECX owner, RET. Resize+18 to0/free backing; release+10 string; resize+04
// to0/free backing; clear publication and install CE3818. On a C++ exception,
// the current native cleanup state drains remaining members then base state.
void destroy_native_debug_feature_owner_00be9560(
    void* actual_owner, NativeDebugFeatureOwnerContext&);

// BE9600[30]: ECX unadjusted owner, stacked flags, EAX captured input, RET4.
// D68B94 slot0. Delete only after successful destruction and only if flags&1.
void* delete_native_debug_feature_owner_00be9600(
    void* actual_owner, std::uint32_t flags, NativeDebugFeatureOwnerContext&);

// Names are hypotheses: earlier FileStoreService naming is unsupported.
// Container helpers are provisional game-container specializations, not a
// recovered template/STL identity. Original numeric profiles are data tokens,
// not callable C++ vtables. FH3/SEH maps, hardware faults, mutable EH spills,
// nested cleanup failure and source CRT/register identities remain boundaries.
// Parser bootstrap, central profile dispatch and game reachability are separate.
} // namespace bsp
