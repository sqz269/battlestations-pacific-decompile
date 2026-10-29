# Independent Dyn radix update review

**Verdict: static source/object review passes; no blocking mismatch found.**
This reviews candidate `3ff41faa87bf10653d41c0e79e9685254e22d541` against
accepted `86d56331f`. It covers the complete in-house entry
`00C54DA0..00C55A55`, its adaptations and dependency interfaces. It supplies no
activation, original ABI, exception-transport, native differential or gameplay
proof, and no vendor-body or additional reconstruction credit.

The reviewer changed no candidate source, shared build configuration, ledgers,
Ghidra state, installation or processes. Only this document and
`reports/cc16_dyn_radix_update_review.json` are committed. Temporary extraction,
compile and static audit artifacts are ignored under
`local/cc16_dyn_radix_review/` in the review worktree.

## Independent evidence

The candidate's four committed files were extracted by exact `git show` bytes,
not copied from a potentially modified working file. Its source SHA-256 is
`2ac7341aaaee3499e595baabb71b26ac4ae0cfae063403c3eb52c3ccb28b5792` and header
SHA-256 is `595bd6ee8c3bad0782098bff4aa1d50ead0ec786e9eac834d3b06630719bdfc7`.
The author's local audit code/results were inspected as claims and comparison
material; they were not the basis of the independent pass.

Every live read used the verifying `bsp.py ghidra` client with configured project
`bsp`, `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, language
`x86:LE:32:default`, image base `00400000`. Seven new bounded byte queries
covered all 3254 bytes. Those bytes exactly match the installed PE read directly
by the reviewer, SHA-256
`900f716ab90729db83551f097e4f64bce810a45321670a9d6d8be4197f0b4f24`.
Installed image SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The refreshed Ghidra assembly manifest hash independently matches
`2c1f114d628c7d48523c8645e35ccd8344c4f196479ac7570331cf6999acd59c`.

The reviewer compiled the unedited committed source/header against the accepted
tree's dependencies with MSVC x86 `19.51.36244.0`,
`/c /std:c++17 /permissive- /EHsc /fp:strict /O2 /W4 /WX`.
Compilation passed. The independent object hash is
`89e5f5ea3d2b7cf74f1adf38b9b34533a89c243da774909f1d0f3b5cbb54a2ee`.
This is a new object compiled at a different path, not an assertion that its
COFF metadata/hash equals the author's object.

An independent PE/COFF/Capstone audit established:

- All 1001 unique Ghidra-listed native instruction addresses appear in the
  committed source once, in order, and decode at those addresses in current bytes.
- All 1001 instructions compare against 1011 emitted kernel instructions. The
  ten additions are exactly eight context pushes and two half-cell pointer loads.
  Other changes are the half load's memory addressing, twelve call relocations,
  internal branch displacements and the final `RET 4`.
- All 97 branch sites preserve instruction IDs and their 73 native targets map
  to the correct emitted targets. A full successor graph reaches all 1001 listed
  instructions. Seven omitted alignment gaps have unconditional-transfer
  predecessors; none is a normal fallthrough path.
- Every unadapted instruction preserves operand type, width, register, immediate,
  addressing scale and displacement. Six byte-encoding differences are shorter
  encodings of identity LEAs on EBX/ESP/ECX. They preserve flags and values. No
  unadapted x87/SSE instruction differs in bytes.
- The sole original absolute data operand is the double at `00D7A280`; no
  unbound original global operand or additional indirect call is hidden here.
- An independent `verify_report_calls.py` run reports **12 call rows checked,
  0 failed**. COFF relocation targets separately agree with all twelve sites.

## Context, stack and bridge review

The public function builds a 12-byte Context containing the addresses of the
same caller-owned CRT, feature word and half cell. The generated public wrapper
passes the actual manager in ECX, zero in unused incoming EDX, and Context* as
the private kernel's one stack argument. Context remains live until that
synchronous call returns; no bridge retains it or substitutes an owner.
Native ECX-manager/no-stack-argument/RET becomes private
fastcall ECX-manager/stack-Context*/RET4, then ordinary public C++ cdecl.
The original EBP/EBX/ESI/EDI save/restore sequence remains intact.

The added EDX loads before native `00C54E0B FLD` use Context+8 to obtain the
actual half-cell address. Original EDX is dead there and is overwritten by
`00C54E11 LEA EDX,[EDI+B0h]` before its next native use. MOV does not disturb
the integer flags. A fresh live byte read confirms `00D7A280` is
`00 00 00 00 00 00 E0 3F` (double 0.5); the source still requires its actual
address and provides no numeric/default replacement.

| Call family | Emitted bridge behavior | Cleanup |
| --- | --- | --- |
| Four `00BF79F0` sites | Shim copies `(context,destination,fill,count)` to cdecl bridge; bridge forwards `(destination,fill,count,feature-cell-address)` to the accepted qualified provider | Bridge caller removes 16 bytes; shim `RET4` removes context; original kernel `ADD ESP,0Ch` removes original three arguments |
| Three `00C40900` sites | Shim captures ECX manager and `(context,endpoint,axis)`; bridge calls the real accepted append interface with identical manager/endpoint/axis/CRT | Bridge caller removes 16 bytes; shim `RET0Ch` removes context plus original two arguments |
| One `00C40A30` site | Shim captures EDI manager and `(context,first,second)`; bridge calls the real accepted add interface with identical objects/CRT | Bridge caller removes 16 bytes; shim `RET0Ch` removes context plus original two arguments |

The repeated ESP-relative shim pushes and generated bridge pushes were decoded
and checked in their actual changing-stack order. All eight inserted context
pushes occur after the original argument setup and before the corresponding call.
Native frame operands remain EBP-relative, including stable Context at EBP+8.
The compiler emits only integer operations and the intended dependency call in
each bridge/public wrapper; it does not prematurely read the feature value or
add FP arithmetic/conversions. The memset bridge pushes Context+4's **address**.
The accepted `fill_native_crt_bytes_00bf79f0` reads that volatile word only at its
original low-fill-zero/count-at-least-256 gate. Its zero-count bypass and scalar/
vector qualification remain in the real provider. Canonical-address custody is
a caller requirement, not a runtime pointer-identity check.

Caller register liveness was checked beyond opcode matching. After accounting
for zero idioms and subsequent call inputs, no adapted memset/add/append call
needs EAX, ECX, EDX or integer flags after return. Direct inspection confirms:
memset returns precede `ADD ESP,0Ch; XOR ECX,ECX`; add returns reload loop state
and re-establish comparison flags; every append prepares ECX again, and the
last append re-establishes flags before its loop branch. Nonvolatile registers
are covered by the actual standard C++ callees/shims and kernel save/restore.

An x87 CFG stack-height audit gives maximum added depth six, consistent depths
at every join, depth zero at all twelve calls and at normal return. Thus bridges
do not have hidden live x87 temporaries to preserve. XMM0 is the only SSE data
register: initial count/zero values are fully consumed before calls, and later
endpoint-copy uses load their value before each store. No SSE value is live
across a call. Original mixed x87/MXCSR instruction and spill behavior is retained;
no CW/MXCSR resets or new FP operations were found in the bridge/public paths.

## Actual stack probes

The kernel calls the real compiler support symbols, not heap replacements:
one EAX=`1040h` fixed probe, two wrapping initial-count-times-eight aligned
allocations, and one wrapping captured-axis-zero-count-times-eight aligned
allocation. It uses returned ESP directly. All remain allocated until the
original EBP-based epilogue; its saved-register location is EBP-104Ch.
The histogram occupies EBP-1040h through EBP-41h, excluding the live local
slots above it and saved registers below it. Context at EBP+8 is separate.

The review inspected actual current MSVC `14.51.36231/lib/x86` archive members
in `libcmt.lib`, `libcmtd.lib`, `msvcrt.lib` and `msvcrtd.lib`:

- `__alloca_probe` matches all 43 native `00BF9410` bytes exactly in each
  archive. `__chkstk` aliases the same definition. It restores ECX, leaves EDX
  and FP state untouched, probes pages and changes ESP as required.
- The first 22-byte `__alloca_probe_16` entry matches native `00C0F700` before
  its relocated tail jump. Its rel32 target is that `__chkstk` definition.
  The alignment arithmetic, unsigned wrapping/carry handling, ECX restoration
  and sixteen-byte resulting ESP therefore match. Following section bytes
  belong to the separate eight-byte-alignment entry and were not mistaken for
  part of this helper.

The first fixed probe is the one call whose caller needs a volatile register
preserved: ECX still holds manager for `00C54DB0 MOV EDI,ECX`. The independently
inspected runtime definition provides that preservation. The other probes have
no live volatile GPR/flags afterward. No compiler/vendor body earns credit.

## Limits before activation

The static result is sufficient to continue source integration. Root still
needs source registration, final target link/build and symbol/object inspection,
then genuine manager, stable CRT function table, stable canonical feature cell,
and actual half-cell lifetime/custody at the real caller. Archive evidence does
not itself prove final link resolution, another compiler configuration, or any
runtime binding. Accepted add/append/qualified memset dependencies were checked
for interface/forwarding closure here, not independently re-proved in full.

Native count/ring/tag/buffer invariants, writable nonwrapping extents disjoint
from live frames, actual CPU/OS support and required DF state remain prerequisites.
No new guard or repair is supplied. Original stack addresses, call/fault PCs,
unwind/FH/SEH identity and exception transport differ through this new interface;
an original ABI adapter is still absent. RDTSC returns observed low bits only,
not an equivalent wall-clock/timing value. Provider allocation-failure behavior,
exception paths, native differential and game behavior were not exercised.

No executable was linked or run, no game or probe launched, and no tests were
added or run. The reviewer performed one optimized object-only compilation and
read-only static verification. The candidate remains unwired at this checkpoint.
