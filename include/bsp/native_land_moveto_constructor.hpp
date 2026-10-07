#pragma once

#include "bsp/native_land_state_lifetime.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {

// Borrow the actual >=3Ch MoveTo receiver. The three binary32 fields are
// transported as raw DWORDs, exactly like the native MOVSS load/store pairs;
// no numeric conversion, arithmetic or semantic interpretation is supplied.
struct NativeLandMoveToConstructorView {
    void* actual_root;
    NativeLandStateCleanupView state;
    const void* volatile& owner_04;
    std::uint32_t volatile& word_08;
    NativeObserverOwnerStorage& callback_18;
    std::uint8_t volatile& byte_28;
    NativeObserverOwnerStorage* volatile& first_endpoint_2c;
    std::uint32_t volatile& word_30;
    std::uint32_t volatile& word_34;
    std::uint32_t volatile& word_38;
};

// PURE extent/address validation only: no represented value loads, callbacks,
// native calls, allocation, profile construction or lifetime extension.
// A mismatch reports a SOURCE admission error, not a native fallback.
NativeLandMoveToConstructorView native_land_moveto_constructor_view(
    void* actual_root, std::size_t backing_bytes,
    const NativeLandStateCleanupView&, const void* volatile& owner_04,
    std::uint32_t volatile& word_08, NativeObserverOwnerStorage& callback_18,
    std::uint8_t volatile& byte_28,
    NativeObserverOwnerStorage* volatile& first_endpoint_2c,
    std::uint32_t volatile& word_30, std::uint32_t volatile& word_34,
    std::uint32_t volatile& word_38);

// PURE same-root adoption into complete007B65E0; callback18 then shared state.
NativeLandMoveToCleanupView native_land_moveto_cleanup_view(
    const NativeLandMoveToConstructorView&) noexcept;

// COMPLETE009C2AC0..009C2B64,164B/44instructions, new SOURCE interface.
// Original ECX=root, stack(owner,FIRST,three binary32 words), EAX=root/RET14.
// The historical third-word "int mode" name remains provisional: native
// MOVSS copies +38 unchanged; no semantic mode or float-argument ABI claim.
// D20AEC/D20AD4/CE3CD4 are RAW UNCALLABLE image identities, not SOURCE tables.
// Use direct recovered operations only; other class/profile methods are unbound.
//
// Required ordinary domain: stable actual live same-root storage and copied
// arguments, successful fresh placement, coherent live FIRST endpoint when
// nonnull, genuine NativeObserverLifetime with actual published manager/lock/
// dispatch/CRT context and valid allocation ranges. No existing live resources
// may be overwritten by this constructor. It copies owner identity without
// dereferencing it; later consumers require their own owner/lifetime domain.
// No default world/profile/arena, concurrent/structural reentry, invalid storage,
// allocation failure, rollback, private EH, faults, original ABI or game binding.
void* construct_native_land_moveto_009c2ac0(
    const NativeLandMoveToConstructorView&, const void* owner,
    NativeObserverOwnerStorage* first_endpoint, std::uint32_t word_30,
    std::uint32_t word_34, std::uint32_t word_38, NativeObserverLifetime&);

} // namespace bsp
