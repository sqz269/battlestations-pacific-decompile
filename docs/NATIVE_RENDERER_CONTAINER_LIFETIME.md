# Native renderer record-container reserves

Addresses: `00B228B0`, `00B22940`, `00B229D0`, `00B22A70`.

These are four complete reserve bodies used by renderer array cleanup. They are
not destructors. The source borrows each actual twelve-byte native header and
uses the existing `singleton_lifetime_allocate` / `singleton_lifetime_free`
allocation domain. Equal native and host byte counts preserve raw storage.

| Routine | Complete inclusive range | Bytes | Stride | Per-record x87 DWORD offsets |
| --- | --- | ---: | ---: | --- |
| `reserve_native_renderer_records16_00b228b0` | B228B0..B22933 | 132 | 10h | 00, 04, 08 |
| `reserve_native_renderer_records20_00b22940` | B22940..B229C9 | 138 | 14h | 00, 04, 08, 0C |
| `reserve_native_renderer_records24_00b229d0` | B229D0..B22A65 | 150 | 18h | 00, 04, 08, 0C |
| `reserve_native_renderer_records40_00b22a70` | B22A70..B22B21 | 178 | 28h | 04, 08, 0C, 10, 18, 1C, 20, 24 |

Names are descriptive hypotheses. Every unlisted DWORD is copied with an
integer load/store, in increasing offset order. Field meanings are not inferred.
The original ABI is ECX = actual header, one signed stack DWORD, `RET 4`; there
is no semantic return value. The explicit unused EDX parameter in the source
keeps the request on the stack. These are new source interfaces, with no claim
of original private-frame aliases, complete register/SEH ABI, or game validation.

## Storage and failure order

The header holds current data at +0, signed count at +4, and signed capacity at
+8. The request is clamped to one, then compared with current capacity using a
signed comparison. A sufficient capacity returns before allocation or stores.
Growth allocates `uint32(request) * stride`, including native 32-bit wrapping.
BF55BE is the allocation thunk; BF6989 is the release thunk. The existing shared
source service uses the source CRT malloc/new-handler/free boundary.

After allocation returns, the body tests current count. Each entered iteration
computes its destination with wrapping address arithmetic and skips the row if
that address is null. It captures current source data once for the row, copies
the row in ascending DWORD order, then reloads current count for continuation.
Float slots retain `FLD m32` / `FSTP m32`, including signaling-NaN quieting and
the current x87 environment. A `memcpy` would not provide this behavior.

After all copies, the body reloads current old data and frees it. Only after
free returns does it publish fresh data and requested capacity, in that order.
There is no direct count assignment and no initialization of unused fresh
records. No exception cleanup or stronger rollback has been added. Allocation
failure propagates with any effects of the allocation handler retained; on a
handler that does not mutate storage, the old header and rows remain unchanged.
Copy faults do not gain a fresh-allocation cleanup path. All raw accesses require
valid extents and ownership; no malformed-count, overflow, or alias repair occurs.

## Renderer cleanup consumers and analysis gap

Read-only disk inspection of the continuation of B32920 establishes these
actual renderer header offsets:

| Header | Reserve call site | Target | Current-data free site |
| --- | --- | --- | --- |
| +1D18 | B32D53 | B22A70 | B32D6E |
| +1D0C | B32D89 | B229D0 | B32DA1 |
| +1D00 | B32DBC | B22940 | B32DD4 |
| +1CF4 | B32DEF | B228B0 | B32E0E |

Each block requests zero only if signed capacity is negative, decrements current
positive count to zero, explicitly stores count zero, then frees current data.
It does not clear the stale header pointer or capacity. The containing renderer
body and its EH actions are owned and repaired by the integrator; this packet
does not reconstruct or annotate them. The first block is in the saved B32920
export; subsequent blocks were read from its omitted disk continuation.

The four saved decompilations incorrectly terminate at BF6989. The necessary
returning call-site overrides are B22921, B229B7, B22A53, and B22B0F. Their real
return instructions are B22931, B229C7, B22A63, and B22B1F, all `RET 4`.
Live function body ranges already encompass those return bytes, while pseudocode
omits the publication tails. No Ghidra mutation was performed by this worker.
The integrator owns correction, preserved-old-value comments, naming, save, and
refreshed exports after the address lease is released.

## Verification

`reports/native_renderer_container_lifetime.json` records fresh project/program
verification, complete live/installed-PE byte equality for all 598 bytes, exact
call sites, build results, and a hashed artifact manifest. The build runs the
eight verified native math seeds and both existing CTests. The focused probe
compares the four relocated complete original bodies with source symbols from
the built `bsp_core.lib`, routing only their two external calls to the same real
source allocation/free service. It checks raw storage, header guards, x87 byte
and exception-status behavior, clamped no-growth, negative-capacity reserve-zero,
and allocation-failure propagation. This is fixture evidence, not game evidence.
