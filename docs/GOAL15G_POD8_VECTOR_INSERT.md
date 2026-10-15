# Complete raw POD8 count insertion

Addresses: `00420F80` (owned). Descriptive names are hypotheses.

The complete Source entry reconstructs the growth and both in-place branches,
including the two recovered catch operations. It consumes actual raw header
storage and the existing complete range, allocation, failure and free providers.
It does not replace the operation with a host vector or delegate successful
branches to callbacks. Existing semantic draft-layer/CC13 interfaces are retained.

| Native entry | Inclusive physical span | Coverage | Original ABI |
| --- | --- | --- | --- |
| `00420F80` | `00420F80..004211C0`, 577B/225 instructions, including embedded catches | complete normal operations and both recovered catch operations in current Source C++ EH | ECX header, four stack DWORDs, `RET 10h`; no semantic EAX result established |

Source is `insert_native_pod8_vector_00420f80(owner, unused_edx,
ignored_iterator_owner, position, count, value)` in
`include/bsp/native_pod8_vector_insert.hpp` and
`src/native_pod8_vector_insert.cpp`. Its unused EDX adapter is not an Original
argument. Original stack slots are unread iterator-owner metadata, position,
uint32 count, raw8 value pointer. Header +0 is unread and +4/+8/+C hold
first/last/end. No recovered C++ template, class identity or numeric value type
is asserted by this storage contract.

## Captures and normal schedules

Native first reads value word1, then word0; it captures first and computes
capacity before checking count. Count0 therefore still consumes both raw value
words and the reached header fields. The local pair uses integer words,
including arbitrary floating-point encodings, without floating-point loads.
Counts use modulo32 subtraction, SAR3 bits and unsigned comparisons. Pointer
increments, count*8, growth and endpoint publication also wrap modulo32.
The length gate compares count to `1FFFFFFF - size`, retaining that subtraction.

Growth retains the initial capacity and first capture while reloading last for
each reached size calculation. Candidate capacity is capacity plus its logical
half unless `1FFFFFFF-half < capacity`, which selects zero. If too small, it
recomputes required size and uses that. Allocation occurs before state0.
After allocation it reloads position and CURRENT first, performs prefix copy,
captured-value fill and suffix copy using CURRENT last. It preserves count
reloads at calls, the reused value slot holding allocation, and the byte-only
position-slot tag write. It then captures CURRENT old first/size plus current
count, conditionally frees old first, reloads the saved allocation AFTER free,
and publishes end, last, first in that order. State0 spans the whole operation
after allocation, including old free and publication; no old-buffer rollback or
POD element destruction is added.

When suffix<count, the initial suffix copy occurs before state2. The body
reloads current last/count for excess fill, arms state2, fills, adds saved
count*8 to CURRENT last, reloads that endpoint, and assigns the earlier gap.
When suffix>=count, it saves count*8 and split in the actual argument cells,
copies the final count records to captured old last, reloads split, publishes
last BEFORE backward copy, moves the middle backward and assigns the gap.
Both cdecl argument sets are cleaned together by Native `ADD ESP,18h`;
the optimized Source currently emits that same cleanup. This latter branch has
no insertion catch. There is no added validation or normalization for malformed
headers/endpoints. Every reached access must be backed and equality loops must
terminate under their literal +/-8 schedule.

## Native direct calls and complete dependencies

Call-site ownership below was checked against live containing bodies.
The catch rows use the separate embedded handler's live containing body.
The earlier nearest-start snapshot graph omits some in-place edges and can
misattribute them to that embedded catch; live boundaries/listing control here.

| Containing entry | Site | Target | Consumed contract / cleanup |
| --- | --- | --- | --- |
| `00420F80` | `00420FEA` | `004203A0` | no arguments; owning length throw, no normal return |
| `00420F80` | `0042104B` | `00413390` | ECX grown count, EDX zero/unconsumed; plain RET, EAX allocation |
| `00420F80` | `00421070` | `00419960` | ECX current first, EDX position; allocation/owner/current count/tag, RET10h |
| `00420F80` | `00421080` | `0041E3C0` | ECX owner metadata; prefix endpoint/count/local pair, RET0Ch |
| `00420F80` | `00421098` | `00419960` | ECX captured position, EDX current last; fill endpoint/owner/current count/tag, RET10h |
| `00420F80` | `004210BA` | `00BF65AC` | captured current old first; cdecl ADD ESP,4 |
| `00420F80` | `0042111E` | `0041F4E0` | ECX owner; position/captured old last/position+count*8, RET0Ch |
| `00420F80` | `00421141` | `0041E3C0` | ECX owner; current last/excess/local pair, RET0Ch |
| `00420F80` | `00421157` | `0041C680` | position/reloaded old-last boundary/local pair; cdecl ADD ESP,0Ch |
| `00420F80` | `00421188` | `0041F4E0` | ECX owner; split/captured old last/captured old last, RET0Ch |
| `00420F80` | `00421196` | `0041DAC0` | position/reloaded split/captured old last; deferred cdecl cleanup |
| `00420F80` | `004211A6` | `0041C680` | position/position+reloaded count*8/local pair; combined ADD ESP,18h |
| `004210E7` | `004210EB` | `00BF65AC` | CURRENT reused allocation slot; cdecl ADD ESP,4 |
| `004210E7` | `004210F7` | `00BF6885` | zero/zero rethrow arguments, stdcall library transport |

The actual accepted providers are `native_pod8_range.hpp`,
`native_pod8_inplace_range.hpp`, `native_pod8_failure.hpp` and
`singleton_lifetime_free`. The allocator retains its quotient guard and actual
current malloc/new-handler/retry provider. Direct overflow uses the peer's
owning raw12 Source exception transport; length uses the genuine owning
`NativeSingletonVectorLengthError` service. Current allocation failure remains
the provider's `std::bad_alloc` policy. No fake successful allocator, generic
vector substitute or injected per-operation provider is installed.

The one live Native caller is the checked insert-one wrapper. Its listing is
`00421AA0 CALL 00420F80`, with the four original stack words. That wrapper
and append are primary-owned separate packets; no claim of reconstructing
those addresses is made here.

## Native metadata and current Source EH

Native handler `00C5E4C0` loads FuncInfo `00D84844` and jumps to `00BF6B43`.
FuncInfo magic is `19930522`, maxState4, unwind map `00D84824`, two try blocks
at `00D84868`, zero IP entries/IP map/ESType and EHFlags1. All four unwind
entries are toState=-1/action=null. Try ranges are [0,0,1] with handler array
`00D84814` and [2,2,3] with array `00D84804`; both HandlerType records have
adjectives40h/type0/disp0. First catch is `004210E7`, which reads CURRENT
allocation slot, frees it and falls through to the shared rethrow tail.
Second starts at that tail `004210F3` and adds no cleanup. Recovered throw
transport uses the C++ noncontinuable exception template, separately from the
unavailable typed Ghidra no-return flags. No annotation or listing repair was
performed. The approved typed flow-property query failed because bridge inline
scripts are disabled; the error is retained in the ignored evidence directory.

The Source public entry is a 17B naked thunk: it captures `[ESP+4]`, passes that
actual four-cell address plus captured ECX to a private current C++ function,
then `ADD ESP,8; RET10h`. Native argument-cell reads, whole-cell reuse, and the
position's low-byte tag write address those actual cells. They are not copied
private parameters. The private Source EH/call frames are additional frames.
Its local tag reads its own frame preimage after initializing only the low byte;
the upper bytes and frame addresses are not asserted equal to Native ones.

Fixed local volatile entry pointers to the actual concrete copy/fill/free
functions retain the legitimate catch scopes despite their noexcept
declarations. They are not injectable callbacks. Growth catches free the CURRENT
incoming allocation cell then `throw;`; short-tail catches only `throw;`.
The current optimized body is 866B and emits current `__CxxFrameHandler3`,
maxState4, two try ranges [0,0,1]/[2,2,3], four null unwind actions and the
shared current rethrow tail. This structural correspondence does not establish
Native FH3 handler/funclet/stack-spill/cookie, private CRT/heap, RTTI, exception
filter, SEH, double-exception or register-frame identity. No exception was
injected into either inner catch; their operation is statically recovered and
emitted. Raw Native whole-body byte equivalence is not claimed.

## Verification and integration

Worker strict compile PASS: MSVC x86 `/std:c++17 /EHsc /MD /W4 /WX
/permissive- /fp:strict /O2`. Ignored `local/goal15g_pod8_insert/compile.cmd`
builds the object/listing. `verify_object.py [optional_registered_object]`
checks the actual-cell thunk, all eight actual provider identities, initial
word1/word0/count0 order, position-byte write, current-last DWORD ADD,
end/last/first stores, cdecl18h cleanup and complete current EH maps/catches.
It passes on the worker object. The live listing has 225 instructions/zero
gaps; direct-call report verification PASSes all twelve normal and two
separately owned catch sites plus the one caller row (15 total/zero failures).

One ignored real-provider fixture, `local/goal15g_pod8_insert/pod8_insert_probe.cpp`,
strict-compiles and overlay-links with `/MD` and `/MANIFEST:EMBED`. It PASSes:
growth with value aliased to ACTUAL incoming position/count cells, captured raw
pair and byte-only tag/value-slot publication, old free/new capacity; both
in-place branches including a backward move overwriting the original value;
count0 retention; actual maximum admitted allocation failure with the genuine
Source CRT new-handler declining and restored; and owning length-error throw.
The failure occurs before state0 and leaves the header unchanged. Bounded
current `_heapwalk` observations confirm the old growth block is no longer
used and the new block is used. This is Source current-heap/C++ EH evidence,
not an inner-catch fault test, Native differential/ABI/gameplay or global leak
proof. The run command is `local/goal15g_pod8_insert/run_overlay_probe.cmd`.

Primary owns registration of `src/native_pod8_vector_insert.cpp` in `bsp_core`,
registered Win32 build/existing tests, registered-object check and archive-only
fixture, original-prototype annotation/save/export refresh and main integration.
The worker changes no shared CMake/packet registry or Ghidra state. Existing
library names/history and earlier partial semantic interfaces remain intact.
