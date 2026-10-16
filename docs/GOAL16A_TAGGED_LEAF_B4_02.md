# Tagged B4 leaf packet 02

This bounded packet closes 16 Source functions over 154 Original bytes in
`00B40000..00B4FFFF`. It does not close the whole band. Mechanical names,
backing-storage interfaces and measured results are Source hypotheses;
Original class and semantic C++ prototypes remain unknown.

| Original address | Complete Original bytes | Complete Source bytes | Operation and boundary |
| --- | ---: | ---: | --- |
| `00B401A0` | 11 | 11 | zero dwords +0/+4/+8, EAX=0; whole instruction identity |
| `00B401B0` | 11 | 11 | same measured operation; whole instruction identity |
| `00B403F0` | 15 | 9 | copy raw dword +4 and return destination; new Source ABI |
| `00B40750` | 42 | 38 | four ordered x87 transfers, source +390h..39Ch; new Source ABI |
| `00B40780` | 24 | 20 | three ordered x87 transfers, source +38h..40h; new Source ABI |
| `00B40F10` | 5 | 5 | ADD dword +4,-1; whole instruction identity |
| `00B44BF0` | 3 | 3 | RET4 only; physical unread cleanup word, Source void |
| `00B47660` | 4 | 4 | raw dword +4 read; whole instruction identity |
| `00B47670` | 11 | 11 | zero dwords +0/+4/+8, EAX=0; whole instruction identity |
| `00B47830` | 4 | 4 | raw dword +4 read; whole instruction identity |
| `00B47840` | 3 | 3 | raw dword +0 read; whole instruction identity |
| `00B47870` | 4 | 4 | raw dword +4 read; whole instruction identity |
| `00B47880` | 3 | 3 | raw dword +0 read; whole instruction identity |
| `00B478F0` | 4 | 4 | raw dword +30h read; whole instruction identity |
| `00B47EC0` | 5 | 5 | ADD dword +4,-1; whole instruction identity |
| `00B48040` | 5 | 5 | ADD dword +4,-1; whole instruction identity |

All rows have **complete** coverage. Full Original and full emitted Source
extents are stored separately. Thirteen complete Source bodies equal 73
Original bytes; three complete new interfaces cover the operations in 81
Original bytes. All Source executable sections total 140 bytes / 56
instructions, versus 154 Original bytes / 57 instructions. No prefix or
NativeSize clipping is accepted for the new interfaces.

The indexed sweep found 54 tagged entries without function records in the
worker branch. The selected 16 have no reconstruction records in either
the branch or Main's address index, no committed Main source/header address
matches, and zero indexed callees. The committed Main search pin is retained
in the report. These checks include whole function coverage rather than
treating a descriptive name as implemented Source. Fresh typed metadata
confirms one complete contiguous range per selected entry in project marker
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, x86 base
`00400000`, at modification 5326. Fresh complete live bytes through RET
equal the installed PE extents. No Ghidra writes or server changes occur.

No direct caller was found for the selected functions. `00B44BF0` has two
data references, which do not recover its indirect caller prototype. No
claim of unused code is made. Old `TRIV_body_...` symbols, documentation
and existing tags are preserved; naming/annotation is the integrator's
separate step. The report retains the actual xref responses.

The scan also records named dependencies without treating names as closure.
For example, unselected `00B42240` reaches the named EH vector destructor
iterator whose indexed status is `existing_host_substitution_labelled`.
Unselected `00B49450` / `00B49B40` reach a named ref-counted destructor with
`required_profile_provider` in its Source status, as well as the recorded
free binding. None is replaced by an unread helper or a new stub here.
Other destructor candidates remain outside this independent leaf packet.

Two absolute-data candidates remain outside the selected packet:
`00B400F0` reads `0108FE90`, and `00B478A0` reads `00D61CC0[ECX]`. Main has
real state-definition cell consumers and an actual NativeData type-size
table binding. Their existence is recorded; it is not evidence that these
two entry interfaces are wired to those production owners. No duplicate
cell, fabricated table owner, index guard, bounds assumption or global
provider is introduced. A later owner-binding packet can resolve that
integration explicitly. Other unselected candidates are unqualified here,
not asserted blocked or complete.

The exact zeroing forms put EAX=0 and XOR flags before the first of three
stores, preserving the measured store/fault prefix. The decrement forms
retain one ADD-memory RMW instruction: DEC would preserve different CF/AF
effects. No saturation, null guard or atomic/synchronization claim is added.
The six ordinary C++ getters use memcpy for raw unaligned-safe dword reads;
their entire actual object bodies match Original MOV/RET. RET4-only Source
exposes one explicit unread cleanup cell and no semantic return or receiver.
Its incidental EAX preservation is not declared a recovered return value.

`00B403F0` originally captures ECX destination into EAX, reads source from
entry ESP+4 into ECX, loads source+4 into EDX, stores destination+4, and
RET4. Source instead takes meaningful ECX destination / EDX source inputs.
The complete emitted Source is:

```asm
mov eax, [edx+4]
mov [ecx+4], eax
mov eax, ecx
ret
```

The pointed read precedes the pointed write and all 32 bits are transferred.
EAX is destination at normal return; ECX remains destination and EDX remains
source. Original instead returns with ECX=source and EDX=word. During a
Source destination-store fault EAX holds the word, while Original EAX holds
destination. Source also omits the Original argument-cell read and RET4
cleanup. Both MOV/RET streams leave EFLAGS unchanged. These are qualified
data-operation semantics over corresponding backed storage, not matching
Original ABI or complete machine fault frames.

The first strict compiler run succeeded, but the inspector initially expected
a different valid register schedule for that C++ copy. Its actual complete
nine-byte body was audited and the inspector corrected; Source and compiler
output were unchanged and not recompiled. That diagnostic, original
inspector, object, listing and logs are retained. The compile-only duration
was not persisted before the inspector failure and is left unknown rather
than estimated; the inclusive worker clock contains the whole attempt.

`00B40750` / `00B40780` originally capture destination once from entry
ESP+4 into EAX, use ECX backing storage for four/three FLD/FSTP pairs, then
RET4. Source instead captures meaningful EDX destination into EAX and RETs.
There is no dummy EDX, invented receiver, class, snapshot or environment
reset. The **entire** four/three-pair subsequence is byte-identical in each
strict object, including all opcodes, memory register operands and order.
EAX remains the destination address register and ECX the source address
register. The first FP instruction moves from Original offset +4 to Source
offset +2; absolute FIP, store instruction addresses and reported exception
instruction addresses therefore cannot be equal merely from this proof.

Every FLD is followed by its FSTP before the next source read. Overlap may
therefore change later reads, and the Source retains that actual data order.
These are x87 transfers through the inherited stack/status/control state,
not raw memcpy or a NaN-bit-preservation promise. Completed ordinary pairs
balance the stack; exceptional/full-stack behavior follows the actual
instructions. Conversion/rounding and FP status effects remain those of
FLD/FSTP; no CW/MXCSR reset, added wait or FP-mode runtime validation is
claimed. [AMD architecture manual, x87 data transfer](https://docs.amd.com/api/khub/documents/sfvvekC9mDflu6vd3R0NXA/content)
describes those architectural operations, not a game result from this packet.

The remapped functions have a deliberate argument-cell boundary. Original
stack source/destination cells can themselves alias pointed data; their
pointer is captured before the data operation. A new fastcall caller does
not automatically recreate that physical cell or its initial bytes. The
Source data-operation qualification requires corresponding captured input
pointers and addressed storage. It permits backed data overlap, but does
not reproduce Original argument-cell aliases, argument-load faults, their
ordering against pending FP exceptions, or RET4 cleanup. For the FP entries,
EDX is now destination rather than an unconsumed Original register value,
so complete fault-register frames also differ. No Original ABI adapter is
provided. All three new interfaces return destination in EAX and leave
EFLAGS unchanged; Original class/prototype and caller adoption stay unknown.

`CC13_SHADER_VTABLES.md` described both transfer entries as vector3 copies.
The complete measured `00B40750` body contains **four** loads/stores through
source +39Ch; `00B40780` contains three through +40h. This correction is
recorded here without editing that historical document or inferring classes.

One deferred target_sources line registers the new module in bsp_core.
Strict and actual registered MSVC Win32 qualification account for every
complete executable section, symbol offset zero, relocations and undefined
providers. The real Core archive must contain one byte-identical object
member with a unique provider for every public symbol. The required warm
`scripts/build.ps1` run performs compilation and existing checks. Gate
outcomes and full hashes are authoritative in the JSON report. No new
routine test, probe or broad test suite is introduced.

Prior Y and B4 published Source/docs remain untouched. Their frozen archive,
objects and provenance stay unchanged, with a separate preservation manifest
under `local/goal16a_tagged_b4/`. Current packet evidence, full compiler
objects/listings and archive are retained there in worker worktree
`J:/PROG/battlestations-pacific-decompile-goal15q_b4_x87`.
The local verifier is `local/goal16a_verify.py`. No game/deployment, Native
placement, Original ABI adoption or FP runtime claim follows from these
Source, build and static qualifications. Full elapsed time includes setup,
qualification, the inspector diagnostic, build, review, commit and handoff.

Final review found that the originally requested `native_tagged_leaf_b4_02`
module already belonged to GOAL15O. The superseded first attempt, its
21.400-second warm build, archive, objects and provenance are preserved in
`local/goal16a_tagged_b4/filename_collision_attempt01/`; its acceptance was
withdrawn. This packet now uses the genuinely new
`native_tagged_leaf_b4_16a.cpp/.hpp` module. The published GOAL15O Source,
header, doc and report match both immutable base
`b568bb23a887554c6e18848dc14fa628c4049709` and the checked Main pin, as well
as that report's Source/header hashes. The old registry is preserved exactly
apart from this packet's one new registration line. Its eight complete
functions are verified against the actual final registered object and one
byte-identical archive member, with eight unique symbol providers. Prior
ledger records are retained.

After that repair, fresh strict compilation passed in 1.711 seconds. The
required final warm build passed in 18.983 seconds; `reconstructed_math` and
`tool_tests` both passed (2/2). The final module has 16 complete executable
sections, 140 bytes and 56 instructions; the actual archive supplies each of
its 16 symbols uniquely. Every executable section and relocation is
accounted for, with no undefined externals. These are the final accepted
build artifacts. Both earlier diagnostics and both builds remain part of the
inclusive measured worker interval, beginning at 05:13:34 UTC. The
dispatch-to-first-capture interval is unmeasured; no complete Root+worker
wall-clock claim is made.
