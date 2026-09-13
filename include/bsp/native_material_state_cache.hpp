#pragma once
#include "bsp/native_material_pass_states.hpp"

namespace bsp {
// Actual12h pointer-vector headers in the SAME renderer: third+1ADC,
// render+1AE8 and sampler+1AF4. This view owns no array or references.
struct NativeMaterialStateCacheArray {
    NativeMaterialStateOwnerStorage** data_00;
    std::int32_t count_04;
    std::int32_t capacity_08;
};
static_assert(sizeof(NativeMaterialStateCacheArray) == 12);
enum class NativeMaterialStateCachePhase { fresh, lookup, reserve, publish, release_input, retain_result, complete, failed };
struct NativeMaterialStateCacheAcquired {
    NativeMaterialStateCachePhase phase{NativeMaterialStateCachePhase::fresh};
    NativeMaterialStateOwnerStorage* input{};
    NativeMaterialStateOwnerStorage* result{};
    NativeMaterialStateOwnerStorage** fresh_array{};
    std::int32_t lookup_index{};
    std::uint32_t native_site{};
    bool input_reference_consumed{};
    bool cache_slot_published{};
    bool cache_reference_added{};
    bool replacement_reference_added{};
};
struct NativeMaterialStateCacheContext {
    NativeRenderActualOwners& owners;
};
// ECX cached owner, stack incoming owner, AL match, RET4. Compare captured
// counts, then search each LEFT row in the RIGHT rows. Rows may be reordered;
// matches are not consumed. Duplicate rows therefore retain native directional
// behavior. Equal nonpositive counts return1 without reading row storage.
std::uint8_t match_native_material_render_states_00b5ea10(
    const NativeMaterialStateOwnerStorage&, const NativeMaterialStateOwnerStorage&);
std::uint8_t match_native_material_third_states_00b5eaa0(
    const NativeMaterialStateOwnerStorage&, const NativeMaterialStateOwnerStorage&);
std::uint8_t match_native_material_sampler_states_00b5eb40(
    const NativeMaterialStateOwnerStorage&, const NativeMaterialStateOwnerStorage&);
// ECX vector, stack signed requested capacity, RET4. Clamp to1; signed growth;
// original BF55BE -> BF681B allocation and BF6989 free through the existing CRT
// implementation. Reload source/count after allocation; publish AFTER free.
// Native DWORD overflow and inaccessible/corrupt extents are host errors.
void reserve_native_material_third_cache_00b22650(NativeMaterialStateCacheArray&,
    std::int32_t, NativeMaterialStateCacheAcquired* = nullptr);
void reserve_native_material_render_cache_00b226b0(NativeMaterialStateCacheArray&,
    std::int32_t, NativeMaterialStateCacheAcquired* = nullptr);
void reserve_native_material_sampler_cache_00b22710(NativeMaterialStateCacheArray&,
    std::int32_t, NativeMaterialStateCacheAcquired* = nullptr);
// ECX actual renderer, stack held state owner, EAX held canonical state, RET4.
// Hit consumes input BEFORE reloading and retaining the current cached entry.
// Miss publishes input, increments count and render/third counters1CEC/1CF0,
// then adds the cache reference. Sampler has no corresponding statistic write.
// Caller retains the exact one-shot frame on failure; no rollback or replay.
NativeMaterialStateOwnerStorage* cache_native_material_render_states_00b26500(
    void*, NativeMaterialStateOwnerStorage&, NativeMaterialStateCacheContext&,
    NativeMaterialStateCacheAcquired&);
NativeMaterialStateOwnerStorage* cache_native_material_third_states_00b265c0(
    void*, NativeMaterialStateOwnerStorage&, NativeMaterialStateCacheContext&,
    NativeMaterialStateCacheAcquired&);
NativeMaterialStateOwnerStorage* cache_native_material_sampler_states_00b26680(
    void*, NativeMaterialStateOwnerStorage&, NativeMaterialStateCacheContext&,
    NativeMaterialStateCacheAcquired&);
} // namespace bsp
