#pragma once
#include "bsp/native_gui_widget_copy_storage.hpp"
#include <cstdint>

namespace bsp {
// ACE0A0: native ECX actual destination, EAX same destination, RET. Complete
// 20-byte adapter: genuine AA9390(type16), then stamp D5D058. Borrows the SAME
// live D7A24C scalar; leaves every derived EC..108 and unwritten base byte.
void* construct_native_gui_clip_box_default_00ace0a0(void* actual_destination,
    const volatile std::uint32_t& actual_one_00d7a24c);

// ACE0F0: native ECX destination, stack actual source, EAX destination, RET4.
// Complete 25-byte adapter: genuine AA9520, then stamp D5D058. Inherits its
// fresh one-shot acquisition frame, actual Model26/3E,parent0 domain, current
// clone flags/profile bindings, source/context lifetime and failure contract.
// No derived fields, model publication, source retain or fallback copy added.
void* construct_native_gui_clip_box_copy_00ace0f0(void* actual_destination,
    const void* actual_source, NativeGuiWidgetCopyContext&,
    NativeGuiWidgetCopyAcquired&);

// AA1920: native ECX optional actual source, EAX slot/null, RET. Uses the SAME
// initialized GameNativeGuiWidgetPoolProcess<GuiClipBoxPoolTraits>, F8C13C,
// payload10C/stride110. Does not initialize/register it or allocate a new pool.
// Captures source before ACEA10 allocation; null selects genuine ACE0A0,
// nonnull selects genuine ACE0F0. Null allocation/default leave copy diagnostics
// untouched. Every source/context/actual owner domain outlives callbacks and
// resulting widgets; caller owns creator publication and eventual deletion.
//
// A constructor C++ exception first completes its own timed/list/ref cleanup,
// then this wrapper returns ONLY its failed widget slot (CB6F40/48 -> ACE610).
// A slot-return exception terminates. Independent Model clone acquisitions can
// remain in the caller's persistent frame; they are neither inspected nor
// released here. The failed widget slot is no longer live: saved destination
// identities/diagnostics never authorize payload access or retry. Keep all
// retained Model creators/metadata and their services alive for explicit
// recovery. This is not whole-graph cleanup or a successful-payload destructor.
// New Source binding ABI, not private native FH3/SEH/binary ABI equivalence.
void* create_native_gui_clip_box_00aa1920(const void* optional_actual_source,
    const volatile std::uint32_t& actual_one_00d7a24c,
    NativeGuiWidgetCopyContext&, NativeGuiWidgetCopyAcquired&);
} // namespace bsp
