# Native name-based VFS unmount (BM)

`unmount_native_vfs_name_00be0750` reconstructs the complete ordinary
`BE0750..BE0915` body (454 bytes) over actual Win32 manager, string, provider
and tree storage. The original ABI is ECX manager, stacked `(systemName,
mountPrefix)`, AL boolean, RET8. The source API adds explicit services and a
published `NativeVfsUnmountNameAcquired` frame; it is not an ABI replacement.
Names are descriptive hypotheses.

## Recovered order

1. Canonicalize the second header with `BEE390`, then trim its right side
   with `584110` and actual `CE7898` data (`"/"`). Keep the canonical header at
   one stable address. Borrow the original first header without normalizing it.
2. Start the actual manager+3Ch tree's two-DWORD `{owner,node}` iterator at
   `head.left`. Retain the owner/node captured in EBX/ESI across returning
   `BF6713` validation handlers. Capture the current head before the loop's
   checked-owner comparison; compare the captured node against that captured
   head afterward. Later node/end validations reread captured-owner+4.
3. Match provider name at provider+8/+C and mount key at node+10/+14.
   Both comparisons require equal lengths, accept two zero lengths without
   reading their data pointers, and otherwise call the current CRT `_stricmp`.
   The existing `equal_native_string_headers_00435c40` has the same field-read
   order as these two inlined comparisons. Existing tree order determines the
   first match; equal-priority or duplicate mounts are not collapsed.
4. On a mismatch, call `BD97E0` with the stable iterator and reload node then
   owner from it. On a match, repeat the current end check and capture the
   current node+18 provider. Interlocked-decrement its actual refcount+4.
5. Only if the result is zero, capture that provider's **current** table and
   virtual0 and call the existing `NativeAdoptedSubstreamDispatch` method
   `source_zero_reference(entry,provider,table)`. No null/table/profile preflight
   is inserted, and no provider reference is acquired.
6. After a successful release, call existing `BE0080` with the original tree,
   the **same output iterator**, and the captured owner/node. The erase helper
   owns key/node storage and does not release the provider. Return true after
   erasing one node; reaching the end returns false.
7. On either normal exit, capture canonical data and wrapping length+1, then
   call the canonicalizer services' current `419CC0` getter and `BD1510` return
   with unused argument1. The header remains untouched. A null data pointer
   skips the length read and both calls.

`NativeVfsRuntimeBindings::source_zero_reference` already implements the
provider virtual0 target `BD30E0`. It reloads the provider's current virtual4
and supplies flags1. The checked current profile words are:

| Table | Virtual0 | Virtual4 |
|---|---|---|
| D689E8 FileStore | BD30E0 | BE8090 |
| D69168 physical | BD30E0 | BF4DD0 |
| D64390 MPKG | BD30E0 | BB9EE0 |
| D641F8 MPAK | BD30E0 | BB7B80 |

The unmount source reuses that dispatcher. It adds no substitute dispatcher,
shadow collection, manager publication, provider cache, or ownership domain.
The runtime must supply the existing contexts required by a reached provider
deletion. The fixture exercises concrete FileStore deletion only.

## Failure boundary and EH evidence

Live project `bsp`, `/battlestationspacific.exe`, x86 image base400000, was
verified by the repository Ghidra client before every live read. Installed PE
bytes match the live unmount body, EH spans, trim literal and four profiles.
The JSON report pins their hashes and exact direct/tail call rows.

`CC6728..CC6731` decodes to `mov eax,E00D7C; jmp BF6B43`; it still has no
Ghidra function. FuncInfo `E00D7C` has magic19930522, maxState1, unwind map
`E00D74`, zero try blocks and EHFlags1. Its sole map entry is
`{toState=-1,action=CC6720}`. Existing `CC6720..CC6727` is
`lea ecx,[ebp-1Ch]; jmp 41DD20`, destroying the canonical local. State0 is
armed after canonicalization and disarmed before either normal pool getter.
There is no compensating provider retain, tree erase retry, or VFS exit.

Source exceptions from an unresolved nested provider, CRT handler, allocator
or cleanup may leave borrowed storage in flight. The acquired frame preserves
its normalized header, iterator, register captures, final-release arguments,
and failing call site. Callers must publish and retain it before invocation,
and retain its context/manager/input storage after failure. Replaying the
frame throws `logic_error`; destroying an active or failed frame terminates.
No guessed native unwind or recovery operation is exposed. Consequently the
source failure policy differs from original FH3; ordinary source completion
and known native-EH completion must not be conflated.

The normal outer cleanup uses the potentially throwing pool service interface.
Existing trim/erase dependencies still use `NativeStringStorage::release`
with its inherited `noexcept` boundary. Native throwing getters within those
dependencies, FH3/SEH identity, simultaneous exceptions, arbitrary aliases of
the original stack/register spill locations, concurrent mutation and original
gameplay remain unproved. Raw pointers held after deletion are diagnostics,
not additional ownership or permission to dereference freed storage.

## Validation

The new TU compiles with MSVC Win32 C++17 `/W4 /WX /O2 /fp:strict /EHsc /MD`.
The ignored fixture uses 39 frozen source/header inputs, the newly compiled
object, and separately frozen libraries from the assigned integrator checkout
`battlestations-pacific-decompile-orch4-20260910`. The core library hash is
`134578367593f74415bb6fd12b51d103dfc6575dcb94d2f692640dde69c78251`;
pre-copy, post-copy and frozen-copy hashes agree. An earlier copied library
from the unrelated main checkout is retained with its receipt but is not
linked into the passing fixture.

The fixture executable has an embedded manifest and runs only reconstructed
source. `GameNativeReadOnlyData` maps verified original read-only literals and
tables, with no executable original pages or resolved native imports.

- Actual pool storage and allocated priority-tree nodes pass absent-name,
  empty-tree, canonicalized/case-insensitive matching, and first duplicate
  removal checks. Caller input headers remain unchanged.
- A provider with two references and an invalid table pointer reaches one
  remaining reference without touching its table. Restoring actual D689E8
  before the second removal reaches real runtime `BD30E0 -> BE8090` deletion.
  Remaining nodes/providers and the actual string pool clean up normally.
- An isolated failure run supplies an unknown current virtual0. Dispatch
  rejects it after refcount reaches zero; no erase occurs, `failure_site` is
  BE088A, and the stable canonical/iterator storage remains intact. This run
  deliberately retains the unresolved invocation until process termination.

No permanent tests were added. No Ghidra writes, metadata edits, CMake edits or
full integration build were performed in this worker packet; primary owns
those actions. This is reconstructed, strict-TU-compiled and source-fixture-
tested evidence, not a complete native ABI, original-executable or game result.
