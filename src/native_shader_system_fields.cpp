#include "bsp/native_shader_system_fields.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <limits>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
template<class T> T current(const void* p, std::size_t offset = 0) noexcept {
    return *reinterpret_cast<const volatile T*>(static_cast<const std::byte*>(p) + offset);
}
template<class T> void publish(void* p, std::size_t offset, T value) noexcept {
    *reinterpret_cast<volatile T*>(static_cast<std::byte*>(p) + offset) = value;
}
struct Field {
    const char* name;
    std::int32_t width, semantic;
    std::uint32_t allocate, temporary, construct, reserve, release, publication;
};
// Literal contents and all operands are pinned to EACH native call site.
constexpr Field vertex_fields[16]{
    {"ScreenSpacePos", 4, 0, 0x00b35c0c, 0x00b35c2e, 0x00b35c4f, 0x00b35c7d, 0x00b35cc0, 0x00b35c8e},
    {"ObjectSpacePos", 4, 0, 0x00b35cc7, 0x00b35cea, 0x00b35d0c, 0x00b35d37, 0x00b35d7a, 0x00b35d48},
    {"WorldSpacePos", 4, 0, 0x00b35d81, 0x00b35da4, 0x00b35dc6, 0x00b35df1, 0x00b35e34, 0x00b35e02},
    {"Normal", 3, 3, 0x00b35e3b, 0x00b35e5e, 0x00b35e80, 0x00b35eab, 0x00b35eee, 0x00b35ebc},
    {"Tangent", 3, 5, 0x00b35ef5, 0x00b35f18, 0x00b35f3a, 0x00b35f65, 0x00b35fa8, 0x00b35f76},
    {"BiNormal", 3, 4, 0x00b35faf, 0x00b35fd2, 0x00b35ff4, 0x00b3601f, 0x00b36062, 0x00b36030},
    {"CustomValue0", 1, 2, 0x00b36069, 0x00b3608c, 0x00b360ad, 0x00b360d8, 0x00b3611b, 0x00b360e9},
    {"CustomValue1", 1, 2, 0x00b36122, 0x00b36145, 0x00b36169, 0x00b36194, 0x00b361d9, 0x00b361a5},
    {"CustomValue2", 1, 2, 0x00b361e0, 0x00b36203, 0x00b36227, 0x00b36252, 0x00b3629b, 0x00b36263},
    {"CustomValue3", 1, 2, 0x00b362a2, 0x00b362c5, 0x00b362e9, 0x00b36314, 0x00b3635d, 0x00b36325},
    {"CustomVec2_0", 2, 2, 0x00b36364, 0x00b36387, 0x00b363ac, 0x00b363d7, 0x00b36420, 0x00b363e8},
    {"CustomVec2_1", 2, 2, 0x00b36427, 0x00b3644a, 0x00b3646f, 0x00b3649a, 0x00b364e3, 0x00b364ab},
    {"CustomVec2_2", 2, 2, 0x00b364ea, 0x00b3650d, 0x00b36532, 0x00b3655d, 0x00b365a6, 0x00b3656e},
    {"CustomVec2_3", 2, 2, 0x00b365ad, 0x00b365d0, 0x00b365f5, 0x00b36620, 0x00b36669, 0x00b36631},
    {"CustomVec4_0", 4, 2, 0x00b36670, 0x00b36693, 0x00b366b8, 0x00b366e3, 0x00b3672c, 0x00b366f4},
    {"CustomVec4_1", 4, 2, 0x00b36733, 0x00b36756, 0x00b3677b, 0x00b367a6, 0x00b367e5, 0x00b367b7},
};
constexpr Field pixel_fields[16]{
    {"DiffuseColor", 4, 2, 0x00b372fc, 0x00b3731e, 0x00b3733f, 0x00b3736d, 0x00b373b0, 0x00b3737e},
    {"SpecularColor", 4, 2, 0x00b373b7, 0x00b373da, 0x00b373fc, 0x00b37427, 0x00b3746a, 0x00b37438},
    {"EmissiveColor", 4, 2, 0x00b37471, 0x00b37494, 0x00b374b6, 0x00b374e1, 0x00b37524, 0x00b374f2},
    {"SpecularPower", 1, 2, 0x00b3752b, 0x00b3754e, 0x00b3756f, 0x00b3759a, 0x00b375dd, 0x00b375ab},
    {"Normal", 3, 2, 0x00b375e4, 0x00b37607, 0x00b37629, 0x00b37654, 0x00b37697, 0x00b37665},
    {"Depth", 1, 2, 0x00b3769e, 0x00b376c1, 0x00b376e2, 0x00b3770d, 0x00b37750, 0x00b3771e},
    {"CustomValue0", 1, 2, 0x00b37757, 0x00b3777a, 0x00b3779b, 0x00b377c6, 0x00b37809, 0x00b377d7},
    {"CustomValue1", 1, 2, 0x00b37810, 0x00b37833, 0x00b37857, 0x00b37882, 0x00b378c7, 0x00b37893},
    {"CustomValue2", 1, 2, 0x00b378ce, 0x00b378f1, 0x00b37915, 0x00b37940, 0x00b37989, 0x00b37951},
    {"CustomValue3", 1, 2, 0x00b37990, 0x00b379b3, 0x00b379d7, 0x00b37a02, 0x00b37a4b, 0x00b37a13},
    {"CustomVec2_0", 2, 2, 0x00b37a52, 0x00b37a75, 0x00b37a9a, 0x00b37ac5, 0x00b37b0e, 0x00b37ad6},
    {"CustomVec2_1", 2, 2, 0x00b37b15, 0x00b37b38, 0x00b37b5d, 0x00b37b88, 0x00b37bd1, 0x00b37b99},
    {"CustomVec2_2", 2, 2, 0x00b37bd8, 0x00b37bfb, 0x00b37c20, 0x00b37c4b, 0x00b37c94, 0x00b37c5c},
    {"CustomVec2_3", 2, 2, 0x00b37c9b, 0x00b37cbe, 0x00b37ce3, 0x00b37d0e, 0x00b37d57, 0x00b37d1f},
    {"CustomVec4_0", 4, 2, 0x00b37d5e, 0x00b37d81, 0x00b37da6, 0x00b37dd1, 0x00b37e1a, 0x00b37de2},
    {"CustomVec4_1", 4, 2, 0x00b37e21, 0x00b37e44, 0x00b37e69, 0x00b37e94, 0x00b37ed3, 0x00b37ea5},
};
std::int32_t grown_capacity(std::int32_t capacity) noexcept {
    const auto bits = static_cast<std::uint32_t>(capacity) + 5u;
    std::int32_t value; std::memcpy(&value, &bits, 4);
    return value > 10 ? value : 10;
}
void append(NativeMaterialProgramBuilderStorage& builder, NativeStringStorage& strings,
    NativeShaderSystemFieldsOperation& a, const Field (&fields)[16], bool pixel) {
    using Phase = NativeShaderSystemFieldsOperation::Phase;
    if (a.phase != Phase::fresh)
        throw std::logic_error("shader system-field operation cannot replay");
    a.builder = &builder; a.strings = &strings;
    a.function = pixel ? 0x00b372d0 : 0x00b35be0;
    auto& rows = pixel ? builder.fields_34 : builder.fields_10;
    try {
        for (std::uint32_t i = 0; i != 16; ++i) {
            const auto& f = fields[i]; a.row = i; a.source_name = f.name;
            a.field_name_initialized = false; a.field_constructor_returned = false;
            a.field_published = false; a.temporary_initialized = false;
            a.phase = Phase::allocation; a.native_site = f.allocate;
            void* const raw = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x1c, 0x1c});
            a.current_field = static_cast<NativeShaderFieldStorage*>(raw);
            if (raw) {
                a.phase = Phase::temporary_name; a.native_site = f.temporary;
                a.temporary_initialized = true; a.temporary_live = true;
                construct_native_string_cstring_0041e870(&a.temporary_name, f.name, strings);
                a.phase = Phase::field_constructor; a.native_site = f.construct;
                a.field_name_initialized = true;
                construct_native_shader_field_00b34e20(raw, a.temporary_name,
                    0, f.width, f.semantic, 0, 0, strings);
                a.field_constructor_returned = true;
            }
            const auto capacity = current<std::int32_t>(&rows, 8);
            const auto count = current<std::int32_t>(&rows, 4);
            if (count < 0 || capacity < count || capacity > (std::numeric_limits<std::int32_t>::max)() / 4)
                throw std::logic_error("shader field append requires readable native array extents");
            if (count == capacity) {
                a.phase = Phase::reserve; a.native_site = f.reserve;
                reserve_native_shader_field_pointers_00b34680(rows, grown_capacity(capacity));
            }
            a.phase = Phase::publication; a.native_site = f.publication;
            const auto current_count = current<std::uint32_t>(&rows, 4);
            void* const current_data = current<void*>(&rows);
            const auto slot = reinterpret_cast<std::uintptr_t>(current_data) + current_count * 4u;
            if (slot) publish(reinterpret_cast<void*>(slot), 0, a.current_field);
            publish(&rows, 4, current<std::uint32_t>(&rows, 4) + 1u);
            a.field_published = slot != 0;
            a.phase = Phase::name_cleanup; a.native_site = f.release;
            if (a.temporary_live) {
                destroy_native_string_header_0041dd20(&a.temporary_name, strings);
                a.temporary_live = false;
            }
            // Valid readable arrays publish the only creator into the builder.
            // Keep an otherwise unowned creator instead of treating it as success.
            if (a.current_field && !a.field_published)
                throw std::logic_error("shader field creator has no readable publication slot");
            a.current_field = nullptr; a.field_name_initialized = false;
            a.completed_rows = i + 1;
        }
        a.phase = Phase::complete;
    } catch (...) { a.phase = Phase::failed; throw; }
}
} // namespace

NativeShaderSystemFieldsOperation::~NativeShaderSystemFieldsOperation() {
    if (phase != Phase::fresh && phase != Phase::complete && phase != Phase::diagnostic_retired)
        std::terminate();
}
void NativeShaderSystemFieldsOperation::acknowledge_diagnostic_cleanup() noexcept {
    if (phase != Phase::failed || current_field || temporary_live) std::terminate();
    phase = Phase::diagnostic_retired;
}
NativeShaderFieldStorage* construct_native_shader_field_00b34e20(void* raw,
    const NativeString& name, std::int32_t scalar, std::int32_t width,
    std::int32_t semantic, std::int32_t index, std::uint32_t mask, NativeStringStorage& strings) {
    auto* const field = ::new(raw) NativeShaderFieldStorage;
    // Native compares name-header identities, clears destination, then copies.
    copy_native_string_header_00be0a30_fragment(&field->name_00, strings, &name);
    publish(field, 8, scalar); publish(field, 0x0c, width); publish(field, 0x10, mask);
    publish(field, 0x14, semantic); publish(field, 0x18, index);
    return field;
}
void append_native_shader_vertex_system_fields_00b35be0(NativeMaterialProgramBuilderStorage& builder,
    NativeStringStorage& strings, NativeShaderSystemFieldsOperation& acquired) {
    append(builder, strings, acquired, vertex_fields, false);
}
void append_native_shader_pixel_system_fields_00b372d0(NativeMaterialProgramBuilderStorage& builder,
    NativeStringStorage& strings, NativeShaderSystemFieldsOperation& acquired) {
    append(builder, strings, acquired, pixel_fields, true);
}
} // namespace bsp
