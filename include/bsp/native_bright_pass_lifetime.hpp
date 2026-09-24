#pragma once
#include "bsp/native_render_effect_lifetime.hpp"

namespace bsp {
// Complete B101A0..B101BD, the D5E1B4 bright-pass profile's deleting slot.
// Original ECX actual owner, stacked flags byte, EAX original address, RET4.
// Destroy through the existing B0F5E0 implementation, then free only when
// flags&1. An escaping destruction exception never reaches the free.
// The explicit context changes this C++ interface's ABI. It borrows the same
// canonical children/counts and does not register a pass or admit teardown of
// an incompletely initialized resource graph.
void* delete_native_bright_pass_00b101a0(
    void* actual_owner, std::uint32_t flags, NativeRenderEffectLifetimeContext&);
} // namespace bsp
