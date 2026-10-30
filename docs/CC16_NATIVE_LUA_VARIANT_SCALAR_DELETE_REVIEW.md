# Independent raw Lua variant scalar-delete review

Frozen candidate `65d9e8f90e0c2794ddd503ee4acfab443ec5a5fc` passes this bounded
Source/native/COFF/EH review. There is no blocking finding or required correction.
Root's separate common build, actual entry link and final PE checks remain required.
This review neither compiles nor links nor executes reconstructed code.

The review worktree starts at published Main
`33d4b470b7ec752da670c1d2807a41389cd76dab`; the remote ref was rechecked unchanged.
The candidate starts at `879f5d855aa43e099fb349ea7b18d58c7f2d1a40`. Its four Git
blobs were pinned before acceptance: header271 lines, CPP570 lines, document154
lines and structured report5066 lines. The complete Source/document and report
semantics were reviewed; every supplied binary section, relocation, symbol and
decoded operation was checked against a separate parser.

Readiness `5ef322e05be0fa3197cc9ee6d952dcbe5c9dd142`, admitted qualified cleanup
`a4774a47499cf3ad0f38af058a53fef1e71cea61`, its independent review
`b5d3e2c957ca22cb8f29ca0816f6e2aec87d5e73`, and Main's cleanup admission establish
the prior child contract. This review reuses that review only after exact Source,
raw code and normalized relocation comparisons. All six immutable inputs,21
unchanged dependency pins,15 retained author artifacts, six retained baseline
artifacts, and all26 readiness Source pins verify. The header/CPP old prefixes
are exact; Frame120B, Operation24B, sidecar56B, existing APIs and broad Hosts remain.

## Native scope and complete Source sequence

The installed PE SHA256 remains
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
An independent PE parser and x86 decoder verify all135 bytes: scalar30B/11
instructions, child101B/29 instructions, and the4B D0E6F4 word. The complete
scalar is **00884FA0..00884FBD**; RET4 at884FBB occupies three bytes. A28-byte
prefix truncates that instruction. D0E6F4 contains00884FA0.

Native preserves ESI, captures entry ECX in ESI, calls884AF0, then tests the
CURRENT BYTE at the incoming flags home with mask01. Set bit passes captured ESI
toBF65AC; clear bit skips free. EAX receives captured ESI even after free, followed
by restored ESI and RET4. No scalar-local Native EH action/frame is introduced.
Indexed observations remain zero direct callers and one data-pointer site;
they do not close indirect dispatch or the full CF/D0 caller graph.

The new ordinary Source API captures V and its uintptr bits before the actual
qualified884AF0 call, forwarding the SAME strings, operation and cleanup backing.
It tests the returned child Result, never the latest nested operation diagnostic.
Any returned non-completion immediately returns that status and diagnostic bits,
before flags or free, retaining stopped child metadata. A genuine escaping C++
exception has no ordinary scalar result and bypasses flags/free; there is no
scalar catch, cleanup action, metadata frame, guard or replacement exception.

After completed child return, exactly one volatile unsigned-char alias reads
flags-HOME offset0. No DWORD value is evaluated, entry bit cached or receiver
reloaded. Mask01 alone controls the actual canonical free of captured V. Both
ordinary completed paths return completed status plus captured bits. The8B
aggregate has no field defaults or implicit success/Boolean conversion. Returned
bits can already be dangling; non-completion identity is diagnostic, not success.

## Strict emission, unchanged child and whole EH

The retained strict object is29841B, SHA256
`bcf718e1da6a3616432f6ce4ca1a1975b04413611437c73dae9626af73710bdd`.
The retained command/log attest one warning-free MSVC19.51 Win32 `/c` compile
with `/O2 /Ob2 /EHsc /W4 /WX /fp:strict`. This review did not rerun that compile.

Independent decoding accounts for all73 sections and all37 code sections:
5483B/1652 operations/75 direct calls. Every payload, symbol, relocation and
instruction agrees with the author's complete dump. All36 old code sections
5428B/1631 operations are a477-exact, including normal/qualified kernels, helpers,
public entries and six compiler EH handlers. Relocation normalization changes
only MSVC's source-path-derived anonymous-namespace token, retaining names,
offsets, relocation types and target values. It does not erase target identity.

| New55B/21-operation scalar epoch | Independently decoded effect |
| --- | --- |
| 05h,12h | Capture receiver in preserved ESI; forward captured V and the same three metadata/context arguments to the complete qualified destructor kernel. The public forwarding wrapper is inlined. |
| 1Ah,1Ch | Test returned EAX; nonzero jumps directly to33h, bypassing flags/free. |
| 1Eh,22h,24h | Load borrowed HOME address; one MOV AL,BYTE[EAX]; TEST AL,1. |
| 26h,28h,29h | Clear bit skips free; set bit pushes captured ESI to the genuine canonical-free symbol. |
| 31h,33h,36h | Completed EAX=0; EDX=captured ESI; plain RET. The non-completion branch preserves child EAX. |

Both actual REL32 targets were resolved from the object. The sole new external
definition is this scalar; the same seven provider/runtime/cookie imports remain.
No allocator, callback, Host adapter, scalar-local throw or terminal policy appears.
This observed8B C++ return uses EAX=status/EDX=bits, with five ordinary DWORD
parameters and no hidden result-buffer parameter or scalar result-memory store.
It does not implement Original ECX/EAX/RET4 ABI. Common emission requires fresh
Root checks rather than inheriting this strict observation.

All six whole xdata tables632B and normalized relocations are a477-exact. The
independent structural decode accounts for every byte, including four maxState2
tables and two maxState4 tables, all unwind/try/handler rows, and every catch
target at a decoded instruction start. All unwind action words remain zero.
SafeSEH stays24B with the same six handler identities; raw symbol-index words
renumber. No scalar handler or new EH/action table exists. All24 old FPO records
and normalized relocations match; the scalar adds16B describing55B of code, five
DWORD parameters, one saved register, no locals and no SEH flag.

Metadata drift is explicit: debugS212 ->216B changes the object-path record;
the complete compiler record is exact. chks64 grows568 ->584B with71 ->73 sections.
The existing4B logical BSS has raw pointer0 and zero file payload; file-header
bytes are not BSS data. This is no whole-object or unchanged-metadata claim.

After compile, only the header's alias/lifetime comment changed. The compiled
header and final Git header are separately retained and hash-pinned; their
noncomment tokens agree. The CPP is unchanged. That correction preserves allowed
flags-HOME/raw-receiver aliases while keeping context and metadata independent.

## Provider custody and remaining gates

The actual free definition remains noexcept and directly calls `std::free`.
A fresh static first-linker-index parse of Root's current Core archive finds all
three actual getter/return/free definitions; their complete member hashes equal
the previously admitted members. The free definition's only relocation target is
`__imp__free`. The current archive snapshot `f8244bd2...` is a custody observation,
not this review's common-build/link attestation. Exact archive/member receipts are
in the report. No fake provider or executable client was created or run here.

V must be genuine live20B variant backing in the qualified child's full domain.
If the ACTUAL LATE free bit is set, captured V must be the exact compatible
canonical malloc root. An entry flag, numerical profile, tag or size establishes
none of that. A retained embedded receiver cannot be freed as a root. The flags
HOME need only stay live/readable through its actual late read; supported aliases
may change it during the child. A child-freed HOME is outside the domain. The
context, operation/sidecars and Source result metadata remain independently live
and disjoint from mutable homes/raw payload. Reentry needs independent backing;
active metadata cannot be externally mutated. Actual AA8/AA0/AA4, matching
manager/deletion/section/ring/storage and heap custody remain required.

The inherited child exception actions, primary/secondary captures and returned
non-completion policies have no new code drift. Positive C++ behavior still
requires genuine delivery across the real provider boundaries. Naked/register
transport, existing noexcept/CRT/OS/nonlocal exits, Native FH3/SEH/fault/FP and
private-stack-home/async alias epochs, exception-object custody and Original
secondary-terminal behavior remain qualified. Direct symbols or an ordinary
parent catch do not prove those gates.

No callable D0 table, caller/current-slot/Host/producer adoption or activation is
added. Root common build/link/PE admission and later runtime/gameplay checks remain.
This review changes only its two new evidence files. Source, objects, CMake,
ledgers, Ghidra, callers, peers and installs were not changed; compilation, tests,
links and reconstructed-body/probe/DLL/game runs are zero. New Original/vendor/
duplicate-body, Original ABI, runtime and activation credit are zero.

Machine-readable evidence: `reports/cc16_native_lua_variant_scalar_delete_review.json`.
