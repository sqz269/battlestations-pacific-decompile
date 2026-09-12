#pragma once
#include "bsp/native_particle_type_base.hpp"

namespace bsp {
struct NativeParticleTypeResourceBindings {
    NativeParticleTypeBaseBindings& base;
    void* context;
    // Required actual singleton/resource services. They may mutate names or
    // publications; no semantic resource copy or successful-null fallback.
    void* const volatile* actual_vfs_0109ceec;
    void* const volatile* factory_00f8d31c;
    const char* empty_stem_00f8d320;
    bool (*resolve_existing_name_00bdf4c0)(void*,void* actual_vfs,void* actual_name8h);
    void* (*resource_manager_004c1400)(void*);
    void* (*load_and_cache_00b80720)(void*,void* actual_manager,
        const void* actual_name8h,void* actual_factory);
};
// Three complete original ECX definition, stack C-string, RET4 setters.
// They release both actual8h temporary strings BEFORE publishing definition+7C.
void set_native_sprite_particle_shader_00b089e0(void*,const char*,NativeStringStorage&);
void set_native_axial_particle_shader_00b06210(void*,const char*,NativeStringStorage&);
void set_native_floating_particle_shader_00b07c80(void*,const char*,NativeStringStorage&);
void native_object_particle_shader_noop_00af80e0(void*,const char*) noexcept;
void native_tracer_particle_shader_noop_00b0a040(void*,const char*) noexcept;
// Return whether captured CURRENT target is one of the five reviewed entries.
bool dispatch_known_native_particle_shader(void*,std::uint32_t,const char*,NativeStringStorage&);

// AF8350 ECX actual0Ch descriptor, stack signed requested count, RET4.
void reserve_native_object_particle_models_00af8350(void*,std::int32_t,
    NativeParticleTypeBaseBindings&);
// AF8940 ECX actual98h Object definition, RET: reverse captured-cell decref,
// clear that cell and decrement CURRENT count. Keeps allocation/capacity.
void clear_native_object_particle_models_00af8940(void*,NativeParticleTypeBaseBindings&);
// B80D70 ECX actual manager; stack name8h, RET4/EAX. Current manager+4 is
// passed to the existing real B80720 body with original name/factory RET8.
void* load_native_resource_with_default_factory_00b80d70(void*,const void*,
    NativeParticleTypeResourceBindings&);
// AF9660 ECX actual Object definition, stack C-string, RET4. Complete single
// model/numbered sequence flow through actual mutable-name/resource services.
void load_native_object_particle_models_00af9660(void*,const char*,
    NativeParticleTypeResourceBindings&);
} // namespace bsp
