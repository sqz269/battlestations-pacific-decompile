# CC12 game-state queue application owner readiness

This Source-only review does **not** authorize production C++ changes. The existing
raw game allocation frame is a candidate owner, but the startup path that runs
still owns projected state and a separate semantic queue. A queue adapter cannot
yet borrow `game + 0x5D8` from that startup path. The first missing composition is
the actual application/raw game owner, its genuine publication cells, and its
construction/lifetime services.

Packet: `cc12_game_state_queue_application_owner_readiness`. Owned outputs are
this document, `reports/cc12_game_state_queue_application_owner_readiness.json`,
and the ignored `local/cc12_game_state_queue_application_owner_readiness` bundle.
There are no leased Native addresses, new Native queries, new decoding, C++ edits,
compiler invocations, tests, runtime executions, or Ghidra mutations.

## Frozen Source and accepted dependencies

The review freezes worker HEAD `6f26f8036af3a2d3f342db0daebcbfedf6d116e7` and all
4,206 tracked `src/` and `include/` files. Every Git blob and raw working file is
checked. The Source tree matches the preceding string-cleanup review and main
`ee48bf2ecaf95e0af38197ab135ea01d2ec996ae`, checked immediately before freezing.
The current accepted epoch is `Source3352-particle-model-typed-views`: the original
3 Source3350 overlays, 6 Source3352 overlays, 2 startup overlays, and 2 typed-model
header overlays. Complete retained epoch manifests qualify those overlays. No
widened-copy implementation had entered this frozen Source tree; this review does
not adopt pending work or claim a later main snapshot.

The expanded owner/consumer closure contains 483 complete Source files from 35
roots and 890 quoted include edges. Its 24 queries scan the entire current
4,206-file Source corpus and retain all 4,303 matching lines, including empty
queries. The audit backs 72 excerpts with complete Source files and 13 excerpts
with complete retained documents. SDK/system includes are recorded as external;
they do not become native, platform, or gameplay proof.

Whole worker and accepted Root reports for the request wrapper, enqueue, growth,
map allocator, failure owner, and selected failure handler are retained directly
as Git artifacts. The string-cleanup worker `a97fabfc637cb4a26d388bafb5675401c7eb82c6`
is likewise retained; its Root acceptance was pending at capture. These prior
Native results are dependencies, not new analysis here. No older Root proof tree,
callee body, CRT provider, EH runtime, or unrelated artifact is reopened.

## The owner that current startup actually uses

The supplied names `GameApplicationState` and `FrameContext` have no exact
word-boundary matches in current Source. Renderer begin/end frame context types
have different names and responsibilities. The relevant concrete types are
`GameStartupHost`, `ApplicationFrameState`, `GameStateSlot`, `GameFrameHost`,
`GameNativeGameApplicationFrame`, and `GameNativeGameRuntime`.

`src/game_main.cpp:1201` constructs `GameStartupHost`. That host owns
`ApplicationFrameState frame_state_` and `GameStateSlot game_state_`
(`include/bsp/game_hosts.hpp:721`, `:739`). It has no raw `NativeGameStorage`,
`GameNativeGameRuntime`, or `GameNativeGameApplicationFrame` owner. Its
`application_construct` resets `ApplicationFrameState` and explicitly says the
native subsystem pointers at application `+4..+14` remain unreconstructed
(`src/game_hosts.cpp:1835`). The two-byte application projection is not a raw
application image (`include/bsp/app_frame.hpp:7`).

`GameStateSlot` owns separate value, pending-request-count, and held fields
(`include/bsp/app_frame_game_state.hpp:33`). Their comments identify corresponding
game offsets; the struct does not provide the native game allocation or the
20-byte queue header. `GameFrameHost::game_state` reads `slot.value` through the
existing helper, while `game_on_move` calls the menu frame
(`src/game_hosts.cpp:1073`, `:1101`; `src/app_frame_game_state.cpp:24`).

Startup passes the same projected `game_state_` to `GameMenuHost` and
`GameFrameHost` (`src/game_hosts.cpp:2313`, `:2373`). `GameMenuHost::Impl` borrows
that state and owns its separate `GameStateRequestQueue requests`
(`src/game_hosts_menu.cpp:118`, `:150`). The live menu route is:

1. `MenuPathHost::request_game_state` calls the existing semantic enqueue on
   `owner_.requests`, then mirrors its count to `owner_.state.pending_requests`.
2. Hold release changes the projected `requests_held` flag.
3. Both the menu-path drain and front-end drain call `front_state_request` and
   `pop_front_state_request` on that same semantic queue, mirror its new count,
   and store the dequeued value into the projected state.

These paths are at `src/game_hosts_menu.cpp:1358`, `:1364`, `:1369`, and `:973`.
The generic `drain_state_requests_004e4430` likewise consumes a
`GameFrameControlState::requests` semantic queue (`src/game_frame_control.cpp:194`).
Its presence does not connect the menu to a raw game header.

The semantic queue owns a vector of `unique_ptr<GameStateRequestBlock>` plus head
and count fields. Enqueue takes the request by value, grows through its vector
helper, and value-initializes newly allocated blocks. The accepted Native wrapper
and enqueue instead require a caller-slot pointer and late pointee read after
growth/allocation, with a raw map/header. Existing Source ordering, allocation,
layout, and exception behavior cannot be treated as that raw contract. The title's
literal request `4` alone does not establish an observable mismatch.

## Existing raw allocation candidate

`NativeGameStorage` is already a trivial aligned `0x71A0`-byte extent, explicitly
not an invented full game layout (`include/bsp/native_game_construction.hpp:17`).
`GameNativeGameApplicationFrame` already owns a retained allocation of this type;
no new private game or queue allocation is needed to describe the candidate.

Its existing `construct` performs the canonical object allocation, placement
construction and whole-allocation zeroing, captures the borrowed duplicate name,
prepares the raw name header in the borrowed canonical string-pool context, and
calls the exclusive runtime with that same retained storage. On normal return it
publishes the result to the borrowed genuine application `+14` cell and then
returns the name header (`src/game_native_game_application_frame.cpp:50`). The
header explicitly requires all real construction/lifetime services, the genuine
application and name cells, and the same raw string domain; it promises no
automatic cleanup, rollback, or replay (`include/bsp/game_native_game_application_frame.hpp:8`).

`GameNativeGameRuntime` borrows the allocation. It verifies shared game/grid
publication reference identities and canonical dynamics owners, then records the
same storage passed to the raw constructor. Its physics entry checks that the
current actual game publication still equals its recorded owner before reading
the world (`src/game_native_game_runtime.cpp:13`, `:49`, `:66`). These checks do
not create or publish missing application services.

The raw constructor clears game `+5DC/+5E0/+5E4/+5E8`, clears the hold byte at
`+5EC`, later initializes state at `+5D4`, and publishes the game through its
borrowed actual `00E188A8` cell (`src/native_game_construction.cpp:132`, `:198`,
`:202`). The constructor preserves the unlisted leading queue DWORD at `+5D8`;
the enclosing application frame's earlier whole-allocation zeroing is a distinct
operation and must not be silently attributed to the constructor itself.

Complete-corpus usage checks find `GameNativeGameApplicationFrame` only in its
own header and implementation. `GameNativeGameRuntime` appears only in its own
files, that application frame, and fixed-step attachment declarations/definitions.
`attach_native_game` has exactly two occurrences: its declaration and definition.
There is no production instantiation of either raw owner and no attachment call.
The fixed-step host records the unbound physics site when `native_game_` is absent
(`src/game_hosts_fixed_step.cpp:268`). An existing capable class is therefore not
evidence that current startup owns or supplies the raw game allocation.

## Required shared bytes and lifetime

For a qualified live game owner `G` selected at the accepted wrapper call boundary,
the only permissible queue borrow is `Q = byte_address(G) + 0x5D8`:

| Queue offset | Game offset | Existing raw field |
| --- | --- | --- |
| `+00` | `+5D8` | Opaque leading DWORD |
| `+04` | `+5DC` | Map pointer |
| `+08` | `+5E0` | Map capacity DWORD |
| `+0C` | `+5E4` | Head offset DWORD |
| `+10` | `+5E8` | Request count DWORD |

The state DWORD at `+5D4` and hold byte at `+5EC` are adjacent fields outside this
20-byte header. Producer, pending-count/hold checks, drain, and final raw cleanup
must all use that same game allocation and field identity. A copied header,
private replacement queue, fixed captured menu owner substituted for a required
current-game lookup, or a cast of `GameStateSlot`/vector storage would not satisfy
the accepted contracts.

The existing raw lifetime route passes exactly `G+5D8` to
`destroy_native_game_pointer_blocks_004c4a50` (`src/native_game_lifetime.cpp:119`,
`:239`). That helper drains count to zero, clears head when count transitions to
zero, frees nonnull map entries in reverse while reloading the current map,
frees the current map, and clears map/capacity; the leading DWORD is preserved
(`src/native_game_container_lifetime.cpp:65`). This is a retained Source teardown
contract, with no fresh Native teardown analysis in this packet.

Raw game destruction clears the current-game publication earlier, at
`src/native_game_lifetime.cpp:70`, before the queue cleanup. A future composition
must retain the actual game storage and required providers through that later
cleanup without treating a cleared publication as storage ownership. Request
producers and consumers must retire before the game allocation is released.

The application frame marks allocation disposition indeterminate and clears its
retained pointer before scalar deletion, since an override can release and then
throw. A successful `flags & 1` deletion records release; successful non-freeing
deletion preserves the allocation for explicitly qualified later free. Diagnostic
acknowledgements perform no cleanup (`src/game_native_game_application_frame.cpp:77`).
Those policies must survive any composition. Current menu adapters instead borrow
`Impl` on the stack, and startup destroys menu before frontend
(`src/game_hosts.cpp:1595`, `:2502`); that separate lifetime does not retire raw
queue borrows or establish game ownership.

## Provider and readiness boundary

The canonical allocation service already performs `malloc`, new-handler retry,
and `bad_alloc`, with explicit native/host byte counts
(`src/singleton_lifetime.cpp:51`). Accepted exception SBO string cleanup uses
`singleton_lifetime_free`/`std::free` for that Source-owned heap. The raw name
header prepared by the application frame uses its separately borrowed canonical
raw string-pool context; these two string representations must not be conflated.

Both raw game container and game lifetime adapters currently spell
`free_00BF65AC` as `::operator delete`, while `free_00BF6989` uses `std::free`
(`src/native_game_container_lifetime.cpp:39`; `src/native_game_lifetime.cpp:134`).
The Source spellings alone do not prove arbitrary pointer-domain interchange or
the Native CRT free contract. This review does not assert runtime incompatibility;
it requires an explicit matching contract for queue map allocation, 16-byte block
allocation, growth's old-map release, and every final block/map release. A fresh
parallel allocator or string domain would add an unsupported ownership boundary.

The accepted failure owner, selected EH action and SBO cleanup remain qualified
dependencies. In particular, the selected action's runtime frame-base premise
does not become general Native EH/runtime equivalence merely because the local
cleanup body is understood. No host exception or unresolved forwarder is proposed
as a substitute.

The smallest next Source family is actual `GameStartupHost` application/raw-game
ownership and publication composition, including its existing constructor and
lifetime prerequisites. Only after that owner is concrete can a non-owning queue
view and the Native-order producer/consumer binding be made reviewable. This
packet requests no additional Native address and grants no standalone queue,
production C++, binary ABI, startup, platform, or game-validation approval.

## Replay

The ignored bundle contains the complete Source archives, current overlays,
selected whole files, whole dependency reports/documents, helper scripts, and
the immutable freeze. It is self-contained under this one relative root.

```
python local/cc12_game_state_queue_application_owner_readiness/verify.py
python local/cc12_game_state_queue_application_owner_readiness/verify.py --original-worktree
python local/cc12_game_state_queue_application_owner_readiness/relocate_verify.py
```

Default verification installs a guard that allows read-only opens only beneath
that retained root and the report; it rejects subprocesses, network, dynamic
library loading, and filesystem mutations. It recomputes the full Source corpus,
overlays, include closure, queries, excerpts and audit. The separate worktree mode
also checks actual Git objects and every current raw working Source file. Physical
relocation copies all relied artifacts plus the exact report into a fresh directory
and runs the guarded verifier there. The relocation helper deliberately requires
an absent destination and is a one-shot evidence capture.

All of these checks establish artifact identity and Source-review reproducibility.
They add no compiler, test, ABI, Native execution, SDK/OS, or gameplay validation.
