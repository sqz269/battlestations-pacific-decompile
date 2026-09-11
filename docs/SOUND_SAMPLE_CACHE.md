# Shared sound sample cache

Addresses: 00A83FD0, 00A85440, 00A83E80, 00A82C00, 0043E9A0.
STL contracts: 00A81AD0, 00A81B80, 00A83EB0, 00A83C90, 00A839C0,
00A83700, 00A837C0. Names are hypotheses. Reports contain spans/hashes/ABI
in `sound_sample_cache.json` and `gameplay_effect_sound_prototypes.json`.

## Canonical owner and resource boundary

Existing SoundAuxiliaryTreeOwner for[00F8BBE8] now uses pooled NativeString
keys with concrete00443D00 ordering. Its former word14 is a weak raw sample
pointer. Native1Ch nodes have links0/4/8, keyC/10, sample14, color18, nil19,
untouched padding. Node construction deep-copies the key and copies the
pointer without retaining. Explicit count0C preserves native count timing
around callbacks. The existing empty-owner factory remains valid.

Current tableD5B460 slot4 is00A85440 and slot8 is00A83E80, verified in read-only
image data. Fallback storageE17BF0/F8BBEC is writable .data, observed zero in
the analysis image. F8BBEC lies beyond the section's raw data and is initially
zero-filled by the PE loader; it has no raw file byte to compare. Both remain
live storage inputs. These observations do not establish their runtime values.
Factory00A85440 allocates actual7Ch
and invokes required00A84D70. ECX is unused; stack name, EAX constructor
result, RET4. Null allocation returnsnull. The constructor's metadata/VFS/
resource-loading body remains open. Unknown current tables require services.

std::map supplies library mechanics with an explicit native count. Valid
serialized trees, live iterators and nonnull samples on hits are preconditions.
Native allocation failures, debug checks, layout and SEH are not reproduced.
Pool callbacks see recovered copy/release ordering. Explicit clear releases
pooled keys; implicit C++ destruction alone is not native owner teardown.

## Acquire00A83FD0

ECX cache, stack output-slot/name, EAX output-slot, RET8. First copy the name
into a pooled creator argument. Scan in tree order using the current original
name. Equal lengths compare through CRT casefold; empty names match without
buffer reads. For unequal lengths, choose the longer whole string, require a
slash/backslash before its shorter-length suffix, and compare that suffix
byte-for-byte. This is symmetric, but is neither general basename equivalence
nor case-insensitive matching for differing lengths.

0043E9A0 performs the suffix check: ECX candidate, stack pattern/start,
AL boolean, RET8. Nonnull candidate data and unsigned start<=length are
required. Null pattern data uses current storageE17BF0. Compare until pattern NUL;
reaching that NUL is success. The helper has no casefold operation.

A hit atomically increments sample+4, then reloads the node's current value
into output. A miss invokes current slot4 with the first copy. After creation,
copy the current original input into a second temporary, then a pair
temporary. Unique insertion deep-copies the pair key into its node and stores
the raw pointer without retaining. A duplicate leaves the old node/value
untouched. The caller ignores insertion status and returns the fresh pointer;
it neither releases that duplicate loser nor retains the old map pointer.

Destroy the pair with captured data/current length, then the second copy with
current data/captured length. Only then publish the fresh pointer. Destroy
the first creator-name copy after publication. No old output is released.

## Erase, clear and library evidence

Missing00A83E80 was defined from33 verified bytes through RET4 atA83E9E. It
unpacks an iterator into tree+4 erase00A839C0. The rebuild unlinks its node,
releases the key, frees the node, then decrements current count0C only if
nonzero. A callback's zero stays zero. The sample is never released. Sentinel
erase retains the native out_of_range domain.

00A82C00 reloads begin and current slot8 until empty, then writes
DWORD00F8BBE4=0. It neither frees the head nor releases samples. Its old STL
instantiation label was incorrect: this is a game virtual loop/global reset.
Full auxiliary-owner destructor00A886C0 remains a follow-up.

00A83EB0 is unique insertion with native string ordering and predecessor.
00A83C90 throws length_error only at count>=15555554; its normal path
allocates/copies, increments count, links and rebalances. 00A839C0 throws
out_of_range only for sentinel erasure; normally it obtains successor,
unlinks/rebalances, releases/frees, conditionally decrements and returns
successor. Their whole-function throw labels were retired. The two checked
iterator prototypes now expose mutable ECX, preventing false unchanged-node
assumptions in callers. The library remains linked rather than reimplemented.

The58-byte A839C0 tail through RET0C atA83C89 is decoded, but the stored body
still ends atA83C51. Prototype prior values and retired tag/bookmark records
are saved and verified. No shared Ghidra script gate was changed.

## Validation

The real-Lua Sound fixture covers mutable fallback storage, same-length casefold hits, exact suffix
sharing, differing-length case mismatch misses, repeated retained references,
weak clear leaving a sample alive, and a key-release callback that sees the
old native count before resetting it to0. Constructor resources are controlled
fixture bindings, not metadata/VFS/FMOD/playback validation. Build/fixture
hashes and combined integration are recorded separately in the reports.

## Correction from docs/SOUND_SAMPLE.md

00A84D70 now has a concrete sample constructor, event-group/parameter loading,
and destructor path. `SoundSampleRuntime` supplies that constructor to the
existing cache factory and concrete final release. Installed FSB and FEV
acquisition/release passed through VFS and FMOD. The earlier fixture's controlled
constructor remains its original validation provenance; audible playback and
whole-game validation are still open. Auxiliary-owner teardown remains open.
