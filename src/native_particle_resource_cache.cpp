#include "bsp/native_particle_resource_cache.hpp"

#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_particle_resource_records.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle resource cache reconstruction requires MSVC Win32.
#endif

namespace bsp {
namespace {
void* at(const void* base, std::uint32_t offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
volatile std::uint32_t& word(const void* base, std::uint32_t offset = 0) noexcept {
    return *static_cast<volatile std::uint32_t*>(at(base, offset));
}
void* pointer(const void* base, std::uint32_t offset = 0) noexcept {
    return reinterpret_cast<void*>(word(base, offset));
}

// These destructors model true FH3 unwind actions. In particular they are
// noexcept: a cleanup exception during an active exception must terminate,
// rather than replace the first exception as an explicit catch could do.
struct VectorUnwind {
    void* vector;
    NativeStringRawPoolContext& strings;
    bool armed = true;
    ~VectorUnwind() noexcept {
        if (armed) destroy_native_particle_resource_records_00871370(vector, strings);
    }
};
struct BaseUnwind {
    void* owner;
    NativeParticleResourceCacheContext& context;
    bool armed = true;
    ~BaseUnwind() noexcept {
        if (armed) destroy_native_particle_resource_cache_base_0086a200(owner, context);
    }
};
struct SingletonGuardUnwind {
    std::uint32_t guard[2];
    bool armed = true;
    ~SingletonGuardUnwind() noexcept {
        if (armed) destroy_native_singleton_guard_00411ee0(guard);
    }
};
} // namespace

void* retain_native_particle_resource_00871400(void* resource) {
    InterlockedIncrement(static_cast<volatile LONG*>(at(resource, 4)));
    return resource;
}

void release_native_particle_resource_00871420(void* resource) {
    if (InterlockedDecrement(static_cast<volatile LONG*>(at(resource, 4))) == 0) {
        const std::uint32_t target = word(pointer(resource));
        reinterpret_cast<void (__thiscall*)(void*)>(target)(resource);
    }
}

void* construct_native_particle_resource_cache_key_0086ba10(
    void*, void* output, const void* source, const void*,
    NativeStringRawPoolContext& strings) {
    word(output) = 0;
    word(output, 4) = 0;
    if (output != source) {
        resize_native_string_header_0041dd40(output, strings, word(source), true);
        if (word(source) != 0) {
            const std::uint32_t count = word(output);
            const void* const current_source = pointer(source, 4);
            void* const current_output = pointer(output, 4);
            // Native BF7680 includes its overlapping-range branch.
            if (count != 0) std::memmove(current_output, current_source, count);
        }
    }
    return output;
}

void destroy_native_particle_resource_records_00871370(
    void* vector, NativeStringRawPoolContext& strings) {
    resize_native_particle_resource_records_00870b30(vector, 0, strings);
    singleton_lifetime_free(pointer(vector));
}

void destroy_native_particle_resource_cache_00871480(
    void* inner, NativeStringRawPoolContext& strings) {
    word(inner) = 0x00d0daf0u;
    VectorUnwind cleanup{at(inner, 4), strings};
    clear_native_particle_resource_cache_00871310(inner, strings);
    cleanup.armed = false;
    destroy_native_particle_resource_records_00871370(at(inner, 4), strings);
}

void* delete_native_particle_resource_cache_00871730(
    void* inner, std::uint32_t flags, NativeStringRawPoolContext& strings) {
    destroy_native_particle_resource_cache_00871480(inner, strings);
    if ((flags & 1u) != 0) singleton_lifetime_free(inner);
    return inner;
}

void destroy_native_particle_resource_cache_base_0086a200(
    void* outer, NativeParticleResourceCacheContext& context) noexcept {
    context.actual_cache_publication_00f87668 = nullptr;
    word(outer) = 0x00ce3818u;
}

void destroy_native_particle_resource_cache_owner_00871ae0(
    void* outer, NativeParticleResourceCacheContext& context,
    NativeStringRawPoolContext& strings) {
    BaseUnwind cleanup{outer, context};
    destroy_native_particle_resource_cache_00871480(at(outer, 4), strings);
    destroy_native_particle_resource_cache_base_0086a200(outer, context);
    cleanup.armed = false;
}

void* delete_native_particle_resource_cache_owner_00871b30(
    void* outer, std::uint32_t flags, NativeParticleResourceCacheContext& context,
    NativeStringRawPoolContext& strings) {
    destroy_native_particle_resource_cache_owner_00871ae0(outer, context, strings);
    if ((flags & 1u) != 0) singleton_lifetime_free(outer);
    return outer;
}

void* get_native_particle_resource_cache_owner_00871bd0(
    NativeParticleResourceCacheContext& context) {
    void* const initial = context.actual_cache_publication_00f87668;
    if (initial) return initial;

    void* const manager = get_native_singleton_manager_00415350(
        context.actual_manager_publication_01090aa0);
    auto* const captured = static_cast<CRITICAL_SECTION*>(pointer(manager, 0x10));
    if (captured) {
        EnterCriticalSection(captured);
        word(captured, 0x18) = word(captured, 0x18) + 1u;
    }
    SingletonGuardUnwind cleanup{{0x00ce37fcu,
        reinterpret_cast<std::uint32_t>(captured)}};
    if (!context.actual_cache_publication_00f87668) {
        void* const allocation = singleton_lifetime_allocate({
            SingletonAllocationKind::object, 0x18, 0x18});
        if (allocation) {
            word(allocation) = 0x00d0db54u;
            word(allocation, 8) = 0;
            word(allocation, 0x0c) = 0;
            word(allocation, 0x10) = 0;
            word(allocation, 0x14) = 0;
            word(allocation, 4) = 0x00d0db40u;
        }
        context.actual_cache_publication_00f87668 = allocation;
        void* const current_manager = get_native_singleton_manager_00415350(
            context.actual_manager_publication_01090aa0);
        register_native_singleton_object_00bd0c30(current_manager, nullptr,
            context.actual_cache_publication_00f87668);
    }
    if (captured) {
        word(captured, 0x18) = word(captured, 0x18) - 1u;
        LeaveCriticalSection(captured);
    }
    cleanup.armed = false;
    return context.actual_cache_publication_00f87668;
}
} // namespace bsp
