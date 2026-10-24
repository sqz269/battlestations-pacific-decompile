#pragma once
#include "bsp/native_gui_clip_box_identity.hpp"
#include "bsp/native_gui_group_identity.hpp"

namespace bsp {
// Mandatory genuine providers outside this closed widget transport. No default
// implementation, fallback table, false class answer or no-op timed deletion.
class NativeGuiWidgetLifetimeRequiredCalls {
public:
    virtual ~NativeGuiWidgetLifetimeRequiredCalls() = default;
    // Only recognize caller-retained ACTUAL timed allocations, not semantic
    // GuiTimedEntryStorage with GuiLayoutWidget* or unknown widget storage.
    // The returned current native table is stable/pure and has at least slot0.
    virtual const volatile std::uint32_t* current_timed_table(void* actual_entry,
        std::uint32_t captured_profile) = 0;
    // Genuine captured Group AC6F70 / ClipBox ACE0C0 over CURRENT initialized
    // F8BFD8[3] / F8C174[3] in the SAME native type phase. F8C180 is ClipBox's
    // name word, not an ID. Preserve descriptor, short-circuit reads and AL.
    // The existing process supplies Element/Text/Layer only; no Group/ClipBox
    // type startup is supplied here. No copied IDs or fixed widget-type tags.
    virtual bool call_class_query_0c(void* actual_widget,
        std::uint32_t captured_target, std::uint32_t descriptor) = 0;
    // Exact CURRENT0(flags1) over that SAME timed allocation and its real
    // matching allocator/custody. Retire its existing identity if applicable;
    // never interpret remaining-time+04 as a reference count or make progress
    // by erasing the widget's slot. AA9730 performs its own post-call clear.
    virtual void call_timed_delete_00(void* actual_entry,
        std::uint32_t captured_target, std::uint32_t flags) = 0;
};

struct NativeGuiWidgetLifetimeDispatchProfiles {
    const volatile std::uint32_t* group_00d5cb80; // at least9 DWORDs
    const volatile std::uint32_t* clip_box_00d5d058; // at least9 DWORDs
};

// Metadata-only production binding for a CLOSED raw Group/ClipBox hierarchy.
// Construct widgets through their existing raw AA9390/AA9520 factories, then
// separately admit the SAME completed allocation with Group/ClipBoxReference
// using context(). All reachable widget children/parents must be admitted in
// this SAME registry/context; the same node companions/resources and every
// transitive list/timed backing/provider remain caller-retained through calls
// and failure disposition. Tables/contexts may not be moved or mutated during
// dispatch. Neither raw links/counts nor registry lookup prove full custody.
//
// This owns only the explicit lifetime context. No widget/node allocation,
// native count/credit, owner map, layout/field mirror or automatic teardown.
// AA9730 performs self-detachment in native order via this exact A9BD50 binding;
// no pre-delete AA83A0 call or fallback erase manufactures child-loop progress.
// Producers may use the existing genuine AA83A0 helper for its own native site.
// Unknown canonical owners/widgets/targets reject. Unregistered non-widget
// current-table lookups belong ONLY to the mandatory timed provider, which
// must reject allocations it does not genuinely own/recognize.
//
// New Source orchestration ABI. Original FH3/SEH/register ABI, malformed or
// racing storage, all18 classes, page/factory activation and gameplay unproved.
class NativeGuiWidgetLifetimeDispatch final : public NativeGuiWidgetLifetimeBindings {
public:
    NativeGuiWidgetLifetimeDispatch(NativeNodeDestructionRuntime&,
        NativeRenderActualOwnerRegistry&, const volatile std::uint32_t* actual_base_table_00d5c130,
        const volatile std::uint32_t (&actual_base_lineage_00f8bc88)[2],
        const volatile std::uint32_t& actual_text_descriptor_00f8be28,
        NativeGuiWidgetLifetimeDispatchProfiles, NativeGuiWidgetLifetimeRequiredCalls&);
    NativeGuiWidgetLifetimeDispatch(const NativeGuiWidgetLifetimeDispatch&) = delete;
    NativeGuiWidgetLifetimeDispatch& operator=(const NativeGuiWidgetLifetimeDispatch&) = delete;
    NativeGuiWidgetLifetimeDispatch(NativeGuiWidgetLifetimeDispatch&&) = delete;
    NativeGuiWidgetLifetimeDispatch& operator=(NativeGuiWidgetLifetimeDispatch&&) = delete;

    NativeGuiWidgetLifetimeContext& context() noexcept { return context_; }
    const volatile std::uint32_t* current_table(void*, std::uint32_t captured_profile) override;
    bool call_class_query_0c(void*, std::uint32_t captured_target, std::uint32_t descriptor) override;
    void call_child_scene_20(void*, std::uint32_t captured_target) override;
    void call_child_delete_04(void*, std::uint32_t captured_target, std::uint32_t flags) override;
    void call_timed_delete_00(void*, std::uint32_t captured_target, std::uint32_t flags) override;
    void remove_all_00a9bd50(void* actual_list, const void* pointer_cell) override;
private:
    enum class Kind { group, clip_box };
    struct BorrowedWidget { Kind kind; RenderCommandReference* reference; };
    BorrowedWidget widget(void* actual);
    void require_timed_identity(void* actual);
    NativeRenderActualOwnerRegistry& registry_;
    const NativeGuiWidgetLifetimeDispatchProfiles profiles_;
    NativeGuiWidgetLifetimeRequiredCalls& required_;
    NativeGuiWidgetLifetimeContext context_;
};
} // namespace bsp
