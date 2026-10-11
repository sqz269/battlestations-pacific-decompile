#pragma once

// The cloud entity's scene-property pass (00477710, cloud entity vtable 00CE5E68
// slot +A0h): the branch gate and the particle-cloud (CloudType 0) branch.
// Evidence: docs/CC13_SCENE_BROWSER.md ("Cloud property pass"). Names are
// hypotheses; the two mesh branches are documented, not reconstructed.

#include <cstdint>
#include <map>
#include <memory>
#include <string>

namespace bsp {

// One scene property as 00477710 reads it: kind at +4 (0 integer, 3 boolean
// byte, otherwise float), value at +0Ch; a colour property's +0Ch is a nested
// bag holding R, G, B (and A for xBaseCol).
struct CloudSceneProperty {
    std::int32_t kind{};
    std::int32_t integer{};
    float real{};
    std::uint8_t byte{};
    std::shared_ptr<const std::map<std::string, CloudSceneProperty>> nested;

    // CVTSI2SS for kind 0, else MOVSS (00477793..004777B4).
    float as_float() const noexcept { return kind == 0 ? static_cast<float>(integer) : real; }
};
using CloudScenePropertyBag = std::map<std::string, CloudSceneProperty>;

enum class CloudPropertyBranch { None, Particle, ShadedMesh, DiffuseMesh };

// 00477737..00477780 and 00477C3C / 0047889A: after the base 0048DBD0, the
// pass runs only when the class CloudType (+34h of the class at +188h) or the
// particle instance (+1A4h) is non-zero, the property holder (+C0h) is present
// with kind 1 and byte +1A8h is clear; CloudType 0, 1 and 2 pick the branch.
// Every path ends with the entity's vtable+D8h (00478A2A), not modelled.
CloudPropertyBranch cloud_property_branch_00477710(std::int32_t cloud_type, bool has_instance,
                                                    bool has_holder, std::int32_t holder_kind,
                                                    bool suppressed) noexcept;

// The particle instance at entity+1A4h and the cloud manager
// [[00E188A8]+19E8h]+3Ch (00BBDDF0).
struct CloudParticleTarget {
    virtual ~CloudParticleTarget() = default;
    virtual void set_inside_color(const float rgba[4]) = 0;  // 00BBE540
    virtual void set_ambient(const float rgba[4]) = 0;       // 00BBE4B0
    virtual void set_diffuse(const float rgba[4]) = 0;       // 00BBE4E0
    virtual void set_specular(const float rgba[4]) = 0;      // 00BBE510
    virtual void set_density(float) = 0;                     // 00BBE570
    virtual void set_blur_radius(float) = 0;                 // 00BBE590
    virtual void set_scale(float) = 0;                       // 00BBE470
    virtual void set_alpha(float) = 0;                       // 00BBE490
    virtual void set_affected_by_wind(std::uint8_t) = 0;     // 00BBE5C0, MOVZX byte
    virtual void set_visibility_factor(float) = 0;           // 00B6DA70(instance, factor, 0)
    virtual void register_with_manager() = 0;                // manager->00BBD470(instance)
};

// Address: 00477710, CloudType-0 branch 00477786..00477C37. Original ABI of
// the whole routine: __thiscall, ECX = cloud entity, RET. `bag` is the holder's
// bag (+C0h, then +8). InsideColor, Ambient, Diffuse and Specular (R, G, B, with
// alpha 1.0 from 00D7A24C), Density and BlurRadius are read without a null check
// natively; the host throws std::out_of_range when one is missing. Scale, Alpha
// and AffectedByWind are optional (004772C0 test first); AffectedByWind passes the
// byte value of a kind-3 property, otherwise 0. The visibility factor is the byte at
// manager+98h converted to float.
void apply_cloud_particle_properties_00477710(const CloudScenePropertyBag& bag,
                                              std::uint8_t manager_visibility_byte,
                                              CloudParticleTarget& target);

} // namespace bsp
