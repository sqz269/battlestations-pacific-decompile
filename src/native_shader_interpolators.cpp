#include "bsp/native_shader_interpolators.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <limits>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
using Operation = NativeShaderInterpolatorOperation;
using Phase = Operation::Phase;
template<class T> T current(const void* p, std::size_t offset = 0) noexcept {
    return *reinterpret_cast<const volatile T*>(static_cast<const std::byte*>(p) + offset);
}
template<class T> void publish(void* p, std::size_t offset, T value) noexcept {
    *reinterpret_cast<volatile T*>(static_cast<std::byte*>(p) + offset) = value;
}
void require(bool condition, const char* text) {
    if (!condition) throw std::logic_error(text);
}
void begin(Operation& a, std::uint32_t function) {
    require(a.phase == Phase::fresh, "native interpolator operation cannot replay");
    a.function = function;
}
std::int32_t signed_bits(std::uint32_t bits) noexcept {
    std::int32_t value; std::memcpy(&value, &bits, sizeof value); return value;
}
void extent(const NativeShaderDescriptorArray& rows, std::uint32_t stride) {
    const auto count = current<std::int32_t>(&rows, 4);
    const auto cap = current<std::int32_t>(&rows, 8);
    require(count >= 0 && cap >= count &&
        static_cast<std::uint32_t>(cap) <= static_cast<std::uint32_t>((std::numeric_limits<std::int32_t>::max)()) / stride,
        "native interpolator array requires readable nonnegative extents");
    require(count == 0 || current<void*>(&rows), "native interpolator rows require current data");
}
NativeShaderFieldStorage* descriptor_field(NativeShaderDescriptorStorage* descriptor, std::uint32_t i) {
    auto* const data = current<NativeShaderFieldStorage**>(descriptor, 0xdc);
    return current<NativeShaderFieldStorage*>(data, i * 4u);
}
NativeShaderFieldStorage* output_field(NativeShaderDescriptorArray& rows, std::uint32_t i) {
    return current<NativeShaderFieldStorage*>(current<void*>(&rows), i * 4u);
}
void reserve_words(NativeShaderDescriptorArray& rows, std::int32_t capacity, Operation& a) {
    if (capacity < 1) capacity = 1;
    if (current<std::int32_t>(&rows, 8) >= capacity) return;
    extent(rows, 2);
    require(capacity <= (std::numeric_limits<std::int32_t>::max)() / 2,
        "native interpolator reserve requires representable allocation");
    a.reserve_rows = &rows; a.reserve_capacity = capacity;
    a.phase = Phase::word_allocation; a.native_site = 0x00b346fc;
    const auto bytes = static_cast<std::uint32_t>(capacity) * 2u;
    a.word_allocation = singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
    a.phase = Phase::word_copy; a.native_site = 0x00b34710;
    auto cursor = reinterpret_cast<std::uintptr_t>(a.word_allocation);
    for (std::int32_t i = 0; i < current<std::int32_t>(&rows, 4); ++i, cursor += 2u) {
        if (cursor) publish(reinterpret_cast<void*>(cursor), 0,
            current<std::uint16_t>(current<void*>(&rows), static_cast<std::uint32_t>(i) * 2u));
    }
    a.phase = Phase::word_free; a.native_site = 0x00b3472b;
    singleton_lifetime_free(current<void*>(&rows));
    publish(&rows, 0, a.word_allocation); publish(&rows, 8, capacity);
    a.word_allocation = nullptr;
}
void publish_field(Operation& a, std::uint32_t reserve_site, std::uint32_t store_site, bool initial) {
    auto& rows = *a.output; extent(rows, 4);
    const auto capacity = current<std::int32_t>(&rows, 8);
    if (current<std::int32_t>(&rows, 4) == capacity) {
        a.phase = Phase::field_reserve; a.native_site = reserve_site;
        auto next = signed_bits(static_cast<std::uint32_t>(capacity) + 5u);
        if (next <= 10) next = 10;
        reserve_native_shader_field_pointers_00b34680(rows, next);
    }
    a.phase = Phase::field_publication; a.native_site = store_site;
    // The initial append loads data before count; later appends reverse them.
    void* data{}; std::uint32_t count{};
    if (initial) { data = current<void*>(&rows); count = current<std::uint32_t>(&rows, 4); }
    else { count = current<std::uint32_t>(&rows, 4); data = current<void*>(&rows); }
    const auto slot = reinterpret_cast<std::uintptr_t>(data) + count * 4u;
    if (slot) publish(reinterpret_cast<void*>(slot), 0, a.current_field);
    publish(&rows, 4, current<std::uint32_t>(&rows, 4) + 1u);
    a.field_published = slot != 0;
    require(!a.current_field || a.field_published, "native interpolator creator requires a readable publication slot");
}
void choose(Operation& a, const NativeShaderFieldStorage& source) {
    a.selected_count = 0; a.selected_mask = 0;
    if (!a.texcoord_usage && !a.color_usage) {
        a.selected_count = current<std::uint32_t>(&source, 0x0c);
        for (std::uint32_t i = 0; i < a.selected_count; ++i) a.selected_mask |= 1u << (i & 31u);
        return;
    }
    const auto semantic = current<std::int32_t>(&source, 0x14);
    const volatile std::uint32_t* usage{}; std::uint32_t* offset{};
    if (a.texcoord_usage && semantic == 2) { usage = a.texcoord_usage; offset = &a.texcoord_offset; }
    else if (a.color_usage && semantic == 1) { usage = a.color_usage; offset = &a.color_offset; }
    if (!usage) return;
    const auto width = current<std::uint32_t>(&source, 0x0c);
    for (std::uint32_t i = 0; i < width; ++i) {
        const auto component = width - i - 1u;
        const auto flattened = component + *offset;
        if (usage[flattened >> 2u] & (1u << (flattened & 3u))) {
            ++a.selected_count; a.selected_mask |= 1u << (component & 31u);
        }
    }
    *offset += width;
}
} // namespace

NativeShaderInterpolatorOperation::~NativeShaderInterpolatorOperation() {
    if (phase != Phase::fresh && phase != Phase::complete && phase != Phase::diagnostic_retired) std::terminate();
}
void NativeShaderInterpolatorOperation::acknowledge_diagnostic_cleanup() noexcept {
    if (phase != Phase::failed || current_field || temporary_live || word_allocation) std::terminate();
    phase = Phase::diagnostic_retired;
}
void reserve_native_shader_interpolator_words_00b346e0(NativeShaderDescriptorArray& rows,
    std::int32_t capacity, Operation& a) {
    begin(a, 0x00b346e0); a.output = &rows;
    try { reserve_words(rows, capacity, a); a.phase = Phase::complete; }
    catch (...) { a.phase = Phase::failed; throw; }
}
void select_native_shader_interpolator_fields_00b36800(NativeMaterialProgramBuilderStorage& builder,
    const volatile std::uint32_t* texcoord_usage, const volatile std::uint32_t* color_usage,
    NativeShaderDescriptorArray& output, NativeStringStorage& strings, Operation& a) {
    begin(a, 0x00b36800); a.builder = &builder; a.output = &output; a.strings = &strings;
    a.texcoord_usage = texcoord_usage; a.color_usage = color_usage;
    try {
        a.phase = Phase::field_allocation; a.native_site = 0x00b3682c;
        a.current_field = static_cast<NativeShaderFieldStorage*>(
            singleton_lifetime_allocate({SingletonAllocationKind::object, 0x1c, 0x1c}));
        if (a.current_field) {
            a.phase = Phase::temporary_name; a.native_site = 0x00b3684b; a.temporary_live = true;
            construct_native_string_cstring_0041e870(&a.temporary_name, "ScreenSpacePos", strings);
            a.phase = Phase::field_copy; a.native_site = 0x00b3686b; a.field_name_initialized = true;
            construct_native_shader_field_00b34e20(a.current_field, a.temporary_name, 0, 4, 0, 0, 0, strings);
        }
        publish_field(a, 0x00b36896, 0x00b368a9, true);
        a.phase = Phase::temporary_cleanup; a.native_site = 0x00b368d6;
        if (a.temporary_live) { destroy_native_string_header_0041dd20(&a.temporary_name, strings); a.temporary_live = false; }
        a.current_field = nullptr; a.field_name_initialized = false; ++a.completed_rows;
        for (const std::uint32_t offset : {0x70u, 0x74u}) {
            a.descriptor_offset = offset; a.index = 0;
            auto* descriptor = current<NativeShaderDescriptorStorage*>(&builder, offset);
            while (a.index < current<std::uint32_t>(descriptor, 0xe0)) {
                a.source_field = descriptor_field(descriptor, a.index);
                choose(a, *a.source_field);
                if ((!texcoord_usage && !color_usage) || a.selected_count) {
                    const bool first = offset == 0x70;
                    a.field_published = false; a.field_name_initialized = false;
                    a.phase = Phase::field_allocation; a.native_site = first ? 0x00b36a74 : 0x00b36d10;
                    a.current_field = static_cast<NativeShaderFieldStorage*>(
                        singleton_lifetime_allocate({SingletonAllocationKind::object, 0x1c, 0x1c}));
                    if (a.current_field) {
                        descriptor = current<NativeShaderDescriptorStorage*>(&builder, offset);
                        // Four separate native data-pointer captures precede the field
                        // dereferences. Preserve the scalar inputs across pooled allocation.
                        auto* const index_data = current<void*>(descriptor, 0xdc);
                        auto* const semantic_data = current<void*>(descriptor, 0xdc);
                        auto* const scalar_data = current<void*>(descriptor, 0xdc);
                        auto* const name_data = current<void*>(descriptor, 0xdc);
                        auto* const index_field = current<NativeShaderFieldStorage*>(index_data, a.index * 4u);
                        auto* const semantic_field = current<NativeShaderFieldStorage*>(semantic_data, a.index * 4u);
                        a.captured_semantic = current<std::int32_t>(semantic_field, 0x14);
                        a.captured_index = current<std::int32_t>(index_field, 0x18);
                        auto* const scalar_field = current<NativeShaderFieldStorage*>(scalar_data, a.index * 4u);
                        a.source_field = current<NativeShaderFieldStorage*>(name_data, a.index * 4u);
                        a.captured_scalar = current<std::int32_t>(scalar_field, 8);
                        a.phase = Phase::field_copy; a.native_site = first ? 0x00b36ae6 : 0x00b36d82;
                        a.field_name_initialized = true;
                        construct_native_shader_field_00b34e20(a.current_field, a.source_field->name_00,
                            a.captured_scalar, signed_bits(a.selected_count), a.captured_semantic,
                            a.captured_index, a.selected_mask, strings);
                    }
                    publish_field(a, first ? 0x00b36b46 : 0x00b36de2, first ? 0x00b36b57 : 0x00b36df3, false);
                    a.current_field = nullptr; a.field_name_initialized = false; ++a.completed_rows;
                }
                ++a.index; descriptor = current<NativeShaderDescriptorStorage*>(&builder, offset);
            }
        }
        a.phase = Phase::complete;
    } catch (...) { a.phase = Phase::failed; throw; }
}
void append_native_shader_interpolator_mapping_00b34aa0(NativeMaterialProgramBuilderStorage& builder,
    NativeShaderDescriptorArray& fields, Operation& a) {
    begin(a, 0x00b34aa0); a.builder = &builder; a.output = &fields;
    try {
        a.index = 1;
        while (a.index < current<std::uint32_t>(&fields, 4)) {
            const auto semantic = current<std::int32_t>(output_field(fields, a.index), 0x14);
            if (semantic == 2 || semantic == 1) {
                for (a.component = 0; a.component != 4; ++a.component) {
                    if (!(current<std::uint32_t>(output_field(fields, a.index), 0x10) & (1u << a.component))) continue;
                    auto& rows = semantic == 2 ? builder.words_54 : builder.words_60;
                    extent(rows, 2);
                    a.mapping_word = static_cast<std::uint16_t>((a.index & 0xffu) | (a.component << 8u));
                    const auto capacity = current<std::int32_t>(&rows, 8);
                    if (current<std::int32_t>(&rows, 4) == capacity) {
                        auto next = signed_bits(static_cast<std::uint32_t>(capacity) * 2u);
                        if (next < 2) next = 1;
                        reserve_words(rows, next, a);
                    }
                    a.phase = Phase::word_publication; a.native_site = semantic == 2 ? 0x00b34b28 : 0x00b34b97;
                    const auto count = current<std::uint32_t>(&rows, 4);
                    const auto slot = reinterpret_cast<std::uintptr_t>(current<void*>(&rows)) + count * 2u;
                    if (slot) publish(reinterpret_cast<void*>(slot), 0, a.mapping_word);
                    publish(&rows, 4, current<std::uint32_t>(&rows, 4) + 1u);
                }
            } else if (semantic == 8) {
                a.native_site = 0x00b34bc2;
                publish(&builder, 0x6c, static_cast<std::uint16_t>(a.index & 0xffu));
            }
            ++a.index; ++a.completed_rows;
        }
        a.phase = Phase::complete;
    } catch (...) { a.phase = Phase::failed; throw; }
}
} // namespace bsp
