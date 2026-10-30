# Concrete raw Lua variant retirement cycle

This packet implements the three complete normal bodies in the reached retirement
cycle with real raw pool and malloc/free services. Strict MSVC Win32 object
compilation, Native/PE comparison, full emitted-code decoding and bounded epoch
checks pass. Caller-owned metadata retains partial state without rollback.

Published base: `03ef58daf98d9dff844e8a4039be830868d429b7`.
Explicit read-only readiness input:
`83a63b4df1aa5962208bfb232d00da331ced9c45`, report SHA-256
`0f7bf4bacc0584563fb52c2f79282fe5f0fd4afb8e289a4b13531f514ba4e339`.

Exactly four new files are owned: the narrow header/source, this document and
`reports/cc16_native_lua_variant_retirement.json`. Existing Left55/Seg33b broad
host bodies are unchanged, preserving their callback/unwind behavior.
No CMake, ledger, Ghidra, feature guard, caller, installation or peer-tree edit
was made. Root owns independent review, common build/link and source admission.

## Complete body and context boundary

| Native normal body | Bytes / operations | New Source operation |
| --- | --- | --- |
| `00886920..008869C7` |168 /59| clear_native_lua_variant_00886920 |
| `00884AF0..00884B54` |101 /29| destruct_native_lua_variant_00884af0 |
| `006EE890..006EE925` |150 /50| destroy_native_lua_variant_subtree_006ee890 |

All 419 owned native bytes match the fresh current PE and the earlier
target-verified live blocks. The installed PE SHA-256 remains
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Source work used those frozen live captures plus current disk checks; no new
Ghidra query, import, edit or save was performed.

The APIs take genuine live raw variant20/map12/node52/string-descriptor8 backing
and the SAME `NativeStringRawPoolContext&` through the entire cycle. Its actual
pool01090AA8, manager01090AA0, small-return gate01090AA4 and qualified raw deletion
domain must belong to the real retained producer. Module code creates none of
those cells, owners, constructors or callback tables.

Every nonnull buffer return calls the actual raw manager/publication overload
of419CC0 and then actual BD1510, including large/disabled-small cases.
Descriptor/map/node frees use real singleton_lifetime_free in the matching
singleton_lifetime_allocate Source malloc/new-handler domain. There is no
abstract broad-host subclass, arbitrary callback argument, std::map projection,
Source-native semantic cast or empty/default success.

These are ordinary C++ Result-returning interfaces with independent metadata,
not Native ECX/stack/result/flags/fault/FH3/SEH ABI replacements. D0E6F4 is stored
as numerical identity data only. No receiver, heap or callable table is
authenticated by that value.

The existing labelled logic and necessary library composition earn zero new
unique Original-body/byte, vendor-body, duplicate-body, ABI or activation credit.

## Normal schedules

Clear captures the tag DWORD once and accepts every value. Only tag1/tag5 capture
a selected payload pointer. Other tags and null selected payloads still write
tag=-1 without an added payload clear.

For tag1, it captures the real descriptor, then its current buffer and conditional
late length. Actual pool return precedes free of the captured descriptor.
Retained variant+8 is cleared only afterward, followed by tag=-1.

For tag5, captured mapM remains the receiver throughout callbacks. The qualified
erase-all operation completes first. Clear then reloads current M.head for
sentinel free, clears captured M+4/M+8 **before** freeing captured M, and only
afterward clears retained variant+8 and stores tag=-1. It does not read map+0,
stamp a profile or clear the name.

Destruction stamps numerical D0E6F4, retains name_home=V+Ch and arms cleanup_state0
before clear. A non-completed clear returns immediately before the name read.
After complete clear, it captures the current name bufferV+10h, demotes state
to-1, and only for nonnull buffer reads late lengthV+Ch and calls the actual
pool services. It does not clear the name or free the receiver.

Subtree cleanup accepts **any nonzero** nil BYTE31h as its stopping condition.
It never admits traversal from map+0 or count. For a zero byte it:

1. Captures right and recursively retires it. Non-completion propagates before
   any later left read.
2. Captures current left only after right return. It records keyK=N+Ch and
   rewrites the retained node_argument_home VALUE from incoming node toK,
   then arms state0 before value destruction atN+1Ch.
3. Propagates a non-completed value operation before reading/returning the key
   or freeing N.
4. Captures current key bufferK+8, demotes state to-1, and conditionally reads
   late lengthK+4 before actual pool return.
5. Frees captured N. Only afterward reads captured LEFT's current nil byte,
   then advances current node toLEFT.

No null/cycle fix, tag-only slice, added name/key clear or raw object-history
policy changes this normal schedule.

## Qualified whole-range library operation

The in-house clear caller captures nonnull M, first ownerM/node=head.left and
last ownerM/node=head. Those exact caller identities discharge the Native
owner/null validation conditions. Source still revalidates current begin/end
from current head before taking the whole-range operation.

The range frame is prepared **before** endpoint capture88693F. It uses
caller-owned backing; there is no journal allocation, vector growth, logging,
virtual adapter, exception construction or helper callback between endpoint
capture, current begin/end predicates and root capture. Only raw reads,
predicates and pure retained-metadata/Source ABI stores occur.

If begin/end cannot be revalidated, Source returns
`whole_range_domain_mismatch` with captured/current metadata, before traversal,
reset or free. This is explicit non-completion outside the admitted stable,
nonaliasing canonical domain. No partial-erase callback is guessed, skipped
into success or replaced by a fabricated empty map.

After successful predicates, current M.head/current parent-root are reloaded
and the real subtree operation is called. After its normal completion the code
performs all four distinct current-head reads:

| Read | Ordered effects |
| --- | --- |
| H1=current M.head | H1.parent=H1 |
| H2=current M.head | M.count=0, then H2.left=H2 |
| H3=current M.head | H3.right=H3 |
| H4=current M.head | Capture H4.left; write output ownerM then node |

The output is an actual 8-byte Source pair in independent parent-frame backing.
Clear subsequently reloads current M.head again for sentinel free.
Header+0 remains untouched.

This is a concrete qualified erase-all library contract, **not** a complete
Source006EE960 function. Generic validation callbacks, iterator/partial erase,
6EE5D0 and vendor copy bodies are not reconstructed or credited.

## Metadata, rejection and failure custody

Frame span size is a genuine sufficient-backing precondition supplied by the
caller for the actual maximum active Source call chain. No capacity is inferred
from count and no fixed maximum is introduced. Frame, operation, context and
provider backing stay independently live and disjoint from retired objects.
External callbacks that reenter retirement use an independent operation/span
and do not mutate active metadata.

A returned `completed` means that API entry executed its entire normal body.
The operation's result field is only the latest nested diagnostic while
active_depth is nonzero. All four direct nested status edges test and immediately
propagate non-completion before later native reads/free/reset effects.

Insufficient metadata returns `metadata_backing_required`; an unqualified range
returns `whole_range_domain_mismatch`. These Source-only stops are outside the
admitted valid/sufficient-span domain. Active frames are retained, not popped,
cleaned, retried or presented as completed. No pointer/address/old-operation
blacklist rejects a genuine current object.

Escaping actual pool C++ exceptions mark retained active frames and rethrow the
same exception. Existing concrete noexcept/CRT/manager boundaries remain intact.
No exception is converted to a metadata status. No RAII raw rollback, guessed
termination, failed-state cleanup or automatic replay was introduced.
Frame/Operation destructors are statically required trivial.

The metadata stores current/captured variant, payload, node, right/left, key,
name, argument-home VALUE, buffer/length, pool, endpoint/root/head/output,
provider and native-site/cleanup-state epochs. Its home is not a claimed Native
stack address. Original name/key EH action targets and demotions are preserved
as diagnostic facts, while their actual FH3 interpreter/action-frame delivery
remains unproved.

SEH and foreign nonlocal transfer need not pass through the C++ catches.
A partially consumed graph can remain with active metadata. Retain its actual
resources and providers through disposition; public resets or surviving process
cells do not establish successful retirement or extend service lifetimes.

## Static proof and admission limits

Final compilation passed MSVC19.51.36244.0 Win32:
`/c /std:c++17 /O2 /Ob2 /DNDEBUG /MD /EHsc /W4 /WX /fp:strict /FAs`.
Seventeen relevant dependency pins, including both broad-host implementation
files, match the frozen base.

Independent COFF decoding accounts for all 16 code sections / 2,423 bytes.
The three public sections are 849,356,478 bytes. The only unresolved service
symbols are actual raw419CC0, BD1510 and singleton_lifetime_free; the existing
Core archive positively defines all three. Other unresolved references are
the real MSVC EH/security runtime. No allocation/vector/log/arbitrary-callback
import exists.

The critical clear interval is Source offsetD9 through17B, ending before marker
6EE9AA at17C: **zero CALL instructions**. Range-frame enter/init is earlier
atA0; actual subtree call is183. Four post-callback current-head reads appear
at1C7,1D7,1ED,1FD, with ordered reset/output stores independently checked.
Each nested status test is recorded in the report before its later body effects.
The metadata-enter helper itself has no calls.

The report pins the full object, complete decoded public bodies, code-section
census, Native bytes, field/phase contracts, exact imports and hashed ignored
artifacts under `local/native_lua_variant_retirement/`.
The whole emitted object is preserved there for independent review.

No full build, link, test, probe, target body, game or DLL was executed.
Root common build/link/source admission remains pending. Full host, vector,
reserve/copy, payload/queue, scalar dispatch, CF/D0 authentication and producer
adoption remain outside this packet.
