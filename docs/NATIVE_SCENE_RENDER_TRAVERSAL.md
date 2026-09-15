# Native scene render traversal (EL)

This packet reconstructs the complete normal bodies at B72190 (84 bytes),
B6D990 (105 bytes), and B6FB80 (36 bytes): 225 native bytes. The source uses
actual scene/node/camera storage and the existing rendering providers. Its
explicit access register and finite borrowed profile domain are source interfaces,
not a drop-in native ABI or proof of a complete application renderer.

| Routine | Native extent | Coverage |
| --- | --- | --- |
| B72190 | B72190..B721E3, 84 bytes | Complete normal body within the finite profile domain |
| B6D990 | B6D990..B6D9F8, 105 bytes | Complete normal body within the finite profile domain |
| B6FB80 | B6FB80..B6FBA3, 36 bytes | Complete normal body within the finite profile domain |

## Original ABI and normal flow

Let S be ESP at entry, pointing at the caller's return address. B72190 receives
the actual scene in ECX and a context pointer at S+4; RET4 consumes that word.
It captures scene+18 with MOVSS, reads the first node from scene+0C, captures
the context and context+8 camera even for an empty list, and overwrites the
original S+4 context word with the captured scalar. Each iteration checks current
node+48 flags against current captured-camera+19C flags. On acceptance it executes
FLD1, reads the current node profile and then current table+20 target, spills
visibility and the saved scalar as two outgoing floats, and calls the captured
target with context, scalar, visibility, and zero flags. It reads next+3C only
after return. It preserves EBX/ESI/EDI and has no native cleanup state or FH3 frame.

B6D990 receives ECX node and four public words at S+4/+8/+0C/+10: context,
LOD float, visibility float, flags; RET10h consumes them. MOVSS parent+AC and
COMISS against current D7A218 gate traversal; JBE also rejects unordered values.
The accepted path loads first child at parent+34 and captures context/flags only
when that child is nonnull. Each child iteration loads the original visibility
with FLD32, captures the current child profile, multiplies by current parent+AC,
and only then loads current table+20. FSTP32 writes the product into the original
public FLAGS word S+10, followed by FLD32/FSTP32 from that same word into the
outgoing visibility argument. Original flags remain captured in EBX. LOD is
loaded/spilled separately, and the original visibility is never accumulated or
replaced. Next+3C is loaded after the child call. EBX/EBP/ESI/EDI are preserved;
the nonpositive/unordered, empty-list, and loop exits balance their own pushes.

B6FB80 is the complete camera forwarding body. It loads current flags,
FLD32 visibility, loads context, pushes/spills the four arguments, FLD32/spills
LOD, and calls B6D990 at B6FB9C with unchanged actual ECX. Its own public words
remain in place; the child may reuse its outgoing FLAGS word. It returns RET10h.
No body in this packet installs a native exception handler or owns cleanup.

## Actual tables and providers

`NativeRender20Profiles` borrows four fixed pointers to live DWORD table backing:

| Numeric profile | Current native +20 target | Existing pointer provenance |
| --- | --- | --- |
| D62C88 | B6D990 | Camera environment's base-node table; identical to both model environments' base-node table |
| D62CF0 | B6FB80 | Camera environment's camera table |
| D62DE8 | B748E0 | Model environment's model table; identical to the Traceline model environment's model table |
| D0C928 | AF26A0 | Traceline lifetime access's actual Traceline table |

The constructor checks nonnull pointers and canonical base/model table identity
before native execution. The caller supplies the actual model environment used
by the live Traceline owner, not an unrelated matching facade. Binding records and
their backing identity must remain stable through traversal and reentry. Table
contents stay current; no table snapshot, parallel registry, allocation, or
successful placeholder callback is introduced. Reachable owners must belong to
these canonical environments and the existing provider lifetime domains.

D0C8C8 is excluded: its native +20 is BF698E (`__purecall`), not AF26A0.
Other tables referencing B6D990 also exist and are outside the four-table binding.
An admitted table's *current* target can differ from the listed installed value.
The captured target is dispatched to the actual existing B748E0/AF26A0 source,
or to the new B6D990/B6FB80 source. Other captured values, including zero or
BF698E, reach the existing application's `call_virtual20` contract. That actual
dispatcher must retain its failure/terminal behavior; the packet invents no
successful unknown-target behavior. Existing child providers retain their
documented context, callback, exception, and rendering limitations.

## Source ABI, scratch slots, and dispatch ordering

The three entries are MSVC Win32 naked fastcall functions. ECX and all original
public stack words retain their roles; EDX supplies `NativeTracelineRenderAccess`.
The appended `render20_profiles` pointer is at access+38h (old members retain
their offsets; size increases from 56 to 60 bytes). Existing initializers default
it to null. New entries require a live binding, services, and actual zero cell.

Scene traversal keeps access at S-20 and the translated table pointer at S-16,
below its three native saved registers. LEA reserves these eight bytes while
preserving the first-node TEST flags. After four outgoing argument words, the
private table/access reads are ESP+14h/+10h. The original scalar word remains S+4.

Node traversal keeps access at S-8 and table at S-4 before its saved registers.
The loop begins at S-24. After pushing flags and reserving the two float words,
ESP+34h is the original S+10 FLAGS spill, and ESP+2Ch is the original S+8 LOD.
After pushing context, ESP+24h/+20h address the private table/access. Camera
forwarding keeps only access at S-4 and restores EDX from ESP+10h after outgoing
arguments. All private storage is removed on every normal return path.

`translate_captured_profile` has a private integer register ABI: EAX is the
already-read numeric profile, EDX is access; EAX returns the actual table or zero.
It preserves ECX, nonvolatile registers, net ESP and the FP stack; its source is
only integer loads/comparisons/branches/RET, with no call, allocation, slot read,
owner reread, FP or SSE instruction. Thus it can execute while the original x87
value is live. B6D990 performs FMUL after translation and before the target read.
Generated confirmation remains pending the coordinated build.

The captured-target dispatcher receives target in EAX, owner in ECX, access in
EDX and the unchanged four outgoing public words. Three private saved words are
removed before the selected known body is tail-jumped to. The shared selector is
also naked and integer-only; model/Traceline targets remain available without the
new binding, while node/camera targets require it. A separate saved table pointer
distinguishes unavailable binding/profile from a mapped target value of zero.
Unavailable profiles throw a source-only diagnostic after the native FP spills;
they do not silently render or manufacture a native hardware fault.

The application fallback copies the four outgoing words into an additional C++
call with the captured target, then returns over the original four. Its adapter
reconstructs float arguments from captured words and calls the existing service.
This has extra C++ stack/FP/exception behavior: no original stack-alias identity,
incidental EFLAGS/volatile-register identity, unrestricted FP-capacity guarantee,
hardware-fault/FH3 parity, or exception unwinding through naked frames is claimed.
Valid normal-path source calls and the application's established dispatch domain
bound this implementation. Original whole-body byte identity is not claimed.

The old `render_child_bridge` still invokes its current `profile(actual)` callback
after its caller's FP spills, then uses the shared known-target selector. Its
null-binding node/camera application fallback is preserved. Only that selection
and the include changed in the old renderer source; its other bodies are intact.
No claim moves the legacy callback before the native FP operations.

## Evidence, callers, and validation

The installed PE SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Fresh BSP-guarded bytes match all three native bodies and five complete 24h table
prefixes (eight spans, 405 bytes). The retained EL native artifact includes exact
hex/hashes, prototypes/comments, xrefs, B91A50 preparation, and copied sealed
EF/EJ evidence. The report records every direct/indirect native transfer and
provider/source pins. Original placeholder prototypes are not ABI evidence.

All four current B72190 callers are retained: A8AE66 passes the captured command's
B1BF30 context result before the later B1BF20 scene result; B1EBC5 reads current
command+28 context then command+4 scene after queue append/late flag; B1F488 and
B1F593 pass current command+28 and the callers' first public scene argument after
append. B6D990 is also reached by B91B07 after its caller's actual flag selection
and separate visibility/LOD spills; no caller body gains reconstruction credit.
The camera virtual reference is D62D10; additional base-node table references
are recorded without broadening the admitted profile domain.

At capture, B72190 and B6D990 were defined; B6FB80's full 36 bytes were not a
Ghidra function. Its missing definition is a primary-owned metadata repair, not
worker function-definition credit. No name/prototype/comment was changed here.
The canonical call audit checks six direct rows: five pass, and B6FB9C fails
because its containing B6FB80 function is undefined. Three indirect-slot rows
are explicitly skipped by that verifier and retain native instruction evidence.

Primary source/stack-order review accepted the frozen draft. Build, generated
object review, and existing tests are pending the single coordinated EL+EN build;
no separate worker build, new test, or runtime fixture was run. Generated review
must cover every new code section, integer-only helpers, all stack exits, emitted
application-fallback FP moves, preflight, and the complete affected old renderer
object. Source reconstruction and PE evidence do not establish game validation,
complete queue/job closure, or a runnable renderer.

The initial coupled build at `cd637cb1e8e841b790d33abaceb2d5a061ae2c12` failed because `bound` is a reserved MSVC assembly instruction name; no CTests ran. The primary changed only its four label/reference tokens to `scene_profile_ready` and `node_profile_ready`, preserving every operation and stack offset. The failed inputs, logs, partial artifacts and complete build tree are archived with hashes in the report. The corrected source will use a separate en2 attempt; generated and runtime claims remain pending.
