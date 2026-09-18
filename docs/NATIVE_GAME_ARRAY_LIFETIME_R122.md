# Native game array destruction (R122)

Addresses: `004CAFD0`, `004C1990`, `004C4600`, `004C5860`, `004CB2F0`,
`007F8180`, `004B7EF0`; parent binding at `004DCF90`.

## Result

The game destructor's four array calls now have concrete normal-path defaults.
They destroy actual vector/list storage and participant records, including
references, strings, list nodes and observer registrations. The existing raw
string pool and `NativeObserverLifetime` supply the string and observer
dependencies in one borrowed lifetime domain. No second graph, reference count,
string allocator or successful empty cleanup is introduced.

| Entry | Inclusive end | Bytes | Coverage / ABI |
|---|---|---:|---|
| 004CAFD0 | 004CAFF9 | 42 | Complete normal storage cleanup; ECX header, RET |
| 004C1990 | 004C19D7 | 72 | Complete normal list cleanup; ECX header, RET |
| 004C4600 | 004C4604 | 5 | Complete JMP thunk to 004C1990 |
| 004C5860 | 004C58A7 | 72 | Complete normal participant-pair list cleanup; ECX header, RET |
| 004CB2F0 | 004CB364 | 117 | Complete normal participant destructor; ECX actual118h record, RET |
| 007F8180 | 007F81BF | 64 | Complete normal release of three reference fields; ECX participant, RET |
| 004B7EF0 | 004B7F47 | 88 | Complete normal observer-link base destruction; ECX actual base, RET |

The reconstruction covers 460 native bytes. It exposes explicit source
interfaces, not drop-in binary replacements. CRT reverse iteration is a normal
library adapter for the observed destructor/stride pairs; native CRT/FH3
exception cleanup is not reimplemented. These are raw storage adapters, not
new C++ standard-library container types.

## Recovered behavior

`004CAFD0` conditionally frees current header+4 and then clears +4/+8/+C,
preserving +0. The source retains the unconditional clears after a returning
free callback.

The two list bodies capture the first node, reset the current sentinel's next
and previous links, compare against the current sentinel, then clear count+8.
Each loop iteration captures the next pointer BEFORE freeing the node and
compares it against the CURRENT header+4 afterward. They finally free the
CURRENT sentinel and clear header+4. Count changes made during a free callback
are retained. The node payload is not destructed by these bodies. Constructors
4C8140 and 4D6BA0/4D2920 establish their actual headers and node layouts.

`007F8180` walks participant+104/+108/+10C in ascending order. For each current
nonnull pointer it performs a real atomic decrement at pointee+4, invokes the
captured pointee's CURRENT vslot0 only if zero, and clears the field afterward.
The callback remains a required virtual service: this packet does not infer
the payload's class or manufacture a terminal implementation.

`004CB2F0` stamps CE7794, releases those three references, destroys the three
8h strings at +EC in reverse order, destroys the list at +98, then destroys
the observer-link base at +38. The base stamps CE74FC; if base+14 is nonnull it
passes that captured first endpoint in ECX and the actual base in EDX to
6952A0. It then calls 695870 on the actual base. Existing observer code supplies
pair lookup, dispatch invalidation, reference decrement, detach-all, canonical
edge deletion, recursive locking and callback-array free. The base does not
clear its +14 link.

Strings call the existing actual-header 41DD20 body with the caller's raw pool
publication, shutdown gate and actual01090AA0 manager publication. Every
nonnull string still resolves its pool; large frees and disabled small returns
retain the existing native behavior.

## Parent composition and failures

`NativeGameLifetimeCalls` derives from the minimal array lifetime call service.
Its concrete array default accepts an explicit `NativeGameArrayLifetimeContext`
and retained child operation. It requires that context to use the same call
service. Null or mismatched bindings fail at the reached call.

The four calls in 4DCF90 use separate child operations:

| Site | Game offset | Stride | Count | Destructor |
|---|---:|---:|---:|---|
| 004DD230 | 7134 | C | 4 | 004C4600 |
| 004DD3F5 | 1008 | 118 | 8 | 004CB2F0 |
| 004DD412 | 748 | 118 | 8 | 004CB2F0 |
| 004DD56B | 560 | 10 | 5 | 004CAFD0 |

The normal adapter walks backward over the captured count and stride and
invokes actual element bodies. Source failure retains the completed suffix,
current element, call site and unwind-state observation, and prohibits replay.
The parent cannot acknowledge its failed operation while a child remains
running or failed. Diagnostic acknowledgement only retires bookkeeping after
the caller resolves resources; it performs no rollback or cleanup.

## Ghidra corrections

Five returning-free gaps contained 49 missing instruction bytes: three bytes
in the vector body and 23 bytes in each list body. The list repairs recover
the loop back edge and final header+4 clear. Their saved function bodies also
ended too early; the locked definition tool rebuilt them through their verified
RET instructions, preserving prior recorded metadata. Recreating 4C1990 also
removed its associated 4C4600 thunk, which was restored with the same five-byte
JMP and target. Final live queries report complete ranges and zero remaining
call gaps. Separate flow and body-repair receipts retain these steps.

`CG_vector_deleting_dtor_004cb2f0` was a heuristic name. This is an ordinary
participant destructor with no array-count or scalar-delete flags argument.
Descriptive names remain hypotheses, not recovered symbols.

## Validation and limits

* All 460 code bytes match the live Ghidra program and original PE. Seven copied
  bodies execute in the local differential fixture, with 13 call/jump/global
  relocations and preserved internal control flow.
* 60 paired array cases compare 499 free/terminal observations and 1,306,184
  bytes. Counts 0/1/4/8, null and populated storage, list sizes 0..4, reference
  counts 0/1/2, short/large strings, disabled small returns, repeated observer
  registration, duplicate dispatch slots, additional callback edges and
  callback-driven sentinel/count replacement and replacement of current/next
  reference fields are covered.
* Both lanes use the actual raw string pool, shared raw manager, observer lock,
  observer pair registration and concrete canonical edge deletion. Every
  created domain drains and its pool/lock publications clear. Virtual reference
  payloads are fixture objects with actual atomic counts and terminal frees;
  their behavior does not establish any particular game payload profile.
* Comparisons normalize known allocation addresses, including one-past pointers.
  They include owner bytes, tracked live-node/reference bytes, freed-block
  markers, observer endpoint/dispatch state, string-pool counters and occupied
  return-ring entries. They omit uninitialized pool storage, freed-memory reads
  and OS critical-section internals. String/observer implementations and normal
  CRT iteration are shared dependencies, not newly differential-tested bodies.
* The parent comparator passes 52 cases, 3,537 boundary snapshots and
  141,402,076 matching bytes. Its array calls remain controlled to verify
  parent ordering and new context/child-operation identity; the separate array
  fixture exercises the concrete parent default.
* A source-only vector-array failure retains one completed element and the
  current element, rejects replay and permits explicit diagnostic cleanup.
  The existing four parent failure/replay observations also pass.
* Strict MSVC Win32 build and all three existing CTests pass. No permanent
  tests were added. Local probes and exact source/artifact evidence are archived.

Native FH3/SEH, hardware faults, concurrency, arbitrary private-stack aliases,
drop-in ABI and gameplay remain unproven. The ordinary executable still does
not admit the raw71A0 game owner, so no application run is attributed to these
destructors.

## Follow-up packets

Bind the remaining parent cleanup services, including embedded game state,
race/configuration ownership, resource-manager and DYN teardown, and the other
raw container bodies. Reuse these array defaults and the R120 grid lifetimes.
Then connect the complete game construction/destruction graph to 73D410 and
application teardown before claiming raw game-owner admission.
