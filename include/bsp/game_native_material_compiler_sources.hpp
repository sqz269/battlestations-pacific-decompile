#pragma once
#include "bsp/native_shader_field_initialization.hpp"
#include "bsp/native_vertex_shader_compilation.hpp"
#include "bsp/native_pixel_shader_compilation.hpp"

namespace bsp::game {
// Borrowed source/compilation contexts for the same application renderer,
// strings, system constants, VFS, D3DX module and process formatting buffer.
// Caller-owned operation frames retain failed native work. B35110 formatting
// is nonreentrant and requires the native normal readable/writable extents.
struct GameNativeMaterialCompilerSources {
    NativeShaderFieldInitializationContext& source;
    NativeVertexShaderCompilationContext& vertex;
    NativePixelShaderCompilationContext& pixel;
};
} // namespace bsp::game
