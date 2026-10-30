# Native Lua variant retirement immutable Source review

Candidate `17b77e69a79d82a5b89841edb23c2046cfe86cd1`, based on
`03ef58daf98d9dff844e8a4039be830868d429b7`, passes independent review for its
**three complete normal Source bodies and qualified whole-range composition**.
No correctness defect was found within that declared domain. Whole failure
behavior remains held: the new C++ catch paths do not perform the Native
state-0 name/key cleanup effects. This is a known implementation boundary,
not merely an unverified Native frame ABI. Common build/link and any Source
registration remain the primary integrator's separate gates.

The reviewed candidate changes exactly its header, source, document and report.
This review changes only this document and
`reports/cc16_native_lua_variant_retirement_review.json`. Read-only inputs were
immutable readiness `83a63b4df1aa5962208bfb232d00da331ced9c45` and independent
design `b76e5230bf4918d2709ad4d57f263e36aa22638c`. Their complete documents,
the complete candidate header/source/document, relevant real provider
definitions, Native bytes, all emitted code and relevant EH data were inspected.

## Exceptional cleanup is not supplied

| Native action | Native meaning while state 0 is armed | New Source failure behavior |
| --- | --- | --- |
| `00C970D0 ->0041DD20(saved V+0Ch)` | Release CURRENT name after a failure from variant clear. | Destruction retains its frame and rethrows; it does not run this release. |
| `00C830A0 ->006EE020(K)` | Release CURRENT key through the rewritten node-argument home after a failure from value destruction. | Subtree retains its frame and rethrows; it does not run this release. |

Native unwind maps `00DC9464` and `00DB10B8` explicitly encode state0 to -1
with those actions. In contrast, the compiled new Source EH tables have two
unwind entries with zero action pointers. Their catch handlers only call
`retain_cpp_failure`, then rethrow through `__CxxThrowException`: clear offset
`334`, destructor `147`, subtree `1C1`. No name/key cleanup helper is imported
or called from those paths. The metadata `cleanup_state`, homes and sites are
diagnostic facts; they are not performed cleanup actions.

The header and candidate document explicitly disclaim automatic raw cleanup
and equivalent action-frame delivery, so their normal-only claim is truthful.
Wording such as "FH3 unproved" must not be used on its own to describe this
gap: the Source cleanup effects above are known to be absent. The existing
broad Host implementations and their callback/C++ escape policies are unchanged.

Before whole failure equivalence can be admitted, a separate implementation
must deliver the actual armed name/key effects with the correct current homes,
nested order and demotion boundaries, using the same retained real services.
It must establish entered/completed action custody and the disposition of a
further failure without replaying consumed operations or inventing a new
termination/rollback policy. Original FH3 action-frame, SEH and foreign nonlocal
transfer compatibility remain additional gates. Retained diagnostic state
alone neither closes this work nor permits abandoning actual resources/providers.

## Normal schedules and metadata

The three complete Native envelopes are clear `00886920..008869C7` (168B),
destructor `00884AF0..00884B54` (101B), and subtree
`006EE890..006EE925` (150B): 419B total. The 201B selected `006EE960` range
span supplies evidence for its reached whole-range contract, without claiming
a complete vendor function or general erase/copy behavior.

Clear captures the tag once and accepts all DWORD tags. Only nonnull tag1/tag5
payloads are cleared after retiring their captured allocation. Unknown tags
and null selected payloads still write tag=-1, without an added payload clear.
Tag1 captures buffer before conditional late length; it gets the actual pool
for every nonnull return before freeing the captured descriptor.

Tag5 captures map M independently of retained variant V. The range metadata
frame is prepared before endpoint capture. The actual caller builds
`{M,head.left}` / `{M,head}`; owner/null predicates are discharged by those
actual nonnull equal identities. Current begin/end are still revalidated.
Between endpoint capture, these predicates and current root capture, emitted
clear offsets `D9..17B` contain zero CALLs. Range metadata enter is at `A0`;
the actual subtree call is at `183`. No fallible metadata work, arbitrary
callback or game write is inserted into that interval.

After subtree completion, four distinct current M.head reads at Source offsets
`1C7,1D7,1ED,1FD` preserve parent reset, count0 then left reset, right reset,
and current-left capture/output owner then node. The output is an independent
8B Source pair. Clear then reloads current M.head for sentinel free, zeros
captured M+4/+8 before freeing captured M, and finally clears retained V+8
and writes tag=-1. It never rereads V.payload to select a replacement map.
Map+0 and count do not admit traversal.

Subtree accepts every nonzero nil byte. It recurses right, captures CURRENT
left only on return, records K=N+Ch and the overwritten argument-home VALUE,
then destroys value N+1Ch. It captures CURRENT key buffer after value return,
demotes before its conditional late length/pool return, frees captured N, then
reads captured LEFT's current nil byte. Destruction stamps numerical D0E6F4,
arms name state0 before clear, and captures CURRENT name only after clear
completion; demotion precedes its conditional late length/pool return. Name
and key headers are not cleared; the destructor does not free its receiver.

All four nested non-completion edges return before later Native reads/free/
reset/success: clear to subtree, destructor to clear, subtree to right subtree,
and subtree to value destructor. Insufficient metadata and a failed range
predicate are explicit Source non-completions. Active frames remain retained.
Real escaping C++ exceptions remain exceptions and are never converted to a
status. Operation.result is only a nested diagnostic while active_depth is
nonzero; only an API's returned completed value proves that entry completed.

Caller-owned typed frame backing must cover the actual maximum active call
chain, including nested maps, and remain independently live/disjoint through
failure disposition. Frame/Operation/Pair are 120B/24B/8B on Win32. No heap
journal, count/depth policy, history/address blacklist, rollback, retry or
terminating operation destructor was introduced. Metadata and real services
are caller preconditions, not runtime authentication. Reentry uses an independent
operation/span; concurrent mutation in the capture/predicate interval is outside
the qualified caller contract.

## Real providers and independent compiler evidence

Every nested operation forwards the SAME NativeStringRawPoolContext reference:
actual pool AA8, raw manager AA0 and live small-return gate AA4. The direct
provider overload is the real raw-manager 419CC0, followed by actual BD1510.
Large and disabled-small returns still resolve the getter. Descriptor/map/node
free is the real singleton_lifetime_free (`std::free`) in the matching
singleton_lifetime_allocate malloc/new-handler Source domain. Genuine live
backing, matching allocation provenance and the pool's qualified raw deletion
domain must remain live. Numerical D0E6F4 and matching sizes prove none of them.

Both broad Host files and all 17 declared dependencies match the frozen base;
all 26 readiness pins also verify. All five author artifacts match their recorded
digests. The preserved installed PE SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`;
all 19 readiness blocks (1,236B), including every selected candidate block,
match a fresh independent PE mapping. Every selected code byte also decodes
completely. No live Ghidra query/write or target execution was used.

Independent MSVC 19.51.36244.0 Win32 compile-only verification passed with
`/c /std:c++17 /O2 /Ob2 /DNDEBUG /MD /EHsc /W4 /WX /fp:strict /FAs`.
The 34-section COFF contains 16 code sections, 2,423B, 723 decoded instructions
and 42 code relocations. Every code byte matches the retained object; all
relocation targets and primary symbol values match after normalizing only the
compiler's local anonymous-namespace discriminator. Public bodies are
849B/356B/478B. EH data is byte-identical. Full object hashes differ with the
compilation location/timestamp, debug records and checksums; this is not a
claim of full-object identity or Original binary equivalence.

Only raw419CC0, BD1510 and singleton_lifetime_free are unresolved application
services; the other imports are MSVC EH/security runtime. The digest-verified
existing Core archive positively defines all three application symbols. This
is provider existence evidence, not a fresh whole-source-to-archive attestation.

No tests, full build, executable link, probe, body, DLL or game was run. No
CMake/guard/caller/ledger/Ghidra/install change was made. There is no actual
producer, queue/payload, resize/copy, current-vslot/scalar, CF/D0 callable
authentication, Native ABI/private-frame/FH3/SEH, Game or runtime adoption.
Unique Original-byte, vendor, duplicate-body, ABI and activation credit is zero.
