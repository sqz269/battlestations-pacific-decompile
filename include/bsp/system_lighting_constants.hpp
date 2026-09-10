#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

namespace bsp {
using SystemLightingWords3 = std::array<std::uint32_t, 3>;
using SystemLightingWords4 = std::array<std::uint32_t, 4>;
using SystemLightingMatrixWords = std::array<std::uint32_t, 16>;

// Required adapters for the ACTUAL object returned by shadow virtual+08.
// Results are native unsigned DWORDs; LogicalTexture nominal metadata is not
// this contract. Each returned owner must survive its immediately following call.
class SystemShadowTexture {
public:
    virtual ~SystemShadowTexture() = default;
    virtual bool width_3c(std::uint32_t&, std::string&) = 0;
    virtual bool height_40(std::uint32_t&, std::string&) = 0;
};

// Borrowed field references, not native layout, allocation or retention logic.
// Keep this companion and its actual owner alive through the entire fragment.
class SystemShadowMapOwner {
public:
    SystemShadowMapOwner(const std::array<SystemLightingMatrixWords, 4>& matrices,
        const SystemLightingWords3& direction, const SystemLightingWords4& limits)
        : matrices_144(matrices), direction_38(direction), limits_390(limits) {}
    virtual ~SystemShadowMapOwner() = default;
    virtual bool texture_08(SystemShadowTexture*&, std::string&) = 0;
    const std::array<SystemLightingMatrixWords, 4>& matrices_144; // +144,+184,+1C4,+204
    const SystemLightingWords3& direction_38;
    const SystemLightingWords4& limits_390;
};

struct SystemDirectionalLight {
    SystemShadowMapOwner* const& shadow_174;
    const SystemLightingWords4& diffuse_184;
    const SystemLightingWords4& specular_194;
    const SystemLightingWords4& diffuse_mode3_1b4;
    const SystemLightingWords3& direction_1e0;
};
struct SystemLightEnvironment {
    const SystemLightingWords4& ambient_18;
    const SystemLightingWords4& ambient_mode3_28;
    const std::array<SystemLightingWords4, 6>& ambient_cube_38;
};
// Reference the live pointer slots. The sentinel is a real node, and first==
// sentinel has different semantics from an absent scene/lighting owner.
struct SystemLightListNode {
    SystemLightListNode* const& next_00;
    SystemDirectionalLight* const& light_08;
};
struct SystemSceneLighting {
    SystemLightEnvironment* const& environment_10;
    SystemLightListNode* const& sentinel_1c;
};
struct SystemLightingScene {
    SystemSceneLighting* const& lighting_1c;
};

// Native ECX owner, EAX borrowed pointer, RET (cube getter stack index, RET4).
// C++ projections have a new ABI. All owners/fields must be actually bound.
SystemSceneLighting* get_system_scene_lighting_00b72110(const SystemLightingScene&);
const SystemLightingWords4& get_system_ambient_00b7aa20(const SystemLightEnvironment&);
const SystemLightingWords4& get_system_mode3_ambient_00b7aa30(const SystemLightEnvironment&);
const SystemLightingWords4& get_system_ambient_cube_00b7aa40(
    const SystemLightEnvironment&, std::uint32_t index); // caller ensures index<6
SystemShadowMapOwner* get_system_shadow_owner_00b7aab0(const SystemDirectionalLight&);

enum class SystemLightingPrefixStatus {
    complete,
    scene_absent,
    lighting_absent,
    shadow_absent,
    native_empty_light_list_failure,
    invalid_projection,
    callback_failed,
    output_too_short
};

// Interior00B46ED1..00B475B3, not a callable native function. Parent00B46A70
// receives ECX scene, EDX camera, no stack arguments, plain RET.
// Prefix is the caller's initialized77-register float buffer; no clear/resize.
// Pass the actual camera+198 mode and native00CFAD80 word references. The
// latter is installed128.0f. Required shadow callbacks execute at native points;
// callback failure keeps earlier writes. Empty list explicitly stops this host
// boundary at00BF6713: the native CRT handler/possible return is unresolved.
// No texture dimension fallback, null-texture skip, zero-size guard or upload.
SystemLightingPrefixStatus write_system_lighting_shadow_prefix_00b46ed1(
    const SystemLightingScene* scene, const std::uint32_t& camera_mode_198,
    const std::uint32_t& exponent_word_00cfad80, float* prefix,
    std::size_t word_count, std::string& error);
}
