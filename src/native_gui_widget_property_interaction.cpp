#include "bsp/native_gui_widget_property_interaction.hpp"
#include "bsp/native_window_focus_dispatch.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
void* at(const void* p, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + offset);
}
Word load(const void* p, Word offset = 0) noexcept {
    return *static_cast<const volatile Word*>(at(p, offset));
}
std::uint8_t byte(const void* p, Word offset = 0) noexcept {
    return *static_cast<const volatile std::uint8_t*>(at(p, offset));
}
void store(void* p, Word offset, Word value) noexcept {
    *static_cast<volatile Word*>(at(p, offset)) = value;
}
void store_byte(void* p, Word offset, std::uint8_t value) noexcept {
    *static_cast<volatile std::uint8_t*>(at(p, offset)) = value;
}
Word bits(const void* p) noexcept { return reinterpret_cast<Word>(p); }
Word current_target(void* object, Word slot) noexcept {
    const Word profile = load(object);
    return load(reinterpret_cast<void*>(profile), slot);
}
} // namespace

void continue_native_gui_widget_property_interaction_00aaabd6(void* widget,
    void* visitor, NativeGuiWidgetPropertyScratch& scratch,
    NativeGuiWidgetPropertyInteractionContext& c) {
    void* const key = scratch.bytes;
    void* const field = at(key, 8);
    void* const fallback = at(key, 0x10);
    auto metadata = [&](void* pair) { store(key, 0x28, bits(pair)); };
    auto read = [&](const char* literal) {
        store(key, 0, 0); store(key, 4, bits(literal));
        const Word target = current_target(visitor, 0xc);
        metadata(key); // Native metadata write follows current slot load.
        c.properties.dispatch.call_virtual0c(target, visitor, key, field, fallback);
    };
    const Word type_target = current_target(widget, 0x5c);
    const Word type = type_target == 0x00a9e110u
        ? read_native_gui_widget_type_00a9e110(widget)
        : c.widgets.call_type_5c(type_target, widget);
    if (type != 1) {
        store(fallback, 0, 3); store_byte(fallback, 4, 1); metadata(fallback);
        store(field, 0, 3); metadata(field);
        void* const visible = at(widget, 0xe4);
        store(field, 4, bits(visible)); read(c.visible_00d5c1e4);
        const auto requested = byte(visible); // BEFORE current widget profile.
        const Word target = current_target(widget, 0x34);
        if (target == 0x00aa8530u)
            set_native_gui_widget_visible_00aa8530(widget, requested, c.visibility);
        else c.widgets.call_visible_34(target, widget, requested);
    }
    store(fallback, 0, 3); store_byte(fallback, 4, 0); metadata(fallback);
    metadata(field); store(field, 0, 3); // MouseBlock reverses these two stores.
    void* const block = at(widget, 0x84);
    store(field, 4, bits(block)); read(c.mouse_block_00d5c1d8);
    if (byte(block) != 0) {
        store_byte(widget, 0x78, 1);
    } else {
        store(fallback, 0, 3); store_byte(fallback, 4, 0); metadata(fallback);
        store(field, 0, 3); metadata(field);
        store(field, 4, bits(at(widget, 0x78))); read(c.mouse_hit_00d5c1cc);
    }
    recompose_native_gui_widget_transform_00aa7220(widget, c.transform_scratch, c.transform);
}

void read_native_gui_widget_properties_through_transform_00aaa710(void* widget,
    void* visitor, NativeGuiWidgetPropertyScratch& scratch,
    NativeGuiWidgetPropertyInteractionContext& c) {
    read_native_gui_widget_property_prefix_00aaa710(widget, visitor, scratch, c.properties);
    continue_native_gui_widget_property_interaction_00aaabd6(widget, visitor, scratch, c);
}
} // namespace bsp
