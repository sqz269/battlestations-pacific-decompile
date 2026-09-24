# Actual node tree retirement

Addresses: `00B6DFA0`, `00B6F310`, `00B8F4C0`, `00B6D850`.

The four raw bodies operate on actual node hierarchy, point-light backlinks
and notification-owner arrays. They require real current virtual targets and
canonical zero retirement. They do not close the full node/light destruction
graph for nodes carrying actual 3Ch scene resources.

| Routine | Exact extent | Native ABI and last instruction | Coverage |
|---|---|---|---|
| B6DFA0 unlink and release | `[B6DFA0,B6E009)` 105B | ECX node, no arguments; B6E007 JMP EAX, 2B | Complete body with required current18 provider |
| B6F310 release links/children/self | `[B6F310,B6F3C0)` 176B | ECX node, no arguments; B6F3BF RET, 1B; zero tail B6F3BA JMP EAX, 2B | Complete body with required current18/current0 providers and valid descriptor domain |
| B8F4C0 unregister notification owner | `[B8F4C0,B8F4EB)` 43B | ECX actual owner, stack node word; B8F4E8 RET4, 3B | Complete body using genuine byte-identical erase engine |
| B6D850 recursive unregister | `[B6D850,B6D884)` 52B | ECX node, no arguments; B6D883 RET, 1B | Complete body with prepared persistent recursive frames |

All 376 owned bytes match live Ghidra and the installed PE. There is no local
EH registration, compiler cleanup, x87/SSE operation or rollback. These C++
interfaces do not preserve the binary calling convention or unspecified EAX
results. The worker changed no Ghidra metadata.

## Ordering and actual storage

B6DFA0 captures parent+30, then previous+40 before clearing parent. It repairs
previous+3C with current next, reloads next and previous for next+40, compares
the captured parent's current head, and takes that head's current next when
replacing it. It decrements captured parent+38 and clears node+30 again. With
no parent it captures root+A4, calls genuine raw B72220(root,node) if nonnull,
then clears A4. Finally it loads CURRENT profile/current18 and invokes the
captured target. There is no node payload access after that tail invocation.

B6F310 traverses the unsigned current count at node+168, reloading begin+164
and each actual light. `NativePointLightLinksRuntime` resolves metadata for
the SAME physical light+1E0 descriptor; genuine B7C1A0 removes one borrowed
node backlink. The reached B6EC70 argument is exactly zero. Existing raw
`shrink_native_node_point_lights_to_zero_00b6ec70` is equivalent for the valid
nonnegative count/capacity domain: decrement count repeatedly, store zero,
retain pointer/capacity/stale elements. This packet does not claim to add a
general B6EC70 resize provider.

The child loop repeatedly reloads current head+34, captures child+3C, publishes
the new head, and clears its previous+40 if nonnull. It captures the child's
CURRENT profile and current18 target BEFORE clearing child+30 and child+3C.
It invokes that captured target, then reloads parent head. It does not
decrement child_count+38. Point-light and child cleanup occur before testing
byte44, even if that release flag was already set.

When byte44 is zero, B6F310 captures A0 BEFORE clearing A4,30,34,40,3C in that
order, then writes only byte44=1. If captured A0 was nonnull, it calls raw
B8F4C0(captured owner,node), then clears current A0 again. It reads CURRENT
CE2220 and decrements actual node+4. Zero loads CURRENT profile/current0 and
tail invokes; no node payload is read afterward. Nonzero leaves scene170,
derived geometry and other owned payload alive.

B8F4C0 captures the incoming node word, compares captured node+A0 to the
incoming owner, and on equality passes ADDRESS OF THE ORIGINAL node argument
cell to owner+178's erase helper. It clears captured node+A0 even when no key
was found. Mismatch has no effect. It does not clear a host notification
callback, unlike the existing logical wrapper.

B6D850 loads current A0, unregisters/clears it if nonnull, then loads current
head+34 and recurses through actual children. Each child's +3C is reloaded
after the recursive call. Its direct recursion uses prepared stable frames
and fresh diagnostics, with no additional native callback or mutation.

## Reused providers and boundaries

B7BED0 `[B7BED0,B7BF37)` and B7B620 `[B7B620,B7B687)` are byte-for-byte equal
over all 103 bytes. The new raw unregister wrapper reuses existing
`erase_system_ambient_backlink_00b7b620` over the actual 12B owner+178
descriptor. Its `SceneResource*` type carries opaque 32-bit node identities;
there is no host scene dereference, fake point-light+1E0 binding or duplicate
erase engine. Native empty-span behavior, first-equal swap-last and one count
decrement agree in the admitted domain.

Existing raw B72220 root unlink, B7C1A0 actual backlink removal and reached
B6EC70(0) are used directly. Arrays require accessible nonnegative,
nonoverflowing count/capacity and the providers' normal disjoint backing
domain; arbitrary corrupt descriptors, asynchronous mutation and malformed
cyclic hierarchies are excluded. No allocator or ownership substitute is added.

`NativeNodeTreeRetirementDispatch` must purely resolve a captured numeric
profile to its borrowed actual table and invoke the exact captured no-argument
18 or 0 target. There is no default, trace-only target or logical fallback.
`NativeNodeTreeRetirementFrames` purely returns already prepared persistent
caller frame/diagnostic objects for each active recursive unregister. It may
not allocate, invoke native callbacks or change native storage during lookup.

The context borrows the SAME `NativeRenderActualOwners` used for canonical
actual+4 retirement. Source verifies the resolved companion's count ADDRESS
equals the reached node+4. The required zero provider must additionally check
the actual count is STILL zero before invoking its genuine canonical terminal,
and honor the exact target/current profile. A decrement that merely returns
zero after changing the count is outside the supported domain. No new
canonical registry, duplicate count or owner admission is introduced.

Existing camera/light terminal companions remain admissible only in their
supported domain, such as scene170 null, retained scene array empty and roots
already cleared. They still route legacy scene/root destruction. They must
NOT be installed as zero providers for nodes carrying actual 3Ch scene
identities. Full raw node/light terminal closure remains a separate packet.

## Frames and failure

B6DFA0 has no caller scratch input: its caller supplies context and fresh
disjoint `NativeNodeTreeRetirementAcquired`. B6F310 and B6D850 additionally use
a stable frame containing the fully written node argument word for B8F4C0.
The standalone B8F4C0 takes a reference to the actual initialized node argument
cell (native entry ESP+4); its address is passed onward unchanged. Saved
registers, return slots and private native stack overlap are not an asserted
source ABI. Recursive binding contexts must preserve their prepared objects
through every reached call and explicit failure disposition.

Acquired flags describe completed operations and the active native call site.
They do not synthesize rollback, a second count or a native destructor. A
throwing genuine dispatch preserves preceding unlink/array/flag mutations;
a callback can have effects beyond flags for calls that did not return. No
native FH3/SEH fault-unwind equivalence is claimed.

## Verification

Strict `scripts/build.ps1` Win32 and all three existing CTests pass. The report
records all 11 native call/tail sites: seven direct calls checked with zero
failures and four explicit indirect imports/calls/tail jumps.

One ignored focused fixture compares all four copied original bodies with
source using genuine root, backlink, erase and resize-zero providers and
concrete recursive B6F310 dispatch. It checks actual parent/root unlink,
recursive A0 unregister including missing keys, current child-head mutation,
late byte44/current import, retained backing and unchanged child_count38. A
controlled accessible profile-table alias places its current18 word at child30;
capturing that target before clearing child30 is necessary for both executions.
That alias uses a genuine B6F310 target and a nonzero remaining count; it is
not a claim that this overlapping profile is a normal game class.

A separate source check constructs a genuine physical point light from its
actual pool, binds its canonical `NativePointLightReference` without a new
credit, and reaches actual zero through B6F310. The provider checks exact
actual+4 identity/count zero and scene-null/cleared hierarchy, then runs the
real point-light terminal/destructors, pool return and metadata retirement.
The borrowed type slots are initialized fixture state and predicates are the
genuine implementations; this does not establish type startup or native
terminal byte equivalence. Actual 22-word point/light/node profiles are pinned
to live/PE bytes. No trace-only terminal is used.

Copied-body direct calls and IAT operands are relocated to genuine providers;
original/source use equivalent current borrowed profile tables. The zero case
is source composition evidence, separate from the 376B differential comparison.
No tracked tests, game run, general invalid-state or full terminal-graph claim
is added. Frozen source/byte/call/build/probe evidence and unchanged prior
archives are recorded in `reports/native_node_tree_retirement_cc10.json`.
