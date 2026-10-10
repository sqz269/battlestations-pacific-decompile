# CC12 game free-adapter domain readiness

The existing `singleton_lifetime_free` service is a suitable matching service for
the two named defaults **under an explicit canonical Source allocation contract**.
The concrete correction is ready for primary review: replace `::operator delete(p)`
with `singleton_lifetime_free(p)` in both `NativeGameContainerLifetimeCalls` and
`NativeGameLifetimeCalls`. No new allocator is needed. This packet applies no C++
change and grants no production application or queue binding approval.

The contract admits null or storage allocated by the same canonical Source
`singleton_lifetime_allocate` service. A different allocation domain must supply
the corresponding matching release override at every reached boundary. This is a
Source service decision, not a claim that arbitrary CRT heaps or the original
Native free entry are interchangeable.

## Current evidence boundary

Packet `cc12_game_free_adapter_domain_readiness` owns only this document,
`reports/cc12_game_free_adapter_domain_readiness.json`, and its ignored local
bundle. It performs no Native query/decoding, Ghidra mutation, C++ edit, build,
test, Native execution, or SDK/OS/game execution.

Source is frozen at `f416540a4ffb3e0e1fc2ec5356b140c162889ae9`. The accepted
application-owner worker `6348370331d59e6c15c468af29a9b6e2515cb8d8` and Root
`f3b12e203977efdcc816b4d3f6fc45b8b065eb71` are whole retained dependencies. The
later string-cleanup acceptance is retained separately; the earlier worker's
pending-at-capture record remains unchanged.

All 4,206 Source Git blobs and raw working files are checked. Thirteen earlier
accepted overlays remain identical. Exactly two additional files differ from the
application-owner freeze: `include/bsp/native_wide_string.hpp` and
`src/native_wide_string.cpp`. Their complete bytes match Root
`b7727f1286c325d3bb594d74bc4a22657cda2f9c`'s accepted
`Source3352-native-wide-raw-context` manifest and retained payloads. This is the
current 15-overlay Source epoch; the older frozen Source is not relabeled as
unchanged. Main `d7d1a18af7c88a9b2ae9288606da87cd0b6008fa` had the same Source
tree at capture. No new build credit is taken from that accepted wide-string work.

The bounded free-domain closure retains 182 complete Source files from 34 roots
and 296 quoted include edges. Twelve queries cover the entire 4,206-file corpus,
retaining all 2,223 matches. Thirty-five excerpts are backed by complete Source
files. All eight tracked test files are retained and queried separately. Whole
prior reports are dependency artifacts; their Native bodies, probes and older
proof trees are neither reopened nor replayed.

## Existing allocation and release routes

| Owner/service | Existing allocation | Existing release and implication |
| --- | --- | --- |
| Game container default | `allocate_00BF55BE` calls `singleton_lifetime_allocate({object,n,n})` | `free_00BF65AC` currently calls `::operator delete`; `free_00BF6989` calls `std::free`. The proposed change affects only the former. |
| Game construction/array leaves | `NativeGameArrayCalls::allocate_00BF681B` uses the same canonical service; raw tree/list helpers forward to that virtual provider | These raw blocks require a matching lifetime provider. The parent correction supplies the canonical default for the composed game service. |
| Profile collection default | Both allocation entry adapters use `singleton_lifetime_allocate` | `NativeProfileCollectionCalls::free_00BF65AC` already calls `singleton_lifetime_free`. This is an existing matching implementation. |
| Raw application allocation | `GameNativeGameApplicationFrame` allocates the `0x71A0` game extent canonically | Explicit retained-storage free already uses `singleton_lifetime_free`; scalar deletion requires matching `free_00BF6989`. Both remain unchanged. |

The implementations are at `src/native_game_container_lifetime.cpp:39`,
`src/native_game_array_elements.cpp:52`, `src/native_game_storage_defaults.cpp:9`,
`src/native_game_construction.cpp:58`, `src/native_profile_collections.cpp:12`,
and `src/game_native_game_application_frame.cpp:50`/`:95`.

The existing canonical allocator performs `malloc(request.host_bytes)`, retries
through the new handler, and otherwise throws `bad_alloc`. Its matching free is
already `singleton_lifetime_free(void*) noexcept`, implemented with `std::free`
(`src/singleton_lifetime.cpp:51`). Native and host byte counts remain explicit.
Reusing this service requires no assumption about the implementation of
`::operator delete`, no pointer-origin guessing, and no new storage owner.

The container reserve path already pairs its replacement allocation with
`free_00BF6989` for the old buffer (`src/native_game_container_lifetime.cpp:75`,
`:84`). That path is not a reason to redirect all release functions. The named
`free_00BF65AC` default is also used for raw list nodes, sentinels, SBO heap bytes,
and pointer-block/map cleanup; the default contract must cover every nonnull raw
block routed through it, not just a future state queue.

## Why both defaults are required

`NativeGameLifetimeCalls` declares its own `free_00BF65AC(void*)` overriding the
common virtual method inherited through array, embedded, profile-collection,
and container lifetime bases (`include/bsp/native_game_lifetime.hpp:19`, `:67`).
Its current implementation independently calls `::operator delete`
(`src/native_game_lifetime.cpp:134`). Changing only the container method would
leave calls dispatched through the parent on that implementation, including
profile calls whose base default already uses the canonical free service.

Parent bindings pass `*this` to the raw helpers and require the same call-service
identity in borrowed contexts (`src/native_game_lifetime.cpp:192`, `:234`). This
is shared virtual dispatch, not a second allocator or another queue owner.

`NativeGameCurrentVirtualCalls` is an existing concrete final subclass. It supplies
only `virtual_scalar`, `virtual_terminal`, and `virtual_04`, and inherits both
free defaults. Its contract already requires qualified owner/destruction domains
and explicitly supplies no allocator repair
(`include/bsp/native_game_current_virtual_calls.hpp:13`). Complete Source queries
find exactly five type occurrences, all in its own header/implementation, and no
ordinary production instantiation. The only other occurrences of the two adapter
types outside their own files are that subclass declaration and an eventual
profile-context sharing comment.

The accepted application-owner review independently establishes that ordinary
startup has no composed raw game/application owner. These are dormant Source
defaults. Correcting their declared service domain does not activate them or
prove that every game-owned pointer is ready for destruction.

## Concrete correction proposed for the next packet

1. In `src/native_game_container_lifetime.cpp`, change only the body of
   `NativeGameContainerLifetimeCalls::free_00BF65AC` to
   `singleton_lifetime_free(p);`. The canonical service header is already included.
2. In `src/native_game_lifetime.cpp`, make the same body change and add a direct
   `bsp/singleton_lifetime.hpp` include.
3. Document the canonical raw-storage/default-release contract at the two existing
   header declarations. Explain that custom allocation domains must override their
   matching reached releases and that the parent's method supplies the common
   release default through its relevant base references.

Keep both virtual signatures and their existing ability to propagate exceptions
from overrides; do not add `noexcept`. Keep every call, including explicit null
calls, in its existing order. Do not add guards, destructors, field clears,
rollback, retry, publication changes, or fallback frees. Existing diagnostic
failure/lifetime policies remain intact.

This is a proposal for two function bodies, one direct include, and two adjacent
contract comments. Primary acceptance and a separately scoped implementation
packet are still required; none of those Source edits are included here.

## Domains deliberately outside the correction

The standalone `NativeGameArrayLifetimeCalls` and
`NativeGameEmbeddedLifetimeCalls` defaults continue to use their existing
`::operator delete` implementations. The proposed parent override changes dispatch
only when the actual service is the parent or a subclass inheriting its method.
It does not silently qualify standalone services for canonical-allocated blocks.

`NativeGameGridCpuCalls` has its own `::operator new`/`::operator delete` pairing
for internal buffers (`src/native_game_grid.cpp:165`). Do not change that pair.
The separately supplied outer grid object has an additional allocation obligation:
the raw game constructor allocates its `0x84` storage canonically, while grid scalar
deletion uses the grid CPU release adapter. This review preserves that separate
admission frontier and does not claim the entire grid ownership chain is qualified.

Physics/SAP release methods have distinct names and use explicit memory-context
callbacks (`src/native_game_physics_lifetime.cpp:43`). Class/Lua-prefixed releases,
selected virtual object destructors, ordinary C++ `new`/`delete`, and vector or
`unique_ptr` ownership also remain separate. The change must not become a global
replacement of `operator delete`.

The raw application frame already requires `free_00BF6989` to match its canonical
allocation and marks allocation disposition indeterminate before scalar deletion.
Its retained-storage release and failure acknowledgements are unchanged. There
is no extra allocator/string pool, unresolved forwarder, or new private queue.

Finally, a C++ virtual adapter named for `00BF65AC` is not the original Native
free entry or its ABI. Reusing the host canonical free establishes a Source
allocation/release contract only. It proves neither arbitrary cross-CRT pointer
compatibility nor Native exception behavior, and it does not establish a current
runtime heap fault merely from the old Source spellings.

## Existing checks and replay

After a separately authorized implementation, the required Win32 build is
`scripts/build.ps1`. The existing CTest registrations are `reconstructed_math`,
conditionally `native_math_differential`, and conditionally `tool_tests`.
The complete eight-file test query has no direct use of the two target adapters
or their concrete subclass. Its only canonical-service hit is the device-registry
manager fixture's free at `tests/math_tests.cpp:249`; that is not a default-adapter
domain test.

The retained R125 report records 71 historical paired container cases and two
Source failure cases; R121 records 52 parent cases and four Source failure cases.
R121 explicitly used free callbacks that retained fixture memory. Their call-order,
alias and exception observations are relevant existing evidence, but their old
results cannot certify newly changed free defaults. Before reusing such a fixture,
qualify its actual allocation and release overrides. This packet only reads the
whole reports and records current Source/hash differences; it does not execute or
recapture those Native probes. No broad suite or new permanent tests are proposed.

The self-contained local bundle supports:

```
python local/cc12_game_free_adapter_domain_readiness/verify.py
python local/cc12_game_free_adapter_domain_readiness/verify.py --original-worktree
python local/cc12_game_free_adapter_domain_readiness/relocate_verify.py
```

Default replay allows read-only file access only within this bundle and the report,
and rejects subprocess, network, dynamic-loading and filesystem-mutation activity.
It verifies the full Source archives, fifteen overlays, complete accepted wide
manifest, include closure, complete-corpus queries, backed excerpts, test queries,
and recomputed audit. The separate worktree mode checks actual Git objects and
current working bytes. The one-shot relocation helper copies every relied artifact
and the exact report into a fresh directory and runs guarded replay there.

The resulting decision is ready for primary acceptance of the bounded dormant
Source correction. Actual application/game ownership, queue binding, unrelated
provider domains, Native ABI, and runtime/game validation remain outside that
decision.
