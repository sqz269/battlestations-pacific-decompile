# Actual node and light scene attachment

Addresses: `00B6ED80`, `00B6EE10`, `00B7C020`, `00B7BD60`.

The new source consumes actual node/light storage and actual 3Ch resource
identities. It composes the genuine raw registration gates, raw registry and
canonical actual+4 owners. It does not close the legacy node/light destructor,
outer-scene attachment or game-runtime ownership graph.

| Routine | Extent | Original ABI | Coverage |
|---|---|---|---|
| B6ED80 set node scene | `[B6ED80,B6EE0C)` 140B | ECX node; scene and recursion words; RET8 at B6EE09, 3B | Complete body, required actual dispatch and valid domains below |
| B6EE10 remove matching scene | `[B6EE10,B6EE7E)` 110B | Same; RET8 at B6EE7B, 3B | Complete body, same boundaries |
| B7C020 append retained light scene | `[B7C020,B7C0BF)` 159B | ECX light; scene and recursion words; RET8 at B7C0BC, 3B | Complete body, same boundaries |
| B7BD60 remove retained light scene | `[B7BD60,B7BDEA)` 138B | Same; RET8 at B7BDE7, 3B | Complete body, same boundaries |

All 547 bytes agree between the saved executable and live Ghidra. These bodies
use integer loads/stores, not x87/SSE. There is no local EH registration,
cleanup funclet or rollback. The EAX result is not a supported source return
value. Source interfaces are not drop-in thiscall binary replacements.

## Actual domains and dispatch

Node+170 is one actual resource pointer. Light+178 is a separate actual 12B
array descriptor `{begin, signed count, signed capacity}` at +178/+17C/+180.
Children use current +34 head, numeric profile at +0 and current virtual
+50/+54; +3C is loaded again after each completed child call. Resource+4 is
the one real reference count; resource+14 embeds the raw 28h registry with
0Ch list nodes and 8B iterator pairs.

`NativeNodeSceneChildDispatch` requires a pure profile lookup returning the
borrowed actual table, followed by invocation of the exact captured target
on actual storage with the captured scene and supplied recursion word. Lookup
may not perform native callbacks or mutate native state. There is no default,
logical projection or no-op target. These four methods pass recursion 1 to
children. The same interface permits a reached actual self with recursion 0
for the separate root-propagation caller. A concrete application must bind
every reached target and retain initialized recursive frames/acquisitions.

The current CE221C and CE2220 imported services and the SAME canonical
`NativeRenderActualOwnerRegistry` used by raw resource/ambient construction
are borrowed. A zero result resolves the captured actual owner, verifies its
companion count aliases that actual+4, and invokes its genuine current-profile
terminal. The existing canonical terminal is noexcept and requires an
observed actual zero count and valid current profile. Arbitrary decrement
continuations that report zero after changing the count are excluded. No
admission, count copy or additional credit is introduced here.

Do not install these raw identities in the legacy `SceneAttachmentRuntime`,
`NativeNodeDestructionRuntime`, `NativeCameraReference` scene-removal callbacks
or host `SceneResource` cells. Their complete destruction/root attachment
closure remains separate. The existing `SystemAmbientBacklinks` pointer type
is used only as an opaque 32-bit identity carrier; this code never dereferences
one of those entries as a host `SceneResource`.

## Captured and current schedule

* B6ED80 loads initial node+170 at B6ED83 before capturing the scene argument
  at B6ED8A. If different, it unregisters the captured initial owner, then
  reloads current +170 at B6ED9D. If this differs, it publishes the captured
  request, reads current increment, then reads current decrement for the
  captured reloaded old owner. After possible zero dispatch it reloads +170
  again at B6EDD5 and registers that current resource.
* B6EE10 captures expected at B6EE11 before initial +170 at B6EE18. A matching
  path unregisters the initial owner and reloads current +170 at B6EE2D. It
  clears the slot before decrement/zero dispatch. A replacement installed by
  terminal callbacks therefore survives. A mismatching path still recurses.
* B7C020 captures begin/count/end before capturing the request at B7C03E.
  Search uses unsigned cursor/end and the native signed nonnegative index
  test. An absent identity reads current capacity then current count; doubling
  uses the native wrapped word followed by signed minimum-one clamp. After
  B7B390, it reloads count before begin, writes the captured request if the
  computed address is nonzero, increments current count, registers the
  captured resource, then reads current increment. A duplicate still recurses.
* B7BD60 also captures begin/count/end before request. A found identity passes
  the address of the ORIGINAL scene argument cell to B7B620, then unregisters
  and decrements the captured request. This is not a local key copy. An absent
  identity still recurses. Its initial unused PUSH ECX is not invented as a
  source scratch field.

Every method reads only the current low byte of its recursion argument after
all preceding calls. It then loads current child head, current child profile,
current reached slot and, after invocation, fresh sibling +3C. All recursive
calls use the originally captured request, regardless of callback changes to
the caller argument word. No slot, import or sibling is hoisted across calls.

## Caller frames, diagnostics and failure

`NativeNodeSceneAttachmentFrame` exposes the initialized mutable scene and
recursion argument words and separate nested gate frames. Scene/recursion
are the two native stack arguments at entry +4/+8. Nested gate
`node_argument` cells are written at the native push sites. Their remaining
initialized preimages are borrowed; nested registry frames preserve their
documented current-cell contracts. Frames must be stable and accessible
through callbacks. This layout does not claim an identical native private
stack or arbitrary overlap with native saved registers/return slots.

Fresh `NativeNodeSceneAttachmentAcquired` is disjoint caller metadata. It
records started/completed operations, active call site, captured/released
identity, publications and nested registration acquisition. It performs no
native stores, admission, destructor or rollback. A callback that mutates and
then throws may have effects beyond completed flags. Keep the metadata,
frames, bindings, payloads and retained allocation prefixes available for
explicit disposition; do not infer reference credits from a throwing call.

In particular, light append and count increment precede registration, which
precedes the scene increment. Registration failure leaves that array entry
and any registry prefix acquired so far. It does not grant a scene credit.
Calling ordinary removal as an invented rollback would decrement a credit
that was never acquired.

The admitted array domain requires accessible nonoverflowing spans,
nonnegative valid counts/capacities, disjoint successful CRT allocations and
valid current descriptor fields after callbacks. Corrupt signed spans,
wrapped/null allocator success and arbitrary descriptor/backing overlap are
not supported. Raw nested registry query/erase/count helpers retain their
explicit valid-iterator restriction; this is not whole invalid-state
compatibility. Source C++ exceptions are not native FH3/SEH fault unwinding.

## Providers and evidence

Direct sites call raw B83D50/B83EC0 gates or existing B7B390/B7B620 array
providers. Gate predicates use current bootstrap tokens/profile dispatch;
registry operations use actual borrowed keys without owner changes. The
existing array provider uses the actual singleton CRT allocation/free domain;
no new allocator substitute was introduced. Root repaired B7B390's returning
free flow at B7B3DC; fresh read-only evidence has all 95 bytes/38 instructions,
including B7B3E1 ADD ESP,4, B7B3E4 begin store and B7B3E6 capacity store.

The report lists all 19 calls: seven direct calls mechanically checked with
zero failures, and 12 indirect import/virtual calls with their exact native
operand and explicit binding. Workers made no Ghidra changes.

Strict `scripts/build.ps1` Win32 completed with all three existing CTests.
One ignored focused executable compares all four copied original bodies with
the new source. It constructs genuine raw resources, ambient owners and
canonical companions, uses real gates/registry/array providers, and binds
recursive dispatch to the matching four bodies. Observed payloads and call
traces agree for current-old replacement, changed import, mutable incoming
arguments/late low byte, changed sibling/current profile, equal/duplicate and
absent paths, and real zero retirement whose free callback installs a valid
retained replacement. A source-only genuine registry allocation failure
checks the surviving light append without a credit or rollback.

The copied functions' direct calls are relocated to genuine source provider
bridges; their virtual targets are relocated in borrowed tables at the actual
numeric profile addresses. Original terminal dispatch reaches a bridge to
the real canonical zero provider. This is composition evidence for these
547 bytes, not independent byte proof for the dependencies, native fault
unwinding, arbitrary recursive profiles or game validation. The fixture
aborts if the required profile/terminal address pages are occupied; it does
not replace an existing mapping. No tracked test suite was added.

Source/header, native byte hashes, call evidence, compiled artifacts, fixture
and prior archive receipts are recorded in
`reports/native_node_scene_attachment_cc10.json` and the frozen local archive.
