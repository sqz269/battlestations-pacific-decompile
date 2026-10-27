#pragma once
#include "bsp/native_gui_widget_copy_storage.hpp"
#include "bsp/native_gui_widget_lifetime.hpp"
#include "bsp/native_live_effect_manager.hpp"
#include "bsp/native_string.hpp"
#include <cstdint>

namespace bsp {
// Same actual EC..FB tail; EC/F0 are borrowed DWORDs, F4/F8 the actual pooled
// string length/data header. FC belongs to the pool's separate100h slot.
struct NativeGuiButtonTailStorage {
    std::uint32_t listener_ec;
    std::uint32_t label_widget_f0;
    std::uint32_t label_length_f4;
    char* label_data_f8;
};
NativeGuiButtonTailStorage& native_gui_button_tail(void* actual_button);

// Borrow ONLY the sole process producer's SAME initialized actual0Ch list and
// focus DWORD. No list/sentinel/global owner, count mirror, retain or private
// initialization is supplied. Preflight is outside native operations. The
// initialized finite ring, allocator and focus storage stay live across calls.
struct NativeGuiButtonRegistryBinding {
    NativeEffectDeletionListStorage& actual_list_00f8bd38;
    volatile std::uint32_t& actual_focused_00f8bcf8;
    NativeGuiButtonRegistryBinding(NativeEffectDeletionListStorage&,
        volatile std::uint32_t& actual_focused);
};
struct NativeGuiButtonLifetimeContext {
    NativeGuiWidgetLifetimeContext& base;
    NativeStringRawPoolContext& strings;
    NativeGuiButtonRegistryBinding& registry;
};
struct NativeGuiButtonDefaultAcquired {
    bool started{}, base_returned{}, label_cleanup_armed{}, registry_entered{};
    // Diagnostic pointer, NOT another list/node owner. A growth failure leaves
    // this node unlinked in the original; no rollback/free is invented here.
    NativeEffectDeletionNode* registry_node{};
    bool count_incremented{}, link_started{}, complete{};
    bool label_cleanup_entered{}, base_cleanup_entered{};
};
// The copy arm reaches no Button registry/focus input. It still needs the real
// base destruction and string domains for its observed constructor unwind.
struct NativeGuiButtonCopyContext {
    NativeGuiWidgetCopyContext& base;
    NativeGuiWidgetLifetimeContext& lifetime;
    NativeStringRawPoolContext& strings;
};
struct NativeGuiButtonCopyAcquired {
    NativeGuiWidgetCopyAcquired base;
    bool base_returned{}, label_copy_entered{}, complete{}, base_cleanup_entered{};
};

// AAC6F0 ECX actual, EAX same, RET. AA9390(type15), D5BED0, EC..F8 zero;
// capture registry head/tail before allocation, buy0Ch node, grow CURRENT count,
// link through captured head+4 then CURRENT node.previous. Genuine stock list
// routines reuse the existing SAME raw allocator/list implementation. Failure
// destroys string then base; the fresh acquired frame preserves reached work.
void* construct_native_gui_button_default_00aac6f0(void* actual_button,
    const volatile std::uint32_t& actual_one_00d7a24c,
    NativeGuiButtonLifetimeContext&, NativeGuiButtonDefaultAcquired&);
// AA3C60 ECX destination, stack source, EAX same, RET4. Raw AA9520, D5BED0,
// DWORD EC/F0 copies and same actual F4 string copy. Copy NEVER registers.
// Failed string construction calls only AA9730, retaining partial string state.
void* construct_native_gui_button_copy_00aa3c60(void* actual_destination,
    const void* actual_source, NativeGuiButtonCopyContext&, NativeGuiButtonCopyAcquired&);
// Complete AA4440 null/non-null arms, split to require only reached inputs.
// Explicit GuiButtonPoolTraits startup first; constructor failure returns its
// captured failed slot after the constructor's own cleanup, not another slot.
void* create_native_gui_button_default_00aa4440(
    const volatile std::uint32_t& actual_one_00d7a24c,
    NativeGuiButtonLifetimeContext&, NativeGuiButtonDefaultAcquired&);
void* create_native_gui_button_copy_00aa4440(const void* actual_source,
    NativeGuiButtonCopyContext&, NativeGuiButtonCopyAcquired&);

// AAC530 ECX actual, RET. Stamp Button, remove ALL matches from SAME registry,
// clear ANY nonzero actual focus without identity comparison, release CURRENT
// F4 string, then raw AA9730. Derived failure consumes base-only unwind once.
void destroy_native_gui_button_00aac530(void* actual_button, NativeGuiButtonLifetimeContext&);
// AA3D10 ECX actual, stack flags, EAX saved identity, RET4. Full destructor,
// CURRENT low flags byte afterward, then SAME AABD60 pool return iff bit0.
// Explicit scalar deletion has no count decrement or count test.
void* delete_native_gui_button_00aa3d10(void* actual_button,
    const volatile std::uint32_t& actual_flags_slot, NativeGuiButtonLifetimeContext&);
} // namespace bsp
