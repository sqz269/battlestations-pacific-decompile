#pragma once

namespace bsp {
class GeneratedModelLifetimeRuntime;

// Full AE1C20..AE1C5D[62]: original ECX actual entry, no public arguments,
// RET. Array data/count/capacity are entry+44/+48/+4C. EDX adds the borrowed
// SAME canonical lifetime runtime; one private word keeps it across calls.
void __fastcall release_native_shadow_entry_nodes_00ae1c20(
    void* actual_entry, GeneratedModelLifetimeRuntime* same_lifetimes);

// Finite actual NativeModelReference domain only. Every reached hierarchy key
// must have a stable binding in the same scene/lifetime runtime. Parentless
// node+A4 is null or an existing supported RenderNodeRootList C++ view; it is
// not an arbitrary raw native root-table pointer. Existing model/resource,
// child, point-light and terminal provider requirements remain in force.
//
// Data/count are freshly read each iteration. A nonnull node's exact array
// slot is captured BEFORE the branch and cleared AFTER normal unlink/release.
// Release can end model backing/companion lifetimes: only that captured array
// slot is touched afterward. Its backing and the entry must remain valid.
// Full AE19B0(0) runs after the loop, including when initial signed count<=0.
//
// Missing/wrong/dead bindings fail in a source-only adapter; no earlier native
// slot18 check is added. Invalid hierarchy in the existing noexcept provider
// may terminate. No rollback, native fault/FH3, exception traversal across
// naked frames, arbitrary stack-alias ABI or actual AE0A50 producer/game-domain
// closure is established. No owner, registry, retention or successful fallback
// is created. same_lifetimes is only used on a nonnull-node release path.
} // namespace bsp
