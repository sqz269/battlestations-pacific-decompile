#include "bsp/native_renderer_surface_save_publish.hpp"

#include <cstddef>
#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Renderer surface-save publication requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Context = NativeRendererSurfaceSavePublishContext;
static_assert(sizeof(void*) == 4 && sizeof(Context) == 12);
static_assert(sizeof(D3DSURFACE_DESC) == 32 && offsetof(D3DSURFACE_DESC, Width) == 24);
static_assert(offsetof(D3DSURFACE_DESC, Height) == 28 && sizeof(D3DLOCKED_RECT) == 8);
static_assert(offsetof(D3DLOCKED_RECT, pBits) == 4);

void __fastcall resize_header(void* header, const Context* context,
    std::uint32_t length, std::uint32_t preserve) {
    resize_native_string_header_0041dd40(header, *context->strings, length, preserve != 0);
}

// Same explicit valid-range/zero-omission boundary as other raw string copies.
// Original BF7680 supports backwards overlap for this persistent destination.
void __cdecl copy_header_bytes(void* destination, const void* source, std::size_t length) {
    if (length != 0) std::memmove(destination, source, length);
}

void* __fastcall selected_surface_field(void* renderer, const Context* context,
    std::uint32_t captured_profile, std::uint32_t ignored_scalar) noexcept {
    __assume(captured_profile == 0x00d5f0a8);
    std::uint32_t selector;
    const volatile void* const profile = context->actual_renderer_profile_00d5f0a8;
    __asm { mov eax, profile }
    __asm { mov eax, dword ptr [eax + 128h] }
    __asm { mov selector, eax }
    __assume(selector == 0x00b24dc0);
    return native_renderer_field197c_00b24dc0(renderer, nullptr, ignored_scalar);
}

HRESULT __fastcall load_surface_import(const Context* context, void*,
    IDirect3DSurface9* destination, const PALETTEENTRY* destination_palette,
    const RECT* destination_rectangle, IDirect3DSurface9* source,
    const PALETTEENTRY* source_palette, const RECT* source_rectangle,
    DWORD filter, D3DCOLOR color_key) {
    return context->load_import->load(destination, destination_palette,
        destination_rectangle, source, source_palette, source_rectangle, filter, color_key);
}
} // namespace

NativeD3dx9SurfaceLoadImport::NativeD3dx9SurfaceLoadImport(HMODULE module) {
    if (module == nullptr) throw std::invalid_argument("actual d3dx9_40 module required");
    const FARPROC address = GetProcAddress(module, "D3DXLoadSurfaceFromSurface");
    if (address == nullptr) throw std::runtime_error("actual d3dx9_40 load export required");
    static_assert(sizeof(address) == sizeof(function_));
    std::memcpy(&function_, &address, sizeof(function_));
}

HRESULT NativeD3dx9SurfaceLoadImport::load(IDirect3DSurface9* destination,
    const PALETTEENTRY* destination_palette, const RECT* destination_rectangle,
    IDirect3DSurface9* source, const PALETTEENTRY* source_palette,
    const RECT* source_rectangle, DWORD filter, D3DCOLOR color_key) const {
    return function_(destination, destination_palette, destination_rectangle,
        source, source_palette, source_rectangle, filter, color_key);
}

__declspec(naked) void* __fastcall native_renderer_field197c_00b24dc0(
    const void*, void*, std::uint32_t) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 197ch]
        ret 4
    }
}

__declspec(naked) void __fastcall publish_native_renderer_surface_save_00b5e490(
    void*, const NativeRendererSurfaceSavePublishContext*, const void*) {
    __asm {
        // Preserve fixed EDX context in an added nonvolatile save.
        push ebp
        mov ebp, edx
        push ebx
        push esi
        push edi
        mov esi, ecx
        mov edi, dword ptr [esi + 48h]
        push edi
        call dword ptr [EnterCriticalSection]
        add dword ptr [edi + 18h], 1
        mov eax, dword ptr [esi + 40h]
        mov ebx, dword ptr [esp + 14h]
        lea edi, [esi + eax*8 + 14h]
        cmp edi, ebx
        je native_00b5e4d6
        mov ecx, dword ptr [ebx]
        push 1
        push ecx
        mov ecx, edi
        mov edx, ebp
        call resize_header
        cmp dword ptr [ebx], 0
        je native_00b5e4d6
        mov edx, dword ptr [edi]
        mov eax, dword ptr [ebx + 4]
        mov ecx, dword ptr [edi + 4]
        push edx
        push eax
        push ecx
        call copy_header_bytes
        add esp, 0ch
    native_00b5e4d6:
        mov eax, dword ptr [esi + 40h]
        add eax, 1
        cdq
        mov ecx, 5
        idiv ecx
        mov dword ptr [esi + 40h], edx
        mov esi, dword ptr [esi + 48h]
        add dword ptr [esi + 18h], -1
        push esi
        call dword ptr [LeaveCriticalSection]
        pop edi
        pop esi
        pop ebx
        pop ebp
        ret 4
    }
}

__declspec(naked) void __fastcall capture_native_renderer_surface_save_00b23c50(
    void*, const NativeRendererSurfaceSavePublishContext*, const void*) {
    __asm {
        // Preserve fixed EDX context in an added nonvolatile save.
        push ebp
        mov ebp, edx
        sub esp, 28h
        push ebx
        mov ebx, dword ptr [esp + 34h]
        cmp dword ptr [ebx], 0
        push esi
        mov esi, ecx
        je native_00b23d33
        mov eax, dword ptr [esi + 1a10h]
        push edi
        lea edi, [esi + 1d2ch]
        push eax
        mov ecx, edi
        call acquire_native_renderer_surface_save_surface_00b5e380
        mov edx, dword ptr [esi]
        mov dword ptr [esi + 1d24h], eax
        // Save profile identity captured before the surface publication.
        push 0
        push edx
        mov edx, ebp
        mov ecx, esi
        call selected_surface_field
        mov ecx, dword ptr [eax + 2ch]
        mov edx, dword ptr [esi + 1d24h]
        push 0
        push 1
        push 0
        push 0
        push ecx
        push 0
        push 0
        push edx
        mov ecx, ebp
        call load_surface_import
        mov eax, dword ptr [esi + 1d24h]
        mov ecx, dword ptr [eax]
        lea edx, [esp + 14h]
        push edx
        push eax
        mov eax, dword ptr [ecx + 30h]
        call eax
        mov eax, dword ptr [esi + 1d24h]
        mov ecx, dword ptr [eax]
        push 0
        push 0
        lea edx, [esp + 14h]
        push edx
        push eax
        mov eax, dword ptr [ecx + 34h]
        call eax
        mov edx, dword ptr [esp + 30h]
        imul edx, dword ptr [esp + 2ch]
        mov eax, dword ptr [esp + 10h]
        xor ecx, ecx
        test edx, edx
        jbe native_00b23d09
        jmp native_00b23cf0
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x90
    native_00b23cf0:
        or dword ptr [eax], 0ff000000h
        mov edx, dword ptr [esp + 30h]
        imul edx, dword ptr [esp + 2ch]
        add ecx, 1
        add eax, 4
        cmp ecx, edx
        jb native_00b23cf0
    native_00b23d09:
        mov eax, dword ptr [esi + 1d24h]
        mov ecx, dword ptr [eax]
        mov edx, dword ptr [ecx + 38h]
        push eax
        call edx
        mov esi, dword ptr [esi + 1d24h]
        mov eax, dword ptr [esi]
        mov edx, dword ptr [eax + 30h]
        lea ecx, [esp + 14h]
        push ecx
        push esi
        call edx
        push ebx
        mov ecx, edi
        mov edx, ebp
        call publish_native_renderer_surface_save_00b5e490
        pop edi
    native_00b23d33:
        pop esi
        pop ebx
        add esp, 28h
        pop ebp
        ret 4
    }
}

} // namespace bsp
