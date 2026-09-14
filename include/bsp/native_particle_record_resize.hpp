#pragma once

#include "bsp/native_resource_record_vector.hpp"

#include <cstdint>

namespace bsp {
class ActualNativeStringPoolStorage;
struct SingletonLifetimeCallbacks;

// Borrow the existing concrete record/string providers. The caller owns both
// services and the actual 0Ch vector for the duration of this call.
struct NativeParticleRecordResizeContext {
    ActualNativeStringPoolStorage& strings;
    const SingletonLifetimeCallbacks& validation;
};

// Full 004DC410..004DC4DB normal body. Native ECX is the actual vector;
// EDX is this added source context; the original signed count remains the
// public stack word and RET4 consumes it. The count is captured at entry and
// reloaded from that same word only after each nonnull sentinel construction.
// No operation frame, canonical-owner admission or replay guard is required.
// The C++ exception path preserves the observed current-data read but is not
// a binary FH3/SEH replacement. See docs/NATIVE_PARTICLE_RECORD_RESIZE_CY.md.
void __fastcall resize_native_particle_record_array_004dc410(
    NativeResourceRecordVectorStorage& actual_vector,
    NativeParticleRecordResizeContext& context,
    volatile std::int32_t requested_count);

} // namespace bsp
