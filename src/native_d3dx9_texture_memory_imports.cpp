#include "bsp/native_d3dx9_texture_memory_imports.hpp"

#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native D3DX9 texture-memory imports require MSVC Win32.
#endif

namespace bsp {
namespace {
template<class Function>
Function resolve(HMODULE module, const char* name) {
    const FARPROC address = GetProcAddress(module, name);
    if (address == nullptr) {
        throw std::runtime_error("actual d3dx9_40 texture-memory export required");
    }
    Function function;
    static_assert(sizeof(function) == sizeof(address));
    std::memcpy(&function, &address, sizeof(function));
    return function;
}
} // namespace

NativeD3dx9TextureMemoryImports::NativeD3dx9TextureMemoryImports(HMODULE module)
    : cube_volume_(module) {
    if (module == nullptr) throw std::invalid_argument("actual d3dx9_40 module required");
    image_info_ = resolve<ReadImageInfoFromMemory>(
        module, "D3DXGetImageInfoFromFileInMemory");
    create_texture_ = resolve<CreateTextureFromMemory>(
        module, "D3DXCreateTextureFromFileInMemoryEx");
}

} // namespace bsp
