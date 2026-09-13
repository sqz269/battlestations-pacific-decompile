# Native renderer frame statistics

Addresses: `00B0CC10`, `00B0CCB0`, `00B0CCE0`.

These three complete raw-storage providers clear a 40-DWORD bank, publish it
to its adjacent bank before clearing, and initialize both banks plus 15 trailing
words. The actual receiver is inline renderer storage at `renderer+1B78h`.
The descriptive frame-statistics names are hypotheses; original symbols and
the individual meanings of all 40 bank words and 15 trailing words are unknown.

| Routine | Inclusive native body | Bytes | Coverage | Original ABI |
|---|---|---:|---|---|
| Clear bank | `00B0CC10..00B0CCA1` | 146 | complete | ECX bank; zero stack argument slots; `RET` at `B0CCA1`; EAX incidentally zero |
| Publish and clear | `00B0CCB0..00B0CCCB` | 28 | complete | ECX bank pair; zero stack argument slots; tail `JMP B0CC10` at `B0CCC7`, then its `RET` |
| Construct | `00B0CCE0..00B0CD50` | 113 | complete | ECX storage; zero stack argument slots; EAX original receiver; `RET` at `B0CD50` |

Total: three normal bodies, 287 native bytes. The public entries in
`bsp/native_renderer_frame_statistics.hpp` are new C++ interfaces; they are not
native ECX entry thunks. Ghidra's current no-argument signatures omit the raw
receiver, and a one-argument `__fastcall` decompilation would not establish EDX
as an input. No native stack arguments or callee stack cleanup are present.

## Storage and order

| Domain | Relative bytes | Actual renderer bytes | Behavior |
|---|---|---|---|
| Current bank | `000..09F` | `1B78..1C17` | Copied, then cleared |
| Published bank | `0A0..13F` | `1C18..1CB7` | Receives all 40 current DWORDs |
| Trailing words | `140..17B` | `1CB8..1CF3` | Constructor clears; publish does not touch |

`B0CC10` executes `XOR EAX,EAX`, then exactly these 40 DWORD stores:

```
00 04 08 0C 10 14 18 1C 20 24 28 2C 30 34 38 3C
40 4C 44 48 50 54 58 60 64 68 6C 5C 70 74 78 7C
80 84 88 8C 90 94 98 9C
```

The explicit assembly block preserves `4C` before `44/48`, and `5C` after
`60/64/68/6C`, including DWORD write width. No `memset` replaces the sequence.

`B0CCB0` saves ESI/EDI, keeps the receiver in EAX, forms EDI=`receiver+A0`,
sets ECX=`28h`, and ESI=receiver. `REP MOVSD` at `B0CCC1` copies the full A0h
bytes before the tail jump to the complete clear leaf. The reconstructed copy
uses the same instruction before calling the leaf. Its C++ compiler preserves
ESI/EDI around the new interface. The native body does not execute `CLD`;
direction flag clear is a precondition, as in the ordinary Win32 ABI.

`B0CCE0` calls the same leaf on the current bank and then the adjacent bank.
It next clears exactly these 15 trailing DWORDs, in order:

```
140 144 150 154 158 15C 160 164 168 16C 170 174 178 148 14C
```

It returns the original receiver. The native receiver survives in EDX because
the complete clear leaf never writes EDX. The C++ implementation retains a
local pointer across the two calls instead of assuming a C++ callee preserves
that volatile register.

All reached storage must be valid: writable through `+9F` for clear,
readable/writable through `+13F` for publish, and writable through `+17B` for
construction. The two fixed banks are adjacent and nonoverlapping. No null
acceptance, bounds repair, pointer ownership, semantic renderer object or
counter-field schema is introduced. There is no allocation, native EH frame,
reference counting, destruction, locking, virtual dispatch, global access or
external provider in these three bodies. Raw invalid-memory faults lie outside
this valid-storage contract; ordinary C++ compilation does not establish SEH
equivalence or behavior during concurrent unsynchronized access.

## Call boundaries and producers

Every direct xref and callee of the three bodies was read. The only reached
callee is the complete clear leaf owned by this packet; no host interface is
needed. Live containing-function queries verified the call-site attribution.

| Call site | Containing function | Native target | Contract |
|---|---|---|---|
| `00B0CCC7` | `00B0CCB0` | `00B0CC10` | Tail jump after complete copy; ECX original bank pair |
| `00B0CCE2` | `00B0CCE0` | `00B0CC10` | Clear first bank; ECX receiver |
| `00B0CCED` | `00B0CCE0` | `00B0CC10` | Clear second bank; ECX saved receiver+A0 |
| `00B2DAD6` | `00B2D8E0` | `00B0CCB0` | EndFrame forms ECX=renderer+1B78 at `B2DAD0` |
| `00B32709` | `00B32410` | `00B0CCE0` | Renderer constructor forms ECX=renderer+1B78 at `B326FB` |

`B32701` publishes the enclosing constructor's EH state `13h`; `B3270E`
subsequently clears `renderer+1CF4`, immediately beyond this 17Ch-byte block.
That enclosing state is not an EH frame or cleanup obligation inside these
three providers. Full `B32410` remains incomplete independently.

Current source producers were reconciled before describing this storage:

| Renderer offsets | Existing source | Producer evidence |
|---|---|---|
| `1BA0/1BA4` | `native_renderer_cached_states.cpp` | Current live `B244D1/B246CC` add one after returning SetRenderState/SetSamplerState COM calls |
| `1B94/1B9C` | `native_renderer_material_state_binding.cpp` | Explicit raw changed-binding counter increments |
| `1B98` | `native_renderer_cache_clear.cpp` | Raw increment after returning changed-binding work |
| `1BA8/1BAC` | `native_renderer_texture_stage_state.cpp`, `native_renderer_vertex_layout_binding.cpp` | Current post-call DWORD counter writes |
| `1BB4/1BB8/1BBC/1BC0` | `native_renderer_vertex_binding.cpp`, `native_renderer_index_binding.cpp` | Current stream/index/pixel/vertex binding counter writes |
| `1BC4/1BCC` | `native_renderer_texture_binding.cpp`, `native_renderer_surface_bindings.cpp` | Current texture/depth binding counter writes |
| `1BD0/1BD4` | `native_renderer_viewport_clear.cpp` | Explicit raw viewport/clear counter increments |

These concrete producers establish meaningful nonzero data inside the copied
bank; they do not assign exclusive meanings to each word. For example the
existing surface-target binder also increments `1BA0`. None is a dependency
needed to execute these complete block operations.

The previous readiness wording "query/container dependency B0CCB0" is
incorrect. The query loop only follows this call at `B2DADB..B2DB14`, using
the distinct pointer array `renderer+19A0`, count `+19A4`, query predicate `+8`
and virtual `+10`. Neither that query path nor full EndFrame is closed by this
packet. Queue execution and debug rendering remain separate integration work.

## Verification and limits

Read-only Ghidra batches use the guarded `bsp.py ghidra` CLI with configured
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 and image base
`00400000`. The full three listings, body ranges, xrefs and reached callees were
read. No Ghidra functions were missing; no Ghidra mutation or save was made.
Fresh live bytes match all 287 body bytes in the installed PE. The report
records exact bytes, per-body SHA256 and call rows.

The external fixture under `C:/Users/sqz269/bsp-ay-frame-statistics` retains an
untouched 321-byte original span containing the three bodies and intervening
padding. Relative branches remain intact after relocation. One focused Win32
comparison runs clear, publish and construction on distinct nonzero 40-DWORD
banks, 15 nonzero trailing words and four canary DWORDs on each side. It checks
the entire resulting storage and constructor receiver return. Source and
compiled instruction inspection establish store order; the fixture observes
final storage, not intermediate memory-write events.

Validation results are recorded in `reports/native_renderer_frame_statistics.json`.
The strict MSVC Win32 source compile, compiled instruction check, original-byte
fixture and five live report call-row checks passed. `scripts/build.ps1` also
passed, including both existing CTests (`reconstructed_math` and
`native_math_differential`); all eight seed spans matched beforehand.
Central CMake registration
belongs to the integrator; the worker's existing project build alone does not
prove linkage of this new file. No repository test or test framework was added.
Fixture agreement is not original-caller ABI compatibility, counter-producing
gameplay, complete renderer construction, complete EndFrame or game validation.

## AY integration analysis refresh

The integrator saved all sixteen AY original signatures and reviewed names,
verified full stored bodies and refreshed exports. CBBC8E, CBD436 and C64F13
are ten-byte analysis-only EH handlers defined under leases and the write lock.
Earlier missing-function observations are retained as worker capture history.
Two existing raw string bodies were extended separately; neither those
extensions nor the EH definitions add to the sixteen normal-body count.
Exact combined validation follows separately from worker fixture evidence.
