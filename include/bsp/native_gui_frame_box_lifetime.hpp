#pragma once
#include <cstdint>

namespace bsp {
struct NativeGuiWidgetLifetimeContext;
struct NativeStringRawPoolContext;
class NativeRenderActualOwners;
class NativeGuiFrameBoxVectorTidyOperation;
struct GuiFrameBoxPoolTraits;
template<class Traits> class NativeGuiWidgetPool;

// Required borrowed services and ONE persistent prepared119 operation. No
// default service, owner, terminal, pool, graph retention or typed widget cast.
// Construct this context, the119 operation and its distinct fresh111 array
// outside the parent's failing scope, BEFORE entry/callbacks, and retain them
// independently with the SAME complete actual graph and candidate backing.
// Whole184 may prepare this context after its real vector returns, before its
// parent-private163 binding/current80 call. Preparing it does NOT admit invoking
//125 during that active constructor or on a failed/destructed partial Box.
struct NativeGuiFrameBoxLifetimeContext {
    NativeGuiWidgetLifetimeContext& widgets;
    NativeStringRawPoolContext& strings;
    NativeRenderActualOwners& owners;
    NativeGuiFrameBoxVectorTidyOperation& vector;
};

// Whole AD2880 [AD2880,AD28FD): native ECX actual11Ch FrameBox, no stacked
// args, RET; no semantic EAX result. This explicit Source ABI is not that
// binary/private-FH3 ABI. Require genuinely constructed, live DWORD-aligned
// actual base/container/name/vector storage, not a semantic projection or an
// unfinished/failed copy. Raw96 preserves proxyF0/other unwritten bytes: no
// blanket-zero prerequisite or initialization is introduced here.
//
// Same actual widget/node/current-profile/child/timed services, raw string
// publication01090AA8/gate01090AA4/manager01090AA0, and canonical render owners
// are required.111's ONLY established zero family is its genuine completed
// canonical registered2D creator with the matching registry/texture context/
// pool; failed103 records are excluded. No default terminal/provider is added.
//
// The SAME119 expected first/end correspondence MUST survive preceding name
// getter/return callbacks until reached normal/EH tidy captures begin/end.
// No general callback-replaced-vector selection or late metadata allocation.
// After capture119's cached-range/current-free rules apply. Independently hold
// all candidate backing/creators/providers and metadata BEFORE callbacks;
// container links and diagnostic pointers are not ownership or retry authority.
// Do not destroy entered/failed119 metadata on the parent's unwind path: its
// destructor explicitly terminates. No same-lifetime reentry/replay, even with
// another fresh operation. Public phase/range diagnostics are admission checks,
// not immutable one-shot or actual-allocation/provenance proof.
//
// Preflight checks the known payload, expected constructed range, context/119
// object and service-object/cell regions before raw effects; it reads no raw
// vector header or name.119's private111-array region/phases are not exposed,
// so its SAME fresh-array identity/full backing/provider/scratch disjointness
// remain caller proofs and its real tidy preflight still runs at that call.
// All Source objects/cells and reached raw storage have genuine live lifetimes.
// Context/metadata identities remain stable, disjoint from raw/freed backing
// and callee/private Original scratch, with externally serialized access.
//
// DATA108 before state1; nonnull CURRENT length104+1 ->419CC0/BD1510 without
// header clear. Consume state0 BEFORE119 normal tidy, state-1 BEFORE AA9730.
// Ordinary C++ unwind consumes1->0 before119 cleanupAD2320, then0->-1 before
// base; normal tidy failure does base ONLY. Cleanup escape terminates. Retain
// failed graph/frames outside this scope; no rollback/retain/raw rediscovery.
// Native FH3/compiler state placement/SEH/private aliases/CRT and complete
// populated-production ownership/activation are not proved by this interface.
void destroy_native_gui_frame_box_00ad2880(void* actual_frame_box,
    NativeGuiFrameBoxLifetimeContext&);

// Whole AD29B0 [AD29B0,AD29D0): native ECX actual FrameBox, flags DWORD on
// stack, EAX saved identity, RET4. Borrow a live aligned DWORD; read LOWBYTE
// only AFTER successful destructor. Preflight checks flags/pool metadata, not
// their values or pool state. On bit0 call the existing actual FrameBox pool
// engine on the SAME initialized pool/slab/trailer that allocated this slot.
// No early pool/trailer read, count gate/decrement or slot return on throw.
// Flags and all metadata survive destruction/slot return; full pool/storage/
// allocator lifetime and disjointness remain caller proofs. Return is identity
// only after possible return; do not dereference it or use after pool teardown.
void* scalar_delete_native_gui_frame_box_00ad29b0(void* actual_frame_box,
    const volatile std::uint32_t& current_flags,
    NativeGuiFrameBoxLifetimeContext&,
    NativeGuiWidgetPool<GuiFrameBoxPoolTraits>& actual_matching_pool);
} // namespace bsp
