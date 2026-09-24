#pragma once
#include <cstdint>

namespace bsp {

// Every lookup resolves the CURRENT raw profile/slot. The table resolver is
// side-effect-free and returns actual numeric entries, not callable source
// addresses. Base targets below have concrete raw implementations; every
// other target requires its real binding. No unknown-profile/base fallback.
class NativeGuiWidgetVisibilityBindings {
public:
    virtual ~NativeGuiWidgetVisibilityBindings() = default;
    virtual const volatile std::uint32_t* current_table(void* actual_widget,
        std::uint32_t captured_profile) = 0;
    virtual std::uint8_t call_visibility_38(void* actual_widget,
        std::uint32_t captured_target) = 0;
    virtual void call_visibility_changed_3c(void* actual_widget,
        std::uint32_t captured_target, std::uint8_t visible) = 0;
};
struct NativeGuiWidgetVisibilityContext {
    const volatile float& actual_zero_00d7a218;
    NativeGuiWidgetVisibilityBindings& bindings;
};

// Complete 33B base+38 leaf. Original ECX widget, EAX0/1, RET; source borrows
// the live zero address in EDX. Explicit MOVSS/COMISS/JBE retain unordered
// behavior and comparison flags. Null model+4C does not read the constant.
std::uint32_t __fastcall native_gui_widget_is_visible_00a9e0d0(
    void* actual_widget, const volatile float* actual_zero_00d7a218) noexcept;
// The complete original3B base+3C body is RET4. This helper is selected ONLY
// for targetA9E100; it is not a successful fallback for derived callbacks.
void __fastcall native_gui_widget_visibility_changed_00a9e100(
    void* actual_widget, void* unused_edx, std::uint32_t visible) noexcept;

// Complete normal222B body. Native ECX widget; five byte-valued stack args;
// RET14. Source uses byte semantics, not original upper stack-DWORD preimages.
// GUI children are the ACTUAL circular list at+64 (head+68; node next+0,
// child+8). Even recurse0 walks that list; it masks child apply_requested.
// Current profile, head and next links are reread at their native points.
void propagate_native_gui_widget_visibility_00aa8450(void* actual_widget,
    std::uint8_t old_ancestor, std::uint8_t new_ancestor,
    std::uint8_t requested, std::uint8_t recurse, std::uint8_t apply_requested,
    NativeGuiWidgetVisibilityContext&);

// Complete normal127B body. Native ECX widget, byte-valued requested on
// stack, RET4. Ancestors combine AL with bitwise AND starting at1; callbacks
// may change their current+70 link. After propagation, reload widget+4C/+75
// and use the genuine B6DA70 node hierarchy provider with FLD1/FLDZ factor.
void set_native_gui_widget_visible_00aa8530(void* actual_widget,
    std::uint8_t requested, NativeGuiWidgetVisibilityContext&);

// Require live aligned raw objects, valid retained list nodes across callback
// returns, finite parent/child traversal, and genuine returning dispatch.
// Native CRT invalid-parameter behavior, upper stacked bytes, exceptions,
// unmasked FP faults, outer binary ABI and application/game binding excluded.
} // namespace bsp
