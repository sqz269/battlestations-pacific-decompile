// GUI widget library reconstructions, packet cc13-gui1 (docs/CC13_GUI_1.md).
#include "bsp/gui_widgets_cc13.hpp"
#include "bsp/native_animation_registry.hpp"
#include "bsp/gui_manager_cc13.hpp"

#include <cstring>
#include <intrin.h>
#include <type_traits>

namespace bsp::gui_widgets_cc13 {
using gui_manager_cc13::name_less_00aa2420;
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

namespace {
GuiTaggedValue floats(GuiValueTag tag, const void* object, std::size_t offset, std::size_t count) noexcept {
    GuiTaggedValue v;
    v.tag = tag;
    for (std::size_t i = 0; i < count; ++i) v.f[i] = read_field<float>(object, offset + 4 * i);
    return v;
}
GuiTaggedValue constant(GuiValueTag tag, float a, float b = 0, float c = 0, float d = 0) noexcept {
    GuiTaggedValue v;
    v.tag = tag;
    v.f = {a, b, c, d};
    return v;
}
GuiTaggedValue boolean(bool value) noexcept {
    GuiTaggedValue v;
    v.tag = GuiValueTag::Bool;
    v.b = value;
    return v;
}
GuiTaggedValue string(const char* value) noexcept {
    GuiTaggedValue v;
    v.tag = GuiValueTag::String;
    v.s = value;
    return v;
}
void write_unless_default(GuiPropertyWriter& writer, const char* name, const GuiTaggedValue& value,
    const GuiTaggedValue& def) {
    if (!writer.equals_00bd5680(value, def)) writer.write(name, value);
}
// A native 8-byte string's buffer, or the given empty literal for a null one.
const char* native_c_str(const void* object, std::size_t header, const char* empty) noexcept {
    const char* buffer = read_field<const char*>(object, header + 4);
    return buffer != nullptr ? buffer : empty;
}
constexpr char kEmpty[] = ""; // 00F8C0E4 (Model), 00CE3A0C (reader default)
} // namespace

void gui_widget_describe_properties_00aaaed0(const void* w, GuiPropertyWriter& writer, GuiDescribeHost& host) {
    write_unless_default(writer, "Pos", floats(GuiValueTag::Vec3, w, 0x0C, 3), constant(GuiValueTag::Vec3, 0));
    write_unless_default(writer, "Size", floats(GuiValueTag::Vec2, w, 0x20, 2), constant(GuiValueTag::Vec2, 0));
    write_unless_default(writer, "Pivot", floats(GuiValueTag::Vec2, w, 0x18, 2), constant(GuiValueTag::Vec2, 0));
    write_unless_default(writer, "Scale", floats(GuiValueTag::Vec2, w, 0x28, 2), constant(GuiValueTag::Vec2, 1, 1));
    write_unless_default(writer, "Rotate", floats(GuiValueTag::Float, w, 0x48, 1), constant(GuiValueTag::Float, 0));
    // 00F8BCE0 (white) and 00F8BCCC (opaque black): lazily filled statics.
    const GuiTaggedValue white = constant(GuiValueTag::Color, 1, 1, 1, 1);
    write_unless_default(writer, "Color", floats(GuiValueTag::Color, w, 0x50, 4), white);
    write_unless_default(writer, "LowColor", floats(GuiValueTag::Color, w, 0xA4, 4),
        constant(GuiValueTag::Color, 0, 0, 0, 1));
    write_unless_default(writer, "HighColor", floats(GuiValueTag::Color, w, 0xB4, 4), white);
    write_unless_default(writer, "BlendFactor", floats(GuiValueTag::Float, w, 0xC4, 1),
        constant(GuiValueTag::Float, 0));
    const auto align = read_field<std::int32_t>(w, 0xE0);
    if (align == 1) writer.write("WideScreenAlign", string("Left"));   // 00CE92E4
    else if (align == 2) writer.write("WideScreenAlign", string("Right")); // 00CE92DC
    if (host.element_kind(w) != 1)
        write_unless_default(writer, "Visible", boolean(read_field<std::uint8_t>(w, 0xE4) != 0), boolean(true));
    if (read_field<std::uint8_t>(w, 0x84) != 0)
        writer.write("MouseBlock", boolean(true));
    else
        write_unless_default(writer, "MouseHit", boolean(read_field<std::uint8_t>(w, 0x78) != 0), boolean(false));
    std::array<const void*, 256> children{};
    const std::size_t count = host.sorted_children(w, children);
    for (std::size_t i = 0; i < count && i < children.size(); ++i) {
        writer.open_table(host.child_name(children[i]));
        host.describe_child(children[i], writer);
        writer.close_table();
    }
}

void gui_progbar_read_properties_00ac7a90(void* p, GuiPropertyReader& reader, GuiDescribeHost& host) {
    host.read_base(0x00AB3310, p, reader);
    GuiTaggedValue field = floats(GuiValueTag::Float, p, 0x13C, 1);
    reader.read_with_default("DefaultValue", field, constant(GuiValueTag::Float, kGuiProgbarDefaultValue));
    write_field<float>(p, 0x13C, field.f[0]);
    write_field<float>(p, 0x138, read_field<float>(p, 0x13C));
}

void gui_progbar_describe_properties_00ac7b00(const void* p, GuiPropertyWriter& writer, GuiDescribeHost& host) {
    host.describe_base(0x00AB2B70, p, writer);
    write_unless_default(writer, "DefaultValue", floats(GuiValueTag::Float, p, 0x13C, 1),
        constant(GuiValueTag::Float, kGuiProgbarDefaultValue));
}

float gui_progbar_value_00ac7820(const void* p) noexcept { return read_field<float>(p, 0x138); }

void gui_scrollbar_describe_properties_00ac8770(const void* s, GuiPropertyWriter& writer, GuiDescribeHost& host) {
    gui_widget_describe_properties_00aaaed0(s, writer, host);
    if (read_field<std::int32_t>(s, 0xF8) == 0) writer.write("Alignment", string("Vertical"));
    write_unless_default(writer, "MinValue", floats(GuiValueTag::Float, s, 0xF0, 1), constant(GuiValueTag::Float, 0));
    write_unless_default(writer, "MaxValue", floats(GuiValueTag::Float, s, 0xF4, 1), constant(GuiValueTag::Float, 0));
}

void gui_model_describe_properties_00acc4e0(const void* m, GuiPropertyWriter& writer, GuiDescribeHost& host) {
    gui_widget_describe_properties_00aaaed0(m, writer, host);
    writer.write("ModelName", string(native_c_str(m, 0x10C, kEmpty)));
    write_unless_default(writer, "ModelTextureOverride", string(native_c_str(m, 0x114, kEmpty)), string(kEmpty));
    write_unless_default(writer, "ScaleVector", floats(GuiValueTag::Vec3, m, 0xF4, 3), constant(GuiValueTag::Vec3, 1, 1, 1));
    write_unless_default(writer, "RotationEuler", floats(GuiValueTag::Vec3, m, 0x100, 3), constant(GuiValueTag::Vec3, 0));
}

void gui_model_read_properties_00acce20(void* m, GuiPropertyReader& reader, GuiModelReadHost& host) {
    host.read_base_00aaa710(m, reader);
    host.read_string(m, 0x10C, "ModelName", nullptr, reader);
    host.read_string(m, 0x114, "ModelTextureOverride", kEmpty, reader);
    GuiTaggedValue scale = floats(GuiValueTag::Vec3, m, 0xF4, 3);
    reader.read_with_default("ScaleVector", scale, constant(GuiValueTag::Vec3, 1, 1, 1));
    for (int i = 0; i < 3; ++i) write_field<float>(m, 0xF4 + 4 * i, scale.f[i]);
    GuiTaggedValue rotation = floats(GuiValueTag::Vec3, m, 0x100, 3);
    reader.read_with_default("RotationEuler", rotation, constant(GuiValueTag::Vec3, 0));
    for (int i = 0; i < 3; ++i) write_field<float>(m, 0x100 + 4 * i, rotation.f[i]);
    host.load_model_00acccf0(m);
}

void* gui_progbar_construct_00ac77c0(void* self, GuiWidgetLifetimeHost& host) {
    host.construct_icon_00ab5c60(self);
    write_field<std::uint32_t>(self, 0, kGuiProgbarVtable);
    write_field<float>(self, 0x138, kGuiProgbarDefaultValue);
    write_field<std::int32_t>(self, 0x60, 5);
    return self;
}

void* gui_progbar_copy_construct_00ac7840(void* self, const void* source, GuiWidgetLifetimeHost& host) {
    host.copy_icon_00ab5d30(self, source);
    write_field<std::uint32_t>(self, 0, kGuiProgbarVtable);
    write_field<float>(self, 0x138, read_field<float>(source, 0x138));
    return self;
}

void gui_progbar_on_loaded_00ac78e0(void* self, GuiWidgetLifetimeHost& host) {
    host.loaded_base_00aa7170(self);
    host.progbar_virtual_88(self, 0, 0, 1.0f);
}

void* gui_scrollbar_construct_00ac7c20(void* self, GuiWidgetLifetimeHost& host) {
    host.construct_widget_base_00aa9390(self, 7);
    write_field<std::uint32_t>(self, 0, kGuiScrollbarVtable);
    write_field<float>(self, 0xEC, 0.0f);
    write_field<float>(self, 0xF0, 0.0f);
    write_field<float>(self, 0xF4, 0.0f);
    write_field<std::int32_t>(self, 0xF8, 1);
    return self;
}

void* gui_scrollbar_copy_construct_00ac7cb0(void* self, const void* source, GuiWidgetLifetimeHost& host) {
    host.copy_widget_base_00aa9520(self, source);
    write_field<std::uint32_t>(self, 0, kGuiScrollbarVtable);
    for (std::size_t offset : {0xECu, 0xF0u, 0xF4u}) write_field<float>(self, offset, read_field<float>(source, offset));
    write_field<std::int32_t>(self, 0xF8, read_field<std::int32_t>(source, 0xF8));
    return self;
}

void* gui_layer_name_00ac3fa0(void* layer) noexcept { return static_cast<std::byte*>(layer) + 0x100; }

void gui_layer_set_name_00ac57e0(void* layer, const void* name, GuiNativeStringHost& host) {
    void* header = static_cast<std::byte*>(layer) + 0x100;
    if (header == name) return;
    host.resize_0041dd40(header, read_field<std::uint32_t>(name, 0), true);
    if (read_field<std::uint32_t>(name, 0) == 0) return;
    // 00BF7680 memcpy(destination buffer, source buffer, DESTINATION length).
    std::memcpy(read_field<char*>(header, 4), read_field<const char*>(name, 4), read_field<std::uint32_t>(header, 0));
}

void gui_widget_set_local_z_00aa7910(GuiWidgetTransform& w, float z, GuiWidgetTransformHost& host) {
    w.position.z = z;
    recompose_local_transform(w, host);
}

void gui_widget_set_rotate_00aa7930(GuiWidgetTransform& w, float rotate, GuiWidgetTransformHost& host) {
    w.rotate = rotate;
    recompose_local_transform(w, host);
}

void gui_widget_set_scale_00aa7950(GuiWidgetTransform& w, const float (&scale)[2], GuiWidgetTransformHost& host) {
    w.scale_x = scale[0];
    w.scale_y = scale[1];
    recompose_local_transform(w, host);
}

void gui_widget_set_local_z_and_bounds_00aa7d40(GuiWidgetTransform& w, float z, GuiWidgetTransformHost& host) {
    // MOVSS loads and stores of +0Ch/+10h back to themselves: no change.
    w.position.z = z;
    recompose_local_transform(w, host);
    refresh_local_bounds(w, host);
}

void gui_widget_set_authored_x_00aa7d80(GuiWidgetTransform& w, float x, GuiWidgetTransformHost& host) {
    recompose_local_transform(w, host);
    refresh_local_bounds(w, host);
    w.authored_x = x;
}

void gui_widget_set_visibility_recurses_00aa6820(void* widget, bool recurses) noexcept {
    write_field<std::uint8_t>(widget, 0x75, recurses ? 1 : 0);
}

void* gui_widget_node_name_00aa6800(void* widget, GuiSceneNodeHost& host) {
    return host.node_name_header_00b6d800(read_field<void*>(widget, 0x4C));
}

namespace {
// LOCK DEC [object+4] through 00CE2220 (InterlockedDecrement); vtable +0 at 0.
void release_ref(void* object, GuiWidgetMemberHost& host) {
    auto* count = reinterpret_cast<volatile long*>(static_cast<std::byte*>(object) + 4);
    if (_InterlockedDecrement(count) == 0) host.release_ref_object(object);
}
void release_native_string(void* self, std::size_t header, GuiWidgetMemberHost& host) {
    if (auto* buffer = read_field<void*>(self, header + 4))
        host.release_string_buffer(buffer, read_field<std::uint32_t>(self, header) + 1u);
}
template <class T>
void append_to_mit_vector(void* self, std::size_t header, T value, void (*reserve)(void*, std::int32_t, GuiWidgetMemberHost&),
    GuiWidgetMemberHost& host) {
    void* h = static_cast<std::byte*>(self) + header;
    const auto capacity = read_field<std::int32_t>(h, 8);
    if (read_field<std::int32_t>(h, 4) == capacity) reserve(h, capacity * 2 > 1 ? capacity * 2 : 1, host);
    auto* data = read_field<std::byte*>(h, 0);
    auto* slot = data + read_field<std::int32_t>(h, 4) * static_cast<std::int32_t>(sizeof(T));
    if (slot != nullptr) std::memcpy(slot, &value, sizeof(T));
    write_field<std::int32_t>(h, 4, read_field<std::int32_t>(h, 4) + 1);
}
} // namespace

void* gui_sound_construct_00acdb30(void* self, GuiWidgetMemberHost& host) {
    host.construct_widget_base_00aa9390(self, 0x0E);
    write_field<std::uint32_t>(self, 0, 0x00D5CFD0);
    for (std::size_t offset = 0xEC; offset <= 0x104; offset += 4) write_field<std::uint32_t>(self, offset, 0);
    return self;
}

void* gui_model_construct_00acc740(void* self, GuiWidgetMemberHost& host) {
    host.construct_widget_base_00aa9390(self, 9);
    write_field<std::uint32_t>(self, 0xEC, 0);
    write_field<std::uint32_t>(self, 0xF0, 0);
    write_field<std::uint32_t>(self, 0, 0x00D5CF38);
    for (std::size_t offset = 0x10C; offset <= 0x118; offset += 4) write_field<std::uint32_t>(self, offset, 0);
    return self;
}

void gui_model_release_scene_00acc6a0(void* self, GuiWidgetMemberHost& host) {
    if (auto* scene = read_field<void*>(self, 0xF0)) release_ref(scene, host);
    write_field<void*>(self, 0xF0, nullptr);
    write_field<std::uint32_t>(self, 0xEC, 0);
    host.release_scene_nodes_00aa8320(self);
}

void gui_model_destroy_00acc7b0(void* self, GuiWidgetMemberHost& host) {
    write_field<std::uint32_t>(self, 0, 0x00D5CF38);
    if (auto* scene = read_field<void*>(self, 0xF0)) {
        release_ref(scene, host);
        write_field<void*>(self, 0xF0, nullptr);
    }
    release_native_string(self, 0x114, host);
    release_native_string(self, 0x10C, host);
    host.destroy_widget_base_00aa9730(self);
}

void* gui_line_construct_00ac2d00(void* self, GuiWidgetMemberHost& host) {
    host.construct_widget_base_00aa9390(self, 4);
    write_field<std::uint32_t>(self, 0, 0x00D5C9E8);
    for (std::size_t offset = 0xF0; offset <= 0xFC; offset += 4) write_field<std::uint32_t>(self, offset, 0);
    write_field<std::int32_t>(self, 0x100, 4);
    return self;
}

void* gui_line_copy_construct_00ac2d60(void* self, const void* source, GuiWidgetMemberHost& host) {
    host.copy_widget_base_00aa9520(self, source);
    write_field<std::uint32_t>(self, 0, 0x00D5C9E8);
    host.copy_vector_0079ee20(static_cast<std::byte*>(self) + 0xEC, static_cast<const std::byte*>(source) + 0xEC);
    return self;
}

void gui_line_on_loaded_00ac1bb0(void* self, GuiWidgetMemberHost& host) {
    host.loaded_base_00aa7170(self);
    host.call_virtual(self, 0x50, static_cast<std::byte*>(self) + 0x50);
    host.call_virtual(self, 0x80, nullptr);
}

void* gui_curve_construct_00abe030(void* self, GuiWidgetMemberHost& host) {
    host.construct_widget_base_00aa9390(self, 0x0D);
    write_field<std::uint32_t>(self, 0, 0x00D5C7F8);
    for (std::size_t offset = 0xEC; offset <= 0x118; offset += 4) write_field<std::uint32_t>(self, offset, 0);
    write_field<float>(self, 0x11C, 0.0f);
    write_field<float>(self, 0x120, 0.0f);
    write_field<float>(self, 0x124, 1.0f); // 00D7A24C
    write_field<float>(self, 0x128, 1.0f);
    write_field<std::uint32_t>(self, 0x12C, 0);
    for (std::size_t offset = 0x134; offset <= 0x13C; offset += 4) write_field<std::uint32_t>(self, offset, 0);
    return self;
}

void gui_curve_clear_points_00abc920(void* self, GuiWidgetMemberHost& host) {
    void* points = static_cast<std::byte*>(self) + 0x10C;
    if (read_field<std::int32_t>(points, 8) < 0) host.reserve_qword_vector_0086a350(points, 0);
    // The native loop decrements a positive count to zero one step at a time,
    // then stores 0 unconditionally.
    write_field<std::int32_t>(points, 4, 0);
    resize_native_float_array_00818030(static_cast<std::byte*>(self) + 0xF8, 0);
}

void gui_curve_add_point_00abc970(void* self, const float (&point)[2], float width, GuiWidgetMemberHost& host) {
    std::uint64_t packed;
    std::memcpy(&packed, point, sizeof(packed));
    append_to_mit_vector<std::uint64_t>(self, 0x10C, packed,
        [](void* h, std::int32_t n, GuiWidgetMemberHost& hh) { hh.reserve_qword_vector_0086a350(h, n); }, host);
    append_to_mit_vector<float>(self, 0xF8, width,
        [](void* h, std::int32_t n, GuiWidgetMemberHost&) { reserve_native_float_array_008153e0(h, n); }, host);
}

void gui_curve_on_loaded_00abc040(void* self, GuiWidgetMemberHost& host) {
    host.loaded_base_00aa7170(self);
    host.set_active_00aa6a30(self, true);
}
bool GuiWidgetNameLess::operator()(const void* left, const void* right) const {
    // 00AA2420: the right operand's name is fetched first, then the left's.
    const std::string right_name = names->name(right);
    const std::string left_name = names->name(left);
    return name_less_00aa2420(left_name, right_name);
}

void sort_widgets_by_name(const void** first, const void** last, GuiWidgetNameSource& names) {
    GuiWidgetNameLess pred{&names};
    msvc9_sort::sort(first, last, last - first, pred);
}
} // namespace bsp::gui_widgets_cc13
