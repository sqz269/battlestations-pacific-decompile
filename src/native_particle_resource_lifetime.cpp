#include "bsp/native_particle_resource_lifetime.hpp"
#include "bsp/native_particle_layer_lifetime.hpp"
#include "bsp/native_particle_type_lifetime.hpp"
#include "bsp/native_particle_emitter_definition_lifetime.hpp"
#include "bsp/native_particle_emitter_derived_lifetimes.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle resource lifetime requires MSVC Win32.
#endif

namespace bsp {
namespace {
using U = std::uint32_t;
using I = std::int32_t;
void* at(const void* p, U offset) { return reinterpret_cast<void*>(reinterpret_cast<U>(p) + offset); }
template<class T> volatile T& field(const void* p, U offset) { return *static_cast<volatile T*>(at(p, offset)); }
struct ResourceUnwind {
    void* resource;
    NativeStringRawPoolContext& strings;
    int state = 1;
    ~ResourceUnwind() noexcept {
        if (state >= 1) destroy_native_string_header_0041dd20(at(resource, 8), strings);
        if (state >= 0) destroy_native_ref_counted_base_00bd30f0(resource);
    }
};
NativeStringRawPoolContext& resource_strings(NativeStringRawPoolContext& strings) { return strings; }
NativeStringRawPoolContext& resource_strings(NativeParticleTypeLifetimeContext& context) { return context.strings; }
bool known_terminal(U profile, NativeStringRawPoolContext&) noexcept { return profile == 0x00d5dc38; }
bool known_terminal(U profile, NativeParticleTypeLifetimeContext&) noexcept {
    switch (profile) {
    case 0x00d5dc38: case 0x00d5dbc4: case 0x00d5debc: case 0x00d5de88: case 0x00d5de48: return true;
    default: return false;
    }
}
bool dispatch_emitter_scalar(void*, U, NativeStringRawPoolContext&) { return false; }
bool dispatch_emitter_scalar(void* child, U profile, NativeParticleTypeLifetimeContext& context) {
    switch (profile) {
    case 0x00d5dbc4: delete_native_particle_definition_00afa350(child, 1, context); return true;
    case 0x00d5debc: delete_native_particle_cone_definition_00b03b40(child, 1, context); return true;
    case 0x00d5de88: delete_native_particle_sphere_definition_00b02fb0(child, 1, context); return true;
    case 0x00d5de48: delete_native_particle_smartarea_definition_00b01ea0(child, 1, context); return true;
    default: return false;
    }
}
template<class Context>
void release_child(void* child, Context& context) {
    if (InterlockedDecrement(static_cast<volatile LONG*>(at(child, 4))) == 0) {
        const U terminal_profile = field<U>(child, 0);
        if (known_terminal(terminal_profile, context)) {
            // Proven slot0 is BD30E0: reload CURRENT table and invoke slot4
            // with flags1. The actual reference count is decremented once.
            const U scalar_profile = field<U>(child, 0);
            if (scalar_profile == 0x00d5dc38)
                scalar_delete_native_particle_layer_00aface0(child, 1, resource_strings(context));
            else if (!dispatch_emitter_scalar(child, scalar_profile, context)) {
                using Scalar = void* (__thiscall*)(void*, U);
                field<Scalar>(reinterpret_cast<void*>(scalar_profile), 4)(child, 1);
            }
            return;
        }
        using Terminal = void (__thiscall*)(void*);
        const auto target = field<Terminal>(reinterpret_cast<void*>(terminal_profile), 0);
        target(child);
    }
}
}

template<class Context>
void destroy_resource(void* resource, Context& context) {
    auto& strings = resource_strings(context);
    field<U>(resource, 0) = 0x00d5d958;
    const I initial_emitters = field<I>(resource, 0x30);
    ResourceUnwind unwind{resource, strings};
    if (initial_emitters > 0) {
        U index = 0;
        auto* cursor = at(resource, 0x10);
        do {
            void* const child = field<void*>(cursor, 0);
            release_child(child, context);
            ++index;
            cursor = at(cursor, 4);
        } while (static_cast<I>(index) < field<I>(resource, 0x30));
    }
    const I initial_layers = field<I>(resource, 0x54); // Before count30 store.
    field<U>(resource, 0x30) = 0;
    if (initial_layers > 0) {
        U index = 0;
        auto* cursor = at(resource, 0x34);
        do {
            void* const child = field<void*>(cursor, 0);
            release_child(child, context);
            ++index;
            cursor = at(cursor, 4);
        } while (static_cast<I>(index) < field<I>(resource, 0x54));
    }
    field<U>(resource, 0x54) = 0;
    void* const data = field<void*>(resource, 0x0c);
    unwind.state = 0; // Captured data precedes state transitionAF4321.
    if (data != nullptr) {
        const U size = field<U>(resource, 8) + 1u;
        auto* const pool = native_string_pool_get_or_create_00419cc0(
            strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
        return_native_string_pool_00bd1510(pool, data, size, strings.actual_small_returns_disabled_01090aa4);
    }
    unwind.state = -1;
    destroy_native_ref_counted_base_00bd30f0(resource);
}

void destroy_native_particle_resource_00af4280(void* resource, NativeStringRawPoolContext& strings) {
    destroy_resource(resource, strings);
}
void destroy_native_particle_resource_00af4280(void* resource, NativeParticleTypeLifetimeContext& context) {
    destroy_resource(resource, context);
}
void* delete_native_particle_resource_00af46e0(void* resource, U flags, NativeParticleTypeLifetimeContext& context) {
    destroy_resource(resource, context);
    if ((flags & 1u) != 0) singleton_lifetime_free(resource);
    return resource;
}
void* delete_native_particle_resource_00af46e0(void* resource, U flags, NativeStringRawPoolContext& strings) {
    destroy_native_particle_resource_00af4280(resource, strings);
    if ((flags & 1u) != 0) singleton_lifetime_free(resource);
    return resource;
}
}
