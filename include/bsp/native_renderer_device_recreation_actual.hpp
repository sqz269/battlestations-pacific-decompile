#pragma once
#include <cstdint>

namespace bsp {
struct NativeRendererResourceReleaseContext;
struct NativeRendererResourceRestoreContext;
struct NativeRendererFrameTargetsContext;
struct NativeLogicalBufferDeviceSaveContext;
struct NativeLogicalBufferDeviceRestoreContext;
struct NativeHardwareLayoutConstructContext;
struct NativeRendererResetReadinessProfiles;
struct NativeRendererGammaContext;
struct NativeTexture2DRetainedRecreationContext;
struct NativeCubeVolumeRetainedRecreationContext;
struct SingletonLifetimeCallbacks;
class NativeXLiveDeviceAdapter;

// Borrow the SAME actual renderer, publications, ownership domains and tree
// used by the full dependency providers. No owner, cache, array or manager is
// mirrored. Texture profile tables in release.actual_resources must extend
// through +2F here; their current +28/+2C select the retained-source callbacks.
struct NativeRendererDeviceRecreationContext {
    NativeRendererResourceReleaseContext& release;
    NativeRendererResourceRestoreContext& restore;
    NativeRendererFrameTargetsContext& frame_targets;
    NativeLogicalBufferDeviceSaveContext& logical_save;
    NativeLogicalBufferDeviceRestoreContext& logical_restore;
    NativeHardwareLayoutConstructContext& hardware_layout;
    const SingletonLifetimeCallbacks& tree_validation;
    void* actual_hardware_layout_tree_0108d530;
    const NativeRendererResetReadinessProfiles& physical_profiles;
    const NativeRendererGammaContext& gamma;
    NativeTexture2DRetainedRecreationContext& texture_2d;
    NativeCubeVolumeRetainedRecreationContext& cube_volume;
    const volatile std::uint32_t* actual_online_publication_00f8abe8;
    const NativeXLiveDeviceAdapter* online_device; // reached only on nonzero gate
};

// Complete B29670..B29B16 normal body. Native ECX renderer/no stack args/RET;
// this source ABI adds the borrowed context. Actual renderer through +1D8C,
// all reached raw storage and current concrete profiles must remain valid.
// Preserve current lifecycle-lock reload at normal exit, checked tree cursors,
// unsigned live owner counts, retained record cursors, COM/SDK order, ignored
// HRESULTs, direct writable device/presentation outputs and x87 gamma spill.
void recreate_native_renderer_device_00b29670(void* actual_renderer,
    NativeRendererDeviceRecreationContext&);

// Only the optional guard has native EH cleanup. An exceptional path does NOT
// release the separate lifecycle lock or undo completed resource/device work.
// A skipped guard is uninitialized; later enabling its cleanup is outside the
// valid caller domain. Unwritten GetDeviceCaps stack bytes remain unspecified.
// New source interfaces do not establish original caller/FH3/SEH identity,
// concurrent worker/device safety, complete file loading or gameplay validity.
} // namespace bsp
