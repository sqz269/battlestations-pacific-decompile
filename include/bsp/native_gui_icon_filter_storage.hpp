#pragma once
#include "bsp/native_gui_icon_size_comparison_storage.hpp"

namespace bsp {
class NativePlatformLoadPublication;
// Ordinary caller-retained homes only, with no initialization/status/owner.
// The complete sign-extended EC word is staged into the actual DWORD argument
// HOME for Raw1230 only on its reached Native PUSH path. Its returned state is
// consumed by the distinct nested Raw24 Scratch. No typed state/vector family
// or fabricated actual Icon/reference is created.
struct NativeGuiIconFilterScratch {
    volatile std::uint32_t selected_index_home;
    NativeGuiIconSizeComparisonScratch size_comparison;
};
static_assert(sizeof(NativeGuiIconFilterScratch) == 28);

// Whole AB2600..AB2679 normal schedule,122B/35ops: ECXactual138BIcon, no incoming
// stack arguments, full EAX0or1/plainRET. This eight-argument ordinary Source
// interface changes that ABI. Names are existing Source hypotheses only.
//
// First CURRENT byte100==0 or CURRENT DWORD104!=0 returns0 without loading the
// platform publication. Otherwise load the SAME authoritative whole-platform
// pointer cell once through its genuine existing transport, then read actual
// platform+14 late. A null pointer has no default/fallback. This must be the
// whole-platform publication, never a projected focus-domain receiver.
//
// UCOMISS/LAHF/TEST AH,44h/JP retains the actual zero/one, signed-zero and NaN
// decisions. CURRENT rotation48/zero, then CURRENT scale28/CURRENT one, then
// CURRENT scale2C/same captured one are read in Native order. Only on that path
// MOVSX reads CURRENT EC WORD, stages the FULL DWORD, then Raw1230 consumes the
// actual F0 vector and HOME. Always call complete Raw24 on the returned actual
// state, even after a returning invalid-parameter handler. Test AL, not allEAX,
// and normalize the final full Source DWORD0/1. No semantic GuiIconWidget,
// injected platform/difference bool, caller filter/count clamp or callback.
//
// Zero is the SAME authenticated live original-VA D7A218 cell; one comes from
// extent_constants.one_D7A24C and is not copied into a bank. All genuine Raw17/
// Raw24 minimum/epsilon/texture/current-table/canonical+04 qualifications remain.
// The local decision DWORDs only carry private Source control flow; they issue
// no Origin, Native permission, freshness/readiness or disposition authority.
//
// Caller proves actual Icon/state/vector/subobjects, whole-platform cell/object,
// data cells, callback/registry/providers and distinct stable per-active Scratch.
// Keep them through callbacks/unknown escape. Private Scratch/control/argument
// backing is separate from actual payloads/OTHER provider-write extents; declared
// getter/HOME/Raw24 writes are intentional separate subobjects. The SDK handler
// and Raw24/Raw17 ordinary exception policies are inherited without weakening.
// No catch, cleanup/retry/reset/NoThrow/FP-control or new terminal policy is added.
// Original private-stack/register/FLAGS/fault/SEH/FH3/unmaskedFP/emitted code,
// actual raw producer/current80/default80/normal custody and activation remain
// unproved. Normal return is solely this Source result, not Native EH authority.
std::uint32_t gui_icon_prefers_bilinear_filter_storage_00ab2600(
    void* actual_138b_icon, NativeGuiIconFilterScratch& scratch,
    const NativePlatformLoadPublication& actual_platform_publication,
    const GuiTextureCallbacks& callbacks,
    const GuiIconStateTextureConstants& extent_constants,
    const volatile float& actual_zero_00d7a218,
    const volatile float& actual_minimum_width_00d5c3b8,
    const volatile double& actual_epsilon_00d7a268);
} // namespace bsp
