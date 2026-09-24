#include "bsp/native_procedural_factory_storage.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw procedural factory storage requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
Word read(const void* p, Word byte_offset = 0) noexcept {
    Word result;
    __asm {
        mov eax, p
        add eax, byte_offset
        mov ecx, [eax]
        mov result, ecx
    }
    return result;
}
void stamp(void* p, Word profile) noexcept {
    __asm {
        mov eax, p
        mov ecx, profile
        mov [eax], ecx
    }
}
Word bits(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }

const void* registry_table(Word profile,
    NativeProceduralFactoryStorageContext& context,
    NativeProceduralFactoryStorageAcquired& acquired) {
    switch (profile) {
    case 0x00d5e594u: return context.actual_registry_profile_00d5e594;
    case 0x00d5e59cu: return context.actual_registry_profile_00d5e59c;
    default:
        acquired.unsupported_binding = true;
        throw std::invalid_argument("unsupported actual resource registry profile");
    }
}

void* construct(void* factory, Word profile,
    const NativeProceduralFactoryStorageFrame& frame,
    NativeProceduralFactoryStorageContext& context,
    NativeProceduralFactoryStorageAcquired& acquired) {
    if (acquired.started || acquired.registration.started)
        throw std::logic_error("procedural factory acquisition is not fresh");
    acquired.started = true;
    frame.cleanup_self = bits(factory); // native PUSH ECX
    frame.cleanup_self = bits(factory); // MOV [ESP+4], captured ESI
    acquired.native_eh_state = 0;
    try {
        stamp(factory, profile);
        acquired.getter_entered = true;
        void* const registry = get_native_resource_registry_00b1b730(
            context.actual_manager_publication_01090aa0,
            context.actual_registry_publication_00f8d41c); // BBC61B / BBC76B
        acquired.getter_returned = true;
        acquired.returned_registry = registry;
        const Word name = frame.name_argument; // AFTER genuine getter
        acquired.captured_name = name;
        const Word current_profile = read(registry);
        acquired.captured_registry_profile = current_profile;
        const void* const table = registry_table(current_profile, context, acquired);
        const Word target = read(table, 4);
        acquired.captured_registration_target = target;
        if (target != 0x00b1b3a0u) {
            acquired.unsupported_binding = true;
            throw std::invalid_argument("unsupported current resource registry registration slot");
        }
        frame.registration.factory_argument = bits(factory); // PUSH ESI
        frame.registration.name_argument = name; // PUSH captured ECX
        acquired.registration_entered = true;
        register_native_resource_factory_00b1b3a0(registry, frame.registration,
            context.insertion, acquired.registration); // exact BBC62D / BBC77D current target
        acquired.constructor_completed = true;
        return factory;
    } catch (...) {
        acquired.source_failed = true;
        acquired.native_eh_state = -1; // consume state0 exactly once
        acquired.cleanup_started = true;
        void* const current_self = pointer(frame.cleanup_self); // CC4B50 / CC4B90
        acquired.cleanup_receiver = current_self;
        destroy_native_procedural_factory_base_00bbc440(current_self);
        acquired.cleanup_completed = true;
        throw;
    }
}

void* scalar_delete(void* factory, const volatile Word& flags) {
    const bool release = (*reinterpret_cast<const volatile unsigned char*>(&flags) & 1u) != 0;
    destroy_native_procedural_factory_base_00bbc440(factory);
    if (release) singleton_lifetime_free(factory); // BBC661 / BBC7B1, actual CRT domain
    return factory; // identity only; no payload access after free
}
} // namespace

void* construct_native_caustics_factory_00bbc5f0(void* factory,
    const NativeProceduralFactoryStorageFrame& frame,
    NativeProceduralFactoryStorageContext& context,
    NativeProceduralFactoryStorageAcquired& acquired) {
    return construct(factory, 0x00d64470u, frame, context, acquired);
}
void* construct_native_shore_wave_factory_00bbc740(void* factory,
    const NativeProceduralFactoryStorageFrame& frame,
    NativeProceduralFactoryStorageContext& context,
    NativeProceduralFactoryStorageAcquired& acquired) {
    return construct(factory, 0x00d644acu, frame, context, acquired);
}
void destroy_native_procedural_factory_base_00bbc440(void* factory) noexcept {
    stamp(factory, 0x00d64468u);
}
void* delete_native_caustics_factory_00bbc650(void* factory, const volatile Word& flags) {
    return scalar_delete(factory, flags);
}
void* delete_native_shore_wave_factory_00bbc7a0(void* factory, const volatile Word& flags) {
    return scalar_delete(factory, flags);
}
} // namespace bsp
