# Native game tree cleanup bindings (R127)

Addresses: new normal bodies `004C18D0`, `004C1910`, `004D16C0`; consumed range
contracts `004D2000`, `004D22F0`, `004D41A0`, `004CEF40`; existing range
`004D1A50`; parent `004DCF90`.

## Result

All five parent tree methods now have concrete defaults. The calls at4DD3C6,
4DD461,4DD4D5,4DD50A and4DD53F receive the same actual profile/string context
and the parent's retained `operation.trees` observations. Context identity is
checked before touching a tree. This removes five required address-only bindings.

| Entry | Scope | Behavior |
| --- | --- | --- |
| 4C18D0 / 4C1910 | Two complete53B normal bodies | 18h node,nil15. Recurse right,capture left after recursion,free current node,follow captured left. Payload untouched. |
| 4D16C0 | Complete82B normal body | 28h node,nil25. Recurse right;capture string data1C before left,then length18+1 before getter/return;free node,follow captured left. |
| 4D2000 / 4D22F0 / 4D41A0 | Current full-range branches of three201B library entries | Call the corresponding subtree body;reload current head for root/count/left/right resets;capture new first and store output owner before output node. Reject partial ranges. |
| 4CEF40 | Full-range adapter over existing code | Reuse settings cleanup's actual int-only14h/nil11 service. No new general partial-erasure API. |
| 4D1A50 | Existing complete implementation | Delegate the existing raw string-tree range service in the same actual pool domain. Its partial implementation is retained. |

Subtree ABI: ECX tree,one stack node,RET4. Range ABI: ECX tree,five stack
arguments,EAX output,RET14. Explicit source interfaces are not native binary
replacements. Names are descriptive hypotheses,not recovered symbols.

Current native producers and the game constructor establish the storage:
game5B0 uses4C2700 (14h/nil11),5BC uses4C2750 (28h/nil25),5C8 uses4C27A0
(18h/nil15),628 uses4C2830 (18h/nil15),and the raw string tree uses4C26B0
(18h/nil15). Existing source allocation helpers supply the same raw storage;
sentinel nil,links and count are finalized by the owner constructor.

Three returning-free gaps of11 bytes each were repaired. All three53/53/82-byte
bodies now have complete fall-through listings. Final flow audits have no
returning-call gaps;unreachable single-byte padding after range RETs is unchanged.
Prior values,call rows,original bytes and annotation evidence are in the report.

## Validation

Strict MSVC Win32 build and all three existing CTests pass. The byte collector
verified1,328 live/installed PE bytes:188 newly reconstructed subtree bytes,
1,005 bytes in five original range references,and135 bytes in two existing
subtree references. This does not count the reference bodies as new ports.

The64 paired cases match434 observations and472,808 normalized bytes. They
exercise all five actual parent defaults,empty/one/three/seven-node graphs,
plain payload preservation,string-null masks,149/150-byte string allocation
boundaries and small-return gate0/1. Actual raw pool/manager and node allocation
services are used. Callback mutations verify current-head/count reloads,left
capture after right recursion,and data/size/left retention across pool calls.

For the newly reconstructed bodies,the fixture compares pool/free boundaries
and final state. Existing4CEF40/4D1A50 services are compared at entry/final
header,iterator and pool-ring state. Their source internal frees are not directly
instrumented:fixture bookkeeping follows the reviewed pre-call traversal. This
is not a new proof of every intermediate operation in those existing services.

A source-only first-node free failure retains all nodes and the original count
after one string was returned. Borrowed progress records the owner,cursor and
site;external cleanup resolves that graph without replay. Four actual partial
ranges and ten missing/foreign context cases are rejected. These observations
do not establish native FH3 cleanup,rollback or safe replay of failed destruction.

The separate controlled parent comparison passes52 paired cases,3,537 snapshots,
141,402,076 normalized bytes and four failure/replay cases. It checks shared
context/progress argument identity;the nested fixture supplies concrete binding
evidence. No permanent test suite was added.

## Remaining work

Resolve the remaining non-tree parent dependencies and admit the raw game owner
into the application. General partial4D2000/4D22F0/4D41A0 is not reconstructed;
the current packet consumes the exact full-range destructor calls. Malformed
graphs,private-stack aliases,native exceptions,concurrency,binary ABI,ordinary
raw-game admission and gameplay remain unvalidated. No unrelated application
run was repeated for these currently unreachable bodies.
