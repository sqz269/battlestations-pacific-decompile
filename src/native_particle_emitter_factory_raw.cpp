#include "bsp/native_particle_emitter_factory_raw.hpp"

#include "bsp/native_particle_cone_raw.hpp"
#include "bsp/native_particle_smartarea_raw.hpp"
#include "bsp/native_particle_sphere_raw.hpp"
#include "bsp/native_string_compare.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>
#include <optional>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle emitter factory requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);

Word word(const void* p) noexcept {
    return *static_cast<const volatile Word*>(p);
}

const void* at(const void* p, Word offset) noexcept {
    return reinterpret_cast<const void*>(reinterpret_cast<Word>(p) + offset);
}

using Constructor = void* (*)(void*, const void*, Word, Word, Word,
    NativeParticleEmitterConstructionContext&);

const volatile Word* current_table(Word profile,
    NativeParticleEmitterFactoryRawContext& c) {
    switch (profile) {
    case 0x00d5debcu: return c.cone_profile_00d5debc;
    case 0x00d5de88u: return c.sphere_profile_00d5de88;
    case 0x00d5de48u: return c.smartarea_profile_00d5de48;
    default: throw std::invalid_argument("Unknown current emitter factory parser profile");
    }
}
} // namespace

struct NativeParticleEmitterFactoryRawAcquired::Impl {
    explicit Impl(std::int32_t kind) : incoming_builder_kind(kind) {}
    const std::int32_t incoming_builder_kind;
    std::optional<NativeParticleSphereRawAcquired> sphere;
    std::optional<NativeParticleConeRawAcquired> cone;
    std::optional<NativeParticleSmartAreaRawAcquired> smartarea;
};

NativeParticleEmitterFactoryRawAcquired::NativeParticleEmitterFactoryRawAcquired(
    std::int32_t incoming_builder_kind)
    : impl_(std::make_unique<Impl>(incoming_builder_kind)) {}

NativeParticleEmitterFactoryRawAcquired::~NativeParticleEmitterFactoryRawAcquired() = default;

NativeParticleSphereRawAcquired* NativeParticleEmitterFactoryRawAcquired::sphere_child() noexcept {
    return impl_->sphere ? &*impl_->sphere : nullptr;
}

NativeParticleConeRawAcquired* NativeParticleEmitterFactoryRawAcquired::cone_child() noexcept {
    return impl_->cone ? &*impl_->cone : nullptr;
}

NativeParticleSmartAreaRawAcquired* NativeParticleEmitterFactoryRawAcquired::smartarea_child() noexcept {
    return impl_->smartarea ? &*impl_->smartarea : nullptr;
}

void* create_native_particle_definition_00af9fb0(const void* kind, const void* name,
    Word word10, Word word70, void* text, NativeParticleEmitterFactoryRawContext& c,
    NativeParticleEmitterFactoryRawAcquired& a) {
    using Phase = NativeParticleEmitterFactoryRawAcquired::Phase;
    if (a.phase != Phase::fresh)
        throw std::logic_error("Emitter factory operation cannot replay");
    a.phase = Phase::running;
    try {
        Constructor construct = nullptr;
        int construction_state = -1;
        Word allocation_site = 0;
        Word construction_site = 0;
        a.native_site = 0x00af9fcau;
        const char* const data = reinterpret_cast<const char*>(word(at(kind, 4)));
        a.native_site = 0x00af9fd9u;
        if (data && _stricmp(data, "ConeEmitter") == 0) {
            a.allocation_bytes = 0x94;
            construct = &construct_native_particle_cone_definition_00b03940;
            construction_state = 0;
            allocation_site = 0x00af9fef;
            construction_site = 0x00afa016;
        } else {
            a.native_site = 0x00afa031u;
            if (equal_native_string_header_00425850(kind, "SphereEmitter")) {
                a.allocation_bytes = 0x8c;
                construct = &construct_native_particle_sphere_definition_00b02b90;
                construction_state = 1;
                allocation_site = 0x00afa03f;
                construction_site = 0x00afa066;
            } else {
                a.native_site = 0x00afa08cu;
                if (equal_native_string_header_00425850(kind, "SmartAreaEmitter")) {
                    a.allocation_bytes = 0x90;
                    construct = &construct_native_particle_smartarea_definition_00b01cb0;
                    construction_state = 2;
                    allocation_site = 0x00afa09a;
                    construction_site = 0x00afa0c1;
                }
            }
        }

        if (construct) {
            a.native_site = allocation_site;
            a.allocation = singleton_lifetime_allocate({SingletonAllocationKind::object,
                a.allocation_bytes, a.allocation_bytes});
            a.owner = a.allocation;
            a.unwind_state = construction_state;
            if (a.allocation) {
                a.native_site = construction_site;
                a.owner = construct(a.allocation, name, word10, word70, 0, c.construction);
            }
            // Native null and successful constructor paths both disarm before
            // the owner/profile dereference. No parser-failure owner cleanup.
            a.unwind_state = -1;
        } else {
            a.native_site = 0x00afa0d2u;
            a.owner = reinterpret_cast<void*>(word70);
        }

        a.native_site = 0x00afa0dau;
        a.current_profile = word(a.owner);
        const volatile Word* const table = current_table(a.current_profile, c);
        a.native_site = 0x00afa0dcu;
        if (!table)
            throw std::invalid_argument("Missing current emitter factory profile cells");
        a.captured_parser = table[0x14 / 4];
        a.native_site = 0x00afa0e2u;
        switch (a.captured_parser) {
        case 0x00b02fd0u:
            if (!c.sphere)
                throw std::invalid_argument("Missing concrete Sphere emitter parser context");
            a.impl_->sphere.emplace(a.impl_->incoming_builder_kind);
            (void)load_native_sphere_emitter_definition_00b02fd0(
                a.owner, text, *c.sphere, *a.impl_->sphere);
            break;
        case 0x00b03ec0u:
            if (!c.cone)
                throw std::invalid_argument("Missing concrete Cone emitter parser context");
            a.impl_->cone.emplace(a.impl_->incoming_builder_kind);
            (void)load_native_cone_emitter_definition_00b03ec0(
                a.owner, text, *c.cone, *a.impl_->cone);
            break;
        case 0x00b02210u:
            if (!c.smartarea)
                throw std::invalid_argument("Missing concrete SmartArea emitter parser context");
            a.impl_->smartarea.emplace(a.impl_->incoming_builder_kind);
            (void)load_native_smartarea_emitter_definition_00b02210(
                a.owner, text, *c.smartarea, *a.impl_->smartarea);
            break;
        default:
            throw std::invalid_argument("Unknown captured emitter factory slot14 target");
        }
        a.native_site = 0x00afa0f6u;
        a.phase = Phase::complete;
        return a.owner;
    } catch (...) {
        // DF2E6C: all three factory unwind states free the captured raw block.
        // The real constructor has already completed its own cleanup here.
        if (a.unwind_state >= 0) {
            a.unwind_state = -1;
            singleton_lifetime_free(a.allocation);
            a.allocation_released = true;
        }
        a.phase = Phase::failed;
        throw;
    }
}
} // namespace bsp
