#include "bsp/native_cube_volume_retained_recreation.hpp"
#include "bsp/native_shader_device_reset.hpp"

#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native retained cube/volume callbacks require MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);

void* current_pointer(const void* base, std::size_t offset) noexcept {
    return *reinterpret_cast<void* const volatile*>(
        static_cast<const unsigned char*>(base) + offset);
}

template<class Texture>
void release_texture(void* owner) {
    auto* const captured = static_cast<Texture*>(current_pointer(owner, 0x10));
    if (captured != nullptr) {
        (void)captured->AddRef();
        (void)captured->Release();
    }
    auto* const current = static_cast<Texture*>(current_pointer(owner, 0x10));
    if (current != nullptr) {
        (void)current->Release();
        *reinterpret_cast<void* volatile*>(static_cast<unsigned char*>(owner) + 0x10) = nullptr;
    }
}

template<class Function>
Function resolve(HMODULE module, const char* name) {
    const FARPROC address = GetProcAddress(module, name);
    if (address == nullptr) throw std::runtime_error("actual d3dx9_40 texture export required");
    Function function;
    static_assert(sizeof(function) == sizeof(address));
    std::memcpy(&function, &address, sizeof(function));
    return function;
}
} // namespace

NativeD3dx9CubeVolumeMemoryImports::NativeD3dx9CubeVolumeMemoryImports(HMODULE module) {
    if (module == nullptr) throw std::invalid_argument("actual d3dx9_40 module required");
    cube_ = resolve<Cube>(module, "D3DXCreateCubeTextureFromFileInMemory");
    volume_ = resolve<Volume>(module, "D3DXCreateVolumeTextureFromFileInMemory");
}

HRESULT NativeD3dx9CubeVolumeMemoryImports::create_cube(IDirect3DDevice9* device,
    const void* data, UINT length, IDirect3DCubeTexture9** output) const {
    return cube_(device, data, length, output);
}
HRESULT NativeD3dx9CubeVolumeMemoryImports::create_volume(IDirect3DDevice9* device,
    const void* data, UINT length, IDirect3DVolumeTexture9** output) const {
    return volume_(device, data, length, output);
}

void release_native_cube_texture_retained_00b3d7c0(void* owner) {
    release_texture<IDirect3DCubeTexture9>(owner);
}
void release_native_volume_texture_retained_00b3d800(void* owner) {
    release_texture<IDirect3DVolumeTexture9>(owner);
}

void recreate_native_cube_texture_retained_00b3e1f0(void* owner,
    NativeCubeVolumeRetainedRecreationContext& context) {
    auto* const device = static_cast<IDirect3DDevice9*>(get_native_renderer_device_00b1fef0(
        current_pointer(context.actual_renderer_global_f8d394, 0)));
    const UINT length = static_cast<UINT>(dispatch_native_memory_stream_length(
        current_pointer(owner, 0x2c), context.retained_memory));
    const void* const data = native_memory_stream_data_00bef610(current_pointer(owner, 0x2c), nullptr);
    auto* const output = reinterpret_cast<IDirect3DCubeTexture9**>(
        static_cast<unsigned char*>(owner) + 0x10);
    (void)context.imports.create_cube(device, data, length, output);
}

void recreate_native_volume_texture_retained_00b3e230(void* owner,
    NativeCubeVolumeRetainedRecreationContext& context) {
    auto* const device = static_cast<IDirect3DDevice9*>(get_native_renderer_device_00b1fef0(
        current_pointer(context.actual_renderer_global_f8d394, 0)));
    const UINT length = static_cast<UINT>(dispatch_native_memory_stream_length(
        current_pointer(owner, 0x30), context.retained_memory));
    const void* const data = native_memory_stream_data_00bef610(current_pointer(owner, 0x30), nullptr);
    auto* const output = reinterpret_cast<IDirect3DVolumeTexture9**>(
        static_cast<unsigned char*>(owner) + 0x10);
    (void)context.imports.create_volume(device, data, length, output);
}
} // namespace bsp
