#include "bsp/material_scalar_constants.hpp"
#include "bsp/instance_grouping.hpp"
#include "bsp/material_entry_dispatch.hpp"
#include "bsp/material_lighting.hpp"
#include <cstring>

namespace bsp {
namespace {
bool destination(std::vector<float>& words, std::uint8_t reg,
    float*& output, const char* stage, std::string& error) {
    const std::size_t offset = static_cast<std::size_t>(reg) * 4;
    if (offset + 4 > words.size()) {
        error = std::string(stage) + " requires its actual shared float4 storage";
        return false;
    }
    output = words.data() + offset;
    return true;
}
// Keep each native DWORD read before its corresponding write, including when
// source and destination alias. No x87 load may quiet a signaling NaN here.
void copy_word(float* output, const float* input) noexcept {
    std::uint32_t bits;
    std::memcpy(&bits, input, sizeof(bits));
    std::memcpy(output, &bits, sizeof(bits));
}
void zero_padding(float* output) noexcept {
    constexpr std::uint32_t positive_zero = 0;
    for (std::size_t i = 1; i != 4; ++i)
        std::memcpy(output + i, &positive_zero, sizeof(positive_zero));
}
bool visibility(std::vector<float>& words, std::uint8_t reg,
    const InstanceRenderEntry& entry, const char* stage, std::string& error) {
    if (reg == 0xff) return true;
    float* output;
    if (!destination(words, reg, output, stage, error)) return false;
    copy_word(output, &entry.visibility);
    zero_padding(output);
    return true;
}
bool diffuse(std::vector<float>& words, std::uint8_t reg,
    const MaterialLighting* lighting, const char* stage, std::string& error) {
    if (reg == 0xff) return true;
    float* output;
    if (!destination(words, reg, output, stage, error)) return false;
    if (!lighting) {
        error = "Selected diffuse constant requires the actual section material lighting";
        return false;
    }
    const float* input = lighting->diffuse_color_00b179f0(0);
    for (std::size_t i = 0; i != 4; ++i) copy_word(output + i, input + i);
    return true;
}
}

bool write_material_visibility_lod_00b42e4a(const CompiledMaterialPass& pass,
    const InstanceRenderEntry& entry, const float* actual_stream_threshold,
    MaterialEntryConstantState& state, std::string& error) {
    error.clear();
    if (!visibility(state.vertex_words, pass.vb.registers[43], entry,
        "VS visibility", error)) return false;
    if (!visibility(state.pixel_words, pass.pb.registers[43], entry,
        "PS visibility", error)) return false;
    const auto reg = pass.pb.registers[44];
    if (reg == 0xff) return true;
    float* output;
    if (!destination(state.pixel_words, reg, output, "PS cLODValue", error))
        return false;
    if (!actual_stream_threshold) {
        error = "Selected cLODValue requires the actual stream threshold";
        return false;
    }
    const float* leading = &entry.leading_value;
    const float fraction = 0.01f; // Native00D7A238 bits3C23D70A.
    float leading_spill, result_spill;
    // Original input FLD/FSTP pairs and returned ST0 spill/reload are visible
    // here. The existing helper retains its x87 arithmetic schedule. Passing
    // the resolved threshold as a third cdecl argument replaces native ECX's
    // object/index lookup, without introducing a floating-point threshold copy.
    __asm {
        mov eax, leading
        fld dword ptr [eax]
        fstp leading_spill
        mov eax, actual_stream_threshold
        push dword ptr [eax]
        sub esp, 8
        fld fraction
        fstp dword ptr [esp + 4]
        fld leading_spill
        fstp dword ptr [esp]
        call compute_stream_threshold_fade_00b73770
        add esp, 12
        fstp result_spill
        fld result_spill
        mov eax, output
        fstp dword ptr [eax]
    }
    zero_padding(output);
    return true;
}

bool write_material_diffuse_00b4305d(const CompiledMaterialPass& pass,
    const MaterialLighting* actual_lighting, MaterialEntryConstantState& state,
    std::string& error) {
    error.clear();
    if (!diffuse(state.pixel_words, pass.pb.registers[47], actual_lighting,
        "PS diffuse", error)) return false;
    return diffuse(state.vertex_words, pass.vb.registers[47], actual_lighting,
        "VS diffuse", error);
}
}
