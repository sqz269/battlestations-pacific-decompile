#include "bsp/native_shader_interpolator_source.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using U = std::uint32_t;
using Op = NativeShaderInterpolatorSourceOperation;
static_assert(sizeof(void*) == 4);
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
std::int32_t signed_word(U value) noexcept {
    std::int32_t result; std::memcpy(&result, &value, sizeof(result)); return result;
}
void begin(Op& a, NativeMaterialProgramBuilderStorage& b, NativeShaderConstantHeaderContext& c, U fn) {
    if (a.phase != Op::Phase::fresh) throw std::logic_error("interpolator source operation is one-shot");
    a.builder = &b; a.context = &c; a.function = fn; a.phase = Op::Phase::running;
}
void cstring_line(Op& a, const char* text, U site) {
    a.native_site = site;
    a.line_child = std::make_unique<NativeShaderStructDeclarationOperation>();
    append_native_shader_cstring_line_00b34f20(a.builder->source_4c, text, *a.context, *a.line_child);
}
template<class... Args> void formatted(Op& a, U site, const char* format, Args... args) {
    a.native_site = site;
    a.format_child = std::make_unique<NativeShaderConstantHeaderOperation>();
    append_native_shader_line_00b35110(a.builder->source_4c, *a.context, *a.format_child, format, args...);
}
void literal_line(Op& a, const char* literal, U length, bool captured_release_length,
    U resize_site, U append_site, U release_site) {
    put(&a.temporary, 0, 0); put(&a.temporary, 4, 0);
    a.temporary_live = true; a.temporary_captured = false; a.native_site = resize_site;
    resize_native_string_header_0041dd40(&a.temporary, a.context->strings, length, true);
    a.captured_data = static_cast<char*>(pointer(&a.temporary, 4));
    a.captured_length = word(&a.temporary); a.temporary_captured = true;
    if (a.captured_data) std::memmove(a.captured_data, literal, a.captured_length + 1u);
    a.native_site = append_site;
    a.line_child = std::make_unique<NativeShaderStructDeclarationOperation>();
    append_native_shader_string_line_00b35030(a.builder->source_4c, a.temporary, *a.context, *a.line_child);
    a.native_site = release_site;
    if (a.captured_data)
        a.context->strings.release(a.captured_data, (captured_release_length ? a.captured_length : word(&a.temporary)) + 1u);
    a.temporary_live = false;
}
const char* name(const void* fields, U index, const NativeShaderConstantHeaderContext& c) {
    const auto* slots = pointer(fields);
    const auto* field = pointer(slots, index * 4u);
    const auto* text = static_cast<const char*>(pointer(field, 4));
    return text ? text : c.actual_empty_0108d6f2;
}
void mappings(Op& a, bool unpack, bool color) {
    a.mapping_offset = color ? 0x60u : 0x54u; a.row = 0;
    static constexpr char channels[] = "xyzw";
    while (a.row < word(a.builder, a.mapping_offset + 4u)) {
        const auto* mapping = static_cast<const char*>(pointer(a.builder, a.mapping_offset)) + a.row * 2u;
        // Every row rereads mapping data and actual list data; fields/mappings
        // may change during the preceding pooled formatting operation.
        a.field_index = byte(mapping); a.component = byte(mapping, 1);
        a.field_name = name(a.fields, a.field_index, *a.context);
        const int field_channel = static_cast<signed char>(channels[a.component]);
        const int packed_channel = static_cast<signed char>(channels[a.row & 3u]);
        const U packed_register = a.row >> 2u;
        if (unpack)
            formatted(a, color ? 0xb371c5 : 0xb3715c,
                color ? "\t\tPixelIn.%s.%c = INT.Color%i.%c;" : "\t\tPixelIn.%s.%c = INT.TexCoord%i.%c;",
                a.field_name, field_channel, packed_register, packed_channel);
        else
            formatted(a, color ? 0xb3570e : 0xb356a2,
                color ? "\t\tINT.Color%i.%c = OUT.%s.%c;" : "\t\tINT.TexCoord%i.%c = OUT.%s.%c;",
                packed_register, packed_channel, a.field_name, field_channel);
        ++a.row; ++a.completed_rows;
    }
}
bool needs_vpos(const NativeMaterialProgramBuilderStorage& b) {
    return byte(pointer(&b, 0x70), 0x30) != 0 || byte(pointer(&b, 0x74), 0x30) != 0;
}
void groups(Op& a, bool color) {
    const U count = word(a.builder, color ? 0x64 : 0x58);
    // Native CDQ/AND3/ADD/SAR2 is signed truncation after DWORD(count+3).
    a.group_count = static_cast<U>(signed_word(count + 3u) / 4);
    a.last_width = static_cast<U>(signed_word(count) % 4);
    if (!a.last_width) a.last_width = 4;
    const U group_offset = color ? 0x84u : 0x7cu;
    put(a.builder, group_offset, a.group_count);
    put(a.builder, group_offset + 4u, a.last_width);
    a.row = 0;
    while (a.row < word(a.builder, group_offset)) {
        U width = 4;
        if (a.row == word(a.builder, group_offset) - 1u) width = word(a.builder, group_offset + 4u);
        formatted(a, color ? 0xb36f84 : 0xb36f1e,
            color ? "\tfloat%i Color%i\t: COLOR%i;" : "\tfloat%i TexCoord%i\t: TEXCOORD%i;",
            width, a.row, a.row);
        ++a.row; ++a.completed_rows;
    }
}
} // namespace
NativeShaderInterpolatorSourceOperation::~NativeShaderInterpolatorSourceOperation() {
    if (phase == Phase::running || phase == Phase::failed || temporary_live) std::terminate();
}
void NativeShaderInterpolatorSourceOperation::acknowledge_diagnostic_cleanup() noexcept {
    if (phase == Phase::running || temporary_live) std::terminate();
    if (line_child && (line_child->phase == NativeShaderStructDeclarationOperation::Phase::running ||
        line_child->phase == NativeShaderStructDeclarationOperation::Phase::failed)) std::terminate();
    if (format_child && (format_child->phase == NativeShaderConstantHeaderOperation::Phase::running ||
        format_child->phase == NativeShaderConstantHeaderOperation::Phase::failed)) std::terminate();
    phase = Phase::diagnostic_retired;
}
void append_native_shader_interpolator_pack_00b35540(NativeMaterialProgramBuilderStorage& b,
    NativeShaderConstantHeaderContext& c, Op& a) {
    begin(a, b, c, 0xb35540); a.fields = &b.fields_28;
    try {
        literal_line(a, "\nsInterpolators PackInterpolators(sVertexOut OUT)", 49, false, 0xb35570, 0xb355a1, 0xb355c4);
        literal_line(a, "{", 1, true, 0xb355d9, 0xb3560b, 0xb3562a);
        cstring_line(a, "\t\tsInterpolators INT;\n", 0xb35636);
        cstring_line(a, "\t\tINT.Position = OUT.ScreenSpacePos;", 0xb35642);
        mappings(a, false, false); mappings(a, false, true);
        const auto fog = byte(&b, 0x6c);
        if (fog != 0xff) formatted(a, 0xb35741, "\t\tINT.Fog = OUT.%s;", name(a.fields, fog, c));
        cstring_line(a, "\t\treturn INT;", 0xb35750);
        literal_line(a, "}", 1, true, 0xb35767, 0xb35799, 0xb357b8);
        a.phase = Op::Phase::complete;
    } catch (...) { a.phase = Op::Phase::failed; throw; }
}
void append_native_shader_interpolator_struct_00b36e30(NativeMaterialProgramBuilderStorage& b,
    std::uint8_t position, std::uint8_t fog, std::uint8_t vpos, NativeShaderConstantHeaderContext& c, Op& a) {
    begin(a, b, c, 0xb36e30); a.position = position; a.fog = fog; a.vpos = vpos;
    try {
        literal_line(a, "\nstruct sInterpolators\n{", 24, true, 0xb36e60, 0xb36e91, 0xb36eb0);
        if (position) cstring_line(a, "\tfloat4 Position\t: POSITION0;", 0xb36ec3);
        groups(a, false); groups(a, true);
        if (byte(&b, 0x6c) != 0xff && fog) cstring_line(a, "\tfloat  Fog\t: FOG;", 0xb36fab);
        if (vpos && needs_vpos(b)) cstring_line(a, "\tfloat2  vPos\t: VPOS;", 0xb36fd0);
        cstring_line(a, "};\n", 0xb36fdc);
        a.phase = Op::Phase::complete;
    } catch (...) { a.phase = Op::Phase::failed; throw; }
}
void append_native_shader_interpolator_unpack_00b37000(NativeMaterialProgramBuilderStorage& b,
    const NativeShaderDescriptorArray& fields, NativeShaderConstantHeaderContext& c, Op& a) {
    begin(a, b, c, 0xb37000); a.fields = &fields;
    try {
        literal_line(a, "\nsPixelIn UnpackInterpolators(sInterpolators INT)", 49, false, 0xb3702f, 0xb37061, 0xb37084);
        literal_line(a, "{", 1, false, 0xb37099, 0xb370cb, 0xb370ee);
        cstring_line(a, "\t\tsPixelIn PixelIn;\n", 0xb370fa);
        mappings(a, true, false); mappings(a, true, true);
        const auto fog = byte(&b, 0x6c);
        if (fog != 0xff) {
            const bool zero = byte(&b, 0x98) != 0;
            formatted(a, 0xb37219, zero ? "\t\tPixelIn.%s = 0;" : "\t\tPixelIn.%s = INT.Fog;", name(a.fields, fog, c));
        }
        if (needs_vpos(b)) formatted(a, 0xb37239, "\t\tPixelIn.vPos = INT.vPos;");
        cstring_line(a, "\t\treturn PixelIn;", 0xb37248);
        literal_line(a, "}", 1, true, 0xb3725f, 0xb37291, 0xb372b1);
        a.phase = Op::Phase::complete;
    } catch (...) { a.phase = Op::Phase::failed; throw; }
}
} // namespace bsp
