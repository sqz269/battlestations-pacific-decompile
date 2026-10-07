# Complete borrowed MoveTo construction, cc11

The complete ordinary `009C2AC0` constructor now has a conditional SOURCE
implementation. It borrows actual caller storage, creates the real observer
registration, and exposes the SAME profile/vector/callback aliases to existing
complete `007B65E0` cleanup. It supplies no state arena, callable class table,
approach constructor, observer world, or original ABI adapter.

## Native evidence and storage

`009C2AC0..009C2B64` is 164 bytes and 44 instructions with zero listing gaps.
The original PE, live Ghidra bytes and the fixture's whole-body literal match:
SHA256 `9160d22694c0335e800fa049a58458958603a352a1d2917e90043c038efda669`.
Original ECX is the nonnull MoveTo root; the stack carries owner identity, actual
FIRST endpoint and three binary32 words. EAX returns the root and RET14 consumes
all five arguments. Its only CALL is `009C2B4B -> 00694A60`.

The native composite passes `approach+CC` at `009B2EC9`, then constructs Follow
at `approach+108`; the actual interval is 3Ch. The largest constructor store is
DWORD `+38`, ending at `+3C`. This is backing evidence, not a complete class,
approach or arena lifetime claim. All twelve direct constructor call sites were
inspected; the composite's preceding lower constructor remains unbound.

| Field | Required actual alias |
|---|---|
| `+00/+04/+08` | raw profile / copied owner identity / DWORD zero |
| `+0C/+10/+14` | existing actual vector data/count/capacity |
| `+18..+27` | actual `NativeObserverOwnerStorage` callback prefix |
| `+28` | byte zero |
| `+29..+2B` | untouched padding |
| `+2C` | actual FIRST endpoint identity, guarded by input null test |
| `+30/+34/+38` | copied raw binary32 words |

The mapper checks the backing extent and reference addresses only. It does not
read represented values, translate endpoints, allocate, invoke callbacks or
extend lifetimes. A mismatch is a SOURCE admission error. Direct aggregate
callers must satisfy the same alias contract.

## Exact publication and natural providers

The SOURCE constructor preserves this native order:

1. Copy owner to `+4`; zero vector `+C/+10/+14`, then DWORD `+8`.
2. Stamp callback `CE3CD4`; zero its `+4/+8/+C` and byte state `+28`.
3. Copy words `+30`, then `+34`.
4. Stamp root `D20AEC`, then callback `D20AD4`.
5. Publish null `+2C`, then copy word `+38`.
6. For captured nonnull FIRST only, publish that SAME identity at `+2C`, then
   call complete `register_observer_pair_00694a60` with FIRST and actual root+18.

`00694A60` retains its captured recursive section, separately locked lookup,
real `00694850` edge creation or modulo32 reference increment. Existing current
CRT allocation and canonical CF7E64 edge operations are reused; no replacement
lock/list/callback implementation is introduced. Complete `007B65E0` then
destroys actual callback18 through `00695870`, followed by complete shared
`007B45F0`. Provider-final profiles are retained: callback `CE3CD4`, root
`D056D0`. The retained `+2C` and payload words are not cleared. Freed backing
metadata may dangle and must not be dereferenced or reused after cleanup.

Historical naming evidence called the third argument an **int mode**. A later
annotation calls it a speed range. Both labels remain hypotheses: native
`009C2B27/009C2B41` uses MOVSS to copy its 32 bits unchanged into `+38`. The
SOURCE API transports DWORD representations for all three lanes, with no
floating arithmetic, conversion, semantic mode or native float-argument ABI
claim. The new interface and emitted integer stores are not original encoding.

## Raw profile admission and remaining boundaries

Live/disk 56-byte table evidence has SHA256
`5b3e60123eb65ddf07c292e1d8f91767b167d1df283d3e0a6b0c977b0e0cd478`.
Actual `D20AD4` has six entries: `9C2B70,9BDEB0,42B120,42B130,42B140,6953C0`.
Actual `D20AEC` has eight: `9C3D10,7B3DB0,7B3DC0,9C18C0,7B3DE0,7B3DF0,7B45E0,9C1850`.
These numeric image identities remain **UNCALLABLE** in SOURCE. The undefined
callback scalar `9C2B70`, class scalar `9C3D10`, other callback methods, tick and
type-aware virtual dispatch are not supplied. Ordinary registration and direct
cleanup need no callback-table invocation; canonical edge deletion uses its
already complete genuine provider. Direct empty MoveTo entry is separately
reconstructed; this packet does not construct an executable profile for it.

The admitted domain requires fresh placement into stable live actual >=3Ch
storage, no overwritten live resources, stable copied arguments, live coherent
FIRST endpoint when nonnull and actual published `NativeObserverLifetime`
manager/lock/dispatch/CRT context. Native observer allocation/count/range
requirements must hold and allocations/free must succeed. Owner identity is
only copied here; later consumers require their own owner and lifetime domain.
Structural reentry, concurrent mutation, invalid placement, null allocation,
overflow, constructor failure/rollback, asynchronous faults and private native
EH are excluded. No runtime guard or fallback is added for those paths.

Whole `009B2E50`, lower `009AFE70`, approach/task construction, ownership,
observer manager world and native ABI/gameplay binding remain unbound. Follow
`009C2980` is separately 213 bytes; its actual singleton service and caller x87
initialization still require primary recovery/adoption and are not included.

## Focused verification and reproducibility

The ignored `local/cc11_land_moveto_constructor_probe.cpp` uses actual 3Ch
field-shaped storage, live FIRST prefixes, genuine registration/canonical edge
deletion, complete cleanup and a prepublished actual OS recursive lock. Source
and full-original164B normal bodies exercise nonnull and null target branches.
Only the original registration CALL's rel32 operand is relocated to a complete
SOURCE ABI bridge. The bridge observes published fields before the real call;
it supplies no mutation, behavioral reentry or alternate endpoint/provider.

The fixture preserves `7F800123` sNaN bits, `80000000` negative-zero bits and
`00000001` denormal bits, plus all three padding bytes. It verifies root/endpoint
identity, the real CF7E64 edge, pending-slot suppression and captured recursive
lock restoration through cleanup. The copied owner token is not an approach
constructor/arena claim. Actual manager lookup is unexercised. Original SEH
setup/restore runs only ordinary success; exception handling and historical CRT,
loader/native-game equivalence are not established. No freed backing is read.

Five fresh TUs compiled strictly with MSVC Win32 `/EHsc /std:c++17 /MD /O2 /Gy
/DNDEBUG /W4 /WX`: constructor, observer_edges, observer_lifetime, shared-state
and probe. Link used `/INCREMENTAL:NO /OPT:REF /MANIFEST:EMBED`; the PE has
asInvoker/uiAccess=false. Three current a121efaa0 support libraries were copied
under unique names with source-before/copy/source-after hash equality, then
remained immutable. All 28 source/header/library input hashes match after the
run. The probe passed **57 checks**. COFF retains exact represented store order
and the sole registration call after `+2C/+38` publication.

`reports/landing_moveto_constructor_cc11.json` records native receipts, call
checks, fresh-TU commands, hashes and limits. The ignored manifest is
`local/cc11_land_moveto_constructor_manifest.json`; the manifested executable is
`local/cc11_land_moveto_constructor_probe.exe`. Root owns source registration,
independent probe and full main build. No tracked tests or shared files changed.

## Primary integration

Main `99bef9d8df06491eb890ffa88dda3bb3b627318b` passed the full Win32 build and all three existing CTests. Root independently rebuilt five actual constructor/observer-edges/observer-lifetime/shared-state/probe TUs, pinned 25 current Source/header/fixture inputs (20 compiler includes), three current libraries and the PE before/after, and reproduced 57 assertions. All 164 original bytes match disk/live/fixture literals and both direct rows passed. Fresh COFF has exactly one genuine registration-call relocation and retains the publication order through integer word stores. The original third-word label remains a hypothesis, with qualified MOVSS evidence appended to preserved history; manager/arena/private-EH/ABI/game qualifications remain. The PE32 asInvoker manifest was verified. No tracked tests were added.
