# CC12 next native string consumer: 008F0310

**Source 0 / ready 0 / registered 0.** One additional consumer has a bounded
physical contract suitable for a conditional raw-operation proposal. The saved
listing needs Root repair before implementation. No C++, probe, runtime process,
Source packet, ledger or Ghidra mutation was produced by this readiness pass.

## Whole body and actual interface

The selected function is `FUN_008f0310`, complete native range
`[008F0310,008F0340)`: **48 bytes / 16 physical instructions**, SHA-256
`c4c713091d8c81c8083a9c53eba6d356e55cb459b8c619ac703ead3796b04abf`.
All installed PE bytes match live Ghidra bytes before and after. The first twelve
indexed callers of 00438E40 were considered; completed constructors/copy, leased
tag-6/parent/map work and pending type-8 work were excluded. Only this entry was
claimed or inspected live. The new worktree begins at Main
`35a834a227c3e04de082241124e758ec120ada35`.

A possible descriptive interface, requiring future Root registration, is:

```cpp
uint32_t __fastcall replace_native_reference_payload_008f0310(
    void* destination_ecx, void* unused_edx,
    const char* borrowed_text_stack, uint32_t raw_scalar_stack);
```

The name and interface are reconstruction hypotheses. Physically, ECX is an
actual eight-byte writable payload. Native entry `[ESP+4]` is a borrowed text
pointer and `[ESP+8]` is a scalar DWORD. There is no source-payload pointer.
The body saves ESI, tests old destination +4 and, if nonnull, passes that pointer
to the original free call. It then restores ESP by four and clears +4. It loads
the text argument into ECX, calls the physical duplicate, and stores the actual
new allocation or null at destination +4. Finally it reads the scalar argument
into EAX and writes destination +0, restores ESI and executes RET 8.

**Full EAX is the scalar argument, including when text is null.** The new owned
pointer must be read from destination +4. The write order is pointer then scalar
after the duplicate; this differs from the earlier 55-byte copy operation.
Normal EBX/ESI/EDI/EBP survive. No phase, vtable or other record fields are touched.

The whole-body proposal retains 40 literal bytes. Only operand `[12,16)` of CALL
008F031B may bind the actual unchanged current canonical free, and operand
`[31,35)` of CALL 008F032E may bind the admitted physical duplicate57. No convenient
branch, fake semantic class, vtable, callback provider, new provider adapter or
virtual R125 substitute is acceptable. The existing admitted duplicate's size
adapter remains its actual dependency.

## Ownership and caller boundary

Destination +4 must be null or a sole-owned actual allocation from the matching
current duplicate/provider. Borrowed text must be null or readable through its
NUL with representable length+1, remain live during duplication, and be disjoint
from the old allocation, payload and active frames/argument slots. Current CRT
execution requires DF clear. The old free happens before loading the text
argument. A surviving pointer argument does not retain the allocation it names:
text inside the old allocation would be a use-after-free and is excluded.

The consumer frees a real old copy once. The caller observes and frees the actual
new pointer from +4 exactly once before disposing of or reusing payload storage.
Borrowed text is never freed here. Old bytes must not be read afterward; the
allocator may reuse the old address. No `noexcept`, OOM/EH or native class cleanup
promise follows from this normal-return contract.

The index reported zero callers; live caller and inbound-xref queries also found
none. No direct caller argument-transport witness exists in this packet. The
two stack arguments and RET8 are established from the whole callee body only.
No caller window/cell was claimed, no call site was invented and no World/class
expansion was attempted. Native production caller, producer/destructor and
original private-heap ownership remain explicit graph gaps.

## Saved listing issue and Root prerequisite

Ghidra lists 14 instructions and decompiles a false return after the old free.
The installed/live 48 bytes decode to 16 instructions. Missing starts are:

| Start | Bytes | Instruction |
| --- | --- | --- |
| 008F0320 | 83c404 | ADD ESP,4 |
| 008F0323 | c7460400000000 | MOV [ESI+4],0 |

The read-only flow report identifies exactly `[008F0320,008F032A)` after CALL
008F031B. Exact instruction-context reads report no instruction at both missing
starts and preserve the existing continuation at 008F032A. The exact call-site
override, callee NoReturn and candidate NoReturn values are **unmeasured** here;
the similar earlier defect does not prove their current values. No scripting
setting, clear-override endpoint or repair command was invoked.

Full before/after documentation is identical: name `FUN_008f0310`, undefined
return, unknown calling convention, empty parameter/comment arrays and label
`LAB_008f032a` at relative offset 26. Those stored annotations do not override
the recovered register/stack evidence. Their normalized documentation hash is
`6bf33188f1afb68a20f0079982e36e30dbbeed4240efcf326f8ebc8450c43c4c`.

Root must own a fresh repair family and the entry/callsite/gap addresses, verify
the same full 48 bytes and one internal gap, then use the supported locked repair
while preserving old values. Do not change or claim the original private free.
Verify all 16 exact starts, unchanged full extent/RET8, genuine old-free
fallthrough and preservation of names/prototype/comments/labels. Save and refresh
the affected export under the required lock. A listing repair grants no Source
or production-class credit.

## Nested stack and defined flags

Let T be the first argument-slot ESP before the outer CALL, with text at T and
scalar at T+4. Outer entry is T-4 and saved ESI is T-8. Optional old-free argument,
entry and return are T-12/T-16/T-12; ADD4 restores T-8.

Duplicate entry is T-12. Its EBX/ESI/EDI saves are T-16/T-20/T-24; the size
argument is T-28. Memcpy destination/source/count are T-40/T-36/T-32, entry is
T-44, return T-40 and final ADD16 yields T-24. Child RET0 returns T-8, then outer
RET8 yields T+8. These are derivations for the future fixture, not new runtime
observations.

With null text, ECX is zero, EAX is the scalar after the caller's MOV, and mask
`0x8C5` expects `0x44`; undefined XOR AF is excluded. EDX retains its incoming
value only when old +4 was null; actual old free may clobber it. With nonnull
text, EAX is still the scalar, ECX/EDX are volatile, and mask `0x8D5` must derive
from `ADD32(T-40,16)`. No blanket full-flags, FP, MXCSR or segment claim is made.

## Conditional future implementation and fixture

The report's graph is bounded to 13 nodes and 13 labeled edges, retaining the
unknown caller, native owner/private free and EH dependencies. Unknown dependency
edges are explicitly distinguished from observed native calls. Existing admitted
duplicate/current matching free contracts are reused without querying their
bodies. Type-2/type-5 raw constructor admissions provide no child-class lifetime
credit. The earlier 55-byte consumer and pending type-8 work provide no dependency
admission for this proposal.

Only after Root's repair and review, a future packet could own the whole48 source,
header, doc and report named in the JSON. A minimal fresh fixture uses four new
translation units: consumer, admitted duplicate/adapter, unchanged canonical
allocation/free and a new RET8 caller/probe. Seven entries suffice: three native
and Source raw pairs (old-null/text-null, real-old/text-null, real-old/nonempty),
plus ordinary Source old-null/nonempty. Four actual old setups/frees and three
actual new copies/result frees exercise ownership. Use a distinctive scalar and
assert it in full EAX on every path; observe the allocation through destination
+4, not EAX. Verify complete n+1 including NUL, payload/borrowed text/frame guards,
actual two argument slots, defined flags and normal nonvolatiles. Allow allocation
address reuse and never read freed storage.

Before zero old-setup/consumer/free entries and afterward, future gates must
cover complete actual COFF/map bodies, both REL32 bindings, duplicate/adapter/
canonical allocation/free, ordinary/raw callers, whole main, all retained helpers,
actual cookie/stack/import bodies, and physical I386 UCRT malloc/free/_callnewh
plus distinct VCRUNTIME memcpy IAT/export/NTpath/file-ID/full-SHA/ASLR-prefix.
One manifested Win32 process, full consumed-input post checks and exact immutable
artifact inventory are required. No such build or runtime was performed here.

## Evidence preservation

The family is `local/cc12_native_string_next_consumer_readiness20261008a/`.
All 112 prior Source-worker artifacts and 1,100 older artifacts retain their
original hash associations. The four prior Source-worker tracked files also
match their recorded hashes; that Source credit remains zero pending Root.
Current provider files match the previous physical file identities, which is a
file check only and provides no new runtime proof. Current Main dispatch/CMake
context was frozen separately from strictly checked consumed local inputs.

The early missing private index result was retained and resolved by building the
private index once. A selection-scope erratum narrows an overbroad assertion in
the initial metadata: top-level receipt names were inspected, not every nested
historical snapshot. Selection relies on the retained fresh bounded indexed
list. No live duplicate query was repeated. The final manifest includes all
helpers, receipts, issues and any pycache, excludes only itself, and is checked
against exact inventory. Source, registration, native class/caller/private CRT,
EH, startup and gameplay remain unadmitted.

## Root listing repair and Source registration

Root verified all48 PE bytes/16starts, used the supported locked internal-gap repair once, preserved the old CALL_RETURN response, complete prototype/comments/labels and1277 prior pins, saved and refreshed the export. The current listing now includes ADD ESP4 at0320 and clear+4 at0323; no gaps remain. Family seal `147bb521452219953ea701a1e21c8a2be0f2d0e52857587a4b396bf368c073f3`, exact20 files. Qualified Source packet `cc12_native_reference_payload_replace` registered with EAXscalar/+4actualchild and currentmatchingowned-copy domain; Sourcecredit0 pending worker and independentRoot fixture/mainbuild. No originalprivate CRT/class/caller/game admission.
