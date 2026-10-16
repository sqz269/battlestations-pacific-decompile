# B3 tagged leaf packet 04

This registered module contains **12 complete Source entries covering 69
Native bytes**: nine exact machine bodies (39 bytes) and three complete semantic
bodies with explicitly qualified machine-state effects (30 Native bytes,
36 Source bytes). The entire module has twelve executable COMDATs totaling
**75 Source bytes**. Native and Source module bytes are not claimed identical.

Main was merged as `a9fc0ea14f1154400aab14513442d071391cfa17` after the opening
brief. `docs/WORKER_VERIFICATION_CHECKLIST.md` and the current coordination lease
rules were read. Only this module's deferred `bsp_core` registration line was
appended to `cmake/startup.cmake`; that shared append-only file was not leased.
No Ghidra mutation or shared packet-registry edit was performed.

## Complete routines

Each entry consumes ECX only, has no stack argument/cleanup, and ends in plain
`RET`. No EDX argument, receiver class, default, global substitute, or guard is
invented. All names are operation hypotheses; Original symbols, class meanings,
and C++ calling conventions remain unknown. New interfaces use one-argument
`__fastcall` declarations in `include/bsp/native_tagged_leaf_b3_04.hpp`.

| Original | Native/Source bytes | Coverage | Evidence |
| --- | --- | --- | --- |
| `00B3E370` | 10 / 12 | complete | qualified semantic address formula |
| `00B3E4F0` | 10 / 12 | complete | qualified semantic address formula |
| `00B3E670` | 10 / 12 | complete | qualified semantic address formula |
| `00B3FF40` | 3 / 3 | complete | exact ECX-to-EAX identity |
| `00B3FF50` | 3 / 3 | complete | exact ECX-to-EAX identity |
| `00B3FF60` | 3 / 3 | complete | exact ECX-to-EAX identity |
| `00B3FF90` | 4 / 4 | complete | exact ECX+78h modulo32 address bits |
| `00B3FFB0` | 4 / 4 | complete | exact byte `[ECX+0Eh]` to AL; upper EAX retained |
| `00B3FFC0` | 4 / 4 | complete | exact dword `[ECX+34h]` to EAX |
| `00B3FFD0` | 7 / 7 | complete | exact dword `[ECX+170h]` to EAX |
| `00B3FFE0` | 7 / 7 | complete | exact dword `[ECX+190h]` to EAX |
| `00B3FFF0` | 4 / 4 | complete | exact dword `[ECX+08h]` to EAX |

All twelve have no identified live xrefs or direct caller setups. This is not
proof of unused code or indirect-caller closure. Every selected whole body was
read, its live extent checked, and existing typed/raw Source coverage searched;
none had an existing complete entry. No library name or existing interface was
changed. There are no calls, host bindings, or named-but-incomplete dependencies.

## Scaled-address semantic qualification

For all three scaled-address entries, let `S` be the unsigned32 word read at
receiver+4, then `B` the unsigned32 word read at receiver+0. Native and actual
registered Source read those same four-byte addresses in that order, write no
memory, and return `B + 4*S - 4 (mod 2^32)` for every word pair, including zero
and high-bit values. Source uses two raw `memcpy` reads and unsigned arithmetic;
it adds no empty-case or pointer-validation behavior. Backing must be readable
and stable for eight bytes. No alignment, record class, or field meaning is
imposed. This establishes complete normal-return Source semantics.

The complete pinned instruction sequences are:

```text
Native (10 bytes): 8b41048b098d4481fcc3
  MOV EAX,[ECX+4]; MOV ECX,[ECX]; LEA EAX,[ECX+EAX*4-4]; RET
Source (12 bytes): 8b51048b018d049083c0fcc3
  MOV EDX,[ECX+4]; MOV EAX,[ECX]; LEA EAX,[EAX+EDX*4]; ADD EAX,-4; RET
```

On normal return, Native leaves ECX=B, preserves incoming EDX, and leaves flags
unchanged. Source leaves ECX=receiver and EDX=S, and its final ADD changes
EFLAGS. The new C++ interface permits volatile ECX/EDX/flags; Original complete
machine-state/drop-in compatibility is not proved. The pinned builds retain
the same load order/widths, but intermediate register contents and fault/SEH
contexts differ. Invalid backing and concurrent mutation are outside the
Source qualification. No portable C++ fault behavior is claimed.

The initial exact-byte gate therefore failed honestly: all Source sections
were 75 bytes rather than 69, and each complete semantic COMDAT was 12 bytes
rather than its Native 10. Those full Source bodies remain implemented and
qualified; no NativeSize prefix, extra tail, or frame is omitted from proof.
The report retains complete Native and Source bytes/operands separately.

## Every inspected body and disposition

Twenty new Native bodies totaling 180 bytes were inspected. Twelve were
completed as above. The other eight remain explicitly deferred:

| Original | Native bytes | Coverage/disposition |
| --- | --- | --- |
| `00B3D500` | 3 | Source deferred: `XOR AL,AL; RET`, upper-EAX boundary |
| `00B3D5E0` | 85 | Source deferred: whole SSE copy read; entry stack +4 destination and +10h source, two middle slots unread; 10h local frame, `RET 10h`; separate ABI/overlap/fault qualification |
| `00B3DC80` | 1 | Source deferred: plain RET, outside selected twelve |
| `00B3DC90` | 3 | Source deferred: RET4 without consumed input; cleanup boundary |
| `00B3E380` | 5 | Source deferred: ADD dword `[ECX+4]`,-1; separate RMW/CF qualification and packet capacity |
| `00B3E500` | 5 | Source deferred: same ADD-memory decrement contract |
| `00B3E680` | 5 | Source deferred: same ADD-memory decrement contract |
| `00B3FFA0` | 4 | Source deferred: FLD32 `[ECX+18h]` to ST0; separate x87 return/exception qualification |

These deferrals have no unresolved named callee. All twenty inspected bodies
contain no call. Cumulative inventory: 67 of 79 bodies inspected, 41 complete
Source entries/189 Native bytes (38 exact entries/159 bytes and three qualified
semantic entries/30 bytes), 26 deferred, 12 uninspected. Candidate/tag/index
holes are screening hints rather than completion evidence.

## Registered verification and artifacts

The final registered `./scripts/build.ps1` passed both existing CTest suites.
The actual Release `bsp_core` object and its `bsp_core.lib` membership were
verified. Its actual CL tlog contains `/O2 /MD /W4 /WX`. Both standalone strict
and actual CMake objects account for every entire nonempty executable section,
require executable-code/COMDAT flags and symbol offset zero, decode full Native
and Source operands, and have zero section relocations or undefined externals.
Nine entries have whole-byte identity; three have the complete qualified
sequences above. Fresh live Ghidra bytes match the installed PE for all twelve.

The verified live client checks project `bsp`, configured saved project
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, x86 language and
image base. Exact hashes, full per-entry bytes, discovery details, compiler
provenance, and wall/build timing are in the report. No new tests or runtime
fixtures were added. `tools/verify_report_calls.py` is run on this report and
document. No frame/game/installation claim is made.

Ignored artifacts under `local/goal15p_tagged_leaf_b3_04/` include `audit.py`,
`compile.cmd`, strict `leaf.obj`, preserved actual `cmake_leaf.obj`, both proof
JSONs, full COFF dumps, live byte/prototype/xref captures, compile-command and
library-membership evidence, initial strict-gate comparison artifacts, and final
registered build logs. Reproduce with `compile.cmd`, `audit.py`, and `audit.py
build/win32/bsp_core.dir/Release/native_tagged_leaf_b3_04.obj` after the build.
