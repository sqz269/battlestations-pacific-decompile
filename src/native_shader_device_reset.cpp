#include "bsp/native_shader_device_reset.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d9.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native shader device reset requires MSVC Win32.
#endif

namespace bsp {
namespace {

void* load_pointer(const void* storage, std::uint32_t offset) noexcept {
    return *reinterpret_cast<void* const volatile*>(
        static_cast<const unsigned char*>(storage) + offset);
}
void store_pointer(void* storage, std::uint32_t offset, void* value) noexcept {
    *reinterpret_cast<void* volatile*>(static_cast<unsigned char*>(storage) + offset) = value;
}

template<class Shader>
void save_release(void* owner) {
    UINT bytes = static_cast<UINT>(reinterpret_cast<std::uintptr_t>(owner));
    auto* const captured = static_cast<Shader*>(load_pointer(owner, 8));
    if (captured == nullptr) return;
    (void)captured->AddRef();
    (void)captured->Release();
    auto* const query_shader = static_cast<Shader*>(load_pointer(owner, 8));
    (void)query_shader->GetFunction(nullptr, &bytes);
    void* const bytecode = singleton_lifetime_allocate({
        SingletonAllocationKind::object, bytes, bytes});
    auto* const copy_shader = static_cast<Shader*>(load_pointer(owner, 8));
    store_pointer(owner, 12, bytecode);
    (void)copy_shader->GetFunction(bytecode, &bytes);
    auto* const released = static_cast<Shader*>(load_pointer(owner, 8));
    if (released != nullptr) {
        (void)released->Release();
        store_pointer(owner, 8, nullptr);
    }
}

template<bool Pixel>
void restore(void* owner, const void* actual_renderer_global) {
    if (load_pointer(owner, 8) != nullptr) return;
    if (load_pointer(owner, 12) == nullptr) return;
    auto* const device = static_cast<IDirect3DDevice9*>(
        get_native_renderer_device_00b1fef0(load_pointer(actual_renderer_global, 0)));
    const auto* const bytecode = static_cast<const DWORD*>(load_pointer(owner, 12));
    auto* const output = static_cast<unsigned char*>(owner) + 8;
    if constexpr (Pixel) {
        (void)device->CreatePixelShader(bytecode,
            reinterpret_cast<IDirect3DPixelShader9**>(output));
    } else {
        (void)device->CreateVertexShader(bytecode,
            reinterpret_cast<IDirect3DVertexShader9**>(output));
    }
    singleton_lifetime_free(load_pointer(owner, 12));
    // Complete returning-free tails B5E8C7..D0 and B5E917..20.
    store_pointer(owner, 12, nullptr);
}

} // namespace

void* get_native_renderer_device_00b1fef0(const void* renderer) noexcept {
    return load_pointer(renderer, 0x1a10);
}
void save_release_native_pixel_shader_00b5e750(void* owner) {
    save_release<IDirect3DPixelShader9>(owner);
}
void save_release_native_vertex_shader_00b5e810(void* owner) {
    save_release<IDirect3DVertexShader9>(owner);
}
void restore_native_pixel_shader_00b5e890(void* owner, const void* actual_renderer_global) {
    restore<true>(owner, actual_renderer_global);
}
void restore_native_vertex_shader_00b5e8e0(void* owner, const void* actual_renderer_global) {
    restore<false>(owner, actual_renderer_global);
}

} // namespace bsp
