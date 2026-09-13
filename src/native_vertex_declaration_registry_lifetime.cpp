#include "bsp/native_vertex_declaration_registry_lifetime.hpp"

#include "bsp/native_render_context.hpp"
#include "bsp/native_render_resource_record_construction.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_vertex_declaration_cache.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native declaration registry lifetime requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(NativeRenderResourceRecord) == 0x2c);
static_assert(offsetof(NativeRenderResourceRecord, resource_28) == 0x28);
template<class T> volatile T& field(void* p, std::size_t offset) {
    return *reinterpret_cast<volatile T*>(static_cast<unsigned char*>(p) + offset);
}
void* plus(void* p, std::uint32_t offset) {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
std::int32_t signed_bits(std::uint32_t value) {
    std::int32_t result;
    std::memcpy(&result, &value, sizeof result);
    return result;
}
void require_release_slot(void* registry,
    NativeVertexDeclarationRegistryLifetimeContext& c) {
    const auto profile = field<std::uint32_t>(registry, 0);
    const volatile std::uint32_t* table = nullptr;
    if (profile == 0x00d5f024) table = c.base_vtable_00d5f024;
    else if (profile == 0x00d5f060) table = c.cache.registry_vtable_00d5f060;
    if (!table || table[4] != 0x00b31d40)
        throw std::invalid_argument("unimplemented current declaration registry release slot");
}
} // namespace

void initialize_native_declaration_registry_00b32534_fragment(void* registry) noexcept {
    field<void*>(registry, 4) = nullptr;
    field<std::uint32_t>(registry, 8) = 0;
    field<std::uint32_t>(registry, 0xc) = 0;
    field<std::uint32_t>(registry, 0x10) = 0;
    field<std::uint32_t>(registry, 0) = 0x00d5f060;
}

void resize_native_declaration_records_00b30270(void* header, std::uint32_t requested,
    NativeVertexDeclarationCacheContext& c) {
    if (signed_bits(requested) > signed_bits(field<std::uint32_t>(header, 8)))
        reserve_native_declaration_records_00b2fe20(header, requested, c);
    auto i = field<std::uint32_t>(header, 4);
    while (signed_bits(i) < signed_bits(requested)) {
        auto* const p = plus(field<void*>(header, 0), i * 0x2cu);
        if (p != nullptr) {
            auto* const record = ::new (p) NativeRenderResourceRecord;
            volatile auto& current = *record;
            current.name_length_00 = 0;
            current.name_data_04 = nullptr;
            try {
                current.sentinel_0c = allocate_native_render_alias_sentinel_004c3020();
            } catch (...) {
                // DF6360 state1 -> CBDA79, then state0 -> no-op00401130.
                // No cleanup of previously constructed rows or current vector.
                destroy_native_string_header_0041dd20(record, c.strings);
                throw;
            }
            current.alias_count_10 = 0;
            current.payload_14_24[4] = 0;
            current.payload_14_24[3] = 0;
            current.payload_14_24[2] = 0;
            current.payload_14_24[1] = 0;
            current.payload_14_24[0] = 0;
        }
        ++i;
    }
    while (signed_bits(requested) < signed_bits(field<std::uint32_t>(header, 4))) {
        field<std::uint32_t>(header, 4) = field<std::uint32_t>(header, 4) - 1u;
        const auto index = field<std::uint32_t>(header, 4);
        auto* const data = field<void*>(header, 0);
        auto* const record = static_cast<NativeRenderResourceRecord*>(plus(
            data, index * 0x2cu));
        destroy_native_declaration_record_00b2f910(*record, c.strings);
    }
    field<std::uint32_t>(header, 4) = requested;
}

void destroy_native_declaration_records_00b316a0(void* header,
    NativeVertexDeclarationCacheContext& c) {
    resize_native_declaration_records_00b30270(header, 0, c);
    c.free_array_00bf6989(field<void*>(header, 0));
}

void release_native_cached_declaration_00b31d40(void* declaration,
    NativeRenderActualOwners& owners) {
    release_native_render_actual_owner(owners, declaration);
}

void flush_native_declaration_registry_00b31630(void* registry,
    NativeVertexDeclarationRegistryLifetimeContext& c) {
    while (field<std::uint32_t>(registry, 8) != 0) {
        auto count = field<std::uint32_t>(registry, 8);
        auto* data = field<void*>(registry, 4);
        auto* declaration = field<void*>(plus(data, count * 0x2cu - 4u), 0);
        const auto stride = field<std::uint32_t>(declaration, 0xcc);
        field<std::uint32_t>(registry, 0x10) = field<std::uint32_t>(registry, 0x10) - stride;
        count = field<std::uint32_t>(registry, 8);
        data = field<void*>(registry, 4);
        declaration = field<void*>(plus(data, count * 0x2cu - 4u), 0);
        require_release_slot(registry, c);
        release_native_cached_declaration_00b31d40(declaration, c.owners);
        count = field<std::uint32_t>(registry, 8);
        if (count != 0) {
            auto* const record = static_cast<NativeRenderResourceRecord*>(plus(
                field<void*>(registry, 4), count * 0x2cu - 0x2cu));
            destroy_native_declaration_record_00b2f910(*record, c.cache.strings);
            field<std::uint32_t>(registry, 8) = field<std::uint32_t>(registry, 8) - 1u;
        }
    }
    resize_native_declaration_records_00b30270(plus(registry, 4), 0, c.cache);
}

void destroy_native_declaration_registry_00b32030(void* registry,
    NativeVertexDeclarationRegistryLifetimeContext& c) {
    field<std::uint32_t>(registry, 0) = 0x00d5f024;
    try {
        flush_native_declaration_registry_00b31630(registry, c);
    } catch (...) {
        destroy_native_declaration_records_00b316a0(plus(registry, 4), c.cache);
        throw;
    }
    // Native state -1 before B30270: a later failure must not repeat cleanup.
    destroy_native_declaration_records_00b316a0(plus(registry, 4), c.cache);
}

void* delete_native_declaration_registry_00b32210(void* registry,
    std::uint32_t flags, NativeVertexDeclarationRegistryLifetimeContext& c) {
    destroy_native_declaration_registry_00b32030(registry, c);
    if (flags & 1u) singleton_lifetime_free(registry);
    return registry;
}
} // namespace bsp
