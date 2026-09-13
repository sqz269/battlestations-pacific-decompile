#include "bsp/native_shader_vertex_source.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using U = std::uint32_t;
using Op = NativeShaderVertexSourceOperation;
static_assert(sizeof(void*) == 4);
#include "shader_vertex_literals.inc"
U word(const void* p, U offset = 0) noexcept {
    return *reinterpret_cast<const volatile U*>(static_cast<const char*>(p) + offset);
}
void put(void* p, U offset, U value) noexcept {
    *reinterpret_cast<volatile U*>(static_cast<char*>(p) + offset) = value;
}
void* pointer(const void* p, U offset = 0) noexcept { return reinterpret_cast<void*>(word(p, offset)); }
std::uint8_t byte(const void* p, U offset) noexcept {
    return *reinterpret_cast<const volatile std::uint8_t*>(static_cast<const char*>(p) + offset);
}
void child(Op& a, U site) noexcept { a.step = Op::Step::child; a.native_site = site; }
void cstring_line(Op& a, const char* text, U site) {
    child(a, site); a.string_child = std::make_unique<NativeShaderStructDeclarationOperation>();
    append_native_shader_cstring_line_00b34f20(a.builder->source_4c, text, a.context->lines, *a.string_child);
}
void string_line(Op& a, const NativeString& text, U site) {
    child(a, site); a.string_child = std::make_unique<NativeShaderStructDeclarationOperation>();
    append_native_shader_string_line_00b35030(a.builder->source_4c, text, a.context->lines, *a.string_child);
}
void temporary(Op& a, const char* literal, U length, U resize_site, U copy_site) {
    put(&a.temporary, 0, 0); put(&a.temporary, 4, 0);
    a.temporary_live = true; a.temporary_captured = false;
    a.step = Op::Step::temporary_resize; a.native_site = resize_site;
    resize_native_string_header_0041dd40(&a.temporary, a.context->lines.strings, length, true);
    a.captured_data = static_cast<char*>(pointer(&a.temporary, 4));
    a.captured_length = word(&a.temporary); a.temporary_captured = true;
    a.step = Op::Step::temporary_copy; a.native_site = copy_site;
    if (a.captured_data) std::memmove(a.captured_data, literal, a.captured_length + 1u);
}
void release_temporary(Op& a, U site, bool captured_length = false) {
    a.step = Op::Step::temporary_release; a.native_site = site;
    if (a.captured_data) a.context->lines.strings.release(a.captured_data,
        (captured_length ? a.captured_length : word(&a.temporary)) + 1u);
    a.temporary_live = false; // Native header remains the stale preimage.
}
void literal_line(Op& a, const char* literal, U length, U resize_site, U copy_site,
    U line_site, U release_site, bool captured_length = false) {
    temporary(a, literal, length, resize_site, copy_site);
    string_line(a, a.temporary, line_site); release_temporary(a, release_site, captured_length);
}
void structure(Op& a, const char* name, U length, const NativeShaderDescriptorArray& fields,
    std::uint8_t semantics, U resize_site, U copy_site, U struct_site, U release_site) {
    temporary(a, name, length, resize_site, copy_site);
    child(a, struct_site); a.string_child = std::make_unique<NativeShaderStructDeclarationOperation>();
    append_native_shader_struct_00b38b50(*a.builder, 0, a.temporary, fields,
        semantics, 0, a.context->lines, *a.string_child);
    release_temporary(a, release_site);
}
void zero_fields(Op& a, const char* name, const NativeShaderDescriptorArray& fields,
    U resize_site, U copy_site, U zero_site, U release_site) {
    temporary(a, name, 3, resize_site, copy_site);
    child(a, zero_site); a.field_child = std::make_unique<NativeShaderFieldInitializationOperation>();
    append_native_shader_zero_fields_00b357d0(*a.builder, a.temporary, fields, *a.context, *a.field_child);
    release_temporary(a, release_site);
}
template<class T> bool pending(const std::unique_ptr<T>& p) noexcept {
    return p && (p->phase == T::Phase::running || p->phase == T::Phase::failed);
}
} // namespace
NativeShaderVertexSourceOperation::~NativeShaderVertexSourceOperation() {
    if (phase == Phase::running || phase == Phase::failed || temporary_live) std::terminate();
}
void NativeShaderVertexSourceOperation::acknowledge_diagnostic_cleanup() noexcept {
    if (phase == Phase::running || temporary_live || pending(string_child) || pending(constant_child) ||
        pending(interpolator_child) || pending(sampler_child) || pending(field_child)) std::terminate();
    phase = Phase::diagnostic_retired;
}
void generate_native_shader_vertex_source_00b39110(NativeMaterialProgramBuilderStorage& b,
    NativeShaderFieldInitializationContext& c, Op& a) {
    if (a.phase != Op::Phase::fresh) throw std::logic_error("vertex source operation is one-shot");
    a.builder = &b; a.context = &c; a.phase = Op::Phase::running;
    try {
        // EBX stays zero across the full body. A zero length does not clear
        // a non-null data pointer. Publish data=0 then length=0 AFTER release.
        a.captured_length = word(&b.source_4c);
        if (a.captured_length) {
            a.captured_data = static_cast<char*>(pointer(&b.source_4c, 4));
            a.step = Op::Step::source_release; a.native_site = 0xb39155;
            if (a.captured_data) c.lines.strings.release(a.captured_data, a.captured_length + 1u);
            put(&b.source_4c, 4, 0); put(&b.source_4c, 0, 0);
        }
        a.step = Op::Step::mode; a.mode = word(pointer(&b, 0x74), 0x10c);
        const char* define = nullptr;
        switch (a.mode) {
        case 0: define = "#define RM_NORMAL 1"; break;
        case 3: define = "#define RM_UNDERWATER 1"; break;
        case 7: define = "#define RM_MAP 1"; break;
        case 6: define = "#define RM_DRAW_SHADOW 1"; break;
        case 1: define = "#define RM_REFLECTION 1"; break;
        case 4: define = "#define RM_REFRACTION 1"; break;
        default: break;
        }
        if (define) cstring_line(a, define, 0xb391af);
        child(a, 0xb391b8); a.constant_child = std::make_unique<NativeShaderConstantHeaderOperation>();
        append_native_system_constant_header_00b38ff0(b, 1, c.lines, *a.constant_child);
        // Native pushes the mode header first, then the base header.
        a.step = Op::Step::headers;
        a.mode_header = static_cast<const char*>(pointer(pointer(&b, 0x74), 0xec));
        if (!a.mode_header) a.mode_header = c.lines.actual_empty_0108d6f2;
        a.base_header = static_cast<const char*>(pointer(pointer(&b, 0x70), 0xec));
        if (!a.base_header) a.base_header = c.lines.actual_empty_0108d6f2;
        child(a, 0xb391ef); a.constant_child = std::make_unique<NativeShaderConstantHeaderOperation>();
        append_native_shader_line_00b35110(b.source_4c, c.lines, *a.constant_child,
            "\n%s\n%s", a.base_header, a.mode_header);
        structure(a, "sVertexIn", 9, b.fields_04, 1, 0xb39207, 0xb39222, 0xb3923d, 0xb39260);
        structure(a, "sSysValues", 10, b.fields_10, 0, 0xb39275, 0xb39290, 0xb392ae, 0xb392d1);
        structure(a, "sVertexOut", 10, b.fields_1c, 0, 0xb392e6, 0xb39301, 0xb3931f, 0xb39342);
        child(a, 0xb3934e); a.interpolator_child = std::make_unique<NativeShaderInterpolatorSourceOperation>();
        append_native_shader_interpolator_struct_00b36e30(b, 1, 1, 0, c.lines, *a.interpolator_child);
        child(a, 0xb39355); a.sampler_child = std::make_unique<NativeShaderSamplerDeclarationsOperation>();
        append_native_vertex_sampler_declarations_00b38080(b, c.lines, *a.sampler_child);
        if (byte(pointer(&b, 0x74), 0x15)) cstring_line(a, vertex_shadow_helper, 0xb39369);
        cstring_line(a, vertex_ambient_fog_helpers, 0xb39375);
        literal_line(a, "\nvoid ShaderCode(sVertexIn IN, inout sSysValues SYS, inout sVertexOut OUT)",
            74, 0xb3938a, 0xb393a5, 0xb393bc, 0xb393df);
        literal_line(a, "{", 1, 0xb393f4, 0xb3940f, 0xb39426, 0xb39449);
        string_line(a, *reinterpret_cast<const NativeString*>(static_cast<const char*>(pointer(&b, 0x70)) + 0xf0), 0xb3945a);
        literal_line(a, "}", 1, 0xb3946f, 0xb3948a, 0xb394a1, 0xb394c4);
        literal_line(a, "\nvoid EffectCode(inout sSysValues SYS, inout sVertexOut OUT)",
            60, 0xb394d9, 0xb394f4, 0xb3950b, 0xb3952e);
        literal_line(a, "{", 1, 0xb39543, 0xb3955e, 0xb39575, 0xb39598);
        string_line(a, *reinterpret_cast<const NativeString*>(static_cast<const char*>(pointer(&b, 0x74)) + 0xf0), 0xb395a9);
        literal_line(a, "}", 1, 0xb395be, 0xb395d9, 0xb395f0, 0xb39613);
        child(a, 0xb3961a); a.interpolator_child = std::make_unique<NativeShaderInterpolatorSourceOperation>();
        append_native_shader_interpolator_pack_00b35540(b, c.lines, *a.interpolator_child);
        literal_line(a, "\nsInterpolators main(sVertexIn IN)", 34, 0xb3962f, 0xb3964a, 0xb39661, 0xb39684);
        literal_line(a, "{", 1, 0xb39699, 0xb396b4, 0xb396cb, 0xb396ee);
        cstring_line(a, "\t\tsSysValues\t\tSYS;\n\t\tsVertexOut\t\tOUT;\n", 0xb396fa);
        zero_fields(a, "SYS", b.fields_10, 0xb3970f, 0xb3972a, 0xb39745, 0xb39768);
        zero_fields(a, "OUT", b.fields_1c, 0xb3977d, 0xb39798, 0xb397b3, 0xb397d6);
        child(a, 0xb397dd); a.field_child = std::make_unique<NativeShaderFieldInitializationOperation>();
        append_native_shader_vertex_decode_00b35820(b, c, *a.field_child);
        cstring_line(a, "\n\t\tShaderCode(IN,SYS,OUT);\n\t\tEffectCode(SYS,OUT);\n\t\t\n\t\tOUT.ScreenSpacePos = SYS.ScreenSpacePos;\n", 0xb397e9);
        cstring_line(a, "\t\treturn PackInterpolators(OUT);", 0xb397f5);
        // Only this outer temporary preserves its pre-child LENGTH as well
        // as its data pointer for release (ESI=data, EBP=length).
        literal_line(a, "}", 1, 0xb3980a, 0xb39825, 0xb3983c, 0xb3985b, true);
        a.phase = Op::Phase::complete;
    } catch (...) { a.phase = Op::Phase::failed; throw; }
}
} // namespace bsp
