#pragma once
#include "bsp/native_material_program_compiler_actual.hpp"

namespace bsp::game {
// Borrowed production domains for B3B3C0's pass/reflection/shader producers.
// Keep caller operation frames and native references alive until their actual
// cleanup completes. Registration creates host companions only: no native
// allocation, initialization, retain, release, or replacement registry.
struct GameNativeMaterialCompilerOwners {
    NativeMaterialPassConstructionAccess& passes;
    NativeCompiledShaderReflectionContext& reflection;
    NativeD3d9ShaderConstructionActualContext& shaders;
    NativeMaterialProgramCompilerActualRegistration registration;
};
} // namespace bsp::game
