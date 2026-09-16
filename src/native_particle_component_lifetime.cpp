#include "bsp/native_particle_component_lifetime.hpp"
#include "bsp/native_particle_resource_cache.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Particle component lifetime requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
using Signed = std::int32_t;
static_assert(sizeof(void*) == 4);
Word address(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
Word read(const void* p, Word offset = 0) noexcept {
    Word value;
    std::memcpy(&value, pointer(address(p) + offset), sizeof value);
    return value;
}
void write(void* p, Word offset, Word value) noexcept {
    std::memcpy(pointer(address(p) + offset), &value, sizeof value);
}
void* member(void* p, Word offset) noexcept { return pointer(address(p) + offset); }

struct BaseCleanup {
    void* owner;
    ~BaseCleanup() noexcept { destroy_native_ref_counted_base_00bd30f0(owner); }
};
struct ParticleCleanup {
    void* owner;
    NativeStringRawPoolContext& strings;
    int state = 2;
    // Invoked only on an escaping C++ exception: another cleanup exception
    // terminates, rather than replacing the first or continuing lower states.
    ~ParticleCleanup() noexcept {
        if (state >= 2) destroy_native_particle_component_resources_0086b6c0(member(owner, 0x28));
        if (state >= 1) destroy_native_string_header_0041dd20(member(owner, 0x20), strings);
        if (state >= 0) destroy_effect_component_base_0086b7e0(owner, strings);
    }
};
} // namespace

void reserve_native_particle_component_resources_0086a4d0(void* header, Signed requested) {
    const Signed capacity = requested < 1 ? 1 : requested;
    if (static_cast<Signed>(read(header, 8)) >= capacity) return;
    const Word bytes = static_cast<Word>(capacity) * 4u;
    const Word replacement = address(singleton_lifetime_allocate(
        {SingletonAllocationKind::pointer_slots, bytes, bytes}));
    Word destination = replacement;
    for (Word i = 0; static_cast<Signed>(i) < static_cast<Signed>(read(header, 4));
         ++i, destination += 4u) {
        if (destination) write(pointer(destination), 0, read(pointer(read(header) + i * 4u)));
    }
    singleton_lifetime_free(pointer(read(header)));
    write(header, 0, replacement);
    write(header, 8, static_cast<Word>(capacity));
}

void resize_native_particle_component_resources_0086abb0(void* header, Signed requested) {
    if (requested > static_cast<Signed>(read(header, 8)))
        reserve_native_particle_component_resources_0086a4d0(header, requested);
    for (Word i = read(header, 4); static_cast<Signed>(i) < requested; ++i) {
        const Word destination = read(header) + i * 4u;
        if (destination) write(pointer(destination), 0, 0);
    }
    while (requested < static_cast<Signed>(read(header, 4)))
        write(header, 4, read(header, 4) - 1u);
    write(header, 4, static_cast<Word>(requested));
}

void destroy_native_particle_component_resources_0086b6c0(void* header) {
    resize_native_particle_component_resources_0086abb0(header, 0);
    singleton_lifetime_free(pointer(read(header)));
}

void destroy_effect_component_base_0086b7e0(void* component, NativeStringRawPoolContext& strings) {
    write(component, 0, 0x00d0d570);
    BaseCleanup base{component};
    destroy_native_string_header_0041dd20(member(component, 8), strings);
}

static void destroy_component_with_context(void* component, NativeStringRawPoolContext& strings, NativeParticleResourceCacheContext* context) {
    write(component, 0, 0x00d0d5b4);
    ParticleCleanup cleanup{component, strings};
    while (read(component, 0x2c) != 0) {
        void* const resource = pointer(read(pointer(
            read(component, 0x28) + read(component, 0x2c) * 4u - 4u)));
        if (context) release_native_particle_resource_00871420(resource, *context, strings);
        else if (InterlockedDecrement(reinterpret_cast<volatile LONG*>(member(resource, 4))) == 0) {
            using ZeroReferences = void (__thiscall*)(void*);
            const auto current_slot = reinterpret_cast<ZeroReferences>(read(pointer(read(resource))));
            current_slot(resource);
        }
        const Word current_count = read(component, 0x2c);
        if (current_count != 0) write(component, 0x2c, current_count - 1u);
    }
    cleanup.state = 1;
    destroy_native_particle_component_resources_0086b6c0(member(component, 0x28));
    cleanup.state = 0;
    destroy_native_string_header_0041dd20(member(component, 0x20), strings);
    cleanup.state = -1;
    destroy_effect_component_base_0086b7e0(component, strings);
}

void* scalar_delete_native_particle_component_0086bc60(void* component, Word flags,
    NativeStringRawPoolContext& strings) {
    destroy_native_particle_component_0086bb80(component, strings);
    if (flags & 1u) singleton_lifetime_free(component);
    return component;
}

void destroy_native_particle_component_0086bb80(void* component, NativeStringRawPoolContext& strings) {
    destroy_component_with_context(component, strings, nullptr);
}
void destroy_native_particle_component_0086bb80(void* component,
    NativeParticleResourceCacheContext& context, NativeStringRawPoolContext& strings) {
    destroy_component_with_context(component, strings, &context);
}
void* scalar_delete_native_particle_component_0086bc60(void* component, Word flags,
    NativeParticleResourceCacheContext& context, NativeStringRawPoolContext& strings) {
    destroy_native_particle_component_0086bb80(component, context, strings);
    if ((flags & 1u) != 0) singleton_lifetime_free(component);
    return component;
}

} // namespace bsp
