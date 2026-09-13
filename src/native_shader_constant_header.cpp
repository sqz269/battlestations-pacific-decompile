#define _CRT_SECURE_NO_WARNINGS
#include "bsp/native_shader_constant_header.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_material_parameters.hpp"
#include <cstdio>
#include <cstring>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using Frame = NativeShaderConstantHeaderOperation;
using Step = Frame::Step;
std::uint32_t word(const void* p, std::size_t offset = 0) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(static_cast<const char*>(p) + offset);
}
char* data(const void* p) noexcept {
    return *reinterpret_cast<char* const volatile*>(static_cast<const char*>(p) + 4);
}
void clear(void* p) noexcept {
    *static_cast<volatile std::uint32_t*>(p) = 0;
    *reinterpret_cast<char* volatile*>(static_cast<char*>(p) + 4) = nullptr;
}
void begin(Frame& a, NativeShaderConstantHeaderContext& c, std::uint32_t fn) {
    if (a.phase != Frame::Phase::fresh) throw std::logic_error("constant header operation is one-shot");
    if (!c.actual_format_scratch_0108d6f8 || !c.actual_empty_0108d6f2)
        throw std::invalid_argument("actual shader format buffer and empty literal are required");
    a.context = &c; a.function = fn; a.phase = Frame::Phase::running;
}
void at(Frame& a, Step step, std::uint32_t site) noexcept { a.step = step; a.native_site = site; }
void copy_bytes(void* dst, const void* src, std::uint32_t size) {
    if (size) std::memmove(dst, src, size); // BF7680 admits overlap.
}
void release_current(Frame& a) noexcept {
    destroy_native_string_header_0041dd20(&a.temporary, a.context->strings);
    a.temporary_live = false;
}
void decimal(NativeString& out, std::int32_t value, Frame& a) {
    clear(&out);
    at(a, Step::number_format, 0x005f1874);
    std::sprintf(a.decimal_text, "%d", value);
    clear(&a.decimal_temporary); a.decimal_live = true; a.decimal_captured = false;
    at(a, Step::number_resize, 0x005f18a2);
    resize_native_string_header_0041dd40(&a.decimal_temporary, a.context->strings,
        static_cast<std::uint32_t>(std::strlen(a.decimal_text)), true);
    a.decimal_data = data(&a.decimal_temporary); a.decimal_length = word(&a.decimal_temporary); a.decimal_captured = true;
    if (a.decimal_data) copy_bytes(a.decimal_data, a.decimal_text, a.decimal_length + 1u);
    if (&out != &a.decimal_temporary) {
        at(a, Step::number_copy, 0x005f18d6);
        resize_native_string_header_0041dd40(&out, a.context->strings, a.decimal_length, true);
        if (a.decimal_length) copy_bytes(data(&out), a.decimal_data, word(&out));
    }
    at(a, Step::number_release, 0x005f1902);
    if (a.decimal_data) a.context->strings.release(a.decimal_data, a.decimal_length + 1u);
    a.decimal_live = false;
}
void line_part(NativeString& out, Frame& a, const char* text, std::uint32_t length, bool newline) {
    clear(&a.line_temporary); a.line_live = true; a.line_captured = false;
    at(a, newline ? Step::newline_resize : Step::line_resize, newline ? 0x00b351e6 : 0x00b35167);
    resize_native_string_header_0041dd40(&a.line_temporary, a.context->strings, length, true);
    a.line_data = data(&a.line_temporary); a.line_length = word(&a.line_temporary); a.line_captured = true;
    if (a.line_data) copy_bytes(a.line_data, text, a.line_length + 1u);
    if (a.line_length) {
        a.old_length = word(&out);
        at(a, newline ? Step::newline_append : Step::line_append, newline ? 0x00b3521f : 0x00b351a0);
        resize_native_string_header_0041dd40(&out, a.context->strings, a.old_length + a.line_length, true);
        copy_bytes(data(&out) + a.old_length, a.line_data, a.line_length);
    }
    at(a, newline ? Step::newline_release : Step::line_release, newline ? 0x00b35249 : 0x00b351ca);
    if (a.line_data) a.context->strings.release(a.line_data, a.line_length + 1u);
    a.line_live = false;
}
void vline(NativeString& out, Frame& a, const char* format, std::va_list args) {
    a.destination = &out;
    at(a, Step::line_format, 0x00b3513b);
    std::vsprintf(a.context->actual_format_scratch_0108d6f8, format, args);
    line_part(out, a, a.context->actual_format_scratch_0108d6f8,
        static_cast<std::uint32_t>(std::strlen(a.context->actual_format_scratch_0108d6f8)), false);
    line_part(out, a, "\n", 1, true);
}
void line(NativeString& out, Frame& a, const char* format, ...) {
    std::va_list args; va_start(args, format);
    try { vline(out, a, format, args); } catch (...) { va_end(args); throw; }
    va_end(args);
}
void append_suffix(Frame& a, std::uint32_t resize_site) {
    a.append_length = word(&a.temporary);
    if (a.append_length) {
        a.old_length = word(&a.suffix);
        at(a, Step::suffix_append, resize_site);
        resize_native_string_header_0041dd40(&a.suffix, a.context->strings,
            a.old_length + a.append_length, true);
        // EBX is refreshed only after resize, and remains captured across release.
        a.suffix_data = data(&a.suffix);
        copy_bytes(a.suffix_data + a.old_length, data(&a.temporary), a.append_length);
    }
}
std::int32_t signed_word(std::uint32_t v) noexcept {
    std::int32_t s; std::memcpy(&s, &v, 4); return s;
}
void declaration(NativeMaterialProgramBuilderStorage& owner,
    const NativeCompiledShaderConstantStorage& record, std::int32_t reg, Frame& a) {
    a.record = &record; a.builder = &owner;
    clear(&a.suffix); a.suffix_data = nullptr; a.suffix_live = true;
    if (reg >= 0) {
        at(a, Step::prefix, 0x00b38ca5); a.temporary_live = true;
        construct_native_string_cstring_0041e870(&a.temporary, " : register(c", a.context->strings);
        at(a, Step::prefix_copy, 0x00b38cc2);
        resize_native_string_header_0041dd40(&a.suffix, a.context->strings, word(&a.temporary), true);
        a.suffix_data = data(&a.suffix);
        if (word(&a.temporary)) copy_bytes(a.suffix_data, data(&a.temporary), word(&a.suffix));
        at(a, Step::temporary_release, 0x00b38cfa); release_current(a);
        a.temporary_live = true; decimal(a.temporary, reg, a);
        append_suffix(a, 0x00b38d2d);
        at(a, Step::temporary_release, 0x00b38d5f); release_current(a);
        a.temporary_live = true;
        at(a, Step::prefix, 0x00b38d74);
        construct_native_string_cstring_0041e870(&a.temporary, ")", a.context->strings);
        append_suffix(a, 0x00b38d95);
        at(a, Step::temporary_release, 0x00b38dc7); release_current(a);
    }
    // Match each branch's getter order; reads after pooled callbacks are live.
    const char* suffix = a.suffix_data ? a.suffix_data : a.context->actual_empty_0108d6f2;
    const bool matrix = native_system_constant_second_dimension_00b5b840(record) > 1u;
    const bool array = native_system_constant_array_count_00b5b860(record) > 1u;
    const bool vector = !matrix && native_system_constant_first_dimension_00b5b850(record) > 1u;
    const char* name = data(native_shader_constant_name_00b5b820(&record));
    if (!name) name = a.context->actual_empty_0108d6f2;
    at(a, Step::declaration, matrix ? (array ? 0x00b38e3e : 0x00b38e84) :
        (array ? (vector ? 0x00b38eea : 0x00b38f2c) : (vector ? 0x00b38f71 : 0x00b38fa2)));
    // Evaluate native arguments explicitly in right-to-left push order.
    if (matrix) {
        const auto n = array ? native_system_constant_array_count_00b5b860(record) : 0u;
        const auto b = native_system_constant_second_dimension_00b5b840(record);
        const auto f = native_system_constant_first_dimension_00b5b850(record);
        if (array) line(owner.source_4c, a, "float%ix%i %s[%i]%s;", signed_word(f), signed_word(b), name, signed_word(n), suffix);
        else line(owner.source_4c, a, "float%ix%i %s%s;", signed_word(f), signed_word(b), name, suffix);
    } else if (array) {
        const auto n = native_system_constant_array_count_00b5b860(record);
        if (vector) {
            const auto f = native_system_constant_first_dimension_00b5b850(record);
            line(owner.source_4c, a, "float%i %s[%i]%s;", signed_word(f), name, signed_word(n), suffix);
        } else line(owner.source_4c, a, "float %s[%i]%s;", name, signed_word(n), suffix);
    } else if (vector) {
        const auto f = native_system_constant_first_dimension_00b5b850(record);
        line(owner.source_4c, a, "float%i %s%s;", signed_word(f), name, suffix);
    } else line(owner.source_4c, a, "float %s%s;", name, suffix);
    at(a, Step::suffix_release, 0x00b38fc1);
    if (a.suffix_data) a.context->strings.release(a.suffix_data, word(&a.suffix) + 1u);
    a.suffix_live = false;
}
} // namespace

NativeShaderConstantHeaderOperation::~NativeShaderConstantHeaderOperation() {
    if (phase == Phase::running || phase == Phase::failed) std::terminate();
}
void NativeShaderConstantHeaderOperation::acknowledge_diagnostic_cleanup() noexcept {
    if (phase == Phase::failed) phase = Phase::diagnostic_retired;
}
std::uint32_t native_system_constant_second_dimension_00b5b840(const NativeCompiledShaderConstantStorage& r) noexcept { return word(&r, 8); }
std::uint32_t native_system_constant_first_dimension_00b5b850(const NativeCompiledShaderConstantStorage& r) noexcept { return word(&r, 0xc); }
std::uint32_t native_system_constant_array_count_00b5b860(const NativeCompiledShaderConstantStorage& r) noexcept { return word(&r, 0x10); }
NativeCompiledShaderConstants& native_system_constant_list_00b5b890(void* owner) noexcept {
    return *reinterpret_cast<NativeCompiledShaderConstants*>(static_cast<char*>(owner) + 4);
}
NativeString& construct_native_shader_decimal_005f1840(NativeString& out, std::int32_t value,
    NativeShaderConstantHeaderContext& context, Frame& a) {
    begin(a, context, 0x005f1840); a.destination = &out;
    try { decimal(out, value, a); a.phase = Frame::Phase::complete; return out; }
    catch (...) { a.phase = Frame::Phase::failed; throw; }
}
void append_native_shader_line_00b35110(NativeString& out, NativeShaderConstantHeaderContext& context,
    Frame& a, const char* format, ...) {
    begin(a, context, 0x00b35110);
    std::va_list args; va_start(args, format);
    try { vline(out, a, format, args); a.phase = Frame::Phase::complete; }
    catch (...) { va_end(args); a.phase = Frame::Phase::failed; throw; }
    va_end(args);
}
void append_native_system_constant_00b38c60(NativeMaterialProgramBuilderStorage& owner,
    const NativeCompiledShaderConstantStorage& record, std::int32_t reg,
    NativeShaderConstantHeaderContext& context, Frame& a) {
    begin(a, context, 0x00b38c60);
    try { declaration(owner, record, reg, a); a.completed_rows = 1; a.phase = Frame::Phase::complete; }
    catch (...) { a.phase = Frame::Phase::failed; throw; }
}
void append_native_system_constant_header_00b38ff0(NativeMaterialProgramBuilderStorage& owner,
    std::uint8_t flag, NativeShaderConstantHeaderContext& context, Frame& a) {
    begin(a, context, 0x00b38ff0); a.builder = &owner;
    try {
        at(a, Step::registry, 0x00b38ffe);
        a.list = &native_system_constant_list_00b5b890(context.actual_registry_0108fe94);
        std::uint32_t end = word(a.list, 4) * 0x20u;
        std::uintptr_t current = word(a.list);
        end += static_cast<std::uint32_t>(current);
        while (current != end) {
            const auto index = a.cursor < context.actual_register_limit_00e13078 && flag ? a.cursor : 0xffffffffu;
            const auto& record = *reinterpret_cast<const NativeCompiledShaderConstantStorage*>(current);
            declaration(owner, record, signed_word(index), a);
            at(a, Step::cursor, 0x00b39039);
            const auto second = native_system_constant_second_dimension_00b5b840(record);
            const auto count = native_system_constant_array_count_00b5b860(record);
            end = word(a.list, 4) * 0x20u;
            end += word(a.list);
            current += 0x20u; a.cursor += second * count; ++a.completed_rows;
        }
        a.phase = Frame::Phase::complete;
    } catch (...) { a.phase = Frame::Phase::failed; throw; }
}
} // namespace bsp
