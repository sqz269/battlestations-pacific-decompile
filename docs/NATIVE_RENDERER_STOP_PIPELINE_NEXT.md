# Native renderer stop and pipeline dependency closure

This read-only packet starts from `76040d02e30767ed336d7fb2295756ef7e095e27`.
It investigates `00B28A90`, `00B26920`, `00B241C0` and `00B33BF0`, the renderer
stop branch required when actual queue construction observes control `+20h == 2`.
No C++, shared build configuration, ledger or Ghidra annotations changed.
No new test, native execution, game launch or runtime wait was performed.

All code and data spans in the accompanying report were freshly compared
between saved `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, and the
installed PE. Current prototypes/comments and exact source blobs at the base
commit are pinned. Descriptive names and field roles remain hypotheses unless
the actual access or interface establishes them.

## Stop is an idle acknowledgment followed by reset

`00B28A90` is 44 bytes, ECX renderer, RET, with no FS exception registration.
The concrete renderer table `00D5F0A8` contains this address at `+11Ch`.
It captures the renderer, loads optional worker at `+1970h`, calls `00B33BF0`
only when nonnull, calls `00B33AA0` with a zero stack byte, calls `00B26920`
on the captured renderer, then invokes `Sleep(100)`. It neither clears the
worker pointer nor joins a thread. The direct field view needs through renderer
`+1973h`; the pipeline it invokes touches later fields.

`00B33BF0` is 14 bytes, ECX actual worker, RET through a tail jump. It writes
worker byte `+04 = 0`, then loads the current acknowledgment object at `+10`,
reads that object's current table and tailcalls slot `+08`. Its minimum worker
extent is `14h`. The actual event profile `00D6821C` resolves slot `+08` to
`00BD17C0`, which loads HANDLE `+04` and calls `WaitForSingleObject(INFINITE)`.
There is no null check, finite timeout, wake signal or wait-result branch.

The complete constructor/start/thread/destructor spans confirm the distinction:

- `00B33DA0` creates separate initially unsignaled auto-reset events at `+0C`
  and `+10`, creates thread `00B33C20` suspended, stores its handle at `+08`
  and ID at `+14`, sets priority `-2`, then resumes it. It does not add recovery
  when event or thread API calls fail. Bytes `+04/+05` initially hold zero.
- `00B33BC0` writes callback `+1C`, unsigned rate `+18`, context `+20`, then
  run byte `+04 = 1`, and calls current wake-event slot `+04`. It does not reset
  the acknowledgment event. Full start/worker fields require `24h` storage.
- `00B33C20` initially waits on wake. After run becomes zero it signals the
  acknowledgment and waits for wake again; shutdown is checked after wake.
  A never-started worker need not have produced an acknowledgment for stop.
- `00B33B50` separately sets shutdown `+05`, signals wake, joins the actual
  thread with INFINITE, closes its handle, nulls `+08`, and scalar-deletes the
  two current event owners. It does not itself clear run.

The worker thread remains a separate execution boundary. It reads timer
singleton `01090AB0` virtual `+20`, performs x87 timing with binary32 stores,
calls the supplied callback with **ECX=context**, observes renderer lifecycle
lock `+199C/+18`, and calls the current published renderer's `+0C/+14` slots.
The renderer global is reloaded around those operations. Timer/output contract,
callback execution, full begin/end renderer implementations and thread lifetime
must remain named dependencies. Existing `Win32Event` is a semantic HANDLE
wrapper, not the native eight-byte polymorphic object.

## Actual pipeline sequence and native binding boundaries

`00B26920` is 225 bytes, ECX renderer, RET. Its virtual-table lookups occur
inside each iteration, rather than once before each loop:

| Operation | Native target/fields | Required behavior |
| --- | --- | --- |
| 20 null texture bindings | current renderer `+130h` -> `00B24710` | indices 0 through 19; logical null |
| Four null vertex streams | current renderer `+134h` -> `00B24840` | indices 0 through 3; logical null |
| Null index stream | current renderer `+138h` -> `00B24B00` | null logical owner and base zero |
| Clear actual cache | `00B241C0(renderer+34h)` | intrusive references and sparse validity/storage writes |
| Default color binding | `00B23D80` | reload wrapper `renderer+197Ch`, pass slot zero |
| Clear device depth surface | current device `renderer+1A10h`, COM `+9Ch` | call `SetDepthStencilSurface(nullptr)` under native optional guard |

The table words at `00D5F1D8` freshly resolve the three logical binding targets.
The direct pipeline view requires `1A14h` renderer storage. Including those
concrete binding methods' counters requires through `+1BC7h`. These are touched
extents, not complete renderer object sizes.

The four binding bodies have established instruction sequences but their
current implementations in `d3d9_states.cpp` use shared or borrowed semantic
objects. They do not close actual intrusive storage merely because their
Ghidra names and semantic ledger rows exist:

- `00B24710`: current slot is `renderer+504h + index*ACh`. Enter guard before
  identity comparison. Publish new pointer before retaining actual `+04`, then
  release the captured old owner. For nonnull new owner call current virtual
  `+1Ch`; null skips that getter. Sampler indices at least 16 add `F1h`. Call
  current device `+104h`, then increment `+1BC4h`, ignoring HRESULT.
- `00B24840`: stream record `renderer+1774h + index*10h`. Same logical pointer
  returns after guard handling. With two nonnull logical owners, read old/new
  virtual `+2Ch`, captured cached stride, new virtual `+24h` and its returned
  object's `+CCh`, cached offset, and new virtual `+28h`. Equivalent physical
  values use `00B23710` to change logical ownership without a device call.
  Otherwise update ownership, gather current new-owner values, store offset
  before stride, reload device, call COM `+190h`, increment `+1BB4h`.
- `00B24B00`: writes base `+17BCh` even on identical logical owner. Changed
  owner at `+17B8h` follows publish/retain/release; nonnull new owner supplies
  COM through current `+28h`. Device `+1A0h` and counter `+1BB8h` follow.
- `00B23D80`: no logical ownership change or identity cache. Nonnull actual
  surface wrapper supplies COM `+2Ch`, and counter `+1BA0h` increments after
  the device call even on HRESULT failure. Null slot zero reloads actual
  default wrapper `+197Ch` and dereferences its `+2Ch` without null fallback;
  other null slots pass null. Device slot is `+94h`.

In the stop-specific null bindings, new texture/stream/index getters are
skipped, but releases of old logical owners remain required. A specialized
null-unbind slice does not reconstruct the complete three general bind bodies.
`NativeSurfaceOwnerStorage` already has real surface field `+2C`, but its
existence alone supplies neither this pipeline nor every cached owner profile.

## Exact cache reset, including fields deliberately preserved

`00B241C0` is 664 bytes, ECX **cache subobject**, RET, no FS exception frame.
Its minimum touched extent is `1938h`; gamma at `+1938h` is outside that
extent. Offsets below are relative to the cache, not the renderer.

| Sequence | Actual field operation |
| --- | --- |
| 1 | Release then clear reference slots `+00`, `+04`, `+08` |
| 2 | `memset(+0C, 0, D2h)`; clear DWORDs `+1738`, `+173C` |
| 3 | Release then clear `+1780`, `+1784`; store `FFFFFFFF` at `+1788` |
| 4 | Four records at `+1740`, stride `10h`: release first DWORD, then clear next three DWORDs |
| 5 | Sixteen records at `+428`, stride `ACh`: clear eight DWORDs and byte `+20`, then release owner DWORD `+A8` |
| 6 | Twenty banks at `+1198`, stride `48h`: clear three DWORDs and WORD `+0C` |
| 7 | Release then clear `+18D4` |
| 8 | Write positive zero DWORDs at `+18D8`, `+18E8`, `+18DC`, `+18E0`, `+18E4`, then `+18EC` through `+1934` |

There are **26 reference slots**. For each, capture the pointer; null does not
write that slot. For nonnull, perform `InterlockedDecrement(captured+04)`; zero
loads the captured object's current table and calls current virtual `+00` with
ECX=captured, with no scalar-delete flags. Only after that call returns is the
cache slot cleared. No use of the captured owner follows terminal return.
A throwing terminal leaves that slot uncleared and aborts later reset work;
the native function has no cleanup rollback or no-throw guarantee.

The sixteen texture-reference clears are distinct from the **twenty** texture
bindings made by the pipeline and the **twenty** sampler-valid banks. The four
extra texture slots at cache `+F90`, `+103C`, `+10E8`, `+1194` are not released
by this routine. It also preserves render-valid bytes outside its D2h memset,
record gaps and cached value arrays, the plane set at `+178C..+18CF`, DWORD
`+18D0`, and gamma `+1938`. Whole-cache memset, clearing twenty texture owners,
or `D3D9StateCache::invalidate()` would change this contract.

Five direct owners share this exact reset. Fresh full spans confirm calls from
pipeline `00B26920` at `00B2698A`, reset release `00B262C0` at `00B26460`,
cache construction `00B29430` at `00B295AC`, end/present `00B2D8E0` at
`00B2DACB`, and renderer destruction-shaped `00B32920` at `00B329CA`.
The four renderer callers form ECX=renderer+34h; cache construction passes its
already captured cache identity. The full reset/end/destruction owners retain
their other named-but-incomplete resource, device and execution dependencies.

`00B23710` is a separate 59-byte actual intrusive-slot assignment helper:
ECX destination pointer cell, EDX source pointer cell, RET/EAX destination.
Read source first, then old destination; equal returns. Otherwise publish new,
retain new nonnull `+04`, release captured old `+04`, and call current terminal
virtual0 if zero. It does not reread the source after publication or clear the
destination after terminal return. No exception frame or extra ownership exists.

## Synchronization and EH are part of the dependency

`00B33AA0` writes the supplied raw low byte, in order, to actual globals
`0108D6DC` and `0108D6DD`, preserving the gap and nesting DWORD at `0108D6E0`.
It is a 17-byte stack-argument/RET4 leaf; the renderer ECX is unused. A bool
parameter would discard the raw byte contract.

`00B33AD0` increments the current non-atomic nesting DWORD before testing mode.
If enabled and actual renderer `+04` lock is nonnull, it calls
`EnterCriticalSection`, then increments the captured lock's signed depth `+18`.
Only AL is defined as 0 or 1. `00B33B00` captures current mode in DL before
decrementing nesting; when enabled it reloads renderer `+04`, decrements that
lock's depth before `LeaveCriticalSection`, then reloads mode after that call.
At nesting zero it updates the observed byte if different. RET4 consumes but
does not inspect the saved-enter argument. `TrackedCriticalSection` already
establishes actual `1Ch` storage, depth at `18h`; `RendererSynchronization`
remains a semantic bool-based state rather than these actual globals.

The five EH registrations for pipeline, texture, vertex, index and color bind
each contain one unwind state with an eight-byte guard record (saved AL at +0,
captured renderer at +4). Fresh handler/FuncInfo/unwind bytes all resolve to
common destructor `00B21110`: test **current** global mode, then load saved byte
and renderer and call `00B33B00`. There is no catch or rollback of bindings.
Pipeline state zero is armed only for its final device-depth call, after all
25 renderer virtual bindings, cache clear, color bind and optional guard entry.
The nested binding methods provide their own guard unwind state.

When entry mode was zero, native saved guard fields are uninitialized; changing
mode to nonzero before exit/unwind can expose them. Do not silently repair that
with initialized locals or a different guard policy. Existing stable-mode
restrictions remain necessary for a bounded C++ interface; arbitrary concurrent
or reentrant mode changes have not been made safe by this investigation.

## Prioritized implementation packets

1. **Actual synchronization leaves and guard cleanup:** `00B33AA0`, `00B33AD0`,
   `00B33B00`, `00B21110`; optionally the independent 17-byte lifecycle observer
   `00B20220`. Ready using actual byte/DWORD global references, actual renderer
   `+04` and existing exact tracked-lock storage. Preserve raw bytes, wrapping
   non-atomic nesting, lock capture/reloads, AL-only results and current-mode
   cleanup. No worker, queue, allocator or fabricated lock provider is needed.
2. **Actual eight-byte event owner:** `00BD1970`, `00BD19B0`, `00BD1910`,
   `00BD17C0`, `00BD1960`. Ready with raw table DWORD `+00`, actual HANDLE `+04`,
   the existing operator-new/free services and real Win32 APIs. Preserve
   CreateEventA's raw zero-extended manual-reset byte, false initial state,
   nullable returned HANDLE, unconditional CloseHandle, concrete-to-base table
   transition and delete flag bit zero. Native allocation may throw. Signal,
   wait and reset return their actual API EAX. Subsequent borrowed worker
   start/stop composition must dispatch the current concrete event profile;
   do not introduce a semantic fake event provider or start the worker thread.
3. **Cache construction and bank initialization:** `00B29430`, `00B27E80`.
   `00B27E80` is a complete 145-byte twenty-record initializer, stride ACh,
   touched extent D70h. It writes each owner slot zero **before** its later
   conditional release read. `00B29430` requires actual cache extent `193Ch`,
   calls that initializer at `+428`, uses the existing exact
   `construct_camera_plane_set_00b659d0` at `+178C`, initializes gamma separately
   and initializes all 26 reset-owned pointers before calling `00B241C0`.
   Its construction-only call therefore has a proved all-null reference
   preimage in the ordinary unpublished-storage domain. A constructor packet
   can implement that exact reset write path without pretending a missing
   terminal owner exists; record it as the null-reference slice of `00B241C0`,
   not a complete general cache release. The plane-set source and same live
   `00D7A24C` constant are existing concrete dependencies, not new substitutes.

The complete general `00B241C0` and `00B23710` mechanics are now sufficiently
specified, but a production packet must first choose a **real** terminal
dispatch contract. Existing `NativeRenderActualOwners` decrements actual `+04`
and resolves only on zero, which supplies the correct identity mechanism for
already registered profiles. Its `RenderCommandReference` terminal is
nonthrowing; it does not establish all cached state/texture/stream/target
profiles or a general potentially throwing native terminal edge. Do not fill
that gap with callback stubs, invented owner layouts, or no-op deletion.

After actual guard and terminal-owner support, the three complete logical
bindings plus `00B23D80` can form a separate current-table/actual-device packet.
Only then can `00B26920` and `00B28A90` be integrated as full stop/reset bodies
with a real renderer/event boundary. Queue construction, singleton publication,
worker thread execution and the separately investigated queue execution roots
remain outside this packet. Source/build/native-fixture/game validation must
continue to be recorded separately.

The machine-readable evidence and readiness graph are in
`reports/native_renderer_stop_pipeline_next.json`.
