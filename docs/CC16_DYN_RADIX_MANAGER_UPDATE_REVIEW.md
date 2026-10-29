# Independent Dyn radix manager update review

No blocker was found for **normal-path source admission** of candidate
`2947ca96010887ce48c5f5e7ef42d006e8730822`. This conclusion covers the complete
native instruction schedule and qualified provider composition. It does not
establish exception-complete behavior, original ABI, activation or runtime
equivalence. This review changes only this document and its JSON report.

The candidate header/source match private admission `541c8b471` byte for byte.
The original entry is `00C5A8B0..00C5B175`: 2246 bytes, 654 instructions,
ECX actual raw manager, no stack arguments, plain RET. EDX is not an input.
The public C++ void interface and private fastcall kernel with one context
stack argument/RET4 are explicitly new interfaces. Names remain hypotheses.

## Independent evidence

All 654 native rows and all candidate source lines were read. Fresh bounded
target-verifying reads used `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; the resulting 2246 bytes match installed PE
SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Body SHA256 is
`c9fa16f24060e7f21e482b8b32ac02e5ad61c412ff665fbd0e784201cd998b96`.

Fresh MSVC Win32 object compilation against actual source-integration headers
passed `/std:c++17 /permissive- /EHsc /W4 /WX /fp:strict /O2 /Ob2 /MD`.
The retained static audit scripts were inspected and reused locally, with only
their root path adjusted. Fresh COFF/CFG inspection verified 654 address rows
against 704 emitted kernel instructions, all 52 branches, eleven dependency
relocations, 25 explicit adaptations and zero unexplained encoding differences.
All listed instructions are reachable; the three omitted native alignment
gaps total 16 bytes and have no incoming edge. All fifteen original normal-call
and EH-tail rows pass the live call verifier.

## Normal body, frame and floating point

The source preserves signed count loops versus unsigned growth/copy tests,
32-bit wrap, eight-byte key/endpoint and pair entries, the four stable radix
passes, low16 endpoint indices, selected-axis sweep and six AABB rejection
comparisons. Capacity commits precede allocation, live size/data reloads remain
inside copies, old data is freed before replacement publication, and computed
null destinations retain their original suppression. No rollback or container
substitution is added.

The original aligned frame and genuine `10A0h` stack probe remain. For S after
the four register saves, the histogram is `S+B0..S+10AF`; borrowed context is
`S+10B0` and state is `S+10BC`. The footer owner reload after POP EDI correctly
uses `ESP+10AC`. PUSH ESP supplies the argument-stack address before the added
word. New/free recover S as argument+4; append as argument+8; memset as
argument+12. Shims consume only that extra word, plus append's original RET8;
all original caller ADDs remain. In particular, C5AF99 still stores the pair
allocation at original `ESP+18 = S+14` before caller ADD4. CFG stack joins and
these epochs were independently checked.

The real MSVC `__alloca_probe` is not a stub: all four inspected CRT archives
contain the exact 43 original BF9410 bytes, with `__chkstk` aliasing the same
definition and no relocation. Its code restores ECX, leaves EDX/nonvolatiles/FP
untouched and actually probes/changes the stack. No vendor body is added.

All 101 x87 instructions and 27 FSTP sites match, with maximum added depth four
and zero depth at every call/return under the valid ABI entry contract. The
first three keys use FLD/FSTP; the fourth and tails use MOVSS. C5ADB7 retains
`DC C9` (ST1 destination), C5ADC3 retains `D8 C9` (ST0 destination), both self
multiplies and every unspilled product/addition retain their operand direction.
The half pointer load uses only MOV, preserving the signed CMP flags through
C5AE23. Float spill epochs, FCOMIP/FSTP/JA rejection and JBE axis selection
retain ties/unordered behavior. No C++ double arithmetic, forced FP controls
or native unspilled-value conversion is introduced. Wrapper MOVUPS operations
copy pointer bits only.

The interface requires the actual D7A280 double cell, whose observed bytes
encode 0.5, rather than supplying a constant. It also requires the actual
0109EEA4 cell reference, publication cell 0109E9F8, initialized profile/nodes,
CRT/heap custody and five genuine image name pointers. Fourteen owner reloads
remain at their native epochs. Feature value is read by the existing real
memset provider at its native gate, not captured by the bridge. Names and cell
addresses are borrowed through execution/cleanup. The existing SSE2, DF=0,
extent, lifetime and nonconcurrent profile-provider contracts remain required.

## EH actions and explicit limits

Live FuncInfo E04498 has maxState4 and unwind map E04478:

| State / predecessor | Native action tail to C38040 | Scope relative to S |
| --- | --- | --- |
| 0 / -1 | CC84DA, JMP at CC84E0 | Sweep +80 |
| 1 / 0 | CC84E5, JMP at CC84EB | Object +68 |
| 2 / 1 | CC84F0, JMP at CC84F6 | PossiblePair +50 |
| 3 / 2 | CC84FB, JMP at CC8501 | Pair +98 |

The bridge helper demotes state before each real C38040 action and retains the
original scope storage while doing so. It reuses actual RDTSC/modulo tick/count
updates and pops the actual current profile node through its parent. Sort is
state -1 and gains no fabricated cleanup. Cold-entry failure cleans only
already-active scopes; pair allocation failure composes Pair, PossiblePair,
Object and Sweep. No manager/allocation rollback is invented.

Compiler try/catch and rethrow are emitted, but this is **static cleanup-action
composition**. The kernel/shims have no registered native FH record; ordinary
C++ rethrow transport and caller stack/nonvolatile restoration across those
naked frames have not been executed or established here. Original FH3/SEH,
fault/unmasked-FP/stack-overflow transport, secondary cleanup exceptions and
physical timing/frame identity remain unproved. The source's explicit normal
and static-EH qualifications are necessary for admission.

## Dependency update after cleanup integration

The pre-cleanup receipt at `c30ce9b7d` proves all eleven dependency files equal
to candidate base `5c2547156` and pinned admission `541c8b471`. Root then merged
the additive cleanup repair. The late checkout receipt with HEAD `4cc539c7b`
records four changed
files: both Dyn profile scopes and Dyn engine runtime headers/sources. The other
seven dependencies remain identical. This review does not describe the final
eleven-file set as unchanged. File hashes pin the observed checkout inputs,
including the cleanup integration, rather than assuming HEAD alone names them.

A second strict compile against the new headers passed. All eleven emitted
kernel/wrapper/shim/bridge/helper functions have identical instructions,
padding and relocations before and after that header change. Crucially,
append_bridge still references the old **four-argument** C50390 symbol. That
path uses the old three-argument C44000 constructor. ScopeContext layout is
unchanged; the C5 context has no new genuine flat BF65AC binding. Availability
of the repaired overloads is not adoption: inherited partial-construction
failure debt remains until a separately reviewed real binding/call change.

No source, shared registration, ledger, Ghidra state or runtime input was
changed. No test, probe or game was launched. Root owns combined build/link,
annotations, caller/provider adoption and any activation. Review body/vendor/
ABI/gameplay credit is zero; candidate normal-body credit is not counted twice.
