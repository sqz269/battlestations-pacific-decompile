#pragma once

#include <cstdint>

#include "bsp/native_damageable_class_model.hpp"

namespace bsp {

class NativeVehiclePointerArrayCalls;
struct SingletonLifetimeCallbacks;

// Required native boundary at 00443490. The recovered caller supplies a device
// class id in ECX and zero in DL. No default implementation is supplied: a host
// must connect the actual device-class lookup/activation operation.
class NativeVehicleEntryActivationCalls {
public:
    virtual ~NativeVehicleEntryActivationCalls() = default;
    virtual void call_00443490(std::uint32_t ecx_device_class_id,
        std::uint8_t edx_retain) = 0;
};

struct NativeVehicleClassActivationContext {
    NativeDamageableClassModelContext& model;
    NativeVehiclePointerArrayCalls& pointer_arrays;
    NativeVehicleEntryActivationCalls& entries;
    const SingletonLifetimeCallbacks& validation;
};

// Complete 297-byte 007F6E50 ordinary valid-layout body. ECX is one actual
// 98h-byte platform descriptor; RET has no stack cleanup. It activates every
// device-class reference in the intrusive list at +1Ch, including all four
// returning iterator-validation calls. An exact x87 tail removes zero-flag arc
// records narrower than the resident 00D08B88 half-degree threshold and keeps
// the listing's final flag-transition reads even though their result is discarded.
void activate_native_vehicle_platform_007f6e50(void* actual_platform,
    NativeVehicleEntryActivationCalls&, const SingletonLifetimeCallbacks&);

// Complete 112-byte 009598D0 ordinary valid-layout body. ECX is the actual
// vehicle class, enemy is the stacked argument and the native returns with
// RET4. The model activation and pointer-array growth use their existing
// source APIs; each current nonnull +94h entry runs full 007F6E50.
void activate_native_vehicle_class_009598d0(void* actual_class,
    std::uint32_t enemy, NativeVehicleClassActivationContext&,
    NativeDamageableClassModelAcquired&);

// These are explicit MSVC Win32 source interfaces, not original ABI/FH3/SEH
// bridges. Native object/list/array invariants remain required. Device-class
// call00443490 and model slot20 binding need real terminal implementations.

} // namespace bsp
