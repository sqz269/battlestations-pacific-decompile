#pragma once

#include <cstdint>

namespace bsp {
struct NativePhysicalFileDateContext;
struct SingletonLifetimeCallbacks;

// The stack visitor retains original identity D683B0. Bind the application's
// actual three-DWORD profile storage at that identity; no synthesized table or
// callback interface. The bridge rereads +4/+8 at each reached dispatch site.
struct NativeVfsDateRouteContext {
    NativePhysicalFileDateContext& physical;
    const void* actual_date_profile_00d683b0;
};

// Full BD97E0: ECX actual {owner,node} iterator, RET; no specified result.
// Mount node links +0/+4/+8 and nil byte+21. Returning CRT sites continue;
// nil-node advance tail-calls CRT and returns if that handler returns.
void advance_native_vfs_mount_iterator_00bd97e0(void* actual_iterator,
    const SingletonLifetimeCallbacks&);

// Full BD90B0: ECX visitor, RET. Writes original base identity D68380 only.
void reset_native_vfs_date_visitor_00bd90b0(void* actual_visitor) noexcept;
// Full BD9F00: ECX visitor, RET, EAX0/1. Tests +4/+8/+C/+10/+14 in order.
bool has_native_vfs_date_visitor_result_00bd9f00(const void* actual_visitor) noexcept;
// Full BD9E80: ECX visitor, stack payload/name, RET8. No specified result.
// Payload+8 is provider. Dispatch current provider+20, then copy ascending
// from its returned pointer, not an assumed hidden-buffer identity.
void read_native_vfs_date_provider_00bd9e80(void* actual_visitor,
    const void* actual_mount_payload, const void* actual_name_header,
    NativeVfsDateRouteContext&);

// Full 672-byte BDD0A0 traversal, QUALIFIED to the D683B0 date visitor route.
// ECX manager, stack name/visitor, RET8, no specified result. Manager+18=-1;
// tree+3C, current head+40, start head.left. Mount payload=node+10. Visit in
// existing tree order, retaining no provider and constructing no nodes.
// Other visitor identities/slots and unknown provider date slots are explicit
// std::invalid_argument SOURCE boundaries, not recovered native failures.
void visit_native_vfs_date_mounts_00bdd0a0(void* actual_manager,
    const void* actual_name_header, void* actual_date_visitor,
    NativeVfsDateRouteContext&);

// Full BDD340: ECX captured manager; stack output/name; EAX output; RET8.
// Constructs a D683B0 stack visitor, copies/normalizes the name, traverses that
// passed manager, copies its result, then frees the copied name. Physical date
// dispatch independently reads context.physical.manager_0109CEEC at invocation.
// Native raw headers/tree/profile storage and the actual owning pool are required.
// This C++ interface is not a binary ABI or native SEH replacement.
void* query_native_vfs_file_date_00bdd340(void* actual_manager,
    void* actual_date_output, const void* actual_name_header,
    NativeVfsDateRouteContext&);
} // namespace bsp
