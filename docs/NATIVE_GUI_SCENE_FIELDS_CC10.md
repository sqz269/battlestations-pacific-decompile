# Raw outer-scene resource and root-node fields

Addresses: `00B723F0`, `00B721F0`.

These leaves operate on the actual `24h` scene prefix. Scene+1C contains an
actual resource identity; scene+C contains an actual node identity. They never
interpret those cells as host `SceneResource`, `CameraTransform` or
`RenderNodeRootList` pointers. Existing logical APIs remain separate. Names
are descriptive hypotheses, not recovered symbols.

| Routine | Exact body / final instruction | Coverage |
| --- | --- | --- |
| `B723F0` resource setter | `[B723F0,B7242B)`,59B; `B72428 RET4`,length3 | Complete actual-storage body with required current imports/canonical terminal |
| `B721F0` prepend node | `[B721F0,B72211)`,33B; `B7220E RET4`,length3 | Complete actual-storage body including payload aliases |

The setter captures its requested argument first (`B723F0`), then the current
resource at scene+1C (`B723F5`). Equality returns without writes or callbacks.
For a change it publishes the captured requested pointer, increments its
actual+4 through current CE221C if nonnull, then decrements the captured old
owner+4 through current CE2220 if nonnull. The decrement import is read after
the increment callback, so the callback may replace it. A zero result alone
resolves the old owner in the shared canonical registry. The companion must
borrow that exact+4 atomic and genuinely dispatch its current virtual0.
Missing identities/profiles are errors; there is no fallback or copied count.
No final field write, rollback, admission or additional credit is added, so
callback changes to scene+1C survive. A change to the caller argument cell
after its initial capture does not select another owner for the increment.

The prepend leaf first captures scene+C, stores that value to node+3C, and
zeros node+40. It then rereads scene+C. A nonnull current head receives node
at head+40; finally scene+C receives node. The reread matters when scene+C
aliases node+40: the zero store changes the current head to null, so the old
head is not updated. All payload accesses are volatile and retain this order.
There are no reference operations, calls, node attachment or type projections.

The original setter's three calls are indirect: current CE221C at B72407,
current CE2220 at B72415, and captured old owner's current virtual0 at B72425.
The report records each exact operand and instruction; the static call
verifier reports them as indirect rather than proving their runtime targets.
Both complete bodies,92 bytes, match live Ghidra and the installed executable.
No listing repairs or Ghidra mutations are required by this packet.

Strict MSVC Win32 build and all three existing CTests pass. One ignored probe
compares the original bodies with source: a setter sequence using three real
raw3Ch resources and their actual-count canonical companions, plus ordinary
and deliberately aliased prepend storage. The increment callback retains and
publishes a third resource, changes the caller word, and replaces the current
decrement import. The comparison checks the captured requested/old counts,
current import order, surviving field mutation, equal/no-op and null branches.
Cleanup disposes the observed credits through real resource/ambient terminal
paths. The original setter's zero-count virtual call is not exercised by this
comparison; that required canonical path reuses the separately proven resource
companion. No synthetic terminal callback or broad tracked suite is added.

Existing raw B72110 and B72220 providers are unchanged. Whole attachment,
registry insertion/removal, AC59A0, logical scene-graph integration, original
caller-stack ABI, concurrent mutation and gameplay remain outside this packet.
