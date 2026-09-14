#pragma once
#include "bsp/native_frame_target_owner.hpp"
#include <cstdint>

namespace bsp {
using NativePostEffectFrameAtomic = long (__stdcall*)(volatile long*);
struct NativePostEffectFrameBindingContext {
    NativeFrameTargetOwnerContext& frame_targets;
    NativePostEffectFrameAtomic const volatile& actual_increment_00ce221c;
    NativePostEffectFrameAtomic const volatile& actual_decrement_00ce2220;
    const volatile std::uint32_t* actual_frame_profile_00d5e600;
};

// B4E3D0..B4E40A: ECX receiver, one stacked incoming frame pointer, RET4;
// saves ESI, no specified result. Capture +08 and skip identical pointers.
// Publish before retaining incoming; then decrement the captured old pointer.
// Each atomic IAT cell is read only at its own original call, so the decrement
// target is fresh after incoming retention. At zero, dispatch current old
// profile virtual0 BD30E0 and its fresh deleting slot B1FCF0 with flags1.
// This source uses the existing concrete D5E600 frame-owner provider/domain.
// It adds no reference companion, shadow count, clear, rollback or catch.
// Valid actual storage, live native counters, actual callable IAT cells and
// the same frame/surface allocation domains are caller prerequisites. Native
// hardware faults/private-frame aliases and drop-in ABI are not established.
void set_native_post_effect_frame_target_00b4e3d0(void* actual_receiver,
    void* incoming, NativePostEffectFrameBindingContext&);

// B4CC00..B4CC03: ECX actual24h receiver, EAX current DWORD+20, RET.
// A raw count accessor with no validation, normalization or ownership effect.
// The one-input fastcall leaf uses the original physical ECX/EAX/RET behavior;
// broader interfaces and whole-program ABI compatibility remain unclaimed.
std::uint32_t __fastcall get_native_post_effect_count_00b4cc00(
    const void* actual_receiver) noexcept;
} // namespace bsp
