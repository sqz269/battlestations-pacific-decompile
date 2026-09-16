#include "bsp/native_camera_view_set.hpp"
#include "bsp/native_camera_matrix_math.hpp"
#include "bsp/native_camera_owner.hpp"

#include <stdexcept>

namespace bsp {
void set_native_camera_view_00b71490(NativeCameraOwner& owner,
    const void* actual_view_matrix) {
    static_assert(sizeof(CameraMatrix) == 0x40 && sizeof(void*) == 4);
    // Explicit raw-cell accesses keep the native mask before profile capture.
    auto& flags = static_cast<volatile std::uint32_t&>(owner.storage.camera.valid_flags_2f0);
    flags = flags & 0xfffffe4bu;
    const std::uint32_t captured_profile =
        static_cast<const volatile std::uint32_t&>(owner.storage.node.vtable_00);
    CameraMatrix temporary;
    const auto* world = static_cast<const CameraMatrix*>(
        invert_native_camera_scaled_affine_00b63b30(&temporary, actual_view_matrix));

    // Do not validate/load the slot before inverse or recapture owner[0].
    if (captured_profile != 0x00d62cf0u || !owner.environment.vtable_00d62cf0)
        throw std::logic_error("camera view setter requires captured D62CF0 profile");
    const auto target = owner.environment.vtable_00d62cf0[0x34 / 4];
    if (target != 0x00b71460u)
        throw std::logic_error("camera view setter requires current B71460 target");
    owner.pose.invoke_world_matrix_34(owner.pose.context, target, owner.camera, *world);
    refresh_camera_direction_00b70660(owner.camera);
}
} // namespace bsp
