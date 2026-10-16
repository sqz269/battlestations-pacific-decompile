#include "bsp/native_gui_clip_box_factory.hpp"
#include "bsp/native_gui_widget_base_storage.hpp"
#include "bsp/native_gui_widget_pools_cc13.hpp"
#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI ClipBox factory requires MSVC Win32.
#endif
static_assert(sizeof(void*) == 4);

namespace bsp {
void* construct_native_gui_clip_box_default_00ace0a0(void* destination,
    const volatile std::uint32_t& actual_one) {
    construct_native_gui_widget_base_00aa9390(destination, 16, actual_one);
    *static_cast<volatile std::uint32_t*>(destination) = 0x00d5d058u;
    return destination;
}

void* construct_native_gui_clip_box_copy_00ace0f0(void* destination,
    const void* source, NativeGuiWidgetCopyContext& context,
    NativeGuiWidgetCopyAcquired& acquired) {
    construct_native_gui_widget_copy_00aa9520(destination, source, context, acquired);
    *static_cast<volatile std::uint32_t*>(destination) = 0x00d5d058u;
    return destination;
}

void* create_native_gui_clip_box_00aa1920(const void* source,
    const volatile std::uint32_t& actual_one, NativeGuiWidgetCopyContext& context,
    NativeGuiWidgetCopyAcquired& acquired) {
    const void* const captured_source = source;
    void* const slot = allocate_gui_widget_pool_slot<GuiClipBoxPoolTraits>();
    if (!slot) return nullptr;
    try {
        if (captured_source)
            return construct_native_gui_clip_box_copy_00ace0f0(
                slot, captured_source, context, acquired);
        return construct_native_gui_clip_box_default_00ace0a0(slot, actual_one);
    } catch (...) {
        // The callee has ended only its failed widget base lifetime. Model
        // acquisitions retained by the caller remain independent and live.
        try { free_gui_widget_pool_slot<GuiClipBoxPoolTraits>(slot); }
        catch (...) { std::terminate(); }
        throw;
    }
}
} // namespace bsp
