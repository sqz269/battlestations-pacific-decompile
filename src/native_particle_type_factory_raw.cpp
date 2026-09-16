#include "bsp/native_particle_type_factory_raw.hpp"
#include "bsp/native_particle_type_construction.hpp"
#include "bsp/native_string_compare.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle type factory requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
template<class T> volatile T& field(const void* p, Word offset = 0) noexcept {
    return *reinterpret_cast<volatile T*>(reinterpret_cast<Word>(p) + offset);
}
const volatile Word* current_table(Word profile, NativeParticleTypeFactoryRawContext& c) {
    auto& p = c.sprite_floating.properties;
    switch (profile) {
    case 0x00d5dd18: return p.sprite_profile_00d5dd18;
    case 0x00d5dcc0: return p.axial_profile_00d5dcc0;
    case 0x00d5dcec: return p.floating_profile_00d5dcec;
    case 0x00d5db00: return p.object_profile_00d5db00;
    case 0x00d5e048: return p.tracer_profile_00d5e048;
    case 0x00d5de48: return c.smartarea_profile_00d5de48;
    case 0x00d5de88: return c.sphere_profile_00d5de88;
    case 0x00d5debc: return c.cone_profile_00d5debc;
    default: throw std::invalid_argument("Unknown current particle factory owner profile");
    }
}
} // namespace

void* create_native_particle_type_definition_00b00ce0(const void* kind,
    const void* name, void* parent, void* text,
    NativeParticleTypeFactoryRawContext& c, NativeParticleTypeFactoryRawAcquired& a) {
    using Phase = NativeParticleTypeFactoryRawAcquired::Phase;
    if (a.phase != Phase::fresh) throw std::logic_error("Particle factory operation cannot replay");
    a.phase = Phase::running;
    a.owner = parent;
    a.actual_text = text;
    using Construct = void* (*)(void*, const void*, Word, void*, NativeParticleTypeConstructionContext&);
    Construct construct = nullptr;
    Word bytes = 0, final_profile = 0, allocation_site = 0, constructor_site = 0;
    int state = -1;
    try {
        a.native_site = 0x00b00cfa;
        const char* const data = field<const char*>(kind, 4);
        a.native_site = 0x00b00d09;
        if (data && _stricmp(data, "SpriteParticle") == 0) {
            bytes = 0x90; state = 0; final_profile = 0x00d5dd18;
            allocation_site = 0x00b00d1f; constructor_site = 0x00b00d45;
            construct = &construct_native_sprite_particle_base_00b08830;
        } else if ((a.native_site = 0x00b00d66, equal_native_string_header_00425850(kind, "AxialParticle"))) {
            bytes = 0xa4; state = 1; final_profile = 0x00d5dcc0;
            allocation_site = 0x00b00d74; constructor_site = 0x00b00d9a;
            construct = &construct_native_axial_particle_base_00b058e0;
        } else if ((a.native_site = 0x00b00dca, equal_native_string_header_00425850(kind, "FloatingParticle"))) {
            bytes = 0x8c; state = 2;
            allocation_site = 0x00b00dd8; constructor_site = 0x00b00dfc;
            construct = &construct_native_floating_particle_definition_00b00770;
        } else if ((a.native_site = 0x00b00e17, equal_native_string_header_00425850(kind, "ObjectParticle"))) {
            bytes = 0x98; state = 3;
            allocation_site = 0x00b00e25; constructor_site = 0x00b00e49;
            construct = &construct_native_object_particle_definition_00af89e0;
        } else if ((a.native_site = 0x00b00e6f, equal_native_string_header_00425850(kind, "TracerParticle"))) {
            bytes = 0xe8; state = 4;
            allocation_site = 0x00b00e7d; constructor_site = 0x00b00ea1;
            construct = &construct_native_tracer_particle_definition_00b0a0b0;
        }
        if (construct) {
            a.native_site = allocation_site;
            a.allocation = singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
            a.unwind_state = state;
            if (a.allocation) {
                a.native_site = constructor_site;
                // The current parent word is read after the allocation call.
                a.owner = construct(a.allocation, name, field<Word>(parent, 0x10), parent, c.construction);
                if (final_profile) field<Word>(a.allocation) = final_profile;
            } else {
                a.owner = nullptr;
            }
            a.unwind_state = -1;
        }
        a.native_site = 0x00b00eba;
        a.captured_profile = field<Word>(a.owner);
        a.native_site = 0x00b00ebc;
        const volatile Word* const table = current_table(a.captured_profile, c);
        if (!table) throw std::invalid_argument("Missing current particle factory profile cells");
        a.captured_target = table[2];
        a.native_site = 0x00b00ec2;
        switch (a.captured_target) {
        case 0x00b08ac0:
            a.sprite_floating.emplace(a.initial_builder_kind);
            (void)load_native_sprite_particle_definition_00b08ac0(a.owner, text, c.sprite_floating, *a.sprite_floating);
            break;
        case 0x00b07d60:
            a.sprite_floating.emplace(a.initial_builder_kind);
            (void)load_native_floating_particle_definition_00b07d60(a.owner, text, c.sprite_floating, *a.sprite_floating);
            break;
        case 0x00b064a0:
            a.axial.emplace();
            (void)load_native_axial_particle_definition_00b064a0(a.owner, text, c.axial, *a.axial);
            break;
        case 0x00af8bd0:
            a.object.emplace(a.initial_builder_kind);
            (void)load_native_object_particle_definition_00af8bd0(a.owner, text, c.object, *a.object);
            break;
        case 0x00b0ad50:
            a.tracer.emplace(a.initial_builder_kind);
            (void)load_native_tracer_particle_definition_00b0ad50(a.owner, text, c.tracer, *a.tracer);
            break;
        default:
            throw std::invalid_argument("Unsupported current particle factory slot08 target (emitter slots create records)");
        }
        a.phase = Phase::complete;
        return a.owner;
    } catch (...) {
        a.native_state_at_failure = a.unwind_state;
        a.phase = Phase::failed;
        if (a.unwind_state >= 0) {
            // DF3440: every state goes directly to -1; callee cleanup has
            // already completed before its factory allocation is freed.
            a.unwind_state = -1;
            singleton_lifetime_free(a.allocation);
            a.allocation = nullptr;
        }
        throw;
    }
}
} // namespace bsp
