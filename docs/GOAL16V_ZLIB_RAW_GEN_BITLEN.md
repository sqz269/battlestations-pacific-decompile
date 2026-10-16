# GOAL16X: complete gen_bitlen kernel and frame entry

READY: this implements the accepted GOAL16V readiness packet for only
00BCDF60 `gen_bitlen__prov`. Both owned Native ranges are complete:
00BCDF60..00BCE0A9 is330B/86instructions and00BCE0B0..00BCE176 is199B/58;
total529B/144instructions. One private Source kernel linearizes all144 named
instructions. Its measured extent is526B, plus a genuine17B/four-instruction
frame adapter: whole Source module543B/148instructions, two Source functions,
one Native address/529B coverage. Adapter earns no additional Native credit.

## Topology and complete instruction mapping

The six-byte unowned alignment LEA at00BCE0AA..AF is not copied or credited.
Native JMP at00BCE0A8 skips it to00BCE0B0. All18 Native direct branches target
owned instruction starts; the two ranges share one plain RET at00BCE176.
Original no_return remains false. No listing/flow/body/Ghidra repair was made.

All144 emitted kernel instructions match Native operation, register, memory
width/base/index/scale/segment/displacement, effective immediate, address size,
prefix and mapped branch destination. The Source jump across the omitted hole
is EB00, targeting the following correction instruction. Two WORD ADDFFFFh
encodings use sign-extended imm8 FF instead of imm16 FFFF; the effective16-bit
operand and all arithmetic flags are equal. The executed LEA ECX,[ECX] uses
two bytes instead of three and remains an instruction with no flags/data read.
These three one-byte shortenings explain526 versus529 owned bytes. Five branch
encodings also change for the compacted layout: eight changed encodings total.
No raw embedded instruction shortcut or prefix clipping is used.

Full per-instruction Native/Source bytes, semantics and branch-index mapping
are retained for strict and actual registered objects. This is qualified whole
instruction-effect evidence, not a byte-identity policy or absolute address,
instruction-fetch/self-alias/fault/return placement equivalence claim.

## Real inputs and chosen Source interface

Native/private kernel inputs are EAX raw backing and ECX raw descriptor. There
are no actual stack arguments or incoming EDX data input. Descriptor DWORDs
+4,+0,+8 are read in that order with real saves/locals interleaved. Its raw
auxiliary descriptor supplies extra pointer+4, optional static-tree pointer+0,
base+8 and maximum+10h. No +Ch field is consumed. These are observed widths and
addresses, not recovered zlib class/layout/version/owner declarations.

Public chosen Source is noreturn cdecl(backing,descriptor,actualKernelEntryESP).
Its entire adapter MOV-loads EAX from[ESP+4], ECX from[ESP+8], ESP from[ESP+Ch],
then JMPs the genuine private Source kernel. Three real inputs, no fabricated
provider, frame, continuation, canonical register, local snapshot or repair.
Other incoming GPRs and flags remain unchanged by this prefix. The third input
selects an independently aliasable actual Source execution frame, without
forcing C++ transport words into its locals/control or inventing an Original
unused-register argument. It does not execute a captured Original frame.

Source-only noreturn never becomes Original no-return. The kernel's current
DWORD at entryS is the actual continuation; plain RET normally leavesESP=S+4.
No ordinary C++ returning API/nonvolatile restoration, real continuation
consumer, Original declaration or parent/caller adoption is promised. The
private Source-only no-stack-argument cdecl spelling does not establish its
custom EAX/ECX entry for an ordinary C++ call; only the actual adapter enters it.

## Actual frame, narrow stores and malformed states

SUB ESP20h reserves32B, then actual interleaved pushes save EBX atS-24h, EBP
atS-28h, ESI atS-2Ch and EDI atS-30h. After saves F=S-30h until common epilogue.

| Local at F | Reused actual contents |
|---|---|
| +10h | scan index, initialized573 on entered pass, later RMW decrement/reload |
| +14h | first-pass heap cursor, DWORD ADD4 per iteration |
| +18h | overflow0 then DWORD ADD1, later loaded into EBP |
| +1Ch | captured maximum, later remaining rebuilt leaf count |
| +20h | first-pass count, later correction count-cursor pointer |
| +24h | captured max_code |
| +28h | captured extra-base |
| +2Ch | extra pointer, then max-1, then saved rebuild heap cursor |

Eight separate DWORD zero writes at backing+B34h,B38h,...,B50h and twelve
distinct memory RMW instructions remain ordered. Initial descriptor captures,
saved CPU bits and root WORD-zero store precede the heap_max reload. First pass
reads actual heap-cursor cells and nested zero-extended WORD parent/length,
uses signed max/node/base comparisons and wraps DWORD arithmetic. Node/max_code
CMP precedes the WORD length store; the following JG preserves pre-store flags.
Only later iterations reload actual maximum; actual extra/local pointers are
conditionally reread. Optional captured static-tree pointer is tested before
its actual WORD read. Length cost additions retain low32 IMUL and DWORD RMWs.

Actual remaining/cursor locals control the loop, with no snapshots or progress
guard. Any nonzero overflow enters the correction do-loop, including negative
malformed bits; later subtract2/TEST/signedJG determines continuation. Backward
WORD search has no lower bound. ADDWORD FFFFh is retained as ADD, never DEC.
Captured count[max] pointer can differ from a later reloaded maximum under
aliases. Rebuild reads actual scan/count/cursor/max_code locals, conditionally
reloads heap cursor only after unequal-length opt_len RMW, and preserves
per-WORD updates, partial-register effects and actual nonleaf paths.

All pointer/index/bound/count/WORD patterns and descriptor/backing/tree/extra/
static/local/save/control aliases remain admitted. Invalid or mutable states
may fault or not terminate; no clamp, cap, null guard, topology assumption,
canonical data/global/owner or rollback was added. Actual POPs restore actual
cells. EAX backing survives, ECX/EDX are path-specific, and final arithmetic
flags come from ADD ESP20h after four POPs, not the prior loop TEST/CMP.
The full seven-phase contract and each RMW site are retained in the report.
No x87/SIMD/string instruction or FP environment/FIP normalization occurs.

## Complete actual artifacts and build

Fresh verified projectC:/Users/sqz269/bsp.gpr, program/battlestationspacific.exe,
base00400000 metadata/live/installed captures cover both529B ranges and retain
Original name/body/no_return=false. Old comments/prototype/xrefs are preserved;
no Source adapter prototype, fake fastcall or annotation was applied to Original.

First strict MSVC32 C++17 /MD /O2 /W4 /WX /fp:strict /Gy compile passed in
1.47054s. Normal warm scripts/build.ps1 passed once in18.426832s, terminal0;
existing reconstructed_math0.22s and tool_tests7.31s passed, total7.57s. The
pre-existing unrelated LNK4006 spawn duplicate warning remains in the log.
No compiler rejection, second compile/build, new test, probe or selected-body
execution occurred. Static effects are not dynamically measured fault/runtime
or game/ordinary-return/unwind/full Original ABI proof.

Entire strict1380B and actual registered1408B objects have five fully recorded
sections, two complete executable sections526B/17B, and both function symbols
at offset0. Debug metadata184B becomes212B; all code/data/metadata/symbol/aux/
string bytes are retained. Whole code has539 fixed bytes plus one relocated
four-byte operand at adapter+13, a true REL32 JMP to its defined private Source
kernel. No other relocation, undefined provider, extra helper, writable global
or absolute Native target exists.

Both complete Core82475-symbol dictionaries uniquely bind the public adapter
to the same entire1408B member at header20054778, equal to the actual registered
object. The private kernel is a real local symbol, not a fabricated public
export. All2399 prior complete object files and archive payloads remain equal;
only one new payload is added. One registered Source item/actual command and one
owned CMake append were verified. Default no new tests/probes was retained.

## Boundaries and cost

Existing stock Source trees.c490..567 and its genuine482B/144instruction private
section remain algorithm evidence only: cdecl/EBP and XORPS/XMM0 plus two16B
MOVUPS stores differ from Native. No stock helper is delegated or adopted.
Call-free kernel needs no external provider/global; raw descriptor/heap/table
data are genuine inputs. Named parent build_tree00BCF1C0 remains outside this
Native storage/continuation/caller closure. Original private class/version/
formal ABI/compiler flags, absolute placement/control/return/fault address,
portable C++ semantics, Native SEH/unwind/full ABI and gameplay are unproved.

Immutable V39/T2485 and their P/R references are retained by seal, without
transitive copy/rehash. Current2399 objects/Core/project/11TLOGs/LastTest and
CMake were frozen once before Source/compilation. V readiness1189.256575s and
dispatch/admin gap620.743425s are separate. X starts2026-10-16 10:16:12UTC and
includes all preflight/preservation/Source/mapping/build/docs/ledger/commit/
release/hash sealing; final endpoint is in the local handoff. No speedup claim;
Root independent review/integration/annotations/runtime remain separate work.
