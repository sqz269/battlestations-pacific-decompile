#pragma once
#include "bsp/native_gui_widget_model_clone.hpp"

namespace bsp {
struct NativeGuiWidgetCopyContext {
    NativeGuiWidgetModelCloneContext& model_clone;
    const volatile std::uint32_t (&clone_flags_00d5c0b8)[19];
};
struct NativeGuiWidgetCopyAcquired {
    bool started{};
    bool complete{};
    std::uint32_t active_call_site{};
    std::int32_t native_eh_state{-1};
    NativeGuiWidgetModelCloneAcquired model_clone;
};

// Complete normal AA9520 for current source4C selecting actual Model B752B0,
// flags26/3E,parent0. Fresh aligned destination covers at least E4 bytes and
// must not overlap the live source. Current source60 must index the actual
// 19-DWORD flags table; current source4C must be nonnull and canonically bound.
// All contexts, source storage and live profile/flag cells outlive callbacks.
//
// Copies through actual storage, preserving every unwritten byte. Float copies
// perform x87 FLD/FSTP, including signalling-NaN conversion. Sentinel allocation
// precedes the late source node/type/clone reads. No parent/child/source retain.
// Successful clone creator is transferred directly to destination4C, then its
// current138 flags lose bits0/1. The acquisition frame is not another owner.
//
// C++ failure cleanup projects native states0/2: ref base, or timed/list/ref
// base. A completed model whose later clone work fails is NOT rolled back;
// its actual acquired creators remain in the caller frame for explicit recovery.
// Native FH3/SEH and binary ABI are not claimed. Frame is fresh and one-shot.
void* construct_native_gui_widget_copy_00aa9520(void* destination,
    const void* source, NativeGuiWidgetCopyContext&, NativeGuiWidgetCopyAcquired&);
} // namespace bsp
