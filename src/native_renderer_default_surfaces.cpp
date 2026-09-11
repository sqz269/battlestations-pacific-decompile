#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include "bsp/native_renderer_default_surfaces.hpp"
#include "bsp/native_renderer_surface_bindings.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);

Word word(const void* base, Word byte_offset = 0) noexcept {
    Word value;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov value, eax
    }
    return value;
}
void put(void* base, Word byte_offset, Word value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
template<class T> T pointer(Word value) noexcept { return reinterpret_cast<T>(value); }
template<class Method> Method method(const void* captured, Word slot) noexcept {
    const auto table = pointer<const void*>(word(captured));
    return pointer<Method>(word(table, slot));
}
using Reference = ULONG (STDMETHODCALLTYPE*)(void*);
using GetColor = HRESULT (STDMETHODCALLTYPE*)(void*, DWORD, IDirect3DSurface9**);
using GetDepth = HRESULT (STDMETHODCALLTYPE*)(void*, IDirect3DSurface9**);
using GetDescription = HRESULT (STDMETHODCALLTYPE*)(void*, D3DSURFACE_DESC*);

void captured_pair(void* captured) {
    (void)method<Reference>(captured, 4)(captured);
    // A real AddRef callback may replace this captured object's current table.
    (void)method<Reference>(captured, 8)(captured);
}
volatile LONG* references(void* owner) noexcept {
    return pointer<volatile LONG*>(reinterpret_cast<Word>(owner) + 4u);
}
const Word* current_profile(const void* owner,
    const NativeRendererDefaultSurfacesContext& context) noexcept {
    const Word identity = word(owner);
    __assume(identity == 0x00d619a0);
    return context.original_surface_profile_00d619a0;
}
void final_zero(NativeSurfaceOwnerStorage* captured,
    const NativeRendererDefaultSurfacesContext& context) {
    const Word invoker = word(current_profile(captured, context));
    __assume(invoker == 0x00bd30e0);
    // Full BD30E0 reads a fresh profile for its +4 scalar-delete selector.
    const Word terminal = word(current_profile(captured, context), 4);
    __assume(terminal == 0x00b3f5b0);
    (void)delete_native_surface_00b3f5b0(*captured, 1, context.surface_owner);
}
struct ConstructorCleanup {
    void* volatile& current_raw_slot;
    bool armed = true;
    ~ConstructorCleanup() noexcept {
        if (armed) return_d3d9_surface_slot_00b3dcc0(current_raw_slot);
    }
};
void publish_and_drop_temporary(void* renderer, Word offset,
    NativeSurfaceOwnerStorage* captured_old, NativeSurfaceOwnerStorage* incoming,
    const NativeRendererDefaultSurfacesContext& context) {
    if (captured_old != incoming) {
        put(renderer, offset, reinterpret_cast<Word>(incoming));
        if (incoming) InterlockedIncrement(references(incoming));
        if (captured_old && InterlockedDecrement(references(captured_old)) == 0)
            final_zero(captured_old, context);
    }
    // Native is unconditional, including a null allocator result (+4 fault).
    if (InterlockedDecrement(references(incoming)) == 0) final_zero(incoming, context);
}
} // namespace

void __fastcall capture_native_renderer_default_surfaces_00b238d0(
    void* renderer, const NativeRendererDefaultSurfacesContext* context) {
    void* captured_device = pointer<void*>(word(renderer, 0x1a10));
    if (captured_device) captured_pair(captured_device);
    void* current_device = pointer<void*>(word(renderer, 0x1a10));
    IDirect3DSurface9* volatile color_output = nullptr;
    const auto get_color = method<GetColor>(current_device, 0x98);
    (void)get_color(current_device, 0, const_cast<IDirect3DSurface9**>(&color_output));

    IDirect3DSurface9* color = color_output;
    if (color) {
        captured_pair(color);
        color = color_output;
    }
    D3DSURFACE_DESC unused_description;
    (void)method<GetDescription>(color, 0x30)(color, &unused_description);

    void* allocation = allocate_d3d9_surface_slot_00b3f2a0();
    void* volatile raw_slot = allocation;
    NativeSurfaceOwnerStorage* incoming;
    NativeSurfaceOwnerStorage* old;
    {
        ConstructorCleanup cleanup{raw_slot}; // Native state 0, current spill.
        incoming = allocation ? construct_native_surface_00b3f630(
            allocation, color_output, 0, 0, context->surface_owner) : nullptr;
        old = pointer<NativeSurfaceOwnerStorage*>(word(renderer, 0x197c));
        cleanup.armed = false; // Capture old first; native state becomes -1.
    }
    publish_and_drop_temporary(renderer, 0x197c, old, incoming, *context);

    color = color_output;
    if (color) {
        captured_pair(color);
        color = color_output;
    }
    (void)method<Reference>(color, 8)(color);

    captured_device = pointer<void*>(word(renderer, 0x1a10));
    if (captured_device) captured_pair(captured_device);
    current_device = pointer<void*>(word(renderer, 0x1a10));
    IDirect3DSurface9* volatile depth_output = nullptr;
    const auto get_depth = method<GetDepth>(current_device, 0xa0);
    (void)get_depth(current_device, const_cast<IDirect3DSurface9**>(&depth_output));

    allocation = allocate_d3d9_surface_slot_00b3f2a0();
    raw_slot = allocation;
    {
        ConstructorCleanup cleanup{raw_slot}; // Native state 1, same raw spill.
        incoming = allocation ? construct_native_surface_00b3f630(
            allocation, depth_output, 0, 0, context->surface_owner) : nullptr;
        old = pointer<NativeSurfaceOwnerStorage*>(word(renderer, 0x198c));
        cleanup.armed = false;
    }
    publish_and_drop_temporary(renderer, 0x198c, old, incoming, *context);

    // Native depth cleanup releases first, then reloads for a conditional pair.
    IDirect3DSurface9* depth = depth_output;
    (void)method<Reference>(depth, 8)(depth);
    depth = depth_output;
    if (depth) captured_pair(depth);

    const auto* current_depth = pointer<NativeSurfaceOwnerStorage*>(word(renderer, 0x198c));
    bind_native_renderer_depth_surface_00b21690(renderer, current_depth, context->synchronization);
}
} // namespace bsp
