#include "bsp/gui_timed_entry_types.hpp"
#include "bsp/gui_section_runtime.hpp"
#include "bsp/gui_widget_owner.hpp"
#include <stdexcept>

namespace bsp {
namespace {
GuiWidgetOwner& borrowed_widget(GuiTimedEntryStorage& entry, GuiWidgetOwner& owner) {
    if (!entry.widget_08)
        throw std::logic_error("Timed entry requires its live canonical borrowed widget");
    return owner.runtime().owner(*entry.widget_08);
}
void require_profile(const GuiTimedEntryStorage& entry, std::uint32_t profile) {
    if (entry.profile_00 != profile)
        throw std::logic_error("Timed entry current profile does not match this operation");
}

bool approach_alpha(const float& current, const float& target,
    const float& rate, float delta, float& output) noexcept {
    const float* current_ptr = &current;
    const float* target_ptr = &target;
    const float* rate_ptr = &rate;
    float current_spill, target_spill;
    bool changed;
    // AC2F55..AC2FB0. Keep the x87 equality/parity test, direction flags,
    // unspilled rate*delta, float32 candidate spill and ordered-only clamp.
    __asm {
        mov ecx, current_ptr
        fld dword ptr [ecx]
        fstp current_spill
        mov ecx, target_ptr
        fld dword ptr [ecx]
        fstp target_spill
        fld target_spill
        fld st(0)
        fld current_spill
        fld st(0)
        fxch st(2)
        fucomip st(0), st(2)
        fstp st(1)
        lahf
        test ah, 44h
        jnp alpha_equal
        fcomi st(0), st(1)
        mov ecx, rate_ptr
        fld dword ptr [ecx]
        fmul delta
        jbe alpha_add
        fsubp st(1), st(0)
        fstp current_spill
        fld current_spill
        fxch st(1)
        fcomip st(0), st(1)
        jmp alpha_clamp
    alpha_add:
        faddp st(1), st(0)
        fstp current_spill
        fld current_spill
        fcomip st(0), st(1)
    alpha_clamp:
        fstp st(0)
        jbe alpha_changed
        movss xmm0, target_spill
        movss current_spill, xmm0
    alpha_changed:
        mov changed, 1
        jmp alpha_done
    alpha_equal:
        fstp st(1)
        fstp st(0)
        mov changed, 0
    alpha_done:
    }
    if (changed) {
        // AC2FB5..AC2FBF reloads/spills the selected candidate to the call.
        float* output_ptr = &output;
        __asm {
            mov ecx, output_ptr
            fld current_spill
            fstp dword ptr [ecx]
        }
    }
    return changed;
}
} // namespace

GuiTimedEntryStorage& construct_gui_timed_entry_base_00ad3970(
    GuiTimedEntryStorage& entry, GuiLayoutWidget& widget) noexcept {
    entry.profile_00 = kGuiTimedEntryBaseProfile;
    entry.remaining_04 = 0.0f;
    entry.widget_08 = &widget;
    return entry;
}
GuiTimedEntryStorage& construct_gui_timed_entry_alpha_00ac2ed0(
    GuiTimedEntryStorage& entry, GuiLayoutWidget& widget,
    const volatile float& one_00d7a24c) noexcept {
    construct_gui_timed_entry_base_00ad3970(entry, widget);
    const volatile float* one = &one_00d7a24c;
    GuiTimedEntryStorage* destination = &entry;
    __asm {
        mov eax, one
        movss xmm0, dword ptr [eax]
        mov ecx, destination
        mov dword ptr [ecx], 00d5ca74h
        movss dword ptr [ecx + 0ch], xmm0
        movss dword ptr [ecx + 10h], xmm0
    }
    return entry;
}
void reset_gui_timed_entry_base_00ad3990(GuiTimedEntryStorage& entry) noexcept {
    entry.profile_00 = kGuiTimedEntryBaseProfile;
}
void destroy_gui_timed_entry_profile(GuiTimedEntryStorage& entry) {
    if (entry.profile_00 != kGuiTimedEntryAlphaProfile &&
        entry.profile_00 != kGuiTimedEntrySectionProfile)
        throw std::logic_error("Timed deletion requires a supported current derived profile");
    reset_gui_timed_entry_base_00ad3990(entry);
}

bool update_gui_timed_entry_alpha_00ac2f40(GuiTimedEntryStorage& entry,
    GuiWidgetOwner& domain_owner, float delta) {
    require_profile(entry, kGuiTimedEntryAlphaProfile);
    float color[4];
    auto& read_owner = borrowed_widget(entry, domain_owner);
    const float* const current_color =
        read_owner.implementation().read_color54(read_owner, color);
    if (!current_color)
        throw std::logic_error("Timed alpha current54 returned no color storage");
    float candidate;
    if (!approach_alpha(current_color[3], entry.target_0c, entry.rate_10, delta, candidate))
        return false;
    // AC2FB2 reloads entry+8 AFTER current54 and arithmetic.
    auto& write_owner = borrowed_widget(entry, domain_owner);
    write_owner.implementation().set_alpha4c(write_owner, candidate);
    return true;
}

bool approach_gui_timed_entry_section_00abe7b0(const float& current,
    const float& target, const float& rate, float delta, float& output) noexcept {
    const float* current_ptr = &current;
    const float* target_ptr = &target;
    const float* rate_ptr = &rate;
    float current_spill, target_spill;
    bool changed;
    // ABE7CC..ABE84B. Section compares current against target, whereas alpha
    // compares target against current; retain the distinct unordered branch.
    __asm {
        mov ecx, current_ptr
        fld dword ptr [ecx]
        fstp current_spill
        mov ecx, target_ptr
        fld dword ptr [ecx]
        fstp target_spill
        fld current_spill
        fld st(0)
        fld target_spill
        fld st(0)
        fxch st(2)
        fucomip st(0), st(2)
        fstp st(1)
        lahf
        test ah, 44h
        jnp section_equal
        fcomi st(0), st(1)
        mov ecx, rate_ptr
        fld dword ptr [ecx]
        fmul delta
        jbe section_subtract
        faddp st(2), st(0)
        fxch st(1)
        fstp current_spill
        fld current_spill
        fcomip st(0), st(1)
        jmp section_clamp
    section_subtract:
        fsubp st(2), st(0)
        fxch st(1)
        fstp current_spill
        fld current_spill
        fxch st(1)
        fcomip st(0), st(1)
    section_clamp:
        fstp st(0)
        jbe section_changed
        movss xmm0, target_spill
        movss current_spill, xmm0
    section_changed:
        mov changed, 1
        jmp section_done
    section_equal:
        fstp st(1)
        fstp st(0)
        mov changed, 0
    section_done:
    }
    if (changed) {
        float* output_ptr = &output;
        __asm {
            mov ecx, output_ptr
            movss xmm0, current_spill
            movss dword ptr [ecx], xmm0
        }
    }
    return changed;
}
bool update_gui_timed_entry_section_00abe7b0(GuiTimedEntryStorage& entry,
    GuiWidgetOwner& domain_owner, float delta) {
    require_profile(entry, kGuiTimedEntrySectionProfile);
    auto& widget = borrowed_widget(entry, domain_owner);
    if (widget.implementation().type5c(widget) != 0x11) return false;
    // ABE7C9 reloads entry+08 after current5C; the callback may change it.
    auto& current = borrowed_widget(entry, domain_owner);
    auto* section = dynamic_cast<GuiSectionRuntimeImplementation*>(&current.implementation());
    if (!section || &section->owner() != &current)
        throw std::logic_error("Section timed update requires its same canonical companion");
    return update_gui_section_timed_owner_00abe7b0(*section, entry, delta);
}

bool advance_gui_timed_entry_00ad39a0(GuiTimedEntryStorage& entry,
    GuiWidgetOwner& domain_owner, float delta, GuiLayoutWidget* /*unused_widget*/,
    const GuiTimedEntryConstants& constants) {
    float* remaining = &entry.remaining_04;
    float countdown;
    bool dispatch;
    __asm {
        mov ecx, remaining
        fld dword ptr [ecx]
        fsub delta
        fstp countdown
        fld countdown
        fst dword ptr [ecx]
        fldz
        fcomip st(0), st(1)
        fstp st(0)
        seta dispatch
    }
    if (!dispatch) return true;
    const volatile float* zero = &constants.zero_00d7a208;
    float effective_delta;
    __asm {
        mov eax, zero
        movss xmm0, dword ptr [eax]
        subss xmm0, countdown
        movss effective_delta, xmm0
    }
    bool active;
    switch (entry.profile_00) {
    case kGuiTimedEntryAlphaProfile:
        active = update_gui_timed_entry_alpha_00ac2f40(entry, domain_owner, effective_delta);
        break;
    case kGuiTimedEntrySectionProfile:
        active = update_gui_timed_entry_section_00abe7b0(entry, domain_owner, effective_delta);
        break;
    default:
        throw std::logic_error("Timed entry current8 has no supported concrete profile");
    }
    entry.remaining_04 = 0.0f;
    return active;
}
} // namespace bsp
