#include "bsp/gui_widget_frame_runtime.hpp"
#include "bsp/gui_text_runtime_factory.hpp"
#include "bsp/gui_type_dispatch.hpp"
#include "bsp/gui_timed_entry_owner.hpp"
#include "bsp/input_device_state.hpp"
#include <limits>
#include <optional>
#include <stdexcept>

namespace bsp {
namespace {
void require(bool value, const char* message) {
    if (!value) throw std::logic_error(message);
}
float argument_spill(float value) noexcept {
    float result;
    __asm {
        fld value
        fstp result
    }
    return result;
}
void advance_elapsed(float& elapsed, float seconds) noexcept {
    float* destination = &elapsed;
    __asm {
        fld seconds
        mov eax, destination
        fadd dword ptr [eax]
        fstp dword ptr [eax]
    }
}
void copy_float(float& output, const float& input) noexcept {
    float* destination = &output;
    const float* source = &input;
    __asm {
        mov eax, source
        fld dword ptr [eax]
        mov edx, destination
        fstp dword ptr [edx]
    }
}
float product(const float& first, const float& second) noexcept {
    const float* left = &first;
    const float* right = &second;
    float result;
    __asm {
        mov eax, left
        fld dword ptr [eax]
        mov edx, right
        fmul dword ptr [edx]
        fstp result
    }
    return result;
}
float subtract_float(float position, float pivot) noexcept {
    float origin;
    __asm {
        fld position
        fsub pivot
        fstp origin
    }
    return origin;
}
void axis_bounds(float origin, float extent, float& lower, float& upper) noexcept {
    float* lo = &lower;
    float* hi = &upper;
    __asm {
        fld origin
        mov eax, lo
        fst dword ptr [eax]
        fadd extent
        mov eax, hi
        fstp dword ptr [eax]
    }
}
bool axis_contains(const float& lower, const float& upper,
    const volatile float& current) noexcept {
    const float* lo = &lower;
    const float* hi = &upper;
    const volatile float* point = &current;
    unsigned char result;
    // Preserve both ordered inclusive FCOMI/JC tests, including unordered.
    __asm {
        mov eax, lo
        fld dword ptr [eax]
        mov eax, point
        fld dword ptr [eax]
        fcomi st, st(1)
        fstp st(1)
        jc outside_first
        mov eax, hi
        fld dword ptr [eax]
        fcomip st, st(1)
        fstp st
        setnc result
        jmp done
    outside_first:
        fstp st
        mov result, 0
    done:
    }
    return result != 0;
}
bool ordered_equal(float value, const volatile float& zero) noexcept {
    const volatile float* constant = &zero;
    unsigned char result;
    // UCOMISS; LAHF; TEST AH,44h; JNP in AA89ED and AA8A6D.
    // Exactly ordered equality; NaN follows the non-equal arm.
    __asm {
        movss xmm0, value
        mov eax, constant
        ucomiss xmm0, dword ptr [eax]
        lahf
        test ah, 44h
        setnp result
    }
    return result != 0;
}
double elapsed_difference(const float& now, float first) noexcept {
    const float* current = &now;
    double result;
    __asm {
        mov eax, current
        fld dword ptr [eax]
        fsub first
        fstp result
    }
    return result;
}
bool threshold_exceeds_elapsed(const GuiInputDeviceRef& mouse, double elapsed) {
    const float* threshold = mouse.mouse_double_click_storage();
    unsigned char result;
    // A9A380's ST0 result, then AA8A38's FLD/FXCH/FCOMIP/JBE.
    __asm {
        mov eax, threshold
        fld dword ptr [eax]
        fld elapsed
        fxch
        fcomip st, st(1)
        fstp st
        seta result
    }
    return result != 0;
}
bool elapsed_exceeds_threshold(const GuiInputDeviceRef& mouse, double elapsed) {
    const float* threshold = mouse.mouse_double_click_storage();
    unsigned char result;
    // The second native call compares in the opposite order at AA8A9B.
    __asm {
        mov eax, threshold
        fld dword ptr [eax]
        fld elapsed
        fcomip st, st(1)
        fstp st
        seta result
    }
    return result != 0;
}
bool has_entries(GuiWidgetOwner& owner, const GuiTimedEntryConstants& constants) {
    std::int32_t count;
    std::memcpy(&count, &owner.extra_fields().pointers_88_90[1], 4);
    if (count <= 0) return false;
    owner.require_timed_entry_ownership();
    auto& entries = owner.timed_entries(constants.one_00d7a24c);
    entries.validate_live();
    auto* const data = entries.data();
    for (std::int32_t index = 0; index < count; ++index)
        if (data[index]) return true;
    return false;
}
}

struct GuiWidgetFrameRuntime::ActiveFrame {
    GuiWidgetFrameRuntime& runtime;
    GuiWidgetOwner& widget;
    ActiveFrame* previous;
    ActiveFrame(GuiWidgetFrameRuntime& value, GuiWidgetOwner& owner)
        : runtime(value), widget(owner), previous(value.active_) {
        require(!runtime.operation_active(owner), "GUI frame reentry into an active owner");
        runtime.active_ = this;
    }
    ~ActiveFrame() { runtime.active_ = previous; }
};
GuiWidgetFrameListenerOwner::GuiWidgetFrameListenerOwner(void* identity) : identity_(identity) {
    require(identity != nullptr, "GUI listener adapter requires its actual owner identity");
}
GuiWidgetFrameRuntime::GuiWidgetFrameRuntime(GuiWidgetFrameServices services)
    : services_(services) {
    services_.widgets.bind_frame_runtime(*this);
}
GuiWidgetFrameRuntime::~GuiWidgetFrameRuntime() noexcept {
    if (active_) std::terminate();
    services_.widgets.unbind_frame_runtime(*this);
}
void GuiWidgetFrameRuntime::bind_listener(GuiWidgetFrameListenerOwner& owner) {
    const auto found = listeners_.find(owner.actual_identity());
    require(found == listeners_.end() || found->second == &owner,
        "GUI listener identity is already bound to another owner");
    listeners_.emplace(owner.actual_identity(), &owner);
}
void GuiWidgetFrameRuntime::unbind_listener(GuiWidgetFrameListenerOwner& owner) {
    require(active_ == nullptr, "GUI listener adapters must survive active frame callbacks");
    const auto found = listeners_.find(owner.actual_identity());
    require(found != listeners_.end() && found->second == &owner,
        "GUI listener adapter does not own this identity binding");
    listeners_.erase(found);
}
bool GuiWidgetFrameRuntime::operation_active(const GuiWidgetOwner& owner) const noexcept {
    for (auto* frame = active_; frame; frame = frame->previous)
        if (&frame->widget == &owner) return true;
    return false;
}
void GuiWidgetFrameRuntime::bind_listbox_frames(const GuiListboxFrameServices& services) {
    require(!active_ && !listbox_frames_ && &services.base_frames == this &&
        services.input_groups_00f8bbf4.same_publication(services_.input_groups_00f8bbf4),
        "Listbox frame binding requires its idle same runtime and input publication");
    listbox_frames_ = &services;
}
void GuiWidgetFrameRuntime::unbind_listbox_frames(const GuiListboxFrameServices& services) {
    require(!active_ && listbox_frames_ == &services,
        "Listbox frame services must own the binding and survive active frames");
    listbox_frames_ = nullptr;
}
bool gui_widget_uses_base_frame40_profile(GuiWidgetType type) noexcept {
    switch (type) {
    case GuiWidgetType::Screen:
    case GuiWidgetType::Group:
    case GuiWidgetType::Text:
    case GuiWidgetType::ClipBox:
    case GuiWidgetType::Section:
    case GuiWidgetType::FrameBox:
        return true;
    default:
        return false;
    }
}
float gui_mouse_double_click_seconds_00a9a380(const MouseInputDevice& mouse) noexcept {
    return mouse.double_click_seconds;
}
void gui_widget_base_bounds64_00a9e120(float&, float&, float&, float&) noexcept {}
void GuiWidgetFrameRuntime::align_bounds64(GuiWidgetOwner& widget,
    float& left, float& top, float& right, float& bottom) {
    require(&widget.runtime() == &services_.widgets,
        "GUI bounds require the same widget runtime");
    require(widget.base_lifetime_.phase == GuiWidgetBaseDeletionPhase::not_started &&
        !widget.scene_release_active_, "GUI bounds cannot borrow a retiring widget");
    std::optional<ActiveFrame> bounds_frame;
    if (!operation_active(widget)) bounds_frame.emplace(*this, widget);
    if (widget.layout().type == GuiWidgetType::Text) {
        auto* text = dynamic_cast<GuiTextRuntimeImplementation*>(&widget.implementation());
        require(text && widget.text_lifetime(), "GUI hit test requires its actual Text owner");
        text->align_bounds64_00ab6d70(left, top, right, bottom);
        return;
    }
    require(gui_widget_uses_base_frame40_profile(widget.layout().type)
        || widget.layout().type == GuiWidgetType::Icon || widget.layout().type == GuiWidgetType::Listbox,
        "GUI hit test current64 profile has no established implementation");
    gui_widget_base_bounds64_00a9e120(left, top, right, bottom);
}
bool GuiWidgetFrameRuntime::contains_pointer_00aa6a40(GuiWidgetOwner& widget) {
    require(&widget.runtime() == &services_.widgets, "GUI hit test requires the same widget runtime");
    require(widget.base_lifetime_.phase == GuiWidgetBaseDeletionPhase::not_started &&
        !widget.scene_release_active_, "GUI hit test cannot borrow a retiring widget");
    // AA87B0 already borrows this widget. A direct AA6A40 call must retain the
    // same protection through its actual current64 callback as well.
    std::optional<ActiveFrame> hit_frame;
    if (!operation_active(widget)) hit_frame.emplace(*this, widget);
    const auto& transform = widget.layout().transform;
    float width, height;
    copy_float(width, transform.size.width);
    const float pivot_x = product(transform.pivot_x, width);
    copy_float(height, transform.size.height);
    const float pivot_y = product(transform.pivot_y, height);
    const auto position = resolved_position(transform); // actual AA6750 projection
    float left, top, right, bottom;
    // Both subtractions are spilled before either size is added.
    const float origin_x = subtract_float(position.x, pivot_x);
    const float origin_y = subtract_float(position.y, pivot_y);
    axis_bounds(origin_x, width, left, right);
    axis_bounds(origin_y, height, top, bottom);
    align_bounds64(widget, left, top, right, bottom);
    return axis_contains(left, right, services_.pointer_x_00f8bc74)
        && axis_contains(top, bottom, services_.pointer_y_00f8bc78);
}
GuiWidgetFrameListenerOwner& GuiWidgetFrameRuntime::listener(GuiWidgetOwner& widget) const {
    const auto found = listeners_.find(widget.extra_fields().layout_listener_dc);
    require(found != listeners_.end(), "GUI current listener has no actual owner adapter");
    return *found->second;
}
void GuiWidgetFrameRuntime::dispatch_current68(GuiWidgetOwner& widget, GuiWidgetOwner*) {
    require(&widget.runtime() == &services_.widgets,
        "GUI current68 requires the same widget runtime");
    require(gui_widget_uses_base_frame40_profile(widget.layout().type) ||
        widget.layout().type == GuiWidgetType::Icon,
        "GUI current68 profile has no established implementation");
    widget.require_no_active_owned_operation();
    ActiveFrame call(*this, widget);
    auto& fields = widget.extra_fields();
    if (!fields.layout_listener_dc || fields.byte_79) {
        if (auto* parent = widget.layout().transform.parent)
            dispatch_current68(services_.widgets.owner(*parent), &widget);
        return;
    }
    listener(widget).call_current00(widget);
    const auto mouse = services_.input_groups_00f8bbf4.device(1);
    if (!mouse) return;
    mouse.require_mouse();
    if (mouse.mouse_current_down(0) && !mouse.mouse_previous_down(0))
        listener(widget).call_current04(widget);
    if (mouse.mouse_current_down(1) && !mouse.mouse_previous_down(1))
        listener(widget).call_current08(widget);
}
void GuiWidgetFrameRuntime::listener_tail(GuiWidgetOwner& widget) {
    auto& fields = widget.extra_fields();
    if (!widget.layout().transform.mouse_hit || !fields.layout_listener_dc) return;
    const bool inside = contains_pointer_00aa6a40(widget);
    if (inside != fields.byte_d4) listener(widget).call_current18(widget, inside);
    fields.byte_d4 = inside; // BL survives the callback, then overwrites current+D4.

    const auto mouse = services_.input_groups_00f8bbf4.device(1);
    if (!mouse) return;
    mouse.require_mouse();
    // ESI keeps this same device even if a listener callback switches backend.
    if (!fields.byte_79) {
        if (!mouse.mouse_current_down(0) && mouse.mouse_previous_down(0))
            listener(widget).call_current0c(widget);
        if (!mouse.mouse_current_down(1) && mouse.mouse_previous_down(1))
            listener(widget).call_current10(widget);
        return;
    }
    auto& first = fields.fields_7c_80[0];
    if (!fields.byte_d4) { first = 0.0f; return; }
    const float captured_first = first;
    if (mouse.mouse_current_down(0) && !mouse.mouse_previous_down(0)) {
        if (ordered_equal(captured_first, services_.zero_00d7a218)) {
            copy_float(first, fields.fields_7c_80[1]);
            listener(widget).call_current04(widget);
        } else {
            const double elapsed = elapsed_difference(fields.fields_7c_80[1], captured_first);
            if (threshold_exceeds_elapsed(mouse, elapsed)) {
                first = 0.0f;
                listener(widget).call_current14(widget);
            }
        }
        return;
    }
    if (!ordered_equal(captured_first, services_.zero_00d7a218)) {
        const double elapsed = elapsed_difference(fields.fields_7c_80[1], captured_first);
        if (elapsed_exceeds_threshold(mouse, elapsed)) first = 0.0f;
    }
}
void GuiWidgetFrameRuntime::update40(GuiWidgetOwner& widget, float seconds) {
    if (widget.layout().type == GuiWidgetType::Listbox) {
        require(&widget.runtime() == &services_.widgets && listbox_frames_,
            "Listbox frame requires its same bound frame services");
        auto* listbox = dynamic_cast<GuiListboxTypeImplementation*>(&widget.implementation());
        require(listbox && &listbox->runtime().owner() == &widget,
            "Listbox current40 requires its canonical companion");
        widget.require_no_active_owned_operation();
        ActiveFrame active(*this, widget);
        listbox->runtime().update40_00a9d030(seconds, *listbox_frames_);
        return;
    }
    if (widget.layout().type == GuiWidgetType::Icon) {
        require(&widget.runtime() == &services_.widgets, "Icon frame requires the same widget runtime");
        auto* icon = dynamic_cast<GuiIconTypeImplementation*>(&widget.implementation());
        require(icon != nullptr, "Icon current40 requires the same canonical Icon runtime");
        widget.require_no_active_owned_operation();
        ActiveFrame active(*this, widget);
        // AB1151/AB1157 spills a COPY for AA87B0. The native caller's original
        // argument remains the later timer-subtraction input. Do not dispatch
        // current40 again or release the owner borrow between base and tail.
        update_base_active(widget, argument_spill(seconds));
        icon->runtime().update_after_base40_00ab1150(widget, seconds, services_.zero_00d7a218);
        return;
    }
    require(gui_widget_uses_base_frame40_profile(widget.layout().type),
        "GUI current40 has an unsupported derived profile");
    update_base_00aa87b0(widget, seconds);
}
void GuiWidgetFrameRuntime::update_base_00aa87b0(GuiWidgetOwner& widget, float seconds) {
    require(&widget.runtime() == &services_.widgets, "GUI frame requires the same widget runtime");
    widget.require_no_active_owned_operation();
    ActiveFrame active(*this, widget);
    update_base_active(widget, seconds);
}
void GuiWidgetFrameRuntime::update_base_from_active_00aa87b0(GuiWidgetOwner& widget, float seconds) {
    require(&widget.runtime() == &services_.widgets && operation_active(widget),
        "GUI base continuation requires this owner's active derived frame");
    update_base_active(widget, seconds);
}
void GuiWidgetFrameRuntime::update_base_active(GuiWidgetOwner& widget, float seconds) {
    auto& layout = widget.layout();
    const auto count = layout.children.size();
    require(count <= static_cast<std::size_t>((std::numeric_limits<std::int32_t>::max)()),
        "GUI child count exceeds the native signed domain");
    advance_elapsed(widget.extra_fields().fields_7c_80[1], seconds);
    for (std::size_t index = 0; index < count; ++index) {
        require(layout.children.size() == count && layout.transform.children.size() == count,
            "GUI child membership must stay stable throughout the frame");
        auto* const child_layout = layout.children[index].get();
        require(child_layout && layout.transform.children[index] == &child_layout->transform,
            "GUI owning and transform lists must identify the same current child");
        auto& child = services_.widgets.owner(*child_layout);
        const bool visible = child.implementation().is_visible38(child);
        if (visible || has_entries(child, services_.timed)) update40(child, argument_spill(seconds));
        require(layout.children.size() == count && layout.children[index].get() == child_layout,
            "GUI current child must survive its frame callbacks");
    }
    widget.timed_entries(services_.timed.one_00d7a24c)
        .update_entries_00aa87b0_fragment(seconds, services_.timed);
    listener_tail(widget);
}
} // namespace bsp
