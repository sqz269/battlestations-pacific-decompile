#include "bsp/native_particle_three_lifetimes.hpp"
#include "bsp/native_particle_type_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle type lifetimes require MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
void* at(const void* p, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + offset);
}
void* parameter(void* owner, Word offset) noexcept {
    void* value;
    std::memcpy(&value, at(owner, offset), sizeof value);
    return value;
}
void stamp(void* owner, Word profile) noexcept {
    std::memcpy(owner, &profile, sizeof profile);
}
void release_parameter(void* captured, NativeParticleTypeLifetimeContext& context) {
    if (captured) {
        destroy_native_particle_parameter_00affdf0(captured);
        return_native_particle_parameter_00b00090(captured, context.parameter_pool_00f8d344);
    }
}
struct BaseUnwind {
    void* owner;
    NativeParticleTypeLifetimeContext& context;
    bool active = true;
    ~BaseUnwind() noexcept {
        if (active) destroy_native_particle_type_base_00b00fb0(owner, context);
    }
};
} // namespace

void destroy_native_axial_particle_type_00b05940(void* owner,
    NativeParticleTypeLifetimeContext& context) {
    stamp(owner, 0x00d5df30);
    void* const first = parameter(owner, 0x8c);
    BaseUnwind cleanup{owner, context}; // CBB7E8 -> DF3930; state0.
    release_parameter(first, context);
    release_parameter(parameter(owner, 0x90), context);
    cleanup.active = false; // B0599E precedes the normal base call.
    destroy_native_particle_type_base_00b00fb0(owner, context);
}

void destroy_native_floating_particle_type_00b07730(void* owner,
    NativeParticleTypeLifetimeContext& context) {
    stamp(owner, 0x00d5dfb0);
    void* const first = parameter(owner, 0x84);
    BaseUnwind cleanup{owner, context}; // CBB8B8 -> DF3A44; state0.
    release_parameter(first, context);
    release_parameter(parameter(owner, 0x80), context);
    release_parameter(parameter(owner, 0x88), context);
    cleanup.active = false; // B077A6 precedes the normal base call.
    destroy_native_particle_type_base_00b00fb0(owner, context);
}

void destroy_native_sprite_particle_type_00b088b0(void* owner,
    NativeParticleTypeLifetimeContext& context) {
    stamp(owner, 0x00d5dff4);
    void* const first = parameter(owner, 0x84);
    BaseUnwind cleanup{owner, context}; // CBB948 -> DF3AF8; state0.
    release_parameter(first, context);
    release_parameter(parameter(owner, 0x80), context);
    release_parameter(parameter(owner, 0x88), context);
    cleanup.active = false; // B08926 precedes the normal base call.
    destroy_native_particle_type_base_00b00fb0(owner, context);
}

void* delete_native_axial_particle_definition_00b008c0(void* owner, Word flags,
    NativeParticleTypeLifetimeContext& context) {
    destroy_native_axial_particle_type_00b05940(owner, context);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}
void* delete_native_axial_particle_base_00b05ce0(void* owner, Word flags,
    NativeParticleTypeLifetimeContext& context) {
    destroy_native_axial_particle_type_00b05940(owner, context);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}
void* delete_native_floating_particle_definition_00b008e0(void* owner, Word flags,
    NativeParticleTypeLifetimeContext& context) {
    destroy_native_floating_particle_type_00b07730(owner, context);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}
void* delete_native_floating_particle_base_00b07c60(void* owner, Word flags,
    NativeParticleTypeLifetimeContext& context) {
    destroy_native_floating_particle_type_00b07730(owner, context);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}
void* delete_native_sprite_particle_definition_00b00900(void* owner, Word flags,
    NativeParticleTypeLifetimeContext& context) {
    destroy_native_sprite_particle_type_00b088b0(owner, context);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}
void* delete_native_sprite_particle_base_00b089c0(void* owner, Word flags,
    NativeParticleTypeLifetimeContext& context) {
    destroy_native_sprite_particle_type_00b088b0(owner, context);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}
} // namespace bsp
