# Actual point-light arrays for the Text Model clone

Addresses: `00B6E500`, `00B6EED0`, `00B6F090`, `00B6F150`, `00B6F3C0`,
`00B7BE40`, `00B7BED0`, `00B7C160`, `00B7C1A0`, `00B7C710`, `00B7C770`.
Descriptive names are hypotheses. Existing library names are retained.

Text flags26h,parent0 now copies positive point-light lists into actual native
arrays. The destination stores raw light addresses at node+164; each light's
same +1E0 descriptor stores raw node addresses. `GeneratedModelPointLightLinks`
remains a separate diagnostic interface and is never stored in these arrays.
The earlier positive-count boundary in the Text clone docs is superseded here.

| Routine | Native ABI | Coverage |
|---|---|---|
| B6E500 | ECX descriptor, signed capacity on stack, RET4 at B6E55C | Complete valid-extent grow/copy/free/publish |
| B6EED0 | ECX node, raw light on stack, RET4 at B6EF1A | Complete append forward then reverse |
| B6F090 | ECX light-pointer descriptor, address of raw pointer on stack, AL bool, RET4 at B6F0C7/B6F0F4 | Complete valid-extent first-match swap-last erase |
| B6F150 | ECX source, destination/flags/parent on stack, RET0C | Partial: Text flags26h,parent0 including B6F164..B6F1BE; other flags/parent and child-clone B6F223..B6F26F variants external |
| B6F3C0 | ECX node, raw light on stack, RET4 at B6F3D0 | Complete forwarding erase contract |
| B7BE40 | ECX light, raw node on stack, RET4 at B7BE80 | Complete append to physical reverse descriptor |
| B7BED0 | ECX node-pointer descriptor, address of raw pointer on stack, AL bool, RET4 at B7BF07/B7BF34 | Complete valid-extent first-match swap-last erase |
| B7C160 | ECX light, RET at B7C199 | Complete live unlink pass and resize0 |
| B7C1A0 | ECX light, raw node on stack, RET4 at B7C1B0 | Complete forwarding erase contract |
| B7C710 | ECX raw light, name on stack, EAX raw light, RET4 at B7C73A | Partial: only descriptor initialization B7C725..B7C737; base constructor/profile store B7C710..B7C724 external |
| B7C770 | ECX light, direct destructor | Partial: B7C79C..B7C7BD unlink/resize/free only; profile/EH setup B7C770..B7C79B and Light base/EH tail B7C7C0 onward external |

The B7C710 assembly establishes the +1E0/+1E4/+1E8 producer and corrects the
older analysis-only claim of RET0: its actual name argument is followed by
RET4. Its initialization fragment requires the base construction and current
profile phase to have happened already. It creates neither a default light,
a point-light reference count, nor a pool slot. In particular, the existing
directional-light `NativeLightTailStorage.direction_1e0` is a different derived
layout and cannot be used as point-light backlink storage or kept as an active
direction view over these bytes.

`NativePointLightLinksBinding` borrows one live actual light, checks a minimum
1ECh extent and exact descriptor address identity+1E0, and changes no bytes.
The runtime holds only this association and backing-allocation provenance.
Node hierarchy/scene lookup and owner references remain in their existing
runtimes. No second node map or native-array mirror exists. Bindings must stay
live until all linked nodes are removed; a borrowed actual light may also be
unlinked by B7C160 before the light owner retires it.

New backing uses the existing real `singleton_lifetime_allocate/free` CRT
service. Native B6E500 and 59E5E0 both call operator_new at BF55BE, a thunk to
BF681B, and free at BF6989. Their cdecl cleanup is ADD ESP,4 immediately after
each call. Both copy the live count, free the old backing, then publish pointer
and capacity. Every newly allocated backing is recorded by its matching
runtime. Growth checks old-pointer provenance before allocating; only this
runtime's recorded backing may be freed. Foreign arrays can be borrowed/read
and edited within their supplied valid extent; foreign growth and destructor
ownership are explicit boundaries. No foreign-memory adoption is provided.
The runtime destructor requires callers to complete native cleanup and unbind
their lights; it does not silently dispose live native owners.

B6F150 captures each source light before destination reserve, appends the raw
identity, increments destination count, then appends destination's raw address
to the bound light. Counts and slots reload through the loop. Zero and signed
negative source counts retain the native skip behavior. B6EED0 implements the
same append ordering and supplies a concrete producer for actual lists.
Duplicates are retained. No retain/release is added. Allocation, missing owner,
malformed extent and overflow diagnostics are not native exception equivalence:
an exception after a forward append leaves that append in place and must not
be treated as a completed clone or restarted operation.

The B6F310 adapters for Model, Camera, Group, PlainNode and DirectionalLight
now remove from each physical light array using their actual node identity.
The shared release loop's callback removes the current backlink directly;
the diagnostic owner has its own vector callback. First-match removal swaps
the last pointer and leaves its stale slot, backing and capacity unchanged.
Node logical release then shrinks +168 to zero. Node B6F440 destruction frees
that same forward backing through its runtime. The constructor unwind only
ever owns its initialized empty array; a string provider may not replace this
backing during node construction, and foreign pointers are never CRT-freed.

The B7C770 fragment first calls the complete B7C160 pass: each current raw node
loses one matching forward light pointer, then reverse count shrinks to zero.
The destructor repeats resize0, frees reverse backing, and leaves the native
pointer/capacity words unchanged. Unbind the descriptor before the outer owner
ends its lifetime. This fragment does not perform Light base destruction or
return the point-light slot. Ghidra falsely ends B7C770 at B7C7BC because free
is marked no-return; disk bytes establish ADD ESP,4 at B7C7BD and the later
Light base call at B7C7CA. No out-of-body call is attributed to its live body.

Verification: all ten affected translation units compiled with MSVC Win32
`/std:c++17 /W4 /WX /O2 /MD /EHsc /fp:strict`. A single local executable linked
the real CRT allocator and checked duplicate raw links, positive clone copying,
swap-last removal, unlink/free ordering, unchanged freed descriptors, foreign
growth rejection, and canaries around actual+1E0. It passed. This is a physical
descriptor fixture, not a fully constructed native point light or a game run.
The ordinary build configured successfully but failed before C++ compilation
with MSB3191 access denied creating the Lua/zlib tlog directories under this
worktree's build/win32. The integrator registers the new source and runs the
combined build. No CMake edit, broad test suite, native ABI replacement,
native-byte differential result, gameplay or visual claim is included.

## 2026-09-12 timed and clip ownership integration

The combined Win32 library passes the physical-array fixture using raw
identities, duplicate clone entries, swap-last removal, unlink/free, foreign
growth rejection and +1E0 canaries. The Ghidra B7C770 body has been repaired
through RET B7C7DF, including its B7C7CA base Light destructor call; this does
not expand the C++ implementation beyond the backlink phase. Before-values and
repair events are retained in reports/orch5_point_light_flow_repair.json and
reports/orch5_point_light_body_definition.json. See the batch report for hashes.
