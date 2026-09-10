#include "bsp/system_constant_builder.hpp"
#include "bsp/system_camera_constants.hpp"
#include "bsp/system_camera_axes.hpp"
#include "bsp/system_time_constants.hpp"
#include "bsp/system_fog_constants.hpp"
#include "bsp/system_lighting_constants.hpp"

namespace bsp {
namespace {
void remember_failure(HRESULT result, HRESULT& accumulated) noexcept {
    if (FAILED(result) && SUCCEEDED(accumulated)) accumulated = result;
}
bool upload_count_valid(std::uint32_t count, const SystemConstantPrefix& prefix,
    std::string& error) {
    if (count > prefix.size() / 4) {
        error = "System constant upload exceeds the supplied native prefix span";
        return false;
    }
    return true;
}
}

const CameraPlane* get_camera_context_depth_scale_00b6feb0(
    const CameraFrameState& camera) noexcept {
    const auto* slot = &camera.context_depth_scale_43c;
    const CameraPlane* result;
    __asm {
        mov eax, slot
        mov eax, dword ptr [eax]
        mov result, eax
    }
    return result;
}

bool write_system_context_constants_00b475b4(const CameraFrameState& camera,
    SystemConstantPrefix& prefix, std::string& error) {
    if (!get_camera_context_depth_scale_00b6feb0(camera)) return true;
    const auto* actual = get_camera_context_depth_scale_00b6feb0(camera);
    if (!actual) {
        error = "Camera context depth-scale owner disappeared before its native read";
        return false;
    }
    const float* source = actual->data();
    float* destination = prefix.data() + 71 * 4;
    for (unsigned lane = 0; lane != 4; ++lane) {
        const float* input = source + lane;
        float* output = destination + lane;
        __asm {
            mov eax, input
            mov ecx, output
            movss xmm0, dword ptr [eax]
            movss dword ptr [ecx], xmm0
        }
    }
    return true;
}

bool upload_system_constant_prefix_00b475fd(const SystemConstantPrefix& prefix,
    D3D9StateCache* const volatile& renderer_slot,
    const volatile std::uint32_t& count_slot, HRESULT& device_result,
    std::string& error) {
    const auto vertex_count = count_slot;
    auto* const vertex_renderer = renderer_slot;
    if (!upload_count_valid(vertex_count, prefix, error)) return false;
    if (vertex_count) {
        if (!vertex_renderer) {
            error = "System vertex upload requires its actual renderer";
            return false;
        }
        remember_failure(vertex_renderer->set_vertex_shader_constants_f_00b21820(
            0, prefix.data(), vertex_count), device_result);
    }
    const auto pixel_count = count_slot;
    auto* const pixel_renderer = renderer_slot;
    if (!upload_count_valid(pixel_count, prefix, error)) return false;
    if (pixel_count) {
        if (!pixel_renderer) {
            error = "System pixel upload requires its actual renderer";
            return false;
        }
        remember_failure(pixel_renderer->set_pixel_shader_constants_f_00b218c0(
            0, prefix.data(), pixel_count), device_result);
    }
    return true;
}

bool build_and_upload_system_constants_00b46a70(const SystemLightingScene* scene,
    CameraFrameState& camera, SystemConstantPrefix& prefix,
    SystemConstantBuilderEnvironment& environment, HRESULT& device_result,
    std::string& error) {
    error.clear();
    //00B46A7B: capture the SAME live service later reloaded by the time stage.
    const auto* captured_service = environment.time.renderer_00f8d39c;
    if (!captured_service) {
        error = "System camera matrices require the actual captured service";
        return false;
    }
    const SystemCameraMatrixService camera_service{captured_service->matrix_1d8};
    if (!pack_system_camera_constants_00b46a84(camera, camera_service,
        environment.parameters_0108fc30, prefix.data(), prefix.size(), error)) return false;
    auto* captured_timer = write_system_camera_axes_00b46c50(camera,
        prefix.data(), prefix.size(), environment.time.timer_01090ab0,
        environment.axes_crt);
    const auto* captured_fog = write_system_time_constants_00b46cb4(camera,
        captured_timer, prefix.data(), prefix.size(), environment.time);
    if (!write_system_fog_constants_00b46d97(captured_fog, camera.fog_184,
        prefix.data(), prefix.size(), error)) return false;
    const auto lighting = write_system_lighting_shadow_prefix_00b46ed1(scene,
        camera.render_mode, environment.exponent_00cfad80, prefix.data(),
        prefix.size(), error, environment.invalid_parameter_runtime);
    if (lighting != SystemLightingPrefixStatus::complete
        && lighting != SystemLightingPrefixStatus::scene_absent
        && lighting != SystemLightingPrefixStatus::lighting_absent
        && lighting != SystemLightingPrefixStatus::shadow_absent) return false;
    return write_system_context_constants_00b475b4(camera, prefix, error)
        && upload_system_constant_prefix_00b475fd(prefix,
            environment.renderer_00f8d394, environment.register_count_00e13078,
            device_result, error);
}
}
