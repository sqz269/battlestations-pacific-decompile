# Controller-tree range erasure

Whole `006A6A20` has200definedbytes/76instructions in two typed ranges within
a201-byte span. Outside NOP `006A6A9F` is excluded. Native ECX is the retained
tree, EDX unused, five stacked output/FIRST/LAST words, EAX output/RET14h.
Source reuses the published real8B trivial NativeInputControllerTreeIterator
and adds borrowed RawCtx after both by-value iterators: RET18h. It is a new
Source ABI. Established STL library identity and the older populated
NativeKeyboardTreeIterator AV/typed overload are preserved.

Capture initial FIRST.owner before CURRENT head.left; a returning diagnostic
does not repair that capture. Read FIRST.node after the diagnostic. If it
equals the retained begin, capture LAST.owner and then CURRENT head BEFORE
its diagnostic; read CURRENT LAST.node afterward and compare the retained head.
The full path captures CURRENT root and calls actual controller subtree with
the same opaque tree/RawCtx. Only success permits headA.root=self; capture
headB BEFORE count0/left=self; reload headC AFTER left and set right=self.
Capture final CURRENT head.left BEFORE output pointer; store tree owner then
captured node and return that actual output. Never free the head here.

The complete partial path compares cached FIRST.owner with CURRENT LAST.owner
and diagnoses BEFORE comparing cached node with CURRENT LAST.node. Capture OLD
owner/node before advancing the actual private FIRST argument. Pass OLD as a
distinct by-value argument to the whole actual controller eraser, with a real
private8B output whose contents are ignored. That child advances its own OLD
copy too. After success reload CURRENT FIRST.node BEFORE FIRST.owner and loop.
Diagnostics may return or throw; no owner/endpoint repair, extra ownership
guard, prefix shortcut, callback, generic range, layout or context bridge is
added. Child failure retains completed FIRST advance and child topology/pool
progress; no local EH, cleanup guard, catch or rollback is introduced.

| Actual owned Native transfer | Meaning |
| --- | --- |
| `006A6A3A CALL 00BF6713` | initial FIRST owner diagnostic |
| `006A6A56 CALL 00BF6713` | candidate LAST owner diagnostic |
| `006A6A6A CALL 006A62D0` | CURRENT root to actual controller subtree |
| `006A6AAA CALL 00BF6713` | partial cached owner versus CURRENT LAST owner diagnostic |
| `006A6AB9 CALL 0069AD50` | advance actual private FIRST |
| `006A6AC7 CALL 006A6010` | actual whole controller eraser with OLD/private output |

Read-only comparison below promotes no additional Source. Full Native bytes
and operands are retained. Only six E8 displacement fields map to the published
int/string reference; the other176definedbytes agree exactly. The equal
outside NOP is still excluded. These rows identify the decoded comparison
sites and targets separately from the six owned transfer claims above.

| Owned site | Owned target | Read-only reference site | Reference target |
| --- | --- | --- | --- |
| `006A6A3A` | `00BF6713` | `006A1B8A` | `00BF6713` |
| `006A6A56` | `00BF6713` | `006A1BA6` | `00BF6713` |
| `006A6A6A` | `006A62D0` | `006A1BBA` | `006A0AA0` |
| `006A6AAA` | `00BF6713` | `006A1BFA` | `00BF6713` |
| `006A6AB9` | `0069AD50` | `006A1C09` | `0069AE50` |
| `006A6AC7` | `006A6010` | `006A1C17` | `006A07D0` |

Fresh evidence covers400defined PE/live bytes/152starts, both complete typed
bodies and161exact start/target/outside-NOP contexts. The historical readiness
gates are superseded by fresh published-git ancestry at Main d15ba1ba: whole
eraser27fda626, subtree f9d3833e and fixed-nil21 leaves via44b08ee2 are published.
All46 provider LF fingerprints match that Main snapshot; borrowed actual
RawCtx and current SDK diagnostic bindings are used directly.

All actual CMake code was read:267B/115physicalinstructions in five code
sections, six public-code REL32, no data relocation or local EH. Public
220B/90 includes its2B NOP at8E; four helpers are47B/25. FIRST.owner load8
precedes head0E/11 and diagnostic1E; FIRST.node loads26. LAST.owner2D then
head30 precede diagnostic3E and CURRENT LAST.node46. Actual subtree5B precedes
headB66/count69/left70/headC72/right75. Result node7B precedes output7D and
owner/node stores80/84. Partial keeps OLD values in callee-saved ESI/EBX across
FIRST incrementAC; stack materialization occurs afterward, before eraserBE.
Its ignored output is private EBP-C. NodeC3 reload precedes ownerC6; both paths
use RET18 at8B/D9. EBP remains the compiler frame, not Native EBP's bound/head.
Private argument origins, stack materialization and register/flag/fault scope
are qualified separately from these public raw-access schedules.

Default MSVC Win32 build exited0; three existing suites passed in10.49s.
No new tests/probes or Native execution occurred. Existing tests do not prove
this routine against Native game state. CurrentCpp/RawCtx/current CRT, child
nested-cleanup failure, Original FH3/SEH/RTTI/heap/fault/private-frame/register/
flags/drop-in ABI/runtime/game limits remain. Parent/member/owner functions and
all existing AV/typed providers are outside this Source packet.
