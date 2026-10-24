#include "bsp/native_gui_scrollbar_storage.hpp"
#include "bsp/native_gui_widget_base_storage.hpp"
#include "bsp/native_gui_widget_pools_cc13.hpp"
#include <cstddef>
#include <cstring>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeGuiScrollbarTailStorage) == 0x10);
static_assert(offsetof(NativeGuiScrollbarTailStorage, horizontal_f8) == 0xc);
static_assert(sizeof(NativeString) == 8);
void require_storage(void* raw) {
    if (!raw || reinterpret_cast<std::uintptr_t>(raw) % 4)
        throw std::invalid_argument("native Scrollbar requires its aligned actual FCh storage");
}
volatile Word& word(void* raw, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile Word*>(static_cast<std::byte*>(raw) + offset);
}
Word layout_target(void* raw, Word captured_profile, NativeGuiScrollbarCalls& calls) {
    const volatile Word* const table = calls.current_table(raw, captured_profile);
    if (!table)
        throw std::logic_error("Scrollbar requires its genuine current7C layout binding");
    const Word target = table[0x7c / 4];
    if (!target)
        throw std::logic_error("Scrollbar requires its genuine current7C layout binding");
    return target;
}
void call_current_layout(void* raw, NativeGuiScrollbarCalls& calls) {
    const Word captured_profile = word(raw, 0);
    const Word captured_target = layout_target(raw, captured_profile, calls);
    calls.call_layout_7c(raw, captured_target);
}
void call_captured_layout(void* raw, Word captured_profile, NativeGuiScrollbarCalls& calls) {
    const Word captured_target = layout_target(raw, captured_profile, calls);
    calls.call_layout_7c(raw, captured_target);
}
void return_failed_slot(void* slot) noexcept {
    try { free_gui_widget_pool_slot<GuiScrollbarPoolTraits>(slot); }
    catch (...) { std::terminate(); }
}
void copy_tail(void* destination, const void* source) {
    __asm {
        mov eax, destination
        mov edx, source
        fld dword ptr [edx + 0xec]
        fstp dword ptr [eax + 0xec]
        fld dword ptr [edx + 0xf0]
        fstp dword ptr [eax + 0xf0]
        fld dword ptr [edx + 0xf4]
        fstp dword ptr [eax + 0xf4]
        mov ecx, dword ptr [edx + 0xf8]
        mov dword ptr [eax + 0xf8], ecx
    }
}
struct ValueSelection { Word captured_profile; Word value; };
// Keep the exact FLD/FSTP and FCOMIP schedule. JBE also takes unordered;
// an in-range/unordered argument is reloaded bitwise AFTER profile capture.
ValueSelection select_value(void* raw, const volatile Word* argument) {
    Word rounded_bound;
    Word captured_profile;
    Word value;
    __asm {
        mov ecx, raw
        mov edx, argument
        fld dword ptr [ecx + 0xf0]
        fstp dword ptr rounded_bound
        fld dword ptr [edx]
        fld dword ptr rounded_bound
        fcomip st(0), st(1)
        jbe value_upper
        mov eax, dword ptr [ecx]
        fstp st(0)
        movss xmm0, dword ptr rounded_bound
        jmp value_done
    value_upper:
        fld dword ptr [ecx + 0xf4]
        fstp dword ptr rounded_bound
        fld dword ptr rounded_bound
        fxch st(1)
        fcomip st(0), st(1)
        fstp st(0)
        jbe value_argument
        mov eax, dword ptr [ecx]
        movss xmm0, dword ptr rounded_bound
        jmp value_done
    value_argument:
        mov eax, dword ptr [ecx]
        movss xmm0, dword ptr [edx]
    value_done:
        mov captured_profile, eax
        movss dword ptr value, xmm0
    }
    return {captured_profile, value};
}
struct ClampArgument { Word needed; Word spilled; Word captured_final_profile; };
ClampArgument store_minimum(void* raw, const volatile Word* argument) {
    Word needed;
    Word spilled = 0;
    Word captured_final_profile = 0;
    __asm {
        mov ecx, raw
        mov edx, argument
        movss xmm0, dword ptr [edx]
        fld dword ptr [ecx + 0xf4]
        movss dword ptr [ecx + 0xf0], xmm0
        fld dword ptr [edx]
        fcomi st(0), st(1)
        fstp st(1)
        jbe minimum_value
        movss dword ptr [ecx + 0xf4], xmm0
    minimum_value:
        fld dword ptr [ecx + 0xec]
        fxch st(1)
        fcomi st(0), st(1)
        fstp st(1)
        jbe minimum_done
        fstp dword ptr spilled
        mov needed, 1
        jmp minimum_return
    minimum_done:
        mov eax, dword ptr [ecx]
        fstp st(0)
        mov captured_final_profile, eax
        mov needed, 0
    minimum_return:
    }
    return {needed, spilled, captured_final_profile};
}
ClampArgument store_maximum(void* raw, const volatile Word* argument) {
    Word needed;
    Word spilled = 0;
    Word captured_final_profile = 0;
    __asm {
        mov ecx, raw
        mov edx, argument
        fld dword ptr [edx]
        movss xmm0, dword ptr [edx]
        fld dword ptr [ecx + 0xf0]
        movss dword ptr [ecx + 0xf4], xmm0
        fcomip st(0), st(1)
        jbe maximum_value
        movss dword ptr [ecx + 0xf0], xmm0
    maximum_value:
        fld dword ptr [ecx + 0xec]
        fcomip st(0), st(1)
        jbe maximum_done
        fstp dword ptr spilled
        mov needed, 1
        jmp maximum_return
    maximum_done:
        mov eax, dword ptr [ecx]
        fstp st(0)
        mov captured_final_profile, eax
        mov needed, 0
    maximum_return:
    }
    return {needed, spilled, captured_final_profile};
}
Word spill_current_minimum(void* raw) {
    Word result;
    __asm {
        mov ecx, raw
        fld dword ptr [ecx + 0xf0]
        fstp dword ptr result
    }
    return result;
}
ClampArgument property_maximum_tail(void* raw) {
    Word maximum;
    Word needed;
    Word spilled = 0;
    __asm {
        mov ecx, raw
        fld dword ptr [ecx + 0xf4]
        fstp dword ptr maximum
        fld dword ptr maximum
        fld dword ptr [ecx + 0xf0]
        fcomip st(0), st(1)
        jbe property_value
        movss xmm0, dword ptr maximum
        movss dword ptr [ecx + 0xf0], xmm0
    property_value:
        fld dword ptr [ecx + 0xec]
        fcomip st(0), st(1)
        jbe property_done
        fstp dword ptr spilled
        mov needed, 1
        jmp property_return
    property_done:
        fstp st(0)
        mov needed, 0
    property_return:
    }
    return {needed, spilled, 0};
}
void lookup_loaded_child(void* raw, const char* literal, Word length,
    NativeStringRawPoolContext& strings, NativeGuiScrollbarCalls& calls) {
    NativeString name;
    resize_native_string_header_0041dd40(&name, strings, length, true);
    if (char* const data = name.data())
        std::memcpy(data, literal, static_cast<std::size_t>(name.block_size()));
    // The native EH state covers AAA2B0 only. Normal string return runs after
    // clearing that state; a failed return must not retry the same header.
    try { (void)calls.find_child_00aaa2b0(raw, &name); }
    catch (...) {
        try { destroy_native_string_header_0041dd20(&name, strings); }
        catch (...) { std::terminate(); }
        throw;
    }
    destroy_native_string_header_0041dd20(&name, strings);
}
} // namespace
NativeGuiScrollbarTailStorage& native_gui_scrollbar_tail(void* raw) {
    require_storage(raw);
    return *reinterpret_cast<NativeGuiScrollbarTailStorage*>(static_cast<std::byte*>(raw) + 0xec);
}
void* construct_native_gui_scrollbar_default_00ac7c20(void* raw, const volatile Word& one) {
    require_storage(raw);
    construct_native_gui_widget_base_00aa9390(raw, 7, one);
    word(raw, 0) = 0x00d5ccf8;
    word(raw, 0xec) = 0;
    word(raw, 0xf0) = 0;
    word(raw, 0xf4) = 0;
    word(raw, 0xf8) = 1;
    return raw;
}
void* construct_native_gui_scrollbar_copy_00ac7cb0(void* raw, const void* source,
    NativeGuiWidgetCopyContext& context, NativeGuiWidgetCopyAcquired& acquired) {
    require_storage(raw);
    if (!source) throw std::invalid_argument("Scrollbar copy requires its captured live source");
    construct_native_gui_widget_copy_00aa9520(raw, source, context, acquired);
    word(raw, 0) = 0x00d5ccf8;
    copy_tail(raw, source);
    return raw;
}
void* create_native_gui_scrollbar_default_00aa15c0(const volatile Word& one) {
    void* const slot = allocate_gui_widget_pool_slot<GuiScrollbarPoolTraits>();
    if (!slot) return nullptr;
    try { return construct_native_gui_scrollbar_default_00ac7c20(slot, one); }
    catch (...) { return_failed_slot(slot); throw; }
}
void* create_native_gui_scrollbar_copy_00aa15c0(const void* source,
    NativeGuiWidgetCopyContext& context, NativeGuiWidgetCopyAcquired& acquired) {
    if (!source) throw std::invalid_argument("Scrollbar copy arm requires its nonnull native source");
    const void* const captured_source = source;
    void* const slot = allocate_gui_widget_pool_slot<GuiScrollbarPoolTraits>();
    if (!slot) return nullptr;
    try { return construct_native_gui_scrollbar_copy_00ac7cb0(slot, captured_source, context, acquired); }
    catch (...) { return_failed_slot(slot); throw; }
}
void destroy_native_gui_scrollbar_00ac7ca0(void* raw, NativeGuiWidgetLifetimeContext& context) {
    require_storage(raw);
    word(raw, 0) = 0x00d5ccf8;
    destroy_native_gui_widget_base_00aa9730(raw, context);
}
void* delete_native_gui_scrollbar_00ac8210(void* raw, const volatile Word& flags,
    NativeGuiWidgetLifetimeContext& context) {
    destroy_native_gui_scrollbar_00ac7ca0(raw, context);
    const auto low = *reinterpret_cast<const volatile unsigned char*>(&flags);
    if ((low & 1u) != 0) free_gui_widget_pool_slot<GuiScrollbarPoolTraits>(raw);
    return raw;
}
void set_native_gui_scrollbar_value_00ac7d00(void* raw, const volatile Word& argument,
    NativeGuiScrollbarCalls& calls) {
    require_storage(raw);
    const ValueSelection selected = select_value(raw, &argument);
    const Word captured_target = layout_target(raw, selected.captured_profile, calls);
    word(raw, 0xec) = selected.value;
    calls.call_layout_7c(raw, captured_target);
}
void set_native_gui_scrollbar_minimum_00ac7d80(void* raw, const volatile Word& argument,
    NativeGuiScrollbarCalls& calls) {
    require_storage(raw);
    const ClampArgument clamp = store_minimum(raw, &argument);
    if (clamp.needed) {
        set_native_gui_scrollbar_value_00ac7d00(raw, clamp.spilled, calls);
        call_current_layout(raw, calls);
    } else call_captured_layout(raw, clamp.captured_final_profile, calls);
}
void set_native_gui_scrollbar_maximum_00ac7de0(void* raw, const volatile Word& argument,
    NativeGuiScrollbarCalls& calls) {
    require_storage(raw);
    const ClampArgument clamp = store_maximum(raw, &argument);
    if (clamp.needed) {
        set_native_gui_scrollbar_value_00ac7d00(raw, clamp.spilled, calls);
        call_current_layout(raw, calls);
    } else call_captured_layout(raw, clamp.captured_final_profile, calls);
}
void loaded_native_gui_scrollbar_00ac8890(void* raw,
    NativeStringRawPoolContext& strings, NativeGuiScrollbarCalls& calls) {
    require_storage(raw);
    lookup_loaded_child(raw, "_BackArrow_Icon", 0xf, strings, calls); // D5CDF0
    lookup_loaded_child(raw, "_ForwArrow_Icon", 0xf, strings, calls); // D5CDE0
    lookup_loaded_child(raw, "_Background_Icon", 0x10, strings, calls); // D5CDCC
    lookup_loaded_child(raw, "_Slider_Icon", 0xc, strings, calls); // D5CDBC
    const Word minimum = spill_current_minimum(raw);
    set_native_gui_scrollbar_value_00ac7d00(raw, minimum, calls);
    calls.loaded_base_00aa7170(raw);
}
void finish_native_gui_scrollbar_properties_after_reads_00ac85a0(void* raw,
    NativeGuiScrollbarCalls& calls) {
    require_storage(raw);
    const Word minimum = spill_current_minimum(raw);
    set_native_gui_scrollbar_minimum_00ac7d80(raw, minimum, calls);
    const ClampArgument clamp = property_maximum_tail(raw);
    if (clamp.needed) set_native_gui_scrollbar_value_00ac7d00(raw, clamp.spilled, calls);
    call_current_layout(raw, calls);
}
} // namespace bsp
