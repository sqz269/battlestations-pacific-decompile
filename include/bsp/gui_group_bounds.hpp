#pragma once
#include "bsp/native_group_owner.hpp"
#include "bsp/system_camera_axes.hpp"

namespace bsp {
// Direct B8E6C0 body over the SAME live group owner. Source names four float32
// lanes and may overlap native storage. Native clears flags138 mask30 and
// byte175 before four forward FLD/FSTP pairs into08..17. These are NOT raw-word
// copies: preserve current x87 signaling-NaN/denormal/exception behavior. No
// enclosing-owner notification, virtual dispatch or eager bounds recomputation.
void set_native_gui_group_bounds_00b8e6c0(NativeGroupOwner&, const void* sphere);

struct GuiGroupBoundsCrtAccess {
    const CameraAxesCrtAccess* sqrt;
    const volatile std::uint32_t* sse2_conversion_0109eea4;
};
// Radius subexpression AC5F00..AC5F38. Load the supplied actual D7A308 double,
// execute the EXISTING CRT sqrt kernel, spill/reload float32, reload actual
// 0109EEA4 for BF7420, then signed EAX -> CVTSI2SS. Default installed input is
// double2.0. No magic radius, cached mode, C++ cast or alternate CRT is used.
// The existing CRT access/handler and native x87 fallback own their established
// error policies. This C++ float-return wrapper is not original Screen ABI.
float gui_screen_root_radius_00ac5f00(const volatile double& squared_radius,
    const GuiGroupBoundsCrtAccess&);
} // namespace bsp
