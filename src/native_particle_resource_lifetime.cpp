#include "bsp/native_particle_resource_lifetime.hpp"
#include "bsp/native_particle_layer_lifetime.hpp"
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
void release_child(void* child, NativeStringRawPoolContext& strings) {
    if (InterlockedDecrement(static_cast<volatile LONG*>(at(child, 4))) == 0) {
        const U terminal_profile = field<U>(child, 0);
        if (terminal_profile == 0x00d5dc38) {
            // Proven Layer slot0 is BD30E0: reload the current table and call
            // its scalar slot4 with flags1, without a second decrement.
            const U scalar_profile = field<U>(child, 0);
            if (scalar_profile == 0x00d5dc38)
                scalar_delete_native_particle_layer_00aface0(child, 1, strings);
            else {
                using Scalar = void* (__thiscall*)(void*, U);
                field<Scalar>(reinterpret_cast<void*>(scalar_profile), 4)(child, 1);
            }
            return;
        }
        using Terminal = void (__thiscall*)(void*);
        auto* const table = reinterpret_cast<void*>(terminal_profile);
        const auto target = field<Terminal>(table, 0);
        target(child);
    }
}
}

void destroy_native_particle_resource_00af4280(void* resource, NativeStringRawPoolContext& strings) {
    field<U>(resource, 0) = 0x00d5d958;
    const I initial_emitters = field<I>(resource, 0x30);
    ResourceUnwind unwind{resource, strings};
    if (initial_emitters > 0) {
        U index = 0;
        auto* cursor = at(resource, 0x10);
        do {
            void* const child = field<void*>(cursor, 0);
            release_child(child, strings);
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
            release_child(child, strings);
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

void* delete_native_particle_resource_00af46e0(void* resource, U flags, NativeStringRawPoolContext& strings) {
    destroy_native_particle_resource_00af4280(resource, strings);
    if ((flags & 1u) != 0) singleton_lifetime_free(resource);
    return resource;
}
}
