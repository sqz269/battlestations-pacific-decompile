#pragma once

#include "bsp/effect_admission.hpp"
#include "bsp/native_render_context.hpp"
#include "bsp/native_render_pointer_arrays.hpp"

namespace bsp {

// Borrow these actual fields of ONE native item. The arrays contain raw native
// identities, not RenderCommandReference companion pointers. The application
// must supply an established same-owner mapping; semantic PointEffectInstance-
// Storage arrays cannot be reinterpreted as this raw header specialization.
// No owner, array allocation, count or copied flag is created by this view.
struct NativeWreckEffectItemView {
    NativeRenderPointerArrayStorage& entries_0c;
    NativeRenderPointerArrayStorage& auxiliary_18;
    volatile std::uint8_t& field_09;
};

struct NativeWreckEffectItemCleanupContext {
    EffectManager* volatile& lock_publication_00f87650;
    EffectManagerLifetimeAccess& lock_lifetime;
    NativeRenderActualOwners& actual_references;
};

// Complete008673B0..0086749D normal sequence: ECX actual item, no stack args,
// plain RET, no defined result. Acquire existing866440 owner and capture its
// actual+04 section. Null section is allowed; null manager is not ignored.
// Capture child data then count once, release each captured nonnull owner,
// clear that cell after callback, then unconditionally clear the cell again.
// Capture auxiliary data then count; repeatedly erase at that fixed position
// using byte-identical existing81B010/867210 behavior, reloading count then data
// for each end test. Write actual byte09=1, then leave the captured section.
void cleanup_native_wreck_effect_item_008673b0(NativeWreckEffectItemView,
    NativeWreckEffectItemCleanupContext&);

// Existing canonical getter, array and atomic reference providers are reused.
// Raw spans/iterator storage must survive their native final accesses. Current
// virtual0 terminal dispatch is required through actual_references, including
// exact borrowed+04 validation; there is no successful unknown-owner fallback.
// C++ failures release the captured section and preserve prior partial stores.
// Original FH3/SEH transport, arbitrary native throwing destructors, concurrent
// pointer mutation and installed gameplay binding are outside this source ABI.
} // namespace bsp
