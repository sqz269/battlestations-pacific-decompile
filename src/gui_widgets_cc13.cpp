// GUI widget library reconstructions, packet cc13-gui1 (docs/CC13_GUI_1.md).
#include "bsp/gui_widgets_cc13.hpp"
#include "bsp/native_animation_registry.hpp"
#include "bsp/gui_manager_cc13.hpp"

#include <cstdlib>
#include <cstring>
#include <intrin.h>
#include <type_traits>

namespace bsp::gui_widgets_cc13 {

__declspec(noinline) bool native_gui_layer_is_kind_of_00aa38b0(
    const volatile std::uint32_t (&actual_lineage_00f8bf88)[3], std::uint32_t id) noexcept {
    return gui_type_is_kind_of(actual_lineage_00f8bf88, id);
}

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

void gui_movie_reset_position_00aac7f0(GuiWidgetTransform& movie, GuiWidgetTransformHost& host) {
    movie.position = GuiWidgetPoint{};
    recompose_local_transform(movie, host);
    refresh_local_bounds(movie, host);
}

void gui_movie_bind_video_textures_00aac820(void* movie, GuiMovieHost& host) {
    if (read_field<void*>(movie, 0xF0) == nullptr) host.loaded_virtual_78(movie);
    for (std::uint32_t slot = 0; slot < 3; ++slot) {
        auto* material = static_cast<std::byte*>(host.node_material(read_field<void*>(movie, 0x4C)));
        void* texture = host.player_texture(read_field<void*>(movie, 0xF0), slot);
        host.set_texture_slot_00b189f0(read_field<void*>(material, 0x20), slot, texture);
    }
}

void gui_movie_forward_player_10_00aac8c0(void* movie, GuiMovieHost& host) {
    if (auto* player = read_field<void*>(movie, 0xF0)) host.player_virtual_10(player);
}

void* gui_framebox_stream_00aceb00(void* framebox, GuiNodeGeometryHost& host) {
    void* node = read_field<void*>(framebox, 0x4C);
    if (node == nullptr) return nullptr;
    void* geometry = host.node_geometry_00b74640(node);
    if (geometry == nullptr) return nullptr;
    if (host.stream_count_00b72b20(geometry) == 0) return nullptr;
    return host.stream_00b73260(geometry, 0);
}

std::int32_t gui_framebox_state_count_00ad2720(const void* framebox) noexcept {
    const auto first = read_field<std::uintptr_t>(framebox, 0xF4);
    if (first == 0) return 0;
    const auto bytes = static_cast<std::int32_t>(read_field<std::uintptr_t>(framebox, 0xF8) - first);
    return bytes / 0x2C; // IMUL 2E8BA2E9h, SAR 3, sign fix
}

void gui_anim_icon_loaded_00ad2ff0(void* icon, GuiAnimIconHost& host) {
    host.icon_reset_00ab10f0(icon);
    if (read_field<std::uint8_t>(icon, 0x142) != 0) return;
    const auto first = read_field<std::uintptr_t>(icon, 0xF4);
    const std::int32_t count = first != 0
        ? static_cast<std::int32_t>(read_field<std::uintptr_t>(icon, 0xF8) - first) >> 6 : 0;
    host.select_state_virtual_88(icon, count - 1, 0, 1.0f);
}

void* gui_preload_texture_00aa2b30(std::string_view name, const GuiTextureCallbacks& callbacks) {
    std::array<float, 4> uv{0.0f, 0.0f, 1.0f, 1.0f};
    std::array<float, 2> size{}; // uninitialized in the native frame; never read back
    return resolve_gui_texture_00aa2660(name, uv, size, 1.0f, callbacks);
}

void gui_manager_adopt_hoisted_widget_00aa59b0(void* manager, const void* entry, GuiHoistHost& host) {
    host.attach_child_00aaa5a0(read_field<void*>(manager, 0x44), read_field<void*>(entry, 8), 0);
    host.list_push_back(static_cast<std::byte*>(manager) + 0x34, entry);
}

void gui_delete_owned_object(void** slot, std::uint32_t vtable_offset, GuiVirtualDeleteHost& host) {
    if (*slot != nullptr) {
        host.call_virtual_with_1(*slot, vtable_offset);
        *slot = nullptr;
    }
}

void gui_native_string_release_00aa3480(void* header, GuiWidgetMemberHost& host) {
    release_native_string(header, 0, host);
}

void gui_string_list_destroy_00aa57a0(void* list, GuiListHost& host) {
    host.clear_00aa51d0(list);
    host.free_block(read_field<void*>(list, 4));
    write_field<void*>(list, 4, nullptr);
}

void gui_movie_clear_subtitles_00ab00c0(void* movie, GuiMovieSubtitleHost& host) {
    write_field<std::uint8_t>(movie, 0x104, 0);
    if (auto* text = read_field<void*>(movie, 0x108)) host.text_set_localised_00abbe50(text, "", false);
    write_field<void*>(movie, 0x108, nullptr);
    void* subtitles = static_cast<std::byte*>(movie) + 0x10C;
    host.erase(subtitles, 0x14, read_field<void*>(movie, 0x110), read_field<void*>(movie, 0x114));
    for (std::uint32_t i = 0;; ++i) {
        auto* first = read_field<std::byte*>(movie, 0x124);
        if (first == nullptr) break;
        const auto count = static_cast<std::uint32_t>(
            static_cast<std::int32_t>(read_field<std::byte*>(movie, 0x128) - first) / 0x24);
        if (i >= count) break;
        void** slot = reinterpret_cast<void**>(first + i * 0x24 + 0x18);
        if (*slot != nullptr) {
            host.delete_object_virtual_4(*slot);
            *slot = nullptr;
        }
    }
    if (read_field<void*>(movie, 0x124) != read_field<void*>(movie, 0x128))
        host.clear_extratitles_tail(static_cast<std::byte*>(movie) + 0x120);
}

void gui_movie_resize_records(void* vector, std::uint32_t record_bytes, std::uint32_t count, const void* value,
    GuiMovieSubtitleHost& host) {
    auto* first = read_field<std::byte*>(vector, 4);
    auto* last = read_field<std::byte*>(vector, 8);
    const std::uint32_t size = first != nullptr
        ? static_cast<std::uint32_t>(static_cast<std::int32_t>(last - first) / static_cast<std::int32_t>(record_bytes))
        : 0;
    if (size < count) {
        host.insert_n(vector, record_bytes, last, count - size, value);
    } else if (first != nullptr && count < size) {
        host.erase(vector, record_bytes, first + count * record_bytes, last);
    }
    // The callee owns the by-value record: release its leading native string.
    if (auto* buffer = read_field<void*>(value, 4))
        host.release_string_buffer(buffer, read_field<std::uint32_t>(value, 0) + 1u);
}

void gui_line_read_properties_00ac2de0(void* line, GuiTableReader& reader, GuiReaderHost& host) {
    host.read_base_00aaa710(line, reader);
    if (!reader.has("Points")) return;
    reader.open("Points");
    for (std::int32_t key = 1; reader.has_index(key); ++key) {
        GuiTaggedValue value;
        value.tag = GuiValueTag::Vec2;
        reader.read_index(key, value);
        const float point[2] = {value.f[0], value.f[1]};
        host.push_back_vec2_004222c0(static_cast<std::byte*>(line) + 0xEC, point);
    }
    reader.close();
}

void gui_group_read_properties_00ac6fd0(void* group, GuiTableReader& reader, GuiReaderHost& host) {
    host.read_base_00aaa710(group, reader);
}

void gui_curve_read_properties_00abe1d0(void* curve, GuiTableReader& reader, GuiReaderHost& host) {
    host.read_base_00aaa710(curve, reader);
    host.clear_vec2_vector(static_cast<std::byte*>(curve) + 0x130);
    void* widths = static_cast<std::byte*>(curve) + 0xF8;
    resize_native_float_array_00818030(widths, 0);
    if (reader.has("PosArray")) {
        reader.open("PosArray");
        for (std::int32_t key = 1; reader.has_index(key); ++key) {
            GuiTaggedValue value;
            value.tag = GuiValueTag::Vec3;
            reader.read_index(key, value);
            const float point[2] = {value.f[0], value.f[1]};
            host.push_back_vec2_004222c0(static_cast<std::byte*>(curve) + 0x130, point);
            const auto capacity = read_field<std::int32_t>(widths, 8);
            if (read_field<std::int32_t>(widths, 4) == capacity)
                reserve_native_float_array_008153e0(widths, capacity * 2 < 2 ? 1 : capacity * 2);
            if (auto* data = read_field<float*>(widths, 0)) data[read_field<std::int32_t>(widths, 4)] = value.f[2];
            write_field<std::int32_t>(widths, 4, read_field<std::int32_t>(widths, 4) + 1);
        }
        reader.close();
    }
    const auto first = read_field<std::uintptr_t>(curve, 0x134);
    const std::uint32_t count = first != 0
        ? static_cast<std::uint32_t>((read_field<std::uintptr_t>(curve, 0x138) - first) >> 3) : 0;
    if (count < 2) {
        // 00ABE352..: reserve(+ECh, count + 2), then points [0] and [1] through
        // checked iterators, which fail for these counts.
        host.checked_iterator_failure_00bf6713();
        return;
    }
    void* texture = static_cast<std::byte*>(curve) + 0x104;
    reader.read_string("Texture", texture);
    host.curve_set_texture_00abdd80(curve, texture);
    host.curve_virtual_7c(curve);
}

void* gui_named_entry_copy_00aa3d30(void* entry, const void* source, GuiNativeStringHost& strings) {
    write_field<std::uint32_t>(entry, 0, 0);
    write_field<std::uint32_t>(entry, 4, 0);
    if (entry != source) copy_native_string(entry, source, strings);
    copy_dword(entry, source, 8);
    return entry;
}

void* gui_named_entry_construct_00aa9a20(void* entry, const void* name, std::uint32_t value,
    GuiNativeStringHost& strings) {
    write_field<std::uint32_t>(entry, 0, 0);
    write_field<std::uint32_t>(entry, 4, 0);
    if (entry != name) copy_native_string(entry, name, strings);
    write_field<std::uint32_t>(entry, 8, value);
    return entry;
}

void gui_named_entry_place_copy(void* slot, const void* source, GuiNativeStringHost& strings) {
    if (slot != nullptr) gui_named_entry_copy_00aa3d30(slot, source, strings);
}

void gui_widget_delete_timed_entry_00aa8b80(void* widget, std::int32_t index, GuiTimedEntryHost& host) {
    void* array = static_cast<std::byte*>(widget) + 0x88;
    if (read_field<std::int32_t>(widget, 0x8C) < index) return;
    if (index >= read_field<std::int32_t>(widget, 0x8C)) host.resize_pointer_array_00aa77b0(array, index + 1);
    void** slot = read_field<void**>(widget, 0x88) + index;
    if (*slot != nullptr) {
        host.delete_entry(*slot);
        *slot = nullptr;
    }
}

void gui_timed_entry_array_destroy_00aa7bd0(void* array, GuiTimedEntryHost& host) {
    host.resize_pointer_array_00aa77b0(array, 0);
    host.free_block(read_field<void*>(array, 0));
}

void gui_framebox_destroy_00ad2880(void* framebox, GuiWidgetMemberHost& host, GuiFrameBoxHost& framebox_host) {
    write_field<std::uint32_t>(framebox, 0, 0x00D5D130);
    release_native_string(framebox, 0x104, host);
    framebox_host.destroy_state_vector_00ad21b0(static_cast<std::byte*>(framebox) + 0xF0);
    host.destroy_widget_base_00aa9730(framebox);
}

void gui_anim_icon_update_00ad3040(void* icon, float seconds, GuiAnimIconUpdateHost& host) {
    host.icon_update_00ab1150(icon, seconds);
    if (read_field<std::uint8_t>(icon, 0x140) == 0) return;
    const auto first = read_field<std::uintptr_t>(icon, 0xF4);
    if (first == 0) return;
    const std::int32_t count = static_cast<std::int32_t>(read_field<std::uintptr_t>(icon, 0xF8) - first) >> 6;
    if (count == 0) return;

    constexpr double kMilliseconds = 1000.0;  // 00CE47A0
    constexpr double kTwoTo32 = 4294967296.0; // 00CE3978 (float)
    auto clock = host.clock_counter_and_frequency();
    double next = kMilliseconds / static_cast<double>(read_field<float>(icon, 0x138));
    const auto last = read_field<std::int32_t>(icon, 0x13C);
    double last_ms = static_cast<double>(last);
    if (last < 0) last_ms += kTwoTo32;
    next += last_ms;
    const float clock_seconds = static_cast<float>(static_cast<double>(clock[0]) / static_cast<double>(clock[1]));
    const float now = static_cast<float>(kMilliseconds * clock_seconds);
    if (!(static_cast<double>(now) > next)) return;

    clock = host.clock_counter_and_frequency();
    const float seconds_now = static_cast<float>(static_cast<double>(clock[0]) / static_cast<double>(clock[1]));
    const float stamp = static_cast<float>(static_cast<double>(seconds_now) * kMilliseconds);
    write_field<std::int32_t>(icon, 0x13C, static_cast<std::int32_t>(static_cast<std::int64_t>(stamp)));

    const bool forward = read_field<std::uint8_t>(icon, 0x142) != 0;
    const bool loop = read_field<std::uint8_t>(icon, 0x141) != 0;
    const auto current = read_field<std::int16_t>(icon, 0xEC);
    if (forward) {
        if (count - 1 == current) {
            if (loop) host.select_state_virtual_88(icon, 0, 0, 1.0f);
            return;
        }
    } else if (current == 0) {
        if (loop) host.select_state_virtual_88(icon, static_cast<std::int16_t>(count - 1), 0, 1.0f);
        return;
    }
    host.select_state_virtual_88(icon, static_cast<std::int16_t>(current + (forward ? 1 : -1)), 0, 1.0f);
}

void gui_framebox_insert_state_record_00ad2a80(void* framebox, const void* record, std::int16_t index,
    GuiFrameBoxRecordHost& host) {
    auto* object = read_field<std::byte*>(record, 0);
    _InterlockedIncrement(reinterpret_cast<volatile long*>(object + 4)); // 00CE221C
    const std::int32_t count = gui_framebox_state_count_00ad2720(framebox);
    void* vector = static_cast<std::byte*>(framebox) + 0xF0;
    if (index == count) {
        host.push_back_record_00ad29d0(vector, record);
    } else {
        if (read_field<std::uintptr_t>(framebox, 0xF4) == 0 || static_cast<std::uint32_t>(index) >=
                static_cast<std::uint32_t>(gui_framebox_state_count_00ad2720(framebox)))
            host.checked_iterator_failure_00bf6713();
        host.assign_record_00ad0d00(read_field<std::byte*>(framebox, 0xF4) + index * 0x2C, record);
    }
    if (read_field<std::int16_t>(framebox, 0xEC) == index) host.framebox_virtual_88(framebox);
}

void gui_scrollbar_on_loaded_00ac8890(void* scrollbar, GuiScrollbarLoadHost& host) {
    for (const char* name : {"_BackArrow_Icon", "_ForwArrow_Icon", "_Background_Icon", "_Slider_Icon"})
        static_cast<void>(host.find_child_by_suffix_00aaa2b0(scrollbar, name)); // result unused in the image
    host.set_value_00ac7d00(scrollbar, read_field<float>(scrollbar, 0xF0));
    host.loaded_base_00aa7170(scrollbar);
}

void* gui_layer_copy_construct_00ac6040(void* self, const void* source, GuiLayerCopyHost& host,
    GuiNativeStringHost& strings) {
    host.copy_widget_base_00aa9520(self, source);
    write_field<std::uint32_t>(self, 0, 0x00D5BE38);
    copy_dword(self, source, 0xEC);
    copy_dword(self, source, 0xF0);
    write_field<std::uint8_t>(self, 0xF5, read_field<std::uint8_t>(source, 0xF5));
    write_field<float>(self, 0xF8, read_field<float>(source, 0xF8));
    copy_dword(self, source, 0xFC);
    write_field<std::uint32_t>(self, 0x100, 0);
    write_field<std::uint32_t>(self, 0x104, 0);
    copy_native_string(static_cast<std::byte*>(self) + 0x100, static_cast<const std::byte*>(source) + 0x100, strings);
    write_field<std::uint32_t>(self, 0x108, 0);
    write_field<float>(self, 0x10C, 0.1f);
    write_field<float>(self, 0x110, 1000.0f);
    write_field<float>(self, 0x114, 1.0f);
    write_field<std::uint32_t>(self, 0x118, 0);
    write_field<float>(self, 0x11C, 0.0f);
    write_field<std::uint8_t>(self, 0x120, read_field<std::uint8_t>(source, 0x120));
    write_field<std::uint8_t>(self, 0x121, read_field<std::uint8_t>(source, 0x121));
    host.setup_00ac59a0(self);
    if (read_field<std::uint8_t>(self, 0xF5) != 0) {
        auto* store = read_field<std::byte*>(self, 0xF0);
        write_field<std::int32_t>(store, 0x20, read_field<std::int32_t>(store, 0x20) + 1);
    }
    return self;
}

namespace msvc9_containers {
namespace {
std::byte* node_at(const void* object, std::size_t offset) noexcept { return read_field<std::byte*>(object, offset); }
bool is_nil(const void* node) noexcept { return read_field<std::uint8_t>(node, 0x15) != 0; }
} // namespace

void* tree_rightmost_00aa1040(void* node) noexcept {
    while (!is_nil(node_at(node, 8))) node = node_at(node, 8);
    return node;
}

void* tree_buy_head_node_00aa2920() {
    auto* node = static_cast<std::byte*>(::operator new(0x18));
    write_field<void*>(node, 0, nullptr);
    write_field<void*>(node, 4, nullptr);
    write_field<void*>(node, 8, nullptr);
    write_field<std::uint8_t>(node, 0x14, 1);
    write_field<std::uint8_t>(node, 0x15, 0);
    return node;
}

void* tree_init_head(void* tree) {
    void* head = tree_buy_head_node_00aa2920();
    write_field<void*>(tree, 4, head);
    write_field<std::uint8_t>(head, 0x15, 1);
    write_field<void*>(head, 4, head);
    write_field<void*>(head, 0, head);
    write_field<void*>(head, 8, head);
    write_field<std::uint32_t>(tree, 8, 0);
    return tree;
}

void tree_iterator_decrement_00aa1210(void* iterator, TreeIteratorHost& host) {
    if (read_field<void*>(iterator, 0) == nullptr) host.checked_iterator_failure_00bf6713();
    std::byte* node = node_at(iterator, 4);
    if (is_nil(node)) {
        // --end(): the rightmost node, head->right.
        node = node_at(node, 8);
        write_field<void*>(iterator, 4, node);
        if (is_nil(node)) host.checked_iterator_failure_00bf6713(); // empty tree
        return;
    }
    if (!is_nil(node_at(node, 0))) {
        // The rightmost node of the left subtree.
        node = static_cast<std::byte*>(tree_rightmost_00aa1040(node_at(node, 0)));
        write_field<void*>(iterator, 4, node);
        return;
    }
    // Climb while we are a left child (the iterator node is updated as it goes).
    std::byte* parent = node_at(node, 4);
    while (!is_nil(parent) && read_field<std::byte*>(iterator, 4) == node_at(parent, 0)) {
        write_field<void*>(iterator, 4, parent);
        parent = node_at(parent, 4);
    }
    // 00AA1285: the test is on the iterator's node, which the loop only ever
    // moves to non-nil parents, so it cannot fire; --begin() therefore lands on
    // the head (end()) instead of failing.
    if (is_nil(read_field<std::byte*>(iterator, 4))) {
        host.checked_iterator_failure_00bf6713();
        return;
    }
    write_field<void*>(iterator, 4, parent);
}

void* tree_iterator_pre_decrement(void* iterator, TreeIteratorHost& host) {
    tree_iterator_decrement_00aa1210(iterator, host);
    return iterator;
}

void tree_clear_00aa28f0(void* tree, TreeEraseHost& host) {
    std::byte* head = node_at(tree, 4);
    host.erase_subtree_00aa22c0(tree, node_at(head, 4));
    write_field<void*>(head, 4, head);
    write_field<std::uint32_t>(tree, 8, 0);
    write_field<void*>(head, 0, head);
    write_field<void*>(head, 8, head);
}

void tree_destroy(void* tree, TreeEraseHost& host) {
    std::byte* head = node_at(tree, 4);
    host.erase_range_00aa49e0(tree, node_at(head, 0), head);
    ::operator delete(node_at(tree, 4));
    write_field<void*>(tree, 4, nullptr);
    write_field<std::uint32_t>(tree, 8, 0);
}

void* list_buy_sentinel_00aac180() {
    auto* node = static_cast<std::byte*>(::operator new(0x0C));
    write_field<void*>(node, 0, node);
    write_field<void*>(node, 4, node);
    return node;
}

void* list_buy_node_00aac210(void* next, void* prev, const std::uint32_t& value) {
    auto* node = static_cast<std::byte*>(::operator new(0x0C));
    write_field<void*>(node, 0, next);
    write_field<void*>(node, 4, prev);
    write_field<std::uint32_t>(node, 8, value);
    return node;
}

void* list_buy_named_entry_node_00aa4d30(void* next, void* prev, const void* entry, GuiNativeStringHost& strings) {
    auto* node = static_cast<std::byte*>(::operator new(0x14));
    write_field<void*>(node, 0, next);
    write_field<void*>(node, 4, prev);
    try {
        gui_named_entry_copy_00aa3d30(node + 8, entry, strings);
    } catch (...) {
        ::operator delete(node);
        throw;
    }
    return node;
}

void list_clear_00aac1d0(void* list) noexcept {
    std::byte* head = node_at(list, 4);
    std::byte* node = node_at(head, 0);
    write_field<void*>(head, 0, head);
    write_field<void*>(head, 4, head);
    write_field<std::uint32_t>(list, 8, 0);
    while (node != head) {
        std::byte* next = node_at(node, 0);
        ::operator delete(node);
        node = next;
    }
}

void list_destroy_00aac3b0(void* list) noexcept {
    list_clear_00aac1d0(list);
    ::operator delete(node_at(list, 4));
    write_field<void*>(list, 4, nullptr);
}

void* list_construct(void* list, std::uint32_t node_bytes) {
    auto* head = static_cast<std::byte*>(::operator new(node_bytes));
    write_field<void*>(head, 0, head);
    write_field<void*>(head, 4, head);
    write_field<void*>(list, 4, head);
    write_field<std::uint32_t>(list, 8, 0);
    return list;
}

void* list_copy_construct_00aa57c0(void* list, const void* source, ListCopyHost& host) {
    write_field<void*>(list, 4, host.buy_sentinel_00a9b720());
    write_field<std::uint32_t>(list, 8, 0);
    std::byte* source_head = node_at(source, 4);
    host.insert_range_00a9d560(list, node_at(list, 4), node_at(source_head, 0), source_head);
    return list;
}

void* allocate_elements(std::uint32_t count, std::uint32_t element_bytes) {
    if (count != 0 && 0xFFFFFFFFu / count < element_bytes) throw std::bad_alloc();
    return ::operator new(static_cast<std::size_t>(count) * element_bytes);
}

void deallocate(void* pointer) noexcept { ::operator delete(pointer); }

void free_pointer(void** slot) noexcept {
    if (*slot != nullptr) std::free(*slot);
}

void free_and_clear_00aa1ad0(void** slot) noexcept {
    if (*slot != nullptr) {
        ::operator delete(*slot);
        *slot = nullptr;
    }
}

std::uint32_t vector_size(const void* vector, std::uint32_t element_bytes) noexcept {
    const auto first = read_field<std::uintptr_t>(vector, 4);
    if (first == 0) return 0;
    return static_cast<std::uint32_t>(
        static_cast<std::int32_t>(read_field<std::uintptr_t>(vector, 8) - first) / static_cast<std::int32_t>(element_bytes));
}

} // namespace msvc9_containers

void gui_manager_set_cursor_position_00aa1fa0(void* manager, float x, float y, GuiCursorHost& host) {
    write_field<float>(manager, 0x5C, x);
    write_field<float>(manager, 0x60, y);
    host.set_local_xy_00aa7d00(read_field<void*>(manager, 0x50), read_field<float>(manager, 0x5C),
        read_field<float>(manager, 0x60));
}

void gui_reset_render_command_queue_00aa2b80(GuiRenderQueueHost& host) {
    host.reset_queue_00b1c4b0(host.render_command_queue_004c11f0());
}

void gui_curve_set_texture_00abdd80(void* curve, const void* name, GuiCurveTextureHost& host,
    GuiNativeStringHost& strings) {
    void* own = static_cast<std::byte*>(curve) + 0x104;
    if (host.names_equal_00435c40(own, name)) return;
    copy_native_string(own, name, strings);
    float uv[4] = {0.0f, 0.0f, 1.0f, 1.0f};
    for (int i = 0; i < 4; ++i) write_field<float>(curve, 0x11C + 4 * i, uv[i]);
    float size[2] = {0.0f, 0.0f}; // an uninitialized local in the image
    // The resolver gets the widget's own +11Ch rect (it may rewrite it).
    float* live_uv = reinterpret_cast<float*>(static_cast<std::byte*>(curve) + 0x11C);
    void* texture = host.resolve_texture_00aa2660(own, *reinterpret_cast<float(*)[4]>(live_uv), size, 1.0f);
    host.set_texture_slot_00b189f0(read_field<void*>(curve, 0x12C), 0, texture);
    if (texture != nullptr) {
        auto* count = reinterpret_cast<volatile long*>(static_cast<std::byte*>(texture) + 4);
        if (_InterlockedDecrement(count) == 0) host.release_ref_object(texture);
    }
}

void gui_curve_destroy_00abde90(void* curve, GuiCurveDestroyHost& host) {
    write_field<std::uint32_t>(curve, 0, 0x00D5C7F8);
    if (auto* material = read_field<std::byte*>(curve, 0x12C)) {
        if (_InterlockedDecrement(reinterpret_cast<volatile long*>(material + 4)) == 0)
            host.release_ref_object(material);
        write_field<void*>(curve, 0x12C, nullptr);
    }
    if (auto* block = read_field<void*>(curve, 0x134)) ::operator delete(block);
    zero_range(curve, 0x134, 0x13C);
    // Mit qword vector +10Ch.
    void* points = static_cast<std::byte*>(curve) + 0x10C;
    if (read_field<std::int32_t>(points, 8) < 0) host.reserve_qword_vector_0086a350(points, 0);
    write_field<std::int32_t>(points, 4, 0);
    std::free(read_field<void*>(points, 0));
    if (auto* buffer = read_field<void*>(curve, 0x108))
        host.release_string_buffer(buffer, read_field<std::uint32_t>(curve, 0x104) + 1u);
    void* widths = static_cast<std::byte*>(curve) + 0xF8;
    resize_native_float_array_00818030(widths, 0);
    std::free(read_field<void*>(widths, 0));
    void* control = static_cast<std::byte*>(curve) + 0xEC;
    if (read_field<std::int32_t>(control, 8) < 0) host.reserve_qword_vector_0086a350(control, 0);
    write_field<std::int32_t>(control, 4, 0);
    std::free(read_field<void*>(control, 0));
    host.destroy_widget_base_00aa9730(curve);
}

void* gui_model_copy_construct_00accf70(void* self, const void* source, GuiModelCopyHost& host,
    GuiNativeStringHost& strings) {
    host.copy_widget_base_00aa9520(self, source);
    write_field<std::uint32_t>(self, 0, 0x00D5CF38);
    write_field<std::uint32_t>(self, 0xEC, 0);
    write_field<std::uint32_t>(self, 0xF0, 0);
    for (std::size_t offset = 0xF4; offset <= 0x108; offset += 4)
        write_field<float>(self, offset, read_field<float>(source, offset));
    auto* name = static_cast<std::byte*>(self) + 0x10C;
    auto* texture_override = static_cast<std::byte*>(self) + 0x114;
    write_field<std::uint32_t>(name, 0, 0);
    write_field<std::uint32_t>(name, 4, 0);
    copy_native_string(name, static_cast<const std::byte*>(source) + 0x10C, strings);
    write_field<std::uint32_t>(texture_override, 0, 0);
    write_field<std::uint32_t>(texture_override, 4, 0);
    copy_native_string(texture_override, static_cast<const std::byte*>(source) + 0x114, strings);
    host.load_model_00acccf0(self, name, texture_override);
    return self;
}

// ===========================================================================
// Packet cc13-gui2 (docs/CC13_GUI_2.md)
// ===========================================================================

namespace {
GuiTaggedValue integer_value(std::int32_t value) noexcept {
    GuiTaggedValue v;
    v.tag = kGuiValueTagInt;
    v.i = value;
    return v;
}
constexpr char kMovieEmpty[] = ""; // 00F8BD94
constexpr char kTextEmpty[] = "";  // 00F8BE4C
constexpr char kCurveEmpty[] = ""; // 00F8BEA4
constexpr char kLiteralEmpty[] = ""; // 00CE3A0C
} // namespace

void gui_movie_describe_properties_00aad9a0(const void* m, GuiPropertyWriter& writer, GuiDescribeHost& host) {
    gui_widget_describe_properties_00aaaed0(m, writer, host);
    write_unless_default(writer, "MovieName", string(native_c_str(m, 0xFC, kMovieEmpty)), string(kLiteralEmpty));
}

void gui_text_describe_properties_00ab7b60(const void* t, GuiPropertyWriter& writer, GuiDescribeHost& host) {
    gui_widget_describe_properties_00aaaed0(t, writer, host);
    writer.write("Font", string(native_c_str(t, 0x1C8, kTextEmpty)));
    write_unless_default(writer, "FontScale", floats(GuiValueTag::Float, t, 0x1D8, 1), constant(GuiValueTag::Float, 1));
    write_unless_default(writer, "Multiline", boolean(read_field<std::uint8_t>(t, 0xFC) != 0), boolean(true));
    write_unless_default(writer, "DistanceBetweenLines", floats(GuiValueTag::Float, t, 0x10C, 1),
        constant(GuiValueTag::Float, 0));
    write_unless_default(writer, "DefaultText", string(native_c_str(t, 0xF4, kTextEmpty)), string(kLiteralEmpty));
    if (read_field<std::uint32_t>(t, 0x1C0) != 0)
        write_unless_default(writer, "ShaderName", string(native_c_str(t, 0x1C0, kTextEmpty)), string(kLiteralEmpty));
    const char* align = "Left"; // 00CE92E4
    switch (read_field<std::int32_t>(t, 0x100)) {
    case 2: align = "Right"; break;     // 00CE92DC
    case 1: align = "Center"; break;    // 00D5C67C
    case 3: align = "Justified"; break; // 00D5C670
    default: break;
    }
    writer.write("Align", string(align));
    const char* vertical = "Top"; // 00CE92D8
    switch (read_field<std::int32_t>(t, 0x104)) {
    case 2: vertical = "Bottom"; break; // 00CE92D0
    case 1: vertical = "Center"; break;
    default: break;
    }
    writer.write("VerticalAlign", string(vertical));
    if (read_field<std::uint8_t>(t, 0x118) != 0) {
        writer.open_table("MISColors");
        writer.write("Normal", floats(GuiValueTag::Color, t, 0x11C, 4));
        writer.write("Focus", floats(GuiValueTag::Color, t, 0x12C, 4));
        writer.write("Selected", floats(GuiValueTag::Color, t, 0x13C, 4));
        writer.write("Disabled", floats(GuiValueTag::Color, t, 0x14C, 4));
        writer.close_table();
    }
    const std::int32_t default_shadow = read_field<std::int32_t>(t, 0x1D0);
    write_unless_default(writer, "DefaultShadow", integer_value(default_shadow), integer_value(-1));
    if (read_field<std::int32_t>(t, 0x1D0) != -1) return;
    write_unless_default(writer, "Shadowed", boolean(read_field<std::uint8_t>(t, 0x15C) != 0), boolean(false));
    if (read_field<std::uint8_t>(t, 0x15C) == 0) return;
    write_unless_default(writer, "ShadowColor", floats(GuiValueTag::Color, t, 0x168, 4),
        constant(GuiValueTag::Color, 0, 0, 0, 0.75f)); // 00E12FD8
    writer.write("ShadowPos", string("Behind")); // both arms of the +160h test
    write_unless_default(writer, "ShadowOffset", floats(GuiValueTag::Float, t, 0x164, 1),
        constant(GuiValueTag::Float, 0.05f)); // 00D5C5C0
}

void gui_curve_describe_properties_00abc7a0(const void* c, GuiPropertyWriter& writer, GuiDescribeHost& host) {
    gui_widget_describe_properties_00aaaed0(c, writer, host);
    const auto* first = read_field<const float*>(c, 0x134);
    const auto* last = read_field<const float*>(c, 0x138);
    const std::size_t count = first != nullptr ? static_cast<std::size_t>(last - first) / 2 : 0;
    if (count != 0) {
        writer.open_table("PosArray");
        const auto* widths = read_field<const float*>(c, 0xF8);
        for (std::size_t i = 0; i < count; ++i) {
            const GuiTaggedValue point = constant(GuiValueTag::Vec3, first[2 * i], first[2 * i + 1], widths[i]);
            writer.write_index(static_cast<std::int32_t>(i + 1), point);
        }
        writer.close_table();
    }
    writer.write("Texture", string(native_c_str(c, 0x104, kCurveEmpty)));
}

namespace {
// The inlined intrusive-pointer assignment of 00AB3840 / 00AB3B30.
void assign_state_texture(GuiIconState& state, void* texture, GuiIconStateHost& host) {
    void* const old = state.texture;
    if (old == texture) return;
    state.texture = texture;
    if (texture != nullptr) host.retain_texture(texture);
    if (old != nullptr) host.release_texture(old);
}
} // namespace

void gui_icon_rebuild_current_state_00ab10d0(GuiIconWidget& icon, GuiIconStateHost& host) {
    host.rebuild_virtual_80(icon, static_cast<std::int32_t>(static_cast<std::uint16_t>(icon.current_state)));
}

void gui_icon_apply_delayed_texture_00ab3840(GuiIconWidget& icon, const std::string& name, void* texture,
    GuiIconStateHost& host) {
    for (std::size_t i = 0; i < icon.states.size(); ++i) {
        GuiIconState& state = icon.states[i];
        if (state.texture_name.size() != name.size()) continue;
        if (!name.empty() && _stricmp(state.texture_name.c_str(), name.c_str()) != 0) continue;
        assign_state_texture(state, texture, host);
        if (state.size.width == 0.0f && state.size.height == 0.0f)
            state.size = host.native_size_00ab17b0(state.texture, state.resolved_uv);
        state.load_pending = false;
        state.flag_3d = true;
        if (static_cast<std::int32_t>(i) == icon.current_state)
            host.rebuild_virtual_80(icon, static_cast<std::int32_t>(i));
    }
}

void gui_icon_state_set_delayed_placeholder_00ab3b30(GuiIconState& state, GuiIconStateHost& host) {
    state.load_pending = true;
    state.flag_3d = false;
    const std::string name(kGuiIconDelayedPlaceholder);
    assign_state_texture(state, host.load_texture_00f8d394_64(name), host);
}

void gui_icon_set_state_texture_00ab44b0(GuiIconWidget& icon, std::uint32_t index, const std::string& name,
    const GuiUvRect& uv, GuiIconStateHost& host) {
    GuiIconState& state = icon.states.at(index); // 00BF6713 on a bad index
    if (&state.texture_name != &name) state.texture_name = name;
    state.authored_uv = uv;
    if (icon.delayed_texture_load) return;
    GuiUvRect resolved{0.0f, 0.0f, 1.0f, 1.0f};
    GuiWidgetSize unused_size{}; // uninitialized in the image
    void* const texture = host.resolve_texture_00aa2660(name, resolved, unused_size, 1.0f);
    host.set_state_texture_00ab2690(icon, index, texture, resolved);
    host.release_texture(texture);
}

std::int32_t gui_icon_add_state_00ab64c0(GuiIconWidget& icon, const GuiWidgetTransform& base, void* texture,
    const GuiUvRect& uv, GuiIconStateHost& host) {
    GuiIconState record;
    record.resolved_uv = GuiUvRect{0.0f, 0.0f, 1.0f, 1.0f};
    record.authored_uv = GuiUvRect{0.0f, 0.0f, 1.0f, 1.0f};
    record.pivot_x = base.pivot_x;
    record.pivot_y = base.pivot_y;
    record.size = base.size;
    if (texture != nullptr) {
        host.retain_texture(texture);
        record.texture = texture;
        record.texture_name = host.texture_name_00b33e40(texture);
    }
    gui_icon_set_resolved_uv_00ab1680(record, uv);
    // 00AB63A0 -> 00AB5400 -> record copy 00AB3A70, which retains the texture.
    icon.states.push_back(record);
    if (record.texture != nullptr) host.retain_texture(record.texture);
    const auto count = static_cast<std::int32_t>(icon.states.size());
    // 00AB3280 releases the stack record's reference.
    if (record.texture != nullptr) host.release_texture(record.texture);
    return count - 1;
}
void gui_curve_create_geometry_00abcc70(void* c, GuiGeometryHost& host) {
    if (read_field<std::uint8_t>(c, 0x74) == 0) {
        void* const scene = read_field<void*>(c, 0x4C);
        void* const mesh = host.new_mesh_object_00b73b60();
        host.set_geometry_00b75170(scene, 0, mesh, -1.0f, -1.0f); // 00D7A260
        void* const material = host.create_material_00535320("guidefault.mshd"); // 00D5C25C
        write_field<void*>(c, 0x12C, material);
        auto* const bytes = static_cast<std::byte*>(c);
        host.register_float_00b18b20(material, "cOverbrightFactor", reinterpret_cast<float*>(bytes + 0x94));
        host.register_float4_00b18aa0(material, "cLowColor", reinterpret_cast<float*>(bytes + 0xA4));
        host.register_float4_00b18aa0(material, "cHighColor", reinterpret_cast<float*>(bytes + 0xB4));
        host.register_float_00b18b20(read_field<void*>(c, 0x12C), "cBlendFactor", reinterpret_cast<float*>(bytes + 0xC4));
        host.set_parameter_owner_00b18a40(read_field<void*>(c, 0x12C), c, 1);
        host.register_clip_parameters_00aa9f10(c, read_field<void*>(c, 0x12C));
        host.release_object(mesh);
    }
    const float origin[3] = {0.0f, 0.0f, 0.0f};
    host.set_local_position_00aa7dc0(c, origin);
}

void gui_line_create_geometry_00ac2430(void* l, GuiGeometryHost& host) {
    void* const scene = read_field<void*>(l, 0x4C);
    void* const mesh = host.new_mesh_object_00b73b60();
    host.set_geometry_00b75170(scene, 0, mesh, -1.0f, -1.0f);
    host.release_object(mesh);
    void* const format = host.vertex_format_00f8d394_38("SimpleColor.mvfm"); // 00CF0078
    void* const vertices = host.vertex_buffer_00f8d394_5c(4, 1, format);
    host.mesh_set_vertex_stream_00b73bb0(mesh, 0, vertices);
    void* const indices = host.index_buffer_00f8d394_60(8, 1, 0x65);
    host.mesh_set_index_stream_00b73b70(mesh, indices);
    void* const section = host.create_draw_section_00533fa0();
    void* const material = host.create_material_00535320("guidefault.mshd");
    host.section_set_dwords(section, 2, 0, 0);
    host.section_set_material_00b864c0(section, material);
    host.release_object(material);
    host.section_rebuild_layout_00b865a0(section, mesh);
    host.mesh_append_section_00b73c60(mesh, section);
    host.material_set_texture_00b189f0(material, 0, host.gui_manager_texture_28());
    host.release_object(indices);
    host.release_object(vertices);
    host.release_object(format);
    host.release_object(section);
}
void gui_model_setup_material_constants_00acc8a0(void* m, void* node, GuiModelLoadHost& host) {
    auto* const bytes = static_cast<std::byte*>(m);
    if (host.is_generated_model(node)) {
        for (std::uint32_t g = 0; g < host.geometry_count_00b74650(node); ++g) {
            void* const mesh = host.geometry_00b74640(node, g);
            for (std::uint32_t s = 0; s < host.section_count_00b72b40(mesh); ++s) {
                void* const section = host.section_00b732c0(mesh, s);
                host.set_effect_priority_00b172e0(host.material_effect(host.section_material(section)), 0);
                host.register_float_00b18b20(host.section_material(section), "cOverbrightFactor",
                    reinterpret_cast<float*>(bytes + 0x94));
                host.register_float4_00b18aa0(host.section_material(section), "cLowColor",
                    reinterpret_cast<float*>(bytes + 0xA4));
                host.register_float4_00b18aa0(host.section_material(section), "cHighColor",
                    reinterpret_cast<float*>(bytes + 0xB4));
                host.register_float_00b18b20(host.section_material(section), "cBlendFactor",
                    reinterpret_cast<float*>(bytes + 0xC4));
                host.register_clip_parameters_00aa9f10(m, host.section_material(section));
            }
        }
    }
    for (void* child = host.first_child(node); child != nullptr; child = host.next_sibling(child))
        gui_model_setup_material_constants_00acc8a0(m, child, host);
}

bool gui_model_load_resource_00accb90(void* m, const std::string& name, GuiModelLoadHost& host) {
    if (void* const old = read_field<void*>(m, 0xF0)) {
        host.release_object(old);
        write_field<void*>(m, 0xF0, nullptr);
    }
    write_field<void*>(m, 0xEC, nullptr);
    host.assign_string(m, 0x10C, name);
    // The comparison against "" (00CE3A0C) reduces to "name is not empty".
    if (!name.empty()) {
        if (void* const resource = host.load_resource_00b80d70(name)) {
            void* const instance = host.instantiate_resource_08(resource, 6, 1.0f);
            write_field<void*>(m, 0xF0, instance);
            host.release_object(resource);
            void* const root = host.instance_root(instance);
            write_field<void*>(m, 0xEC, root);
            if (root != nullptr) {
                host.set_parent_00b6e680(root, read_field<void*>(m, 0x4C));
                gui_model_setup_material_constants_00acc8a0(m, read_field<void*>(m, 0x4C), host);
            }
        }
    }
    return read_field<void*>(m, 0xEC) != nullptr;
}

void gui_model_load_model_00acccf0(void* m, const std::string& name, const std::string& texture_override,
    GuiModelLoadHost& host) {
    gui_model_load_resource_00accb90(m, name, host);
    host.assign_string(m, 0x114, texture_override);
    auto* const bytes = static_cast<std::byte*>(m);
    host.set_transform_vectors_00acbec0(m, reinterpret_cast<float*>(bytes + 0xF4),
        reinterpret_cast<float*>(bytes + 0x100));
    if (texture_override.empty()) return;
    void* const root = read_field<void*>(m, 0xEC); // not null-tested
    void* const target = host.is_generated_model(root) ? root : host.first_child(root);
    for (std::uint32_t s = 0; s < host.section_count_00b72b40(host.geometry_00b74640(target, 0)); ++s) {
        void* const section = host.section_00b732c0(host.geometry_00b74640(target, 0), s);
        void* const material = host.section_material(section);
        void* const texture = host.load_texture_00f8d394_64(texture_override);
        host.material_set_texture_00b189f0(material, 0, texture);
        host.register_clip_parameters_00aa9f10(m, material);
        host.release_object(texture);
    }
}
void* gui_listbox_copy_construct_00aa5b40(void* self, const void* source, GuiListboxCopyHost& host,
    msvc9_containers::ListCopyHost& lists) {
    const auto copy = [&](std::size_t from, std::size_t to_inclusive) {
        std::memcpy(static_cast<std::byte*>(self) + from, static_cast<const std::byte*>(source) + from,
            to_inclusive - from + 1);
    };
    host.copy_widget_base_00aa9520(self, source);
    write_field<std::uint32_t>(self, 0, kGuiListboxVtable);
    copy(0xEC, 0xFB);  // dword +ECh, floats +F0h..+F8h
    msvc9_containers::list_copy_construct_00aa57c0(static_cast<std::byte*>(self) + 0xFC, static_cast<const std::byte*>(source) + 0xFC,
        lists);
    copy(0x108, 0x11B); // dwords
    copy(0x11C, 0x121); // bytes
    copy(0x124, 0x137); // floats
    copy(0x138, 0x138);
    copy(0x13C, 0x13F);
    copy(0x140, 0x141);
    copy(0x144, 0x147);
    copy(0x148, 0x14A);
    msvc9_containers::list_copy_construct_00aa57c0(static_cast<std::byte*>(self) + 0x14C,
        static_cast<const std::byte*>(source) + 0x14C, lists);
    copy(0x158, 0x16B); // dwords +158h..+164h, float +168h
    host.copy_string_vector_00557bc0(static_cast<std::byte*>(self) + 0x16C,
        static_cast<const std::byte*>(source) + 0x16C);
    return self;
}
void* gui_nine_slice_construct_00ac0c90(void* self, void* widget, const float (&size)[2], GuiNineSliceHost& host) {
    write_field<float>(self, 0x0C, size[0]);
    write_field<float>(self, 0x10, size[1]);
    write_field<std::uint32_t>(self, 0, kGuiNineSliceBaseVtable);
    write_field<void*>(self, 0x04, widget);
    void* const clone = host.clone_subtree_00aab4c0(widget, 0);
    write_field<void*>(self, 0x08, clone);
    host.clone_virtual_34(clone, 0);
    write_field<std::uint32_t>(self, 0, kGuiNineSliceVtable);
    static constexpr const char* kSlices[8] = {
        "UL_Icon", "U_Icon", "UR_Icon", "L_Icon", "R_Icon", "BL_Icon", "B_Icon", "BR_Icon"};
    for (std::size_t i = 0; i < 8; ++i)
        write_field<void*>(self, 0x14 + 4 * i, host.find_child_00aa7e00(widget, kSlices[i], 1));
    return self;
}

void gui_preload_lua_assets_00ac6be0(void* textures, void* models, void* sounds, const void* table,
    GuiLuaPreloadHost& host) {
    GuiLuaPreloadHost::Entry entry;
    for (bool more = host.first_00b67080(table, entry); more; more = host.next_00b67190(table, entry)) {
        if (entry.value_is_table) {
            gui_preload_lua_assets_00ac6be0(textures, models, sounds, entry.value_table, host);
            continue;
        }
        if (!entry.key_is_string || !entry.value_is_string) continue;
        if (entry.key == "Texture") {
            void* const texture = host.resolve_texture_00aa2660(entry.value, 0.0f);
            host.push_texture_00ac6b50(textures, texture);
            if (texture != nullptr) host.release_object(texture);
        } else if (entry.key == "ModelName") {
            void* const model = host.load_resource_00b80d70(entry.value);
            host.push_model_00ac6ac0(models, model);
            if (model != nullptr) host.release_object(model);
        } else if (entry.key == "SoundEfx") {
            host.push_sound_from_cache_00a83fd0(sounds, entry.value);
        }
    }
}
} // namespace bsp::gui_widgets_cc13
