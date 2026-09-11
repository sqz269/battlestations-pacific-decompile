# Actual resource-cache consumer discovery

This is a read-only discovery packet for **00B1A4F0**, not a reconstructed cache.
The complete body is `[00B1A4F0,00B1AA2A)`, 1,338 bytes. Its native interface is
ECX = secondary cache, stack = `(name-header, options, retain-new-byte,
load-if-missing-byte)`, EAX = resource, `RET 10h`. The last two arguments occupy
DWORD stack slots but the body tests only their low bytes. These names are
behavioral hypotheses. In particular, virtual `+0Ch` retains; it does not clone.

The companion report pins 55 freshly guarded original spans (5,163 bytes), their
installed-PE comparison, and the selected current source dependencies. The
three BSS observations are PE-loader zero-initialization evidence, not physical
file bytes or proof that live publication slots remain null. All live queries
used `tools/bsp.py ghidra`, whose client verifies `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe` before querying. No source, saved analysis, shared
metadata, game installation or predecessor proof was changed.

## Actual owner and dispatch

`004DE4B0` creates a 1Ch-byte ParticleClock owner, publishes it at `00F8D420`,
and installs primary identity `00CE7D38` and secondary identity `00CE7D24` at
owner+4. It zeroes owner+8/+C/+10/+14 and leaves owner+18 untouched. The native
cache passed to B1A4F0 is **owner+4**, with vector data/count/capacity at cache
+4/+8/+C (the complete 12-byte vector header starts at cache+4).

`00B1B400` (193 bytes, ECX complete owner, stack name/options, RET8) and
`00B1B4D0` (280 bytes, ECX complete owner, stack name, RET4) copy/lowercase a
name and call B1A4F0 at secondary offset+4 with both low-byte flags set to one.
The latter supplies a temporary `Default` options string. The current profile's
resolver and creator ignore that options argument. Existing world/descriptor
callers obtain the complete owner through 4DE4B0; the constructor and wrappers
are pinned, rather than inferring the secondary layout from a typed C++ owner.

| CE7D24 slot | Original word | Meaning and present source state |
| --- | --- | --- |
| +0 | 004DE360 | Secondary deleting adjustor; not called by this consumer |
| +4 | 00B19E40 | Full 80-byte hidden-output name copy; ready, no current source |
| +8 | 00B1B810 | Full 20-byte current factory-registry lookup wrapper; incomplete chain |
| +C | 004DDB20 | Full 21-byte `InterlockedIncrement(resource+4)`, returning that resource |
| +10 | 004DDB40 | Release slot; not called by this consumer |

The base secondary profile CE7D08 contains BF698E pure-virtual words at +4/+8.
It appears during construction/destruction; it is not another supported live
creation provider. Each reached call in B1A4F0 rereads the cache's current table
and selected slot. No replacement dispatch table or generic provider callback
is justified by this discovery.

## Full consumer order and ownership

1. Capture the cache, then call **BECCD0 on the current 0109CF04 platform**,
   before reading/copying the input name or arming any string cleanup. Message
   processing can mutate cache/global state before its first subsequent read.
2. Zero/copy an actual 8-byte requested-name header, then arm its cleanup and
   normalize through the complete actual BEE690 provider. Initial-copy failure
   has no newly armed cleanup. The first scan captures current vector base and
   end once, then examines every alias in each record. It captures each list's
   sentinel and validates against the current sentinel before dereference and
   increment. The apparent validation at B1A593 compares EAX with itself; its
   failure call is unreachable. Equality first requires equal stored lengths,
   then uses empty-length/C-string `_stricmp` semantics.
3. An alias match with a nonnull record+28 is a hit. A matching alias whose
   resource is null ends that record's alias scan and continues to the next
   record. Initialize/arm the resolved header even on the hit path. Every hit
   calls current cache+Ch regardless of the retain-new flag.
4. On a miss, call current cache+4 with a distinct hidden output, requested
   header and options. Copy from **EAX's returned header**, then destroy the
   actual hidden-output temporary and normalize the resolved header. The
   temporary reuses the stack word previously used for the cached resource;
   decompiler variable identities are not an ownership model.
5. If requested/resolved compare unequal (no stored-length equality precheck
   here), capture the current vector base/end again and compare only the first
   alias of each record. An empty list invokes returning BF6713; it is not
   silently skipped. On a match, append the requested alias using full 4CE6F0,
   full 4CE780 and the original link-store order **before** rereading record+28.
   A nonnull resource takes the retain path; null proceeds to creation without
   searching later records.
6. If still missing and load-if-missing is zero, return zero after the two
   string cleanups. Otherwise invoke current cache+8 with resolved name/options.
   Keep its nullable resource result; no resource cleanup is armed.
7. Construct a temporary raw 2Ch record: zero its name; arm name-only cleanup;
   allocate the alias sentinel; store sentinel/count; zero five date words in
   descending order; arm full-record cleanup; copy the resolved name; append
   the resolved alias. The list's reserved word is untouched.
8. Query BDD340 using **current 0109CEEC captured at B1A80D**. Copy five words
   from its returned pointer in interleaved ascending load/store order. The
   existing actual date route separately rereads current 0109CEEC inside its
   physical provider; these are different capture points. Append requested
   alias if the two current names compare unequal.
9. Store the nullable resource at temporary+28 and append through full B1A3C0
   on cache+4, even when resource is null. Only the new-result path tests the
   retain-new byte before optional current+Ch retain. Destroy the temporary
   record, then resolved/requested names, and return the retained/original
   pointer. No body or EH action retains/releases temporary record+28.

The FH3 registration at CBC698 names FuncInfo DF4940 and unwind map DF4964:
state0 -> requested CBC670; state1 -> resolved CBC678 then state0; state2 ->
hidden result CBC680 then state1; state3 -> partial record name CBC688 then
state1; state4 -> full record CBC690/4D45A0 then state1. All five actions are
full 8-byte spans. Normal cleanup captures current data before disarming,
then reads current length and returns through the actual owning pool. A
resource created before sentinel/name/alias/date/vector failure has no release
added by this body. The vector's established orphan-array/prefix behavior also
remains relevant. Neither failure is repaired by inventing an RAII resource.

## Registry and concrete resource constructors

B1B810 ignores incoming ECX/options, calls the current-registry getter B1B730,
then B19E90 with the name. B1B730 is a 200-byte double-checked getter for F8D41C:
allocate 10h, B1AA70 base construction, install D5E59C, publish, register the
current publication through 415350/BD0C30, then return the current slot. Its
actual lifetime/exception adapter remains incomplete.

Registry+4 is a raw tree owner: tree+4 is head, tree+8 count; head+4 is root.
Nodes are 1Ch bytes: left/parent/right +0/+4/+8, actual key length/data +C/+10,
factory pointer +14, color +18, nil byte +19. Owner+4's reserved word is not
initialized by B1AA70. B19C30 allocates the sentinel, after which B1AA70 makes
its links self-referential and sets nil. No reconstructed tree population is
assumed. B19B90 lower-bound and B19D60 find are finite, independently ready.

B19E90 (108 bytes) finds through those helpers, captures the current tree head
before returning-CRT owner validation, compares the returned node against that
capture, validates again against the current iterator-owner head, then invokes
**current factory vtable+4** on node+14. Miss returns zero. This dispatch body is
held for a later reviewed profile/context packet.

The actual water registration chain is BBCB40 -> BBC900 -> BBC5F0/BBC740 ->
B1B730 -> current registry+4 (B1B3A0). BBCB40 is called at original 73D8CC;
the full constructor bodies and that call are pinned. BBC900 creates four
four-byte factory objects keyed `CausticsTextureSource` and
`ShoreWaveTextureSource0/1/2`. The registered pointers first carry D64470/D644AC,
then BBC900 writes their **final D644E8/D644F0 profiles**. Both base/final pairs
select BBC6F0/BBC810 at +4; no function-pointer guess is needed.

Those 80-byte creators allocate 34h and return zero if allocation returns null.
They call C30470 (35 bytes) -> B19980 (22 bytes), then install D64478/D644B4.
B19980 writes CEB130, count+4=1, then D5E554. C30470 writes D79B54, zeroes
+10/+14/+18/+8 in that order, then only byte+1C. Other bytes are preserved.
The final resource profiles have 13 words. Their later virtual destruction and
texture behavior are **not** established by reconstructing these constructors.
BBC6D0/BBC7F0 -> C304A0 remain named but incomplete owner cleanup routes.
The creator FH3 actions CC4B70/CC4BB0 each free the captured allocation through
BF65AC on constructor failure; they do not run a made-up resource destructor.

Population is separately incomplete: B1B3A0 -> B1B2B0 -> B1B0B0, with
B19530/B19890/B1AB40/B1AF90/B1ADA0 and its B19640/B19830/B1AD10 children.
`STL_xlen_throw_B1ADA0` is not merely a throw helper: normal insertion and
balancing remain to reconstruct. Familiar names are not full-source providers.

## BECCD0 policy and current boundary

The full 103-byte BECCD0 captures its incoming platform, drains all thread
messages with PeekMessageA(PM_REMOVE), calls xlive ordinal5030 through C2F1D2,
and TranslateMessage/DispatchMessageA only if unconsumed. It drains WM_QUIT;
there is no loop-exit test or frame callback. It finally calls the complete
424-byte BECB20 on that **captured** platform with loading=true.

Existing `resource_load_events.cpp`, `platform_cursor.cpp`, `platform_services.cpp`
and `xlive_library.cpp` provide the complete typed loop/policy and concrete SDK
pretranslation. BECB20 reaches current F8ABE8/F8BBF4, 4BA6D0, A409F0, BECA40,
A9A140, current input virtual+4 and ShowCursor. PlatformServices binds canonical
typed platform/input/XLive owners. It does not establish an actual 0109CF04
raw-owner adapter. `platform_window.cpp` expressly leaves native BE2AC0/BE2960
publication unmodelled. Its validation throws are source binding boundaries,
not recovered native failures. This packet does not replace them with an
arbitrary ResourceLoadEventHost, no-op pretranslation, or a fixed cached owner.

## Next finite source proposal (not implemented here)

Own eight full entries, **503 bytes**: BBC6F0, BBC810, C30470, B19980, B19E40,
4DDB20, B19B90 and B19D60. Four new files:
`src/native_resource_cache_leaves.cpp`, `include/bsp/native_resource_cache_leaves.hpp`,
`docs/NATIVE_RESOURCE_CACHE_LEAVES.md`, `reports/native_resource_cache_leaves_audit.json`.
Claim CC4B70/CC4BB0 and their FH3 metadata as evidence dependencies only.

Use caller-supplied actual raw name/tree/object storage; ActualNativeStringPoolStorage
for B19E40; existing returning SingletonLifetimeCallbacks CRT boundary for find;
existing singleton_lifetime_allocate/free for the exact 34h allocation/free;
real InterlockedIncrement for retain; current `_stricmp` and complete raw 443D00
for lookup. Preserve B19E40's identity comparison before unconditional output
zeroing, unarmed failure state and all current-header rereads. Find must capture
both output words before either store and reread the head on fallback. Neither
tree construction/population, registry dispatch B19E90/B1B810, resource destruction,
platform binding nor B1A4F0 belongs in that packet. Exact proposed APIs and all
host/CRT/EH limits are in the report. Any fixture should be one focused original
composition, not a broad suite or a claim that original CRT/EH helpers execute.


Primary verified all 42 sealed worker pins, 69 additional report pins, and freshly reread all 55 guarded spans (5,163 bytes). The three BSS spans establish PE virtual zero initialization, not running application globals. The eight-leaf source proposal is approved separately; the registry singleton, population and full consumer remain outside this discovery. Immutable evidence: `local/resource_cache_consumer_discovery_primary/`.
