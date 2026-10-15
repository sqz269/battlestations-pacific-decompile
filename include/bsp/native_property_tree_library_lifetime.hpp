#pragma once

#include "bsp/native_scene_property_recursive_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native property tree library lifetime requires MSVC Win32.
#endif

namespace bsp {

// Borrow two DISTINCT canonical owners from the same retained physical process:
// E17540 for outer library14h nodes; the existing bags context supplies E175B0
// for inner bag14h nodes and the same canonical raw string cells/current gate.
// Explicitly complete BOTH initialize_property_node_pool_once_00cc8a30() and
// initialize_property_library_node_pool_once_00cc8a70() before construction.
// Returned nonzero CRT registration status still leaves an initialized owner.
// This context starts nothing and accepts no arbitrary pool/list/callback.
class NativePropertyTreeLibraryLifetimeContext final {
public:
    NativePropertyTreeLibraryLifetimeContext(
        game::GameNativeStringProcess& actual_strings,
        game::GameNativePhysicalPoolProcess& actual_physical);

    NativeScenePropertyLifetimeContext& bags() noexcept { return bags_; }
    void* library_pool() const noexcept { return library_pool_; }

private:
    NativeScenePropertyLifetimeContext bags_;
    void* const library_pool_;
};

// Complete ordinary bodies with new C++ interfaces. Require an actual raw10Ch
// library/borrowed108h map, coherent64 heads/count, unique genuine live14h outer
// E17540 slots with intact allocator+10 membership and canonical pooled8h keys.
// Nonnull mapped values must be actual114h bags from admitted raw producers,
// satisfying the existing complete bag/record lifetime contracts. Owning graphs
// are finite, unshared and acyclic; outer node/key storage is disjoint from all
// mapped ownership. A numeric phase word alone does not establish provenance.
// Keep both pools/commonE188B4 list and string contexts live. Externally serialize
// shared-list mutations; separate startup mutexes do not provide that contract.
// Retire all bags, inner/outer nodes and keys BEFORE either pool tears down pages.
// No concurrent mutation/trim/reentry, aliasing, malformed slots, overflow,
// provider failure or fault; allow sufficient recursion stack. Respect the live
// raw-string return gate and the same pool generation as key publication.
//
// Phase DWORDs below remain Original DATA, not callable Source vtables. These
// APIs do not supply Original ECX/register/RET/FS/SEH transport, static dispatcher,
// class ABI, historical CRT, exceptional unwind, alias callbacks or game proof.

// Whole [008F4C50,008F4D4E):254B span,74 reachable instructions and3B unreachable
// alignment. Capture mapped+8 THEN next+C before retirement; nonnull bag scalar
// flags1, clear mapped after return, raw key return, SAME E17540 slot return.
// Advance saved next only, zero each head after its chain, then count+4 LAST.
// Preserve map phase+0; return the same actual map as observed in Original EAX.
void* clear_native_property_library_map_008f4c50(void* actual_map,
    NativePropertyTreeLibraryLifetimeContext& context);

// Whole [008F56A0,008F56F8):88B/22 instructions. Store root0=D1650C; clear the
// borrowed map=root+4; store SAME map0=D162C8; clear SAME map again unconditionally.
// This destroys contents only: no outer-root free or owner repair.
void destroy_native_property_tree_library_008f56a0(void* actual_library,
    NativePropertyTreeLibraryLifetimeContext& context);

} // namespace bsp
