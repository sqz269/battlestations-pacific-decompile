#pragma once
#include "bsp/native_settings_choices.hpp"

namespace bsp {
struct NativeSettingsVectorLifetimeCalls : NativeSettingsChoiceCalls {
    virtual int register_shutdown_00bf6ff5(void (*)());
    virtual void free_00bf6989(void*);
};
// Complete CD2D60/CD2D70: no native inputs, atexit thunk, EAX status, RET.
// The composition owner supplies stable callbacks for the two process headers.
int register_native_settings_resolutions_00cd2d60(NativeSettingsVectorLifetimeCalls&,void (*)());
int register_native_settings_antialias_00cd2d70(NativeSettingsVectorLifetimeCalls&,void (*)());
// Complete CDEE40, no native inputs/RET: signed capacity<0 reserves zero;
// the positive-count loop is register-only. Capture current backing, clear
// count, free captured backing. Pointer/capacity remain dead, not reset.
void shutdown_native_settings_resolutions_00cdee40(void*,NativeSettingsVectorLifetimeCalls&);
// Complete CDEE80: resize DWORD header to zero, reload backing, free; RET.
void shutdown_native_settings_antialias_00cdee80(void*,NativeSettingsVectorLifetimeCalls&);
} // namespace bsp
