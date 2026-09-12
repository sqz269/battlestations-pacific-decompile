# Actual logical vertex factory and lifetime

This packet reconstructs the complete normal bodies of seventeen routines in
`native_logical_vertex_owner.cpp`. The renderer factory returns the same raw
74h logical owner it registers. Its intrusive counter is the real DWORD at+04,
and its allocator uses the actual108FE18 pool's78h slots. There is no semantic
`LogicalVertexStream`, companion allocation pool, duplicate reference count,
or second renderer registry.

The source APIs require established actual physical owners, synchronization,
canonical final-zero ownership, and a real renderer-recreation provider.
`B29670` remains unreconstructed here. The constructor calls that required
service only on the native retry condition; a successful no-op provider does
not meet the contract.

## Reconstructed routines

| Native entry | Coverage | Original ABI | Source behavior |
| --- | --- | --- | --- |
| B287C0 | complete | ECX renderer; count/flags/declaration on stack; EAX stream; RET0Ch | Allocate, construct, append actual+1AAC, inspect current+58, optionally append actual+19B0. |
| B4B370 | complete | Incoming ECX ignored; EAX slot; tail JMP | Bind canonical108FE18 and run B4AE80. |
| B4AE80 | complete | ECX pool; EAX slot; RET | Real pool lock, slab growth, WORD pop, next-free slab scan. |
| B48EB0 | complete | ECX slab; stack slab index; EAX slab; RET4 | Initialize32 slots, free indices31..0 and per-slot+74. |
| B4BC00 | complete normal and native C++ unwind order | ECX owner; count/declaration/flags on stack; EAX owner; RET0Ch | Base initialization, actual dynamic or private backing, declaration retention. |
| B61E20 | complete | ECX owner; stack declaration; EAX owner; RET4 | Actual reference count, monotonic ID, metadata and raw declaration lookups. |
| B62010 | complete normal and native C++ unwind order | ECX owner; RET | Release actual+4C, free actual+50, restore base profile. |
| B4B5D0 | complete normal and native C++ unwind order | ECX owner; RET | Physical/renderer unregistration, declaration and physical release, base cleanup. |
| B4BF10 | complete | ECX owner; stack flags; EAX original address; RET4 | Destroy then return the slot iff flags bit0. |
| B4B1E0 | complete normal and native C++ unwind order | ECX physical; stack raw stream; RET4 | Optional guard around unique actual-pointer registration; no AddRef. |
| B22D10 | complete | ECX array; stack signed capacity bits; RET4 | Signed minimum1 reserve, real allocation/copy/free, current-header publication. |
| B1FE50 | complete, no Ghidra function record | ECX renderer; AL byte; RET | Read renderer+19AC. |
| B47C90 | complete | ECX declaration; usage/occurrence; AL bool; RET8 | Flat-record semantic presence; upper EAX is not part of source result. |
| B47CE0 | complete | ECX declaration; usage/occurrence; EAX index/-1; RET8 | Flat-record semantic index. |
| B47C40 | complete | ECX declaration; usage/occurrence; EAX offset; RET8 | Raw per-usage record DWORD0. |
| B47C20 | complete | ECX declaration; usage/occurrence; EAX type; RET8 | Raw per-usage record DWORD1. |
| B47C60 | complete | ECX declaration; usage/occurrence; EAX size; RET8 | Actual D61CC0 lookup using raw record type. |

The report records each inclusive body end and the final instruction's address
and length. B1FE50 is exactly seven bytes, `8A81AC190000 C3`; its inclusive end
is B1FE56 and its final RET is one byte. No function was created in Ghidra.
B62010's existing false CALL_RETURN omits B6206D..B62076. Those ten bytes are
`ADD ESP,4; MOV [ESI+50],0`, and the source includes that returning-free tail.
B22D10 and B4AE80 also retain post-free publication. No saved analysis changed.

## Construction and registration

The producer B61E20 installs CEB130, initializes actual+04 to1, publishes and
increments actual108FEE0, installs D62B68 and initializes only the observed
fields. In particular+4C/+50 start null, whereas absent semantic type fields
+14/+20/+2C retain their input bytes. It reads stride from declaration+CC and
uses the existing actual declaration layout: flat14h records at+0C and fifteen
per-usage array headers at+18+12*usage. The four usage queries are0,3,5,10,
occurrence0; the color offset is published only when its actual type size is4.

B4BC00 installs D61D6C and initializes+58/+68/+6C/+70 before optional guard
entry. Flags are preserved at+60. Their low nibble selects the D3D pool;
nibble0 adds the WRITEONLY usage bit through adjusted flags. Values above3
consume the original uninitialized pool local: the source requires its input
bits explicitly, corresponding to entry ESP-1Ch. It does not invent a pool.

Dynamic `(flags&F000)==1000` reads current renderer+1974 through the established
B1FEB0 contract, stores that physical at logical+58, retains physical+04,
captures the current renderer's inline lock+19F4 and increments that lock's
+18 depth. It reloads logical+58 for registration, then reloads F8D394 for
decrement/leave. Offset+5C becomes FFFFFFFF. Neither the inner tracked lock nor
the raw pointer array receives a synthesized ownership or rollback guard.

Private creation uses current renderer+1A10 and actual D3D virtual+68 with
wrapped `count*declaration+CC`, derived usage, FVF0, pool, the escaped temporary
COM output cell and null shared handle. The retry predicate is **null output,
nonzero HRESULT, excluding8876017C and8007000E**. Retry reloads the renderer and
device and rereads declaration stride. It then allocates and initializes the
actual private2Ch physical owner with D61E34, publishes logical+58, calls its
current+14 Attach through the established implementation, and unconditionally
reloads/releases the temporary COM pointer. Null failures are not repaired.

Count+64 and original flags+60 are written before the declaration assignment.
The assignment publishes/retains the incoming actual declaration, then releases
captured old. The guard is disarmed before normal leave. Native constructor
FuncInfo DF849C has two states: guard cleanup followed by B62010 base cleanup;
there is no derived physical/declaration rollback during failed construction.

B287C0 arms allocation cleanup only around construction. That cleanup's
CBD230 action reaches the established B49960/B49570 actual slot return.
After successful construction it appends the raw pointer to actual+1AAC,
including a null allocator result if execution permits one. It then reads the
current renderer profile D5F0A8+58, verifies its observed B1FE50 target, and
appends to actual+19B0 only when the returned byte is exactly1. Byte2 does not
qualify. Registrations do not retain the stream.

## Lifetime and canonical ownership

B4B5D0 reinstalls D61D6C and arms base cleanup before optional entry. Its flags
read/comparison precede the inner guard state. Dynamic destruction captures
the renderer's inline lock, reloads logical+58 for B4B3F0 removal, then uses the
current global renderer for decrement/leave. Non-dynamic destruction preserves
the otherwise-discarded physical+04 memory read.

B268E0 removes the raw pointer from current renderer+1AAC. Declaration+68 and
physical+58 are then reloaded separately and unconditionally decremented; the
native decrement import is the same InterlockedDecrement operation for both.
Fields are not cleared early. Declaration final-zero uses the established
canonical actual-owner registry. Physical final-zero dispatches only the
observed immutable D61E34/D61E7C profiles and their real private/pooled deleting
implementations. The secondary registry+19B0 is not removed by this destructor.

Normal guard leave follows disarming, then B62010 runs with the outer state
disarmed. Unwind guard/base/allocation helpers are nonthrowing cleanup actions,
so a second exception terminates as in the native FH3 cleanup domain. Original
SEH behavior on invalid raw pointers is not an implemented source contract.

B62010 reinstalls D62B68, captures actual+4C, decrements **that same owner's
actual+04**, resolves its canonical companion only at zero, and executes its
current terminal. It clears+4C only after return, reloads+50, frees and then
clears it. Normal and exceptional completion restore CEB130. The unknown
nonnull runtime producer/type of+4C remains a boundary: this packet establishes
the full ownership sequence without establishing any particular game writer.
A missing identity or mismatched counter is an error, never a no-op. This
supersedes the earlier null-only implementation impasse described in
`NATIVE_VERTEX_STREAM_OWNER_DISCOVERY.md`; it does not resolve that investigation.

`NativeLogicalVertexReference` is the single canonical companion for the
factory's exact raw stream. It borrows actual+04, neither initializes nor
retains it, accepts the current D61D6C/BD30E0/B4BF10 profile, and invokes the
real destruction/pool-return sequence. Its explicit retirement callback only
retires the companion after the native terminal; it does not manufacture
native renderer success. Callers bind the companion in their existing actual
registry. Existing B49980/B49A80 logical mapping APIs consume this same storage.

## Evidence and verification

All33 pinned spans,3079 bytes, matched both the installed PE and freshly guarded
live Ghidra reads. This includes17 owned bodies, original immutable profiles,
type-size table, EH actions/maps, and the returning-free continuation. The
installed image SHA256 is recorded in the report. The report carries69 call
rows; `verify_report_calls.py` checked47 direct rows with zero failures and
reported22 explicit indirect/native-import rows separately.

The new source passed MSVC Win32 `/MD /W4 /WX /O2 /fp:strict`. The existing
`scripts/build.ps1` build and `reconstructed_math` test passed. CMake integration
belongs to the primary agent, so that existing build is not the compilation
claim for this new source; its separate strict object compilation supplies it.
The eight existing installed-PE/live-Ghidra seed comparisons also passed.

The scratch `local/vertex_owner_at/probe.cpp` executes relocated original
instruction bodies and current source on actual storage. Its1012 compared
DWORD/scalar checks cover65 pool allocations per side (three slabs and pointer
array growth), all five semantic helpers through base construction, complete
dynamic factories with secondary bytes0/1/2, complete private factories with
flags0/1/10001, full destruction/pool return, signed pointer reserve, and the
base+4C/+50 final-zero cleanup. Private creation used a real D3D9 HAL device and
six real vertex buffers, checked through their actual GetDesc. Six source
stream terminals and one actual native-context terminal used the canonical
same-counter registry.

The +4C context is an explicitly assigned **fixture** identity with an already
reconstructed native lifetime; it does not prove the game's nonnull writer.
Optional guards were disabled in this differential run; the pool and inline
dynamic renderer locks were real Win32 critical sections. Device recreation,
returning allocation/COM failure faults, optional-entry transitions, exception
injection and runtime game adoption were not exercised. Existing synchronization
and physical provider verification remains separate dependency evidence.

The fixture relocated48 owned address operands and10 dependency address
operands, plus static pointer tables. Relative calls and all non-address
instructions remained intact. Its only function bridges replace four external
CRT allocation/free entries and the two already reconstructed physical Attach/
private-deletion providers. It does not replace any of the17 owned routines.
The Win32 import cells resolve to actual lock and atomic operations.

Reproducible local preparation and replay files are `evidence.py`, `relocate.py`,
`profile_cells.hpp`, `compile.cmd`, `compile_probe.cmd`,
`replay_current_library.cmd`, `probe.cpp`, and their
logs under that scratch directory. Initial replay links the strict new object
with the primary checkout's current `bsp_core.lib`, Lua and zlib libraries.
After integration the primary can remove that object argument and compile
**probe.cpp only** against its newly built current library; no private source
copy is needed. The generated original image/profile data and relocations are
fixture inputs, not installed game changes.
The fixture embeds its manifest and uses default stack and SAFESEH settings.

This is source and fixture verification of the specified bodies, not a drop-in
binary ABI replacement, full device reconstruction, rendering proof, or game
validation.
