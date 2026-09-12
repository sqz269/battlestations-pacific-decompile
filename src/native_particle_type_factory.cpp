#include "bsp/native_particle_type_factory.hpp"
#include "bsp/native_particle_type_base.hpp"
#include "bsp/native_particle_definition.hpp"
#include "bsp/native_string_compare.hpp"
#include <cstring>
#include <stdexcept>

namespace bsp {
static_assert(sizeof(void*) == 4, "Original particle type storage requires Win32");
namespace {
template<class T> T load(const void* p, std::uint32_t n = 0) noexcept {
    T value;
    std::memcpy(&value, reinterpret_cast<const void*>(
        reinterpret_cast<std::uintptr_t>(p) + n), sizeof value);
    return value;
}
void word(void* p, std::uint32_t n, std::uint32_t value) noexcept {
    *reinterpret_cast<volatile std::uint32_t*>(
        reinterpret_cast<std::uintptr_t>(p) + n) = value;
}
void profile(void* p, const volatile std::uint32_t* table) noexcept {
    word(p, 0, reinterpret_cast<std::uintptr_t>(table));
}
}

void* construct_native_sprite_particle_base_00b08830(void* p, const void* name,
    std::uint32_t value, void* parent, NativeParticleTypeFactoryBindings& a) {
    construct_native_particle_type_base_00b01150(p, name, value, parent, a.base);
    word(p, 0x10, 0);
    word(p, 0x80, 0); word(p, 0x84, 0); word(p, 0x88, 0);
    profile(p, a.sprite_base_profile_00d5dff4);
    return p;
}
void* construct_native_axial_particle_base_00b058e0(void* p, const void* name,
    std::uint32_t value, void* parent, NativeParticleTypeFactoryBindings& a) {
    construct_native_particle_type_base_00b01150(p, name, value, parent, a.base);
    word(p, 0x10, 0); word(p, 0xa0, 0);
    *reinterpret_cast<volatile std::uint8_t*>(
        reinterpret_cast<std::uintptr_t>(p) + 0x80u) = 0;
    word(p, 0x8c, 0); word(p, 0x90, 0);
    profile(p, a.axial_base_profile_00d5df30);
    word(p, 0x84, 0); word(p, 0x88, 0);
    return p;
}
void* construct_native_floating_particle_base_00b076f0(void* p, const void* name,
    std::uint32_t value, void* parent, NativeParticleTypeFactoryBindings& a) {
    construct_native_particle_type_base_00b01150(p, name, value, parent, a.base);
    word(p, 0x80, 0); word(p, 0x84, 0); word(p, 0x88, 0);
    profile(p, a.floating_base_profile_00d5dfb0);
    word(p, 0x10, 1);
    return p;
}
void* construct_native_floating_particle_definition_00b00770(void* p,
    const void* name, std::uint32_t value, void* parent,
    NativeParticleTypeFactoryBindings& a) {
    construct_native_floating_particle_base_00b076f0(p, name, value, parent, a);
    profile(p, a.floating_profile_00d5dcec);
    return p;
}
void* construct_native_object_particle_definition_00af89e0(void* p,
    const void* name, std::uint32_t value, void* parent,
    NativeParticleTypeFactoryBindings& a) {
    construct_native_particle_type_base_00b01150(p, name, value, parent, a.base);
    profile(p, a.object_profile_00d5db00);
    word(p, 0x8c, 0); word(p, 0x90, 0); word(p, 0x94, 0);
    word(p, 0x80, 0); word(p, 0x84, 0); word(p, 0x10, 2);
    return p;
}
void* construct_native_tracer_particle_definition_00b0a0b0(void* p,
    const void* name, std::uint32_t value, void* parent,
    NativeParticleTypeFactoryBindings& a) {
    construct_native_particle_type_base_00b01150(p, name, value, parent, a.base);
    profile(p, a.tracer_profile_00d5e048);
    word(p, 0x98, 0); word(p, 0x9c, 0); word(p, 0xa0, 0); word(p, 0x10, 3);
    return p;
}
void* create_native_particle_type_definition_00b00ce0(const void* kind,
    const void* name, void* parent, void* text,
    NativeParticleTypeFactoryBindings& a) {
    using Construct = void* (*)(void*, const void*, std::uint32_t, void*,
        NativeParticleTypeFactoryBindings&);
    Construct construct = nullptr;
    std::size_t bytes = 0;
    const volatile std::uint32_t* final_profile = nullptr;
    const char* data = load<const char*>(kind, 4);
    if (data && _stricmp(data, "SpriteParticle") == 0) {
        bytes = 0x90; construct = &construct_native_sprite_particle_base_00b08830;
        final_profile = a.sprite_profile_00d5dd18;
    } else if (equal_native_string_header_00425850(kind, "AxialParticle")) {
        bytes = 0xa4; construct = &construct_native_axial_particle_base_00b058e0;
        final_profile = a.axial_profile_00d5dcc0;
    } else if (equal_native_string_header_00425850(kind, "FloatingParticle")) {
        bytes = 0x8c; construct = &construct_native_floating_particle_definition_00b00770;
    } else if (equal_native_string_header_00425850(kind, "ObjectParticle")) {
        bytes = 0x98; construct = &construct_native_object_particle_definition_00af89e0;
    } else if (equal_native_string_header_00425850(kind, "TracerParticle")) {
        bytes = 0xe8; construct = &construct_native_tracer_particle_definition_00b0a0b0;
    }
    void* owner = parent;
    if (construct) {
        owner = a.base.owners.allocate_00bf681b(bytes);
        if (owner) {
            try {
                construct(owner, name, load<std::uint32_t>(parent, 0x10), parent, a);
                if (final_profile) profile(owner, final_profile);
            } catch (...) {
                a.base.owners.free_00bf65ac(owner);
                throw;
            }
        }
    }
    const void* table = load<const void*>(owner);
    const auto target = load<std::uint32_t>(table, 8);
    if (!a.parser_virtual08)
        throw std::logic_error("particle type requires current parser virtual08");
    a.parser_virtual08(a.context, owner, target, text);
    return owner;
}
} // namespace bsp
