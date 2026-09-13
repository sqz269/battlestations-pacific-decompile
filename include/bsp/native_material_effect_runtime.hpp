#pragma once
#include "bsp/native_material_effect_programs.hpp"

namespace bsp {
struct NativeShaderDescriptorReadContext;
struct NativeVfsNameResolutionContext;
struct NativeMaterialStateCacheContext;

// Remaining compiler dependency. Implementations must publish their persistent
// child frame before acquiring native state; return an actual canonical held
// pass (or a native compiler-null result), never an unavailable-child default.
class NativeMaterialEffectCompiler {
public:
    virtual ~NativeMaterialEffectCompiler() = default;
    virtual NativeMaterialPassStorage* build_program_00b3c3a0(
        const NativeMaterialProgramRequest&, NativeMaterialProgramChild&) = 0;
};

// Concrete five-child composition for B45EE0/B46950/B5F6A0. Construct with the
// SAME effect lifetime used by NativeMaterialEffectProgramsContext. All borrowed
// contexts and the caller's actual descriptor/name headers must outlive a failed
// retained frame. Native cleanup may leave consumed header preimages behind.
// Child outputs must be empty on entry; this adapter never discards old state.
class NativeMaterialEffectNativeChildren final : public NativeMaterialEffectProgramChildren {
public:
    NativeMaterialEffectNativeChildren(NativeMaterialEffectDestructionAccess&,
        NativeShaderDescriptorReadContext&, NativeVfsNameResolutionContext&,
        NativeMaterialStateCacheContext&, NativeMaterialEffectCompiler&);
    void read_descriptor_00b43b00(NativeShaderDescriptorStorage&, const void*,
        std::uint32_t, NativeMaterialProgramChild&) override;
    bool resolve_name_00bdf4c0(void*, NativeString&, NativeMaterialProgramChild&) override;
    NativeMaterialPassStorage* build_program_00b3c3a0(
        const NativeMaterialProgramRequest&, NativeMaterialProgramChild&) override;
    NativeMaterialStateOwnerStorage* cache_render_00b26500(void*,
        NativeMaterialStateOwnerStorage*, NativeMaterialProgramChild&) override;
    NativeMaterialStateOwnerStorage* cache_third_00b265c0(void*,
        NativeMaterialStateOwnerStorage*, NativeMaterialProgramChild&) override;
    NativeMaterialStateOwnerStorage* cache_sampler_00b26680(void*,
        NativeMaterialStateOwnerStorage*, NativeMaterialProgramChild&) override;
private:
    NativeShaderDescriptorReadContext& descriptors_;
    NativeVfsNameResolutionContext& names_;
    NativeMaterialStateCacheContext& states_;
    NativeMaterialEffectCompiler& compiler_;
};
} // namespace bsp
