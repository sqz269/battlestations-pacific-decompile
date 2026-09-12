#pragma once
#include "bsp/native_particle_type_loading.hpp"

namespace bsp {
struct NativeParticleTypePropertyBindings;

// Complete native routines over actual particle-definition/TextBuffer storage.
// Original ECX definition, stack TextBuffer, RET4/AL true (including EOF).
// New C++ interfaces; native FH3 ABI and gameplay are not validated. Current
// Model +20 dispatch, shader services and atlas ownership remain borrowed.
bool load_native_object_particle_definition_00af8bd0(void*, void*,
    NativeParticleTypeLoadingBindings&);
bool load_native_tracer_particle_definition_00b0ad50(void*, void*,
    NativeParticleTypeLoadingBindings&);

// ECX Object definition, stack runtime Size parameter, RET4. Publish +84 then
// cache +88: null -> +0, kind0 -> raw+4, kind1 -> AFFA70(0), else AFFAE0(0).
// Uses actual 20h/28h segments, preserves constant bits and unchecked kinds.
void set_native_object_particle_size_00af80f0(void*, void*);

// ECX Tracer definition, stack filename, RET4. Reset only count+9C; grow
// actual +98/+9C/+A0 pointer descriptor by unsigned 2*capacity+2. Append
// borrowed actual atlas items, stopping on the first miss. No resource proxy.
void load_native_tracer_particle_textures_00b0a920(void*, const char*,
    NativeParticleTypePropertyBindings&);
} // namespace bsp
