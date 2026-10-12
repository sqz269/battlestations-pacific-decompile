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

namespace {
void curve_default_fields(void* self) noexcept;
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
    curve_default_fields(self); // UV 1.0 is 00D7A24C
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
namespace {
void zero_range(void* self, std::size_t from, std::size_t to) noexcept {
    for (std::size_t offset = from; offset <= to; offset += 4) write_field<std::uint32_t>(self, offset, 0);
}
void copy_dword(void* self, const void* source, std::size_t offset) noexcept {
    write_field<std::uint32_t>(self, offset, read_field<std::uint32_t>(source, offset));
}
// The 00AC57E0 / 00AA3C60 string copy over two 8-byte native string headers.
void copy_native_string(void* destination, const void* source, GuiNativeStringHost& strings) {
    if (destination == source) return;
    strings.resize_0041dd40(destination, read_field<std::uint32_t>(source, 0), true);
    if (read_field<std::uint32_t>(source, 0) == 0) return;
    std::memcpy(read_field<char*>(destination, 4), read_field<const char*>(source, 4),
        read_field<std::uint32_t>(destination, 0));
}
void curve_default_fields(void* self) noexcept {
    write_field<std::uint32_t>(self, 0, 0x00D5C7F8);
    zero_range(self, 0xEC, 0x118);
    write_field<float>(self, 0x11C, 0.0f);
    write_field<float>(self, 0x120, 0.0f);
    write_field<float>(self, 0x124, 1.0f);
    write_field<float>(self, 0x128, 1.0f);
    write_field<std::uint32_t>(self, 0x12C, 0);
    zero_range(self, 0x134, 0x13C);
}
} // namespace

void* gui_sound_copy_construct_00acdba0(void* self, const void* source, GuiWidgetMemberHost& host,
    GuiWidgetCopyHost& copy) {
    host.construct_widget_base_00aa9390(self, 0x0E);
    write_field<std::uint32_t>(self, 0, 0x00D5CFD0);
    write_field<std::uint32_t>(self, 0xEC, 0);
    write_field<std::uint32_t>(self, 0xF0, 0);
    zero_range(self, 0xF8, 0x104);
    copy_dword(self, source, 0xF4);
    copy.copy_sound_ref_00acd8e0(static_cast<std::byte*>(self) + 0xEC, static_cast<const std::byte*>(source) + 0xEC);
    copy.copy_sound_ref_00acd9e0(static_cast<std::byte*>(self) + 0xF0, static_cast<const std::byte*>(source) + 0xF0);
    return self;
}

void gui_sound_destroy_00acdc40(void* self, GuiWidgetMemberHost& host, GuiWidgetCopyHost& copy) {
    write_field<std::uint32_t>(self, 0, 0x00D5CFD0);
    if (auto* p = read_field<void*>(self, 0xEC)) release_ref(p, host);
    write_field<void*>(self, 0xEC, nullptr);
    if (auto* p = read_field<void*>(self, 0xF0)) copy.stop_sound_instance(p);
    if (auto* p = read_field<void*>(self, 0xF0)) release_ref(p, host);
    write_field<void*>(self, 0xF0, nullptr);
    release_native_string(self, 0x100, host);
    release_native_string(self, 0xF8, host);
    // The member destructors of the two ref pointers find them null.
    host.destroy_widget_base_00aa9730(self);
}

void* gui_grid_construct_00acb0a0(void* self, GuiWidgetMemberHost& host) {
    host.construct_widget_base_00aa9390(self, 8);
    write_field<std::uint32_t>(self, 0, 0x00D5CE60);
    zero_range(self, 0xF0, 0x100);
    write_field<float>(self, 0x104, 0.0f);
    write_field<float>(self, 0x108, 0.0f);
    write_field<std::uint8_t>(self, 0x10C, 1);
    write_field<float>(self, 0x110, 0.0f);
    write_field<float>(self, 0x114, 0.0f);
    write_field<std::uint32_t>(self, 0x118, 0);
    write_field<float>(self, 0x11C, 0.0f);
    write_field<float>(self, 0x120, 0.0f);
    return self;
}

void* gui_grid_copy_construct_00acb3f0(void* self, const void* source, GuiWidgetMemberHost& host,
    GuiWidgetCopyHost& copy) {
    host.copy_widget_base_00aa9520(self, source);
    write_field<std::uint32_t>(self, 0, 0x00D5CE60);
    copy.grid_copy_columns_00acb290(static_cast<std::byte*>(self) + 0xEC, static_cast<const std::byte*>(source) + 0xEC);
    for (std::size_t offset : {0x104u, 0x108u}) write_field<float>(self, offset, read_field<float>(source, offset));
    write_field<std::uint8_t>(self, 0x10C, read_field<std::uint8_t>(source, 0x10C));
    for (std::size_t offset : {0x110u, 0x114u, 0x11Cu, 0x120u})
        write_field<float>(self, offset, read_field<float>(source, offset));
    // A temporary native string "Visual_Group" (12 characters + NUL) is built
    // and returned to the pool after the lookup.
    write_field<void*>(self, 0x118, copy.find_child_00aa7e00(self, "Visual_Group"));
    return self;
}

void gui_grid_destroy_00acbbf0(void* self, GuiWidgetMemberHost& host, GuiWidgetCopyHost& copy) {
    write_field<std::uint32_t>(self, 0, 0x00D5CE60);
    copy.grid_clear_00acb9b0(self);
    void* vector = static_cast<std::byte*>(self) + 0xEC;
    if (auto* begin = read_field<void*>(self, 0xF0)) {
        copy.grid_destroy_columns_00aca730(begin, read_field<void*>(self, 0xF4), vector);
        copy.free_block(read_field<void*>(self, 0xF0));
    }
    zero_range(self, 0xF0, 0xF8);
    host.destroy_widget_base_00aa9730(self);
}

void* gui_anim_icon_copy_construct_00aa1c60(void* self, const void* source, GuiWidgetLifetimeHost& host) {
    host.copy_icon_00ab5d30(self, source);
    write_field<std::uint32_t>(self, 0, 0x00D5BCE0);
    write_field<float>(self, 0x138, read_field<float>(source, 0x138));
    copy_dword(self, source, 0x13C);
    for (std::size_t offset : {0x140u, 0x141u, 0x142u})
        write_field<std::uint8_t>(self, offset, read_field<std::uint8_t>(source, offset));
    return self;
}

void* gui_curve_copy_construct_00abe100(void* self, const void* source, GuiWidgetMemberHost& host) {
    host.copy_widget_base_00aa9520(self, source);
    curve_default_fields(self);
    return self;
}

void* gui_button_construct_00aac6f0(void* self, GuiWidgetMemberHost& host, GuiButtonRegistry& registry) {
    host.construct_widget_base_00aa9390(self, 0x0F);
    write_field<std::uint32_t>(self, 0, 0x00D5BED0);
    zero_range(self, 0xEC, 0xF8);
    registry.push_back(self);
    return self;
}

void* gui_button_copy_construct_00aa3c60(void* self, const void* source, GuiWidgetMemberHost& host,
    GuiNativeStringHost& strings) {
    host.copy_widget_base_00aa9520(self, source);
    write_field<std::uint32_t>(self, 0, 0x00D5BED0);
    copy_dword(self, source, 0xEC);
    copy_dword(self, source, 0xF0);
    write_field<std::uint32_t>(self, 0xF4, 0);
    write_field<std::uint32_t>(self, 0xF8, 0);
    copy_native_string(static_cast<std::byte*>(self) + 0xF4, static_cast<const std::byte*>(source) + 0xF4, strings);
    return self;
}

void gui_button_destroy_00aac530(void* self, GuiWidgetMemberHost& host, GuiButtonRegistry& registry,
    std::uint32_t& global_00f8bcf8) {
    write_field<std::uint32_t>(self, 0, 0x00D5BED0);
    registry.remove(self);
    if (global_00f8bcf8 != 0) global_00f8bcf8 = 0;
    release_native_string(self, 0xF4, host);
    host.destroy_widget_base_00aa9730(self);
}

void gui_icon_describe_properties_00ab2b70(void* icon, GuiPropertyWriter& writer, GuiDescribeHost& host,
    GuiIconDescribeHost& icon_host) {
    const auto* first = read_field<const std::byte*>(icon, 0xF4);
    const auto* last = read_field<const std::byte*>(icon, 0xF8);
    const std::size_t count = first != nullptr ? static_cast<std::size_t>(last - first) / 0x40 : 0;
    const float width = read_field<float>(icon, 0x20);
    const float height = read_field<float>(icon, 0x24);
    if (first != nullptr && count == 1 && read_field<std::int16_t>(icon, 0xEC) == 0) {
        const auto native = icon_host.state_texture_size_00ab27a0(icon, 0);
        if (width == native[0] && height == native[1]) {
            write_field<float>(icon, 0x20, 0.0f);
            write_field<float>(icon, 0x24, 0.0f);
        }
    }
    gui_widget_describe_properties_00aaaed0(icon, writer, host);
    write_field<float>(icon, 0x20, width);
    write_field<float>(icon, 0x24, height);

    write_unless_default(writer, "DynamicVB", boolean(read_field<std::uint8_t>(icon, 0x10C) != 0), boolean(false));
    write_unless_default(writer, "HasTexture", boolean(read_field<std::uint8_t>(icon, 0x100) != 0), boolean(true));
    GuiTaggedValue type;
    type.tag = kGuiValueTagInt;
    type.i = read_field<std::int32_t>(icon, 0x110);
    GuiTaggedValue type_default;
    type_default.tag = kGuiValueTagInt;
    write_unless_default(writer, "PartialDisplayType", type, type_default);
    write_unless_default(writer, "PartialDisplayRatio", floats(GuiValueTag::Float, icon, 0x114, 1),
        constant(GuiValueTag::Float, 1.0f));
    write_unless_default(writer, "DelayedTextureLoad", boolean(read_field<std::uint8_t>(icon, 0x101) != 0),
        boolean(false));
    write_unless_default(writer, "AutoRotate", floats(GuiValueTag::Float, icon, 0x130, 1),
        constant(GuiValueTag::Float, 0.0f));
    if (read_field<std::uint32_t>(icon, 0x104) != 0)
        write_unless_default(writer, "ShaderName", string(native_c_str(icon, 0x104, kEmpty)),
            string("GuiDefault.mshd"));

    if (count == 0) return;
    writer.open_table("States");
    for (std::size_t i = 0; i < count; ++i) {
        writer.open_index_table(static_cast<std::int32_t>(i + 1));
        const std::byte* state = read_field<const std::byte*>(icon, 0xF4) + i * 0x40;
        write_unless_default(writer, "Texture", string(native_c_str(state, 0x04, kEmpty)), string(kEmpty));
        write_unless_default(writer, "Size", floats(GuiValueTag::Vec2, state, 0x34, 2),
            floats(GuiValueTag::Vec2, icon, 0x20, 2));
        write_unless_default(writer, "Pivot", floats(GuiValueTag::Vec2, state, 0x2C, 2),
            floats(GuiValueTag::Vec2, icon, 0x18, 2));
        const float l = read_field<float>(state, 0x1C), u = read_field<float>(state, 0x20);
        const float r = read_field<float>(state, 0x24), b = read_field<float>(state, 0x28);
        if (l != 0.0f || u != 0.0f || r != 1.0f || b != 1.0f) {
            writer.open_table("UV_LURB");
            std::int32_t key = 1;
            for (float v : {l, u, r, b}) writer.write_index(key++, constant(GuiValueTag::Float, v));
            writer.close_table();
        }
        writer.close_table();
    }
    writer.close_table();
}

void* gui_icon_copy_construct_00ab5d30(void* self, const void* source, GuiIconLifetimeHost& host) {
    host.copy_widget_base_00aa9520(self, source);
    write_field<std::uint32_t>(self, 0, kGuiIconVtable);
    write_field<std::int16_t>(self, 0xEC, read_field<std::int16_t>(source, 0xEC));
    host.copy_states_00ab58f0(static_cast<std::byte*>(self) + 0xF0, static_cast<const std::byte*>(source) + 0xF0);
    write_field<std::uint8_t>(self, 0x100, read_field<std::uint8_t>(source, 0x100));
    write_field<std::uint8_t>(self, 0x101, read_field<std::uint8_t>(source, 0x101));
    write_field<std::uint32_t>(self, 0x104, 0); // ShaderName is not copied
    write_field<std::uint32_t>(self, 0x108, 0);
    write_field<std::uint8_t>(self, 0x10C, read_field<std::uint8_t>(source, 0x10C));
    copy_dword(self, source, 0x110);
    write_field<float>(self, 0x114, read_field<float>(source, 0x114));
    for (std::size_t offset : {0x118u, 0x11Cu, 0x120u, 0x124u}) copy_dword(self, source, offset);
    write_field<float>(self, 0x128, read_field<float>(source, 0x128));
    write_field<std::uint16_t>(self, 0x12C, read_field<std::uint16_t>(source, 0x12C));
    write_field<float>(self, 0x130, read_field<float>(source, 0x130));
    write_field<std::uint8_t>(self, 0x134, read_field<std::uint8_t>(source, 0x134));
    host.rebuild_00ab3cb0(self, read_field<std::uint16_t>(self, 0xEC));
    return self;
}

void gui_icon_destroy_00ab5e70(void* self, GuiIconLifetimeHost& host) {
    write_field<std::uint32_t>(self, 0, kGuiIconVtable);
    host.cancel_delayed_loads(self);
    if (auto* buffer = read_field<void*>(self, 0x108))
        host.release_string_buffer(buffer, read_field<std::uint32_t>(self, 0x104) + 1u);
    host.destroy_states_00ab5850(static_cast<std::byte*>(self) + 0xF0);
    host.destroy_widget_base_00aa9730(self);
}

GuiIconDelayedLoads& gui_icon_delayed_loads_00ab5700(GuiIconDelayedLoads*& global_00f8bd98,
    GuiIconDelayedLoadsHost& host) {
    if (global_00f8bd98 == nullptr) {
        host.lock();
        if (global_00f8bd98 == nullptr) {
            global_00f8bd98 = new GuiIconDelayedLoads(); // 10h bytes, fields zeroed
            host.register_singleton(*global_00f8bd98);
        }
        host.unlock();
    }
    return *global_00f8bd98;
}

void gui_icon_delayed_loads_destroy_00ab5560(GuiIconDelayedLoads& singleton,
    GuiIconDelayedLoads*& global_00f8bd98) {
    // 00AB4850(0) then _free of the array block: no entry survives.
    while (!singleton.entries().empty()) singleton.cancel_owner_00ab50e0(singleton.entries().back().owner);
    global_00f8bd98 = nullptr;
}

std::vector<const void*> gui_icon_delayed_loads_on_file_loaded_00ab58d0(const std::string& name,
    GuiIconDelayedLoads*& global_00f8bd98, GuiIconDelayedLoadsHost& host) {
    return gui_icon_delayed_loads_00ab5700(global_00f8bd98, host).deliver_00ab51b0(name);
}

} // namespace bsp::gui_widgets_cc13
