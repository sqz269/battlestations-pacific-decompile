#include "bsp/native_shader_field_initialization.hpp"
#include "bsp/native_lua_script_overrides.hpp"
#include "bsp/native_physical_file_date.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using Frame = NativeShaderFieldInitializationOperation;
using Step = Frame::Step;
std::uint32_t word(const void* p, std::size_t offset = 0) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(static_cast<const char*>(p) + offset);
}
template<class T> T* pointer(const void* p, std::size_t offset = 4) noexcept {
    return *reinterpret_cast<T* const volatile*>(static_cast<const char*>(p) + offset);
}
void clear(NativeString& s) noexcept {
    *reinterpret_cast<volatile std::uint32_t*>(&s) = 0;
    *reinterpret_cast<char* volatile*>(reinterpret_cast<char*>(&s) + 4) = nullptr;
}
void at(Frame& a, Step step, std::uint32_t site) noexcept { a.step = step; a.native_site = site; }
void begin(Frame& a, NativeShaderFieldInitializationContext& context, std::uint32_t fn) {
    if (a.phase != Frame::Phase::fresh) throw std::logic_error("shader field initialization is one-shot");
    a.context = &context; a.function = fn; a.phase = Frame::Phase::running;
}
const char* text_or_empty(const void* header, const Frame& a) noexcept {
    const auto* data = pointer<char>(header);
    return data ? data : a.context->lines.actual_empty_0108d6f2;
}
std::int32_t signed_word(std::uint32_t value) noexcept {
    std::int32_t result; std::memcpy(&result, &value, 4); return result;
}
void swizzle(const NativeShaderFieldStorage& field, NativeString& out, Frame& a) {
    a.field_before_swizzle = &field; a.swizzle_output = &out;
    clear(out); a.swizzle_initialized = true; a.assignment_entered = false; a.assignment_returned = false;
    const auto index = word(&field, 0xc) - 1u;
    if (index < 4u) {
        static constexpr const char* values[]{"x", "xy", "xyz", "xyzw"};
        at(a, Step::swizzle_assign, 0x00b34eef); a.assignment_entered = true;
        assign_native_string_cstring_0041e350(&out, values[index], a.context->lines.strings);
        a.assignment_returned = true;
    }
}
template<class... Args> void formatted(Frame& a, std::uint32_t site, const char* format, Args... args) {
    at(a, Step::formatted_line, site);
    a.line = std::make_unique<NativeShaderConstantHeaderOperation>();
    append_native_shader_line_00b35110(a.builder->source_4c, a.context->lines, *a.line, format, args...);
}
} // namespace
NativeShaderFieldInitializationOperation::~NativeShaderFieldInitializationOperation() {
    if (phase == Phase::running || phase == Phase::failed) std::terminate();
}
void NativeShaderFieldInitializationOperation::acknowledge_diagnostic_cleanup() noexcept {
    if (phase == Phase::failed) {
        if (line) line->acknowledge_diagnostic_cleanup();
        phase = Phase::diagnostic_retired;
    }
}
NativeString& construct_native_shader_field_swizzle_00b34e90(const NativeShaderFieldStorage& field,
    NativeString& out, NativeShaderFieldInitializationContext& context, Frame& a) {
    begin(a, context, 0x00b34e90);
    try { swizzle(field, out, a); a.phase = Frame::Phase::complete; return out; }
    catch (...) { a.phase = Frame::Phase::failed; throw; }
}
void append_native_shader_zero_fields_00b357d0(NativeMaterialProgramBuilderStorage& builder,
    const NativeString& instance, const NativeShaderDescriptorArray& fields,
    NativeShaderFieldInitializationContext& context, Frame& a) {
    begin(a, context, 0x00b357d0); a.builder = &builder; a.instance = &instance; a.fields = &fields;
    try {
        while (a.row < word(&fields, 4)) {
            at(a, Step::field_name, 0x00b357e6);
            const auto* slots = pointer<NativeShaderFieldStorage*>(&fields, 0);
            const auto* field = *reinterpret_cast<NativeShaderFieldStorage* const volatile*>(slots + a.row);
            a.captured_field_name = text_or_empty(field, a);
            at(a, Step::instance_getter, 0x00b357fa);
            a.instance_name = native_string_data_or_00419ca0(instance, context.actual_instance_empty_00e17654);
            formatted(a, 0x00b35806, "\t\t%s.%s=0;", a.instance_name, a.captured_field_name);
            ++a.row; ++a.completed_rows;
        }
        a.phase = Frame::Phase::complete;
    } catch (...) { a.phase = Frame::Phase::failed; throw; }
}
void append_native_shader_vertex_decode_00b35820(NativeMaterialProgramBuilderStorage& builder,
    NativeShaderFieldInitializationContext& context, Frame& a) {
    begin(a, context, 0x00b35820); a.builder = &builder; a.fields = &builder.fields_04;
    try {
        at(a, Step::descriptor_gate, 0x00b3583b);
        a.captured_descriptor = pointer<NativeShaderDescriptorStorage>(&builder, 0x70);
        const auto gate = *reinterpret_cast<const volatile std::uint8_t*>(reinterpret_cast<const char*>(a.captured_descriptor) + 0x1f);
        if (gate) {
            at(a, Step::prefix_limit, 0x00b35848);
            const auto descriptor_limit = word(a.captured_descriptor, 0x20);
            const auto input_count = word(&builder, 8);
            a.limit = descriptor_limit < input_count ? descriptor_limit : input_count;
            while (a.row < a.limit) {
                const auto* slots = pointer<NativeShaderFieldStorage*>(&builder, 4);
                const auto* field = *reinterpret_cast<NativeShaderFieldStorage* const volatile*>(slots + a.row);
                at(a, Step::swizzle_construct, 0x00b35883); a.swizzle_live = true;
                swizzle(*field, a.swizzle, a);
                a.swizzle_text = text_or_empty(&a.swizzle, a);
                // The table is reloaded once after the allocating child. Native
                // then loads the row/name twice, right-hand expression first.
                const auto* current = pointer<NativeShaderFieldStorage*>(&builder, 4);
                const auto* right = *reinterpret_cast<NativeShaderFieldStorage* const volatile*>(current + a.row);
                a.right_field_name = text_or_empty(right, a);
                const auto* left = *reinterpret_cast<NativeShaderFieldStorage* const volatile*>(current + a.row);
                a.left_field_name = text_or_empty(left, a);
                formatted(a, 0x00b358d6,
                    "\tIN.%s=IN.%s * cVtxElemScale[%i].%s + cVtxElemOffset[%i].%s;",
                    a.left_field_name, a.right_field_name, signed_word(a.row), a.swizzle_text, signed_word(a.row), a.swizzle_text);
                at(a, Step::swizzle_release, 0x00b35900);
                destroy_native_string_header_0041dd20(&a.swizzle, context.lines.strings); a.swizzle_live = false;
                ++a.row; ++a.completed_rows;
            }
        }
        a.phase = Frame::Phase::complete;
    } catch (...) { a.phase = Frame::Phase::failed; throw; }
}
} // namespace bsp
