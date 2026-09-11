#include "bsp/gui_text_type_dispatch.hpp"
#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
GuiTextContentBinding require_text(GuiTextLifetime& lifetime,
    GuiTextTypeDispatchServices& services) {
    auto binding = lifetime.content_binding();
    if (binding.widget.layout().type != GuiWidgetType::Text ||
        binding.widget.layout().transform.type_id != 3 ||
        &services.buffers.widgets.owner(binding.widget.layout()) != &binding.widget)
        throw std::logic_error("Text dispatch requires its canonical type3 owner");
    return binding;
}
GuiLayoutWidget* find_arrow(GuiLayoutWidget& parent, const char* spelling,
    std::uint32_t length, GuiTextTypeDispatchServices& services) {
    NativeString name;
    name.resize_0041dd40(services.buffers.strings, length, true);
    //AB881A/AB8887 copy CURRENT length+1 after resize's callbacks.
    if (name.data()) std::memcpy(name.data(), spelling, name.length() + 1u);
    GuiLayoutWidget* child;
    try {
        child = find_child_by_name_00aa7e00(services.buffers.widgets, parent, name, 0,
            services.compare_names_00bf7fbf);
    } catch (...) {
        destroy_native_string_header_0041dd20(&name, services.buffers.strings);
        throw;
    }
    destroy_native_string_header_0041dd20(&name, services.buffers.strings);
    return child;
}
} // namespace

const NativeString& native_node_name_00b6d800(const NativeNodeStorage& node) noexcept {
    return node.name_54;
}

GuiLayoutWidget* find_child_by_name_00aa7e00(GuiWidgetOwnerRuntime& widgets,
    GuiLayoutWidget& parent, const NativeString& name, std::uint32_t unused,
    GuiNativeNameCompare compare) {
    static_cast<void>(unused); //Native second stack argument is never read.
    //This is the existing borrowed native GUI child-list projection. The
    //unique_ptr vector separately owns allocations; it cannot express native
    //duplicate payload entries and is not used as a replacement search list.
    for (auto* child : parent.transform.children) {
        if (!child) throw std::logic_error("GUI name lookup found a null child payload");
        auto& owner = widgets.owner(*child);
        auto* node = owner.node_binding();
        if (!node) continue;
        const auto& current = native_node_name_00b6d800(node->storage);
        const auto searched_length = name.length(); //AA7E4C precedes node length.
        const auto current_length = current.length();
        if (current_length != searched_length) continue;
        if (!current_length) return &owner.layout();
        if (!compare) throw std::logic_error("GUI name lookup requires its actual CRT comparer");
        const char* searched_data = name.data();
        const char* current_data = current.data();
        if (compare(current_data, searched_data) == 0) return &owner.layout();
    }
    return nullptr;
}

void construct_gui_text74_00ab7700(GuiTextLifetime& lifetime,
    GuiTextTypeDispatchServices& services) {
    auto binding = require_text(lifetime, services);
    auto* captured = binding.widget.model_reference(); //AB771D before allocation.
    if (!captured || &captured->model_owner().node != binding.widget.node_binding() ||
        captured->model_owner().phase != NativeModelOwner::Phase::live ||
        captured->reference_count.load() <= 0 ||
        &captured->model_owner().environment.nodes != &services.buffers.parenting.nodes ||
        &captured->model_owner().environment.retained_owners != &services.buffers.geometry.actual_owners())
        throw std::logic_error("Text current74 requires its same captured live main Model");
    //Existing actual B73B60/B73D70 allocation, B75170(0,mesh,sentinel,sentinel),
    //creator release. Its model argument remains the PRE-allocation capture.
    services.buffers.geometry.construct_and_associate74_fragment(captured->model_owner());
    binding.widget.set_position_00aa7dc0({0.0f, 0.0f, 0.0f}); //AB779B, current node reload.
}

void load_gui_text78_00ab6aa0(GuiTextLifetime& lifetime,
    GuiTextTypeDispatchServices& services) {
    auto binding = require_text(lifetime, services);
    binding.widget.base_loaded78_00aa7170(); //Native entire body is this tail jump.
}

void set_gui_text_active60_00ab87d0(GuiTextLifetime& lifetime, bool active,
    GuiTextTypeDispatchServices& services) {
    auto binding = require_text(lifetime, services);
    auto* left = find_arrow(binding.widget.layout(), "__Active_Left_Icon", 0x12, services);
    auto* right = find_arrow(binding.widget.layout(), "__Active_Right_Icon", 0x13, services);
    if (left && right) {
        services.buffers.widgets.owner(*left).set_visible34(active);
        services.buffers.widgets.owner(*right).set_visible34(active);
    }
}
} // namespace bsp
