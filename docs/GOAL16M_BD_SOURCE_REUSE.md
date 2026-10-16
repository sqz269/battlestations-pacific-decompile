# GOAL16M: qualified complete Source reuse at four BD entries

Status: **READY — qualified complete Source reuse associations**. This packet
binds four additional Native addresses, 186 bytes/76 instructions, to four
existing callable Source bodies. It adds **zero Source functions and zero
Source code bytes**. No wrapper, header, CMake or test is added or changed.

| New Native entry | Existing complete Source callable | Body | Physical binding |
| --- | --- | --- | --- |
| 00BD3770 | `copy_native_pod8_range_00419960` | 44B/17 instructions | ECX first, EDX last; stack destination and three unread metadata DWORDs; fastcall six words, `RET 10h` |
| 00BD3A60 | `fill_n_native_pod8_range_0041c7e0` | 44B/17 instructions | ECX destination, EDX uint32 count; stack value and three unread metadata DWORDs; fastcall six words, `RET 10h` |
| 00BD3A30 | `fill_native_pod8_range_0041c680` | 36B/15 instructions | cdecl first/last/value in three stack words; plain `RET` |
| 00BD3BB0 | `copy_backward_native_pod8_range_0041dac0` | 62B/27 instructions | cdecl first/last/destination_end in three stack words; plain `RET` |

The first pair is in `src/native_pod8_range.cpp`; the second is in
`src/native_pod8_inplace_range.cpp`, with their existing headers. Fresh typed
single ranges and complete live listings/bytes were captured from
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. Each entire BD body
equals its entire canonical Native body, installed PE bytes and complete
actual Source executable section at symbol offset zero. This is whole-callable
reuse, not an inline fragment or prefix comparison. No prior reconstruction
record or reviewed-name row existed at these four addresses in worker, Root
or Main; the correct existing Ghidra/tag names are retained in new name rows.

The bodies are integer DWORD loops, with no x87/SIMD operations. They leave
the inherited FP data/status/control/tags/TOP and saved FIP unchanged. The
existing literal naked bodies preserve the exact physical schedules:

- **BD3770:** `CMP first,last` precedes the destination stack-word read, even
  for empty input. Nonempty input saves ESI, then tests the current destination
  each iteration; zero skips both reads and stores. Otherwise it reads word0,
  stores word0, freshly reads word1 and stores word1. Both cursors add eight
  modulo32 before the equality test. EAX is the advanced destination.
- **BD3A60:** `TEST EDX,EDX` makes the initial `JBE` equivalent to count zero.
  Zero preserves incoming EAX and skips the value stack-word read and ESI save.
  Nonzero captures the value pointer once. Each iteration tests the current
  destination and otherwise performs read0/store0/read1/store1, then decrements
  the full uint32 count, advances destination and tests count again. The value
  contents are reread each iteration; EAX remains the captured pointer.
- **BD3A30:** first and last stack words are read in that order. Equal endpoints
  skip even the value-pointer stack read. Nonempty input captures value once,
  saves ESI and repeats read0/store0/read1/store1, ADD8 and equality; no null
  skip or pair snapshot is introduced. Final EAX equals last incidentally;
  the existing Source declaration exposes no semantic return value.
- **BD3BB0:** last and destination_end are read before saving ESI and reading
  first. The result is computed before the empty test. For every uint32 delta
  `last-first`, including signed INT_MIN, SAR3 followed by three doublings
  yields `delta & FFFFFFF8h` modulo32; EAX is destination_end minus that value.
  Nonempty input captures destination_end-last as EDX bias. Each iteration
  reads source word0 at old-last minus eight **before** decrementing ECX and
  comparing; it then stores word0, freshly reads word1, stores word1 and
  branches on that same CMP. Empty input still saves/restores both ESI/EDI.

All data accesses are four bytes. Word0 stores can alter word1 rereads,
future source contents, argument/saved-register cells or the return slot.
The literal bodies retain those aliases and the precise partial-register,
flag and stack state at each fault prefix. Saved-register restoration means
POP from the actual saved cell; it is not an unconditional preservation claim
when an aliased store changes that cell. The three metadata words in each
fastcall kernel are discarded, never read. Normal ESP completion advances
14h for fastcall and four for cdecl; the caller retains cdecl argument words.

Successful ordinary terminating range traces finish with flags from an equal
CMP (CF/OF/SF/AF zero, ZF/PF one). The count loop finishes with TEST-zero flags
(CF/OF/SF zero, ZF/PF one, AF undefined). Earlier faults retain the exact
preceding TEST, ADD, SUB or CMP effects. MOV and stack transfers preserve those
flags. Full emitted-byte identity is the evidence for these schedules; no
Native execution, SEH probe or caller adoption is claimed.

No numeric, null, nonoverlap or normal-value restriction replaces the body:
wraparound, skipped reads, nontermination and all register-word patterns
remain represented. A meaningful normal C++ return requires the actually
reached storage/control cells and return path. Absolute instruction/fault EIP,
code placement, code/data or self-modification aliases and external caller/
runtime/SEH compatibility need an explicit placement mapping and are unproved.
The retained STL names and formal Source declarations remain descriptive
hypotheses, not recovered Original classes/templates/formal return types.

The full frozen registered objects account for **273 executable bytes/111
instructions/six existing functions**: range 175B/69 instructions/four
sections, in-place 98B/42 instructions/two sections. The four associated
sections have no relocations. Two existing dispatch wrappers account for the
other 87B/35 instructions and exactly two REL32 calls to the same existing
kernels; there are no metadata relocations, undefined externals or extra
helpers. Both objects byte-match their unique actual Core members. All 2,451
COFF members were inspected; all six public definitions have unique providers
and agree with both linker dictionaries.

Root inputs were frozen at `355324cabb770158b66fafae77819b6147a7c760`.
The supplied validated Core archive SHA-256 is
`51c8ae3849ec480650be89b18e2a8b7044485a68612f21a8ee8c60151215a1b4`.
K discovery's older object hashes/offsets are superseded by the current frozen
object/member proof in `reports/goal16m_bd_source_reuse.json`. Root's captured
Release Win32 project registers both existing Source files with C++17. Since
this packet changes no C++ or CMake, it runs no new build or checks; it relies
explicitly on the complete actual frozen Root providers. No supplied verifier,
new test/probe, Native/game execution or Ghidra write was performed.

The four ledger associations explicitly record existing Source reuse and
zero new Source code. Provisional physical prototypes/comments are proposed
per entry in the report, preserving the correct names; primary alone may
apply them under its Ghidra lock. Evidence is frozen under
`local/goal16m_bd_source_reuse`. The post-commit receipt records the separate
inclusive author clock from 2026-10-16 08:03:08 UTC, including corrections and
packaging. Independent acceptance is a later stage; no speedup claim is made.
