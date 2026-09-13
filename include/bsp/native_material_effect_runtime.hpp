#pragma once
#include "bsp/native_material_effect_programs.hpp"

namespace bsp {
struct NativeShaderDescriptorReadContext;
struct NativeVfsNameResolutionContext;
struct NativeMaterialStateCacheContext;
struct NativeMaterialProgramCompileContext;
struct NativeTextureLoadingContext;

// Remaining compiler dependency. Implementations must publish their persistent
// child frame before acquiring native state; return an actual canonical held
// pass (or a native compiler-null result), never an unavailable-child default.
class NativeMaterialEffectCompiler {
public:
    virtual ~NativeMaterialEffectCompiler() = default;
    virtual NativeMaterialPassStorage* build_program_00b3c3a0(
        const NativeMaterialProgramRequest&, NativeMaterialProgramChild&) = 0;
};

// Actual B3C3A0 wrapper/builder prefix, with its remaining native compiler tail
// supplied by the context. Uses the same lifetime/string domain as the parent.
class NativeMaterialEffectCompilerBinding final : public NativeMaterialEffectCompiler {
public:
    NativeMaterialEffectCompilerBinding(NativeMaterialEffectDestructionAccess&,
        NativeMaterialProgramCompileContext&);
    NativeMaterialPassStorage* build_program_00b3c3a0(
        const NativeMaterialProgramRequest&, NativeMaterialProgramChild&) override;
private:
    NativeMaterialProgramCompileContext& context_;
};

// Configure before loading. Bind texture resolution to actual BDF4C0 after
// checking one string pool and manager publication. Each invocation creates
// its own retained VFS operation; the name context must outlive failed frames.
void bind_native_texture_vfs_name_resolution(NativeTextureLoadingContext&,
    NativeVfsNameResolutionContext&);

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
