# GOAL16T: pqdownheap body and borrowed-frame entry

READY: this implements the accepted GOAL16R design for the complete Original
`00BCDE90..00BCDF5B` body, 204 bytes / 59 instructions. The new Source module
contains that private register-body kernel plus a genuine 17-byte / four
instruction entry adapter. Its entire executable extent is 221 bytes / 63
instructions. There are two Source functions and one new Native address;
the adapter receives zero Native function/address credit.

The current provisional library-derived name `pqdownheap__prov` is retained.
The fetched stock zlib `_pqdownheap` counterpart is a complete 258-byte cdecl
body with different argument/cache/index/stack schedules. It is algorithm
evidence, not this physical provider or proof of Original layout/version,
compiler flags, class or formal C++ ABI.

## Real entry and frame

The private kernel consumes EAX raw backing, EDI raw tree, and the actual DWORD
index at entry ESP+4. It has three RET4 exits. Its Source-only stdcall(int32)
spelling describes only that stack-word/cleanup shape; ordinary C++ cannot
establish its full custom register entry and must not call it directly.

The public Source declaration is a noreturn cdecl transfer with three real
inputs: backing, tree and the actual kernel-entry stack address. Its entire
body is MOV EAX,[ESP+4]; MOV EDI,[ESP+8]; MOV ESP,[ESP+Ch]; JMP the genuine
private Source kernel. There is no absolute Native target or fabricated
provider. It supplies a real Source execution frame, without copying,
constructing, initializing, checking or repairing index/scratch/control cells.

At chosen kernel entry ESP=S, current DWORD[S] is the actual continuation and
DWORD[S+4] the mutable signed-index argument. Below it lie scratch S-4, saved
EBP S-8, saved ESI S-12, and saved EBX S-16 only when the loop path is entered.
These are actual cells, not a new C++ object layout or shadow frame. Only
actually reached accesses need backing to complete normally. All pointer,
index, bound, WORD frequency and BYTE depth patterns remain admitted.

The Source-only noreturn contract means no ordinary returning cdecl API or
nonvolatile restoration is promised by the adapter. It does not make the
Original/private kernel no-return. A selected actual RET4 reads the current
continuation, leaves ESP=S+8 on normal completion, and POP ECX reads the current
scratch word. The chosen Source frame is not a claim to execute a captured
Original frame. No normal-return trampoline, callback, global or owner is
invented; future consumer continuation/caller adoption needs separate evidence.

## Ordered kernel effects

1. PUSH ECX precedes reading DWORD[backing+1448h] heap_len. PUSH EBP/ESI precede
   reading actual index S+4 and capturing heap[index] at backing+B54h+4*index.
   LEA doubles index modulo32, signed CMP compares it with the captured bound,
   and the cached heap value overwrites scratch S-4 before signed JG. The
   incoming pushed ECX bits can affect the earlier bound through an alias;
   no canonical ECX or saved-register value is substituted.
2. Initial signed JG writes the cached value to heap[index] before all three
   POPs and RET4. Otherwise PUSH EBX, then signed JGE uses the surviving bound
   CMP flags. Equality skips right-child reads and skips the scratch reload;
   current EBP remains in use on that path.
3. With two children, right heap value/frequency is read before left. Frequencies
   are unsigned WORDs at tree+4*node. Smaller right frequency selects right;
   equal frequencies read right BYTE depth before left at backing+1450h+node.
   Right depth <= left also selects right. ADD child,1 preserves the observed
   arithmetic/fault prefix. Only this two-child route reloads actual scratch
   into EBP after selection. It then reads the chosen heap value.
4. Root unsigned WORD frequency precedes child frequency. Smaller root frequency
   exits via the ECX-index epilogue. Equal frequencies read root depth before
   child; root depth <= child exits via the EDX-index epilogue. No gathered
   table read, type widening, cached depth, bound guard or class is introduced.
5. Promotion reads current actual S+4, writes the selected child into heap at
   that index, reloads heap_len, and only then writes child index into actual
   S+4. ADD doubles child modulo32; signed CMP/JLE governs the next iteration.
   Heap/bound/index/scratch/saved/control aliases retain that exact schedule.
6. Loop epilogues reread actual index into ECX or EDX, POP EBX/ESI, write the
   current cached EBP to heap[index], then POP EBP/ECX and RET4. Thus their final
   heap store can affect the later EBP/scratch/continuation reads, whereas the
   initial fast-exit store precedes all POPs. No transactional rollback occurs.

EAX backing and EDI tree are not written by the kernel. Saved registers restore
actual cells, not unconditional entry values under aliases. Outgoing ECX is
the current scratch word; EDX and final CMP flags are path dependent. Incoming
CPU state and all individually reached PUSH/POP/data/RET fault prefixes remain
observable. Malformed/wrapped indices or mutable aliases need not terminate or
complete normally; no cap, progress guard, snapshot or domain clamp was added.
The one three-byte 8D0C36 LEA is explicitly emitted with its named instruction
comment. All 59 kernel instructions match all 204 current Native bytes.

## Complete build and artifact proof

Fresh read-only typed/live/installed evidence verifies C:/Users/sqz269/bsp.gpr,
/battlestationspacific.exe, base00400000, complete204B range, no thunk and
Original no_return=false. Neither Original prototypes/comments nor Ghidra were
written. No fake fastcall or Source-adapter noreturn is applied to Original.

The first strict MSVC32 C++17 /MD /O2 /W4 /WX /fp:strict /Gy compile passed in
1.4479368s. Required normal warm scripts/build.ps1 passed once in18.9183319s;
existing reconstructed_math0.21s and tool_tests7.69s passed (total7.94s).
The existing unrelated LNK4006 duplicate-spawn warning is retained in the log.
No Source/compiler rejection, second compile/build, new test, probe, selected
body execution or measured fault result is claimed.

Strict whole COFF1059B and actual registered1087B each contain two complete
executable sections204B/17B and five sections total. Public/private function
offsets are0. Debug metadata184B becomes212B; all raw sections, symbols/aux,
strings and relocations are preserved. There are217 fixed executable bytes
and one four-byte REL32 operand at adapter+13, binding its real JMP to the
defined private Source kernel. There are no other relocations, undefined
providers, extra helpers, writable globals, or absolute Native jumps.

Both complete Core82474-symbol linker dictionaries uniquely bind the public
adapter to the same entire1087B actual-object member at header20054626. The
private kernel is a real defined local function, not an invented dictionary
export. All2398 preceding registered objects and complete archive payloads
remain byte-identical. One registered Source item/actual C++17 command and one
owned CMake append were verified. See reports/goal16r_zlib_raw_pqdownheap.json.

## Evidence limits and time

The adapter adds three actual Source argument reads and a stack/control transfer
before kernel entry. They can fault and change EAX/EDI/ESP in their stated
order; other GPRs and flags survive the prefix. They are not Original prefixes.
Whole kernel identity does not equate public Source/Original call frames,
absolute code/data aliases, instruction/fault/return addresses, ordinary C++
return/portable language semantics, Native SEH/unwind/full ABI, parent adoption
or gameplay. No x87/SIMD instruction normalizes inherited FP state/FIP.

N/L/R seals remain immutable references; their transitive review trees were not
recopied. Before Source/build, current2398 objects/Core/project/11 TLOGs and
LastTest were frozen once. P readiness943.721305s, subsequent administrative
gap, accepted R design782.503525s, dispatch gap and this implementation from
2026-10-16 09:31:11 UTC are recorded separately; no speedup claim. Final commit,
release/hash sealing and complete author endpoint are in the local handoff.
Root independent review, Ghidra annotations, actual new baseline integration
and any future execution remain separate costs and proofs.
