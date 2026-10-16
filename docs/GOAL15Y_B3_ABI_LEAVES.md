# B3 ABI leaves

Addresses: `00B3FFA0`, `00B3D5E0`. Descriptive names and Source interfaces are
hypotheses; no original class or semantic C++ prototype is recovered.

| Address | Coverage | Measured physical binding |
| --- | --- | --- |
| `00B3FFA0` | complete, `00B3FFA0..00B3FFA3`, 4 bytes / 2 instructions | ECX backing input; `FLD m32real [ECX+18h]` pushes ST0; plain RET |
| `00B3D5E0` | complete, `00B3D5E0..00B3D634`, 85 bytes / 21 instructions | entry ESP+4 destination, entry ESP+10h source; +8/+Ch not directly read; local 10h scratch; `RET 10h` |

Fresh read-only typed metadata accepts one complete contiguous range per
body, with unchanged modification number 4886, project marker
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, x86 language
and image base `00400000`. Fresh live whole bytes through RET equal the
installed PE extents. Full PE hash, exact offsets, listings, prototype and
typed responses are retained in the report and worker local evidence.

Initial lookup had no Source reconstruction record for either body. The
prior B3 packet explicitly deferred both. Existing Ghidra names are
`TRIV_body_00b3ffa0` and `TRIV_body_00b3d5e0`, both with undefined return and
no parameters; saved documentation has no comments. Existing tags remain.
Neither xref query found references. The SSE callers endpoint reports
`function:null`, which is inconclusive. These results do not prove unused
code or establish an original argument list.

The warm worktree first preserved the earlier B4 Source/header/doc/report,
registered object, 93,507,096-byte Core archive, VCXPROJ, actual CL command
log, strict object/listing and compile/build provenance. All 16 copies have
matching recorded hashes. Only then was pinned Main
`d1c74e2ecc0649bb769496aa8f0c237db1e0a693` merged, once, by fast-forward.
The prior preservation record stays under this packet's ignored evidence.

The x87 Source API is `float __fastcall(const void*)`: the actual input is
ECX backing storage, with a floating result in ST0. No EDX input, fake
argument, frame or return-type workaround is introduced. Naked assembly
spells FLD and RET directly. As in the prior x87 packet, the single load
uses inherited x87 masks/status/control and may encounter stack, signaling
NaN, denormal, pending-exception or memory faults. Normal execution pushes
ST0 and retains the previous stack values below it. No extra load, store,
wait, status clear or environment reset is added. Undefined x87 condition
flags are not asserted preserved. Absolute saved FIP and exception
instruction addresses still depend on code placement. This is instruction
semantics, not a runtime FP-mode result. See [Intel SDM, FLD](https://cdrdv2-public.intel.com/774492/325383-sdm-vol-2abcd.pdf).

The SSE Source API is a **physical four-word stdcall envelope**:
`void* (void* destination, uint32 unread08, uint32 unread0c, const void* source)`.
Only destination and source are assigned pointed-storage meaning, directly
from the instructions that use them. The two middle words are explicit
caller-supplied cells for the measured cleanup envelope, with no defaults.
They are not recovered semantic parameters. No incoming ECX/EDX receiver is
assumed and no adapter moves inputs or snapshots storage. Pointers may
address any backed storage, including scratch or envelope cells; the
middle cells are not directly read as arguments, though explicit pointed
aliases may read or write their memory.

Let S be entry ESP, L=S-10h, P the actual source captured from [S+10h], and
D the actual destination captured from [S+4]. These are physical addresses,
with the instruction's native wrap, segment, alignment and fault rules.
The complete access order is:

| Step | Actual instruction effect |
| --- | --- |
| 1 | subtract 10h from ESP; load P into EAX |
| 2 | MOVSS read [P], then MOVSS write [L] |
| 3 | MOVSS read [P+4]; ECX=[L]; MOVSS write [L+4] |
| 4 | MOVSS read [P+8]; EDX=[L+4]; MOVSS write [L+8] |
| 5 | MOVSS read [P+Ch]; EAX=D from [S+4] |
| 6 | [D]=ECX; then ECX=[L+8] |
| 7 | [D+4]=EDX; then MOVSS write [L+Ch]; then EDX=[L+Ch] |
| 8 | [D+8]=ECX; [D+Ch]=EDX; add 10h to ESP; RET 10h |

The pointer P is captured once. Its four reads can observe earlier scratch
writes under source aliases. The later scratch reads can observe earlier
destination writes. The last scratch store can overwrite an earlier
destination store. This cannot be reduced to an initial four-word snapshot
or a conventional typed copy while preserving every backed alias and fault
prefix. No guard, alignment narrowing, wider access, validation read or
scratch relocation is added. If the destination aliases the return cell,
RET reads its actual resulting contents.

At a normal return, EAX is captured D. ECX is the value read from L+8 after
the first destination store; EDX is read from L+Ch after the last scratch
store. XMM0 low32 is the last source load, with high96 cleared by the legacy
memory-load form; other XMM registers are untouched. MOVSS is a move with
no SIMD floating-point exception, not arithmetic or a conversion. The
memory/CPU exception paths remain those of the measured instructions.
[Intel SDM, MOVSS](https://cdrdv2-public.intel.com/825757/253667-sdm-vol-2b.pdf)
defines those architectural effects. No x87 instruction or MXCSR/control
change is present. Final ADD ESP,10h sets arithmetic EFLAGS; these are not
claimed unchanged. Other general registers are untouched except the
described EAX/ECX/EDX and normal ESP/EIP return effects. RET consumes the
actual return cell and sixteen envelope bytes.

One focused Source fixture addresses the concrete destination/scratch
alias risk: D=L+8, source word0 equal to the actual return label, and
distinct remaining raw words. That return label keeps the overlapping
return-cell store valid. The measured access order yields destination
`[word0, word3, word0, word3]`, whereas a snapshot yields the original four
words. The fixture links the actual registered Core archive, observes this
trace, stack cleanup, EAX/ECX/EDX and XMM0. It does not execute the Original
entry address, test fault paths or floating exception modes, or establish
original caller adoption. Its outcome is recorded in the report.

The fixture passed this one alias trace, cleanup and register observation.
Its initial `/O2 /W4 /WX` compile was rejected by MSVC C4740 because taking
the internal return-label address suppresses global optimization in the
mixed inline-assembly caller. That failed fixture source/command/log is
retained. The same fixture passed with `/Od /Oy- /W4 /WX`; the linked Source
body remained the already verified Release object from the Core archive.

Strict qualification uses actual MSVC Win32 `/MD /O2 /W4 /WX /fp:strict /Gy`.
The public symbols must start at zero in complete executable sections of
4 and 85 bytes, with no padding, extra executable section or executable
relocation. The actual registered object is checked independently against
the full fresh Native extents and strict sections. The actual Core archive
must contain one byte-identical member and one linker-directory provider
for each public symbol. No invented global, type, helper or provider is
used to close either body. One deferred `target_sources(bsp_core ...)`
line registers the module. One required warm `scripts/build.ps1` run
performs Release/Win32 compilation and the existing checks.

Both complete strict and registered gates passed: 89 executable bytes in
two sections, both public symbols at offset zero, no executable relocations
and no undefined external symbols. Strict compilation took 3.483 seconds.
The one warm build and existing checks took 41.580 seconds; both
`reconstructed_math` and `tool_tests` passed (2/2). The archive member equals
the actual registered object, with one directory provider per symbol.

The worker checklist is applied: complete extents, no host calls or callees,
physical layout only, and no runtime/game claim from static bytes. Original
semantic prototype, class, code placement, real caller adoption and game
behavior remain unresolved. There are no Ghidra writes, game changes,
deployment or Native adoption in this packet.

Authoritative hashes, gate outcomes and timings are in
`reports/goal15y_b3_abi_leaves.json`. Ignored evidence and the one fixture
are retained under `local/goal15y_b3_abi_leaves/` in worker worktree
`J:/PROG/battlestations-pacific-decompile-goal15q_b4_x87`; the local verifier
is `local/goal15y_verify.py`. Full elapsed time through commit, lease release
and final review handoff is supplied separately from compiler/build times.
