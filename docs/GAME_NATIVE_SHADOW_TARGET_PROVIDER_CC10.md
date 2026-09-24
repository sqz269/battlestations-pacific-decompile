# Retained application shadow target providers (CC10)

This packet composes the existing actual shadow target context into
`GameNativeRendererApplication`. Borrowing the ready-only accessor constructs
no native owner, allocates no target, and leaves `73D410`/`B107F0` activation
separate. It does not change the render-resource direct-terminal gate.

## Canonical process cells

`GameNativeRendererScalarProcess` now owns one address-stable copy of each cell:

| Original cell | Source accessor | Established preimage/provider |
| --- | --- | --- |
| `00F8BBF0` (4 bytes) | `shadow_target_publication_00f8bbf0()` | PE loader-zero `.data` tail; no per-application reset |
| `00CE2218` (4 bytes) | `enter_iat_00ce2218()` | Actual `KERNEL32.dll!EnterCriticalSection` export |
| `00CE2210` (4 bytes) | `leave_iat_00ce2210()` | Actual `KERNEL32.dll!LeaveCriticalSection` export |

All three live Ghidra byte blocks equal the installed PE image. `F8BBF0` is
offset `183BF0` in `.data`: raw size `10000`, virtual size `297EDC`. It is
loader-zero storage, not a guessed constructor write. The import table names
the exact two functions; source resolves them once through `GetProcAddress`,
copies the callable representation with size checks, and retains mutable
current pointer cells. Context construction borrows those cells. The native
adapters load each current value at its reached enter/leave site.

The existing process object also supplies the unchanged texture/surface
cumulative cells and increment import. No second counter, process object,
pool, registry, reset, or native CRT callback is introduced. Its trivial
destruction performs no owner cleanup. Normal application drain precedes CRT
shutdown; an interrupted native operation requires process retention instead.

## Application lifetime and identities

The retained `ShadowTargetGraph` follows `RenderResourcesGraph` in `Impl` and
therefore is destroyed before its borrowed resource providers. Its context
borrows the same actual renderer publication, original mapped profiles,
runtime-texture creation and texture-surface getter contexts, resource
lifetime, direct-terminal domain and canonical actual-owner registry. The
manager reference is the existing `GameSingletonHost` accessor to the canonical
string-process `01090AA0` cell.

Only after stable graph construction and all existing/new duplicate-binding
checks succeed does `Impl` assign the existing singleton deletion table's
`shadow_depth_target` pointer. That uses the already reconstructed final
`D5B5E8 -> A900C0` route. No singleton is registered by this assignment.
`shadow_depth_target_context()` requires the application's ready phase and
returns the same retained context on every borrow.

The caller still supplies genuine `2Ch` storage and actual dimension cells
when invoking the producer. The supported normal drain domain requires the
completed final `D5B5E8` owner and valid children. Complete calls to individual
base helpers do not make their transient `D5B554`/`D5B558` profiles drainable.
Do not reuse or settle a failed operation; retain all its providers, backing,
publication, acquired children and any captured singleton lock.

Every source invocation already has a context-owned, address-stable record.
`requires_process_retention()` now checks their phases in addition to existing
renderer/shader conditions, including failures reached through the borrowed
context without a renderer phase transition. The existing startup shutdown
entry and destructor inspect this predicate before native drain or CRT cleanup
and use `_Exit(1)` for incomplete work. Post-drain validation requires all
shadow operations complete and the canonical publication null, without
resetting records or manufacturing cleanup.

`GameStartupHost` deletes `GameSingletonHost` before resetting its renderer
application. Shadow graph destruction therefore performs **no host access**:
it only discards completed metadata. The singleton table is consumed while
the graph is alive; its host is later deleted before graph metadata. Existing
renderer/resources lifetimes and this ordering remain unchanged.

## Focused current-application evidence

The ignored `local/shadow_target_provider_probe/` binary rebuilds the current
`game_main` forwarding fixture and links all 70 current production application
objects plus the three current libraries. There is no observer copy or
replacement production object. Compilation uses `/MD /fp:strict /W4 /WX` and
`/link /MANIFEST:EMBED`; the extracted manifest requests `asInvoker`. Runs use
`tools/run_game.ps1` and isolated settings directories.

Normal mode proves stable accessor identity, zero producer effects from
borrowing, canonical publication/import-cell addresses, actual Windows
exports, and identical current renderer/producer/terminal domains. It then
constructs a genuine `2Ch` target with explicit fixture dimensions `32x32` and
`CC` padding preimage, creates two actual textures and their two-credit
surfaces, and leaves the owner to the actual application singleton drain.
The raw nested owners remain unbound; the service direct-terminal gate remains
null. The completed scalar record, cleared publication and balanced lock are
observed after drain. The ready-only accessor rejects the drained phase.
Exit is `0`; final retained COM releases report device/API `0/0`.

Failure mode in the same binary sets the actual process-owned current enter
cell to null and reaches the existing **source binding-validation** check.
The real constructor prefix has stamped `D5B554` and captured the actual
singleton section, with no section entry/increment or target publication.
Its operation is failed and its raw allocation remains retained; diagnostic
stage tag is `00A8A867`. The application retention predicate becomes true.
Calling the real startup drain entry then executes its `_Exit(1)` guard;
captured exit is `1`, with no native drain, graph destruction or CRT-cleanup
claim. The import cell is not restored and the failed frame is not destroyed,
retried or marked settled. This is not original native fault/FH3 transport.

Strict `scripts/build.ps1` Win32 compilation and all three existing CTests
pass. No tracked tests were added. The report SHA-indexes every new probe,
source-adaptation, build, manifest, live-byte and PE-cell evidence artifact.
There are no newly reconstructed native function bodies or call-site claims
in this source-composition packet; the frozen shadow-owner report retains
that separate body/transfer evidence.

No production target invocation, whole application initializer, complete
`B107F0`, shadow rendering, binary ABI, native EH cleanup, cold shader/compiler
loading, bloom extent repair or distortion preimage is established here.
