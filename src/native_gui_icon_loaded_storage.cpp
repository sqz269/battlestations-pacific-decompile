#include "bsp/native_gui_icon_loaded_storage.hpp"
#include "bsp/native_gui_icon_rebuild_storage.hpp"
#include "bsp/native_gui_icon_state_selection_storage.hpp"
#include "bsp/native_gui_widget_load_hooks.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/gui_icon_runtime.hpp"
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This qualified raw Source composition requires MSVC Win32.
#endif

namespace bsp {
namespace {
void current80_loaded_require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
} // namespace

NativeGuiIconDefaultCurrent80Calls::NativeGuiIconDefaultCurrent80Calls(
    void* receiver, std::uint32_t& state_home,
    NativeGuiIconRebuildStorageFrame& frame, NativeGuiIconRebuildStorageContext& context)
    : bound_receiver_(receiver), state_home_(state_home), rebuild_frame_(frame),
      rebuild_context_(context) {}

void NativeGuiIconDefaultCurrent80Calls::call_rebuild_current80(
    void* actual_icon, std::uint32_t selected_target, std::uint32_t full_state_word) {
    if (phase_ != NativeGuiIconSourceCallPhase::fresh) {
        invalidated_ = true;
        phase_ = NativeGuiIconSourceCallPhase::failed;
        throw std::logic_error("default CURRENT80 Source record cannot replay or reenter");
    }
    phase_ = NativeGuiIconSourceCallPhase::entered;
    entered_receiver_ = actual_icon;
    captured_target_ = selected_target;
    captured_state_word_ = full_state_word;
    try {
        // Actual outgoing DWORD HOME, not a captured value substituted inside80.
        static_cast<volatile std::uint32_t&>(state_home_) = full_state_word;
        current80_loaded_require(actual_icon == bound_receiver_,
            "default CURRENT80 requires its actual bound Source receiver");
        current80_loaded_require(&rebuild_frame_.context == &rebuild_context_,
            "default CURRENT80 requires its SAME retained Raw80 context");
        current80_loaded_require(selected_target == 0x00ab3cb0u,
            "OTHER CURRENT80 Source transport remains unsupported");
        rebuild_native_gui_icon_geometry_storage_00ab3cb0(actual_icon,
            state_home_, rebuild_frame_, rebuild_context_);
        body_returned_ = true; // Actual Source fact only; never disposition.
        current80_loaded_require(!invalidated_,
            "default CURRENT80 Source invocation was invalidated while active");
        phase_ = NativeGuiIconSourceCallPhase::complete;
    } catch (...) {
        phase_ = NativeGuiIconSourceCallPhase::failed;
        if (!failure_) failure_ = std::current_exception();
        throw;
    }
}

NativeGuiIconLoadedStorageContext::NativeGuiIconLoadedStorageContext(
    NativeGuiIconRebuildStorageContext& rebuild,
    NativeGuiWidgetLoadedDispatch& active, GuiWidgetBounds& bounds,
    const volatile double& half)
    : rebuild_(rebuild), active_(active), bounds_(bounds), half_(half),
      default_profile_(static_cast<const volatile std::uint32_t*>(
          rebuild.clip.actual_read_only_data.data_at(0x00d5c4c0u, 0x8cu))) {
    const auto& data = rebuild.clip.actual_read_only_data;
    current80_loaded_require(static_cast<const volatile void*>(&half_) ==
        data.data_at(0x00d7a280u, sizeof(double)),
        "loaded78 requires its SAME authentic half address");
    current80_loaded_require(static_cast<const volatile void*>(
        &rebuild.extent_constants.one_00d7a24c) ==
        data.data_at(0x00d7a24cu, sizeof(float)),
        "loaded78 requires its SAME authentic one address");
}

NativeGuiIconLoadedStorageFrame::NativeGuiIconLoadedStorageFrame(
    void* receiver, NativeGuiIconLoadedStorageContext& context,
    std::uint32_t& state_home, NativeGuiIconRebuildStorageFrame& rebuild_frame)
    : bound_receiver_(receiver), context_(context),
      current_(receiver, state_home, rebuild_frame, context.rebuild_) {}

void native_gui_icon_loaded_storage_00ab10f0(void* actual_icon,
    NativeGuiIconLoadedStorageFrame& f, NativeGuiIconLoadedStorageContext& c) {
    if (f.phase_ != NativeGuiIconSourceCallPhase::fresh) {
        f.invalidated_ = true;
        f.phase_ = NativeGuiIconSourceCallPhase::failed;
        throw std::logic_error("loaded78 Source record cannot replay or reenter");
    }
    f.phase_ = NativeGuiIconSourceCallPhase::entered;
    f.entered_receiver_ = actual_icon;
    f.entered_context_ = &c;
    try {
        current80_loaded_require(actual_icon == f.bound_receiver_ && &c == &f.context_,
            "loaded78 requires its actual bound receiver and SAME context");
        native_gui_widget_loaded_00aa7170(actual_icon, c.active_, c.bounds_, c.half_);
        f.base_returned_ = true;
        current80_loaded_require(!f.invalidated_,
            "loaded78 Source invocation was invalidated by its base provider");

        volatile std::uint32_t* const homes = f.native_argument_homes_;
        const volatile std::uint32_t* const table = c.default_profile_;
        std::uint32_t captured_profile, captured_target, supported;
        __asm {
            // Source address bindings only; no current profile/target sampled.
            mov esi, actual_icon
            mov edi, homes
            mov ebx, table
            fld1
            mov eax, dword ptr [esi]
            cmp eax, ebx
            jne loaded78_other_profile
            mov edx, dword ptr [eax + 88h]
            fstp dword ptr [edi + 8]
            mov captured_profile, eax
            mov captured_target, edx
            mov supported, 1
            jmp loaded78_ratio_staged
        loaded78_other_profile:
            fstp st(0)
            mov captured_profile, eax
            mov supported, 0
        loaded78_ratio_staged:
        }
        f.captured_profile_ = captured_profile;
        if (supported == 0)
            throw std::logic_error("OTHER loaded78 profile transport remains unsupported");
        f.captured_target_ = captured_target;
        static_cast<volatile std::uint32_t&>(f.native_argument_homes_[1]) = 0;
        static_cast<volatile std::uint32_t&>(f.native_argument_homes_[0]) = 0;
        current80_loaded_require(captured_target == 0x00ab1710u,
            "OTHER current88 Source transport remains unsupported");
        select_native_gui_icon_state_storage_00ab1710(actual_icon,
            f.native_argument_homes_, c.rebuild_.extent_constants.one_00d7a24c, f.current_);
        f.selection_returned_ = true;
        current80_loaded_require(!f.invalidated_ && !f.current_.invalidated_,
            "loaded78 Source invocation was invalidated while selected");
        f.phase_ = NativeGuiIconSourceCallPhase::complete;
    } catch (...) {
        f.phase_ = NativeGuiIconSourceCallPhase::failed;
        if (!f.failure_) f.failure_ = std::current_exception();
        throw;
    }
}
} // namespace bsp
