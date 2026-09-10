#include "bsp/lighting_configuration_apply.hpp"
#include "bsp/system_lighting_owners.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Lighting configuration requires MSVC Win32 x87 instructions and spills.
#endif

namespace bsp {
namespace {
std::uint32_t read_word(const std::uint32_t& word) noexcept {
    return static_cast<const volatile std::uint32_t&>(word);
}
void write_word(std::uint32_t& word, std::uint32_t value) noexcept {
    static_cast<volatile std::uint32_t&>(word) = value;
}
void copy_words(std::uint32_t* output, const std::uint32_t* input, unsigned count) {
    // Native load/store pairs, including overlapping input and output.
    for (unsigned i = 0; i < count; ++i) write_word(output[i], read_word(input[i]));
}
bool guard_clear(const std::uint32_t& guard) noexcept {
    return (*reinterpret_cast<const volatile std::uint8_t*>(&guard) & 0x1u) == 0;
}
void set_guard(std::uint32_t& guard) noexcept {
    write_word(guard, read_word(guard) | 0x1u);
}
std::uint32_t x87_spill(const std::uint32_t& input) {
    const auto* source = &input;
    std::uint32_t result;
    __asm {
        mov eax, source
        fld dword ptr [eax]
        fstp dword ptr result
    }
    return result;
}
std::uint32_t cube_scale_and_one(const std::uint32_t& input,
    const std::uint32_t& one, std::uint32_t& captured_one) {
    const auto* source = &input;
    const auto* constant = &one;
    auto* one_output = &captured_one;
    std::uint32_t result;
    __asm {
        mov eax, source
        fld dword ptr [eax]
        mov eax, constant
        mov edx, [eax]
        mov eax, one_output
        mov [eax], edx
        fstp dword ptr result
    }
    return result;
}

void multiply_four(std::uint32_t* output, const std::uint32_t* input,
    std::uint32_t scale, const std::uint32_t* fourth_operand) {
    std::uint32_t products[4];
    // Same x87 stack ordering and per-channel float32 spills as the three
    // scaler leaves. Alpha multiplies scale * source; RGB starts source * scale.
    // The alpha product remains live while output x/y are stored.
    __asm {
        mov esi, input
        mov edi, output
        fld dword ptr [esi]
        fld dword ptr scale
        fld st(0)
        fmulp st(2), st(0)
        fxch st(1)
        fstp dword ptr products[0]
        fld dword ptr [esi + 4]
        fmul st(0), st(1)
        fstp dword ptr products[4]
        mov edx, products[4]
        fld dword ptr [esi + 8]
        fmul st(0), st(1)
        fstp dword ptr products[8]
        mov eax, fourth_operand
        fmul dword ptr [eax]
        mov eax, products[0]
        mov [edi], eax
        mov eax, products[8]
        mov [edi + 4], edx
        fstp dword ptr products[12]
        mov edx, products[12]
        mov [edi + 8], eax
        mov [edi + 12], edx
    }
}

std::uint32_t ensure_half_color(LightingConfigurationGlobals globals,
    bool preload_half, bool capture_one) {
    const bool initialize = guard_clear(globals.half_guard_00e18b18);
    // The cube loop loads CE3800 on every iteration, even with guard bit set.
    if (preload_half || initialize) {
        const auto half = read_word(globals.half_word_00ce3800);
        if (initialize) {
            set_guard(globals.half_guard_00e18b18);
            write_word(globals.half_color_00e18b08[0], half);
            write_word(globals.half_color_00e18b08[1], half);
            write_word(globals.half_color_00e18b08[2], half);
            const auto one = read_word(globals.one_word_00d7a24c);
            write_word(globals.half_color_00e18b08[3], one);
            return one;
        }
    }
    return capture_one ? read_word(globals.one_word_00d7a24c) : 0u;
}
} // namespace

LightingAmbientFields lighting_ambient_fields(ConcreteSystemAmbientLight& owner) noexcept {
    return {owner.ambient_18, owner.ambient_mode3_28, owner.ambient_cube_38};
}
void set_lighting_ambient_00b7af20(
    LightingAmbientFields owner, const SystemLightingWords4& value) {
    copy_words(owner.ambient_18.data(), value.data(), 4);
}
void set_lighting_mode3_ambient_00b7af40(
    LightingAmbientFields owner, const SystemLightingWords4& value) {
    copy_words(owner.ambient_mode3_28.data(), value.data(), 4);
}
void set_lighting_ambient_cube_00b7af60(
    LightingAmbientFields owner, const SystemLightingWords4& value, std::uint32_t index) {
    copy_words(owner.ambient_cube_38[index].data(), value.data(), 4);
}
void set_lighting_base_diffuse_004b62e0(
    LightingDirectionalFields owner, const SystemLightingWords4& value) {
    const auto* scale_source = &owner.diffuse_scale_1d8;
    const auto* input = value.data();
    std::uint32_t scale;
    std::uint32_t first_word;
    __asm {
        mov eax, scale_source
        fld dword ptr [eax]
        mov eax, input
        mov edx, [eax]
        mov first_word, edx
        fstp dword ptr scale
    }
    write_word(owner.base_diffuse_1a4[0], first_word);
    copy_words(owner.base_diffuse_1a4.data() + 1, input + 1, 3);
    // Reload original input after all base stores, as native does.
    multiply_four(owner.diffuse_184.data(), input, scale, input + 3);
}
void set_lighting_diffuse_scale_00b7af90(
    LightingDirectionalFields owner, std::uint32_t scale) {
    write_word(owner.diffuse_scale_1d8, scale); // native MOVSS preserves raw sNaN
    multiply_four(owner.diffuse_184.data(), owner.base_diffuse_1a4.data(), scale,
        owner.base_diffuse_1a4.data() + 3);
}
void set_lighting_specular_scale_00b7b010(
    LightingDirectionalFields owner, std::uint32_t scale) {
    write_word(owner.specular_scale_1dc, scale);
    multiply_four(owner.specular_194.data(), owner.base_specular_1c4.data(), scale,
        owner.base_specular_1c4.data() + 3);
}

SystemLightingWords3& lighting_direction_from_angles_004b4d80(
    SystemLightingWords3& output, std::uint32_t first, std::uint32_t second) {
    auto* destination = output.data();
    std::uint32_t scratch[7];
    __asm {
        mov eax, destination
        fld dword ptr first
        fcos
        fstp dword ptr scratch[0]
        fld dword ptr scratch[0]
        fstp dword ptr scratch[24]
        fld dword ptr second
        fcos
        fstp dword ptr scratch[4]
        fld dword ptr scratch[4]
        fstp dword ptr scratch[20]
        fld dword ptr first
        fsin
        fstp dword ptr scratch[8]
        movss xmm0, dword ptr scratch[8]
        fld dword ptr first
        fcos
        fstp dword ptr scratch[12]
        fld dword ptr scratch[12]
        fstp dword ptr scratch[16]
        fld dword ptr second
        fsin
        fstp dword ptr first
        fld dword ptr first
        movss dword ptr [eax + 4], xmm0
        fmul dword ptr scratch[16]
        fstp dword ptr [eax]
        fld dword ptr scratch[20]
        fmul dword ptr scratch[24]
        fstp dword ptr [eax + 8]
    }
    return output;
}

void apply_lighting_configuration_004bacd0(LightingAmbientFields ambient,
    LightingDirectionalFields light, const LightingConfigurationFields* config,
    LightingConfigurationGlobals globals) {
    if (!config) {
        for (std::uint32_t face = 0; face < 6; ++face) {
            (void)ensure_half_color(globals, true, false);
            set_lighting_ambient_cube_00b7af60(ambient, globals.half_color_00e18b08, face);
        }
        (void)ensure_half_color(globals, false, false);
        set_lighting_ambient_00b7af20(ambient, globals.half_color_00e18b08);
        (void)ensure_half_color(globals, false, false);
        set_lighting_mode3_ambient_00b7af40(ambient, globals.half_color_00e18b08);
        const auto one = ensure_half_color(globals, false, true);
        const bool initialize_diffuse = guard_clear(globals.diffuse_guard_00e18b04);
        copy_words(light.diffuse_mode3_1b4.data(), globals.half_color_00e18b08.data(), 4);
        if (initialize_diffuse) {
            set_guard(globals.diffuse_guard_00e18b04);
            for (auto& word : globals.diffuse_color_00e18af4) write_word(word, one);
        }
        set_lighting_base_diffuse_004b62e0(light, globals.diffuse_color_00e18af4);
        const bool initialize_specular = guard_clear(globals.specular_guard_00e18af0);
        copy_words(light.direction_1e0.data(), globals.direction_00f8758c.data(), 3);
        if (initialize_specular) {
            set_guard(globals.specular_guard_00e18af0);
            write_word(globals.specular_color_00e18ae0[0], 0);
            write_word(globals.specular_color_00e18ae0[1], 0);
            write_word(globals.specular_color_00e18ae0[2], 0);
            write_word(globals.specular_color_00e18ae0[3], read_word(globals.one_word_00d7a24c));
        }
        copy_words(light.base_specular_1c4.data(), globals.specular_color_00e18ae0.data(), 4);
        return; // No write to effective specular or its scale.
    }

    SystemLightingWords4 value;
    if (static_cast<const volatile std::uint8_t&>(globals.scale_ambient_00f88a0c) != 0) {
        const auto multiplier = read_word(config->ambient_f48[3]); // native MOVSS
        multiply_four(value.data(), config->ambient_f48.data(), multiplier, &multiplier);
    } else {
        copy_words(value.data(), config->ambient_f48.data(), 4);
    }
    // The discarded fourth product above must execute before this overwrite.
    value[3] = read_word(globals.one_word_00d7a24c);
    set_lighting_ambient_00b7af20(ambient, value);
    set_lighting_mode3_ambient_00b7af40(ambient, config->ambient_mode3_f58);
    for (std::uint32_t face = 0; face < 6; ++face) {
        const auto& input = config->ambient_cube_f68[face];
        std::uint32_t one;
        const auto multiplier = cube_scale_and_one(input[3], globals.one_word_00d7a24c, one);
        multiply_four(value.data(), input.data(), multiplier, input.data() + 3);
        value[3] = one;
        set_lighting_ambient_cube_00b7af60(ambient, value, face);
    }
    set_lighting_base_diffuse_004b62e0(light, config->base_diffuse_fc8);
    copy_words(light.diffuse_mode3_1b4.data(), config->diffuse_mode3_fd8.data(), 4);
    const auto second = x87_spill(config->second_angle_1004);
    const auto first = x87_spill(config->first_angle_1008);
    SystemLightingWords3 direction;
    lighting_direction_from_angles_004b4d80(direction, first, second);
    for (unsigned i = 0; i < 3; ++i) write_word(light.direction_1e0[i], x87_spill(direction[i]));
    copy_words(light.base_specular_1c4.data(), config->base_specular_fe8.data(), 4);
    set_lighting_diffuse_scale_00b7af90(light, x87_spill(config->diffuse_scale_ffc));
    set_lighting_specular_scale_00b7b010(light, x87_spill(config->specular_scale_1000));
}
} // namespace bsp
