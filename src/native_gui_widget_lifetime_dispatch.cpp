#include "bsp/native_gui_widget_lifetime_dispatch.hpp"
#include "bsp/native_gui_widget_child_links.hpp"
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
void require_profile(const volatile Word* table, Word scalar, Word query) {
    require(table && table[0] == 0x00bd30e0u && table[1] == scalar &&
        table[3] == query && table[8] == 0x00aa8320u,
        "raw GUI lifetime dispatch requires its actual supported profile cells");
}
} // namespace

NativeGuiWidgetLifetimeDispatch::NativeGuiWidgetLifetimeDispatch(
    NativeNodeDestructionRuntime& nodes, NativeRenderActualOwnerRegistry& registry,
    const volatile Word* base, const volatile Word (&lineage)[2],
    const volatile Word& text_descriptor, NativeGuiWidgetLifetimeDispatchProfiles profiles,
    NativeGuiWidgetLifetimeRequiredCalls& required)
    : registry_(registry), profiles_(profiles), required_(required),
      context_{nodes, base, lineage, text_descriptor, *this} {
    require(base && base[3] == 0x00a9e070u && base[8] == 0x00aa8320u,
        "raw GUI lifetime dispatch requires the genuine base query/scene table");
    require_profile(profiles_.group_00d5cb80, 0x00ac73e0u, 0x00ac6f70u);
    require_profile(profiles_.clip_box_00d5d058, 0x00ace620u, 0x00ace0c0u);
}

NativeGuiWidgetLifetimeDispatch::BorrowedWidget NativeGuiWidgetLifetimeDispatch::widget(void* actual) {
    auto* const reference = registry_.find(actual);
    if (auto* group = dynamic_cast<NativeGuiGroupReference*>(reference)) {
        require(!group->retired() && group->actual_identity() == actual &&
            &group->lifetime_context() == &context_,
            "raw Group dispatch requires its same canonical identity and lifetime context");
        return {Kind::group, group};
    }
    if (auto* clip_box = dynamic_cast<NativeGuiClipBoxReference*>(reference)) {
        require(!clip_box->retired() && clip_box->actual_identity() == actual &&
            &clip_box->lifetime_context() == &context_,
            "raw ClipBox dispatch requires its same canonical identity and lifetime context");
        return {Kind::clip_box, clip_box};
    }
    throw std::logic_error("raw GUI lifetime dispatch rejects widgets outside Group/ClipBox");
}
void NativeGuiWidgetLifetimeDispatch::require_timed_identity(void* actual) {
    require(actual && !registry_.find(actual),
        "raw GUI timed dispatch cannot substitute a canonical counted owner");
}
const volatile Word* NativeGuiWidgetLifetimeDispatch::current_table(void* actual, Word profile) {
    if (profile == 0x00d5cb80u || profile == 0x00d5d058u) {
        const auto canonical = widget(actual);
        require((canonical.kind == Kind::group) == (profile == 0x00d5cb80u),
            "raw GUI captured profile does not match its canonical widget class");
        return canonical.kind == Kind::group ? profiles_.group_00d5cb80 : profiles_.clip_box_00d5d058;
    }
    require_timed_identity(actual);
    const auto* const table = required_.current_timed_table(actual, profile);
    require(table != nullptr, "raw GUI timed entry has no genuine current table provider");
    return table;
}
bool NativeGuiWidgetLifetimeDispatch::call_class_query_0c(void* actual, Word target, Word descriptor) {
    const auto canonical = widget(actual);
    require(target == (canonical.kind == Kind::group ? 0x00ac6f70u : 0x00ace0c0u),
        "raw GUI class query has an unsupported captured current target");
    return required_.call_class_query_0c(actual, target, descriptor);
}
void NativeGuiWidgetLifetimeDispatch::call_child_scene_20(void* actual, Word target) {
    (void)widget(actual);
    require(target == 0x00aa8320u, "raw GUI scene release has an unsupported captured current target");
    release_native_gui_widget_scene_00aa8320(actual, context_);
}
void NativeGuiWidgetLifetimeDispatch::call_child_delete_04(void* actual, Word target, Word flags) {
    require(flags == 1, "raw GUI child scalar transport requires native flags1");
    const auto canonical = widget(actual);
    const volatile Word actual_flags = flags;
    if (canonical.kind == Kind::group) {
        require(target == 0x00ac73e0u, "raw Group child has an unsupported captured scalar target");
        static_cast<NativeGuiGroupReference*>(canonical.reference)->delete_scalar_00ac73e0(actual_flags);
    } else {
        require(target == 0x00ace620u, "raw ClipBox child has an unsupported captured scalar target");
        static_cast<NativeGuiClipBoxReference*>(canonical.reference)->delete_scalar_00ace620(actual_flags);
    }
    // No child payload/count inspection, link erase or progress fallback here.
}
void NativeGuiWidgetLifetimeDispatch::call_timed_delete_00(void* actual, Word target, Word flags) {
    require_timed_identity(actual);
    require(target && flags == 1, "raw GUI timed deletion requires a genuine target and native flags1");
    required_.call_timed_delete_00(actual, target, flags);
}
void NativeGuiWidgetLifetimeDispatch::remove_all_00a9bd50(void* list, const void* pointer_cell) {
    require(list && pointer_cell, "raw GUI remove-all requires its actual list and pointer cell");
    // Exact physical 0Ch header shared with its A9B720/A9B790 producers.
    remove_native_gui_widget_list_value_00a9bd50(
        *static_cast<NativeEffectDeletionListStorage*>(list), pointer_cell);
}
} // namespace bsp
