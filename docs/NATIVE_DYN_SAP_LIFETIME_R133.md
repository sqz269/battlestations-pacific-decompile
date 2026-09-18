# Native SAP broad-phase lifetime R133

Addresses: 00C4C380, 004043D0, 004043F0, 0040B590, 00C4BC50, 00C40D80.
Physics bindings: 00C4DAA0, 00C32250. Read-only library reference: 00BF7C6E.

R133 supplies the actual SAPBroadPhaseManager2 remove and scalar teardown methods
previously controlled in the R132 fixture. Physics defaults now call these concrete
source implementations using the same allocator as construction. The identified
D7A160 vtable has C4C380 in slot1 and 4043D0 in slot7. No partial runtime table is created.

## Reconstructed contracts

| Body | Bytes | Original inputs | Behavior |
| --- | ---: | --- | --- |
| C4C380 | 452 | ECX manager, stack proxy; RET4 | Recycle inserted endpoints/pairs or shift pending queue; free pair vector; recycle static/dynamic proxy |
| C4BC50 | 128 | EBX manager, stack proxy; RET4 | Remove each pair from peer vector, unlink and recycle pair |
| C40D80 | 147 | ESI proxy, stack index; RET4 | Swap selected pair pointer with last and decrement count |
| 40B590 | 56 | ECX endpoint pool; RET | Free pages and pointer vector, retaining stale fields |
| 4043F0 | 283 | Stack manager; RET4 | Free pending/static/dynamic/pair storage, reverse-destroy endpoint pools, stamp base profile |
| 4043D0 | 31 | ECX manager, stack flags; RET4 | Destroy, optionally free on low bit0, return captured owner |

These are six complete normal bodies totaling **1,097 bytes**. Names are descriptive
hypotheses. In particular, the old `CG_vector_deleting_dtor_004043f0` heuristic was
incorrect: 4043F0 takes only an owner pointer and is the ordinary manager destructor.
The scalar wrapper is 4043D0.

Existing C36F10 initializes the 248h manager, including endpoint pools at4/38/6C,
pair poolA0, dynamic/static proxy poolsD4/188 and pending vector23C/240/244.
Existing C54AA0/C4CDA0 creates actual pooled50h proxy records and their pair vectors.

C4C380 reads inserted byte38. It returns six endpoints in proxy-offset order
20/2C/24/30/28/34. Only the first endpoint publishes its free head before decrementing
the active count. A queued proxy must be present; its suffix shifts left and the
last backing word remains stale. Static byte1C is captured before the pair-vector
free and selects the recycling pool even if a returning callback changes that byte.

C4BC50 uses signed source and peer counts. A missing pair in the peer vector or a
nonpositive peer count skips peer removal but still recycles the pair. Source vector
and count remain stale. C40D80 retains the original unsigned-underflow growth branch;
its valid removal domain requires a nonempty vector and an index below its count.

4043F0 frees pending storage, static proxy pages/vector, dynamic pages/vector and
pair pages/vector. Counts and vectors reload around free callbacks. It then consumes
BF7C6E's normal reverse iteration of three34h endpoint pools, at6C/38/4, through
40B590 and stamps D7A0E4. This is a fixed consumed library contract, not a general
CRT or native SEH implementation. The destructor does not walk active proxy pair
vectors; removing proxies first is necessary for complete nested allocation cleanup.

## Validation

- Strict MSVC Win32 build and all three existing CTests pass.
- 3,256 live Ghidra bytes match the original PE: 1,097 new body bytes, 2,016 existing
  physics/task reference bytes, 75 CRT reference bytes and 68 vtable/profile bytes.
- Thirteen false no-return CALL gaps totaling80 bytes were repaired. Non-CALL
  padding was retained. Complete body bounds and direct CALL sites were checked.
- Twenty native/source cases match **702 observations / 11,209,552 normalized bytes**.
  Eight exercise actual constructed physics worlds, pools, profiles and queued
  proxies through the new default bindings, with zero or one real idle worker.
  Twelve exercise queued ordering, inserted endpoints, shared pairs, reversed peer
  order, both pair endpoint orientations, missing/nonpositive peer search, deletion
  flags0/1/100/101, empty managers and returning-free mutation of the static byte.
- The native fixture executes relocated C4C380/4043D0 and their reconstructed
  dependencies. Its controlled BF7C6E adapter calls relocated40B590 in the exact
  consumed reverse order. Attachment scalar and the existing task service remain
  shared or controlled as described in the report.
- Fifty-two controlled parent cases match3,537 snapshots /141,402,076 bytes, with
  four source failure/replay cases. The physics fixture also retains attachment
  failure, no-replay and missing-context checks.

Pointer identities are normalized; Windows handles only in identified handle
fields. OS critical-section bytes are omitted. As in R132, the zero-worker fixture
sets its otherwise unspecified task-pointer field to null. Fixture-only handle and
diagnostic cleanup occurs after comparison. SAP child pages and vectors now pass
through the compared native/source destructors rather than separate fixture disposal.

## Remaining work

Other SAP vtable methods and attachment scalar bodies remain runtime dependencies.
The underflow growth branch is retained but unexercised. These explicit C++ APIs
do not establish native register ABI, FH3/SEH, allocator-failure behavior, private
stack aliases, concurrency, malformed queues, repeated cleanup, raw-game admission
or gameplay. No ordinary application runtime rerun was used as proof for this packet.

See `reports/native_dyn_sap_lifetime_r133.json` and its flow-repair companion for
body evidence, exact call sites, fixture provenance, saved annotations and archives.

## Correction from docs/NATIVE_DYN_SHAPE_LIFETIME_R134.md

R134 supplies ConvexMeshShape scalar4062C0 through the original body-creation
context and existing408040 pool release. Its actual convex attachment fixture
replaces the controlled scalar for that class and compares36 native/source cases,
790 observations /20,777,328 normalized bytes. R133's sealed controlled-attachment
fixture remains historical evidence. Other shape classes and virtual methods,
native exception ABI, raw-game admission and gameplay remain open.

## Correction from docs/NATIVE_DYN_SAP_PAIRS_R136.md

R136 supplies seven complete normal pair-storage and enumeration bodies (761
bytes), reusing R133's indexed removal and cleanup over actual constructed SAP
storage. Six native/source pairs match 38,523,008 exact bytes, 2,240 allocator
events and 90 full snapshots. The complete SAP table still depends on endpoint
updates and both incremental and >50 pending batch processing paths. These pair
fixtures do not establish complete-world behavior or gameplay.
