# GOAL16N: raw zlib reset and type operations

This packet supplies two complete callable Source bodies behind new ECX-only
raw-backing interfaces. Correct Native library names `inflateReset` and
`set_data_type` are retained. Formal pointer/fastcall/result spellings are new
interface hypotheses; no Original z_stream/deflate layout, version, historical
compiler flag or callback ABI is recovered from them.

| Native entry | Library operation | Whole bytes | Instructions | Source interface |
|---|---|---:|---:|---|
| 00BC9650 | inflateReset | 69 | 25 | int32 fastcall(raw stream backing) |
| 00BCEE90 | set_data_type | 132 | 40 | void fastcall(raw state backing) |

Both strict and actual registered MSVC32 sections match the entire live and
installed-PE bodies: 201 bytes / 65 instructions, no prefix clipping or helper
omission. Literal readable instructions were selected to retain cache, ordered
storage, flags, fault prefixes and arbitrary stack/backing aliases. Exact code
identity is evidence for these bodies, not a universal Source admission rule.
Full evidence is in `reports/native_zlib_raw_reset_type_bc.json`.

## Reset physical contract

Only ECX is an input. XOR EDX,EDX occurs before either test. Null ECX returns
EAX=-2 without a data read. Otherwise read DWORD[ECX+1Ch] once into EAX; a zero
state address also returns -2. Error EDX=0 and ECX is the original input; MOV
EAX,-2 preserves the last equal-to-zero CMP flags. No object guard beyond these
two observed tests is introduced.

Success caches the state address in EAX and stores zero, in order, to
state+18h, stream+14h, stream+8, stream+18h. LEA then sets ECX to state+528h,
wrapping at 32 bits, without dereferencing that value or imposing an object
extent. Store zero to state+0,+4,+0Ch,+20h,+24h,+30h,+34h in that order, then
store ECX to state+64h,+48h,+44h in that order. Aliasing stores do not cause a
state-pointer reload or change which address was cached. No memset, gathered
stores, allocation, canonical global, provider or private class is substituted.

The final XOR returns EAX=EDX=0, with ECX equal to the computed address bits.
Its flags have CF/OF/SF zero, ZF/PF set, AF undefined. Other registers are not
written. There are no stack saves, argument-slot reads or hidden local accesses;
plain RET reads the actual current return slot and advances ESP by four. Every
individual load/store fault prefix is retained, including aliases with either
backing, the state-pointer cell, and the return slot.

## Type physical contract

Only ECX backing is consumed; incoming EDX is not an argument. Read unsigned
WORDs at +A0h,+A4h,+9Ch,+98h,+94h,+90h,+8Ch in that exact order, interleaving
the observed ADDs. The seventh load precedes PUSH EBX, PUSH ESI, XOR ESI,ESI,
PUSH EDI; its ADD follows those saves. The observed alignment instruction is
the three-byte LEA ECX,[ECX+0], encoded explicitly as 8D4900.

Sum 121 unsigned WORDs starting at +A8h with stride four into ESI, then sum
128 unsigned WORDs starting at +28Ch with stride four into EAX. The first seven
words also contribute to EAX. All WORD patterns are admitted: the sums are at
most 121*65535 and 135*65535, so they fit uint32. Fixed counters bound these
loops; no data-dependent nontermination, input normalization, snapshots,
widened loads, table ownership or guessed structure extent is introduced.

SHR ESI,2 and unsigned CMP EAX,ESI determine SETBE AL. The exit schedule is
POP EDI, SETBE AL, POP ESI, BYTE[ECX+1Ch]=AL, POP EBX, RET. EAX upper 24 bits
retain the binary sum, while AL is zero or one; the void interface invents no
semantic whole-EAX return. EDX ends at backing+48Ch modulo 2^32, ECX stays
backing, and final flags remain those of CMP. Saved registers are restored
from actual stack cells: a backing/store alias can affect the later POP, so
unconditional register-preservation claims are not substituted for the literal
schedule. Pushes can also change subsequently read cells when storage aliases
the stack. Every such ordering and fault prefix remains in the complete body.

## ABI, Source and placement qualifications

Neither body consumes stack argument words or an EDX input; both use plain RET.
Raw types and formal declarations are provisional, not Original C++ recovery.
No x87/SIMD instruction occurs; inherited FP state and x87 FIP are unchanged.
All effective-address arithmetic is x86 wrapping arithmetic, with no invented
alignment or range restriction. Ordinary completion requires actually reached
data/stack/control cells to be backed; invalid accesses retain measured faults.

Whole instruction identity proves the current artifact's register/flags/storage
and fault-prefix schedule. It does not place Source at Original code addresses,
preserve code aliases against Original absolute placement, reproduce fault or
return EIP values, establish Native SEH/runtime/full ABI compatibility, adopt
callers, replace installed binaries or validate game behavior.

B4's sealed discovery screened 21 names and inspected nine whole Native bodies,
927 bytes / 309 instructions. Only these two are implemented; seven inspected
functions / 726 bytes remain deferred and twelve names were not whole-inspected.
Its full Source proof maps fetched zlib 1.2.1 Source, ten actual C objects, the
separate bsp_zlib121.lib, 38 unique public functions through both dictionaries,
and consumer links. All 21 mapped candidate Source extents differ from Native.
The selected stock sections are 119 and 234 bytes with cdecl/stack and storage
schedules differing from Native 69 and 132 bytes. They are algorithm evidence,
not whole physical reuse. Existing stock callback cdecl versus Native ECX/EDX
differences and parent adoption remain outside this packet. A default-fastcall
Original compiler option is only one hypothesis; no Original flags/version or
layout claim is made. No stock API, callback or consumer was redirected.

## Artifacts, preservation and time

Each batch verified C:/Users/sqz269/bsp.gpr, /battlestationspacific.exe,
base 00400000. Fresh whole typed/live/PE/instruction capture precedes Source.
Sealed whole Source coverage was read before coding; a later tracked-reference
recheck records current worker/Root/Main/origin revisions without changing that
chronology. Four new paths were verified absent before creation.

Strict C++17 /MD /O2 /W4 /WX /fp:strict /Gy and registered Release|Win32 C++17
objects each have exactly two complete executable sections, symbols at offset
zero, and no helper, undefined provider, code or data relocation. Every section,
symbol/auxiliary and data byte is recorded. Both Core linker dictionaries
uniquely resolve the two decorated symbols to the same unique real matching
object member. Required scripts/build.ps1 session73915 exited zero in
18.8258716 seconds; existing CTests passed 2/2 in 7.99 seconds. No Source/compiler
form failed; no new tests, probes, Native execution or game validation occurred.

Before Source/compilation/build, 680 complete prior K/J/G/E/B5 input files,
904,355,496 bytes, were copied with hashes, including full packet trees/current
objects/Core/project/tlogs and historical K inputs. The full N discovery copy
contains 126 current files / 14,799,400 bytes; its sealed set is 124 files /
14,765,089 bytes, plus administrative files. Handoff/report pins match B4 and
Root. Prior Source/header/report and unique actual providers remain unchanged.

The full clock starts at B4's 2026-10-16 08:20:15 UTC: discovery 1057.870715 s,
administrative/scheduling gap 436.129285 s, author implementation from 08:45:09.
All preservation, analysis, compilation/build, checks, artifact/dictionary proof,
docs, ledger/index, commit, lease release and handoff overhead remain included.
The final local receipt supplies the complete author endpoint; no throughput or
controlled speedup claim is made. No Ghidra write occurred. The report proposes
provisional physical prototypes/comments for Root after independent review,
preserving library names and previous annotations. Root owns integration.
