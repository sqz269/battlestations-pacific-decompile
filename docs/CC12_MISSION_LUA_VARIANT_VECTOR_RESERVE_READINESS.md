# Raw mission Lua variant vector reserve: 006B87F0

The complete `006B87F0..006B88D2` body is 227 bytes and independently decodes to
77 instructions. Its normal path allocates a replacement, transfers current
elements through `00886DA0`, retires current old elements through virtual slot
zero with argument zero, frees the current old buffer, then publishes the new
data pointer and capacity. It does not locally change the header count.

**Source closure remains held.** The transfer child has no current address-bound
Source implementation, actual virtual targets and callable profiles remain
unproved, and the local exception handler is unopened. Existing allocator/free
services and the admitted `008849B0` initializer do not close those contracts.
This packet is read-only Native/Source evidence: no C++, CMake, ledger, Ghidra,
tests, probes, builds, Original execution or new reconstruction credit.

## Scope and complete-body evidence

- Published baseline: `2fa724372622e47bb1b10a9feb5c25492e5402a1`.
- Owner/lease: `agent/cc12_settings_live_fov_cell` /
  `cc12_lua_variant_vector_reserve_readiness`.
- The 227-byte metadata extent passed the 300-byte gate before body capture.
- Live CLI batches verify existing `C:/Users/sqz269/bsp.gpr`, project `bsp`,
  `/battlestationspacific.exe`, `x86:LE:32:default` and image base `00400000`.
  Live/snapshot function counts both remain 64,729.
- All 227 live bytes equal the original PE. Full independent decode reaches
  the actual `RET 4` at `006B88D0`, ending at `006B88D2`.
- Whole-span SHA-256:
  `576f1a11a1fd789f562957cde4d47009f0c2e4846ffffbf64d279f6726c2a72d`.
- Only direct children received Native metadata queries. No child, handler,
  caller, profile, import target, heap body or adjacent body was opened.

Ghidra's current listing contains 72 instructions. It omits the following ten
bytes/five instructions after the ordinary `_free` call at `006B88B1`:

| Site | Bytes | Actual instruction |
| --- | --- | --- |
| `006B88B6` | `83 c4 04` | `ADD ESP,4` |
| `006B88B9` | `5f` | `POP EDI` |
| `006B88BA` | `89 2e` | `MOV [ESI],EBP` |
| `006B88BC` | `89 5e 08` | `MOV [ESI+8],EBX` |
| `006B88BF` | `5d` | `POP EBP` |

Those bytes establish publication and saved-register restoration omitted by
the pseudocode. They lie wholly inside the owned extent and were decoded from
the matching live/PE span. No flow repair or exact no-return flag query was
needed or performed; the omitted listing is not evidence that free cannot
return. Both paths join the normal FS restoration and return at `006B88C0`.

## Header and ordered normal path

Let `H` be entry ECX and `R` the incoming DWORD at entry `[ESP+4]`. The owned
body uses three complete DWORDs: current data at `H+0`, current count at `H+4`
and capacity at `H+8`. This establishes a 12-byte consumed header, not a complete
class. Comparisons of request, count and capacity are **signed**.

1. The function installs its local FS exception record, captures R in EBX and
   H in ESI. If signed R is below one, `006B8815` writes one back into the actual
   incoming argument slot and reloads EBX. Call the normalized value N.
2. If signed current capacity is at least N, it takes `006B8824 -> 006B88C0`.
   No data/count access, header store or child call occurs on this path. The
   argument normalization and local exception-frame setup still occurred.
3. Otherwise `LEA` and two `ADD`s form `(20*N) mod 2^32`; the DWORD byte count
   is pushed to `00BF55BE` at `006B8834`, with caller cleanup of four bytes.
   There is no overflow, count-versus-capacity, byte-size or result-null check.
   The returned address A is retained in EBP and a stack local.
4. If current count is positive, an ascending transfer pass starts at i=0.
   Each iteration recomputes `offset=(20*i) mod 2^32`, computes destination
   `A+offset`, saves it, and sets local unwind state zero. Only a zero **computed
   destination** skips the child. Otherwise it reloads current `[H]`, computes
   source `[H]+offset`, pushes that source and calls `00886DA0` at `006B886F`
   with destination in ECX, source also in EDX and offset still in EAX.
   After full return or the null skip, i increments; the function compares i
   against freshly read count, resets unwind state to -1 and saves the new i.
   Signed `i < current count` repeats. The three-byte `LEA ECX,[ECX]` at
   `006B884D` is reached only once before the first iteration.
5. A separate retirement pass starts only after that transfer pass finishes.
   It rereads count, and if positive sets i=0 and byte offset=0. Each iteration
   reloads current `[H]`, reads the element's DWORD at `current data+offset`,
   reads profile slot zero, and calls that target with ECX equal to the element
   and one pushed DWORD zero. It increments i and offset by 1 and 20, then
   compares i with fresh current count. It does **not** locally decrement count.
   Zero is an observed argument, not proof of the target's deallocation policy.
6. If the retirement loop was entered, `006B88AA` reloads EBX from the incoming
   argument slot. If it was skipped, EBX still holds N. With an unchanged slot
   these agree; a callee-induced alias mutation must not be hidden by caching N.
7. `006B88AE` reloads current `[H]`, pushes it and calls `00BF6989`. Only after
   its normal return does the omitted tail clean that argument, restore EDI,
   store A at `[H]`, store EBX at `[H+8]`, and restore EBP. It then restores the
   original FS chain, ESI and EBX and executes `RET 4`.

Every selected element uses a 20-byte pitch. The reserve body does not call the
default initializer and has no separate initialization pass for spare capacity.
The transfer child's full source/destination footprint and copy, move, retain
or ownership semantics are unknown. A byte-copy or `std::vector` replacement
would invent that behavior. The retirement path directly consumes only the
element's first DWORD and the first DWORD at that profile; these reads alone
do not establish the rest of either layout.

## Mutation, failure and lifetime boundaries

The allocation result is captured, but data and count are reread across child
boundaries. Transfer and retirement have independent fresh count gates. Counts
and data can change between or during passes; the final free receives the data
word current immediately before that call. Neither a fixed initial count nor
a captured original data pointer preserves this schedule. The capacity gate is
not rerun. Final local stores overwrite data and capacity, while count retains
whatever value it has after the children. No fixed iteration count, once-only
retirement, stable element identity or finite completion is guaranteed under
arbitrary callback mutation or malformed aliasing.

Signed requests below one, including negative DWORD interpretations, become
one. Nonpositive count skips both element passes but still permits allocation,
free and publication when capacity is insufficient. A null H faults at its
capacity read. No backing-size, pointer-alignment, arithmetic-overflow or
overlap validation occurs. A count larger than allocation capacity is not
clamped. Caller storage must keep H alive through the free and final stores;
aliases between the header, buffers, elements or call stack are not guarded.

If an allocation boundary were to return null, the body has no whole-buffer
failure branch: only a computed zero destination is skipped. Positive offsets
from zero can still call the transfer child with invalid nonzero destinations.
If both element passes are skipped, ordinary return still frees current data
and publishes null with the selected capacity. This describes the caller's
control flow; it does not assert that the actual allocator returns null.

On the normal path, all transfer calls precede retirement calls, which precede
the free, which precedes publication. A transfer/retirement/free exception or
fault occurs before those local publication stores; callees may already have
changed other state. The body gives no local rollback or ownership guarantee
for that failure. New storage can be retained in a local while old elements are
partly transferred or retired. The unopened handler may perform cleanup; its
absence from this audit must not be turned into a leak or no-cleanup claim.

## Stack, registers and local exception frame

Let S be entry ESP. The function saves state -1 at `S-4`, handler `00C80DF7` at
`S-8` and the old FS head at `S-12`, then publishes `FS:[0]=S-12`. It reserves
12 local bytes and saves EBX/ESI. Growth additionally saves EBP/EDI. In the
growth body, completed-transfer index is at `S-24`, A at `S-20`, and current
destination at `S-16`. State becomes zero before the transfer-child gate and
returns to -1 after each completed/skipped iteration. It remains -1 for the
retirement and free phase. The handler itself was not inspected.

The original input delivery is ECX=H and one stack DWORD R; the body requires
no other incoming data register and no x87 state. The allocator and free have
explicit caller cleanup. Transfer and virtual calls require compatible
four-byte callee cleanup for the observed frame to balance; their complete
callee ABIs have not been proven by this caller audit. Growth relies on their
preservation of ESI=H, EBP=A and loop/capacity holders EBX/EDI. All four incoming
nonvolatile registers are restored on normal return; `RET 4` consumes R.

There is no meaningful typed return established. Without growth, EAX retains
the old FS head loaded at entry. After growth, EAX is the residual from free;
the publication tail does not replace it with H or A. Normal exit reloads ECX
with the old FS head before restoring it. No bespoke hidden ABI or x87 evidence
was found in the owned body. Exception/fault ABI compatibility remains open.

## Current Source and next packet

- `00886DA0`: Native metadata only, 84 bytes / 24 instructions / one named
  child `00886C10`. Exact current `include/src` address search and reconstruction
  index find no Source implementation for the transfer child or reserve.
- `00BF55BE`: Native metadata only, five-byte entry. The index points to
  `NativeMpkgRuntimeServices::allocate_00bf55be`, which delegates to its pool;
  this is not a binding for the mission vector. Concrete default methods in
  `NativeGameContainerLifetimeCalls` also supply allocation and free services.
  Its allocation passes `{object,n,n}` to `singleton_lifetime_allocate`, whose
  current Source uses `malloc`, retries on a nonzero `_callnewh` result and
  otherwise throws `bad_alloc`; its `free_00bf6989` uses `std::free`. These are
  real method bodies, not a constructed vector dispatcher or heap-identity proof.
- `00BF6989`: Native metadata only, five-byte cdecl `free(void*)` entry. No
  import target or heap implementation was opened.
- `008849B0`: the admitted current Source initializer and primary receipt are
  pinned. The published receipt records the complete 27-byte match, zero
  relocations, unique Core definition and normal build with three existing
  checks. This worker did not rerun them. Reserve never calls this initializer,
  and its numerical `00D0E6F4` stamp still supplies no callable Source profile.

The next small ready evidence dependency is the complete **84-byte `00886DA0`**
body, under a fresh lease and metadata gate. Its named child remains a dependency
to classify, not assumed closure. Handler `00C80DF7`, actual virtual targets,
raw backing/allocator compatibility and owner integration remain separate work.
No profile or handler expansion is authorized by this report.

The companion JSON retains all 77 instructions, seven branch sites, four call
sites, exact stack/header observations, historical parent-pin replay and current
canonical Source pins. Current Source files match their canonical bytes exactly.
The historical initializer worker JSON has physical CRLF endings and canonical
Git LF endings; both hashes and that distinction are recorded. JSON, two-file
scope, pin replay and `git diff --check` are the only worker validation beyond
evidence inspection.
