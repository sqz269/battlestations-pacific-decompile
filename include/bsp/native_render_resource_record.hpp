#pragma once

#include "bsp/storage_pool.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {

struct SingletonLifetimeCallbacks;
class ActualNativeStringPoolStorage;

struct NativeRenderResourceAliasNode {
    NativeRenderResourceAliasNode* next_00;
    NativeRenderResourceAliasNode* previous_04;
    std::uint32_t string_length_08;
    char* string_data_0c;
};

// Actual Win32 2Ch record storage used by renderer+1A74. The name and alias
// nodes retain their real allocations. This storage destructor establishes
// no ownership operation for resource_28 and does not call any resource slot.
struct NativeRenderResourceRecord {
    std::uint32_t name_length_00;
    char* name_data_04;
    std::uint32_t unknown_08;
    NativeRenderResourceAliasNode* sentinel_0c;
    std::uint32_t alias_count_10;
    std::uint32_t payload_14_24[5];
    void* resource_28;
};

// Complete 004D05E0..004D0632. Native ABI: ECX actual list owner, RET0,
// no semantic return. For this record the owner is record+8: its preserved
// word is +0, actual sentinel pointer +4, and count +8. No copied list header.
// Detach the sentinel, zero count, then release each captured node string and
// node; compare captured next against the current sentinel after callbacks.
// Retain the sentinel allocation and all fields outside those list links/count.
// The caller supplies the actual 00419CC0 pool; nodes use lifetime malloc/free.
void clear_native_render_resource_aliases_004d05e0(
    void* actual_list_owner, SizedStoragePool& actual_string_pool);
// Actual owning-pool overload: every string operation uses the current 419CC0
// publication/gate/lifetime binding. Same native algorithm and exception limits.
void clear_native_render_resource_aliases_004d05e0(
    void* actual_list_owner, ActualNativeStringPoolStorage& actual_string_pool);

// Complete 00B30510..00B305B2. Native ECX destination, stack source, RET4,
// returns destination. Resize/copy the actual name, capture source first/end
// before clearing destination aliases, then insert before the current first
// destination node. Exact self-assignment skips name/list operations but still
// executes six sequential tail-field stores. No resource retain/release call.
// Uses the existing pool and invalid-parameter callback/exception domains.
// Evidence and host boundaries: docs/NATIVE_RENDER_RESOURCE_RECORD_ASSIGNMENT.md.
NativeRenderResourceRecord& assign_native_render_resource_record_00b30510(
    NativeRenderResourceRecord& destination,
    const NativeRenderResourceRecord& source, SizedStoragePool& actual_string_pool,
    const SingletonLifetimeCallbacks&);

// Complete 00B2F990..00B2FA07, including the tail hidden by false _free
// no-return analysis. Native ABI: ECX actual record, RET0, no semantic return.
// The caller supplies the shared 00419CC0 string-pool domain used to allocate
// these buffers. Nodes/sentinel use the native lifetime malloc/free domain.
// A valid actual sentinel is required, including for an empty list.
//
// Clear/free aliases, free/null sentinel, then return name data with length+1.
// Leave name fields, unknown_08, payload and resource_28 unchanged. Do not free
// the record itself. State-0 unwind releases its name while clearing the list;
// that cleanup is disarmed before normal name release. This typed interface
// does not reproduce native SEH registration or binary exception ABI.
void destroy_native_render_resource_record_00b2f990(
    NativeRenderResourceRecord&, SizedStoragePool& actual_string_pool);

}
