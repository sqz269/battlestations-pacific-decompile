#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <d3d9.h>

namespace bsp {

// Concrete external import boundary. The caller owns the actual d3dx9_40
// module and keeps it loaded until all worker threads return. Construction
// resolves only D3DXSaveSurfaceToFileA; no DLL load/search/version fallback or
// arbitrary callable setter. Resolution failure throws before any raw launch.
class NativeD3dx9SurfaceSaveImport final {
public:
    explicit NativeD3dx9SurfaceSaveImport(HMODULE actual_d3dx9_40);
    NativeD3dx9SurfaceSaveImport(const NativeD3dx9SurfaceSaveImport&) = delete;
    NativeD3dx9SurfaceSaveImport& operator=(const NativeD3dx9SurfaceSaveImport&) = delete;
    HRESULT save(LPCSTR filename, DWORD format, IDirect3DSurface9* surface,
        const PALETTEENTRY* palette, const RECT* rectangle) const;
private:
    using Function = HRESULT (WINAPI*)(LPCSTR, DWORD, IDirect3DSurface9*,
        const PALETTEENTRY*, const RECT*);
    Function function_;
};

// One application/process binding, not per-owner storage. Both pointers and
// their objects stay fixed/alive from installation through EVERY thread return.
// The actual fallback bytes remain mutable and readable through their NUL.
// 0108FE98 is an immediate byte address, never a pointer-variable dereference;
// neither the captured four zeros nor this interface declares its full extent.
struct NativeRendererSurfaceSaveBinding {
    const NativeD3dx9SurfaceSaveImport* const save_import;
    char* const actual_fallback_bytes_0108fe98;
};

// Host plumbing only. Install a valid binding before any launch; nullptr is
// permitted only when no worker can access it. No concurrent install/uninstall.
// Unbound calls are outside the raw API domain, not original validation paths.
void bind_native_renderer_surface_save_application(
    const NativeRendererSurfaceSaveBinding* binding) noexcept;

// Full B5E380[270]: ECX actual54h-prefix owner, stack current device, EAX
// producer surface, RET4. The reserved EDX C++ parameter is ignored. Launch uses
// CreateThread with the raw owner unchanged, then handle publication/priority.
// No stop-byte reset, HRESULT repair, surface release, or producer publication.
IDirect3DSurface9* __fastcall acquire_native_renderer_surface_save_surface_00b5e380(
    void* actual_owner, void* reserved_edx, IDirect3DDevice9* actual_device);

// Full B5E0C0[360]: ECX actual owner, EAX0, RET. Drain the five raw surface/name
// slots using current lock/index fields; call the concrete import with format0;
// test stop only outside the drain and publish byte45 before final lock leave.
DWORD __fastcall run_native_renderer_surface_save_worker_00b5e0c0(void* actual_owner);

// Full B5E230[12]: actual Win32 thread entry; stack raw owner, EAX0, RET4.
DWORD WINAPI native_renderer_surface_save_thread_entry_00b5e230(void* actual_owner);

// Requires valid current locks, queue indices/storage, COM objects and readable
// filenames; the raw routines add no recovery or ownership policy. The separate
// full publisher B5E490 and complete renderer lifetime remain outside this file.
}
