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
// A light-list identity is the actual canonical registry link, not a second
// list of projection nodes. Each accessor reads at the native point; the caller
// retains the first identity across a returning invalid-parameter handler.
class SystemLightListAccess {
public:
    virtual ~SystemLightListAccess() = default;
    virtual const void* sentinel_1c() = 0;
    virtual const void* next_00(const void* node) = 0;
    virtual SystemDirectionalLight* light_08(const void* node) = 0;
};
class SystemDirectionalLightResolver {
public:
    virtual ~SystemDirectionalLightResolver() = default;
    // Zero is an actual null key. An unbound nonzero key is an explicit host
    // binding error, including a sentinel payload after a returning handler.
    virtual SystemDirectionalLight* resolve_light(std::uint32_t actual_key) = 0;
};
class SystemSceneLighting {
public:
    virtual ~SystemSceneLighting() = default;
    virtual SystemLightEnvironment* environment_10() const = 0;
    virtual SystemLightListAccess& light_list() const = 0;
};
class SystemLightingScene {
public:
    virtual ~SystemLightingScene() = default;
    virtual SystemSceneLighting* lighting_1c() const = 0;
};

// Borrowed diagnostic companions retain references to actual supplied slots.
// Native owner implementations supply the accessors above directly from their
// own canonical registry and retained owner slots.
struct SystemLightListNode {
    SystemLightListNode* const& next_00;
    SystemDirectionalLight* const& light_08;
};
class BorrowedSystemLightListAccess final : public SystemLightListAccess {
public:
    explicit BorrowedSystemLightListAccess(SystemLightListNode* const& sentinel)
        : sentinel_(sentinel) {}
    const void* sentinel_1c() override { return sentinel_; }
    const void* next_00(const void* node) override {
        return static_cast<const SystemLightListNode*>(node)->next_00;
    }
    SystemDirectionalLight* light_08(const void* node) override {
        return static_cast<const SystemLightListNode*>(node)->light_08;
    }
private:
    SystemLightListNode* const& sentinel_;
};
class BorrowedSystemSceneLighting final : public SystemSceneLighting {
public:
    BorrowedSystemSceneLighting(SystemLightEnvironment* const& environment,
        SystemLightListAccess& list) : environment_(environment), list_(list) {}
    SystemLightEnvironment* environment_10() const override { return environment_; }
    SystemLightListAccess& light_list() const override { return list_; }
private:
    SystemLightEnvironment* const& environment_;
    SystemLightListAccess& list_;
};
class BorrowedSystemLightingScene final : public SystemLightingScene {
public:
    explicit BorrowedSystemLightingScene(SystemSceneLighting* const& lighting)
        : lighting_(lighting) {}
    SystemSceneLighting* lighting_1c() const override { return lighting_; }
private:
    SystemSceneLighting* const& lighting_;
};

// Native ECX owner, EAX borrowed pointer, RET (cube getter stack index, RET4).
// C++ projections have a new ABI. All owners/fields must be actually bound.
SystemSceneLighting* get_system_scene_lighting_00b72110(const SystemLightingScene&);
const SystemLightingWords4& get_system_ambient_00b7aa20(const SystemLightEnvironment&);
const SystemLightingWords4& get_system_mode3_ambient_00b7aa30(const SystemLightEnvironment&);
const SystemLightingWords4& get_system_ambient_cube_00b7aa40(
    const SystemLightEnvironment&, std::uint32_t index); // caller ensures index<6
SystemShadowMapOwner* get_system_shadow_owner_00b7aab0(const SystemDirectionalLight&);

// Actual runtime adapter for00BF6713 ->00BF66EF(0,0,0,0,0). The decoded
// installed handler may return. A successful return continues from the native
// call site using the retained lighting and first/sentinel pointers. False is
// an unavailable host runtime/binding, never the native empty-list behavior.
class SystemInvalidParameterRuntime {
public:
    virtual ~SystemInvalidParameterRuntime() = default;
    virtual bool invalid_parameter_noinfo_00bf6713(std::string&) = 0;
};

enum class SystemLightingPrefixStatus {
    complete,
    scene_absent,
    lighting_absent,
    shadow_absent,
    invalid_parameter_runtime_unbound,
    invalid_projection,
    callback_failed,
    output_too_short
};

// Interior00B46ED1..00B475B3, not a callable native function. Parent00B46A70
// receives ECX scene, EDX camera, no stack arguments, plain RET.
// Prefix is the caller's initialized77-register float buffer; no clear/resize.
// Pass the actual camera+198 mode and native00CFAD80 word references. The
// latter is installed128.0f. Required shadow callbacks execute at native points;
// callback failure keeps earlier writes. Empty first==sentinel calls the actual
// supplied invalid-parameter runtime; a returned handler continues with that
// captured sentinel, without re-reading scene, sentinel slot or next pointer.
// Camera mode, environment and sentinel+8 are read AFTER the handler returns.
// No texture dimension fallback, null-texture skip, zero-size guard or upload.
SystemLightingPrefixStatus write_system_lighting_shadow_prefix_00b46ed1(
    const SystemLightingScene* scene, const std::uint32_t& camera_mode_198,
    const std::uint32_t& exponent_word_00cfad80, float* prefix,
    std::size_t word_count, std::string& error,
    SystemInvalidParameterRuntime* invalid_parameter_runtime = nullptr);
}
