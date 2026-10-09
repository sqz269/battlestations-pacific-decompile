#pragma once

#include <cstdint>

namespace bsp {

struct NativeObserverOwnerStorage;
class NativeObserverLifetime;
struct NativePendingEntityOwners;
class NativePendingEntityProducerAccess;

// Qualified Source composition of 007F1E70..007F1ED2[99]. The caller supplies
// one actual Task: base points to it, profile is its +0 DWORD, member_owner is
// its actual +20h observer prefix, and endpoint_cell is both Task+34h and that
// member's +14h pointer cell. These are binding conditions, not offset-derived
// casts or a new 38h task/member type. No copied owners or cells are substituted.
//
// Write opaque profile 00D08AE4 first, capture the volatile endpoint once, then
// optionally run the actual endpoint loop on that captured pointer. Member
// cleanup always follows and reads the actual endpoint cell afresh through its
// admitted Source interface. Base cleanup follows member cleanup normally.
// The endpoint prefix type alone does not supply the loop's wider raw backing;
// retain all receiver/entry storage required by that admitted child.
//
// Private armed guards cover member cleanup on endpoint-loop failure and base
// cleanup on endpoint-loop/member-cleanup failure. Each is disarmed BEFORE its
// normal child begins; no begun normal cleanup is retried. Their destructors
// are explicitly noexcept: cleanup failure during unwind terminates under
// current C++ policy, and later cleanup is not guaranteed after termination.
// This public function may throw. No Native FH3/FS/frame equivalence is implied.
//
// Retain the actual observer lifetime/domain, initialized shared pending owners,
// live producer mapping/providers, and actual pending registry/manager cells
// throughout reached calls and failure cleanup. Keep publication-cell addresses
// stable while their values may change. The base pointer and all actual storage
// identities must remain valid under the existing child contracts. No owner,
// allocation, fake member, callback fallback or production task binding is added.
// Source reference/stack/register/exception policies are explicitly qualified;
// no common EAX result, final profile or Original ABI/gameplay claim is supplied.
void cleanup_native_squadron_launch_task_007f1e70(
    void* actual_task_base,
    volatile std::uint32_t& actual_task_profile_00,
    NativeObserverOwnerStorage& actual_member_owner_20,
    NativeObserverOwnerStorage* volatile& actual_endpoint_cell_34,
    NativeObserverLifetime& retained_lifetime,
    NativePendingEntityOwners& actual_pending_owners,
    NativePendingEntityProducerAccess& actual_pending_access,
    void* volatile& pending_flag_publication,
    void* volatile& pending_manager_publication);

} // namespace bsp
