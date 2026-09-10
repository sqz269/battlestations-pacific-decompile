# Native render-resource record assignment (`00B30510`)

Primary integration rechecked all five native spans and 29 evidence artifacts,
retained the worker's source unchanged, and reran the 116-word original caller
fixture against the integrated library. The strict Win32 build and both existing
CTests passed. The assignment annotation is saved with its previous values,
and its export is refreshed; see this packet's audit `primary_integration`.

`assign_native_render_resource_record_00b30510` reconstructs the complete
162-byte assignment body on the actual `NativeRenderResourceRecord` storage.
It returns the destination reference and accepts the existing string pool and
invalid-parameter callbacks. The name uses the actual eight-byte prefix,
aliases use the actual list at record `+8`, and the last six DWORDs are copied
individually after those operations finish.

The implementation composes `resize_native_string_header_0041dd40` through
`PooledStringStorage`, `clear_native_render_resource_aliases_004d05e0`, and
`insert_native_render_alias_range_004d26a0`. It does not construct a temporary
string owner, overlay `NativeString` on a live record, or replace helper
behavior with a success-only stub. The existing record clear and destruction
bodies are unchanged.

## Native evidence and ABI

The full span `00B30510..00B305B2` (exclusive end) was read from the saved
`bsp` project and compared with the installed executable; all 162 bytes
match. The live reads used `bsp.py ghidra`, which verifies project `bsp`,
program `/battlestationspacific.exe`, the x86 language and image base
`00400000` before querying. No Ghidra state was modified by this worker.

The original ABI is `__thiscall`: ECX is the destination, the stack holds a
source-record pointer, EAX returns the destination, and the epilogue is
`RET 4`. EDI retains the destination and EBP the source. There is no local
exception handler. The recorded Ghidra name was `FUN_00b30510`, the stored
prototype was `undefined FUN_00b30510(void)`, and no existing function comment
was returned. Those values are preserved in the audit; the parameter and ABI
conclusions come from the assembly. A descriptive assignment name remains a
reconstruction hypothesis, not a recovered original symbol.

| Instructions | Reconstructed behavior |
| --- | --- |
| `00B3051D..00B30527` | Skip name work on exact record identity. Otherwise read source length and resize the actual destination prefix with preserve enabled. |
| `00B3052C..00B30542` | Reread source length after resize. If nonzero, read current destination length, current source data and current destination data, in that order, then copy the destination length in bytes. |
| `00B30545..00B3054D` | Form the actual source and destination list-owner addresses at record `+8`; skip list assignment when those addresses are equal. |
| `00B3054F..00B3055E` | Capture source sentinel and its first node before clearing the actual destination list. |
| `00B30563..00B3057D` | Reload the current destination sentinel and its current first node after clear. Insert the captured source range before that destination node. |
| `00B30582..00B305A3` | Perform five sequential current-source DWORD reads/stores at `+14/+18/+1C/+20/+24`, followed by the current pointer at `+28`. |
| `00B305A6..00B305AF` | Return the original destination and pop the source argument. |

The range arguments are actual destination owner/current first, actual source
owner/captured first, and actual source owner/captured sentinel. The native
call also pushes the captured source sentinel as a seventh, unread DWORD.
The full range helper deliberately has no invented semantic parameter for
that unread word.

The `+8` prefix word is not assigned from the source. The alias count is
rebuilt by clear/range operations rather than copied. The source sentinel and
first node stay captured even if cleanup callbacks change the source header;
the destination insertion node is determined after those cleanup callbacks.
This is an operation on the live graphs, without snapshots or disjointness
guards.

Exact self-assignment skips both name and alias work, yet still performs all
six tail reads/stores. The implementation uses volatile field accesses for
those stores and for the capture/reload points; there is no whole-record
early return or copied payload snapshot. The `+28` value is copied as one
pointer. This body performs no resource virtual call, retain, release or
destruction, and does not establish ownership semantics for that pointer.

## Failure behavior and host boundaries

The assignment has no local rollback. A resize exception propagates without
reaching alias work or tail stores. If list insertion fails, the completed
name change and destination clear remain; only the range helper's established
partial rollback runs. Tail fields have not yet been copied. A later helper
failure may supersede an earlier exception according to that helper's own
behavior; assignment adds no cleanup or recovery policy.

The name's post-resize copy uses current destination length, not the captured
or current source length. A callback that changes the source header during
allocation can therefore change which data is copied and whether that copy
is attempted. Equal-size requests, including stale/null header behavior,
remain governed by the full actual-header resize helper.

The existing explicit host boundaries remain in effect:

- `PooledStringStorage` uses the supplied `SizedStoragePool`. Native global
  singleton lookup and first-use side effects are outside this interface.
  The existing host pool's arena bounds checks and exception policy remain.
- Nonzero name copying uses the host `memcpy`. A zero-byte copy is omitted,
  following the existing raw-string helpers, to avoid passing possibly null
  pointers to the standard library. The native can still issue that call.
- Alias nodes use the existing lifetime allocation/free domain. Range checks
  use the supplied invalid-parameter callback, which can return or throw.
  Count growth uses its existing owning host exception transport.
- The helper requires the actual native storage and valid ranges for the
  operations taken. It does not make overlapping or callback-corrupted
  source/destination graphs safe, add an ownership conversion, or reproduce
  native CRT exception-object/SEH ABI.

## Focused verification

The private fixture loads the complete original assignment body from the
installed executable into a sparse Win32 image and verifies the bytes before
executing them. It changes no instruction inside that body. Four separately
verified five-byte hook sites route the native calls to the real reconstructed
resize, clear and range functions and to host copying. These are composition
boundaries: this fixture executes original assignment bytes and the complete
host helper bodies, not the original native bodies of those callees.

The helpers use a real `SizedStoragePool` with a small test configuration and
a recording `SystemAllocator` that performs actual allocations and frees.
The fixture uses allocation/release callbacks to exercise the caller's
specific capture boundaries:

- During name allocation, replace source name data and change source length.
  Assignment still copies the current destination length from the current
  source data.
- During destination alias release, replace the source sentinel, switch the
  destination sentinel, establish its current insertion node/count, and
  change source tail values. The final aliases come from the originally
  captured source range, are inserted at the current destination position,
  and the final payload reflects the later source values.
- In the failure variation, source-header mutation makes range validation
  throw before insertion. The destination keeps its completed name change,
  its old aliases are cleared, and its tail is untouched.
- Exact self-assignment uses deliberately unusable name/list pointers and
  returns the same record without invoking any helper. The assembly and
  volatile implementation separately establish the six identity tail stores.

The original and reconstructed assignment produced the same result:

```text
PASS: full original 00B30510 versus composed assignment: 116 normalized words; callback captures, failure boundary and exact identity.
```

The source and fixture compiled as MSVC Win32 with `/O2 /EHsc /fp:strict
/W4 /WX`. All eight `verify-seeds` spans matched. The final
`scripts/build.ps1` run passed both existing CTest checks. No new tracked
test or shared build-registration file was added.

The branch starts at `3502565`, with only the actual-resize dependency
`6dee4535` and alias-count dependency `4ef92c38` cherry-picked before this
packet. The integrator-owned insertion source/header were copied read-only
under `local/support`, registered only through a private CMake include, and
hashed in the audit. Their bytes matched the integrator's current files after
the successful fixture. The integrator owns registration and final insertion
integration; this worker does not edit those files or shared ledgers.

The report pins native spans, original annotation values, source/dependency
hashes, private fixture artifacts and build outputs. This establishes a full
reconstructed body, compilation, and a focused native-byte caller comparison
through real reconstructed dependencies. It is not a drop-in ABI replacement
or game/runtime validation.
