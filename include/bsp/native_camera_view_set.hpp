#pragma once

namespace bsp {
class NativeCameraOwner;

// Full B71490..B714C6 (55B): native ECX camera, stacked view pointer, RET4.
// Borrow the SAME initialized actual camera and its existing views/environment.
// The input pointer is captured before masking actual+2F0. Capture owner[0]
// before raw inverse (ECX private64B destination, EDX input), then read that
// profile's current+34 target AFTER inverse. Admit D62CF0/+34=B71460 only;
// unsupported dispatch throws after the preceding mask/inverse effects.
// B71460 already refreshes direction; the wrapper performs B70660 again.
// New C++ owner interface, not native thiscall/FH3/private-stack/fault ABI.
// Valid live64B input, actual owner fields/bindings and clear DF are required;
// arbitrary private-stack aliases/concurrent profile mutation are unproved.
void set_native_camera_view_00b71490(NativeCameraOwner&,
    const void* actual_view_matrix);
} // namespace bsp
