#include "bsp/native_renderer_surface_save_worker.hpp"

#include <cstddef>
#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Renderer surface-save worker reconstruction requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(NativeRendererSurfaceSaveBinding) == 8);
static_assert(offsetof(NativeRendererSurfaceSaveBinding, actual_fallback_bytes_0108fe98) == 4);
const NativeRendererSurfaceSaveBinding* application_binding;

// The original C2DFF8/CE23F4 edge is a concrete named external import.
// No validation/fallback is inserted into a running native worker.
HRESULT __stdcall save_surface_import(LPCSTR filename, DWORD format,
    IDirect3DSurface9* surface, const PALETTEENTRY* palette, const RECT* rectangle) {
    return application_binding->save_import->save(filename, format, surface, palette, rectangle);
}
} // namespace

NativeD3dx9SurfaceSaveImport::NativeD3dx9SurfaceSaveImport(HMODULE module) {
    if (module == nullptr) throw std::invalid_argument("actual d3dx9_40 module required");
    const FARPROC address = GetProcAddress(module, "D3DXSaveSurfaceToFileA");
    if (address == nullptr) throw std::runtime_error("actual d3dx9_40 save export required");
    static_assert(sizeof(address) == sizeof(function_));
    std::memcpy(&function_, &address, sizeof(function_));
}

HRESULT NativeD3dx9SurfaceSaveImport::save(LPCSTR filename, DWORD format,
    IDirect3DSurface9* surface, const PALETTEENTRY* palette, const RECT* rectangle) const {
    return function_(filename, format, surface, palette, rectangle);
}

void bind_native_renderer_surface_save_application(
    const NativeRendererSurfaceSaveBinding* binding) noexcept {
    application_binding = binding;
}

__declspec(naked) IDirect3DSurface9* __fastcall
acquire_native_renderer_surface_save_surface_00b5e380(void*, void*, IDirect3DDevice9*) {
    __asm {
        sub esp, 28h
        push esi
        mov esi, ecx
        cmp dword ptr [esi + 50h], 0
        push edi
        jne native_00b5e3ad
        push 0
        push 0
        push esi
        push offset native_renderer_surface_save_thread_entry_00b5e230
        push 0
        push 0
        call dword ptr [CreateThread]
        push 0
        push eax
        mov dword ptr [esi + 50h], eax
        call dword ptr [SetThreadPriority]
    native_00b5e3ad:
        mov edi, dword ptr [esi + 48h]
        push ebx
        mov ebx, dword ptr [EnterCriticalSection]
        push ebp
        push edi
        call ebx
        add dword ptr [edi + 18h], 1
        mov eax, dword ptr [esi + 40h]
        mov ebp, dword ptr [LeaveCriticalSection]
        mov dword ptr [esp + 10h], eax
        mov eax, dword ptr [esi + 48h]
        add dword ptr [eax + 18h], -1
        push eax
        call ebp
        mov edi, dword ptr [esi + 48h]
        push edi
        call ebx
        add dword ptr [edi + 18h], 1
        mov eax, dword ptr [esi + 48h]
        mov edi, dword ptr [esi + 3ch]
        add dword ptr [eax + 18h], -1
        push eax
        call ebp
        mov eax, dword ptr [esp + 10h]
        add eax, 1
        cdq
        mov ecx, 5
        idiv ecx
        cmp edx, edi
        mov dword ptr [esp + 14h], edx
        jne native_00b5e429
    native_00b5e404:
        push 0ah
        call dword ptr [Sleep]
        mov edi, dword ptr [esi + 48h]
        push edi
        call ebx
        add dword ptr [edi + 18h], 1
        mov eax, dword ptr [esi + 48h]
        mov edi, dword ptr [esi + 3ch]
        add dword ptr [eax + 18h], -1
        push eax
        call ebp
        cmp dword ptr [esp + 14h], edi
        je native_00b5e404
    native_00b5e429:
        mov edx, dword ptr [esp + 10h]
        cmp dword ptr [esi + edx*4], 0
        lea esi, [esi + edx*4]
        pop ebp
        pop ebx
        jne native_00b5e484
        mov edi, dword ptr [esp + 34h]
        mov eax, dword ptr [edi]
        mov edx, dword ptr [eax + 48h]
        lea ecx, [esp + 8]
        push ecx
        push 0
        push 0
        push 0
        push edi
        mov dword ptr [esp + 1ch], 0
        call edx
        mov eax, dword ptr [esp + 8]
        mov ecx, dword ptr [eax]
        lea edx, [esp + 10h]
        push edx
        push eax
        mov eax, dword ptr [ecx + 30h]
        call eax
        mov edx, dword ptr [esp + 2ch]
        mov eax, dword ptr [esp + 28h]
        mov ecx, dword ptr [edi]
        mov ecx, dword ptr [ecx + 90h]
        push 0
        push esi
        push 2
        push 15h
        push edx
        push eax
        push edi
        call ecx
    native_00b5e484:
        mov eax, dword ptr [esi]
        pop edi
        pop esi
        add esp, 28h
        ret 4
    }
}

__declspec(naked) DWORD __fastcall
run_native_renderer_surface_save_worker_00b5e0c0(void*) {
    __asm {
        push ecx
        push ebx
        push ebp
        push esi
        mov esi, ecx
        mov ebx, dword ptr [esi + 4ch]
        push edi
        mov edi, dword ptr [EnterCriticalSection]
        push ebx
        call edi
        add dword ptr [ebx + 18h], 1
        mov eax, dword ptr [esi + 4ch]
        mov bl, byte ptr [esi + 44h]
        mov ebp, dword ptr [LeaveCriticalSection]
        add dword ptr [eax + 18h], -1
        push eax
        call ebp
        test bl, bl
        jne native_00b5e208
    native_00b5e0f2:
        mov ebx, dword ptr [esi + 48h]
        push ebx
        call edi
        add dword ptr [ebx + 18h], 1
        mov eax, dword ptr [esi + 48h]
        mov ebx, dword ptr [esi + 3ch]
        add dword ptr [eax + 18h], -1
        push eax
        call ebp
        mov eax, dword ptr [esi + 48h]
        push eax
        mov dword ptr [esp + 14h], eax
        call edi
        mov eax, dword ptr [esp + 10h]
        add dword ptr [eax + 18h], 1
        mov eax, dword ptr [esi + 40h]
        mov dword ptr [esp + 10h], eax
        mov eax, dword ptr [esi + 48h]
        add dword ptr [eax + 18h], -1
        push eax
        call ebp
        mov eax, dword ptr [esp + 10h]
        sub eax, ebx
        add eax, 5
        cdq
        mov ecx, 5
        idiv ecx
        test edx, edx
        jle native_00b5e1e1
        jmp native_00b5e154
        _emit 0xeb
        _emit 0x07
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
    native_00b5e150:
        mov ebx, dword ptr [esp + 10h]
    native_00b5e154:
        mov eax, dword ptr [esi + ebx*8 + 18h]
        test eax, eax
        jne native_00b5e161
        // Original immediate0108FE98, supplied as the same mutable byte address.
        mov eax, dword ptr [application_binding]
        mov eax, dword ptr [eax + 4]
    native_00b5e161:
        mov edx, dword ptr [esi + ebx*4]
        push 0
        push 0
        push edx
        push 0
        push eax
        call save_surface_import
        mov ebx, dword ptr [esi + 48h]
        push ebx
        call edi
        add dword ptr [ebx + 18h], 1
        mov eax, dword ptr [esi + 3ch]
        add eax, 1
        cdq
        mov ecx, 5
        idiv ecx
        mov eax, dword ptr [esi + 48h]
        push eax
        mov dword ptr [esi + 3ch], edx
        add dword ptr [eax + 18h], -1
        call ebp
        mov ebx, dword ptr [esi + 48h]
        push ebx
        call edi
        add dword ptr [ebx + 18h], 1
        mov eax, dword ptr [esi + 48h]
        mov edx, dword ptr [esi + 3ch]
        add dword ptr [eax + 18h], -1
        push eax
        mov dword ptr [esp + 14h], edx
        call ebp
        mov ebx, dword ptr [esi + 48h]
        push ebx
        call edi
        add dword ptr [ebx + 18h], 1
        mov eax, dword ptr [esi + 48h]
        mov ebx, dword ptr [esi + 40h]
        add dword ptr [eax + 18h], -1
        push eax
        call ebp
        mov eax, ebx
        sub eax, dword ptr [esp + 10h]
        mov ecx, 5
        add eax, 5
        cdq
        idiv ecx
        test edx, edx
        jg native_00b5e150
    native_00b5e1e1:
        push 0ah
        call dword ptr [Sleep]
        mov ebx, dword ptr [esi + 4ch]
        push ebx
        call edi
        add dword ptr [ebx + 18h], 1
        mov eax, dword ptr [esi + 4ch]
        mov bl, byte ptr [esi + 44h]
        add dword ptr [eax + 18h], -1
        push eax
        call ebp
        test bl, bl
        je native_00b5e0f2
    native_00b5e208:
        mov ebx, dword ptr [esi + 4ch]
        push ebx
        call edi
        add dword ptr [ebx + 18h], 1
        mov byte ptr [esi + 45h], 1
        mov esi, dword ptr [esi + 4ch]
        add dword ptr [esi + 18h], -1
        push esi
        call ebp
        pop edi
        pop esi
        pop ebp
        xor eax, eax
        pop ebx
        pop ecx
        ret
    }
}

__declspec(naked) DWORD WINAPI
native_renderer_surface_save_thread_entry_00b5e230(void*) {
    __asm {
        mov ecx, dword ptr [esp + 4]
        call run_native_renderer_surface_save_worker_00b5e0c0
        ret 4
    }
}

}
