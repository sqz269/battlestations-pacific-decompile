#pragma once
#include "bsp/native_material_program_compiler.hpp"
#include "bsp/native_sampler_cache_entry.hpp"
#include "bsp/native_shader_descriptor_samplers.hpp"
#include "bsp/native_shader_sampler_owner.hpp"
#include <list>

namespace bsp {
// Native B3B280 steady ESP is B. B5F100 copies the DWORD at B-16 after
// writing only its low byte. Original OS/current-owner release frames can
// leave different upper bytes there. Each successful source1 release consumes
// one captured native residue, with ordering/identity checked AFTER release.
struct NativeMaterialDescriptorSamplerReleaseScratch {
    std::uint32_t sampler_index;
    const void* released_owner_identity;
    std::uint32_t word_b_minus_16;
};
struct NativeMaterialDescriptorSamplersContext {
    NativeMaterialPassDestructionAccess& pass_lifetime;
    void* volatile& actual_manager_01090aa0;
    void* volatile& actual_loader_00f8d420;
    NativeSamplerCacheEntryContext& loader;
    volatile std::uint32_t& stack_b_minus_16;
    const NativeMaterialDescriptorSamplerReleaseScratch* release_stack;
    std::size_t release_stack_count;
};
struct NativeMaterialDescriptorSamplerStep final {
    std::uint32_t sampler_index{},state_index{},native_site{};
    NativeShaderSamplerStorage* sampler{};
    NativeShaderStateListStorage* current_states{};
    NativeSamplerLoaderSingletonStorage* loader{};
    void* captured_owner{};
    bool owner_obligation{},release_entered{},release_returned{};
    std::optional<NativeSamplerLoaderOperation> singleton;
    std::optional<NativeSamplerDefaultLoadOperation> load;
    std::optional<NativeDescriptorSamplerOperation> effect_name,reference,binding;
};
struct NativeMaterialDescriptorSamplersOperation final {
    enum class Phase { fresh,running,complete,failed,diagnostic_retired };
    Phase phase{Phase::fresh};
    NativeMaterialProgramBuilderStorage* builder{};
    NativeMaterialPassStorage* pass{};
    NativeShaderDescriptorStorage* descriptor{};
    NativeMaterialDescriptorSamplersContext* context{};
    std::uint32_t sampler_index{},native_site{};
    std::size_t release_stack_index{};
    std::list<NativeMaterialDescriptorSamplerStep> steps;
    NativeMaterialDescriptorSamplersOperation()=default;
    ~NativeMaterialDescriptorSamplersOperation();
    NativeMaterialDescriptorSamplersOperation(const NativeMaterialDescriptorSamplersOperation&)=delete;
    NativeMaterialDescriptorSamplersOperation& operator=(const NativeMaterialDescriptorSamplersOperation&)=delete;
    // Caller resolves retained actual acquisitions AND all failed child frames.
    // No cleanup, rollback, replay, fallback owner or automatic child retirement.
    void acknowledge_diagnostic_cleanup() noexcept;
};
// Full B3B280 normal schedule. ECX same actualB0h builder, stack actual88h pass
// and actual110h descriptor, RET8. All arrays, names, counters and owner refs
// remain in their existing native storage/canonical domains. Unsigned live
// descriptor/state loops; current stage byte/counters read at each native site.
// The source ABI exposes native stack preimages and persistent host failure
// frames. Native exception/unwind ABI and whole-parent/game parity are unproved.
void append_native_material_descriptor_samplers_00b3b280(
    NativeMaterialProgramBuilderStorage&,NativeMaterialPassStorage&,
    NativeShaderDescriptorStorage&,NativeMaterialDescriptorSamplersContext&,
    NativeMaterialDescriptorSamplersOperation&);
} // namespace bsp
