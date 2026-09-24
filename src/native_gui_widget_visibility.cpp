#include "bsp/native_gui_widget_visibility.hpp"
#include "bsp/native_node_visibility_factor.hpp"
#include <cstddef>
#include <cstdlib>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI widget visibility requires MSVC Win32.
#endif

namespace bsp {
namespace {
std::uint32_t word(const void* p, std::size_t offset = 0) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(
        static_cast<const std::byte*>(p) + offset);
}
std::uint8_t byte(const void* p, std::size_t offset) noexcept {
    return *reinterpret_cast<const volatile std::uint8_t*>(
        static_cast<const std::byte*>(p) + offset);
}
void* pointer(const void* p, std::size_t offset = 0) noexcept {
    return reinterpret_cast<void*>(word(p, offset));
}
std::uint32_t target(void* widget, std::size_t slot,
    NativeGuiWidgetVisibilityContext& context) {
    const auto profile = word(widget);
    const volatile std::uint32_t* const table = context.bindings.current_table(widget, profile);
    return table[slot / 4];
}
std::uint8_t visible(void* widget, NativeGuiWidgetVisibilityContext& context) {
    const auto entry = target(widget, 0x38, context);
    if (entry == 0x00a9e0d0)
        return static_cast<std::uint8_t>(native_gui_widget_is_visible_00a9e0d0(
            widget, &context.actual_zero_00d7a218));
    return context.bindings.call_visibility_38(widget, entry);
}
void changed(void* widget, std::uint8_t value, NativeGuiWidgetVisibilityContext& context) {
    const auto entry = target(widget, 0x3c, context);
    if (entry == 0x00a9e100)
        native_gui_widget_visibility_changed_00a9e100(widget, nullptr, value);
    else context.bindings.call_visibility_changed_3c(widget, entry, value);
}
void validate(bool valid) {
    if (!valid) _invalid_parameter_noinfo();
}
} // namespace

__declspec(naked) std::uint32_t __fastcall native_gui_widget_is_visible_00a9e0d0(
    void*, const volatile float*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 4ch]
        test eax, eax
        jz invisible
        movss xmm0, dword ptr [eax + 0ach]
        comiss xmm0, dword ptr [edx]
        jbe invisible
        mov eax, 1
        ret
    invisible:
        xor eax, eax
        ret
    }
}

__declspec(naked) void __fastcall native_gui_widget_visibility_changed_00a9e100(
    void*, void*, std::uint32_t) noexcept {
    __asm { ret 4 }
}

void propagate_native_gui_widget_visibility_00aa8450(void* widget,
    std::uint8_t old_ancestor, std::uint8_t new_ancestor,
    std::uint8_t requested, std::uint8_t recurse, std::uint8_t apply_requested,
    NativeGuiWidgetVisibilityContext& context) {
    if (!pointer(widget, 0x4c)) return;
    const auto before = static_cast<std::uint8_t>(old_ancestor != 0 && visible(widget, context) != 0);
    std::uint8_t after = 0;
    if (new_ancestor != 0) {
        const auto selected = apply_requested != 0 ? requested : visible(widget, context);
        after = static_cast<std::uint8_t>(selected != 0);
    }
    if (before != after) changed(widget, after, context);
    void* cursor = pointer(pointer(widget, 0x68));
    for (;;) {
        void* const current_end = pointer(widget, 0x68);
        if (cursor == current_end) break;
        validate(cursor != pointer(widget, 0x68));
        const auto child_apply = static_cast<std::uint8_t>(
            (recurse != 0 ? 0xffu : 0u) & apply_requested);
        void* const child = pointer(cursor, 8);
        propagate_native_gui_widget_visibility_00aa8450(child, before, after,
            requested, recurse, child_apply, context);
        validate(cursor != pointer(widget, 0x68));
        cursor = pointer(cursor); // AFTER child callbacks, never a saved next.
    }
}

void set_native_gui_widget_visible_00aa8530(void* widget,
    std::uint8_t requested, NativeGuiWidgetVisibilityContext& context) {
    void* ancestor = pointer(widget, 0x70);
    std::uint8_t ancestors = 1;
    while (ancestor && ancestors != 0) {
        const auto current = visible(ancestor, context);
        ancestor = pointer(ancestor, 0x70); // Callback may replace the link.
        ancestors = static_cast<std::uint8_t>(ancestors & current);
    }
    const auto recurse = byte(widget, 0x75);
    propagate_native_gui_widget_visibility_00aa8450(widget,
        ancestors, ancestors, requested, recurse, 1, context);
    auto* const node = static_cast<NativeNodeStorage*>(pointer(widget, 0x4c));
    if (node) {
        const auto current_recurse = byte(widget, 0x75);
        float factor;
        if (requested != 0) {
            __asm { fld1 }
        } else {
            __asm { fldz }
        }
        __asm { fstp factor }
        set_native_node_visibility_factor_00b6da70(node, nullptr, factor, current_recurse);
    }
}

} // namespace bsp
