#pragma once
#include "bsp/native_gui_widget_copy_storage.hpp"
#include <cstdint>

namespace bsp {
struct NativeGuiWidgetLifetimeContext;
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

// ACE620: complete38B native scalar deleting body; ECX actual ClipBox,
// stack flags DWORD, EAX saved identity, RET4. Stamp D5D058, genuine raw
// AA9730, then read ONLY the caller flags cell's low byte after base success.
// Bit0 returns the SAME actual F8C13C slot (10C payload/110 stride); no count
// gate/decrement, derived-field cleanup or payload read after slot return.
// Flags0 still ends base lifetime; either return value is only its identity.
//
// The borrowed flags cell may be a caller local/actual argument slot. Keep it
// stable and disjoint from the active widget and OTHER mutable provider/scratch
// storage that could be consumed or repurposed. Serialized callbacks may
// intentionally update flags; those updates are observed after base success.
// No private native argument-address ABI is promised. Keep the SAME
// initialized process pool, whole partial graph and every context/service
// alive BEFORE entry and across callbacks. Base unwind can destroy container
// links without deleting unfinished child/timed payloads. On base failure,
// no flags read or pool return occurs; retain external graph owners, not just
// the failed raw slot. No replay, reset, fallback catch or automatic retention.
// Populated-graph bindings remain required. New Source ABI; no new runtime,
// native FH3/SEH, full graph retirement or Original binary ABI claim.
void* delete_native_gui_clip_box_00ace620(void* actual_clip_box,
    const volatile std::uint32_t& actual_flags_slot,
    NativeGuiWidgetLifetimeContext&);
} // namespace bsp
