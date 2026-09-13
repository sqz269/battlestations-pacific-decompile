#include "bsp/native_material_effect_runtime.hpp"
#include "bsp/native_shader_descriptor_reader.hpp"
#include "bsp/native_vfs_name_resolution.hpp"
#include "bsp/native_vfs_device_route.hpp"
#include "bsp/native_vfs_lookup_routes.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_material_state_cache.hpp"
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
void require_empty(const NativeMaterialProgramChild& child) {
    if (child) throw std::invalid_argument("native program child output already retains an operation");
}
struct NameFrame final : NativeMaterialProgramChildFrame {
    NativeVfsNameResolutionAcquired acquired;
};
struct StateFrame final : NativeMaterialProgramChildFrame {
    NativeMaterialStateCacheAcquired acquired;
    ~StateFrame() override {
        if (acquired.phase != NativeMaterialStateCachePhase::fresh &&
            acquired.phase != NativeMaterialStateCachePhase::complete) std::terminate();
    }
};
using Cache = NativeMaterialStateOwnerStorage* (*)(void*, NativeMaterialStateOwnerStorage&,
    NativeMaterialStateCacheContext&, NativeMaterialStateCacheAcquired&);
NativeMaterialStateOwnerStorage* cache_state(void* renderer,
    NativeMaterialStateOwnerStorage* input, NativeMaterialStateCacheContext& context,
    NativeMaterialProgramChild& child, Cache call) {
    require_empty(child);
    if (!input) throw std::invalid_argument("native state-cache input requires an actual held owner");
    auto frame = std::make_unique<StateFrame>();
    auto* const stable = frame.get();
    child = std::move(frame); // Publish before release, allocation or cache mutation.
    return call(renderer, *input, context, stable->acquired);
}
} // namespace

NativeMaterialEffectNativeChildren::NativeMaterialEffectNativeChildren(
    NativeMaterialEffectDestructionAccess& lifetime, NativeShaderDescriptorReadContext& descriptors,
    NativeVfsNameResolutionContext& names, NativeMaterialStateCacheContext& states,
    NativeMaterialEffectCompiler& compiler)
    : descriptors_(descriptors), names_(names), states_(states), compiler_(compiler) {
    if (&lifetime.strings != &descriptors.strings ||
        &lifetime.strings != &names.device.lookup.physical.strings ||
        &lifetime.retained_owners != &states.owners)
        throw std::invalid_argument("native effect children require one string and canonical owner domain");
}
void NativeMaterialEffectNativeChildren::read_descriptor_00b43b00(
    NativeShaderDescriptorStorage& descriptor, const void* name, std::uint32_t generation,
    NativeMaterialProgramChild& child) {
    require_empty(child);
    auto frame = std::make_unique<NativeShaderDescriptorReadOperation>();
    auto* const stable = frame.get();
    child = std::move(frame);
    read_native_shader_descriptor_00b43b00(descriptor, name, generation, descriptors_, *stable);
}
bool NativeMaterialEffectNativeChildren::resolve_name_00bdf4c0(
    void* manager, NativeString& name, NativeMaterialProgramChild& child) {
    require_empty(child);
    auto frame = std::make_unique<NameFrame>();
    auto* const stable = frame.get();
    child = std::move(frame);
    return resolve_native_vfs_existing_name_00bdf4c0(manager, &name, names_, stable->acquired);
}
NativeMaterialPassStorage* NativeMaterialEffectNativeChildren::build_program_00b3c3a0(
    const NativeMaterialProgramRequest& request, NativeMaterialProgramChild& child) {
    require_empty(child);
    return compiler_.build_program_00b3c3a0(request, child);
}
NativeMaterialStateOwnerStorage* NativeMaterialEffectNativeChildren::cache_render_00b26500(
    void* renderer, NativeMaterialStateOwnerStorage* input, NativeMaterialProgramChild& child) {
    return cache_state(renderer, input, states_, child, cache_native_material_render_states_00b26500);
}
NativeMaterialStateOwnerStorage* NativeMaterialEffectNativeChildren::cache_third_00b265c0(
    void* renderer, NativeMaterialStateOwnerStorage* input, NativeMaterialProgramChild& child) {
    return cache_state(renderer, input, states_, child, cache_native_material_third_states_00b265c0);
}
NativeMaterialStateOwnerStorage* NativeMaterialEffectNativeChildren::cache_sampler_00b26680(
    void* renderer, NativeMaterialStateOwnerStorage* input, NativeMaterialProgramChild& child) {
    return cache_state(renderer, input, states_, child, cache_native_material_sampler_states_00b26680);
}
} // namespace bsp
