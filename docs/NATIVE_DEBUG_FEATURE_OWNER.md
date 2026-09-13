# Actual debug-feature owner and record array

This packet implements **8 complete source bodies / 964 native bytes** for the
34h owner published at 0109DB70. `FileStoreService` was an unsupported earlier
name: debug-menu strings identify the feature list and AI debug-render flag.
`DebugFeatureOwner` remains a descriptive hypothesis, not a recovered symbol.
The implementation does not add archive startup or parse debug configuration.

Source: [native_debug_feature_owner.hpp](../include/bsp/native_debug_feature_owner.hpp)
and [native_debug_feature_owner.cpp](../src/native_debug_feature_owner.cpp).
The [report](../reports/native_debug_feature_owner.json) contains full native
spans/hashes, original ABI, all 26 call rows, source binding identities, cleanup
maps, source hashes and current validation status. The prior
[discovery](NATIVE_FILESTORE_SERVICE_BA_DISCOVERY.md) retains caller and parser
reachability evidence and explains the template-provenance decision.

| Address / complete inclusive end | Bytes | Source behavior |
| --- | ---: | --- |
| 0051F460–0051F51C | 189 | Double-checked raw singleton publication |
| 00BE94F0–00BE955C | 109 | Construct empty actual34h storage |
| 00BE8210–00BE8220 | 17 | Clear publication, install CE3818 base |
| 00BE8350–00BE8350 | 1 | Actual no-effect RET hook |
| 00BE9560–00BE95FB | 156 | Full member destruction and late base reset |
| 00BE9600–00BE961D | 30 | Slot0 scalar deletion, low flags bit0 |
| 00BE8DF0–00BE8F23 | 308 | Reserve 88h records with pooled string copy |
| 00BE8F30–00BE8FC9 | 154 | Resize records and release removed strings |

## Raw storage and shared bindings

`NativeDebugFeatureOwnerContext` borrows the actual 01090AA0 manager publication,
the 0109DB70 owner publication, and the application's existing actual pooled
string storage. It creates no private lifetime domain, pool, or service. The
getter calls the existing raw 415350 and BD0C30 implementations. Construction
uses existing actual 41E870, on the nonnull empty literal represented by CE3A0C.

Owner layout is numeric profile+00; native-string array+04/+08/+0C;
native string+10/+14; feature record array+18/+1C/+20; byte+24; selected index+28;
AI debug-render byte+2C; selected record pointer+30. Constructor writes exactly
those fields and preserves the six padding bytes +25..27 and +2D..2F.

The one established profile slot is **D68B94 +00 -> BE9600**, with unadjusted
owner. Adjacent D68B98 belongs to an unproved neighboring profile and is not
treated as this owner's +04 slot. The primary integrator owns central
`NativeSingletonDeletionBindings` support and actual executable shutdown wiring.

The slow getter captures the first manager's physical critical section at+10,
enters and increments its physical+18 counter, then rechecks publication. It
publishes only after construction returns, obtains the manager again, reloads
the owner for registration, and releases the originally captured section.
Registration failure retains the published owner. Fast return uses its first
captured nonnull value; slow return reloads after section leave. No stronger
locking/publication guarantees are introduced.

## Copying and destruction

Record arrays have `{data, signed count, signed capacity}` headers. A record
is a game 8-byte pooled string followed by 128 flag bytes. These are accepted
as **provisional game-container template specializations**, based on that
physical header and pooled game element behavior. Original template identity
is unknown. Contrary library provenance requires reclassification, not a port
of a standard-library routine.

Reserve clamps capacity to one, allocates using wrapped32 capacity*88h,
deep-copies each string and performs 32 forward DWORD payload transfers. That
last operation preserves the native REP MOVSD propagation for overlapping
payloads. Each loop rereads current count/backing at the native points. Old
strings are released forward, then current backing is freed, then new backing
and requested capacity are published. The sole unwind action calls bare RET
401130: the source intentionally supplies no rollback of new backing or earlier
copies if a later string allocation fails.

Resize captures the growth run length, clears both string words and exactly
80h flag bytes for each new record, and reloads current backing for each entry.
Shrinking decrements the live count before releasing its last string, then
reloads count. Signed comparisons and 32-bit wrapped stride arithmetic remain
explicit; no negative-count guard or overflow policy is invented.

Destruction resizes the feature array to zero, frees current backing, releases
the owner string, resizes the name array to zero, frees its current backing,
then clears publication and writes CE3818. Member headers and selection words
are not proactively cleared. The 427880 cleanup contract is composed from
existing **full 427110** and source free; it does not use the old mesh-only
no-grow cleanup fragment, which would miss a negative-capacity growth path.

Cleanup states follow the native FuncInfo maps recorded in the report:
constructor failure cleans the +04 array then base; getter constructor failure
also frees its captured allocation, while registration failure only releases
the guard; destructor state2 cleans the owner string, state1 the name array,
and state0 the base. Failed feature cleanup is not retried.

## Verification and remaining integration

The discovery's call schema has been corrected from `site/operand` to include
`address/native/function`. There are **24 direct call rows and 2 explicit Win32
import boundaries**. The initial live verifier checks all 24 direct rows and
finds the four expected missing-tail call sites BE95B3/BE95BA/BE95CB/BE95D3.
Ghidra still ends BE9560 at BE9599; primary must repair its returning free flow,
extend through BE95FB, refresh exports, and rerun the verifier. BE8DF0's full
body exists but its decompiler also needs returning-free flow restored. The
worker used complete disk listings and performed no Ghidra writes.

The strict Win32 build and two existing CTests are recorded in the report;
final native/source fixture coordination belongs to primary. No permanent
test was added. Source registration appends only this module's deferred CMake
line in the worker worktree, under primary's explicit registry exception;
the unrelated whole-file lease was reported and was not forced or acquired.

These are source interfaces with explicit contexts, not drop-in native ABI
replacements. Original FH3/SEH, hardware faults, mutable EH spill aliases,
nested cleanup failure, source CRT/Win32 identity and arbitrary virtual
dispatch remain boundaries. `NativeStringStorage::release` is noexcept, so a
throw while lazily recreating its actual pool is not native unwind parity.
The debug-profile parser, startup reachability, integrated profile dispatch,
and gameplay behavior are not validated by this packet.
