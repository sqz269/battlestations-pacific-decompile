#include "bsp/native_particle_emitter_derived_lifetimes.hpp"
#include "bsp/native_particle_emitter_definition_lifetime.hpp"
#include "bsp/native_particle_type_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <initializer_list>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native derived emitter lifetimes require MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
void* at(const void* owner, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(owner) + offset);
}
void* parameter(void* owner, Word offset) noexcept {
    void* result;
    std::memcpy(&result, at(owner, offset), sizeof result);
    return result;
}
void write(void* owner, Word offset, Word value) noexcept {
    std::memcpy(at(owner, offset), &value, sizeof value);
}
void release_parameter(void* captured, NativeParticleTypeLifetimeContext& context) {
    if (captured) {
        destroy_native_particle_parameter_00affdf0(captured);
        return_native_particle_parameter_00b00090(captured, context.parameter_pool_00f8d344);
    }
}
struct EmitterBaseUnwind {
    void* owner;
    NativeParticleTypeLifetimeContext& context;
    bool active = true;
    ~EmitterBaseUnwind() noexcept {
        if (active) destroy_native_particle_definition_00afa100(owner, context);
    }
};
} // namespace

void destroy_native_particle_cone_definition_00b039f0(void* owner,
    NativeParticleTypeLifetimeContext& context) {
    write(owner, 0, 0x00d5debc);
    void* const first = parameter(owner, 0x80);
    EmitterBaseUnwind cleanup{owner, context}; // State0, map DF3774.
    release_parameter(first, context);
    for (Word offset : {0x84u, 0x88u, 0x8cu, 0x90u})
        release_parameter(parameter(owner, offset), context);
    cleanup.active = false; // B03A96 precedes the normal base call.
    destroy_native_particle_definition_00afa100(owner, context);
}

void destroy_native_particle_sphere_definition_00b02c40(void* owner,
    NativeParticleTypeLifetimeContext& context) {
    write(owner, 0, 0x00d5de88);
    void* const first = parameter(owner, 0x80);
    EmitterBaseUnwind cleanup{owner, context}; // State0, map DF3680.
    release_parameter(first, context);
    release_parameter(parameter(owner, 0x84), context);
    release_parameter(parameter(owner, 0x88), context);
    cleanup.active = false; // B02CB6 precedes the normal base call.
    destroy_native_particle_definition_00afa100(owner, context);
}

void destroy_native_particle_smartarea_definition_00b01d60(void* owner,
    NativeParticleTypeLifetimeContext& context) {
    write(owner, 0, 0x00d5de48);
    void* const first = parameter(owner, 0x80);
    EmitterBaseUnwind cleanup{owner, context}; // State0, map DF358C.
    release_parameter(first, context);
    release_parameter(parameter(owner, 0x84), context);
    for (Word offset : {0x88u, 0x8cu}) {
        void* const captured = parameter(owner, offset);
        if (captured) {
            release_parameter(captured, context);
            write(owner, offset, 0);
        }
    }
    cleanup.active = false; // B01E02 precedes the normal base call.
    destroy_native_particle_definition_00afa100(owner, context);
}

void* delete_native_particle_cone_definition_00b03b40(void* owner, Word flags,
    NativeParticleTypeLifetimeContext& context) {
    destroy_native_particle_cone_definition_00b039f0(owner, context);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}
void* delete_native_particle_sphere_definition_00b02fb0(void* owner, Word flags,
    NativeParticleTypeLifetimeContext& context) {
    destroy_native_particle_sphere_definition_00b02c40(owner, context);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}
void* delete_native_particle_smartarea_definition_00b01ea0(void* owner, Word flags,
    NativeParticleTypeLifetimeContext& context) {
    destroy_native_particle_smartarea_definition_00b01d60(owner, context);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}
} // namespace bsp
