# Native pending entity cancellation

Packet U reconstructs normal-path `00925A00[138]` over borrowed actual entity
lvalues, existing `NativePendingEntityOwners`, and a required actual lock getter.
It adds no unit, list, count, flag owner, runtime binding, or library implementation.
The source name is descriptive, not a recovered symbol.

| Routine | ABI / coverage |
| --- | --- |
| `00925A00` | ECX=entity, bare RET; complete normal path with required providers |
| `00781260` | ECX=list, stack=&entity, RET4; assessed provider contract only, no source reconstruction |

`00925A06` obtains `009248D0`'s raw8 lock owner. Capture `section_04` once;
enter/increment its canonical `TrackedCriticalSection` depth, and leave/decrement
that same section even if the owner changes. Depth adjustment retains DWORD
wrapping arithmetic. A null section skips locking, not cancellation.

Read parent+3Ch after entry. The gate permits a null parent, or parent+5Ch!=0
with +5Dh, +60h, +5Eh all zero, in that order. Rejection changes neither flags
nor lists. Success performs:

1. If entity+5Fh!=0, call `00781260` on actual kill owner F899B4 at `00925A4D`,
   then clear +5Fh.
2. Capture the +60h comparison at `00925A55`, then clear +5Eh at `00925A58`.
3. If captured true, remove from actual destroy owner F899A8 at `00925A6B`,
   then clear +60h.
4. Clear +5Dh. Preserve +5Ch and all other entity bytes.

`NativePendingEntityCancelView` contains borrowed pointer/byte references, not
snapshots. Required providers resolve those references without side effects and
remove every matching payload node from the supplied actual ring. Removal must
retain the sentinel, allocator word, other nodes and payloads; decrement actual
count per freed node; and preserve checked-iterator/returning-validation behavior.
Missing providers throw before lock acquisition. Providers require nonthrowing
normal-path operation; native exception/FH3/SEH and asynchronous faults are unproved.

Reuse assessment: existing raw-list `004C5940` destroys the entire ring and
sentinel; it cannot remove selected payloads. Existing GUI `00A9BD50` source
projection operates on semantic vectors/unique pointers. Neither is a compatible
provider, and no complete byte/relocation equivalence to a canonical source
provider was established. The API therefore requires `00781260` explicitly.
Its third native call, `0078138A`, uses owner F899CC and the same &entity/RET4
contract. Saved Ghidra membership omits `007812D5[7]`; live/disk bytes are
`83c404834508ff` (stack cleanup and count decrement), recorded as
`no_ghidra_function`, not omitted behavior.

Read-only verified Ghidra `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, supplied both complete spans. All 294 bytes matched
installed PE bytes; 13 relocation operands were validated. One local differential
fixture executes original cancellation bytes and original removal bytes against
the new source using that same removal provider. Eight cases cover parent gates,
noncanonical flag values, null/captured section, duplicate removal and provider
order. All matched, including unrelated bytes, surviving ring links/counts and
depth restoration. No runtime/gameplay or native exception parity is claimed.

Exact source, inputs, original bytes, relocations, objects, toolchain/libraries,
embedded asInvoker manifest and output remain in `local/native_pending_entity_cancel_*`
and its SHA-256 manifest. Build/CTest and call-site results are recorded in
`reports/native_pending_entity_cancel.json`.

## Integration correction: provider exceptions

The original worker API marked provider pointers `noexcept` as a fixture-domain
restriction. Native evidence does not require termination: `00925A06` calls the
actual allocating lock getter before `00925A0B` captures its section and
`00925A15` enters it. The integrated API permits an exception from that getter
to propagate. Existing nonthrowing providers remain compatible.

The body has no EH frame and uses explicit section entry/exit. No automatic
section release or state rollback is introduced if a later provider throws.
A focused source-only pre-entry getter failure check is recorded in
`reports/pending_entity_cancel_provider_exception.json`; it is separate from
the worker's eight original-byte normal-path comparisons and does not establish
native exception transport equivalence.
