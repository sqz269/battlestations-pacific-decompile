#include "bsp/gui_manager_frame.hpp"
#include "bsp/gui_timed_entry_owner.hpp"
#include "bsp/gui_widget_relative_bounds.hpp"
#include <cstring>
#include <limits>
#include <stdexcept>
#include <vector>

namespace bsp {
namespace {
void require(bool value,const char* message) {
    if(!value) throw std::logic_error(message);
}
void read_hidden_position(const volatile float& value, GuiWidgetPoint& output) noexcept {
    const auto* source=&value;
    auto* destination=&output;
    __asm {
        mov eax, source
        movss xmm0, dword ptr [eax]
        mov eax, destination
        movss dword ptr [eax], xmm0
        movss dword ptr [eax+4], xmm0
        xorps xmm0, xmm0
        movss dword ptr [eax+8], xmm0
    }
}
float argument_spill(float value) noexcept {
    float result;
    __asm {
        fld value
        fstp result
    }
    return result;
}
}
bool gui_widget_has_live_entries_00aa7ef0(GuiWidgetOwner& owner,
    const GuiTimedEntryConstants& constants) {
    std::int32_t count;
    std::memcpy(&count, &owner.extra_fields().pointers_88_90[1], 4);
    if (count <= 0) return false; // Native never reads88 or binds an owner here.
    owner.require_timed_entry_ownership();
    auto& entries=owner.timed_entries(constants.one_00d7a24c);
    entries.validate_live();
    auto* const data=entries.data();
    for(std::int32_t index=0;index<count;++index)
        if(data[index]) return true;
    return false;
}
void reset_gui_manager_highlights_00aa0f70(GuiResourceOwner& resources,
    GuiWidgetOwnerRuntime& owners,const volatile float& hidden_position) {
    // Native MOVSS precedes even the first null-slot test.
    GuiWidgetPoint position;
    read_hidden_position(hidden_position,position);
    if(auto* first=resources.state().highlight_frame) {
        set_gui_widget_resolved_position_00aa8240(owners.owner(*first),position);
        auto* fresh=resources.state().highlight_frame;
        require(fresh,"GUI highlight74 was cleared during position callback");
        owners.owner(*fresh).set_visible34(false);
        // Reload only on this arm, after current34 (AA0FB0).
        read_hidden_position(hidden_position,position);
    }
    if(auto* second=resources.state().highlight_circle) {
        set_gui_widget_resolved_position_00aa8240(owners.owner(*second),position);
        auto* fresh=resources.state().highlight_circle;
        require(fresh,"GUI highlight78 was cleared during position callback");
        owners.owner(*fresh).set_visible34(false);
    }
}
void update_gui_manager_00aa4f80(GuiResourceOwner& resources,float seconds,
    std::uint8_t blocked,const GuiManagerFrameServices& services) {
    resources.state_.blocked_70=blocked; // AA4FA3, raw byte, before callbacks.
    if(blocked==0) services.pointer.update_pointer_00aa3910(resources);
    auto& owners=services.frames.widgets();
    reset_gui_manager_highlights_00aa0f70(resources,owners,services.hidden_position_00d7a260);
    // 004D35D0 is a pointer-vector copy, not an owner/page clone. Allocation
    // failure/reentry and checked-STL paths remain outside this projection.
    std::vector<GuiLayoutWidget*> snapshot;
    const auto& pages=resources.pages().pages();
    require(pages.size()<=0x3fffffffu,
        "GUI manager page count exceeds native 004D35D0 allocation domain");
    snapshot.reserve(pages.size());
    for(const auto& page:pages) {
        require(page && page->root,"GUI page registry contains an invalid actual page");
        snapshot.push_back(page->root.get());
    }
    for(auto* root:snapshot) {
        auto& owner=owners.owner(*root);
        require(owner.layout().type==GuiWidgetType::Screen,
            "GUI manager snapshot requires the registered actual Screen root");
        const bool visible=owner.implementation().is_visible38(owner);
        if(visible || gui_widget_has_live_entries_00aa7ef0(owner,services.timed))
            services.frames.update40(owner,argument_spill(seconds));
    }
    resources.state_.blocked_70=0; // AA503F, precedes vector destruction.
}
} // namespace bsp
