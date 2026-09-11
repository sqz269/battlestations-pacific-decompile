# Native renderer resource initialization discovery

`00B2AEB0..00B2B1F2` is an 834-byte complete native body, still represented by two typed fragments. It is **not ready for full raw reconstruction**. The next independent source candidate is `00B238D0..00B23B02`, the 562-byte default-surface capture, under the explicit actual-storage/profile contract below. This packet changes no source, Ghidra definitions, shared metadata, or installed files.

Evidence is pinned in [the report](../reports/native_renderer_resource_init_discovery.json) and the preserved ignored `local/resource_init/` bundle in the discovery worktree. The 34 freshly guarded live/installed-PE spans total 1,872 bytes, including both complete bodies. Every `bsp.py ghidra bytes` command verifies `bsp.gpr` and `/battlestationspacific.exe` before querying. Full parent byte SHA-256: `656d7fc3fed03e6cae9b92c3fd866aa362d6dc022c573f25e9260121408bd134`. Identity prefixes for incomplete dependencies are explicitly not full-body proofs. No build or runtime test was required for this read-only discovery.

## B2AEB0 ABI and current storage

Native ECX is the actual renderer; the ten DWORD argument slots at entry ESP+4 through ESP+28h are HWND, fullscreen (low byte), width, height, back-buffer format, back-buffer count, multisample type, depth format, presentation sync, and fullscreen refresh rate. The function uses FH3 and returns with `RET 28h`; it has no recovered HRESULT contract. EAX is incidental after the final reset call. Arguments are actual callee slots, with later reads after external calls, not a stable options snapshot.

The renderer reads/writes actual fields: physical vertex/index wrappers +1974/+1978, default surface wrappers +197C/+198C in the child, IDirect3D9 +1990, current device/output +1A10, widescreen byte +1A14, requested width/height +1A20/+1A24, 38h-byte presentation parameters +1A28, and bytes +1D8A/+1D8B/+1D8C. Renderer profile D5F0A8 has slot +4 = B2AEB0 and +F0 = B21960. These locations do not establish compatibility with semantic renderer classes.

The full order is:

1. Call original memset BF79F0 on +1A28 for 38h bytes, then interleave actual argument loads and presentation stores. Store windowed=1 initially, swap effect=1, flags=2, quality=0; auto-depth is depth-format != 0. Presentation interval is `~(sync << 31) & 80000000h`, so only the argument's low bit matters. Store dimensions both in the presentation record and +1A20/+1A24.
2. Capture current +1990 for GetDeviceCaps (vtable +38, adapter 0, HAL 1). The stack caps record is uninitialized before the call, and HRESULT is ignored. Use caps DevCaps bit 10000h and low WORD VertexShaderVersion >= 0101h for hardware vertex processing 40h, otherwise 20h; OR 4 for multithreaded behavior. Reload current +1990 and vtable +40 for CreateDevice, using actual +1A10 as an output slot without first clearing it, and the mutable +1A28 presentation record.
3. Ignore CreateDevice HRESULT; compare the current fullscreen callee byte to zero, store lost +1D8A=0, form the comparison result, store initialized +1D8B=1, and replace only Windowed with the fullscreen byte's logical negation. Record actual GetCurrentThreadId into 108D4C8, then call actual timeBeginPeriod(1). At B2B02C compare the current **multisample argument**, entry ESP+1Ch (settled ESP+174h), to zero and set render state A1h accordingly. This is not the depth-format slot.
4. Capture defaults B238D0, set default states B26170, and set stream frequencies B24A40 for streams 0 through 3 to 1, in that order.
5. Allocate from the actual vertex pool through B4B360 and construct B4BBB0 if nonnull. Publish +1974. Create a 16 MiB VB with usage 208h, FVF=0, default pool, shared handle null, and an initially null actual stack output. Ignore HRESULT. Reload current wrapper +1974/profile/slot +14, attach current output with flags 1000h and capacity 1000000h, then reload and unconditionally Release the current output cell. Repeat for index pool B4B350, constructor B4BB60, +1978, 1 MiB IB, format 65h, and capacity 100000h.
6. FILD the current width callee slot, separately reload/test its integer word, store ready +1D8C=1, and add float 2^32 at CE3978 when signed-negative. Load the height integer, FILD its current slot, then test the captured integer for the same correction. Divide width/height and compare to the double at CF5750 (`1.3333333730697632`) using the original x87 order. Unordered takes false; x87 status/traps and the separate reads matter. Capture current renderer profile, prepare gamma +0 via FLDZ/FSTP, store strict ordered-greater into +1A14, then load/call captured profile slot +F0. Store global byte 108D4B8=1, then call full device reset B2ABD0 before the FH3 epilogue.

No HRESULT rollback, nonempty-output rejection, or outer optional renderer guard exists in this body. Provider guards retain their own scopes. `src/d3d9_startup.cpp` exposes `d3d9_create_device_prefix_00b2aeb0` using a stable API/options record, initialized caps, and checked HRESULTs. `src/d3d9_resources.cpp` exposes `create_dynamic_buffers_00b2aeb0` with semantic COM owners, nonempty rejection and failure cleanup. Both remain useful existing APIs, but neither is this full raw body.

## Dependency readiness

| Native target | Current contract and remaining boundary |
| --- | --- |
| B24460, B26170 | Complete raw state/default-state providers in `native_renderer_cached_states.cpp`; borrow actual `NativeRendererSynchronizationGlobals`. |
| B24A40 | Complete raw stream-frequency provider in `native_renderer_stream_frequency.cpp`, including its actual guard. |
| B4BBB0, B4BB60 | Complete 2Ch raw pooled wrapper constructors in `native_physical_buffer_owner.cpp`; install D61E7C and D61E58 respectively. |
| Current wrapper +14 | Concrete pooled profiles resolve to full B4C370/B4C250 Attach providers in the same physical-owner source. Require the existing `NativePhysicalBufferOwnerContext` with actual string/support/lifetime and index/vertex pool storage. A callback may change current wrapper/profile before this dispatch; a future source must declare its admitted profile domain. |
| B4B360, B4B350 -> B4AC60 | **Incomplete raw allocation dependency at the pinned base.** Each complete 10-byte thunk replaces ECX with actual pool 108FDE0/108FDA8 and jumps to B4AC60. The caller's 2Ch is not an allocator policy. B4AC60 uses actual CS/current pool fields, CRT allocation/free, 644h slabs and B48DF0. Its returning-free hidden tail needs full assembly reconstruction. Primary has separately claimed B4AC60/B48DF0 and these thunks; this discovery does not duplicate that work. |
| B49950/B49940 -> B49500 | Both complete 12-byte unwind return thunks bind the matching actual vertex/index pool. Generic raw B49500 return provider already exists in `native_physical_buffer_owner.cpp`. Pool return is not allocation or object destruction. |
| B238D0 | **Incomplete raw default-surface capture**, but its bounded source proposal below has concrete completed providers. Existing `d3d9_states.cpp` fragment checks HRESULTs and uses stable semantic bindings. |
| Current renderer +F0 -> B21960 | **Incomplete gamma dependency.** Pinned table cell and entry prefix bind the target. Prior bounded `GAMMA_RAMP.md` evidence describes 474 bytes, optional guard, current gamma enable/cache flags, device +54 and x87 pow BFEB10. No complete raw source established here; no gamma/display operation was performed. |
| B2ABD0 | **Incomplete 589-byte reset parent.** Its direct graph includes completed B1FD90/B20C50/state/guard providers and separately unresolved B237D0/B23B10/B262C0/B29670 resource/device work. This packet does not trace their transitive closure. |
| BF79F0 / OS imports | Original memset needs an explicit concrete CRT/full-helper contract in a future parent. Actual GetCurrentThreadId and timeBeginPeriod imports are pinned. No replacement callbacks or success policy are proposed. |

B2AEB0's FH3 record DF5C8C has two unwind states, map DF5C7C. State 0 action CBD420 reloads `[EBP-144h]` and returns the raw slot through B49950; state 1 action CBD42B reloads the same current spill and returns through B49940. Both predecessors are -1. These states cover their respective conditional constructors only and are disarmed before device Create calls. They do not own a COM output or imply rollback of already published renderer fields. Raw final-free behavior remains that of the completed pool-return provider.

## Proposed next packet: B238D0

Proposed ownership is only B238D0 [562] plus four new files: `include/bsp/native_renderer_default_surfaces.hpp`, `src/native_renderer_default_surfaces.cpp`, `docs/NATIVE_RENDERER_DEFAULT_SURFACES.md`, and `reports/native_renderer_default_surfaces_audit.json`. No implementation is included or authorized by this discovery.

Proposed API:

```cpp
struct NativeRendererDefaultSurfacesContext {
    NativeSurfaceOwnerContext& surface_owner;
    NativeRendererSynchronizationGlobals& synchronization;
    const std::uint32_t (&original_surface_profile_00d619a0)[2];
};
void __fastcall capture_native_renderer_default_surfaces_00b238d0(
    void* actual_renderer, const NativeRendererDefaultSurfacesContext* context);
```

This is a new ECX/EDX interface (12-byte borrowed context on Win32), not the original ECX-only ABI or a native SEH replacement. The actual canonical surface pool must already be explicitly bound via `bind_static_d3d9_surface_pool_0108db00` and must be the **same** owner as `surface_owner.actual_surface_pool_0108db00`. The context supplies no arbitrary allocation, release, or device callbacks and creates no fallback pool. The existing owner context supplies actual renderer F8D394, canonical pool 108DB00, string pool 419CC0, lifetime 1090AA0, support 108FEDC, tracking 108DAFC and constant D7A24C.

Admit the concrete D619A0 profile identity only where a surface final-zero terminal is reached. Read the current profile/slot 0 at the native call position; require BD30E0. That complete 14-byte adapter tests ECX then reads the **fresh current** profile/slot 4 and invokes B3F5B0 with flags=1. Resolve only these established targets to full `delete_native_surface_00b3f5b0`, including actual owner destruction and canonical pool return. A fresh second profile read must not be silently cached. Other native surface subclasses are outside this proposed source domain, not impossible in the original program. No profile is required on paths that never dispatch a terminal. The original profile prefix is D619A0 = `{BD30E0, B3F5B0}`.

Completed providers are canonical allocate B3F2A0 (full underlying B3ED40), construct B3F630, delete B3F5B0/full B3F4E0 and B3D860, unwind return B3DCC0, and depth bind B21690. They reside in `d3d9_surface_pool.cpp`, `native_surface_owner.cpp`, and `native_renderer_surface_bindings.cpp`. The object is actual 34h `NativeSurfaceOwnerStorage` within a 38h canonical slot; the pool's trailing DWORD must survive construction. B3F630 writes literal D619A0 and initial intrusive count 1. Both capture constructors pass flags=0 and kind=0, **including depth**. Actual COM methods and InterlockedIncrement/Decrement are the remaining external interfaces.

The full current/captured order to preserve is:

1. Capture +1A10; if nonnull AddRef, then reread that captured device's vtable for Release. Reload current renderer +1A10 for GetRenderTarget(0, &actual_color_output), initialized to null. Ignore HRESULT.
2. Capture returned color pointer; if nonnull AddRef then captured-pointer/current-vtable Release, then reload the actual output cell. Unconditionally call current output GetDesc into unused scratch. A failed/null result retains the native fault path.
3. Allocate and spill raw slot, arm constructor state 0, conditionally construct `(current_color_output, 0, 0)`. Capture old +197C after construction and disarm. If old != new, publish new field **before** incrementing new, then decrement old; on final zero dispatch the old current profile. Always decrement the captured new temporary afterwards and dispatch its current profile at zero. A null allocation is not converted into a success/early-return policy; the unconditional temporary decrement addresses +4.
4. Reload current color output; if nonnull do captured AddRef/Release and reload output, then unconditionally Release current output. This is not a single balanced pair that may be erased across real callbacks.
5. Capture current renderer device again for its conditional AddRef/captured Release pair, then reload device for GetDepthStencilSurface into a distinct initialized output slot. Allocate, spill, arm state 1, and construct with `(current_depth_output, 0, 0)`. Capture old +198C, disarm, and perform the same publish/increment/old decrement/new-temporary decrement ordering.
6. Unconditionally Release current depth output **first**, then reload its cell and conditionally perform captured AddRef/Release. This intentionally differs from color cleanup. Finally reload current renderer +198C after all callbacks and invoke the full depth binder B21690, which owns its own optional synchronization scope and device/counter behavior.

B238D0 has no outer synchronization guard and no HRESULT exit. Its FH3 handler CBCF00 points to DF5508, with map DF54F8: state 0 -> CBCEF0 and state 1 -> CBCEF8, both predecessor -1. Each eight-byte funclet reloads the **current raw allocation spill** `[EBP-30h]` and jumps to canonical B3DCC0. The slot is the same physical stack cell for both constructions despite the extra saved EDI changing ESP-relative offsets. Only constructor failure returns that raw allocation; the constructor handles its own internal cleanup. After state -1, old-owner deletion, COM output calls, and the final binder have no outer COM temporary cleanup or publication rollback. A source composition must preserve these exception boundaries rather than placing the entire routine under one RAII cleanup.

A future focused proof should compose the actual full library providers and canonical pool/registry/support/lifetime with real COM surfaces. The concrete risks are callback changes to current renderer device/output/wrapper fields, a nonempty old owner reaching final zero, the asymmetric color/depth COM order, and constructor-only unwind. Any ABI/provider bridges must be declared; byte proof alone is not original-runtime or game validation. This proposal makes no claim about the unresolved logical-stream retained +4Ch type/terminal; no inference from an embedded lock or physical pooled wrapper closes that separate boundary.
