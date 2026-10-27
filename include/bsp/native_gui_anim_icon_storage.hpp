#pragma once
#include "bsp/native_gui_icon_storage.hpp"

namespace bsp {
// Borrowed actual138..143 tail, not a separate frame/state owner. The matching
// F8C1D4 pool's slab-index DWORD144 is outside this144h payload.
struct NativeGuiAnimIconTailStorage {
    float frames_per_second_138;
    std::uint32_t last_step_ms_13c;
    std::uint8_t playing_140, loop_141, forward_142, unwritten_143;
};
static_assert(sizeof(NativeGuiAnimIconTailStorage) == 0x0c);
static_assert(offsetof(NativeGuiAnimIconTailStorage, forward_142) == 0x0a);
NativeGuiAnimIconTailStorage& native_gui_anim_icon_tail(void* actual_anim_icon);

struct NativeGuiAnimIconStorageContext {
    NativeGuiIconStorageContext& icon;
    const volatile std::uint32_t* anim_icon_profile_00d5bce0; // at least35 DWORDs for current88
};
struct NativeGuiAnimIconCopyAcquired {
    NativeGuiIconCopyAcquired icon;
    bool started{}, icon_returned{}, complete{}, failed{};
    std::exception_ptr failure;
};
struct NativeGuiAnimIconDestroyAcquired {
    NativeGuiIconDestroyAcquired icon;
    bool started{}, icon_returned{}, pool_return_entered{}, complete{}, failed{};
    std::exception_ptr failure;
};

// Required real raw service, not GuiIconRuntime's independent C++ state. The
// AB10F0 provider performs SAME AA7170 (current60(0), raw bounds) then CURRENT88
// (0,0,1). Both calls must use this SAME actual Icon/frame vector/Model/domain.
// Selected current88 below is genuine AB1710 over raw fields and its reached
// CURRENT80/GPU services. No no-op, field mirror or projected-frame fallback.
class NativeGuiAnimIconLoadedCalls {
public:
    virtual ~NativeGuiAnimIconLoadedCalls() = default;
    virtual void call_icon_reset_00ab10f0(void* actual_anim_icon) = 0;
    virtual void call_select_current88(void* actual_anim_icon,
        std::uint32_t selected_target, std::uint32_t native_index_argument,
        std::uint32_t partial_mode, std::uint32_t ratio_bits) = 0;
};

// AD2D50: ECX actual144h, EAX same, RET. SAME AB5C60 first; live CE38C8
// reload, flags140/141/142=1, D5BCE0, FPS138, time13C=0, type60=12. Preserve
// unwritten143 and allocator trailer144. No pool/registry initialization.
void* construct_native_gui_anim_icon_default_00ad2d50(void* actual_anim_icon,
    const volatile std::uint32_t& actual_one_00d7a24c,
    const volatile std::uint32_t& actual_fps_00ce38c8);
// AA1C60: ECX destination, stack source, EAX same, RET4. SAME full AB5D30
// including mandatory genuine CURRENT80, then D5BCE0/x87 FPS/time/flag copies.
// No additional frame ownership or derived type60 rewrite;143 stays unwritten.
void* construct_native_gui_anim_icon_copy_00aa1c60(void* actual_destination,
    const void* actual_source, NativeGuiIconCopyContext&, NativeGuiAnimIconCopyAcquired&);
// AA2340 optional-source factory arms, EAX actual148h slot or0, RET. Require
// initialized SAME F8C1D4 pool; constructor failure returns its captured slot
// after the Icon constructor's own cleanup, without guessed extra rollback.
void* create_native_gui_anim_icon_default_00aa2340(
    const volatile std::uint32_t& actual_one_00d7a24c,
    const volatile std::uint32_t& actual_fps_00ce38c8);
void* create_native_gui_anim_icon_copy_00aa2340(const void* actual_source,
    NativeGuiIconCopyContext&, NativeGuiAnimIconCopyAcquired&);

// AD2DC0: stamp D5BCE0 then tail to SAME full AB5E70. Actual strings/state/
// texture references, AB5700 cancellation, raw AA9730 and shared delayed drain
// remain authoritative. One-shot retained frame; entered failure cannot replay.
void destroy_native_gui_anim_icon_00ad2dc0(void* actual_anim_icon,
    NativeGuiAnimIconStorageContext&, NativeGuiAnimIconDestroyAcquired&);
// AA1D50: ECX this, stacked flags low byte AFTER dtor, EAX same, RET4. No +04
// count gate/decrement. Flags1 returns SAME slot via AD3420/AD32B0 F8C1D4;
// flags0 leaves destroyed physical storage with caller. Identity only afterward.
void* delete_native_gui_anim_icon_00aa1d50(void* actual_anim_icon,
    const volatile std::uint32_t& actual_flags_slot, NativeGuiAnimIconStorageContext&,
    NativeGuiAnimIconDestroyAcquired&);
// AD2FF0: ECX actual, RET. Call real AB10F0 FIRST; then CURRENT forward142.
// On zero compute count from SAME CURRENT F4/F8 and select count-1,0,1 through
// freshly selected CURRENT88. Empty reverse still passes FFFFFFFF; no guard or
// fabricated frame. No additional texture/string/count ownership operation.
void load_native_gui_anim_icon_00ad2ff0(void* actual_anim_icon,
    NativeGuiAnimIconStorageContext&, NativeGuiAnimIconLoadedCalls&);
} // namespace bsp
