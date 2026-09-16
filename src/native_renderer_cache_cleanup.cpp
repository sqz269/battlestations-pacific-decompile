#include "bsp/native_renderer_cache_cleanup.hpp"
#include "bsp/native_material_effect_cache.hpp"
#include "bsp/native_render_context.hpp"
#include "bsp/native_render_resource_container_removal.hpp"
#include "bsp/native_render_resource_record_array.hpp"
#include "bsp/native_render_resource_record_construction.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeRenderResourceRecord) == 0x2c);
template<class T> volatile T& field(void* base, std::uint32_t offset) noexcept {
    return *reinterpret_cast<volatile T*>(reinterpret_cast<std::uint32_t>(base) + offset);
}
void* plus(void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uint32_t>(base) + offset);
}
std::int32_t signed_bits(std::uint32_t value) noexcept {
    return static_cast<std::int32_t>(value);
}
NativeRenderResourceRecord* current_record(void* header, std::uint32_t index) noexcept {
    return static_cast<NativeRenderResourceRecord*>(plus(field<void*>(header, 0), index * 0x2cu));
}
void* current_last_child(void* cache) noexcept {
    const auto count = field<std::uint32_t>(cache, 8);
    void* const data = field<void*>(cache, 4);
    return field<void*>(plus(data, count * 0x2cu - 4u), 0);
}
std::uint32_t current_child_size(void* child, NativeRendererCacheCleanupContext& c) {
    const auto profile = field<std::uint32_t>(child, 0);
    const volatile std::uint32_t* table = nullptr;
    switch (profile) {
    case 0x00d61948: table = c.accounting_tables.texture_2d_00d61948; break;
    case 0x00d61870: table = c.accounting_tables.texture_cube_00d61870; break;
    case 0x00d618b0: table = c.accounting_tables.texture_volume_00d618b0; break;
    default: break;
    }
    const auto expected = profile == 0x00d61948u ? 0x00b3ce30u : 0x00a82250u;
    if (table == nullptr || table[3] != expected)
        throw std::invalid_argument("unimplemented current cache child size profile");
    return expected == 0x00b3ce30u ? native_resource_accounted_size_00b3ce30(child) :
        native_resource_zero_accounted_size_00a82250();
}
void current_cache_release(void* cache, void* captured_child, NativeRendererCacheCleanupContext& c) {
    const auto profile = field<std::uint32_t>(cache, 0);
    const volatile std::uint32_t* table = nullptr;
    if (profile == 0x00d5f038u) table = c.cache_base_vtable_00d5f038;
    if (profile == 0x00d5f088u) table = c.texture_cache_vtable_00d5f088;
    if (table == nullptr || table[4] != 0x00b31da0u)
        throw std::invalid_argument("unimplemented current cache release profile");
    // B31DA0: original incoming ECX is unused, stack captured resource, RET4.
    // The existing provider decrements actual+04 BEFORE resolving its canonical
    // companion, whose terminal dispatches the current actual profile at zero.
    release_native_render_actual_owner(c.owners, captured_child);
}
struct BaseUnwind {
    void* cache;
    NativeRendererCacheCleanupContext& context;
    bool armed = true;
    ~BaseUnwind() noexcept {
        if (armed) destroy_native_renderer_resource_array_00b31730(plus(cache, 4), context);
    }
};
struct TextureUnwind {
    void* cache;
    NativeRendererCacheCleanupContext& context;
    int state = 1;
    ~TextureUnwind() noexcept {
        if (state >= 1) destroy_native_string_header_0041dd20(plus(cache, 0x14), context.strings);
        if (state >= 0) destroy_native_renderer_resource_cache_00b32090(cache, context);
    }
};
} // namespace

void resize_native_effect_records_00b30410(void* header, std::uint32_t requested,
    NativeMaterialEffectCacheContext& c) {
    if (signed_bits(requested) > signed_bits(field<std::uint32_t>(header, 8)))
        reserve_native_effect_records_00b2ffe0(header, requested, c);
    auto index = field<std::uint32_t>(header, 4);
    while (signed_bits(index) < signed_bits(requested)) {
        auto* row = current_record(header, index);
        if (row != nullptr) {
            row = ::new (row) NativeRenderResourceRecord;
            field<std::uint32_t>(row, 0) = 0;
            field<void*>(row, 4) = nullptr;
            try {
                auto* const sentinel = allocate_native_render_alias_sentinel_004c3020();
                field<NativeRenderResourceAliasNode*>(row, 0x0c) = sentinel;
                field<std::uint32_t>(row, 0x10) = 0;
                for (std::uint32_t offset = 0x24; offset >= 0x14; offset -= 4)
                    field<std::uint32_t>(row, offset) = 0;
            } catch (...) {
                destroy_native_string_header_0041dd20(row, c.strings);
                // State0 CBDAC0 computes the current array slot for RET-only
                // placement delete401130; retain the current header read.
                (void)current_record(header, index);
                throw;
            }
        }
        ++index;
    }
    while (signed_bits(requested) < signed_bits(field<std::uint32_t>(header, 4))) {
        field<std::uint32_t>(header, 4) = field<std::uint32_t>(header, 4) - 1u;
        const auto retained = field<std::uint32_t>(header, 4);
        destroy_native_effect_record_00b2fa10(*current_record(header, retained), c.strings);
    }
    field<std::uint32_t>(header, 4) = requested;
}

void clear_native_renderer_resource_cache_00b316c0(void* cache, NativeRendererCacheCleanupContext& c) {
    while (field<std::uint32_t>(cache, 8) != 0) {
        const auto size = current_child_size(current_last_child(cache), c);
        field<std::uint32_t>(cache, 0x10) = field<std::uint32_t>(cache, 0x10) - size;
        void* const child = current_last_child(cache);
        current_cache_release(cache, child, c);
        const auto count = field<std::uint32_t>(cache, 8);
        if (count != 0) {
            void* const data = field<void*>(cache, 4);
            auto* const row = static_cast<NativeRenderResourceRecord*>(plus(data, count * 0x2cu - 0x2cu));
            destroy_native_render_resource_record_00b2f990(*row, c.strings);
            field<std::uint32_t>(cache, 8) = field<std::uint32_t>(cache, 8) - 1u;
        }
    }
    resize_native_render_resource_record_array_00b30340(plus(cache, 4), 0, c.strings, c.validation);
}

void destroy_native_renderer_resource_array_00b31730(void* header, NativeRendererCacheCleanupContext& c) {
    resize_native_render_resource_record_array_00b30340(header, 0, c.strings, c.validation);
    singleton_lifetime_free(field<void*>(header, 0));
}

void destroy_native_renderer_resource_cache_00b32090(void* cache, NativeRendererCacheCleanupContext& c) {
    field<std::uint32_t>(cache, 0) = 0x00d5f038u;
    BaseUnwind cleanup{cache, c};
    clear_native_renderer_resource_cache_00b316c0(cache, c);
    cleanup.armed = false;
    destroy_native_renderer_resource_array_00b31730(plus(cache, 4), c);
}

void destroy_native_texture_resource_cache_00b32370(void* cache, NativeRendererCacheCleanupContext& c) {
    field<std::uint32_t>(cache, 0) = 0x00d5f088u;
    void* const child = field<void*>(cache, 0x20);
    TextureUnwind cleanup{cache, c};
    if (child != nullptr) {
        release_native_render_actual_owner(c.owners, child);
        field<void*>(cache, 0x20) = nullptr;
    }
    char* const data = field<char*>(cache, 0x18);
    cleanup.state = 0;
    if (data != nullptr) c.strings.release(data, field<std::uint32_t>(cache, 0x14) + 1u);
    cleanup.state = -1;
    destroy_native_renderer_resource_cache_00b32090(cache, c);
}

} // namespace bsp
