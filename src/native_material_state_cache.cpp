#include "bsp/native_material_state_cache.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <limits>
#include <stdexcept>

namespace bsp {
namespace {
void require(bool ok, const char* reason) { if (!ok) throw std::logic_error(reason); }
template<class T> T current(const void* base, std::size_t offset) noexcept {
    return *reinterpret_cast<const volatile T*>(static_cast<const std::byte*>(base) + offset);
}
template<class T> void publish(void* base, std::size_t offset, T value) noexcept {
    *reinterpret_cast<volatile T*>(static_cast<std::byte*>(base) + offset) = value;
}
std::uint8_t match(const NativeMaterialStateOwnerStorage& left,
    const NativeMaterialStateOwnerStorage& right, std::uint32_t words) {
    const auto right_count = current<std::int32_t>(&right, 0xc);
    const auto left_count = current<std::int32_t>(&left, 0xc);
    if (right_count != left_count) return 0;
    for (std::int32_t i = 0; i < right_count; ++i) {
        const auto* row = current<const std::uint32_t*>(&left, 8);
        const auto* candidate = current<const std::uint32_t*>(&right, 8);
        require(row && candidate, "positive native state extent requires row storage");
        row += static_cast<std::size_t>(i) * words;
        // First left word is captured outside the inner search, just as EBP.
        const auto first = current<std::uint32_t>(row, 0);
        std::int32_t j = 0;
        for (; j < left_count; ++j, candidate += words) {
            if (first != current<std::uint32_t>(candidate, 0)) continue;
            if (current<std::uint32_t>(row, 4) != current<std::uint32_t>(candidate, 4)) continue;
            if (words == 3 && current<std::uint32_t>(row, 8) != current<std::uint32_t>(candidate, 8)) continue;
            break;
        }
        if (j == left_count) return 0;
    }
    return 1;
}
std::int32_t doubled(std::int32_t value) noexcept {
    const auto bits = static_cast<std::uint32_t>(value) * 2u;
    std::int32_t result;
    std::memcpy(&result, &bits, 4);
    return result > 1 ? result : 1;
}
void reserve(NativeMaterialStateCacheArray& array, std::int32_t requested,
    NativeMaterialStateCacheAcquired* acquired) {
    if (requested < 1) requested = 1;
    if (current<std::int32_t>(&array, 8) >= requested) return;
    require(static_cast<std::uint32_t>(requested) <= (std::numeric_limits<std::uint32_t>::max)() / 4u,
        "native pointer-vector DWORD allocation overflow is outside the valid extent domain");
    const auto bytes = static_cast<std::size_t>(requested) * 4u;
    auto** const fresh = static_cast<NativeMaterialStateOwnerStorage**>(
        singleton_lifetime_allocate({SingletonAllocationKind::pointer_slots, bytes, bytes}));
    if (acquired) acquired->fresh_array = fresh;
    try {
        for (std::int32_t i = 0; i < current<std::int32_t>(&array, 4); ++i) {
            require(i < requested, "native cache vector grew beyond the newly allocated extent");
            auto** const source = current<NativeMaterialStateOwnerStorage**>(&array, 0);
            require(source, "positive native pointer-vector count requires data");
            fresh[i] = current<NativeMaterialStateOwnerStorage*>(source, static_cast<std::size_t>(i) * 4u);
        }
    } catch (...) {
        // Native valid inputs do not fault here. With a retained acquisition,
        // preserve the fresh allocation at the host guard; standalone callers
        // have no continuation and discard only this unpublished allocation.
        if (!acquired) singleton_lifetime_free(fresh);
        throw;
    }
    singleton_lifetime_free(current<void*>(&array, 0));
    publish(&array, 0, fresh);
    publish(&array, 8, requested);
    if (acquired) acquired->fresh_array = nullptr;
}
using Compare = std::uint8_t (*)(const NativeMaterialStateOwnerStorage&, const NativeMaterialStateOwnerStorage&);
using Reserve = void (*)(NativeMaterialStateCacheArray&, std::int32_t, NativeMaterialStateCacheAcquired*);
struct Sites {
    std::uint32_t compare, reserve, miss_retain, hit_release, hit_retain;
};
NativeMaterialStateOwnerStorage* cache(void* renderer, NativeMaterialStateOwnerStorage& input,
    NativeMaterialStateCacheContext& context, NativeMaterialStateCacheAcquired& acquired,
    std::size_t offset, std::size_t statistic, Compare compare, Reserve grow, Sites sites) {
    require(renderer && acquired.phase == NativeMaterialStateCachePhase::fresh,
        "native state cache requires an actual renderer and fresh persistent acquisition");
    auto& array = *reinterpret_cast<NativeMaterialStateCacheArray*>(static_cast<std::byte*>(renderer) + offset);
    acquired.input = &input;
    acquired.phase = NativeMaterialStateCachePhase::lookup;
    try {
        require(current<std::int32_t>(&array, 4) >= 0,
            "negative renderer cache count would index outside native storage");
        for (std::int32_t i = 0; i < current<std::int32_t>(&array, 4); ++i) {
            acquired.lookup_index = i;
            auto** data = current<NativeMaterialStateOwnerStorage**>(&array, 0);
            require(data, "nonempty native state cache requires actual pointer storage");
            auto* entry = current<NativeMaterialStateOwnerStorage*>(data, static_cast<std::size_t>(i) * 4u);
            require(entry, "native state cache entries require actual state owners");
            acquired.native_site = sites.compare;
            if (!compare(*entry, input)) continue;
            acquired.phase = NativeMaterialStateCachePhase::release_input;
            acquired.native_site = sites.hit_release;
            // Native decrements first and only dispatches its current terminal
            // on zero. The shared helper preserves that order; metadata lookup
            // is required only by a terminal, never by the nonzero path.
            acquired.input_reference_consumed = true;
            release_native_render_actual_owner(context.owners, &input);
            acquired.phase = NativeMaterialStateCachePhase::retain_result;
            acquired.native_site = sites.hit_retain;
            data = current<NativeMaterialStateOwnerStorage**>(&array, 0);
            require(data, "terminal callback removed the native cache array");
            entry = current<NativeMaterialStateOwnerStorage*>(data, static_cast<std::size_t>(i) * 4u);
            require(entry, "terminal callback removed the native cached owner");
            entry->references_04.fetch_add(1, std::memory_order_seq_cst);
            acquired.replacement_reference_added = true;
            // EAX return reload follows the increment, not the pre-release row.
            data = current<NativeMaterialStateOwnerStorage**>(&array, 0);
            acquired.result = current<NativeMaterialStateOwnerStorage*>(data, static_cast<std::size_t>(i) * 4u);
            acquired.phase = NativeMaterialStateCachePhase::complete;
            return acquired.result;
        }
        const auto capacity = current<std::int32_t>(&array, 8);
        if (current<std::int32_t>(&array, 4) == capacity) {
            acquired.phase = NativeMaterialStateCachePhase::reserve;
            acquired.native_site = sites.reserve;
            grow(array, doubled(capacity), &acquired);
        }
        acquired.phase = NativeMaterialStateCachePhase::publish;
        const auto count = current<std::int32_t>(&array, 4);
        auto** const data = current<NativeMaterialStateOwnerStorage**>(&array, 0);
        require(count >= 0 && count < current<std::int32_t>(&array, 8) && data,
            "native cache append requires an accessible slot");
        publish(data, static_cast<std::size_t>(count) * 4u, &input);
        acquired.cache_slot_published = true;
        publish(&array, 4, current<std::uint32_t>(&array, 4) + 1u);
        if (statistic) publish(renderer, statistic, current<std::uint32_t>(renderer, statistic) + 1u);
        acquired.native_site = sites.miss_retain;
        input.references_04.fetch_add(1, std::memory_order_seq_cst);
        acquired.cache_reference_added = true;
        acquired.result = &input;
        acquired.phase = NativeMaterialStateCachePhase::complete;
        return &input;
    } catch (...) {
        acquired.phase = NativeMaterialStateCachePhase::failed;
        throw;
    }
}
} // namespace
std::uint8_t match_native_material_render_states_00b5ea10(const NativeMaterialStateOwnerStorage& a, const NativeMaterialStateOwnerStorage& b) { return match(a,b,2); }
std::uint8_t match_native_material_third_states_00b5eaa0(const NativeMaterialStateOwnerStorage& a, const NativeMaterialStateOwnerStorage& b) { return match(a,b,3); }
std::uint8_t match_native_material_sampler_states_00b5eb40(const NativeMaterialStateOwnerStorage& a, const NativeMaterialStateOwnerStorage& b) { return match(a,b,3); }
void reserve_native_material_third_cache_00b22650(NativeMaterialStateCacheArray& a, std::int32_t n, NativeMaterialStateCacheAcquired* o) { reserve(a,n,o); }
void reserve_native_material_render_cache_00b226b0(NativeMaterialStateCacheArray& a, std::int32_t n, NativeMaterialStateCacheAcquired* o) { reserve(a,n,o); }
void reserve_native_material_sampler_cache_00b22710(NativeMaterialStateCacheArray& a, std::int32_t n, NativeMaterialStateCacheAcquired* o) { reserve(a,n,o); }
NativeMaterialStateOwnerStorage* cache_native_material_render_states_00b26500(void* r, NativeMaterialStateOwnerStorage& i, NativeMaterialStateCacheContext& c, NativeMaterialStateCacheAcquired& o) {
    return cache(r,i,c,o,0x1ae8,0x1cec,match_native_material_render_states_00b5ea10,reserve_native_material_render_cache_00b226b0,{0xb26520,0xb26557,0xb26579,0xb2658b,0xb265aa});
}
NativeMaterialStateOwnerStorage* cache_native_material_third_states_00b265c0(void* r, NativeMaterialStateOwnerStorage& i, NativeMaterialStateCacheContext& c, NativeMaterialStateCacheAcquired& o) {
    return cache(r,i,c,o,0x1adc,0x1cf0,match_native_material_third_states_00b5eaa0,reserve_native_material_third_cache_00b22650,{0xb265e0,0xb26617,0xb26639,0xb2664b,0xb2666a});
}
NativeMaterialStateOwnerStorage* cache_native_material_sampler_states_00b26680(void* r, NativeMaterialStateOwnerStorage& i, NativeMaterialStateCacheContext& c, NativeMaterialStateCacheAcquired& o) {
    return cache(r,i,c,o,0x1af4,0,match_native_material_sampler_states_00b5eb40,reserve_native_material_sampler_cache_00b22710,{0xb266a0,0xb266d4,0xb266ef,0xb26701,0xb26720});
}
} // namespace bsp
