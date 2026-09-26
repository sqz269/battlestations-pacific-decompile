#include "bsp/native_texture_source_virtual_leaves.hpp"
#include "bsp/native_resource_cache_leaves.hpp"
#include "bsp/singleton_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native texture-source virtual leaves require MSVC Win32.
#endif

namespace bsp {

void* construct_native_texture_source_base_00c30210(void* actual_storage) noexcept {
    void* const captured = actual_storage;
    construct_native_texture_resource_base_00b19980(captured);
    *static_cast<volatile std::uint32_t*>(captured) = 0x00d79aec;
    return captured;
}

void* delete_native_texture_source_base_00c30270(
    void* actual_owner, std::uint32_t flags) noexcept {
    void* const captured = actual_owner;
    destroy_native_procedural_resource_base_00b19750(captured);
    if (flags & 1u) singleton_lifetime_free(captured);
    return captured;
}

void* delete_native_texture_source_00c30550(
    void* actual_owner, std::uint32_t flags, NativeRenderActualOwners& owners,
    NativeProceduralResourceLifetimeOperation& operation) {
    void* const captured = actual_owner;
    destroy_native_procedural_resource_00c304a0(captured, owners, operation);
    if (flags & 1u) {
        singleton_lifetime_free(captured);
        operation.owner_free_returned = true;
    }
    return captured;
}

} // namespace bsp
