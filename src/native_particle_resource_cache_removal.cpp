#include "bsp/native_particle_resource_cache_removal.hpp"
#include "bsp/native_particle_resource_lifetime.hpp"
#include "bsp/native_particle_resource_records.hpp"
#include "bsp/native_particle_resource_cache.hpp"
#include "bsp/native_pooled_resource_path.hpp"
#include "bsp/native_render_alias_insertion.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdlib>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle resource cache removal requires MSVC Win32.
#endif

namespace bsp {
namespace {
using U = std::uint32_t;
void* at(const void* p, U offset) { return reinterpret_cast<void*>(reinterpret_cast<U>(p) + offset); }
template<class T> volatile T& field(const void* p, U offset) { return *static_cast<volatile T*>(at(p, offset)); }
struct NameUnwind {
    void* name;
    NativeStringRawPoolContext& strings;
    bool active = true;
    ~NameUnwind() noexcept { if (active) destroy_native_string_header_0041dd20(name, strings); }
};
struct ResourceUnwind {
    void* resource;
    NativeStringRawPoolContext& strings;
    bool active = true;
    ~ResourceUnwind() noexcept { if (active) destroy_native_particle_resource_00af4280(resource, strings); }
};
const void* diagnostic_text(const void* p) { return p ? p : reinterpret_cast<void*>(0x00f8766c); }
// Complete004254B0 is precisely C3. Preserve argument evaluation, no fake log.
void diagnostic_004254b0(const void*, const void* = nullptr) noexcept {}
}

void* assign_native_particle_resource_record_00870c10(void* destination, const void* source, NativeStringRawPoolContext& strings) {
    if (destination != source) {
        resize_native_string_header_0041dd40(destination, strings, field<U>(source, 0), true);
        if (field<U>(source, 0) != 0) {
            const U count = field<U>(destination, 0);
            auto* const from = field<void*>(source, 4);
            auto* const output = field<void*>(destination, 4);
            if (count != 0) std::memmove(output, from, count);
        }
    }
    auto* const source_list = at(source, 8);
    auto* const destination_list = at(destination, 8);
    if (destination_list != source_list) {
        auto* const end = field<NativeRenderResourceAliasNode*>(source_list, 4);
        auto* const first = field<NativeRenderResourceAliasNode*>(end, 0);
        clear_native_render_resource_aliases_004d05e0(destination_list, strings);
        auto* const destination_end = field<NativeRenderResourceAliasNode*>(destination_list, 4);
        auto* const destination_first = field<NativeRenderResourceAliasNode*>(destination_end, 0);
        insert_native_render_alias_range_004d26a0(destination_list,
            {destination_list, destination_first}, {source_list, first}, {source_list, end}, strings);
    }
    for (U offset = 0x14; offset != 0x2c; offset += 4)
        field<U>(destination, offset) = field<U>(source, offset);
    return destination;
}

void remove_native_particle_resource_by_alias_008714e0(void* cache, const void* original_name, NativeStringRawPoolContext& strings) {
    alignas(4) unsigned char name[8];
    const bool distinct = static_cast<const void*>(name) != original_name;
    field<U>(name, 0) = 0;
    field<void*>(name, 4) = nullptr;
    if (distinct) {
        resize_native_string_header_0041dd40(name, strings, field<U>(original_name, 0), true);
        if (field<U>(original_name, 0) != 0) {
            const U bytes = field<U>(name, 0);
            auto* const from = field<void*>(original_name, 4);
            auto* const destination = field<void*>(name, 4);
            if (bytes != 0) std::memmove(destination, from, bytes);
        }
    }
    NameUnwind unwind{name, strings};
    alignas(4) unsigned char normalized[8];
    copy_construct_native_resource_path_header_00bee780(normalized, name, strings);
    destroy_native_string_header_0041dd20(normalized, strings);
    // 871570 captures count before data. Normalized scratch is deliberately
    // discarded; matching below uses the original copied name at frame-14h.
    const U count = field<U>(cache, 8);
    auto* cursor = field<void*>(cache, 4);
    auto* const end = at(cursor, count * 0x2cu);
    void* matched = nullptr;
    while (cursor != end) {
        auto* const sentinel_cell = at(cursor, 0xc);
        auto* const captured_end = field<void*>(sentinel_cell, 0);
        auto* node = field<void*>(captured_end, 0);
        // 871593 CMP EAX,EAX always succeeds; do not add an invalid call.
        while (node != captured_end) {
            if (node == field<void*>(sentinel_cell, 0)) _invalid_parameter_noinfo();
            const U node_length = field<U>(node, 8);
            const U name_length = field<U>(name, 0);
            if (node_length == name_length) {
                bool equal = true;
                if (node_length != 0) {
                    auto* const name_data = field<char*>(name, 4);
                    auto* const node_data = field<char*>(node, 0xc);
                    equal = _stricmp(node_data, name_data) == 0;
                }
                if (equal) { matched = cursor; break; }
            }
            if (node == field<void*>(sentinel_cell, 0)) _invalid_parameter_noinfo();
            node = field<void*>(node, 0);
        }
        if (matched != nullptr) break;
        cursor = at(cursor, 0x2c);
    }
    if (matched != nullptr) {
        const auto* const record_data = diagnostic_text(field<void*>(matched, 4));
        const auto* const name_data = diagnostic_text(field<void*>(name, 4));
        diagnostic_004254b0(name_data, record_data);
        const U current_count = field<U>(cache, 8);
        auto* const current_data = field<void*>(cache, 4);
        auto* const last = at(current_data, (current_count - 1u) * 0x2cu);
        if (matched != last) assign_native_particle_resource_record_00870c10(matched, last, strings);
        const U after_count = field<U>(cache, 8);
        auto* const after_data = field<void*>(cache, 4);
        destroy_native_particle_resource_record_0086fcf0(at(after_data, after_count * 0x2cu - 0x2cu), strings);
        field<U>(cache, 8) = field<U>(cache, 8) - 1u;
    } else {
        diagnostic_004254b0(diagnostic_text(field<void*>(original_name, 4)));
    }
    unwind.active = false;
    destroy_native_string_header_0041dd20(name, strings);
}

void destroy_native_cached_particle_resource_00871ca0(void* resource,
    NativeParticleResourceCacheContext& cache, NativeStringRawPoolContext& strings) {
    field<U>(resource, 0) = 0x00d0d418;
    ResourceUnwind unwind{resource, strings};
    auto* const owner = get_native_particle_resource_cache_owner_00871bd0(cache);
    remove_native_particle_resource_by_alias_008714e0(at(owner, 4), at(resource, 8), strings);
    unwind.active = false;
    destroy_native_particle_resource_00af4280(resource, strings);
}

void* delete_native_cached_particle_resource_00871fa0(void* resource, U flags,
    NativeParticleResourceCacheContext& cache, NativeStringRawPoolContext& strings) {
    destroy_native_cached_particle_resource_00871ca0(resource, cache, strings);
    if ((flags & 1u) != 0) singleton_lifetime_free(resource);
    return resource;
}
}
