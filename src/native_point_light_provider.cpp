#include "bsp/native_point_light_provider.hpp"
#include "bsp/model_bounds.hpp"
#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
NativeNodeStorage& raw_node(SceneNodeAttachment& binding) {
    const auto key = binding.transform.raw_node_key();
    if (!key || key != binding.pointer_key)
        throw std::invalid_argument("PointLight population requires the actual node identity");
    return *reinterpret_cast<NativeNodeStorage*>(key);
}
void copy_sphere(const float* from, float* to) {
    __asm {
        mov ecx, from
        mov eax, to
        fld dword ptr [ecx]
        fstp dword ptr [eax]
        fld dword ptr [ecx + 4]
        fstp dword ptr [eax + 4]
        fld dword ptr [ecx + 8]
        fstp dword ptr [eax + 8]
        fld dword ptr [ecx + 12]
        fstp dword ptr [eax + 12]
    }
}
const float* model_sphere(NativeNodeStorage& raw, CameraTransform& transform) {
    if ((raw.auxiliary_flags_138 & 0x30u) == 0) {
        if ((raw.valid_flags_5c & 2u) == 0)
            refresh_camera_world_00b6db70(transform);
        std::array<float, 4> local, result;
        std::memcpy(local.data(), raw.untouched_08.data(), sizeof(local));
        transform_sphere_007c1180(local, transform.world, result);
        copy_sphere(result.data(), raw.world_sphere_13c.data());
        raw.auxiliary_flags_138 |= 0x30u;
    }
    return raw.world_sphere_13c.data();
}
struct Distance {
    double length;
    float light_radius;
};
Distance distance_to_light(const void* raw_light, const float* sphere,
    const CameraAxesCrtAccess& access) {
    float delta[3];
    auto* difference = delta;
    const auto* crt = &access;
    Distance result;
    auto* output = &result;
    // B6EF49..B6EF84 / B6EFFA..B6F035: spill each subtraction and capture
    // radius before length. Length already returns a float-rounded ST0;
    // its caller retains that value in an eight-byte temporary.
    __asm {
        mov eax, raw_light
        mov edx, sphere
        mov ecx, difference
        fld dword ptr [eax + 0x1ec]
        fsub dword ptr [edx]
        fstp dword ptr [ecx]
        fld dword ptr [eax + 0x1f0]
        fsub dword ptr [edx + 4]
        fstp dword ptr [ecx + 4]
        fld dword ptr [eax + 0x1f4]
        fsub dword ptr [edx + 8]
        fstp dword ptr [ecx + 8]
        mov edx, output
        fld dword ptr [eax + 0x1f8]
        fstp dword ptr [edx + 8]
        mov edx, crt
        call camera_vector_length_00419440
        mov edx, output
        fstp qword ptr [edx]
    }
    return result;
}
bool group_outside(const Distance& distance, const float* current_sphere) {
    const auto* value = &distance;
    float sum;
    unsigned char answer;
    __asm {
        mov edx, value
        mov eax, current_sphere
        fld dword ptr [eax + 12]
        fadd dword ptr [edx + 8]
        fstp sum
        fld sum
        fld qword ptr [edx]
        fcomip st(0), st(1)
        fstp st(0)
        seta answer
    }
    return answer != 0;
}
bool node_intersects(const Distance& distance, const float* current_sphere) {
    const auto* value = &distance;
    float sum;
    unsigned char answer;
    __asm {
        mov edx, value
        mov eax, current_sphere
        fld dword ptr [eax + 12]
        fadd dword ptr [edx + 8]
        fstp sum
        fld qword ptr [edx]
        fld sum
        fcomip st(0), st(1)
        fstp st(0)
        seta answer
    }
    return answer != 0;
}
bool is_type(NativePointLightPopulationRuntime& runtime,
    SceneNodeAttachment& binding, std::uint32_t token) {
    if (!binding.is_type)
        throw std::logic_error("PointLight population requires current virtual0C");
    return binding.is_type(runtime.nodes.scenes, binding, token);
}
void require_light(NativePointLightPopulationRuntime& runtime, NativePointLightOwner& light) {
    if (&runtime.nodes != &light.environment.nodes ||
        light.phase != NativePointLightOwner::Phase::live ||
        &runtime.nodes.point_lights.light(light.backlinks.identity) != &light.backlinks)
        throw std::logic_error("PointLight population requires the same live owner domain");
}
}

const float* native_model_population_sphere_00b6e8c0(NativeNodeBinding& node) {
    return model_sphere(node.storage, node.transform);
}
const float* native_point_light_supported_world_sphere(
    NativePointLightPopulationRuntime&, SceneNodeAttachment& binding) {
    auto& raw = raw_node(binding);
    if (raw.vtable_00 == 0x00d62de8)
        return model_sphere(raw, binding.transform);
    if (raw.vtable_00 == 0x00d634f8) {
        const auto* bytes = reinterpret_cast<const unsigned char*>(&raw);
        if (bytes[0x175] != 0 && (raw.auxiliary_flags_138 & 0x30u) == 0)
            throw std::logic_error("Group dynamic bounds B8EBE0 are not bound");
        return model_sphere(raw, binding.transform);
    }
    throw std::logic_error("Current native virtual48 has no PointLight bounds binding");
}
void populate_native_node_point_light_00b6ef20(NativePointLightPopulationRuntime& runtime,
    CameraTransform& transform, NativePointLightOwner& light) {
    require_light(runtime, light);
    auto& binding = runtime.nodes.scenes.resolve(transform);
    auto& raw = raw_node(binding);
    const bool group = is_type(runtime, binding, runtime.group_0109032c);
    if (group || is_type(runtime, binding, runtime.object_01090034) ||
        is_type(runtime, binding, runtime.token_01090344) ||
        is_type(runtime, binding, runtime.token_010903c8)) {
        if (!runtime.sphere_virtual48)
            throw std::logic_error("PointLight population requires current virtual48");
        const auto distance = distance_to_light(light.backlinks.identity,
            runtime.sphere_virtual48(runtime, binding), runtime.crt);
        // Reload the dispatch and sphere after the length call, just as native.
        if (!runtime.sphere_virtual48)
            throw std::logic_error("PointLight population lost current virtual48");
        const auto* sphere = runtime.sphere_virtual48(runtime, binding);
        if (group) {
            if (group_outside(distance, sphere)) return;
        } else if (node_intersects(distance, sphere)) {
            append_native_node_point_light_00b6eed0(runtime.nodes.point_lights,
                raw, light.backlinks.identity);
        }
    }
    auto* child = transform.first_child.get();
    while (child) {
        populate_native_node_point_light_00b6ef20(runtime, *child, light);
        child = child->next_sibling.get();
    }
}
void populate_native_root_point_light_00b72140(NativePointLightPopulationRuntime& runtime,
    RenderNodeRootList& roots, NativePointLightOwner& light) {
    require_light(runtime, light);
    auto* node = roots.first;
    while (node) {
        populate_native_node_point_light_00b6ef20(runtime, *node, light);
        node = node->next_sibling.get();
    }
}
void populate_native_point_light_if_unlinked_00b7b090(
    NativePointLightPopulationRuntime& runtime, NativePointLightOwner& light,
    RenderNodeRootList& roots) {
    require_light(runtime, light);
    if (light.light.backlinks_1e0.count == 0)
        populate_native_root_point_light_00b72140(runtime, roots, light);
}

void initialize_native_particle_point_light_volume_00b0ca40_fragment(
    NativePointLightLinksRuntime& links, void* const volatile& actual_light_60,
    CameraTransform& emitter, const volatile std::uint8_t& relative_1b0,
    const float* position, const volatile std::uint32_t& radius_8c,
    const volatile std::uint32_t& minimum_00d7a238) {
    void* light = actual_light_60;
    if (!light) return;
    (void)links.light(light);
    if (relative_1b0 != 0) {
        if ((emitter.valid_flags & 2u) == 0)
            refresh_camera_world_00b6db70(emitter);
        light = actual_light_60; // B0CB13 reload, following possible callbacks
        (void)links.light(light);
        const auto* world = emitter.world.data() + 12;
        float translation[3], result[3];
        auto* translated = translation;
        auto* result_values = result;
        __asm {
            mov ecx, world
            mov edx, translated
            fld dword ptr [ecx]
            fstp dword ptr [edx]
            fld dword ptr [ecx + 4]
            fstp dword ptr [edx + 4]
            fld dword ptr [ecx + 8]
            fstp dword ptr [edx + 8]
            mov ecx, position
            mov eax, result_values
            fld dword ptr [ecx]
            fadd dword ptr [edx]
            fstp dword ptr [eax]
            fld dword ptr [ecx + 4]
            fadd dword ptr [edx + 4]
            fstp dword ptr [eax + 4]
            fld dword ptr [edx + 8]
            fadd dword ptr [ecx + 8]
            fstp dword ptr [eax + 8]
        }
        position = result;
        // All relative sums precede the first destination write.
        __asm {
            mov eax, light
            mov ecx, position
            fld dword ptr [ecx]
            fstp dword ptr [eax + 0x1ec]
            fld dword ptr [ecx + 4]
            fstp dword ptr [eax + 0x1f0]
            fld dword ptr [ecx + 8]
        }
    } else {
        // Absolute position may alias native storage: forward FLD/FSTP pairs.
        __asm {
            mov eax, light
            mov ecx, position
            fld dword ptr [ecx]
            fstp dword ptr [eax + 0x1ec]
            fld dword ptr [ecx + 4]
            fstp dword ptr [eax + 0x1f0]
            fld dword ptr [ecx + 8]
        }
    }
    const auto* minimum = &minimum_00d7a238;
    const auto* radius = &radius_8c;
    const auto* slot = &actual_light_60;
    // Keep input Z in ST0 until AFTER the native minimum MOVSS load.
    __asm {
        mov eax, light
        mov ecx, minimum
        movss xmm1, dword ptr [ecx]
        fstp dword ptr [eax + 0x1f4]
        mov edx, radius
        movss xmm0, dword ptr [edx]
        comiss xmm0, xmm1
        mov edx, slot
        mov eax, dword ptr [edx]
        ja keep_radius
        movaps xmm0, xmm1
    keep_radius:
        movss dword ptr [eax + 0x1f8], xmm0
    }
}
} // namespace bsp
