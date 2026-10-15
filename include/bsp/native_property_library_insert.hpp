#pragma once

#include "bsp/native_property_tree_library_lifetime.hpp"

namespace bsp {

// Borrow the SAME concrete retained string process for both lookup fallbacks
// and pooled keys. The existing lifetime context captures DISTINCT E17540
// outer/E175B0 inner pools from the same concrete physical process/E188B4 list.
// Explicitly finish BOTH CC8A30 and CC8A70 startups before construction. This
// context starts nothing and accepts no arbitrary pool, callback or phase graph.
class NativePropertyLibraryInsertContext final {
public:
    NativePropertyLibraryInsertContext(game::GameNativeStringProcess& actual_strings,
        game::GameNativePhysicalPoolProcess& actual_physical);

    game::GameNativeStringProcess& strings() const noexcept { return strings_; }
    NativePropertyTreeLibraryLifetimeContext& lifetime() noexcept { return lifetime_; }

private:
    game::GameNativeStringProcess& strings_;
    NativePropertyTreeLibraryLifetimeContext lifetime_;
};

// Whole [008F29A0,008F2A4C):172B/65 instructions. Descriptive name is a
// hypothesis. New context-bearing C++ API, not Original ECX=actual108h map,
// stack=query8h/incoming word, EAX=count, RET8 transport or class/FS/SEH ABI.
// Borrow a genuine initialized raw10Ch library's map at+4, coherent64 heads/
// count and unique live14h E17540 nodes with intact allocator+10 membership.
// Query/key headers obey the raw lookup's stable closed NUL-free ASCII/current
// CRT C-locale contract; data may be null only when empty. Keep the SAME live
// canonical string pool/gate generation and both physical pools/list throughout.
// Externally serialize common-list mutations; no concurrent mutation/trim/reentry.
//
// Incoming null is allowed. A nonnull incoming root must be a genuine admitted
// same-current-heap114h bag (whole raw61 initializer/clone and accepted producers),
// owning a finite unshared acyclic graph disjoint from old ownership, map, node,
// key and query storage. A numeric phase DWORD alone establishes no provenance.
// Successful publication transfers incoming root/graph ownership into the map.
// Retire bag, then pooled key, then outer E17540 slot before pool teardown.
// Inherit the complete bag lifetime/lookup/resize/allocator contracts: successful
// providers, no overflow, invalid memory, faults, ownership alias or exceptional
// unwind. All providers outlive borrowers. No null-allocation recovery is added.
//
// HIT: retire nonnull old bag with flags1; physically clear mapped+8 only after
// return, publish incoming, physically reload count. No same-pointer shortcut.
// MISS: fresh slot zeros ONLY key0/4; preserve node==query branch, resize with
// preserve=true, reread query length then current destination length/data and
// source data; copy current destination length. Publish mapped, capture current
// bucket head, link next, publish head, increment count, then physically reload.
// Original ABI, historical/private CRT, loader/parser and game proof are external.
std::uint32_t insert_or_replace_native_property_library_entry_008f29a0(
    void* actual_map, const void* actual_query_header, void* incoming_bag,
    NativePropertyLibraryInsertContext& context);

} // namespace bsp
