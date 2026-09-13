#include "bsp/native_shader_pixel_source.hpp"
#include "bsp/native_physical_file_date.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using U = std::uint32_t;
using Op = NativeShaderPixelSourceOperation;
static_assert(sizeof(void*) == 4);
#include "shader_pixel_literals.inc"
static constexpr char l00d60ff8[] = "#define RM_NORMAL 1";
static constexpr char l00d60fe0[] = "#define RM_UNDERWATER 1";
static constexpr char l00d60fcc[] = "#define RM_MAP 1";
static constexpr char l00d60fb0[] = "#define RM_DRAW_SHADOW 1";
static constexpr char l00d60f98[] = "#define RM_REFLECTION 1";
static constexpr char l00d60f80[] = "#define RM_REFRACTION 1";
static constexpr char l00d60de8[] = "\n%s\n%s";
static constexpr char l00d61804[] = "sPixelIn";
static constexpr char l00d60f68[] = "sSysValues";
static constexpr char l00d617d0[] = "\nvoid ShaderCode(sPixelIn IN, inout sSysValues SYS)";
static constexpr char l00d617bc[] = "], out float Depth)";
static constexpr char l00d61770[] = "\nvoid EffectCode(sPixelIn IN, inout sSysValues SYS, out float4 FinalColor[";
static constexpr char l00d04260[] = "])";
static constexpr char l00d61718[] = "\nvoid main(sInterpolators INT, out float4 Color0 : COLOR0, out float Depth : DEPTH)";
static constexpr char l00d616fc[] = "{\n\nColor0=0; Depth=0;";
static constexpr char l00d616c0[] = "\nvoid main(sInterpolators INT, out float4 Color0 : COLOR0)";
static constexpr char l00d616b0[] = "{\n\nColor0=0;";
static constexpr char l00d61640[] = "\nvoid main(sInterpolators INT, out float4 Color0 : COLOR0, out float4 Color1 : COLOR1, out float Depth : DEPTH)";
static constexpr char l00d61620[] = "{\n\nColor0=0; Color1=0; Depth=0;";
static constexpr char l00d615c8[] = "\nvoid main(sInterpolators INT, out float4 Color0 : COLOR0, out float4 Color1 : COLOR1)";
static constexpr char l00d615ac[] = "{\n\nColor0=0; Color1=0;";
static constexpr char l00d61520[] = "\nvoid main(sInterpolators INT, out float4 Color0 : COLOR0, out float4 Color1 : COLOR1, out float4 Color2 : COLOR2, out float Depth : DEPTH)";
static constexpr char l00d614f4[] = "{\n\nColor0=0; Color1=0; Color2=0; Depth=0;";
static constexpr char l00d61480[] = "\nvoid main(sInterpolators INT, out float4 Color0 : COLOR0, out float4 Color1 : COLOR1, out float4 Color2 : COLOR2)";
static constexpr char l00d61458[] = "{\n\nColor0=0; Color1=0; Color2=0;";
static constexpr char l00d613b0[] = "\nvoid main(sInterpolators INT, out float4 Color0 : COLOR0, out float4 Color1 : COLOR1, out float4 Color2 : COLOR2, out float4 Color3 : COLOR3, out float Depth : DEPTH)";
static constexpr char l00d61378[] = "{\n\nColor0=0; Color1=0; Color2=0; Color3=0; Depth=0;";
static constexpr char l00d612e8[] = "\nvoid main(sInterpolators INT, out float4 Color0 : COLOR0, out float4 Color1 : COLOR1, out float4 Color2 : COLOR2, out float4 Color3 : COLOR3)";
static constexpr char l00d612bc[] = "{\n\nColor0=0; Color1=0; Color2=0; Color3=0;";
static constexpr char l00d612b8[] = "];";
static constexpr char l00d61260[] = "\t\tsPixelIn\t\tIN = UnpackInterpolators(INT);\n\t\tsSysValues\t\tSYS;\n\t\tfloat4\t\t\tFinalColors[";
static constexpr char l00d60e80[] = "SYS";
static constexpr char l00d61218[] = "\n\t\t\tShaderCode(IN,SYS);\n\t\t\tEffectCode(IN,SYS,FinalColors,Depth);";
static constexpr char l00d611dc[] = "\n\t\t\tShaderCode(IN,SYS);\n\t\t\tEffectCode(IN,SYS,FinalColors);";
static constexpr char l00d61188[] = "\nFinalColors[0].xyz = min(pow(FinalColors[0].xyz,0.25)*23,120*FinalColors[0].xyz);\n";
static constexpr char l00d61118[] = "\nif(cElapsedTime[1]>0.5)\nFinalColors[0].xyz = min(pow(FinalColors[0].xyz,0.25)*23,120*FinalColors[0].xyz);\n";
static constexpr char l00d610fc[] = "\nColor0=FinalColors[0];\n";
static constexpr char l00d610c8[] = "Color0.rgb=lerp(cFogDirColor,Color0.rgb,INT.Fog);\n";
static constexpr char l00d61090[] = "Color0.a=SYS.DiffuseColor.a * saturate(cVisibility);\n";
static constexpr char l00d61070[] = "Color0.a=SYS.DiffuseColor.a;\n";
static constexpr char l00d61058[] = "Color0.rgb *= Color0.a;";
static constexpr char l00d61040[] = "\nColor1=FinalColors[1];";
static constexpr char l00d61028[] = "\nColor2=FinalColors[2];";
static constexpr char l00d61010[] = "\nColor3=FinalColors[3];";
static constexpr char l00d6100c[] = "\n}";
U word(const void* p, U offset = 0) noexcept {
    return *reinterpret_cast<const volatile U*>(static_cast<const char*>(p) + offset);
}
void put(void* p, U offset, U value) noexcept {
    *reinterpret_cast<volatile U*>(static_cast<char*>(p) + offset) = value;
}
std::uint8_t byte(const void* p, U offset = 0) noexcept {
    return *reinterpret_cast<const volatile std::uint8_t*>(static_cast<const char*>(p) + offset);
}
void* pointer(const void* p, U offset = 0) noexcept { return reinterpret_cast<void*>(word(p, offset)); }
NativeShaderConstantHeaderContext& lines(Op& a) noexcept { return a.context->lines; }
void cstring_line(Op& a, const char* text, U site) {
    a.native_site = site; a.line = std::make_unique<NativeShaderStructDeclarationOperation>();
    append_native_shader_cstring_line_00b34f20(a.builder->source_4c, text, lines(a), *a.line);
}
void string_line(Op& a, const NativeString& text, U site) {
    a.native_site = site; a.line = std::make_unique<NativeShaderStructDeclarationOperation>();
    append_native_shader_string_line_00b35030(a.builder->source_4c, text, lines(a), *a.line);
}
// Preserve every raw header and the before/after helper ownership boundary.
Op::Temporary& enter(Op& a, U index, U site) {
    auto& t = a.temporary[index];
    t.entered = true; t.returned = false; t.live = false; t.captured = false;
    a.native_site = site; return t;
}
void literal(Op& a, U index, const char* text, U length, U resize_site) {
    auto& t = enter(a, index, resize_site);
    put(&t.header, 0, 0); put(&t.header, 4, 0); t.live = true;
    resize_native_string_header_0041dd40(&t.header, lines(a).strings, length, true);
    t.returned = true; t.captured_data = static_cast<char*>(pointer(&t.header, 4));
    t.captured_length = word(&t.header); t.captured = true;
    if (t.captured_data) std::memmove(t.captured_data, text, t.captured_length + 1u);
}
void construct(Op& a, U index, const char* text, U site) {
    auto& t = enter(a, index, site);
    construct_native_string_cstring_0041e870(&t.header, text, lines(a).strings);
    t.returned = true; t.live = true;
}
void number(Op& a, U site) {
    auto& t = enter(a, 2, site);
    a.sampled_count = word(a.builder, 0xa4);
    construct_native_material_program_number_00711370(t.header, a.sampled_count, lines(a).strings);
    t.returned = true; t.live = true;
}
void join(Op& a, U output, U left, U right, U site) {
    auto& t = enter(a, output, site);
    concatenate_native_string_headers_004261a0(&a.temporary[left].header, &t.header,
        &a.temporary[right].header, lines(a).strings);
    t.returned = true; t.live = true;
}
// 0=current header; 1=captured data/current length; 2=both captured.
void release(Op& a, U index, U capture, U site) {
    a.native_site = site; auto& t = a.temporary[index];
    auto* data = capture ? t.captured_data : static_cast<char*>(pointer(&t.header, 4));
    if (data) lines(a).strings.release(data, (capture == 2 ? t.captured_length : word(&t.header)) + 1u);
    t.live = false; // Native inline cleanup leaves the header words stale.
}
void literal_line(Op& a, const char* text, U length, U capture, U resize_site, U append_site, U release_site) {
    literal(a, 0, text, length, resize_site);
    string_line(a, a.temporary[0].header, append_site);
    release(a, 0, capture, release_site);
}
void declaration(Op& a, const char* name, U length, const NativeShaderDescriptorArray& fields,
    U start, std::uint8_t vpos, U resize_site, U append_site, U release_site) {
    literal(a, 0, name, length, resize_site);
    a.native_site = append_site; a.line = std::make_unique<NativeShaderStructDeclarationOperation>();
    append_native_shader_struct_00b38b50(*a.builder, start, a.temporary[0].header,
        fields, 0, vpos, lines(a), *a.line);
    release(a, 0, 2, release_site);
}
void effect_signature(Op& a) {
    a.sampled_depth = byte(a.builder, 0xa8);
    if (a.sampled_depth) {
        construct(a, 1, l00d617bc, 0xb39bf9); number(a, 0xb39c12);
        construct(a, 3, l00d61770, 0xb39c27);
        join(a, 4, 3, 2, 0xb39c39); join(a, 5, 4, 1, 0xb39c4f);
        string_line(a, a.temporary[5].header, 0xb39c5c);
        release(a, 5, 0, 0xb39c80); release(a, 4, 0, 0xb39ca4); release(a, 3, 0, 0xb39cc8);
    } else {
        construct(a, 1, l00d04260, 0xb39ceb); number(a, 0xb39d03);
        construct(a, 3, l00d61770, 0xb39d18);
        join(a, 4, 3, 2, 0xb39d2a); join(a, 5, 4, 1, 0xb39d41);
        string_line(a, a.temporary[5].header, 0xb39d4e);
        release(a, 5, 0, 0xb39d71); release(a, 4, 0, 0xb39d95); release(a, 3, 0, 0xb39db9);
    }
    release(a, 2, 0, 0xb39ddd); release(a, 1, 0, 0xb39e04);
}
struct MainLines { const char* signature; const char* initialization; U construct, append, release, second; };
void main_signature(Op& a) {
    a.sampled_count = word(a.builder, 0xa4);
    if (a.sampled_count < 1 || a.sampled_count > 4) return;
    a.sampled_depth = byte(a.builder, 0xa8);
    // One branch is retained across BOTH string constructions/appends.
    static constexpr MainLines choices[4][2] = {
        {{l00d616c0,l00d616b0,0xb39f72,0xb39f82,0xb39fa5,0xb39fb3},
         {l00d61718,l00d616fc,0xb39f1a,0xb39f2a,0xb39f4d,0xb39f5b}},
        {{l00d615c8,l00d615ac,0xb3a038,0xb3a048,0xb3a06b,0xb3a079},
         {l00d61640,l00d61620,0xb39fe0,0xb39ff0,0xb3a013,0xb3a021}},
        {{l00d61480,l00d61458,0xb3a12e,0xb3a13e,0xb3a161,0xb3a16f},
         {l00d61520,l00d614f4,0xb3a0a7,0xb3a0b7,0xb3a0da,0xb3a0e8}},
        {{l00d612e8,l00d612bc,0xb3a1db,0xb3a1eb,0xb3a1f8,0xb3a206},
         {l00d613b0,l00d61378,0xb3a19c,0xb3a1ac,0xb3a1b9,0xb3a1c7}}
    };
    const auto& selected = choices[a.sampled_count - 1u][a.sampled_depth ? 1 : 0];
    const bool four = a.sampled_count == 4;
    construct(a, 0, selected.signature, selected.construct);
    string_line(a, a.temporary[0].header, selected.append);
    if (four) {
        a.native_site = selected.release;
        destroy_native_string_header_0041dd20(&a.temporary[0].header, lines(a).strings);
        a.temporary[0].live = false;
    } else release(a, 0, 0, selected.release);
    construct(a, 0, selected.initialization, selected.second);
    string_line(a, a.temporary[0].header, four ? 0xb3a216 : 0xb3a0f8);
    if (four) {
        a.native_site = 0xb3a223;
        destroy_native_string_header_0041dd20(&a.temporary[0].header, lines(a).strings);
        a.temporary[0].live = false;
    } else release(a, 0, 0, 0xb3a11f);
}
void main_locals(Op& a) {
    literal(a, 1, l00d612b8, 2, 0xb3a23a); number(a, 0xb3a274);
    literal(a, 3, l00d61260, 85, 0xb3a292);
    join(a, 4, 3, 2, 0xb3a2c4); join(a, 5, 4, 1, 0xb3a2db);
    string_line(a, a.temporary[5].header, 0xb3a2e8);
    release(a, 5, 0, 0xb3a30b); release(a, 4, 0, 0xb3a32f);
    release(a, 3, 1, 0xb3a34f); // EBP data survives both joins and append.
    release(a, 2, 0, 0xb3a373); release(a, 1, 0, 0xb3a39a);
    literal(a, 0, l00d60e80, 3, 0xb3a3b1);
    a.native_site = 0xb3a3eb; a.initialization = std::make_unique<NativeShaderFieldInitializationOperation>();
    append_native_shader_zero_fields_00b357d0(*a.builder, a.temporary[0].header,
        a.builder->fields_34, *a.context, *a.initialization);
    release(a, 0, 2, 0xb3a40a);
}
void outputs(Op& a) {
    cstring_line(a, byte(a.builder, 0xa8) ? l00d61218 : l00d611dc, 0xb3a426);
    U mode = word(pointer(a.builder, 0x74), 0x10c);
    if (mode == 3 || mode == 9 || mode == 11) cstring_line(a, l00d61188, 0xb3a44a);
    mode = word(pointer(a.builder, 0x74), 0x10c); // Reread after prior append.
    if ((mode == 0 || mode == 12 || mode == 8 || mode == 10) && byte(pointer(a.builder, 0x70), 0x1d) == 0)
        cstring_line(a, l00d61118, 0xb3a47c);
    const bool fog = byte(a.builder, 0x6c) != 0xff && byte(a.builder, 0x98) == 0;
    a.sampled_alpha = byte(a.builder, 0xa9);
    if (fog) {
        cstring_line(a, l00d610fc, a.sampled_alpha ? 0xb3a49e : 0xb3a4c2);
        cstring_line(a, l00d610c8, a.sampled_alpha ? 0xb3a4aa : 0xb3a4ce);
        if (byte(pointer(a.builder, 0x74), 0x31))
            cstring_line(a, a.sampled_alpha ? l00d61090 : l00d61070, 0xb3a53a);
    } else {
        cstring_line(a, l00d610fc, a.sampled_alpha ? 0xb3a4f1 : 0xb3a512);
        if (byte(pointer(a.builder, 0x74), 0x31))
            cstring_line(a, a.sampled_alpha ? l00d61090 : l00d61070, a.sampled_alpha ? 0xb3a505 : 0xb3a526);
        if (byte(pointer(a.builder, 0x70), 0x32)) cstring_line(a, l00d61058, 0xb3a53a);
    }
    if (word(a.builder, 0xa4) >= 2) cstring_line(a, l00d61040, 0xb3a54f);
    if (word(a.builder, 0xa4) >= 3) cstring_line(a, l00d61028, 0xb3a564);
    if (word(a.builder, 0xa4) >= 4) cstring_line(a, l00d61010, 0xb3a579);
    literal_line(a, l00d6100c, 2, 2, 0xb3a58e, 0xb3a5c0, 0xb3a5e0);
}
template<class T> bool pending(const std::unique_ptr<T>& p) noexcept {
    return p && (p->phase == T::Phase::running || p->phase == T::Phase::failed);
}
} // namespace
NativeShaderPixelSourceOperation::~NativeShaderPixelSourceOperation() {
    if (phase == Phase::running || phase == Phase::failed) std::terminate();
    for (const auto& t : temporary) if (t.live) std::terminate();
}
void NativeShaderPixelSourceOperation::acknowledge_diagnostic_cleanup() noexcept {
    if (phase == Phase::running) std::terminate();
    for (const auto& t : temporary) if (t.live) std::terminate();
    if (pending(line) || pending(constants) || pending(interpolators) || pending(samplers) ||
        pending(shadow) || pending(initialization)) std::terminate();
    phase = Phase::diagnostic_retired;
}
void build_native_shader_pixel_source_00b39880(NativeMaterialProgramBuilderStorage& b,
    const NativeShaderDescriptorArray& declaration_fields, const NativeShaderDescriptorArray& unpack_fields,
    NativeShaderFieldInitializationContext& context, Op& a) {
    if (a.phase != Op::Phase::fresh) throw std::logic_error("pixel source operation is one-shot");
    a.builder = &b; a.declaration_fields = &declaration_fields; a.unpack_fields = &unpack_fields;
    a.context = &context; a.phase = Op::Phase::running;
    try {
        a.initial_length = word(&b.source_4c);
        if (a.initial_length) {
            a.initial_data = static_cast<char*>(pointer(&b.source_4c, 4));
            a.native_site = 0xb398c0;
            if (a.initial_data) lines(a).strings.release(a.initial_data, a.initial_length + 1u);
            put(&b.source_4c, 4, 0); put(&b.source_4c, 0, 0);
        }
        const char* define = nullptr;
        switch (word(pointer(&b, 0x74), 0x10c)) {
        case 0: define = l00d60ff8; break;
        case 3: define = l00d60fe0; break;
        case 7: define = l00d60fcc; break;
        case 6: define = l00d60fb0; break;
        case 1: define = l00d60f98; break;
        case 4: define = l00d60f80; break;
        default: break;
        }
        if (define) cstring_line(a, define, 0xb39922);
        a.native_site = 0xb3992b; a.constants = std::make_unique<NativeShaderConstantHeaderOperation>();
        append_native_system_constant_header_00b38ff0(b, 1, lines(a), *a.constants);
        const auto* mode_common = static_cast<const char*>(pointer(pointer(&b, 0x74), 0xec));
        if (!mode_common) mode_common = lines(a).actual_empty_0108d6f2;
        const auto* base_common = static_cast<const char*>(pointer(pointer(&b, 0x70), 0xec));
        if (!base_common) base_common = lines(a).actual_empty_0108d6f2;
        a.native_site = 0xb39963; a.constants = std::make_unique<NativeShaderConstantHeaderOperation>();
        append_native_shader_line_00b35110(b.source_4c, lines(a), *a.constants, l00d60de8, base_common, mode_common);
        const auto fog = static_cast<std::uint8_t>(byte(&b, 0x98) == 0);
        a.native_site = 0xb3997d; a.interpolators = std::make_unique<NativeShaderInterpolatorSourceOperation>();
        append_native_shader_interpolator_struct_00b36e30(b, 0, fog, 1, lines(a), *a.interpolators);
        declaration(a, l00d61804, 8, declaration_fields, 1, 1, 0xb39992, 0xb399cf, 0xb399ee);
        declaration(a, l00d60f68, 10, b.fields_34, 0, 0, 0xb39a05, 0xb39a41, 0xb39a60);
        a.native_site = 0xb39a67; a.samplers = std::make_unique<NativeShaderSamplerDeclarationsOperation>();
        append_native_pixel_sampler_declarations_00b37ef0(b, lines(a), *a.samplers);
        if (byte(pointer(&b, 0x74), 0x15)) {
            a.native_site = 0xb39a77; a.shadow = std::make_unique<NativeShaderShadowSourceOperation>();
            append_native_shadow_helper_00b38230(b, lines(a), *a.shadow);
            a.native_site = 0xb39a7e; a.shadow = std::make_unique<NativeShaderShadowSourceOperation>();
            append_native_map_shadow_helper_00b382b0(b, lines(a), *a.shadow);
        }
        cstring_line(a, pixel_ambient_fog_helpers, 0xb39a8a);
        cstring_line(a, pixel_srgb_helpers, 0xb39a96);
        literal_line(a, l00d617d0, 51, 2, 0xb39aad, 0xb39adf, 0xb39afe);
        literal_line(a, "{", 1, 2, 0xb39b15, 0xb39b47, 0xb39b66);
        string_line(a, *reinterpret_cast<const NativeString*>(static_cast<const char*>(pointer(&b, 0x70)) + 0xf8), 0xb39b76);
        literal_line(a, "}", 1, 2, 0xb39b8d, 0xb39bbf, 0xb39bde);
        effect_signature(a);
        literal_line(a, "{", 1, 1, 0xb39e1b, 0xb39e4d, 0xb39e6f);
        string_line(a, *reinterpret_cast<const NativeString*>(static_cast<const char*>(pointer(&b, 0x74)) + 0xf8), 0xb39e80);
        literal_line(a, "}", 1, 1, 0xb39e97, 0xb39ec9, 0xb39ee8);
        a.native_site = 0xb39ef4; a.interpolators = std::make_unique<NativeShaderInterpolatorSourceOperation>();
        append_native_shader_interpolator_unpack_00b37000(b, unpack_fields, lines(a), *a.interpolators);
        main_signature(a); main_locals(a); outputs(a);
        a.phase = Op::Phase::complete;
    } catch (...) { a.phase = Op::Phase::failed; throw; }
}
} // namespace bsp
