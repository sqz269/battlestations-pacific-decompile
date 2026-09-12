# Native particle-model raw pool allocation

These five complete bodies operate on the actual 38h pool at `00F8D2D0`.
The new MSVC Win32 C++ interfaces borrow actual storage. Descriptive source
names are hypotheses; the interfaces do not reproduce the original ABI.

| Native inclusive range | Bytes | Original ABI | Coverage |
| --- | ---: | --- | --- |
| `00AF5C20..00AF5C62` | 67 | ECX slab, stack slab ID; EAX same slab; RET4 | complete |
| `00AF69E0..00AF6B1B` | 316 | ECX pool; EAX raw slot; RET | complete |
| `00AF6B60..00AF6B69` | 10 | Ignore incoming ECX, select F8D2D0, tail JMP AF69E0 | complete |
| `00AF60B0..00AF611B` | 108 | ECX pool, stack raw slot; RET4 | complete |
| `00AF62F0..00AF62FB` | 12 | ECX raw slot; select F8D2D0, CALL AF60B0; RET | complete |

## Physical storage and behavior

The owner producer `AF6860` writes the allocator-list prefix at +00/+04/+08,
initializes the real Win32 critical section at +0C, sets depth +24 to zero,
sets table/count/capacity at +28/+2C/+30 to zero and earliest +34 to FFFFFFFF,
then reserves 32 table cells. This packet does not create another pool or
substitute zeroed storage. `NativeModelPool` belongs to global `01090054`
with 188h slots and cannot represent this owner.

The slab producer `AF5C20` first writes free-count WORD +5C40 = 32. For each
index 0..31 it then writes free-index WORD `31-index` at +5C00+2*index and
the captured slab-ID DWORD at +2DC+2E0*index. The 5C44h slab thus contains 32
2E0h slots, each with a 2DCh payload and trailing slab ID. Payloads and final
padding WORD +5C42 remain untouched. Assembly preserves the original slab
in EAX through `RET4`, correcting the saved pseudocode's void result.

Allocation enters the real section and increments current depth. If earliest
is FFFFFFFF, it publishes current count as earliest, allocates 5C44h through
the existing CRT service and initializes nonnull storage with the then-current
earliest. If current count equals captured capacity, it publishes wrapped
`capacity*2+2`, allocates the wrapped DWORD byte size, copies cells while
reloading current table/count, frees the then-current old table and publishes
the captured replacement. It appends the captured slab at the current
table/count and increments current count after that store. Native null-cell
guards, wrapping arithmetic, and field reloads are retained without a size
clamp or invented rollback.

It selects the current earliest slab, decrements and reloads the WORD free
count, reads that free-index WORD and returns `slab+index*2E0`. The first
allocation is slot zero. When a slab becomes full, it captures the unsigned
later-index/count comparison, publishes FFFFFFFF, and scans later table cells
for a nonzero free count. Both the ordinary and exhausted-scan paths decrement
current depth and leave the real section before returning.

Raw return enters the section, increments depth, captures slot+2DC and the
selected current table entry, and computes the signed low32-bit slot-minus-
slab displacement divided by 736 with truncation toward zero. Assembly
`AF60D8..AF60E9` uses signed IMUL B21642C9, addition of the original delta,
SAR9 and sign correction. The source expresses that signed quotient. It
writes the truncated WORD at `5C00+old_count*2`, then reloads and increments
the free-count WORD; the reload matters when the free-index write aliases
free count. It lowers earliest by unsigned comparison, decrements depth and
leaves. There is no model destruction, pointer validation or slab reclamation.

Neither raw allocation nor raw return has a native EH frame. Actual allocation
failure retains the entered section, current depth and every prior publication,
including an unpublished slab. Source adds no lock guard or cleanup. The input
domain requires backing for every reached field/cell and a real initialized
section. The genuine pool trim binding must exist before shared-list publication.

## Calls, cleanup and boundaries

| Call site | Containing function | Native target / contract | Cleanup |
| --- | --- | --- | --- |
| AF69EA, AF60BA | AF69E0, AF60B0 | IAT CE2218, real EnterCriticalSection | stdcall callee pops4 |
| AF6A0B | AF69E0 | BF681B, actual CRT malloc/new-handler retry service | AF6A10 ADD ESP,4 |
| AF6A1D | AF69E0 | AF5C20, slab metadata producer | AF5C60 RET4 |
| AF6A3F | AF69E0 | BF55BE, tail JMP BF681B array allocation service | AF6A48 ADD ESP,4 |
| AF6A71 | AF69E0 | BF6989, returning CRT free thunk to BF65AC | AF6A76 ADD ESP,4 |
| AF6AFB, AF6B0F, AF610F | AF69E0, AF69E0, AF60B0 | IAT CE2210, real LeaveCriticalSection | stdcall callee pops4 |
| AF62F6 | AF62F0 | AF60B0, raw-slot return | AF6119 RET4 |
| AF6B65 | AF6B60 | tail JMP AF69E0 after selecting F8D2D0 | no stack argument |
| 874327 | 8742A0 | AF6B60, caller ECX=2DC is overwritten | no stack argument |
| C9636B | C96368 | tail JMP AF62F0, saved raw pointer in ECX | no stack argument |
| AF7F55 | AF7F40 | AF60B0 after AF6C50 and flag-bit0 check | AF6119 RET4 |

All current direct xrefs to the five entries were inspected. The raw-return
thunk is also used by constructor unwind; it does not repeat model destruction.
The deleting caller's AF6C50 operation is an external owner's body, not added
to this packet. The CRT bodies and array/free thunks were read before assigning
contracts; the existing `singleton_lifetime_allocate/free` services are reused.

## Saved-analysis corrections, left read-only

`AF6A76..AF6A78` is `83 C4 04` (`ADD ESP,4`), following the returning call
at AF6A71 and before table publication at AF6A79. Saved Ghidra flow omits the
three bytes and the decompiler falsely returns from the allocation function.
Integration must clear that call site's erroneous flow override, disassemble
the three bytes, preserve prior documentation and refresh the export. This
worker made no Ghidra mutations, renames, prototype edits, definitions or saves.

The separate `AF6AD9..AF6ADF` gap is `8D A4 24 00 00 00 00`, an alignment
LEA skipped by the unconditional jump at AF6AD7; it is not another missing
returning-call continuation. Both gaps are included in the 316-byte span.
The saved name `CG_static_dtor_stub_00af6b60` is incorrect: this is the raw
allocation thunk. No owned starts are missing. The earlier proposal's missing
startup start is stale: live Ghidra now defines `CD7830..CD7845` inclusively.

## Verification and dependencies

Every live query used the guarded `bsp.py ghidra` CLI, verifying the configured
`C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe` target before reading.
All five complete spans, 513 bytes, equal the installed PE. Strict separate
MSVC Win32 compilation uses `/W4 /WX /fp:strict /O2`; compiled return arithmetic
has the same signed IMUL/SAR9 sequence and allocation has no EH handler.

One ignored fixture relocates and executes all five original bodies, rebinding
only actual CRT allocation/free, the real Enter/Leave IAT cells, intra-packet
calls and the two explicit pool immediates. A bytewise runtime check validates
all 513 bytes outside the enumerated four-byte relocations/bindings, including
the returning-free continuation. Current execution links the unchanged owned
allocation object and a frozen primary `bsp_core.lib`. Both fixture pools use
the primary's actual AF6860 initializer and AF5FF0 destructor, with the real
AF6940 trim bound in their explicit allocator-list domains before publication.

The focused comparison passes complete 5C44h slab preimage equality (payloads
and padding preserved), 1057 initial allocations, capacity32-to66 growth,
full-slab skipping, LIFO reuse, varied ignored incoming ECX, both raw return
entries, nested OS lock/depth, and 31 further allocations followed by an
exhausted later-slab scan. Backed raw inputs additionally compare negative
displacement -737 and a free-index write aliasing free count. These abnormal
inputs verify unchecked arithmetic, not ordinary constructed-model use.

The failure comparison triggers a real `malloc(FFFFFFF8h)` failure and actual
CRT new handler returning zero. Both original and source throw `bad_alloc`.
The handler and caller observe depth1, count/earliest1FFFFFFE, published
capacity3FFFFFFE and unchanged old table. A second thread cannot acquire the
section both inside the handler and after the catch. Only after recording
those results does the fixture release the lock and restore descriptor bounds
for real owner destruction. The two intentionally unpublished slabs remain
allocated until fixture-process exit. This is not a claim of CRT/EH ABI identity.

The primary owns source/build registration, ledgers and saved-analysis updates.
Actual owner producer AF6860, trim AF6940, destructor AF5FF0, table cleanup
AF5D00 and startup/exit CD7830/CE0B90 are provided by the primary's separate
`native_particle_model_pool_owner` packet; the fixture consumes its actual
initializer/trim binding/destructor but does not execute its static lifetime.
Physical AF74A0 model construction, model lifetime composition, original ABI
compatibility and gameplay remain separate. No permanent tests were added.

The checked-in audit is `reports/native_particle_model_pool_allocate.json`.
Ignored reproduction is under this worktree's `local/`: `prepare_particle_pool_aj.py`,
`compile_particle_pool_aj.cmd`, `particle_pool_probe_aj.cpp`,
`run_particle_pool_probe_aj.cmd`, `particle-pool-aj-fixture.log` and
`particle-pool-byte-evidence-aj.json`. Full checkout build results and exact
artifact hashes are recorded in the audit; compiling this new module is
separately verified until primary source registration is integrated.

## AJ combined integration verification

The source is registered in bsp_core. The combined strict MSVC Win32 build and
both existing CTests passed. The report records the focused fixture replay, exact
call/tail checks, saved Ghidra name/signature preimages and comment readback.
Required external runtime bindings, original exception ABI and gameplay remain
limited as described above; this integration does not extend the fixture coverage.
