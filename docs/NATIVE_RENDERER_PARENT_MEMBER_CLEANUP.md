# Actual renderer parent member cleanup

These five entries provide the remaining member cleanup calls used by the
actual renderer constructor and destructor. Receivers are original storage;
the C++ names are descriptive hypotheses. They use existing raw reserve,
resize and allocation providers, preserving partial effects and stale fields.

| Entry | Inclusive native body | Receiver and behavior |
| --- | --- | --- |
| 8D4E60 | 8D4E60..8D4E9C, 61 bytes | 0Ch resolution header: reserve0 for negative capacity, decrement current positive count, capture data before final count0, free captured data |
| 86AE00 | 86AE00..86AE16, 23 bytes | 0Ch DWORD header: actual 86A430 resize0, free current data |
| B2F690 | B2F690..B2F6FC, 109 bytes | Actual renderer+1B18 capabilities; destroy nested header array at +50, then DWORD array at +44 |
| B2F700 | B2F700..B2F704, 5 bytes | Tail jump to B2F690 with the same receiver |
| 402F70 | 402F70..402F96, 39 bytes | Embedded tracked Win32 critical section: decrement positive depth before each leave, delete section |

All native entries receive ECX and return with plain RET. The source exposes
MSVC Win32 fastcall functions; host CRT/Win32 imports and C++ exception machinery
are separate from the original binary ABI. Raw valid extents, alignment and
allocation domains remain caller obligations; no bounds guards or rollback are
added. Pair/DWORD header data and capacity remain stale after final free.

B2F690 arms its DWORD-header cleanup before resizing/freeing nested headers.
The normal path disarms before B260B0(+44,0), followed by freeing current DWORD
data. Its source guard calls B29E40 on an exception before that disarm. Native
handler CBD7EB selects FuncInfo DF607C, whose one unwind-map row at DF6074 is
state0 -> -1 through CBD7E0. That funclet reads the captured capability receiver
from [EBP-10], adds 44h, and tail-jumps B29E40. This is renderer+1B5C when invoked
on the parent's actual capability member. A second source cleanup exception
terminates. Original FH3 frames, private spill aliases and asynchronous SEH are
not implemented or validated by this C++ guard.

402F70 retains the existing Ghidra name BSP_RecursiveCriticalSection_Destroy.
Its 1Ch storage is embedded, so it never frees an enclosing allocation. The
caller must own the recursive section acquisitions on the current thread.
Signed nonpositive depth skips leaving; negative depth is retained. The native
loop captures the LeaveCriticalSection import once, decrements current +18h
before each call, and reloads the signed loop condition. DeleteCriticalSection
follows regardless of initial depth. No fabricated section wrapper is involved.

The integrator recovered the returning-free tails of 8D4E60, 86AE00 and B2F690,
recreated their complete bodies under the Ghidra write lock, and verified zero
remaining flow gaps. Repair reports retain intermediate states as history.
Compilation and focused fixture evidence are recorded in the accompanying
report as they are completed. Full parent lifetime and gameplay remain open.
