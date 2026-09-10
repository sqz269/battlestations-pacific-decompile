#include "bsp/system_camera_constants.hpp"
#include "bsp/material_constants.hpp"

namespace bsp {
namespace {
bool require_words(std::size_t capacity, std::size_t offset, std::size_t count,
    std::string& error) {
    if (offset > capacity || count > capacity - offset) {
        error = "System camera constants exceed the initialized prefix capacity";
        return false;
    }
    return true;
}
bool write_matrix(float* prefix, std::size_t capacity, unsigned register_index,
    const CameraMatrix& matrix, std::string& error) {
    const auto offset = static_cast<std::size_t>(register_index) * 4u;
    if (!require_words(capacity, offset, 16u, error)) return false;
    write_system_matrix_00b404a0(prefix + offset, matrix.data());
    return true;
}
}

const CameraMatrix& get_system_camera_service_matrix_00b0d100(
    const SystemCameraMatrixService& service) {
    return service.matrix_1d8;
}

bool pack_system_camera_constants_00b46a84(CameraFrameState& frame,
    const SystemCameraMatrixService& captured_service,
    const float* live_words_0108fc30,
    float* initialized_prefix_words, std::size_t word_count, std::string& error) {
    float* const prefix = initialized_prefix_words;
    auto& camera = frame.camera;
    auto& transform = camera.transform;

    //00B46A88..A92: captured service, then sequential x87 transpose to c0.
    const auto& service_matrix =
        get_system_camera_service_matrix_00b0d100(captured_service);
    if (!write_matrix(prefix, word_count, 0, service_matrix, error)) return false;

    //00B46A99..ACB: all three raw loads precede their stores. c6.w is untouched.
    if ((transform.valid_flags & 2u) == 0)
        refresh_camera_world_00b6db70(transform);
    if (!require_words(word_count, 6u * 4u, 3u, error)) return false;
    const float* const world = transform.world.data();
    float* const eye = prefix + 6u * 4u;
    __asm {
        mov eax, world
        mov ecx, eye
        movss xmm0, dword ptr [eax + 48]
        movss xmm1, dword ptr [eax + 52]
        movss xmm2, dword ptr [eax + 56]
        movss dword ptr [ecx], xmm0
        movss dword ptr [ecx + 4], xmm1
        movss dword ptr [ecx + 8], xmm2
    }

    //00B46AD1..AFD: preserve the second world-valid check after the view write.
    const auto& view = get_camera_view_00b6fcb0(transform);
    if (!write_matrix(prefix, word_count, 7, view, error)) return false;
    if ((transform.valid_flags & 2u) == 0)
        refresh_camera_world_00b6db70(transform);
    if (!write_matrix(prefix, word_count, 11, transform.world, error)) return false;

    //00B46B04..B39: projection(c23) precedes inverse view-projection(c19).
    const auto& view_projection = get_camera_view_projection_00b70490(camera);
    if (!write_matrix(prefix, word_count, 15, view_projection, error)) return false;
    const auto& projection = get_camera_projection_00b6fcf0(camera.projection);
    if (!write_matrix(prefix, word_count, 23, projection, error)) return false;
    const auto& inverse_view_projection = get_camera_inverse_view_projection_00b70510(frame);
    if (!write_matrix(prefix, word_count, 19, inverse_view_projection, error)) return false;

    //00B46B3E..C47: never prefetch/snapshot this live block before camera calls.
    // Explicit MOVSS pairs also prevent x87 NaN conversion or vector-wide loads.
    for (unsigned word = 0; word < 16; ++word) {
        if (!require_words(word_count, 27u * 4u + word, 1u, error)) return false;
        const float* const source = live_words_0108fc30 + word;
        float* const destination = prefix + 27u * 4u + word;
        __asm {
            mov eax, source
            mov ecx, destination
            movss xmm0, dword ptr [eax]
            movss dword ptr [ecx], xmm0
        }
    }
    return true;
}
}
