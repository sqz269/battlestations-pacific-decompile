#pragma once

#include "bsp/native_renderer_surface_save_worker.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <cstdint>

namespace bsp {

// Concrete original C2DFD4/CE240C import boundary. Borrow the caller-owned
// actual d3dx9_40 module through every synchronous load call. Resolve exactly
// D3DXLoadSurfaceFromSurface; no DLL load/search/version fallback or callable
// setter. Resolution errors throw during host construction, before raw calls.
class NativeD3dx9SurfaceLoadImport final {
public:
    explicit NativeD3dx9SurfaceLoadImport(HMODULE actual_d3dx9_40);
    NativeD3dx9SurfaceLoadImport(const NativeD3dx9SurfaceLoadImport&) = delete;
    NativeD3dx9SurfaceLoadImport& operator=(const NativeD3dx9SurfaceLoadImport&) = delete;
    HRESULT load(IDirect3DSurface9* destination, const PALETTEENTRY* destination_palette,
        const RECT* destination_rectangle, IDirect3DSurface9* source,
        const PALETTEENTRY* source_palette, const RECT* source_rectangle,
        DWORD filter, D3DCOLOR color_key) const;
private:
    using Function = HRESULT (WINAPI*)(IDirect3DSurface9*, const PALETTEENTRY*,
        const RECT*, IDirect3DSurface9*, const PALETTEENTRY*, const RECT*, DWORD, D3DCOLOR);
    Function function_;
};

// New fixed context ABI, never owner-tail storage or an original global.
// Borrow the actual owning string-pool publication/gate/lifetime adapter and
// concrete import. All remain alive through the synchronous raw call. Existing
// surface-save application binding remains installed until EVERY worker returns.
// The profile token read from the renderer must be D5F0A8. Borrow its entire
// required [D5F0A8,D5F1D4) window: minimum/full required span 0x12C bytes. This
// does not assert the full vtable allocation extent. The reached current +128
// DWORD must remain B24DC0; it is read after surface publication, using the
// profile identity captured before publication. Other profiles are outside this
// fixed mapping's domain, not dispatched through callbacks or original code.
struct NativeRendererSurfaceSavePublishContext {
    ActualNativeStringPoolStorage* const strings;
    const NativeD3dx9SurfaceLoadImport* const load_import;
    const volatile void* const actual_renderer_profile_00d5f0a8;
};

// Full B5E490[107]. Original ECX actual54h-prefix worker owner, stack source
// eight-byte raw string header, RET4, no semantic result. New EDX context.
// Context/pool is reached only for nonidentical destination/source headers.
// Enter captured lock48; resize with full41DD40 preserve=true; reread current
// fields for overlap-safe copy; advance CURRENT producer with wrapped +1 and
// signed remainder5; leave CURRENT lock48. No EH unlock/rollback. Source and
// current destination buffers must form valid nonwrapping byte ranges. The
// host copy uses memmove and omits zero bytes; it does not expose CRT BF7680's
// ISA/global/partial-fault ABI. Destination header remains captured across calls.
void __fastcall publish_native_renderer_surface_save_00b5e490(void* actual_owner,
    const NativeRendererSurfaceSavePublishContext*, const void* actual_source_header);

// Full B23C50[235]. Original ECX raw renderer, stack source header, RET4;
// new EDX context. Zero source length returns before any renderer/context read.
// Nonzero requires actual renderer prefix through1D80, embedded actual worker
// at1D2C, current real device/surfaces/COM tables, and the fixed profile above.
// Full acquire, profile getter, concrete D3DX load, GetDesc/LockRect/alpha loop/
// UnlockRect/final GetDesc, then full publisher. Ignore HRESULTs and Pitch;
// contiguous DWORD loop reloads wrapped Width*Height after each store. No
// release of the acquired backbuffer/surface or extra failure policy is added.
void __fastcall capture_native_renderer_surface_save_00b23c50(void* actual_renderer,
    const NativeRendererSurfaceSavePublishContext*, const void* actual_source_header);

// Full B24DC0[9]. Original ECX renderer, ignored stack scalar, EAX current
// DWORD197C, RET4. Reserved EDX has no meaning. Raw field getter only: no
// inferred owner, reference count, acquisition, or backbuffer selection.
void* __fastcall native_renderer_field197c_00b24dc0(const void* actual_renderer,
    void* reserved_edx, std::uint32_t ignored_scalar) noexcept;

// Native SEH/stack-unwind and drop-in caller ABI are not exposed by this new
// context interface. Source/readable-header, queue-index, lock, buffer, profile,
// module and worker lifetimes are explicit valid-domain preconditions.
} // namespace bsp
