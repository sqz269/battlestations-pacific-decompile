# Native damageable-section uninitialized fill

Addresses: 008798F0 00878B40 00878F30 00878EF0 00C96620 00C96631
00DC884C 00DC885C 00DC8870 00DC8888 00401130 00BF6885.

## Scope and result

`fill_native_damageable_sections_008798f0` implements the complete logical body
of `008798F0..0087998C` over borrowed actual storage. It calls the real section
copy constructor from `native_damageable_section.hpp`, and preserves the
forward, current-vtable cleanup of the completed prefix. It neither owns a
vector nor supplies the larger append/insert routines `0087C870`, `0087C2D0`
or `0087B950`.

The original interval is 157 bytes: 59 decoded instructions including a
three-byte alignment NOP at `0087995D`. The live listing has 58 instructions;
that unreachable padding is not currently defined in Ghidra. The feature
summary reports only 42 instructions and one call, excluding the separately
owned `Catch_All@00879951` funclet.
The report derives its complete counts and call rows from installed bytes and
the current listing, not the feature summary. No listing repair is required.

All live queries were read-only through `bsp.py ghidra`, which verifies project
`bsp`, `/battlestationspacific.exe`, x86 language and image base. The configured
project is `C:/Users/sqz269/bsp.gpr`. Descriptive names are hypotheses.

## ABI and normal body

Original ECX is the first destination address; EDX is an unsigned 32-bit count.
The first stack argument, `[EBP+8]`, is the source record. Three further stack
DWORDs are ignored; `RET 10h` at `0087998A` removes all four. There is no
specified return value. The explicit C++ API omits the three ignored words and
adds the borrowed actual D0DF04-equivalent vtable identity required by the real
copy-constructor source. It is not a binary ABI replacement.

`00879915` publishes the current/completed end at frame -14h; -18h captures the
initial destination. For each nonzero count, -1Ch and -20h capture the current
placement address and state changes from 0 to 1. A nonnull current address calls
`00878B40` at `0087993E`; a null address skips that call. Only after return/skip
does the routine decrement count, add 30h to current, lower state to 0 and
publish the new completed end. The source pointer is reloaded for every copy.

There are no bounds, alignment, overlap or count-overflow guards. A zero count
reads no source or destination bytes. A null first address with count 1 skips
construction and still advances its private current address to 30h. Counts and
address arithmetic use modulo-32-bit behavior, not C++ signed overflow or
pointer arithmetic outside an allocated array. The source uses uint32_t for
that arithmetic and does not allocate, free or update a vector header.

The copy dependency is the genuine `copy_construct_native_damageable_section_00878b40`
implementation: native vptr, two integer fields, eight ordered x87 float
transfers, and a destination handle cleared before the source handle is read
and retained. In particular, self-aliasing drops that handle; later iterations
must read the now-mutated source. This fill does not snapshot a source record,
batch copies with memcpy, or synthesize a virtual table.

## Complete exception path

Handler `00C96631` loads `00DC8888` then jumps to `00BF6B43`. FH3 metadata has
magic 19930522h, maxState 3, unwind map `00DC8870`, one try block at `00DC885C`,
no IP map and EH flags 1.

| State | Parent | Action |
| --- | --- | --- |
| 0 | -1 | none |
| 1 | 0 | `00C96620`: push frame -1Ch and -20h, call `00401130`, discard both arguments |
| 2 | -1 | none; catch execution state |

`00401130` is exactly one RET instruction. Failed placement construction has
no destructor, free or rollback action. This verified no-op is represented by
doing nothing, not by adding another callback.

The catch-all record at `00DC884C` has flag40h, null type and object displacement,
and handler `00879951`. Its try covers states 0..1 and catch-high state is 2.
The catch captures the initial address and completed end once, then:

1. Reads each completed record's **current** vptr and vtable slot0.
2. Calls that slot with ECX=record and stack flags=0 (`00879968`).
3. Advances by30h and continues forward until the captured end.
4. Rethrows through `00BF6885(0,0)` at `00879975`.

The failed/current record is excluded. Cleanup does not clear records, free
storage or change the completed-end boundary. Slot0's original D0DF04 target is
`00878F30`, the scalar-deleting wrapper, whose flags0 path destroys without
freeing. The source retains the established borrowed actual virtual-call
boundary, as does existing `00879240`; this packet does not claim reconstruction
of the complete virtual family or replace dispatch with a fabricated table.

Cleanup executes inside an explicit catch, not a destructor unwinding another
frame. If a slot call throws, the new exception escapes and subsequent records
are not cleaned. The source deliberately adds no terminate handler. If every
slot returns, bare `throw` preserves the original exception using the current
C++ runtime. The original FH3 personality, private frame aliases, translated
SEH/fault behavior and double-exception runtime details remain unproved.

## Validation boundary

`reports/native_damageable_section_fill_orch4.json` records the exact original
hash, full callsite/EH data, strict Win32 build, existing tests, and focused
fixture results. The fixture uses actual copy-constructor source and compares
the original fill instructions against the reconstruction for the normal path,
including source aliasing. Catch behavior is established by full assembly and
FH3 metadata; no injected-copy mock is treated as provider or native-EH proof.
No game validation or original binary-ABI replacement is claimed.

`scripts/build.ps1` passed in default strict Win32 Release mode; both tests
enabled in this fresh worktree passed. The focused probe compiled with
`/EHsc /W4 /WX /O2` and `/link /MANIFEST:EMBED`, then passed count0,
null/count1 and the count3 middle-source-alias case. Final references were6
from an initial5, and the nonnull handle pattern was `[1,0,0]` in both versions.
All three live call rows passed, including the rethrow attributed to its actual
Ghidra owner `Catch_All@00879951`. A separate read-only worker review found no
source/header/metadata discrepancy. No permanent test suite was added.
