#include "bsp/main_menu_spacing.hpp"
#include "bsp/main_menu_layout_binding.hpp"
#include "bsp/gui_text_runtime_factory.hpp"
#include <stdexcept>

namespace bsp {
namespace {
GuiWidgetOwner& owner(GuiWidgetOwnerRuntime& owners, GuiLayoutWidget* widget) {
    if (!widget) throw std::logic_error("582F30 spacing requires its actual authored widget");
    return owners.owner(*widget);
}
float normalized_height(GuiWidgetOwnerRuntime& owners, GuiLayoutWidget* widget) {
    auto& actual_owner = owner(owners, widget);
    auto* actual = dynamic_cast<GuiTextRuntimeImplementation*>(&actual_owner.implementation());
    if (!actual || actual_owner.text_lifetime() != &actual->lifetime())
        throw std::logic_error("582F30 spacing requires its actual Text runtime companion");
    // AB6BE6/BE9, AB6C08/C0B and AB6C20/C23 ALL spill/reload float32 before
    // returning ST0. Transporting that result as float adds no narrowing.
    return actual->normalized_height_00ab6bd0();
}
double difference(float lower_y, float upper_y) noexcept {
    double result;
    //582F79..582F81 /582FC8..582FD0: preserve the binary64 spill before the
    // Text-height call. Do not round the position difference to binary32.
    __asm {
        fld lower_y
        fsub upper_y
        fstp result
    }
    return result;
}
float subtract_height(double gap, float height) noexcept {
    float result;
    //582F8A/582FD9: the returned height is ST0; FSUBR computes gap-height.
    // The only narrowing of this subtraction is the native field store.
    __asm {
        fld height
        fsubr gap
        fstp result
    }
    return result;
}
} // namespace

GuiLayoutWidget* compute_main_menu_spacing_00582f45_fragment(
    MainMenuLayoutBindings& screen, GuiWidgetOwnerRuntime& owners) {
    const auto dest_y = resolved_position(owner(owners, screen.dest_text_258).layout().transform).y; //582F50
    const auto primary_y = resolved_position(owner(owners, screen.primary_text_24c).layout().transform).y; //582F67
    auto* primary_height_widget = screen.primary_text_24c; //582F6F, reload actual slot
    const auto header_gap = difference(dest_y, primary_y); //582F81, binary64
    const auto primary_height = normalized_height(owners, primary_height_widget); //582F85
    auto* dest2_widget = screen.dest2_text_25c; //582F8E, before first field store
    screen.field_2b0 = subtract_height(header_gap, primary_height); //582F99

    // No whole-fragment preflight or rollback: the first native field is
    // already published if the second reached Text operation cannot finish.
    const auto dest2_y = resolved_position(owner(owners, dest2_widget).layout().transform).y; //582F9F
    auto* dest_position_widget = screen.dest_text_258; //582FA7
    const auto next_dest_y = resolved_position(owner(owners, dest_position_widget).layout().transform).y; //582FB6
    auto* dest_height_widget = screen.dest_text_258; //582FBE, reload actual slot
    const auto description_gap = difference(dest2_y, next_dest_y); //582FD0, binary64
    const auto dest_height = normalized_height(owners, dest_height_widget); //582FD4
    auto* page = screen.main_listbox_page_470; //582FDD, capture for continuation
    screen.field_2b4 = subtract_height(description_gap, dest_height); //582FE6
    return page;
}
} // namespace bsp
