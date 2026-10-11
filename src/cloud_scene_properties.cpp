#include "bsp/cloud_scene_properties.hpp"

#include <stdexcept>

namespace bsp {

namespace {

const CloudSceneProperty& required(const CloudScenePropertyBag& bag, const char* key) {
    const auto it = bag.find(key);
    if (it == bag.end()) throw std::out_of_range(std::string("cloud property missing: ") + key);
    return it->second;
}

const CloudSceneProperty* optional(const CloudScenePropertyBag& bag, const char* key) {
    const auto it = bag.find(key);
    return it == bag.end() ? nullptr : &it->second;
}

// 004773A0(name), then R/G/B (00CE60EC/E8/E4) in its nested bag; alpha is 1.0.
void read_color(const CloudScenePropertyBag& bag, const char* key, float out[4]) {
    const auto& property = required(bag, key);
    if (!property.nested) throw std::out_of_range(std::string("cloud colour has no channels: ") + key);
    out[0] = required(*property.nested, "R").as_float();
    out[1] = required(*property.nested, "G").as_float();
    out[2] = required(*property.nested, "B").as_float();
    out[3] = 1.0f;  // 00D7A24C
}

} // namespace

CloudPropertyBranch cloud_property_branch_00477710(std::int32_t cloud_type, bool has_instance,
                                                    bool has_holder, std::int32_t holder_kind,
                                                    bool suppressed) noexcept {
    if (cloud_type == 0 && !has_instance) return CloudPropertyBranch::None;
    if (!has_holder || holder_kind != 1 || suppressed) return CloudPropertyBranch::None;
    switch (cloud_type) {
    case 0: return CloudPropertyBranch::Particle;
    case 1: return CloudPropertyBranch::ShadedMesh;
    case 2: return CloudPropertyBranch::DiffuseMesh;
    default: return CloudPropertyBranch::None;
    }
}

void apply_cloud_particle_properties_00477710(const CloudScenePropertyBag& bag,
                                              std::uint8_t manager_visibility_byte,
                                              CloudParticleTarget& target) {
    float color[4];
    read_color(bag, "InsideColor", color);   // 00477789..00477836
    target.set_inside_color(color);
    read_color(bag, "Ambient", color);       // 00477844..004778F1
    target.set_ambient(color);
    read_color(bag, "Diffuse", color);       // 004778FF..004779AC
    target.set_diffuse(color);
    read_color(bag, "Specular", color);      // 004779BA..00477A67
    target.set_specular(color);
    target.set_density(required(bag, "Density").as_float());         // 00477A75..00477AA5
    target.set_blur_radius(required(bag, "BlurRadius").as_float());  // 00477AB3..00477AE3
    if (const auto* p = optional(bag, "Scale")) target.set_scale(p->as_float());  // 00477AF1..00477B38
    if (const auto* p = optional(bag, "Alpha")) target.set_alpha(p->as_float());  // 00477B46..00477B8D
    if (const auto* p = optional(bag, "AffectedByWind"))                         // 00477B9B..00477BE4
        target.set_affected_by_wind(p->kind == 3 ? p->byte : std::uint8_t{0});
    target.set_visibility_factor(static_cast<float>(manager_visibility_byte));  // 00477BFA..00477C13
    target.register_with_manager();                                            // 00477C32
}

} // namespace bsp
