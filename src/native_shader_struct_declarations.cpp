#include "bsp/native_shader_struct_declarations.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_string_append.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using Frame = NativeShaderStructDeclarationOperation;
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
void begin(Frame& a, NativeShaderConstantHeaderContext& context, std::uint32_t fn) {
    if (a.phase != Frame::Phase::fresh) throw std::logic_error("shader struct operation is one-shot");
    a.context = &context; a.function = fn; a.phase = Frame::Phase::running;
}
void append_captured(NativeString& out, Frame& a, std::uint32_t site, Step step) {
    if (!a.captured_length) return;
    a.old_length = word(&out);
    at(a, step, site);
    resize_native_string_header_0041dd40(&out, a.context->strings, a.old_length + a.captured_length, true);
    std::memmove(pointer<char>(&out) + a.old_length, a.captured_data, a.captured_length);
}
void append_current(NativeString& out, const NativeString& from, Frame& a,
    std::uint32_t site, Step step = Step::text_append) {
    a.source = &from; a.append_length = word(&from);
    if (!a.append_length) return;
    a.old_length = word(&out);
    at(a, step, site);
    resize_native_string_header_0041dd40(&out, a.context->strings, a.old_length + a.append_length, true);
    // Native reloads source+04 after resize; the two headers may alias.
    const auto* source = pointer<char>(&from);
    auto* destination = pointer<char>(&out);
    std::memmove(destination + a.old_length, source, a.append_length);
}
void construct_temporary(Frame& a, const char* text, std::uint32_t site) {
    a.cstring = text; a.temporary_entered = true; a.temporary_returned = false; a.temporary_live = true;
    at(a, Step::text_construct, site);
    construct_native_string_cstring_0041e870(&a.temporary, text, a.context->strings);
    a.temporary_returned = true;
}
void release_temporary(Frame& a, std::uint32_t site) noexcept {
    at(a, Step::text_release, site);
    destroy_native_string_header_0041dd20(&a.temporary, a.context->strings);
    a.temporary_live = false;
}
void punctuation(NativeString& out, const char* text, Frame& a,
    std::uint32_t resize_site, std::uint32_t append_site, std::uint32_t release_site) {
    clear(a.punctuation); a.punctuation_live = true; a.punctuation_captured = false;
    at(a, Step::punctuation_resize, resize_site);
    resize_native_string_header_0041dd40(&a.punctuation, a.context->strings, 1, true);
    a.captured_data = pointer<char>(&a.punctuation); a.captured_length = word(&a.punctuation);
    a.punctuation_captured = true;
    if (a.captured_data) std::memmove(a.captured_data, text, a.captured_length + 1u);
    append_captured(out, a, append_site, Step::punctuation_append);
    at(a, Step::punctuation_release, release_site);
    if (a.captured_data) a.context->strings.release(a.captured_data, a.captured_length + 1u);
    a.punctuation_live = false;
}
void cstring_line(NativeString& out, const char* text, Frame& a) {
    a.destination = &out;
    construct_temporary(a, text, 0x00b34f47);
    a.captured_length = word(&a.temporary); a.captured_data = pointer<char>(&a.temporary);
    append_captured(out, a, 0x00b34f68, Step::text_append);
    at(a, Step::text_release, 0x00b34f99);
    if (a.captured_data) a.context->strings.release(a.captured_data, a.captured_length + 1u);
    a.temporary_live = false;
    punctuation(out, "\n", a, 0x00b34fae, 0x00b34fe7, 0x00b35016);
}
void field_declaration(const NativeShaderFieldStorage& field, NativeString& out, std::uint8_t semantics, Frame& a) {
    a.field = &field; a.destination = &out; a.semantics = semantics;
    clear(out);
    const auto scalar = word(&field, 8);
    if (scalar < 2u) {
        at(a, Step::type_assign, 0x00b38607);
        assign_native_string_cstring_0041e350(&out, scalar ? "int" : "float", a.context->strings);
    }
    const bool wide = word(&field, 0xc) > 1u;
    construct_temporary(a, "\t\t", wide ? 0x00b3861e : 0x00b386ea);
    if (wide) {
        a.number_entered = true; a.number_returned = false; a.number_live = true;
        at(a, Step::width_number, 0x00b38631);
        construct_native_material_program_number_00711370(a.number, word(&field, 0xc), a.context->strings);
        a.number_returned = true;
        a.joined_entered = true; a.joined_returned = false; a.joined_live = true;
        at(a, Step::width_join, 0x00b38643);
        concatenate_native_string_headers_004261a0(&a.number, &a.joined, &a.temporary, a.context->strings);
        a.joined_returned = true;
        append_current(out, a.joined, a, 0x00b38660);
        at(a, Step::text_release, 0x00b3869b);
        destroy_native_string_header_0041dd20(&a.joined, a.context->strings); a.joined_live = false;
        at(a, Step::text_release, 0x00b386bf);
        destroy_native_string_header_0041dd20(&a.number, a.context->strings); a.number_live = false;
        release_temporary(a, 0x00b386e3);
    } else {
        append_current(out, a.temporary, a, 0x00b38709);
        release_temporary(a, 0x00b38740);
    }
    append_current(out, field.name_00, a, 0x00b38759, Step::field_name);
    if (semantics) {
        construct_temporary(a, "\t\t : ", 0x00b38785);
        append_current(out, a.temporary, a, 0x00b387a4);
        release_temporary(a, 0x00b387db);
        static constexpr const char* tokens[]{"POSITION", "COLOR", "TEXCOORD", "NORMAL", "BINORMAL", "TANGENT", "BLENDINDICES", "BLENDWEIGHT", "FOG", "INDEX", "VPOS"};
        static constexpr std::uint32_t sites[]{0x00b38800, 0x00b38820, 0x00b3883b, 0x00b3885b, 0x00b3887b, 0x00b38896, 0x00b388b6, 0x00b388d3, 0x00b388eb, 0x00b38908, 0x00b38925};
        const auto semantic = word(&field, 0x14);
        if (semantic < 11u) {
            construct_temporary(a, tokens[semantic], sites[semantic]);
            at(a, Step::semantic_append, 0x00b38939);
            append_native_string_00425e10(out, a.temporary, a.context->strings);
            release_temporary(a, 0x00b38947);
        }
        a.temporary_entered = true; a.temporary_returned = false; a.temporary_live = true;
        at(a, Step::semantic_number, 0x00b38954);
        construct_native_material_program_number_00711370(a.temporary, word(&field, 0x18), a.context->strings);
        a.temporary_returned = true;
        append_current(out, a.temporary, a, 0x00b38973);
        release_temporary(a, 0x00b389aa);
    }
    punctuation(out, ";", a, 0x00b389c1, 0x00b389fa, 0x00b38a26);
}
template<class... Args> void formatted(NativeString& out, Frame& a, std::uint32_t site, const char* format, Args... args) {
    a.destination = &out;
    at(a, Step::formatted_line, site);
    a.formatted_line = std::make_unique<NativeShaderConstantHeaderOperation>();
    append_native_shader_line_00b35110(out, *a.context, *a.formatted_line, format, args...);
}
} // namespace
NativeShaderStructDeclarationOperation::~NativeShaderStructDeclarationOperation() {
    if (phase == Phase::running || phase == Phase::failed) std::terminate();
}
void NativeShaderStructDeclarationOperation::acknowledge_diagnostic_cleanup() noexcept {
    if (phase == Phase::failed) {
        if (formatted_line) formatted_line->acknowledge_diagnostic_cleanup();
        phase = Phase::diagnostic_retired;
    }
}
void append_native_shader_cstring_line_00b34f20(NativeString& out, const char* text,
    NativeShaderConstantHeaderContext& context, Frame& a) {
    begin(a, context, 0x00b34f20); a.destination = &out;
    try { cstring_line(out, text, a); a.phase = Frame::Phase::complete; }
    catch (...) { a.phase = Frame::Phase::failed; throw; }
}
void append_native_shader_string_line_00b35030(NativeString& out, const NativeString& source,
    NativeShaderConstantHeaderContext& context, Frame& a) {
    begin(a, context, 0x00b35030); a.destination = &out;
    try {
        append_current(out, source, a, 0x00b35060);
        punctuation(out, "\n", a, 0x00b3508a, 0x00b350c3, 0x00b350f2);
        a.phase = Frame::Phase::complete;
    } catch (...) { a.phase = Frame::Phase::failed; throw; }
}
NativeString& format_native_shader_field_00b385b0(const NativeShaderFieldStorage& field,
    NativeString& out, std::uint8_t semantics, NativeShaderConstantHeaderContext& context, Frame& a) {
    begin(a, context, 0x00b385b0);
    try { field_declaration(field, out, semantics, a); a.phase = Frame::Phase::complete; return out; }
    catch (...) { a.phase = Frame::Phase::failed; throw; }
}
void append_native_shader_struct_00b38b50(NativeMaterialProgramBuilderStorage& owner, std::uint32_t start,
    const NativeString& name, const NativeShaderDescriptorArray& fields, std::uint8_t semantics,
    std::uint8_t allow_vpos, NativeShaderConstantHeaderContext& context, Frame& a) {
    begin(a, context, 0x00b38b50); a.builder = &owner; a.fields = &fields;
    a.destination = &owner.source_4c; a.row = start; a.semantics = semantics; a.allow_vpos = allow_vpos;
    try {
        const auto* name_data = pointer<char>(&name);
        formatted(owner.source_4c, a, 0x00b38b8c, "\nstruct %s\n{", name_data ? name_data : context.actual_empty_0108d6f2);
        while (a.row < word(&fields, 4)) {
            const auto* slots = pointer<NativeShaderFieldStorage*>(&fields, 0);
            const auto* field = *reinterpret_cast<NativeShaderFieldStorage* const volatile*>(slots + a.row);
            at(a, Step::field_format, 0x00b38bb2); a.field_output_live = true;
            field_declaration(*field, a.field_output, semantics, a);
            const auto* text = pointer<char>(&a.field_output);
            formatted(owner.source_4c, a, 0x00b38bd2, "\t%s", text ? text : context.actual_empty_0108d6f2);
            at(a, Step::field_release, 0x00b38bfc);
            destroy_native_string_header_0041dd20(&a.field_output, context.strings); a.field_output_live = false;
            ++a.row; ++a.completed_rows;
        }
        at(a, Step::vpos, 0x00b38c16);
        if (allow_vpos &&
            (*reinterpret_cast<const volatile std::uint8_t*>(pointer<char>(&owner, 0x70) + 0x30) ||
             *reinterpret_cast<const volatile std::uint8_t*>(pointer<char>(&owner, 0x74) + 0x30)))
            formatted(owner.source_4c, a, 0x00b38c2e, "\tfloat2 vPos;");
        cstring_line(owner.source_4c, "};\n", a);
        a.destination = &owner.source_4c; a.phase = Frame::Phase::complete;
    } catch (...) { a.phase = Frame::Phase::failed; throw; }
}
} // namespace bsp
