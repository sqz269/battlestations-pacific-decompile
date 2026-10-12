// GUI widget library reconstructions, packet cc13-gui1 (docs/CC13_GUI_1.md).
#include "bsp/gui_widgets_cc13.hpp"

#include <cstring>
#include <type_traits>

namespace bsp::gui_widgets_cc13 {
namespace {

template <class T>
T read_field(const void* object, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof(value));
    return value;
}

template <class T>
void write_field(void* object, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof(value));
}

template <class F>
auto with_pool(GuiWidgetType type, F&& f) {
    switch (type) {
    case GuiWidgetType::Icon:
    case GuiWidgetType::Progbar: return f(static_cast<GuiIconPoolTraits*>(nullptr));
    case GuiWidgetType::Line: return f(static_cast<GuiLinePoolTraits*>(nullptr));
    case GuiWidgetType::Scrollbar: return f(static_cast<GuiScrollbarPoolTraits*>(nullptr));
    case GuiWidgetType::Grid: return f(static_cast<GuiGridPoolTraits*>(nullptr));
    case GuiWidgetType::Model: return f(static_cast<GuiModelPoolTraits*>(nullptr));
    case GuiWidgetType::Movie: return f(static_cast<GuiMoviePoolTraits*>(nullptr));
    case GuiWidgetType::Curve: return f(static_cast<GuiCurvePoolTraits*>(nullptr));
    case GuiWidgetType::Sound: return f(static_cast<GuiSoundPoolTraits*>(nullptr));
    case GuiWidgetType::ClipBox: return f(static_cast<GuiClipBoxPoolTraits*>(nullptr));
    case GuiWidgetType::Section: return f(static_cast<GuiSectionPoolTraits*>(nullptr));
    case GuiWidgetType::FrameBox: return f(static_cast<GuiFrameBoxPoolTraits*>(nullptr));
    case GuiWidgetType::AnimIcon: return f(static_cast<GuiAnimIconPoolTraits*>(nullptr));
    case GuiWidgetType::Button: return f(static_cast<GuiButtonPoolTraits*>(nullptr));
    default: return f(static_cast<GuiWidgetPoolTraits*>(nullptr));
    }
}

void free_to_pool(GuiWidgetType type, void* slot) {
    with_pool(type, [slot](auto* traits) {
        using T = std::remove_pointer_t<decltype(traits)>;
        free_gui_widget_pool_slot<T>(slot);
        return 0;
    });
}

} // namespace

template <class Pool>
void* gui_widget_default_or_copy_factory(GuiWidgetType type, const void* source,
    GuiWidgetConstructionHost& host) {
    void* slot = allocate_gui_widget_pool_slot<Pool>();
    if (slot == nullptr) return nullptr;
    // EH states 0 (copy) and 1 (default) both unwind to the pool free thunk.
    try {
        return source != nullptr ? host.copy_construct(type, slot, source)
                                 : host.construct_default(type, slot);
    } catch (...) {
        free_gui_widget_pool_slot<Pool>(slot);
        throw;
    }
}

void* gui_widget_factory(GuiWidgetType type, const void* source, GuiWidgetConstructionHost& host) {
    for (const auto& row : kGuiWidgetFactories) {
        if (row.type != type) continue;
        return with_pool(type, [&](auto* traits) {
            using T = std::remove_pointer_t<decltype(traits)>;
            return gui_widget_default_or_copy_factory<T>(type, source, host);
        });
    }
    return nullptr;
}

void* gui_widget_scalar_deleting_destructor(std::uint32_t address, void* self, std::uint32_t flags,
    GuiWidgetDestructionHost& host) {
    for (const auto& row : kGuiWidgetDeletingDestructors) {
        if (row.address != address) continue;
        if (row.vtable_store != 0) write_field<std::uint32_t>(self, 0, row.vtable_store);
        host.run_destructor(row.destructor, self);
        if (flags & 1u) free_to_pool(row.pool_type, self);
        return self;
    }
    return nullptr;
}

void gui_widget_destroy_with_base(void* self, std::uint32_t vtable, std::uint32_t base_destructor,
    GuiWidgetDestructionHost& host) {
    write_field<std::uint32_t>(self, 0, vtable);
    host.run_destructor(base_destructor, self);
}

void gui_line_destroy(void* self, GuiWidgetDestructionHost& host) {
    constexpr std::uint32_t kLineVtable = 0x00D5C9E8;
    constexpr std::uint32_t kWidgetBaseDestructor = 0x00AA9730;
    write_field<std::uint32_t>(self, 0, kLineVtable);
    if (auto* points = read_field<void*>(self, 0xF0)) host.free_block(points);
    write_field<void*>(self, 0xF0, nullptr);
    write_field<void*>(self, 0xF4, nullptr);
    write_field<void*>(self, 0xF8, nullptr);
    host.run_destructor(kWidgetBaseDestructor, self);
}

void* gui_line_scalar_deleting_destructor(void* self, std::uint32_t flags, GuiWidgetDestructionHost& host) {
    gui_line_destroy(self, host);
    if (flags & 1u) free_gui_widget_pool_slot<GuiLinePoolTraits>(self);
    return self;
}

} // namespace bsp::gui_widgets_cc13
