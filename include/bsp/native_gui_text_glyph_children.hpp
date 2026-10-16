#pragma once
#include "bsp/native_gui_widget_lifetime.hpp"

namespace bsp {
struct NativeGuiTextGlyphChildrenContext {
    NativeNodeDestructionRuntime& nodes;
    NativeGuiWidgetLifetimeBindings& children;
};

// Diagnostics only: no owning aliases, implicit cleanup or retry. Retain this
// complete record independently if a callback destroys a captured child. Each
// active recursive invocation requires a distinct fresh persistent record.
// Public fields do not enforce identity: caller must never reset, assign or
// reuse a record after a reached entry, including an interrupted invocation.
struct NativeGuiTextGlyphChildrenAcquired {
    enum class Boundary {
        none, invalid_parameter, detach, current_table, child_delete, memmove
    };
    bool started{}, complete{}, boundary_returned{};
    Boundary boundary{Boundary::none};
    std::uint32_t active_call_site{}, index{}, loop_begin{}, detach_child{};
    std::uint32_t captured_slot{}, captured_child{}, captured_profile{}, captured_target{};
    std::uint32_t captured_end{}, erase_begin{}, erase_bytes{}, erase_new_end{};
    std::uint32_t detach_calls_entered{}, detach_calls_returned{};
    std::uint32_t delete_calls_entered{}, delete_calls_returned{}, slot_clears{};
};

// Complete AB80C0[247B;79 actual starts plus unreachable6B padding]. Native
// ECX actual Text, no arguments, RET; this new C++ interface is not native ABI.
// Reads the SAME actual198/19C pointer vector; no GuiWidgetOwner/Text cast,
// semantic vector, retain, allocation, fallback child terminal or new owner.
//
// Entry preflight checks fresh diagnostics and known disjoint context/receiver
// regions before any raw access. Caller additionally holds the WHOLE graph:
// valid finite serialized raw vector/backing, actual parent/list/node/child
// creators, current tables, contexts and all prepared persistent recursive
// frames. They are disjoint from this record and survive reached callbacks and
// failure disposition. A deleting callback cannot invalidate the CAPTURED slot
// before its post-return clear; later iterations reload current begin/end.
// These known-region checks do not prove transitive backing/graph separation.
// Current-table lookup is pure/stable borrowed metadata, not a native call or
// terminal: it cannot reenter, mutate the graph or invalidate captured values.
// Required child04(flags1) must execute the genuine captured current target.
// Nonempty raw Text children still require AB8EE0->AB8250 recursion; no such
// provider or production admission is supplied here. Empty is one real arm.
//
// BF6713 uses existing qualified current-SDK invalid-parameter transport and
// may return; execution then continues at the Original continuation. BF67A7
// uses the current SDK memmove_s with captured four-word operands, ignoring its
// result. Invalid/corrupt storage, arbitrary diagnostic handler mutation and
// Original CRT/FH3/SEH/private-stack equivalence are not admitted. The actual
// ordered checks and erase tail remain present; no invalid-index truncation.
//
// Boundary metadata is consumed before calls and remains sticky on failure.
// It does NOT classify Native versus host/provider exception origin. There is
// no catch, rollback, destructor cleanup, slot clearing on failed child return
// or retry. Keep retained creator/diagnostic state independently; never recover
// ownership or an ended payload by rereading stale raw fields after failure.
void clear_native_gui_text_glyph_children_00ab80c0(void* actual_text,
    NativeGuiTextGlyphChildrenContext&, NativeGuiTextGlyphChildrenAcquired&);
} // namespace bsp
