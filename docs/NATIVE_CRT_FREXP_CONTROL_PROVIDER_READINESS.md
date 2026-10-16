# Native CRT raw frexp control provider: readiness only

This independently reviews only `__ctrlfp`, `00C138A7..00C138CD`: 39 bytes,
16 Ghidra instructions, one complete body, returning and not a thunk.

The selected body contains zero call or jump instructions.

Source readiness and authorization remain false; Source and Native credit
remain zero. The exact base is `2f47be683a8bba8b133c03e7f6d01a28796cd411`.
No C++, API, compiler, test, probe, emulation, body execution, SDK comparison,
Ghidra mutation, listing repair, Main change or push is authorized by this review.

Fresh evidence is in `local/NativeCrtFrexpControlProviderReadinessSol01/working/`,
with the final immutable copy in that family's `evidence/`. The companion is
`reports/native_crt_frexp_control_provider_readiness.json`.

## Exact evidence and instruction boundary

The original PE was physically copied before analysis, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Selected body SHA-256 is
`778473908a8335c4091f62e6b0dfdb7cc2e6cc322e07237a64edf075d2adf26b`.
All 39 disk/live bytes agree. Every one of the 16 complete InstructionDB rows
agrees with disk/listing starts, lengths and mnemonics after the explicit waiting
bundle described below. All default/effective flow, targets and fallthroughs
agree; overrides are NONE. No gap, neighboring instruction or other body was
selected. Whole-row capture modification is 6981.

At `00C138AB`, Ghidra represents `9B D9 7D FC` as one four-byte `FSTCW`.
Capstone independently decodes a one-byte WAIT followed by three-byte FNSTCW,
producing 17 raw encoding units for the same 39 bytes. The static checker retains
both raw rows and the explicit single waiting bundle; it does not repair a
listing or claim 17 Ghidra instructions. The WAIT byte is material evidence.

Every live analysis/checker batch verifies existing project `bsp`, marker
`C:/Users/sqz269/bsp.gpr` and program `/battlestationspacific.exe`, with
`BSP_GHIDRA_AUTOSTART=0`. The metadata-only existing report/doc checker is actually
run, including its literal zero-call counts; its retained output is authoritative.

## Original physical ABI and memory order

At entry ESP=S, return address is `[S]`, new value N is DWORD `[S+4]`, and mask M
is DWORD `[S+8]`. After `PUSH EBP; MOV EBP,ESP`, frame F=S-4 has N at F+8 and M at
F+0xC. `PUSH ECX` allocates/initializes four local bytes at F-4. Maximum ordinary
machine-stack displacement is eight bytes below entry ESP. `LEAVE; RET` restores
EBP and returns with ESP=S+4; the caller removes the eight argument bytes.

The waiting FSTCW stores **two bytes** at F-4. Local high two bytes still contain
incoming ECX bits 16..31. Let O be the stored old control word, C the incoming
ECX, and L=`(C & 0xffff0000) | O`. The subsequent `AND EAX,DWORD[F-4]` really reads
all four local bytes. They are initialized bytes, not invented uninitialized
state. This incoming-register dependency survives in temporary upper bits and
integer flags even though it cannot change the eventual control operand.

Memory order is: push incoming ECX; waiting old-word store; first four-byte mask
read M1; four-byte N read; second four-byte mask read M2; four-byte local read;
four-byte overwrite of the private mask argument; two-byte FLDCW read from that
overwritten argument; final two-byte old-word read. The two mask reads are
retained separately. With arbitrary intervening memory changes, the exact
temporary is `T=(L & ~M1) | (N & M2)`, with 32-bit operations. With an unchanged
mask, ordinary low-word algebra is
`W=(O & ~uint16(M)) | (uint16(N) & uint16(M))`.

`MOVZX EAX,AX` truncates T and zero-extends W. The following DWORD store to
`[F+0xC]` replaces all four bytes of the provider's own mask argument with W,
whose upper two bytes are zero. FLDCW reads just its low two bytes. There is no
out-pointer, global write, mapped CRT-mask conversion, mask-zero shortcut, or
special-value dispatch. Even mask zero follows both x87 instructions and writes
the argument slot. Ordinary full-low-word mask `0xffff` writes `uint16(N)`.

The final `MOVSX EAX,WORD[F-4]` returns **sign-extended old O**, never the new word
or an unsigned abstraction. ECX returns `N & M2` across all 32 bits. EDX, EBX,
ESI and EDI are untouched; EBP is saved/restored. Final integer condition flags
come from the full 32-bit OR that formed T: CF=OF=0, SF/ZF/PF from T, AF undefined.
MOVZX, the memory store, FLDCW, MOVSX and epilogue do not replace those integer
flags. Thus final flags need not describe either returned EAX or truncated W.

No x87 data register is loaded, popped, rounded or converted. There is no new
ST(0) result and no incoming floating value is consumed. Ordinary x87 depth is
unchanged, including an already full data stack. Invalid stack memory, disabled
FPU access, pending exceptions and handler intervention can prevent completion;
none is removed from the contract to force Source readiness.

## Raw hardware control and caller context

The pinned [Intel SDM Volume 1](https://cdrdv2-public.intel.com/843827/253665-sdm-vol-1-dec-24.pdf)
defines the hardware fields (sections 8.1.5–8.1.6, printed pages 8-7..8-8).
For the accepted caller's N=`0x133f`, M=`0xffff`, the **memory operand** is exactly
`0x133f`: all six exception-mask bits are one, PC=3 requests a 64-bit significand,
and RC=0 requests nearest rounding. Bit 12 is legacy infinity control, ignored
by modern IA-32 x87. Reserved-bit readback is not proved by the integer operand.
No `_controlfp`, `_MCW_*`, SSE/MXCSR or host-library mapping is substituted.

WAIT checks preexisting unmasked exceptions before the old-word store; a
request to mask everything cannot bypass that check. The body does not clear
sticky exceptions or restore status/tags. Control-instruction condition codes
are architecturally undefined, so whole status-word preservation is not claimed.
Changed masks can expose existing flags to later waiting instructions. Fault
delivery and handler effects are unobserved. See the same pinned manual's
sections 8.1.3.2, 8.3.11 and 8.6 (printed pages 8-6, 8-23..8-24, 8-31 onward).
These are conditional architectural inferences, not target execution proof.

Only the accepted, newly pinned 156-file caller family supplies caller context.
Its first control invocation precedes its original-input FLD; EAX is saved as a
private token after that FLD. The later finite-route invocation receives that
token with full-low-word mask after spilling the result to binary64; a result
FLD follows. On ordinary completion this provider reapplies the token's low 16
raw control bits, but it does not restore earlier status/tags or ensure that the
following FLD completes. Exceptional/nonreturning routes and other providers
remain separate gates. Its private mask argument and local saved word are
distinct from the outer caller's private token home and the finite/setter frames.

The original caller has eight recorded invocation sites for **seven distinct
outer providers**; its nested finite setter is not an eighth outer provider.
Existing `native_crt_x87_control2` selects different wrapper entries and is not
this body. Existing `legacy_crt_math` fragments are read-only references, not
new Source/ABI proof. No caller or other provider body was newly queried/exported.

## Fresh phase, external preservation and gate

This task reuses actual Sol/max session `01a144c9-07cf-75c0-83b0-559317761802`
and its original canonical finite-worker agent path. It has a distinct control
worktree/lease owner; no new UUID or renamed session is invented. Actual metadata
independently reports `gpt-6.1-sol`, effort `max`. The named brief was physically
pinned before its first content read, SHA-256
`dd3a3dc9026fba2abc64e9b05051b1b5b9a54db98f64944d60d26cba7297f92c`.

Before the first new CLI invocation, 1,674 current runtime/startup files were
independently copied/rehashed (zero drift from external prior hashes), and Main
and exact-base tools were copied before their respective uses. Literal `-S`,
explicit user-module path, disabled bytecode writes and an empty redirected cache
apply from this phase's bootstrap. All 139 actual decoder module origins and its
Capstone library were checked against those pins before selected decoding. This
is an execution-source/module claim, not OS-wide or hermetic closure.

All prior 4,956 family files were copied byte-identically as external evidence:
the old 2,721-file sealed inner family and separately labeled 2,235 outer files,
including the invalid readback, explicit invalidation and corrected readback.
Neither old files nor their provenance labels are changed or portrayed as fresh
control evidence. The separately sealed caller family and old working outputs,
primary acceptance DATA and routing snapshot are physically pinned before use.

The final immutable manifest is `evidence/manifest.json`, with only that manifest
excluded from its member list; its receipt stays outside the immutable family.
Task-only actual raw tool invocation/result pairs start at this **new phase**;
execution and coordination are distinguished. The full actual original-session
prefix has exact byte/line bounds/hash but is not copied. Sealing-in-flight and
any later post-seal checks are qualified outside that prefix; no completion or
receipt chain is invented. The two owned outputs are committed before sealing.

This contract awaits primary review. Source/API authorization is false and both
credit counters are zero; overall frexp Source remains blocked by its original
provider set. Normal ownership is retained until explicit queued handoff;
both old and new local preservation leases remain separate.
