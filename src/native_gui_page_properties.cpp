#include "bsp/native_gui_page_properties.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/gui_native_geometry.hpp"
#include "bsp/native_node_root_propagation.hpp"
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw GUI Page properties require MSVC Win32.
#endif
namespace bsp {
namespace {
using Word = std::uint32_t;
using Acquired = NativeGuiPageReadPropertiesAcquired;
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
void* at(const void* p, Word offset = 0) noexcept {
    return pointer(reinterpret_cast<Word>(p) + offset);
}
Word read(const void* p, Word offset = 0) noexcept {
    return *static_cast<const volatile Word*>(at(p, offset));
}
void write(void* p, Word offset, Word value) noexcept {
    *static_cast<volatile Word*>(at(p, offset)) = value;
}
Word bits(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
Word target(void* receiver, Word offset, game::GameNativeReadOnlyData& data) {
    const Word profile = read(receiver);
    const auto* table = static_cast<const volatile Word*>(data.data_at(profile, offset + 4));
    return table[offset / 4];
}
} // namespace
NativeGuiPageReadPropertiesAcquired::~NativeGuiPageReadPropertiesAcquired() noexcept {
    if (phase == Phase::entered || phase == Phase::failed) std::terminate();
}
template<class Context> void read_properties(void* page, void* reader,
    NativeGuiPageReadPropertiesFrame& frame, Context& c, Acquired& a) {
    require(a.phase == Acquired::Phase::fresh && !a.scene.started &&
        a.children.phase == NativeGuiWidgetPropertyChildrenAcquired::Phase::fresh &&
        &c.widget.properties.strings == &c.children.strings &&
        &c.scene.scene_fields.owners == &c.children.models.geometry.actual_owners(),
        "Page properties require fresh retained frames and SAME actual string/owner domains");
    // Verify the genuine mapped literals before native writes. These are data,
    // not callable numeric addresses or replacement tables.
    const Word priority_key = bits(c.data.data_at(0x00d5cb00, 9));
    const Word render_key = bits(c.data.data_at(0x00d5caf4, 12));
    void* const base = frame.bytes;
    void* const key = base;
    void* const field = at(base, 8);
    void* const fallback = at(base, 0x10);
    a.phase = Acquired::Phase::entered;
    try {
        write(fallback, 4, 0); write(fallback, 0, 1);
        write(field, 4, bits(at(base, 0x20)));
        write(key, 4, priority_key); write(field, 0, 1);
        const Word priority_target = target(reader, 0xc, c.data);
        write(key, 0, 0);
        a.active_call_site = 0x00ac4c8b;
        c.widget.properties.dispatch.call_virtual0c(priority_target, reader, key, field, fallback);
        const Word priority = read(base, 0x20);
        write(page, 0xfc, priority); write(page, 0x118, priority);
        a.priority_published = true;
        write(fallback, 0, 2); write(fallback, 4, 0);
        write(field, 0, 2); write(field, 4, bits(at(page, 0x11c)));
        write(key, 0, 0); write(key, 4, render_key);
        const Word render_target = target(reader, 0xc, c.data);
        a.active_call_site = 0x00ac4cdd;
        c.widget.properties.dispatch.call_virtual0c(render_target, reader, key, field, fallback);
        a.render_order_read = true;
        a.loaded_profile = read(page);
        const auto* table = static_cast<const volatile Word*>(c.data.data_at(a.loaded_profile, 0x7c));
        a.loaded_target = table[0x78 / 4];
        require(a.loaded_target == 0x00ac59a0,
            "Page current78 requires the genuine raw AC59A0 scene acquisition body");
        a.active_call_site = 0x00ac4ce6;
        acquire_native_gui_scene_storage_warm_manager_00ac59a0(page, frame.scene, c.scene, a.scene);
        a.scene_returned = true;
        a.requested_scene = pointer(read(page, 0xec)); // BEFORE root+4C, after current78.
        a.root_node = pointer(read(page, 0x4c));
        a.active_call_site = 0x00ac4cf2;
        propagate_native_node_root_storage_00b6d890(a.root_node, a.requested_scene,
            c.scene.camera.lifetime.node.scenes.children);
        a.root_returned = true;
        write(base, 0x24, 0); write(base, 0x28, 0); write(base, 0x2c, 0);
        a.active_call_site = 0x00ac4d13;
        set_native_gui_widget_local_position_00aa7dc0(page, at(base, 0x24),
            c.widget.transform_scratch, c.widget.transform, frame.bounds, c.half_00d7a280);
        a.position_returned = true;
        a.active_call_site = 0x00ac4d1b;
        read_native_gui_widget_properties_and_children_00aaa710(page, reader, frame.widget,
            c.widget, c.children, a.children);
        a.properties_returned = true; a.phase = Acquired::Phase::complete;
    } catch (...) { a.phase = Acquired::Phase::failed; throw; }
}
void read_native_gui_page_properties_00ac4c50(void* p, void* r,
    NativeGuiPageReadPropertiesFrame& f, NativeGuiPageReadPropertiesContext& c, Acquired& a) {
    read_properties(p, r, f, c, a);
}
void read_native_gui_page_properties_00ac4c50(void* p, void* r,
    NativeGuiPageReadPropertiesFrame& f, NativeGuiPageReadPropertiesRawContext& c, Acquired& a) {
    read_properties(p, r, f, c, a);
}
} // namespace bsp
