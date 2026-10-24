#pragma once
#include "bsp/native_gui_widget_copy_storage.hpp"
#include "bsp/native_gui_widget_lifetime.hpp"
#include "bsp/native_string.hpp"
#include <cstdint>

namespace bsp {
// Same actual EC..FB tail of the FCh object. F8 is a DWORD, not a C++ bool.
// The pool-owned slab index at FC belongs to the separate100h slot.
struct NativeGuiScrollbarTailStorage {
    float value_ec;
    float minimum_f0;
    float maximum_f4;
    std::uint32_t horizontal_f8;
};
NativeGuiScrollbarTailStorage& native_gui_scrollbar_tail(void* actual_scrollbar);

// Required genuine dispatch over the same raw widget/tree. Lookup is free of
// side effects; its table stays live across the reached call. Numeric profiles
// and entries identify source implementations and are never called as code.
// In particular current7C is the real child-layout operation (normally AC8A80),
// not a successful empty callback. Unbound tables/targets must be rejected.
class NativeGuiScrollbarCalls {
public:
    virtual ~NativeGuiScrollbarCalls() = default;
    virtual const volatile std::uint32_t* current_table(void* actual_widget,
        std::uint32_t captured_profile) = 0;
    virtual void call_layout_7c(void* actual_widget,
        std::uint32_t captured_target) = 0;
    // Actual8h pooled-string header, not a std::string or logical child name.
    virtual void* find_child_00aaa2b0(void* actual_widget,
        const void* actual_name_header) = 0;
    virtual void loaded_base_00aa7170(void* actual_widget) = 0;
};

// AC7C20: ECX actual, EAX same, RET. Existing raw AA9390(type7), then D5CCF8,
// EC/F0/F4=+0, F8=1. No enlarged allocation, copied base state or second owner.
void* construct_native_gui_scrollbar_default_00ac7c20(void* actual_scrollbar,
    const volatile std::uint32_t& actual_one_00d7a24c);
// AC7CB0: ECX destination, stack source, EAX same, RET4. Genuine AA9520 first;
// FLD/FSTP three tail floats, then copy DWORD F8. Same fresh base-copy frame;
// no derived allocation or additional successful/failed native owner is made.
void* construct_native_gui_scrollbar_copy_00ac7cb0(void* actual_destination,
    const void* actual_source, NativeGuiWidgetCopyContext&, NativeGuiWidgetCopyAcquired&);
// Complete null/non-null AA15C0 arms, split to require only reached inputs.
// Explicit GuiScrollbarPoolTraits process startup precedes allocation. A failed
// constructor returns only its captured slot, after that constructor's cleanup.
void* create_native_gui_scrollbar_default_00aa15c0(
    const volatile std::uint32_t& actual_one_00d7a24c);
void* create_native_gui_scrollbar_copy_00aa15c0(const void* actual_source,
    NativeGuiWidgetCopyContext&, NativeGuiWidgetCopyAcquired&);

// AC7CA0 stamps D5CCF8 and tail-jumps to genuine AA9730; no tail storage free.
void destroy_native_gui_scrollbar_00ac7ca0(void* actual_scrollbar,
    NativeGuiWidgetLifetimeContext&);
// AC8210 explicit scalar: same full base destruction, then CURRENT low flags
// byte; return the SAME F8C004 slot iff bit0. No count decrement or count test.
void* delete_native_gui_scrollbar_00ac8210(void* actual_scrollbar,
    const volatile std::uint32_t& actual_flags_slot, NativeGuiWidgetLifetimeContext&);

// ECX actual; original stack argument is one float DWORD, RET4. New interfaces
// borrow that actual DWORD slot rather than making an early C++ float copy.
// Preserve x87 unordered branches/spills and the current7C call schedule. Value
// always calls7C. Min/max call it twice when their first clamp calls SetValue.
void set_native_gui_scrollbar_value_00ac7d00(void* actual_scrollbar,
    const volatile std::uint32_t& actual_float_argument_slot, NativeGuiScrollbarCalls&);
void set_native_gui_scrollbar_minimum_00ac7d80(void* actual_scrollbar,
    const volatile std::uint32_t& actual_float_argument_slot, NativeGuiScrollbarCalls&);
void set_native_gui_scrollbar_maximum_00ac7de0(void* actual_scrollbar,
    const volatile std::uint32_t& actual_float_argument_slot, NativeGuiScrollbarCalls&);

// Full AC8890 normal path: actual pooled temporary strings and four genuine
// AAA2B0 lookups (results discarded), then SetValue(current minimum) and genuine
// AA7170. No child pointer cache, page ownership or extra native retain is made.
void loaded_native_gui_scrollbar_00ac8890(void* actual_scrollbar,
    NativeStringRawPoolContext&, NativeGuiScrollbarCalls&);

// Only AC86DE..AC872A after SAME caller's successful AAA710/visitor reads have
// already published raw F8/F0/F4. Native min call, late max clamp and current7C;
// not the full AC85A0 reader or its Alignment string/visitor cleanup. A full raw
// property visitor remains an explicit prerequisite; no typed-state projection.
void finish_native_gui_scrollbar_properties_after_reads_00ac85a0(
    void* actual_scrollbar, NativeGuiScrollbarCalls&);
} // namespace bsp
