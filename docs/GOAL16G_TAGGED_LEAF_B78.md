# Tagged B7/B8 raw entries

Seven new standalone entries match their complete Native/live/installed-PE
bodies in both strict C++17 and actual registered Win32 objects: **93 bytes,
38 instructions, seven whole executable sections**. These are observed raw
operations. Names and new C++ interfaces are hypotheses, not recovered class,
field ownership or original formal return-type spelling.

| Address | Physical inputs / complete operation | Native / Source bytes |
| --- | --- | ---: |
| 00b726a0 | ECX address bits; EAX=ECX+98h modulo 2^32, no dereference | 7 / 7 |
| 00b72b30 | ECX backing; EAX=raw dword at wrapping ECX+50h | 4 / 4 |
| 00b87080 | ECX destination/EDX source; nullable one-dword copy | 9 / 9 |
| 00b87110 | Same two register inputs; nullable ordered two-dword copy | 15 / 15 |
| 00b875e0 | Two actual stack words; nullable one-dword copy, EAX=destination, RET8 | 19 / 19 |
| 00b87610 | Two actual stack words; nullable ordered two-dword copy, EAX=destination, RET8 | 25 / 25 |
| 00b88b50 | ECX backing saved in EAX; ECX=0, ordered zero stores at +4/+8/+Ch | 14 / 14 |

The getter, address calculation and fastcall one-word copy use ordinary C++17.
The two-word fastcall copy, both stack-argument copies and ordered clear use
literal MSVC32 instructions where full register/fault-prefix preservation
requires it. There are no helpers, unresolved external providers, globals,
fictional classes, invented receivers, extra guards or narrowed vector domains.

`00b726a0` accepts every uint32 ECX bit pattern, including zero and address wrap.
It emits LEA, leaves flags unchanged, and reads no object. The discovery pass
initially flagged `terrain_particles_cc13.cpp:2520` because it contains
`at(geometry(),0x98)` inside caller AE5160. Primary review correctly distinguished
that caller-local operation from a complete standalone B726A0 entry and its
physical input domain. Fresh typed metadata and live/PE bytes confirmed the full
seven-byte LEA/RET. The caller remains unchanged and is not counted a second time.

The raw getter reads only its actual four-byte field. Integer-width address
arithmetic and `memcpy` avoid adding typed-object extent or alignment assumptions;
the actual body is one MOV and RET. No semantic field type is inferred.

The fastcall copies consume both ECX and EDX as destination/source. Their initial
TEST supplies the returned integer flags; a null destination skips every source
read and data store, preserving incoming EAX. The one-word success path leaves
EAX=word0 and retains EDX=source. The two-word path reads word0 into EAX, stores
word0, then reads source+4 into EDX and stores destination+4. It leaves EAX=word0,
EDX=word1. A first store may alter the subsequent second read through partial
overlap; this order is deliberately retained. `memcpy(8)` or preloading both
words would not establish that behavior.

The stack copies consume destination at entry ESP+4 and source at ESP+8. There is
no consumed ECX receiver. EAX holds the destination, including zero. Source's
argument slot is loaded only after a nonnull destination test; null skips even
that slot read. Both use RET8, advancing ESP by twelve bytes including the return
slot. The one-word success path retains ECX=source and EDX=word0. The two-word path
retains EDX=word0 and ECX=word1 after ordered read0/store0/read1/store1. Both retain
the initial TEST flags. The new Source spelling is two-argument `__stdcall`.

The clear copies incoming ECX to EAX and XORs ECX to zero before any possibly
faulting store. It then writes dwords at +4, +8, +Ch, preserving EDX and those XOR
flags. There is no added null guard. EAX returns the backing address. Literal
instructions retain these register/flag prefixes at each store, rather than
merely establishing a valid-memory final state.

All non-stack-copy entries use plain RET and consume no stack arguments. There
are no prologues, pushes, spills, hidden local-stack accesses or cached return
slots in any final body. Inputs may alias one another and the incoming return or
argument slots; the exact actual instruction stream preserves the native load
schedule and CURRENT return-slot behavior. Only actual data accesses require
backing storage. There are no wide loads/stores or invented non-overlap guards.
x87 state/FIP and SSE state remain untouched. Byte identity establishes the
measured entry-body effects; relocated code/fault/return EIP addresses, caller
ABI adoption, installed entry replacement and game integration are separate.

Existing coverage was checked across worker, Root and Main Source/includes and
the indexed ledgers. The selected entries had no complete standalone Source
body. Related `native_pod8_range` and `native_pod8_inplace_range` providers have
larger range/cleanup contracts and do not provide these selected scalar entries.
Every selected body is call-free, so no old named-but-incomplete dependency is
treated as a real provider. B7/B8 index totals were 482/502 functions and 41/121
tag-only candidates; 35 native bodies were inspected. Unselected CRT-policy,
global/vtable-owner, ABI-sensitive and range-reuse candidates retain their own
unqualified status; no common blocker or whole Source absence is asserted.

Before creating the four unique tracked paths, worker, Root and Main were checked
at base b6c817689 and Main/Root 6ba7840db. Precise leases cover only the seven
addresses and new packet paths. There is one append-only CMake registration;
no whole CMake/ledger-shard lease or packet-registry edit was used. Old Native
names and tag metadata are retained in evidence for primary annotations.

Read-only typed metadata verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, language `x86:LE:32:default`, base `00400000`, and
complete single-range bodies; query entries have no flow override. Every full byte capture
came through the CLI and matched the installed PE. No Ghidra write occurred.

The first strict C++17 form compiled seven complete executable sections / 93
bytes with no hidden helper. Four entries / 34 bytes matched Native exactly.
The other three entries / 59 bytes preserved memory/alias order and flags but
allocated volatile registers differently: B87110 reused EAX for word1; B875E0
used ECX for word0; B87610 used EDX for source and ECX for both words. Their full
Source/object/log/timing and complete COFF proof are retained. They were replaced
with the literal schedules to satisfy the full register/fault-prefix contract.
No source prefix was clipped, and no compiler failure was hidden. A preliminary
batch launch failed before compilation because cmd interpreted a forward-slash
path as `local`; invocation through the absolute Windows path corrected it.

Final strict flags are `/std:c++17 /MD /O2 /W4 /WX /fp:strict /Gy`. Full strict and
registered objects account for every section/data byte, primary/auxiliary symbol,
symbol offset and relocation. The seven function sections are distinct and
exhaust all executable sections, each symbol offset zero. Executable relocations
are zero; three `.debug$F` type-seven metadata relocations target owned C++
functions. Exactly one real bsp_core archive member matches the actual registered
object. No helper omission, native-size prefix, relocation mask or tail exclusion
is used. The warm required build and both existing CTests passed in 18.3770982
seconds, including 7.78 seconds in CTest. No new tests or executable probes were
added. Native execution and game validation were not performed.

Before new compilation, the reviewed 16E Source/header/docs/report, full local
evidence, strict/actual objects and full archive were copied and hash-verified:
42 files / 94,048,923 bytes under `local/goal16g_tagged_b78/frozen_16e/`. Prior 16B
frozen evidence remains intact. The new archive still contains exactly one
unchanged member for each old 16B/16E provider; their Source/header/report pins
also remain unchanged. Their seven/21-byte and four/45-byte Source bodies are
not counted as new work here.

Timing retains the requested 1001-second discovery interval from 06:17:24 to
06:34:05 UTC, the 172-second scheduling/review gap, and implementation beginning
06:36:57 UTC. The report checkpoint and final ignored handoff extend the inclusive
clock from the FIRST discovery clock through qualification correction, all setup,
preservation, rejected/final strict forms, warm build, docs, ledger, commit, lease
release and author handoff. Later primary review/integration belongs to Root's
accounting. No controlled overall speedup or semantic-novelty claim is made.
