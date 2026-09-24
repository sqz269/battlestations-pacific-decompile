#pragma once
#include "bsp/native_node_destruction.hpp"
#include <cstdint>

namespace bsp {
// Required bindings for CURRENT raw widget/entry dispatch and the MSVC list
// remove specialization. No logical GuiWidgetOwner/GuiLayoutWidget conversion.
// Lookup is side-effect-free; returned actual tables remain live across calls.
// Numeric entries identify source implementations and are never invoked as code.
class NativeGuiWidgetLifetimeBindings {
public:
    virtual ~NativeGuiWidgetLifetimeBindings() = default;
    virtual const volatile std::uint32_t* current_table(void* actual_owner,
        std::uint32_t captured_profile) = 0;
    virtual bool call_class_query_0c(void* actual_widget,
        std::uint32_t captured_target, std::uint32_t descriptor) = 0;
    virtual void call_child_scene_20(void* actual_child,
        std::uint32_t captured_target) = 0;
    // Native AA9730 passes flags1 regardless of reference count. The real
    // derived destructor MUST detach itself from the parent's SAME +64 list.
    // The caller reloads head/count and supplies no fallback erase/decrement.
    virtual void call_child_delete_04(void* actual_child,
        std::uint32_t captured_target, std::uint32_t flags) = 0;
    virtual void call_timed_delete_00(void* actual_entry,
        std::uint32_t captured_target, std::uint32_t flags) = 0;
    // Complete A9BD50 library contract: ECX actual {opaque,head,count} list,
    // stack address of pointer cell. Capture *cell and initial end, remove ALL
    // equal node+8 values with original iterator checks, unlink/free each
    // matching 0Ch node and decrement CURRENT count. Preserve other payloads,
    // allocator word and sentinel. No successful default implementation.
    virtual void remove_all_00a9bd50(void* actual_list, const void* pointer_cell) = 0;
};

struct NativeGuiWidgetLifetimeContext {
    NativeNodeDestructionRuntime& nodes; // existing ACTUAL node companions only
    const volatile std::uint32_t* actual_base_table_00d5c130; // at least9 DWORDs
    const volatile std::uint32_t (&actual_base_lineage_00f8bc88)[2];
    const volatile std::uint32_t& actual_text_descriptor_00f8be28;
    NativeGuiWidgetLifetimeBindings& bindings;
};

// Exact borrowed raw-container aliases. Same allocation/free domain and actual
// +0/+4/+8 header as the existing renderer pointer arrays; no new container.
void reserve_native_gui_timed_pointers_00aa6f30(void*, std::int32_t);
void resize_native_gui_timed_pointers_00aa77b0(void*, std::int32_t);
void destroy_native_gui_timed_pointers_00aa7f50(void*);
// A9B740 full72B; A9BCE0 tail alias. Same physical 0Ch list as its producer
// A9B720 and existing NativeEffectDeletionListStorage. Never deletes payloads.
void destroy_native_gui_widget_list_00a9b740(void*) noexcept;
void destroy_native_gui_widget_list_thunk_00a9bce0(void*) noexcept;
bool native_gui_widget_is_kind_of_00a9e070(std::uint32_t descriptor,
    const volatile std::uint32_t (&actual_lineage)[2]) noexcept;
std::uint32_t native_gui_text_descriptor_00ab6a30(
    const volatile std::uint32_t& actual_descriptor) noexcept;
void release_native_gui_text_glyph_00ab73b0(void* actual_text,
    NativeGuiWidgetLifetimeContext&);

// Complete normal schedules over AA9390-created raw storage. AA8320 walks
// current child20, captures current0C cell before AB6A30, conditionally releases
// raw Text+188, then main+4C. AA9730 publishes D5C130, calls AA8320, repeatedly
// deletes current first child04(1), detaches parent, releases any replacement
// main node, deletes timed entries current0(1), destroys containers, then
// D5C104 -> BD30F0/CEB130. Retains +04 count; never frees the widget slot.
void release_native_gui_widget_scene_00aa8320(void* actual_widget,
    NativeGuiWidgetLifetimeContext&);
void destroy_native_gui_widget_base_00aa9730(void* actual_widget,
    NativeGuiWidgetLifetimeContext&);

// Fresh raw base and AC6600-prefix cleanup is supported with real containers
// and same-node owners. Raw child scalar destructors, timed scalar owners,
// arbitrary current tables and A9BD50 remain required external bindings.
// C++ exception cleanup follows native states2/1/0: timed backing (NO entry
// deletion), child-list storage (NO child deletion), AA6E10. A second cleanup
// exception terminates. No restart/implicit RAII or page-name/pool cleanup.
// Caller owns unfinished derived state and retained raw slot after failure.
// New source ABI; native FH3/SEH, corrupt storage and gameplay are unproved.
} // namespace bsp
