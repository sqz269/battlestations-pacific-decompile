#pragma once

namespace bsp {

// Complete B6FDE0..B6FDE6: ECX actual camera, EAX current DWORD at +180h,
// plain RET. The address is borrowed and may be null; the camera storage must
// be readable. No validation, retention, viewport registration or copying.
// Pass the actual 458h camera region, never its NativeCameraOwner companion.
// This one-argument Win32 fastcall leaf preserves the physical ECX/EAX/RET
// behavior; whole-program ABI compatibility and gameplay remain unproved.
void* __fastcall get_native_camera_viewport_00b6fde0(
    const void* actual_camera) noexcept;

// B4DEF0 submission is NOT implemented here. Its complete native call order
// is recorded in NATIVE_POST_EFFECT_SUBMISSION_BV.md. The final current pass
// virtual+08 requires an actual raw draw provider: D61BE8 selects B454D0,
// whose B44750 child currently has only semantic fragments. No callback or
// preparation-only substitute is exposed as a completed submission method.

} // namespace bsp
