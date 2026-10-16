# Tagged B4 leaf sweep 03

This registered module supplies twelve complete C++ semantic bodies. Eight have
exact whole Native instruction bodies; four are qualified semantic Source with
explicit remaining machine-state and fault-order limits. Names and interfaces
are hypotheses, not recovered class or field identities. Earlier packet files
were preserved. Current Main `97980d18209c2002ff07e821f81e20d9de5ec6d3` was merged
before qualification; Source coverage and leases were rechecked.

| Address | Coverage | Evidence state | Complete behavior | Native / actual Source bytes |
| --- | --- | --- | --- | ---: |
| 00b40120 | complete | semantic Source | clear raw dword at +0x10; return backing address | 10 / 10 |
| 00b40150 | complete | exact Native | return complete EAX=14; no consumed input | 6 / 6 |
| 00b40160 | complete | exact Native | raw dword read at +4 | 4 / 4 |
| 00b40170 | complete | exact Native | raw dword read at +4 | 4 / 4 |
| 00b40180 | complete | semantic Source | ordered raw dword zeros at 0,4,8; return EAX=0 | 11 / 23 |
| 00b40190 | complete | semantic Source | ordered raw dword zeros at 0,4,8; return EAX=0 | 11 / 23 |
| 00b40580 | complete | exact Native | raw dword at +0x50 != 0, complete EAX=0 or 1 | 9 / 9 |
| 00b40740 | complete | exact Native | raw dword read at +0x188 | 7 / 7 |
| 00b40850 | complete | exact Native | raw ECX identity | 3 / 3 |
| 00b40970 | complete | exact Native | raw dword read at +0xc | 4 / 4 |
| 00b40e80 | complete | semantic Source | word[0]+word[1]*4-4, unsigned modulo 2^32 | 10 / 12 |
| 00b41030 | complete | exact Native | raw dword read at +0x20 | 4 / 4 |

OriginalABI is measured from full bodies. Except constant `00b40150`, ECX is the
sole consumed argument; EDX/incoming EAX/stack arguments are not consumed. All
use plain RET without callee argument cleanup. Full EAX is the observed result,
including the pointer at `00b40120`, which pseudocode had omitted. The constant
has no receiver; its C++ spelling is `__cdecl ()`. Other interfaces use one-input
`__fastcall` without a fictional class or spare EDX parameter.

The reads and stores accept actual backed storage through their full spans.
`memcpy` permits unaligned raw bytes without typed-object/alignment narrowing.
Identity preserves every uint32 bit pattern. The arithmetic uses uint32 values,
so multiplication, addition and subtraction wrap exactly for every input pair.
It reads +4 before 0, matching the actual native and emitted access sequence.
No pointer-valued data, guards, defaults, globals or unresolved helpers are
invented. No new tests or executable probes were added.

The triple stores have one destination and constant source bytes: their three
four-byte destination spans are disjoint, with no external source buffer to
alias. Actual registered and strict objects both write dwords in order 0,4,8.
All twelve backing bytes must be writable; no invalid-memory/fault behavior is
claimed. They do not use a vector/wide store or a helper call.

The four semantic Source entries have these precise limits:

- `00b40120`: Native MOV EAX,ECX precedes the store through EAX. Source stores
  through ECX and copies ECX to EAX afterward. Valid-backed poststate and flags
  agree, but a store fault observes a different incoming EAX state.
- `00b40180` / `00b40190`: Native XOR EAX,EAX precedes all three register-value
  stores. Source uses immediate-zero stores and XORs EAX between the first and
  second stores. Valid-backed memory/result and defined return flags agree;
  EAX/flags at a first-store fault differ. Each whole Source section is 23 bytes,
  not an 11-byte Native prefix.
- `00b40e80`: Native reads +4 into EAX then 0 into ECX and finishes with LEA.
  Source reads +4 into EDX then 0 into EAX, uses LEA and ADD -4. The uint32 result
  and read order agree. Source leaves ECX as the backing address, clobbers EDX,
  and changes flags with ADD; Native leaves ECX=word[0], preserves EDX and flags.
  Fault-time register contents differ. No Original machine-ABI equivalence is
  claimed for these four new interfaces.

Strict MSVC Win32 `/std:c++20 /MD /O2 /W4 /WX /fp:strict /Gy` and the actual
registered `bsp_core` Release/Win32 object both account for all twelve nonempty
executable sections, with symbol offset zero, full section extents, and zero
relocations: **109 Source bytes / 36 instructions**. The exact subset is
**8 whole Native routines / 41 bytes / 18 instructions**. The twelve referenced
Native bodies total 83 bytes; that larger figure is not an exact-Native count.
Per-entry whole emitted/native bytes and operands remain separate in the report.

The append-only `cmake/startup.cmake` registration was added without a whole-file
lease. The required final registered build and both configured CTests passed;
this module was actually compiled. A preliminary eight-entry build completed
before the authorized semantic Source expansion; both build durations, failed
strict attempts and full handoff elapsed are included in timing artifacts.
The report-call verifier is run on this report and document. No Ghidra writes,
packet-registry edits, runtime path claims or game validation were made.

Nineteen unique bodies were inspected/revisited; twelve are included. Remaining
dispositions are explicit:

- `00b40040`: Native FLD m32real/RET is known. Both strict float-return source
  forms emitted a larger stack-mediated load, clobbering EAX and adding memory
  accesses. Float precision/x87 exception/fault equivalence and original return
  typing are not established. No changed return type was invented to fit bytes.
- `00b400f0`: Native reads absolute 0108FE90. Existing renderer construction,
  destruction and shader contexts reference the actual published cell, but this
  packet does not establish a direct canonical native-cell provider/placement.
  No substitute global was created.
- `00b401a0` / `00b401b0`: complete triple-zero Source candidates remain in the
  ignored handoff companion; omitted from the tracked module at the 12-entry cap.
  Their native bodies are known, and equivalent emitted-form differences are
  preserved in failed objects. These are not unexamined or counted as Native.
- `00b40750` / `00b40780`: actual stack destination / RET 4 plus sequential x87
  FLD/FSTP copies. Pseudocode bit copies are insufficient for overlapping aliases,
  x87 exceptions and the physical stack ABI. No Source closure is claimed.
- `00b40f10`: actual ADD dword[ECX+4],-1 / RET is known; its unsigned decrement
  behavior and ADD flags are established, but no compiled Source body was
  qualified within this packet. It is outside the 12-entry cap.

No known indexed callers or live xrefs establish unused status. The caller
endpoint's `function: null` reply is inconclusive. Raw live/PE comparisons,
failed full objects/listings, final strict and registered proofs, source handoff
and timestamps remain under ignored `local/goal15p_tagged_leaf_b4_03/`.
