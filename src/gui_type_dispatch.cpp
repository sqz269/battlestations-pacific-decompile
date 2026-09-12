#include "bsp/gui_type_dispatch.hpp"
#include "bsp/gui_clip_box.hpp"
#include "bsp/gui_lua_reader.hpp"
#include <cstring>
#include <stdexcept>
#include <utility>

namespace bsp {
namespace {
template<class T> void require(const T& value, const char* message) {
    if (!value) throw std::invalid_argument(message);
}
GuiIconRuntimeServices bind_icon_base(GuiWidgetOwner& owner, GuiIconRuntimeServices services) {
    services.base_loaded78_00aa7170 = [&owner] { owner.base_loaded78_00aa7170(); };
    services.set_position_00aa7dc0 = [&owner](const GuiWidgetPoint& point) {
        owner.set_position_00aa7dc0(point);
    };
    services.geometry.recompose_00aa7220 = [&owner] { owner.recompose_00aa7220(); };
    return services;
}
GuiFrameBoxRuntimeServices bind_framebox_base(GuiWidgetOwner& owner, GuiFrameBoxRuntimeServices services) {
    services.base_loaded78_00aa7170 = [&owner] { owner.base_loaded78_00aa7170(); };
    services.set_position_00aa7dc0 = [&owner](const GuiWidgetPoint& point) {
        owner.set_position_00aa7dc0(point);
    };
    auto& geometry = services.geometry;
    geometry.recompose_00aa7220 = [&owner] { owner.recompose_00aa7220(); };
    require(geometry.device && geometry.states && geometry.geometry &&
        geometry.associate_geometry && geometry.create_vertex_stream &&
        geometry.create_material && geometry.register_clip_and_owner &&
        geometry.publish_color_virtual50,
        "FrameBox type factory requires actual geometry/material/clip/color services");
    const auto& texture = services.textures.callbacks;
    require(texture.find_atlas_item && texture.load_texture && texture.width &&
        texture.height && texture.retain && services.textures.release && services.logical_texture,
        "FrameBox type factory requires actual texture ownership services");
    return services;
}
} // namespace

void GuiGroupTypeImplementation::constructed74(GuiWidgetOwner& owner) {
    owner.base_constructed74_00a9ac00();
}
void GuiGroupTypeImplementation::properties_bound(GuiWidgetOwner&, const GuiTable&) {
    //00AC6FD0 tail-jumps to00AAA710; no derived continuation exists.
}
void GuiGroupTypeImplementation::loaded78(GuiWidgetOwner& owner) { owner.base_loaded78_00aa7170(); }
void GuiGroupTypeImplementation::set_active60(GuiWidgetOwner& owner, bool active) {
    owner.base_set_active60_00aa6a30(active);
}
bool GuiGroupTypeImplementation::is_visible38(GuiWidgetOwner& owner) {
    return owner.base_is_visible38_00a9e0d0();
}
void GuiGroupTypeImplementation::visibility_changed3c(GuiWidgetOwner& owner, bool visible) {
    owner.base_visibility_changed3c_00a9e100(visible);
}
void GuiGroupTypeImplementation::set_visible34(GuiWidgetOwner& owner, bool visible) {
    owner.set_visible_00aa8530(visible);
}

GuiListboxTypeImplementation::GuiListboxTypeImplementation(GuiWidgetOwner& owner,
    GuiListboxRuntimeServices services) : runtime_(owner, std::move(services)) {}
class GuiListboxTypeImplementation::Operation {
public:
    explicit Operation(GuiListboxTypeImplementation& value) : owner_(value) { ++owner_.active_calls_; }
    ~Operation() { --owner_.active_calls_; }
private:
    GuiListboxTypeImplementation& owner_;
};
void GuiListboxTypeImplementation::require_owner(GuiWidgetOwner& owner) const {
    require(&runtime_.owner() == &owner, "Listbox type requires its same canonical owner");
}
bool GuiListboxTypeImplementation::has_active_operation() const noexcept {
    return active_calls_ != 0 || runtime_.has_active_operation();
}
void GuiListboxTypeImplementation::constructed74(GuiWidgetOwner& owner) {
    require_owner(owner); // A9AC30 is a bare RET, verified from installed bytes.
}
void GuiListboxTypeImplementation::before_properties(GuiWidgetOwner& owner, const GuiTable&) {
    require_owner(owner);
    throw std::logic_error("Listbox A9E400 properties require the actual derived reader");
}
void GuiListboxTypeImplementation::properties_bound(GuiWidgetOwner& owner, const GuiTable& table) {
    before_properties(owner, table);
}
void GuiListboxTypeImplementation::loaded78(GuiWidgetOwner& owner) {
    require_owner(owner);
    runtime_.loaded78_00a9c050();
}
void GuiListboxTypeImplementation::set_visible34(GuiWidgetOwner& owner, bool visible) {
    require_owner(owner);
    Operation operation(*this);
    // Host preflight excludes the unreconstructed manager/index AA0F50 tail.
    require(runtime_.fields().highlight_index_118 == -1,
        "Listbox A9CCE0 linked highlight page requires actual AA0F50 dispatch");
    owner.set_visible_00aa8530(visible);
    require(runtime_.fields().highlight_index_118 == -1,
        "Listbox highlight page changed during current34 base callback");
}
void GuiListboxTypeImplementation::set_active60(GuiWidgetOwner& owner, bool active) {
    require_owner(owner);
    Operation operation(*this);
    runtime_.set_active60_00a9cd20(active);
}
void GuiListboxTypeImplementation::before_scene_release(GuiWidgetOwner& owner) {
    before_host_tree_retirement(owner);
}
void GuiListboxTypeImplementation::before_host_tree_retirement(GuiWidgetOwner& owner) const {
    require_owner(owner);
    require(!has_active_operation() && runtime_.row_count_104() == 0,
        "Host Listbox retirement requires idle operations and removed borrowed rows");
}
void GuiListboxTypeImplementation::before_scalar_deletion4(GuiWidgetOwner& owner) {
    require_owner(owner);
    throw std::logic_error("Listbox native scalar deletion requires its derived teardown");
}

GuiIconTypeImplementation::GuiIconTypeImplementation(GuiWidgetOwner& owner,
    GuiIconRuntimeServices services)
    : runtime_(owner.layout(), owner.extra_fields().overbright_94,
        bind_icon_base(owner, std::move(services))) {}
void GuiIconTypeImplementation::constructed74(GuiWidgetOwner&) { runtime_.constructed74_00ab2540(); }
void GuiIconTypeImplementation::properties_bound(GuiWidgetOwner&, const GuiTable& table) {
    runtime_.read_properties_00ab3310(table);
}
void GuiIconTypeImplementation::loaded78(GuiWidgetOwner&) { runtime_.loaded78_00ab10f0(); }
void GuiIconTypeImplementation::visibility_changed3c(GuiWidgetOwner&, bool visible) {
    //00AB6189/6194 and00AB6327/6332. The ordinary add-state path seeds both
    //record flags false; the current reader rejects DelayedTextureLoad before
    //publishing it. Do not replace a real delayed branch with the base RET4.
    for (const auto& state : runtime_.state().states) {
        if ((state.load_pending && visible) || (state.flag_3d && !visible))
            throw std::logic_error("Icon visibility requires actual delayed texture lifecycle00AB6120");
    }
}

GuiFrameBoxTypeImplementation::GuiFrameBoxTypeImplementation(GuiWidgetOwner& owner,
    GuiFrameBoxRuntimeServices services, const bool& crt_sse2_conversion)
    : services_(bind_framebox_base(owner, std::move(services))),
      crt_sse2_conversion_(crt_sse2_conversion) {
    require(owner.layout().type == GuiWidgetType::FrameBox, "FrameBox adapter requires type18");
}
void GuiIconTypeImplementation::set_size58(GuiWidgetOwner& owner, const GuiWidgetSize& size) {
    runtime_.set_size58_00ab1ef0(owner, size);
}
void GuiFrameBoxTypeImplementation::rebuild_current88_00aceb30(GuiWidgetOwner& owner) {
    require(owner.layout().type == GuiWidgetType::FrameBox && &owner.implementation() == this,
        "FrameBox current88 requires the same canonical owner");
    const auto selected = state_.current_state;
    gui_framebox_rebuild_00ad0d80(state_, owner.layout(), owner.extra_fields().overbright_94,
        selected, services_);
}
void GuiFrameBoxTypeImplementation::set_size58(GuiWidgetOwner& owner, const GuiWidgetSize& size) {
    require(owner.layout().type == GuiWidgetType::FrameBox && &owner.implementation() == this,
        "FrameBox current58 requires the same canonical owner");
    owner.base_set_size58_00aa7970(size);
    if (state_.current_state == -1) return; // Reload AFTER base callbacks.
    // The borrowed input must survive base recomposition. Native reads it
    // afresh for each record and lane, including input/record aliasing.
    for (std::size_t index = 0; index < state_.states.size(); ++index) {
        const auto* input = &size;
        auto* output = &state_.states[index].size;
        __asm {
            mov eax, input
            mov edx, output
            fld dword ptr [eax]
            fstp dword ptr [edx]
            fld dword ptr [eax + 4]
            fstp dword ptr [edx + 4]
        }
    }
    rebuild_current88_00aceb30(owner);
}
void GuiFrameBoxTypeImplementation::constructed74(GuiWidgetOwner& owner) {
    gui_framebox_constructed74_00acf8f0(owner.layout(), services_);
}
void GuiFrameBoxTypeImplementation::properties_bound(GuiWidgetOwner& owner, const GuiTable& table) {
    gui_framebox_read_properties_00ad08e0(state_, owner.layout().transform, table,
        services_.textures, crt_sse2_conversion_);
}
void GuiFrameBoxTypeImplementation::loaded78(GuiWidgetOwner& owner) {
    gui_framebox_loaded78_00aceb50(state_, owner.layout(), owner.extra_fields().overbright_94, services_);
}
void GuiFrameBoxTypeImplementation::set_state84_00acf070(GuiWidgetOwner& owner, std::int16_t state) {
    require(owner.layout().type == GuiWidgetType::FrameBox && &owner.implementation() == this,
        "FrameBox current84 requires the same canonical owner");
    gui_framebox_select_state_00acf070(state_, owner.layout(), owner.extra_fields().overbright_94,
        state, services_);
}

GuiScreenLayerImplementation::GuiScreenLayerImplementation(GuiWidgetOwner& owner,
    std::uint8_t screen_flag, GuiScreenLayerServices services, const bool& crt_sse2_conversion)
    : services_(std::move(services)), crt_sse2_conversion_(crt_sse2_conversion) {
    require(owner.layout().type == GuiWidgetType::Screen, "cGuiLayer adapter requires type1");
    require(services_.acquire_camera_store78_00ac59a0 && services_.bind_node_to_scene_00b6d890 &&
        services_.visibility_hook_installed_00f8bf4c && services_.visibility_hook_00f8bf4c &&
        services_.release_derived_00ac5480, "cGuiLayer requires actual scene/camera/store/lifetime services");
    state_.name_100 = owner.layout().key;
    state_.share_scene_120 = screen_flag;
    //00AC6705/670A write base size+20/+24; this is not the scale+28/+2C.
    owner.layout().transform.size = {1.0f, 1.0f};
}
void GuiScreenLayerImplementation::before_properties(GuiWidgetOwner& owner, const GuiTable& table) {
    state_.key_108.priority = kGuiLayerPriorityDefault;
    state_.key_108.render_order = kGuiLayerRenderOrderDefault;
    if (const auto* value = table.find("Priority"))
        gui_lua_store_value_00bd63b0(*value,
            gui_lua_field(GuiLuaFieldType::Int, &state_.key_108.priority), nullptr,
            crt_sse2_conversion_);
    state_.applied_priority_fc = state_.key_108.priority;
    if (const auto* value = table.find("RenderOrder"))
        gui_lua_store_value_00bd63b0(*value,
            gui_lua_field(GuiLuaFieldType::Float, &state_.key_108.render_order), nullptr,
            crt_sse2_conversion_);
    //00AC4C9E calls78, then scene bind and position zero, then00AAA710.
    loaded78(owner);
    auto* node = owner.node_binding();
    require(node, "cGuiLayer property reader requires its actual root node");
    services_.bind_node_to_scene_00b6d890(*node, state_.scene_ec);
    owner.set_position_00aa7dc0({});
}
void GuiScreenLayerImplementation::loaded78(GuiWidgetOwner& owner) {
    if (retired_) throw std::logic_error("cGuiLayer has been retired");
    services_.acquire_camera_store78_00ac59a0(owner, state_);
    require(state_.store_f0 && state_.scene_ec,
        "cGuiLayer acquire78 did not establish an actual camera store and scene");
}
bool GuiScreenLayerImplementation::is_visible38(GuiWidgetOwner&) { return state_.visible_f5; }
void GuiScreenLayerImplementation::set_visible34(GuiWidgetOwner& owner, bool visible) {
    if (retired_) throw std::logic_error("cGuiLayer has been retired");
    const int delta = store_count_delta_00ac4450(state_.visible_f5, visible);
    if (delta) {
        require(state_.store_f0, "cGuiLayer visibility edge requires its acquired camera store");
        //Native INC/DEC wraps; preserve its bits without signed C++ overflow.
        auto count = static_cast<std::uint32_t>(state_.store_f0->visible_layers);
        count += static_cast<std::uint32_t>(delta);
        std::memcpy(&state_.store_f0->visible_layers, &count, sizeof(count));
    }
    owner.propagate_visibility_00aa8450({true, true, visible,
        owner.scene_flags().visibility_recurses, true});
    if (auto* node = owner.node_binding())
        services_.bind_node_to_scene_00b6d890(*node, visible ? state_.scene_ec : nullptr);
    //Recheck old state after propagation/binding callbacks, matching native.
    if (services_.visibility_hook_installed_00f8bf4c() && state_.visible_f5 != visible)
        services_.visibility_hook_00f8bf4c(state_.name_100, visible);
    state_.visible_f5 = visible;
}
void GuiScreenLayerImplementation::before_scene_release(GuiWidgetOwner& owner) {
    if (retired_) return;
    services_.release_derived_00ac5480(owner, state_);
    retired_ = true;
}

struct GuiTypeDispatchFactory::Shared {
    GuiTypeDispatchServices services;
    std::unordered_map<GuiLayoutWidget*, std::uint8_t> script_pages;
    explicit Shared(GuiTypeDispatchServices supplied) : services(std::move(supplied)) {}
    std::unique_ptr<GuiWidgetTypeImplementation> create(GuiWidgetOwner& owner) {
        switch (owner.layout().type) {
        case GuiWidgetType::Group:
            return std::make_unique<GuiGroupTypeImplementation>();
        case GuiWidgetType::Listbox:
            require(services.listbox, "Listbox type factory has no actual runtime services");
            return std::make_unique<GuiListboxTypeImplementation>(owner, services.listbox(owner));
        case GuiWidgetType::ClipBox:
            return std::make_unique<GuiClipBoxTypeImplementation>(owner, services.crt_sse2_conversion);
        case GuiWidgetType::Icon:
            require(services.icon, "Icon type factory has no actual resource services");
            return std::make_unique<GuiIconTypeImplementation>(owner, services.icon(owner));
        case GuiWidgetType::FrameBox:
            require(services.framebox, "FrameBox type factory has no actual resource services");
            return std::make_unique<GuiFrameBoxTypeImplementation>(owner, services.framebox(owner),
                services.crt_sse2_conversion);
        case GuiWidgetType::Section:
            require(services.section, "Section type factory has no actual resource services");
            return std::make_unique<GuiSectionRuntimeImplementation>(owner, services.section(owner));
        case GuiWidgetType::Screen: {
            const auto input = script_pages.find(&owner.layout());
            require(input != script_pages.end(), "cGuiLayer requires explicit per-page constructor input");
            const auto flag = input->second;
            script_pages.erase(input); // consume before service callbacks/reentrancy
            require(services.screen, "cGuiLayer type factory has no actual scene services");
            return std::make_unique<GuiScreenLayerImplementation>(owner, flag, services.screen(owner),
                services.crt_sse2_conversion);
        }
        default:
            throw std::invalid_argument("GUI type factory has no implementation for this widget profile");
        }
    }
};
GuiTypeDispatchFactory::GuiTypeDispatchFactory(GuiTypeDispatchServices services)
    : shared_(std::make_shared<Shared>(std::move(services))) {}
void GuiTypeDispatchFactory::prepare_script_page(GuiLayoutWidget& root, std::uint8_t flag) {
    require(root.type == GuiWidgetType::Screen, "script page constructor input requires Screen1");
    require(!root.before_destroy, "script page already has its retained type owner");
    require(shared_->script_pages.emplace(&root, flag).second,
        "script page constructor input has already been prepared");
}
void GuiTypeDispatchFactory::cancel_script_page(GuiLayoutWidget& root) noexcept {
    shared_->script_pages.erase(&root);
}
GuiWidgetImplementationFactory GuiTypeDispatchFactory::make_factory() const {
    return [shared = shared_](GuiWidgetOwner& owner) { return shared->create(owner); };
}
} // namespace bsp
