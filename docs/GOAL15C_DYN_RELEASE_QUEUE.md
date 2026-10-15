# Dynamics pending-release queue append

Owned address: 00C34F70. Read dependencies: 00BF55BE/00BF681B allocator and
00BF6989/00BF65AC CRT free. This is queue append only: it does not destroy,
release references on, dereference, or drain any queued body.

| Routine | Native inclusive body | Bytes | Coverage | Original ABI |
|---|---|---|---|---|
| DYN_QueueBodyRelease_Provisional_00C34F70 | 00C34F70..00C35001 | 146 | complete | ECX manager; one stacked body DWORD; RET4; ESI preserved and EDI preserved on growth |

The source entry `queue_native_dyn_body_release_00c34f70` is MSVC Win32 fastcall:
ECX actual manager, unused EDX placeholder, stacked body DWORD, RET4. It has a
new public source identity, not an installed original physics entry. The native
body reads its body stack cell late at 00C34FF1. The naked source kernel retains
the exact current `[ESP+8]` load after resize/copy/free and only on the nonzero
slot branch, including mutations of that writable input cell before its use.
No EAX result is claimed. External provider frames and current CRT identity
remain qualified; the owned kernel does not replace the original physics owner.

## Actual fields and complete schedule

No complete world/manager record is introduced. Existing
`dyn_physics_substep.hpp` identifies pending-removal storage/count at manager
`+438h/+43Ch`, and `dyn_world_runtime.cpp` initializes those words and `+440h`
to zero. This packet operates on the existing bytes through extent `+444h`.
The native body itself establishes the three fields it uses:

| Offset | Observed use |
|---|---|
| +438h | DWORD-array base address |
| +43Ch | current unsigned DWORD count |
| +440h | DWORD capacity |

1. Read capacity, then compare current count with it. Growth happens on
   **equality**, not count greater than capacity (00C34F73..00C34F7F).
2. Compute `capacity*2+2` modulo 2^32 and publish it to `+440h` at 00C34F85,
   before allocation. Compute that captured capacity times four modulo 2^32;
   request exactly that many native and source bytes. No overflow or zero-size
   guard is added. If the real allocator throws, the already-published capacity
   remains and no local rollback, free, pointer publication, append or count
   increment follows.
3. After allocation, re-read current count. For every unsigned index below the
   current count, guard the computed destination address against zero; only on
   a nonzero destination load CURRENT `+438h`, read its indexed word, and write
   it to the destination. Increment the destination by four and the index by
   one modulo 2^32, then re-read CURRENT count for the next comparison
   (00C34F9D..00C34FC2). Copy writes that alias the manager fields are therefore
   observed by later loads; no count/pointer snapshot or bulk copy is added.
4. Load CURRENT `+438h` after the copy. If nonnull, free it. Only after the free
   returns publish the captured replacement pointer to `+438h`
   (00C34FC4..00C34FD7). Capacity is not written again after allocation.
5. Re-read count, then data; compute `data+count*4` modulo 2^32. If that computed
   slot is nonzero, write the supplied body DWORD (00C34FDE..00C34FF5). The
   guarded value is the computed slot, not the data pointer in isolation.
6. Increment CURRENT count at `+43Ch`, reloading it after the optional append
   store (00C34FF7). A slot that aliases count can change the incremented value;
   the source does not reuse the count from step 5.

All address and size arithmetic retains the native Win32 register instructions,
including DWORD wrap, without C++ pointer increment/difference or signed
overflow. The naked kernel retains the actual DWORD loads/stores, registers,
conditional branches, current-count memory increment and PUSH/POP sequence.
It adds no CPU fence, atomic operation, lock, callback, shape check or concurrency
policy. Only the two external service call destinations change.

## Concrete services and partial state

| Native site | Service | Checked contract and source composition |
|---|---|---|
| 00C34F91 | 00BF55BE | JMP 00BF681B; cdecl one byte-count argument, ADD ESP,4 at 00C34F9A. Use existing `singleton_lifetime_allocate({object,bytes,bytes})`. |
| 00C34FCF | 00BF6989 | JMP 00BF65AC CRT free; cdecl one pointer argument, ADD ESP,4 at 00C34FD4. Use genuine `singleton_lifetime_free`. |

The actual allocator body and source implementation were read: malloc, current
CRT new-handler retry, then `std::bad_alloc` on exhausted allocation. Its exact
requested source byte count is retained, including zero and wrapped sizes. The
free thunk and established real CRT free body were read in this worker session;
the current source service calls `std::free`. No substituted allocator callback,
forced success, fake physics destruction, or new exception policy is introduced.

Current source CRT handler selection, exception objects, private frames, heap
identity and SEH/fault addresses are qualified boundaries. Any new-handler
changes to the actual manager that return are seen by the subsequent count and
pointer reloads. There is no rollback of partial state after a returning handler
or later invalid/malformed memory access. Source allocations and any old storage
eventually freed must have matching CRT provenance; reached spans must be valid.

## Target and caller evidence

Live `bsp.py` queries verify project `bsp`, program
`/battlestationspacific.exe`, x86 image base 00400000, through the configuration
for `C:/Users/sqz269/bsp.gpr`. The complete pseudocode and assembly were read,
and both service sites were checked against the live containing-function body.
The related consumer at 0041E960 was read in complete assembly. Its concrete
site is `0041E99F CALL 00C34F70`, with each stored body pushed and ECX loaded
from the game's manager.

A fresh physical census finds eleven rel32 calls. Ten have live xrefs:
0041E99F, 004470A7, 00448279, 00745B41, 00748BBE, 007D0425, 00882738,
0088278B, 00929DCB and 0092CFE3. The eleventh is disk site 00C37543 in the
unanalysed bytes `00C37540 MOV ECX,[ECX]; 00C37542 PUSH EAX`, an E8 rel32
transfer at 00C37543 targeting the owned queue entry, and `00C37548 RET`.
Live Ghidra reports no instruction or
containing function there. The census's nearest-earlier-function label is not
body ownership: 00C37450's live body ends at 00C37480. This packet records the
raw wrapper discrepancy without claiming it belongs to that destructor,
reconstructing it, or changing the listing. It is a possible later bounded
register-wrapper packet requiring its own ownership and analysis.

## Naming, verification and integration

Saved old name: `DYN_physics_00c34f70`; saved inventory category: `block_dyn`
with the existing Dynamics-module inventory comment. The proposed descriptive
name `DYN_QueueBodyRelease_Provisional_00C34F70` keeps that library category and
marks owner/type identity provisional. No recovered original symbol is claimed;
the old name and category are recorded here and in the report before annotation.
The worker makes no Ghidra change or comment replacement.

Strict MSVC x86 `/std:c++17 /EHsc /W4 /WX /permissive- /O2 /c /FAcs` compilation
passed. Live native bytes were compared with the COFF function section: all 146
bytes match after masking exactly the two CALL rel32 operands at offsets 22h
and 60h. Every other byte is equal. The emitted listing confirms capacity publication before the real
allocation call, pointer/count reloads inside the copy, old free before new
pointer publication, append followed by CURRENT-count reload, and RET4.
Ignored artifacts: `local/output/native_dyn_release_queue.asm` and
`local/output/compile_dyn_release_queue.cmd`; the reproducible byte check is
`local/output/compare_dyn_release_queue_bytes.py` over the object and captured
live byte listing `local/output/native_dyn_release_queue_bytes.txt`.
The byte script accepts optional positional COFF-object and native-byte-listing
paths, so primary can pass `build/win32/bsp_core.dir/Release/native_dyn_release_queue.obj`
after the registered build; without arguments it checks the worker artifacts.

One focused ignored source probe linked the actual allocator/free implementation
and passed: initial capacity 0 to 2, spare append, growth 2 to 6 preserving old
words, an append slot aliasing count (body 7 produces final count 8), and a wrapped
null append slot with count wrapping to zero. Its executable embeds a manifest;
source/script are `local/output/dyn_release_queue_probe.cpp` and
`local/output/run_dyn_release_queue_probe.cmd`. This is source-only behavior
evidence, not an original-executable differential. No OOM was forced; failure
partial state and copy-header alias reloads have source/listing/owned-byte
evidence rather than separate runtime probes.

The primary integrator registers `src/native_dyn_release_queue.cpp` in `bsp_core`
and runs the required full `scripts/build.ps1` build/CTest. The parallel cleanup
packet consumes this public entry. Only the leased address's ledger records and
four owned outputs are committed; no CMake/packet registry/Ghidra edits occur.
Leases remain held for primary integration. There is no game/runtime or original
binary ABI-equivalence claim.
