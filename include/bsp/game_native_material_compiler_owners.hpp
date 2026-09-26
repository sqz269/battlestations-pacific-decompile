#pragma once
#include "bsp/native_material_program_compiler_actual.hpp"
#include "bsp/native_material_pass_copy.hpp"

namespace bsp::game {
// Borrowed production domains for B3B3C0 and B45E00/B455C0.
// Keep caller operation frames and native references alive until their actual
// cleanup completes. Registration creates host companions only: no native
// allocation, initialization, retain, release, or replacement registry.
struct GameNativeMaterialCompilerOwners {
    NativeMaterialPassConstructionAccess& passes;
    NativeCompiledShaderReflectionContext& reflection;
    NativeD3d9ShaderConstructionActualContext& shaders;
    NativeMaterialProgramCompilerActualRegistration registration;
    // SAME pass lifetime, existing state/pass companions and numeric texture
    // domain. Copy preserves native counts, including values greater than one.
    NativeMaterialPassCopyAccess& copies;
    NativeMaterialSecondaryPassRegistration secondary_registration;
    NativeMaterialProgramNumericRendererDomain& numeric_renderer;
};
} // namespace bsp::game
