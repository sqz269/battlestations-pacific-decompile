#include "bsp/system_lighting_constants.hpp"
#include <cstring>

namespace bsp {
SystemSceneLighting* get_system_scene_lighting_00b72110(const SystemLightingScene& scene) {
    return scene.lighting_1c();
}
const SystemLightingWords4& get_system_ambient_00b7aa20(const SystemLightEnvironment& owner) {
    return owner.ambient_18;
}
const SystemLightingWords4& get_system_mode3_ambient_00b7aa30(const SystemLightEnvironment& owner) {
    return owner.ambient_mode3_28;
}
const SystemLightingWords4& get_system_ambient_cube_00b7aa40(
    const SystemLightEnvironment& owner, std::uint32_t index) {
    return owner.ambient_cube_38[index];
}
SystemShadowMapOwner* get_system_shadow_owner_00b7aab0(const SystemDirectionalLight& light) {
    return light.shadow_174;
}

namespace {
void raw_word(float* output, const std::uint32_t& input) {
    std::memcpy(output, &input, sizeof(input));
}
template<std::size_t N>
void raw_sequential(float* output, const std::array<std::uint32_t, N>& input) {
    for (std::size_t i = 0; i < N; ++i) raw_word(output + i, input[i]);
}
void raw_shadow_transpose(float* output, const SystemLightingMatrixWords& input) {
    // Native inline MOVSS, never the x87 camera matrix helper: sNaNs stay raw.
    for (std::size_t row = 0; row < 4; ++row)
        for (std::size_t column = 0; column < 4; ++column)
            raw_word(output + row * 4 + column, input[column * 4 + row]);
}
void store_unsigned_dimension(float* output, std::uint32_t value) {
    //00B4754C..65 /00B47577..90: signed FILD, optional float2^32 correction,
    // then one float spill. Preserve the caller's x87 rounding/precision mode.
    const float correction_00ce3978 = 4294967296.0f;
    __asm {
        mov eax, value
        test eax, eax
        fild dword ptr value
        jge nonnegative
        fadd dword ptr correction_00ce3978
    nonnegative:
        mov eax, output
        fstp dword ptr [eax]
    }
}
void store_dimension_reciprocals(float* dimensions) {
    //00B47590..B4: width and height have already been spilled and both virtual
    //chains completed. This also preserves width-store before height-divide.
    __asm {
        mov eax, dimensions
        fld dword ptr [eax]
        fld1
        fld st(0)
        fdivrp st(2), st(0)
        fxch st(1)
        fstp dword ptr [eax + 8]
        fdiv dword ptr [eax + 4]
        fstp dword ptr [eax + 12]
    }
}
SystemLightingPrefixStatus invalid(std::string& error, const char* message) {
    error = message;
    return SystemLightingPrefixStatus::invalid_projection;
}
}

SystemLightingPrefixStatus write_system_lighting_shadow_prefix_00b46ed1(
    const SystemLightingScene* scene, const std::uint32_t& camera_mode_198,
    const std::uint32_t& exponent_word_00cfad80, float* prefix,
    std::size_t word_count, std::string& error,
    SystemInvalidParameterRuntime* invalid_parameter_runtime) {
    if (!prefix || word_count < 77 * 4) {
        error = "System lighting requires an initialized 77-register prefix";
        return SystemLightingPrefixStatus::output_too_short;
    }
    if (!scene) return SystemLightingPrefixStatus::scene_absent;
    const auto* lighting = get_system_scene_lighting_00b72110(*scene);
    if (!lighting) return SystemLightingPrefixStatus::lighting_absent;
    auto& list = lighting->light_list();
    const auto* sentinel = list.sentinel_1c();
    if (!sentinel) return invalid(error, "Scene lighting sentinel+1C is unbound");
    const auto* first = list.next_00(sentinel);
    if (first == sentinel) {
        if (!invalid_parameter_runtime) {
            error = "Empty first-light list requires the actual invalid-parameter runtime";
            return SystemLightingPrefixStatus::invalid_parameter_runtime_unbound;
        }
        if (!invalid_parameter_runtime->invalid_parameter_noinfo_00bf6713(error))
            return SystemLightingPrefixStatus::callback_failed;
        //00B46EF8 continues with ESI lighting and EBX first/sentinel preserved.
        // Appending a node in a returning handler does not select the new node.
    }
    if (!first) return invalid(error, "Scene lighting first node is unbound");
    const bool mode3 = camera_mode_198 == 3;
    const auto* environment = lighting->environment_10();
    const auto* light = list.light_08(first); // EBX retained through shadow owner lookup
    if (!environment || !light)
        return invalid(error, "Scene environment+10 or first light+08 is unbound");

    raw_sequential(prefix + 43 * 4, mode3
        ? get_system_mode3_ambient_00b7aa30(*environment)
        : get_system_ambient_00b7aa20(*environment));
    raw_sequential(prefix + 44 * 4, mode3 ? light->diffuse_mode3_1b4 : light->diffuse_184);
    raw_sequential(prefix + 45 * 4, light->specular_194);
    raw_sequential(prefix + 52 * 4, light->direction_1e0);
    raw_word(prefix + 42 * 4, exponent_word_00cfad80);
    for (std::uint32_t i = 0; i < 6; ++i)
        raw_sequential(prefix + (46 + i) * 4, get_system_ambient_cube_00b7aa40(*environment, i));

    auto* shadow = get_system_shadow_owner_00b7aab0(*light);
    if (!shadow) return SystemLightingPrefixStatus::shadow_absent;
    for (std::size_t i = 0; i < 4; ++i)
        raw_shadow_transpose(prefix + (53 + i * 4) * 4, shadow->matrices_144[i]);
    // Native captures each complete group before its first destination write.
    const SystemLightingWords3 direction = shadow->direction_38;
    raw_sequential(prefix + 69 * 4, direction);
    const SystemLightingWords4 limits = shadow->limits_390;
    raw_sequential(prefix + 70 * 4, limits);

    SystemShadowTexture* texture = nullptr;
    if (!shadow->texture_08(texture, error)) return SystemLightingPrefixStatus::callback_failed;
    if (!texture) return invalid(error, "Shadow virtual+08 returned null before width virtual+3C");
    std::uint32_t width;
    if (!texture->width_3c(width, error)) return SystemLightingPrefixStatus::callback_failed;
    store_unsigned_dimension(prefix + 76 * 4, width);
    texture = nullptr;
    if (!shadow->texture_08(texture, error)) return SystemLightingPrefixStatus::callback_failed;
    if (!texture) return invalid(error, "Shadow virtual+08 returned null before height virtual+40");
    std::uint32_t height;
    if (!texture->height_40(height, error)) return SystemLightingPrefixStatus::callback_failed;
    store_unsigned_dimension(prefix + 76 * 4 + 1, height);
    store_dimension_reciprocals(prefix + 76 * 4);
    return SystemLightingPrefixStatus::complete;
}
}
